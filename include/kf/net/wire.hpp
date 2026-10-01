#pragma once
#include <kf/lib/types.h>
#include <kf/lib/enum.h>
#include <span>
#include <type_traits>

namespace kf::net {
// Fixed-width little-endian fields. No struct layout, pointers, allocator state,
// or native padding crosses this boundary.
class WireCodec {
public:
    explicit WireCodec(std::span<u8> output) : writable(output), input(output) {}
    explicit WireCodec(std::span<const u8> bytes) : input(bytes), reading(true) {}
    bool valid() const { return good; }
    bool complete() const { return good && position == input.size(); }
    bool is_reading() const { return reading; }
    std::size_t size() const { return position; }
    void reject() { good = false; }

    template<class T> requires std::is_integral_v<T>
    void value(T &field) {
        static_assert(sizeof(T) <= 8);
        if (!good || sizeof(T) > input.size() - position) { good = false; return; }
        std::uint64_t bits = reading ? 0 : static_cast<std::uint64_t>(field);
        for (unsigned byte = 0; byte < sizeof(T); ++byte) {
            if (reading) bits |= static_cast<std::uint64_t>(input[position++]) << (8 * byte);
            else writable[position++] = static_cast<u8>(bits >> (8 * byte));
        }
        if (reading) {
            if constexpr (std::is_same_v<T, bool>) {
                if (bits > 1) { good = false; return; }
            }
            field = static_cast<T>(bits);
        }
    }
    template<class T> requires std::is_enum_v<T>
    void value(T &field) {
        auto bits = static_cast<std::underlying_type_t<T>>(field);
        value(bits);
        if (reading) field = static_cast<T>(bits);
    }
    template<class Enum, class Storage>
    void value(KfEnumStorage<Enum, Storage> &field) {
        auto bits = field.encoded_value();
        value(bits);
        if (reading) field = static_cast<Enum>(bits);
    }
    template<class T, std::size_t N>
    void value(T (&fields)[N]) { for (auto &field : fields) value(field); }
private:
    std::span<u8> writable;
    std::span<const u8> input;
    std::size_t position {};
    bool reading {};
    bool good = true;
};
}
