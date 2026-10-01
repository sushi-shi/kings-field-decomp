#include "avatar_decode.h"

#include <algorithm>
#include <map>
#include <optional>

namespace kf::avatar {
namespace {
struct Archive {
    Bytes bytes;
    std::vector<std::size_t> offsets;
    explicit Archive(Bytes data) : bytes(data) {
        auto count = u16_at(data, 0);
        require(count >= 1 && count <= 1022 && data.size() % 2048 == 0, "Invalid KFIII archive header");
        for (unsigned i = 0; i <= count; ++i) offsets.push_back(std::size_t(u16_at(data, 2 + i * 2)) * 2048);
        require(offsets.front() == 2048 && offsets.back() == data.size() && std::is_sorted(offsets.begin(), offsets.end()), "Invalid KFIII archive offsets");
    }
    Bytes get(std::size_t slot) const {
        require(slot < offsets.size(), "Missing archive slot");
        const auto end = std::find_if(offsets.begin() + slot + 1, offsets.end(), [&](auto offset) { return offset > offsets[slot]; });
        require(end != offsets.end(), "Empty archive slot"); return slice(bytes, offsets[slot], *end - offsets[slot]);
    }
};
using Vec3 = std::array<s16, 3>;
using Material = std::pair<u16, u16>;
struct Face {
    std::vector<std::size_t> vertices, normals;
    std::vector<std::array<u8, 3>> colors;
    std::vector<std::array<u8, 2>> uv;
    std::optional<Material> material;
    bool unlit;
};
struct Object { std::vector<Vec3> vertices, normals; std::vector<Face> faces; };
std::vector<Vec3> vectors(Bytes data, std::size_t offset, std::size_t count)
{
    Reader r(slice(data, offset, count * 8)); std::vector<Vec3> result;
    result.reserve(count);
    for (std::size_t i = 0; i < count; ++i) { result.push_back({r.s16_le(), r.s16_le(), r.s16_le()}); r.skip(2); }
    return result;
}
std::vector<Object> model(Bytes data)
{
    const auto size = u32_at(data, 0), animations = u32_at(data, 4), tmd = u32_at(data, 8);
    require(size >= 12 && animations <= 256 && tmd >= 12 && tmd < size, "Invalid KFIII model header");
    data = slice(data, 0, size); Reader header(slice(data, tmd, size - tmd));
    require(header.u32_le() == 0x41 && header.u32_le() == 0, "Expected a relative TMD model");
    auto count = header.u32_le(); require(count >= 1 && count <= 64, "Invalid TMD object count");
    data = slice(data, tmd + 12, size - tmd - 12); auto table = slice(data, 0, count * 28);
    std::vector<Object> objects; std::size_t triangles = 0, total_vertices = 0;
    for (unsigned i = 0; i < count; ++i) {
        auto row = slice(table, i * 28, 28); auto nv = u32_at(row, 4), nn = u32_at(row, 12), np = u32_at(row, 20);
        require(nv >= 1 && nv <= 16384 && nn <= 16384 && np <= 4096 && u32_at(row, 24) == 0, "Character exceeds mesh limits");
        total_vertices += nv + nn; require(total_vertices <= 65536, "Character exceeds mesh limits");
        Object object {vectors(data, u32_at(row, 0), nv), vectors(data, u32_at(row, 8), nn), {}};
        for (const auto &normal : object.normals) for (auto n : normal) require(n >= -4096 && n <= 4096, "Invalid character normal");
        auto offset = u32_at(row, 16); Reader primitives(slice(data, offset, data.size() >= offset ? data.size() - offset : 0));
        for (unsigned primitive = 0; primitive < np; ++primitive) {
            auto h = primitives.take(4); const auto flags = h[2], mode = h[3]; auto body = primitives.take(std::size_t(h[1]) * 4);
            require((mode & 0xe0) == 0x20 && !(flags & ~7), "Unsupported TMD primitive");
            const unsigned corners = mode & 8 ? 4 : 3; const bool textured = mode & 4, smooth = mode & 16, unlit = flags & 1;
            triangles += corners - 2; require(triangles <= 4096, "Character triangle limit exceeded");
            std::size_t indices = textured ? corners * 4 : 4;
            Face face {{}, {}, std::vector<std::array<u8, 3>>(corners, {128,128,128}), {}, {}, unlit};
            if (!textured || unlit) {
                const auto at = textured ? corners * 4 : 0; const auto colors = flags & 4 ? corners : 1;
                for (unsigned j = 0; j < corners; ++j) {
                    auto rgb = slice(body, at + (colors == 1 ? 0 : j * 4), 3); std::copy(rgb.begin(), rgb.end(), face.colors[j].begin());
                }
                indices = at + colors * 4;
            }
            Reader refs(slice(body, indices, body.size() >= indices ? body.size() - indices : 0));
            if (!unlit && !smooth) face.normals.push_back(refs.u16_le());
            for (unsigned j = 0; j < corners; ++j) { if (!unlit && smooth) face.normals.push_back(refs.u16_le()); face.vertices.push_back(refs.u16_le()); }
            for (auto v : face.vertices) require(v < nv, "Invalid TMD vertex reference");
            for (auto n : face.normals) require(n < nn, "Invalid TMD normal reference");
            if (textured) {
                for (unsigned j = 0; j < corners; ++j) { auto uv = slice(body, j * 4, 2); face.uv.push_back({uv[0], uv[1]}); }
                face.material = {u16_at(body, 6), u16_at(body, 2)};
            }
            object.faces.push_back(std::move(face));
        }
        objects.push_back(std::move(object));
    }
    require(triangles != 0, "Empty character mesh"); return objects;
}
struct Memory {
    std::vector<u16> words = std::vector<u16>(1024 * 512);
    std::vector<bool> present = std::vector<bool>(1024 * 512);
    u16 word(std::size_t at) const { require(at < present.size() && present[at], "Character texture references unloaded memory"); return words[at]; }
    std::array<u8, 4> pixel(u16 page, u16 clut, std::size_t u, std::size_t v) const {
        const auto mode = (page >> 7) & 3; require(mode <= 2, "Unsupported texture mode");
        const auto divisor = 4u >> mode, bits = 4u << mode;
        const auto x = (page & 15) * 64 + u / divisor; require(x < 1024, "Texture page exceeds image memory");
        auto value = word((((page >> 4) & 1) * 256 + v) * 1024 + x);
        if (mode != 2) {
            const auto index = (value >> ((u % divisor) * bits)) & ((1u << bits) - 1);
            value = word((clut >> 6) * 1024 + (clut & 63) * 16 + index);
        }
        std::array<u8, 4> rgba {0, 0, 0, static_cast<u8>(value ? 255 : 0)};
        for (unsigned c = 0; c < 3; ++c) { auto channel = (value >> (c * 5)) & 31; rgba[c] = (channel << 3) | (channel >> 2); }
        return rgba;
    }
};
std::size_t rtim(Bytes data, Memory *memory, bool prefix)
{
    std::size_t at = 0;
    while (at < data.size()) {
        auto header = slice(data, at, 16);
        if (std::all_of(header.begin(), header.end(), [](u8 v) { return v == 0; }) || std::all_of(header.begin(), header.end(), [](u8 v) { return v == 255; })) return at;
        if (prefix && at && !std::equal(header.begin(), header.begin() + 8, header.begin() + 8)) return at;
        for (unsigned block = 0; block < 2; ++block) {
            header = slice(data, at, 16); require(std::equal(header.begin(), header.begin() + 8, header.begin() + 8), "RTIM rectangle header copies disagree");
            const unsigned x = u16_at(header, 0), y = u16_at(header, 2), w = u16_at(header, 4), h = u16_at(header, 6);
            require(w && h && x + w <= 1024 && y + h <= 512, "RTIM rectangle exceeds texture memory");
            auto words = slice(data, at + 16, w * h * 2);
            if (memory) for (unsigned row = 0; row < h; ++row) for (unsigned col = 0; col < w; ++col) {
                const auto to = (y + row) * 1024 + x + col; memory->words[to] = u16_at(words, (row * w + col) * 2); memory->present[to] = true;
            }
            at += 16 + words.size();
        }
    }
    return at;
}
s16 scaled(s32 value, s32 height)
{
    const auto numerator = std::int64_t(value) * 2000;
    auto quotient = numerator / height, remainder = numerator % height;
    // Euclidean remainder is required for nearest-even rounding below zero.
    if (remainder < 0) { --quotient; remainder += height; }
    auto rounded = quotient + (remainder * 2 > height || (remainder * 2 == height && (quotient & 1)));
    require(rounded >= -8192 && rounded <= 8192, "Character position exceeds presentation bounds"); return rounded;
}
std::vector<u8> mesh(const std::vector<Object> &objects, const Memory &memory, u16 slot)
{
    std::map<Material, std::array<std::size_t, 5>> materials; s16 top = INT16_MAX, bottom = INT16_MIN;
    for (const auto &object : objects) {
        for (auto v : object.vertices) { top = std::min(top, v[1]); bottom = std::max(bottom, v[1]); }
        for (const auto &face : object.faces) if (face.material) {
            auto [entry, inserted] = materials.try_emplace(*face.material, std::array<std::size_t, 5>{256,256,0,0,0});
            auto &b = entry->second; for (auto uv : face.uv) {
                b[0] = std::min(b[0], std::size_t(uv[0])); b[1] = std::min(b[1], std::size_t(uv[1]));
                b[2] = std::max(b[2], std::size_t(uv[0])); b[3] = std::max(b[3], std::size_t(uv[1]));
            }
        }
    }
    require(top < bottom, "Character has no height"); std::vector<u8> atlas(1024, 255); std::size_t height = 1;
    for (auto &[material, bounds] : materials) {
        auto [x, y, right, lower, ignored] = bounds; const auto rows = lower - y + 1;
        require(height + rows <= 4096, "Character atlas exceeds limits"); bounds[4] = height;
        for (auto v = y; v <= lower; ++v) {
            for (auto u = x; u <= right; ++u) { auto rgba = memory.pixel(material.first, material.second, u, v); atlas.insert(atlas.end(), rgba.begin(), rgba.end()); }
            atlas.resize(atlas.size() + (256 - (right - x + 1)) * 4);
        }
        height += rows;
    }
    std::vector<u8> triangles;
    for (const auto &object : objects) for (const auto &face : object.faces) {
        static constexpr std::array<unsigned, 6> corners {0,1,2,1,3,2};
        for (auto corner : std::span(corners).first(face.vertices.size() == 4 ? 6 : 3)) {
            auto [x,y,z] = object.vertices[face.vertices[corner]];
            for (s32 c : {s32(x), s32(y) - bottom, s32(z)}) put16(triangles, scaled(c, s32(bottom) - top));
            const auto normal = face.unlit ? Vec3{} : object.normals[face.normals[face.normals.size() == 1 ? 0 : corner]];
            for (auto n : normal) put16(triangles, n);
            std::size_t u = 0, v = 0;
            if (face.material) { auto b = materials.at(*face.material); u = face.uv[corner][0] - b[0]; v = face.uv[corner][1] - b[1] + b[4]; }
            put16(triangles, u); put16(triangles, v);
            triangles.insert(triangles.end(), face.colors[corner].begin(), face.colors[corner].end()); triangles.push_back(face.unlit);
        }
    }
    std::vector<u8> result; put16(result, slot); put16(result, height); put32(result, triangles.size() / 60);
    result.insert(result.end(), triangles.begin(), triangles.end()); result.insert(result.end(), atlas.begin(), atlas.end()); return result;
}
}
std::vector<u8> convert(Bytes mo_bytes, Bytes texture_bytes, Bytes fdat_bytes, Bytes mof_bytes)
{
    Archive mo(mo_bytes), textures(texture_bytes), fdat(fdat_bytes), mof(mof_bytes);
    Memory common; rtim(fdat.get(84), &common, false);
    std::array<std::optional<u8>, 43> areas {};
    for (u8 area = 0; area < 28; ++area) {
        auto db = fdat.get(area * 3 + 1); require(u32_at(db, 0) == 12992, "Unexpected SLUS-00255 entity definitions"); slice(db, 4, 32 * 120);
        for (unsigned i = 0; i < 32; ++i) { auto id = db[4 + i * 120]; if (id < areas.size() && !areas[id]) areas[id] = area; }
    }
    std::vector<u8> output {'K','F','A','1',1,0,0,0}; u16 count = 0;
    auto append = [&](std::vector<u8> entry) {
        require(entry.size() <= KF_AVATAR_MAX_BYTES - output.size(), "Character pack exceeds limits");
        output.insert(output.end(), entry.begin(), entry.end()); ++count;
    };
    for (u16 slot = 0; slot < 43; ++slot) {
        require(slot < mo.offsets.size(), "Missing NPC model slot"); const auto at = mo.offsets[slot];
        if (slot && mo.offsets[slot - 1] == at) continue;
        // Dummy and unresolved runtime palette; never invent missing texture data.
        if (slot == 35 || slot == 18) continue;
        auto end = slot + 1u; while (end < mo.offsets.size() && mo.offsets[end] == at) ++end;
        require(end < mo.offsets.size(), "Empty NPC model slot"); std::optional<u8> area;
        for (unsigned i = slot; i < std::min(end, 43u); ++i) if (areas[i] && (!area || *areas[i] < *area)) area = areas[i];
        require(area.has_value(), "NPC has no map reference"); auto data = textures.get(*area); std::size_t start = 0;
        if (*area < 12) {
            start = rtim(data, nullptr, false); auto separator = slice(data, start, 64);
            require(std::all_of(separator.begin(), separator.end(), [](u8 b) { return b == 255; }), "Missing map texture separator"); start += 64;
        }
        auto memory = common; rtim(data.subspan(start), &memory, true); append(mesh(model(mo.get(slot)), memory, slot));
    }
    require(count == 38, "Unexpected KFIII character roster");
    rtim(textures.get(17), &common, true); append(mesh(model(mof.get(545)), common, 43));
    output[6] = count; output[7] = count >> 8; return output;
}
}
