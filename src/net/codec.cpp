#include <kf/net/codec.h>
#include <kf/lib/avatar.h>
#include <kf/lib/byte_reader.h>

#include <algorithm>
#include <bit>
#include <limits>

namespace {
using kf::codec::Reader;
using kf::codec::require;
using kf::codec::output_fits;
template<class T> bool aligned(const T *value)
{ return value && reinterpret_cast<std::uintptr_t>(value) % alignof(T) == 0; }

// Untrusted traffic is rejected quietly; a peer cannot amplify it into log I/O.
template<class Function> KfCodecResult checked(Function function)
{
    try { function(); return KF_CODEC_OK; }
    catch (const kf::codec::Error &error) { return error.result; }
}

class Writer {
public:
    Writer() = default;
    explicit Writer(std::span<u8> bytes) : bytes_(bytes), counting_(false) {}
    void put(std::span<const u8> bytes) {
        if (!counting_) {
            output_fits(bytes.size() <= bytes_.size() - at_);
            std::copy(bytes.begin(), bytes.end(), bytes_.begin() + at_);
        }
        at_ += bytes.size();
    }
    void byte(u8 value) { put({&value, 1}); }
    void u16_le(u16 value) { byte(value); byte(value >> 8); }
    void u32_le(u32 value) { u16_le(value); u16_le(value >> 16); }
    std::size_t size() const { return at_; }
private:
    std::span<u8> bytes_;
    std::size_t at_ = 0;
    bool counting_ = true;
};

void valid(const KfNetHeader &h)
{ require(h.kind >= 1 && h.kind <= 11 && h.epoch != 0, "invalid packet header"); }
KfNetHeader header(Reader &r)
{
    require(r.u32_le() == 0x314e464b && r.u16_le() == KF_NET_PROTOCOL_VERSION, "protocol mismatch");
    KfNetHeader h {r.byte(), r.u32_le(), r.u32_le(), r.u32_le()};
    valid(h);
    return h;
}
void header(Writer &w, const KfNetHeader &h)
{
    valid(h);
    w.u32_le(0x314e464b); w.u16_le(KF_NET_PROTOCOL_VERSION); w.byte(h.kind);
    w.u32_le(h.epoch); w.u32_le(h.sequence); w.u32_le(h.generation);
}
void valid(const KfNetInputBundle &b)
{
    valid(b.header);
    require(b.header.kind == 1 && b.count > 0 && b.count <= KF_NET_INPUT_REDUNDANCY, "invalid input count");
    u32 previous = 0;
    for (const auto &f : std::span(b.frames).first(b.count)) {
        require(f.sequence > previous && !(f.buttons & ~0xf9ffu) && f.yaw >= -1024 && f.yaw <= 1024 &&
            f.pitch >= -1024 && f.pitch <= 1024, "invalid player input");
        previous = f.sequence;
    }
    require(previous == b.header.sequence, "input sequence mismatch");
}
void valid(const KfNetCommand &c)
{
    valid(c.header);
    require(c.header.kind == 4 && c.header.sequence && c.kind >= 1 && c.kind <= 14, "invalid command");
    if (c.kind == 14) require(c.object >= 1 && c.object <= 2 && c.argument == 0, "invalid command option");
    if (c.kind == 13) require(c.object < KF_AVATAR_SLOTS && c.argument == 0, "invalid avatar");
    if (c.kind == 12) require(c.target_generation && c.object < 190 && (c.argument & 255) < 4 &&
        (c.argument >> 8) < 80, "invalid loot target");
    else require(c.target_generation == 0, "unexpected target generation");
}
void valid(const KfNetInteraction &v)
{
    valid(v.header);
    require(v.header.kind == 11 && v.header.sequence && v.floor >= 1 && v.floor <= 5 && v.stage <= 5 &&
        v.character <= 99 && v.page <= 9 && (v.stage == 0) == (v.page == 0) && v.shop <= 2, "invalid interaction");
}
template<class T, class Function>
KfCodecResult decode(const u8 *data, std::size_t size, T *out, Function function, bool complete = true)
{
    if (!data || !aligned(out) || size > KF_NET_PACKET_LIMIT) return KF_CODEC_INVALID;
    return checked([&] {
        Reader r({data, size});
        const T value = function(r);
        require(!complete || r.remaining() == 0, "trailing packet bytes");
        *out = value;
    });
}
template<class T, class Function>
KfCodecResult encode(const T *input, u8 *data, std::size_t capacity, std::size_t *written, Function function)
{
    if (!aligned(input) || !data || !aligned(written) || capacity > PTRDIFF_MAX) return KF_CODEC_INVALID;
    return checked([&] {
        valid(*input);
        Writer count; function(count, *input);
        output_fits(count.size() <= std::min<std::size_t>(capacity, KF_NET_PACKET_LIMIT));
        Writer w({data, capacity});
        function(w, *input);
        *written = w.size();
    });
}
}

KfCodecResult kf_net_header_decode(const u8 *data, std::size_t size, KfNetHeader *out)
{ return decode(data, size, out, [](Reader &r) { return header(r); }, false); }
KfCodecResult kf_net_header_encode(const KfNetHeader *input, u8 *data, std::size_t capacity, std::size_t *written)
{ return encode(input, data, capacity, written, [](Writer &w, const auto &h) { header(w, h); }); }
KfCodecResult kf_net_input_decode(const u8 *data, std::size_t size, KfNetInputBundle *out)
{
    return decode(data, size, out, [](Reader &r) {
        KfNetInputBundle b {}; b.header = header(r); b.count = r.byte();
        require(b.count > 0 && b.count <= KF_NET_INPUT_REDUNDANCY, "invalid input count");
        for (auto &f : std::span(b.frames).first(b.count))
            f = {r.u32_le(), r.u32_le(), r.u32_le(), std::bit_cast<s32>(r.u32_le()), std::bit_cast<s32>(r.u32_le())};
        valid(b); return b;
    });
}
KfCodecResult kf_net_input_encode(const KfNetInputBundle *input, u8 *data, std::size_t capacity, std::size_t *written)
{
    return encode(input, data, capacity, written, [](Writer &w, const auto &b) {
        header(w, b.header); w.byte(b.count);
        for (const auto &f : std::span(b.frames).first(b.count)) {
            w.u32_le(f.sequence); w.u32_le(f.tick); w.u32_le(f.buttons); w.u32_le(f.yaw); w.u32_le(f.pitch);
        }
    });
}
KfCodecResult kf_net_command_decode(const u8 *data, std::size_t size, KfNetCommand *out)
{
    return decode(data, size, out, [](Reader &r) {
        KfNetCommand c {header(r), r.byte(), r.u16_le(), r.u16_le(), r.u32_le()}; valid(c); return c;
    });
}
KfCodecResult kf_net_command_encode(const KfNetCommand *input, u8 *data, std::size_t capacity, std::size_t *written)
{
    return encode(input, data, capacity, written, [](Writer &w, const auto &c) {
        header(w, c.header); w.byte(c.kind); w.u16_le(c.object); w.u16_le(c.argument); w.u32_le(c.target_generation);
    });
}
KfCodecResult kf_net_interaction_decode(const u8 *data, std::size_t size, KfNetInteraction *out)
{
    return decode(data, size, out, [](Reader &r) {
        KfNetInteraction v {header(r), r.byte(), r.byte(), r.byte(), r.byte(), r.byte()}; valid(v); return v;
    });
}
KfCodecResult kf_net_interaction_encode(const KfNetInteraction *input, u8 *data, std::size_t capacity, std::size_t *written)
{
    return encode(input, data, capacity, written, [](Writer &w, const auto &v) {
        header(w, v.header); w.byte(v.floor); w.byte(v.stage); w.byte(v.character); w.byte(v.page); w.byte(v.shop);
    });
}
KfCodecResult kf_net_fragment_decode(const u8 *data, std::size_t size, KfNetFragment *out)
{
    return decode(data, size, out, [](Reader &r) {
        KfNetFragment f {}; f.header = header(r); f.total = r.u32_le(); f.offset = r.u32_le();
        f.payload_offset = r.position(); f.payload_size = r.remaining();
        require(f.header.kind == 3 && f.total > 0 && f.total <= KF_NET_TRANSFER_LIMIT && f.offset < f.total &&
            f.payload_size > 0 && f.payload_size <= KF_NET_FRAGMENT_BYTES && f.payload_size <= f.total - f.offset,
            "invalid fragment bounds");
        return f;
    }, false);
}
KfCodecResult kf_net_fragment_encode(const KfNetHeader *h, const u8 *payload, std::size_t size,
    u32 offset, u8 *data, std::size_t capacity, std::size_t *written)
{
    if (!payload || !aligned(h) || h->kind != 3 || size == 0 || size > KF_NET_TRANSFER_LIMIT || offset >= size)
        return KF_CODEC_INVALID;
    return encode(h, data, capacity, written, [&](Writer &w, const auto &value) {
        header(w, value); w.u32_le(size); w.u32_le(offset);
        w.put({payload + offset, std::min<std::size_t>(KF_NET_FRAGMENT_BYTES, size - offset)});
    });
}
KfCodecResult kf_net_input_push(KfNetInputInbox *inbox, const KfNetInputBundle *input)
{
    if (!aligned(inbox) || !aligned(input)) return KF_CODEC_INVALID;
    return checked([&] {
        valid(*input);
        require(inbox->begin < KF_NET_INPUT_QUEUE && inbox->count <= KF_NET_INPUT_QUEUE, "invalid input queue");
        u32 newest = inbox->received; unsigned fresh = 0;
        for (const auto &f : std::span(input->frames).first(input->count)) {
            if (f.sequence <= inbox->received) continue;
            require(f.sequence - newest <= 1024, "input sequence jumped");
            newest = f.sequence; ++fresh;
        }
        output_fits(fresh <= KF_NET_INPUT_QUEUE - inbox->count);
        for (const auto &f : std::span(input->frames).first(input->count)) {
            if (f.sequence <= inbox->received) continue;
            inbox->pending[(inbox->begin + inbox->count) % KF_NET_INPUT_QUEUE] = f;
            ++inbox->count; inbox->received = f.sequence;
        }
    });
}
KfCodecResult kf_net_input_pop(KfNetInputInbox *inbox, KfNetInputFrame *out)
{
    if (!aligned(inbox) || !aligned(out) || inbox->begin >= KF_NET_INPUT_QUEUE || inbox->count > KF_NET_INPUT_QUEUE) return KF_CODEC_INVALID;
    if (!inbox->count) return KF_CODEC_END;
    *out = inbox->pending[inbox->begin];
    inbox->begin = (inbox->begin + 1) % KF_NET_INPUT_QUEUE;
    --inbox->count; inbox->consumed = out->sequence;
    return KF_CODEC_OK;
}
KfCodecResult kf_net_fragment_accept(KfNetTransfer *transfer, const u8 *data, std::size_t size,
    u8 *output, std::size_t capacity)
{
    if (!aligned(transfer)) return KF_CODEC_INVALID;
    KfNetFragment f {};
    auto result = kf_net_fragment_decode(data, size, &f);
    if (result == KF_CODEC_OK) result = checked([&] {
        require(output && capacity <= PTRDIFF_MAX, "invalid transfer output");
        output_fits(capacity >= f.total);
        if (!f.offset) *transfer = {f.header.epoch, f.header.sequence, f.header.generation, f.total, 0};
        require(transfer->epoch == f.header.epoch && transfer->sequence == f.header.sequence &&
            transfer->generation == f.header.generation && transfer->total == f.total && transfer->received == f.offset,
            "out of order fragment");
        std::copy_n(data + f.payload_offset, f.payload_size, output + f.offset);
        transfer->received = f.offset + f.payload_size;
    });
    if (result != KF_CODEC_OK) *transfer = {};
    return result;
}
