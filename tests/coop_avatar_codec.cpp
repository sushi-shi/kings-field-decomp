#include <kf/lib/avatar.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <memory>
#include <source_location>
#include <span>
#include <string>
#include <vector>

namespace {
using Bytes = std::vector<std::uint8_t>;
using Pack = std::unique_ptr<KfAvatarPack, decltype(&kf_avatar_close)>;
using Import = std::unique_ptr<KfAvatarImport, decltype(&kf_avatar_import_close)>;
void require(bool value, const char *message, std::source_location where = std::source_location::current())
{
    if (!value) { std::fprintf(stderr, "%s:%u: %s\n", where.file_name(), where.line(), message); std::abort(); }
}
void put16(Bytes &bytes, std::size_t at, std::uint16_t value)
{ bytes.at(at) = value; bytes.at(at + 1) = value >> 8; }
void put32(Bytes &bytes, std::size_t at, std::uint32_t value)
{ put16(bytes, at, value); put16(bytes, at + 2, value >> 16); }
void both32(Bytes &bytes, std::size_t at, std::uint32_t value)
{
    put32(bytes, at, value);
    for (unsigned i = 0; i < 4; ++i) bytes.at(at + 4 + i) = value >> ((3 - i) * 8);
}
void append16(Bytes &bytes, std::uint16_t value)
{ bytes.push_back(value); bytes.push_back(value >> 8); }
void append32(Bytes &bytes, std::uint32_t value)
{ append16(bytes, value); append16(bytes, value >> 16); }
void copy(Bytes &bytes, std::size_t at, std::span<const std::uint8_t> source)
{
    require(at <= bytes.size() && source.size() <= bytes.size() - at, "fixture copy extent");
    std::copy(source.begin(), source.end(), bytes.begin() + at);
}
void text(Bytes &bytes, std::size_t at, const char *value)
{ copy(bytes, at, {reinterpret_cast<const std::uint8_t *>(value), std::strlen(value)}); }
Bytes volume(std::uint32_t sectors = 32)
{
    Bytes bytes(2048); text(bytes, 0, "\1CD001\1"); text(bytes, 40, "SLUS-00255                      ");
    both32(bytes, 80, sectors); bytes[129] = bytes[130] = 8;
    bytes[156] = 34; bytes[181] = 2; both32(bytes, 158, 20); both32(bytes, 166, 2048);
    return bytes;
}
Bytes row(const char *name, std::uint32_t sector, std::uint32_t size, bool directory)
{
    const auto length = std::strlen(name); Bytes bytes(33 + length + (length % 2 == 0));
    bytes[0] = bytes.size(); bytes[25] = directory ? 2 : 0; bytes[32] = length;
    both32(bytes, 2, sector); both32(bytes, 10, size); text(bytes, 33, name); return bytes;
}
Bytes pack_fixture()
{
    Bytes bytes {'K','F','A','1',1,0,1,0,41,0,1,0,1,0,0,0};
    bytes.resize(16 + 60); bytes.resize(bytes.size() + 1024, 255); return bytes;
}
void reject_pack(const Bytes &bytes)
{ Pack pack(kf_avatar_open(bytes.data(), bytes.size()), kf_avatar_close); require(!pack, "malformed presentation pack accepted"); }
void pack_bounds()
{
    const auto bytes = pack_fixture(); Pack pack(kf_avatar_open(bytes.data(), bytes.size()), kf_avatar_close);
    require(pack && kf_avatar_count(pack.get()) == 1, "valid pack rejected");
    KfAvatarMesh mesh {}; require(kf_avatar_mesh(pack.get(), 0, &mesh) && mesh.slot == 41 && mesh.triangles == 1 && mesh.height == 1,
        "mesh metadata changed");
    require(kf_avatar_vertices(pack.get(), 0) && kf_avatar_rgba(pack.get(), 0), "mesh views missing");
    mesh.slot = 1234;
    require(!kf_avatar_mesh(pack.get(), 1, &mesh) && mesh.slot == 1234 && !kf_avatar_vertices(pack.get(), 1) && !kf_avatar_rgba(pack.get(), 1),
        "invalid mesh index modified output");
    for (std::size_t length = 0; length < bytes.size(); ++length) {
        Pack short_pack(kf_avatar_open(bytes.data(), length), kf_avatar_close); require(!short_pack, "truncated pack accepted");
    }
    for (auto [at, value] : {std::pair{4,2}, {6,0}, {8,KF_AVATAR_SLOTS}, {10,0}, {12,0}, {17,127}, {23,127}, {29,1}, {30,1}, {35,2}}) {
        auto bad = bytes; bad[at] = value; reject_pack(bad);
    }
    auto bad = bytes; bad.push_back(0); reject_pack(bad);
    bad = bytes; bad[6] = 2; bad.insert(bad.end(), bytes.begin() + 8, bytes.end()); reject_pack(bad);
    for (unsigned field : {16, 18, 20}) for (int value : {-8193, 8193}) { bad = bytes; put16(bad, field, value); reject_pack(bad); }
    for (unsigned field : {22, 24, 26}) for (int value : {-4097, 4097}) { bad = bytes; put16(bad, field, value); reject_pack(bad); }
    for (int value : {-8192, 8192}) {
        auto edge = bytes; put16(edge, 16, value); put16(edge, 22, value / 2);
        Pack accepted(kf_avatar_open(edge.data(), edge.size()), kf_avatar_close); require(bool(accepted), "inclusive vertex/normal edge rejected");
    }
    for (unsigned count : {0u, 4097u, UINT32_MAX}) { bad = bytes; put32(bad, 12, count); reject_pack(bad); }
    bad = bytes; put16(bad, 10, 4097); reject_pack(bad);
    require(!kf_avatar_open(bytes.data(), KF_AVATAR_MAX_BYTES + 1u) && !kf_avatar_open(nullptr, 0), "unbounded/null pack accepted");
    require(kf_avatar_count(nullptr) == 0 && !kf_avatar_vertices(nullptr, 0) && !kf_avatar_rgba(nullptr, 0), "null pack access");
}
Import importer(std::uint32_t size = 32 * 2048)
{ Import value(kf_avatar_import_open(size), kf_avatar_import_close); require(bool(value), "cannot create fixture importer"); return value; }
void request_is(const Import &value, unsigned offset, unsigned length)
{
    KfAvatarRead read {}; require(kf_avatar_import_request(value.get(), &read) == 1 && read.offset == offset && read.length == length,
        "unexpected sliced disc request");
}
void supply(const Import &value, const Bytes &bytes)
{ require(kf_avatar_import_supply(value.get(), bytes.data(), bytes.size()) == 1, "valid disc slice rejected"); }
void failed(const Import &value)
{
    KfAvatarRead read {123,456}; std::uint32_t length = 789;
    require(kf_avatar_import_request(value.get(), &read) == -1 && read.offset == 123 && read.length == 456 &&
        !kf_avatar_import_result(value.get(), &length) && length == 789, "failed importer published output");
    std::array<std::uint8_t, 8> error; error.fill(0xa5);
    kf_avatar_import_error(value.get(), error.data(), error.size() - 1);
    require(error[0] != 0 && error[6] == 0 && error[7] == 0xa5, "error message not bounded/terminated");
}
void disc_read_bounds()
{
    auto iso = importer(); request_is(iso, 16 * 2048, 2048); supply(iso, volume()); request_is(iso, 20 * 2048, 2048);
    Bytes short_read(2047); require(!kf_avatar_import_supply(iso.get(), short_read.data(), short_read.size()), "short disc read accepted"); failed(iso);
    auto bin = importer(32 * 2352); supply(bin, Bytes(2048)); request_is(bin, 16 * 2352 + 24, 2048);
    supply(bin, volume()); request_is(bin, 20 * 2352 + 24, 2048);
    for (unsigned at : {40,80,84,128,130,157,181,158,166}) {
        auto bad = volume(); bad[at] ^= 1; auto value = importer();
        require(!kf_avatar_import_supply(value.get(), bad.data(), bad.size()), "invalid ISO volume accepted"); failed(value);
    }
    for (unsigned size : {0u, 17 * 2352u - 1, 801 * 1024 * 1024u})
        require(!kf_avatar_import_open(size), "invalid disc size accepted");
    auto bad = volume(); both32(bad, 80, 0); auto zero = importer();
    require(!kf_avatar_import_supply(zero.get(), bad.data(), bad.size()), "empty ISO volume accepted"); failed(zero);
    bad = volume(); both32(bad, 158, 32); auto outside = importer();
    require(!kf_avatar_import_supply(outside.get(), bad.data(), bad.size()), "outside-volume root accepted"); failed(outside);
}
void directory_bounds()
{
    Bytes directory(2048); copy(directory, 0, row("CD", 20, 2048, true));
    for (unsigned mode = 0; mode < 7; ++mode) {
        auto value = importer(); supply(value, volume()); auto bad = directory;
        if (mode == 1) bad[0] = 33;
        if (mode == 2) bad[32] = 255;
        if (mode == 3) bad[33] = '/';
        if (mode == 4) bad[25] |= 128;
        if (mode == 5) bad[26] = 1;
        if (mode == 6) { both32(bad, 2, 21); both32(bad, 10, 1024 * 1024 + 1); }
        require(!kf_avatar_import_supply(value.get(), bad.data(), bad.size()), "cyclic/malformed directory accepted"); failed(value);
    }
    auto value = importer(); supply(value, volume());
    directory.assign(2048, 0); copy(directory, 0, row("CD", 21, 2048, true)); supply(value, directory);
    directory.assign(2048, 0); copy(directory, 0, row("COM", 22, 2048, true)); supply(value, directory);
    directory.assign(2048, 0); auto file = row("MO.T;1", 23, 2048, false); copy(directory, 0, file); copy(directory, file.size(), file);
    require(!kf_avatar_import_supply(value.get(), directory.data(), directory.size()), "duplicate archive accepted"); failed(value);
}

Bytes model()
{
    Bytes bytes;
    for (std::uint32_t value : {100,0,12,0x41,0,1,28,3,52,1,60,1,0}) append32(bytes, value);
    for (std::int16_t value : {0,0,0,0,100,0,0,0,0,100,0,0,0,0,4096,0}) append16(bytes, value);
    for (std::uint8_t value : {4,3,0,0x20,128,64,32,0}) bytes.push_back(value);
    for (std::uint16_t value : {0,0,1,2}) append16(bytes, value);
    require(bytes.size() == 100, "model fixture size"); return bytes;
}
Bytes archive(const std::vector<Bytes> &entries)
{
    Bytes bytes(2048); put16(bytes, 0, entries.size());
    for (std::size_t i = 0; i < entries.size(); ++i) {
        put16(bytes, 2 + i * 2, bytes.size() / 2048);
        bytes.insert(bytes.end(), entries[i].begin(), entries[i].end()); bytes.resize((bytes.size() + 2047) / 2048 * 2048);
    }
    put16(bytes, 2 + entries.size() * 2, bytes.size() / 2048); return bytes;
}
struct SourceFixture {
    std::vector<Bytes> models = std::vector<Bytes>(43, model());
    std::vector<Bytes> textures = std::vector<Bytes>(28, Bytes(80));
    std::vector<Bytes> data = std::vector<Bytes>(85, Bytes(16));
    std::vector<Bytes> objects = std::vector<Bytes>(546);
    SourceFixture() {
        for (unsigned slot : {10,11,12}) models[slot].clear();
        objects[545] = model();
        for (auto &t : textures) std::fill_n(t.begin(), 64, 255);
        for (unsigned area = 0; area < 28; ++area) {
            auto &db = data[area * 3 + 1]; db.resize(4 + 32 * 120); put32(db, 0, 12992);
            for (unsigned i = 0; i < 32; ++i) db[4 + i * 120] = area < 2 && area * 32 + i < 43 ? area * 32 + i : 255;
        }
    }
    std::array<Bytes, 4> archives() const { return {archive(models), archive(textures), archive(data), archive(objects)}; }
};
Bytes iso_image(const std::array<Bytes, 4> &archives)
{
    Bytes disc(23 * 2048); copy(disc, 20 * 2048, row("CD", 21, 2048, true)); copy(disc, 21 * 2048, row("COM", 22, 2048, true));
    constexpr const char *names[] = {"MO.T;1", "RTIM.T;1", "FDAT.T;1", "MOF.T;1"}; std::size_t at = 22 * 2048;
    for (unsigned i = 0; i < archives.size(); ++i) {
        auto record = row(names[i], disc.size() / 2048, archives[i].size(), false); copy(disc, at, record); at += record.size();
        disc.insert(disc.end(), archives[i].begin(), archives[i].end());
    }
    copy(disc, 16 * 2048, volume(disc.size() / 2048)); return disc;
}
Bytes import_disc(const Bytes &disc, bool expected = true, std::source_location where = std::source_location::current())
{
    auto value = importer(disc.size()); unsigned reads = 0;
    for (;;) {
        KfAvatarRead read {}; auto state = kf_avatar_import_request(value.get(), &read);
        if (state == 0) {
            require(expected, "invalid model/texture/archive imported", where);
            std::uint32_t size = 0; auto bytes = kf_avatar_import_result(value.get(), &size);
            require(bytes && size, "complete import has no result", where); return {bytes, bytes + size};
        }
        require(state == 1 && ++reads <= 12 && read.offset <= disc.size() && read.length <= disc.size() - read.offset,
            "unbounded import request", where);
        if (!kf_avatar_import_supply(value.get(), disc.data() + read.offset, read.length)) {
            if (expected) { std::array<std::uint8_t, 512> error {}; kf_avatar_import_error(value.get(), error.data(), error.size()); std::fprintf(stderr, "%s\n", error.data()); }
            require(!expected, "valid source disc rejected", where); failed(value); return {};
        }
    }
}
Bytes raw_track(const Bytes &iso)
{
    Bytes bin(iso.size() / 2048 * 2352, 0xa5);
    for (std::size_t sector = 0; sector < iso.size() / 2048; ++sector) copy(bin, sector * 2352 + 24, std::span(iso).subspan(sector * 2048, 2048));
    return bin;
}
void source_bounds_and_rounding()
{
    SourceFixture source; const auto valid = source.models[0];
    auto bytes = import_disc(iso_image(source.archives()));
    Pack pack(kf_avatar_open(bytes.data(), bytes.size()), kf_avatar_close);
    require(pack && kf_avatar_count(pack.get()) == 39, "synthetic roster coverage changed");
    KfAvatarMesh mesh {}; require(kf_avatar_mesh(pack.get(), 0, &mesh) && mesh.slot == 0 && mesh.triangles == 1 && mesh.height == 1,
        "synthetic mesh metadata changed");
    auto v = kf_avatar_vertices(pack.get(), 0);
    require(v[0].y == -2000 && v[1].x == 2000 && v[2].y == 0 && v[0].nz == 4096 && v[0].r == 128 && v[0].g == 64 && v[0].b == 32,
        "source triangle conversion changed");
    require(import_disc(raw_track(iso_image(source.archives()))) == bytes, "ISO/BIN source conversion differs");
    for (unsigned at : {24,28,32,36,40,44,48}) {
        source.models[0] = valid; put32(source.models[0], at, UINT32_MAX); import_disc(iso_image(source.archives()), false);
    }
    for (auto [at, value] : {std::pair{98,3}, {81,127}, {86,8}}) {
        source.models[0] = valid; source.models[0][at] = value; import_disc(iso_image(source.archives()), false);
    }
    for (unsigned size : {0,11,99,2049}) {
        source.models[0] = valid; put32(source.models[0], 0, size); import_disc(iso_image(source.archives()), false);
    }
    source.models[0] = valid; put32(source.models[0], 44, 0); import_disc(iso_image(source.archives()), false);
    source.models[0] = valid; put16(source.models[0], 70, 0); import_disc(iso_image(source.archives()), false);
    source.models[0] = valid; put16(source.models[0], 52, 32767); import_disc(iso_image(source.archives()), false);
    source.models[0] = valid;
    for (unsigned index = 0; index < 3; ++index) {
        constexpr int x[] = {1,3,-5}, z[] = {-1,-3,5}, y[] = {-2000,0,2000};
        put16(source.models[0], 52 + index * 8, x[index]); put16(source.models[0], 54 + index * 8, y[index]);
        put16(source.models[0], 56 + index * 8, z[index]);
    }
    bytes = import_disc(iso_image(source.archives())); pack.reset(kf_avatar_open(bytes.data(), bytes.size())); v = kf_avatar_vertices(pack.get(), 0);
    require(v && v[0].x == 0 && v[1].x == 2 && v[2].x == -2 && v[0].z == 0 && v[1].z == -2 && v[2].z == 2,
        "negative/positive nearest-even ties changed");
    source.models[0] = valid; auto archives = source.archives();
    for (unsigned at : {0,2,4}) { auto bad = archives; put16(bad[0], at, 0); import_disc(iso_image(bad), false); }
    auto bad = archives; put16(bad[0], 0, 1023); import_disc(iso_image(bad), false);
}
Bytes red_rect()
{
    Bytes bytes; for (std::uint16_t value : {0,0,1,1,0,0,1,1,31}) append16(bytes, value); return bytes;
}
void texture_bounds()
{
    SourceFixture source; auto &model_bytes = source.models[0]; model_bytes.resize(108);
    put32(model_bytes, 0, 108); model_bytes[85] = 5; model_bytes[87] = 0x24;
    std::fill(model_bytes.begin() + 88, model_bytes.end(), 0); put16(model_bytes, 94, 256);
    put16(model_bytes, 100, 0); put16(model_bytes, 102, 0); put16(model_bytes, 104, 1); put16(model_bytes, 106, 2);
    auto rect = red_rect(); source.data[84] = rect; source.data[84].insert(source.data[84].end(), rect.begin(), rect.end());
    const auto valid = source.data[84]; auto bytes = import_disc(iso_image(source.archives()));
    Pack pack(kf_avatar_open(bytes.data(), bytes.size()), kf_avatar_close); KfAvatarMesh mesh {};
    require(kf_avatar_mesh(pack.get(), 0, &mesh) && mesh.height == 2, "textured atlas extent changed");
    const auto rgba = kf_avatar_rgba(pack.get(), 0);
    require(rgba && rgba[1024] == 255 && rgba[1025] == 0 && rgba[1026] == 0 && rgba[1027] == 255, "16-bit texture color changed");
    model_bytes[88] = 1; import_disc(iso_image(source.archives()), false); model_bytes[88] = 0;
    put16(model_bytes, 94, 0); put16(model_bytes, 90, 1); import_disc(iso_image(source.archives()), false);
    put16(model_bytes, 94, 384); import_disc(iso_image(source.archives()), false); put16(model_bytes, 94, 256); put16(model_bytes, 90, 0);
    for (unsigned at : {0,2,4,6,8,10,12,14}) {
        source.data[84] = valid; put16(source.data[84], at, 65535); import_disc(iso_image(source.archives()), false);
    }
    source.data[84] = valid; source.textures[0][0] = 0; import_disc(iso_image(source.archives()), false);
}
Bytes read_file(const char *path)
{
    std::ifstream stream(path, std::ios::binary | std::ios::ate); require(bool(stream), "cannot open character pack");
    auto length = stream.tellg(); require(length > 0 && length <= KF_AVATAR_MAX_BYTES, "invalid character pack file extent");
    Bytes bytes(static_cast<std::size_t>(length)); stream.seekg(0); stream.read(reinterpret_cast<char *>(bytes.data()), bytes.size());
    require(bool(stream), "incomplete character pack read"); return bytes;
}
void retail_parity(const char *disc_path, const char *pack_path)
{
    std::ifstream stream(disc_path, std::ios::binary | std::ios::ate); require(bool(stream), "cannot open retail disc");
    const auto size = stream.tellg(); require(size > 0 && size <= 800 * 1024 * 1024, "invalid retail disc extent");
    auto value = importer(static_cast<std::uint32_t>(size)); unsigned reads = 0; std::size_t bytes_read = 0;
    for (;;) {
        KfAvatarRead read {}; const auto state = kf_avatar_import_request(value.get(), &read);
        if (!state) break;
        require(state == 1 && read.length <= 40 * 1024 * 1024 && std::uint64_t(read.offset) + read.length <= static_cast<std::uint64_t>(size),
            "retail importer request escaped disc");
        Bytes bytes(read.length); stream.seekg(read.offset); stream.read(reinterpret_cast<char *>(bytes.data()), bytes.size());
        require(bool(stream), "incomplete retail disc read"); supply(value, bytes); ++reads; bytes_read += bytes.size();
    }
    std::uint32_t length = 0; auto result = kf_avatar_import_result(value.get(), &length); const auto expected = read_file(pack_path);
    require(result && length == expected.size() && std::equal(expected.begin(), expected.end(), result), "retail importer changed pack bytes");
    std::printf("Retail character parity: %u reads, %zu bytes read, %u identical pack bytes\n", reads, bytes_read, length);
}
}
int main(int argc, char **argv)
{
    require(argc == 1 || argc == 3, "usage: coop-avatar-codec-test [SLUS-disc expected-pack]");
    pack_bounds(); disc_read_bounds(); directory_bounds(); source_bounds_and_rounding(); texture_bounds();
    if (argc == 3) retail_parity(argv[1], argv[2]);
    std::puts("Avatar pack/disc/model/texture bounds and rounding passed");
}
