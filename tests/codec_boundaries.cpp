#include "../src/audio/sound.cpp"
#include "../src/platform/assets.cpp"
#include "../src/renderer/textures.cpp"

#include <cassert>

namespace kf {
std::uint64_t host_clock_ns() { return 0; }
[[noreturn]] void host_fail(const char *) { std::abort(); }
}

int main()
{
    const std::array<u8, 4> truncated {'p', 'B', 'A', 'V'};
    assert(!kf::sound_bank_load(truncated.data(), truncated.size(), nullptr, 0));
    auto bank = std::make_unique<kf::SoundBank>();
    assert(!kf::sound_sequence_load(truncated.data(), truncated.size(), bank.get()));
    assert(!kf::sound.banks && !kf::sound.sequences);

    kf::Image image {1, 1, {1, 2, 3, 4}};
    const auto previous = image.rgba;
    const std::array<u8, 4> tim_header {0x10, 0, 0, 0};
    assert(!kf::image_decode_tim(&image, tim_header.data(), tim_header.size(), 0, 0));
    assert(image.width == 1 && image.height == 1 && image.rgba == previous);
    assert(!kf::image_decode_tim(&image, tim_header.data(), tim_header.size(), tim_header.size(), 0));
    assert(image.rgba == previous);

    kf::TextureStore textures;
    textures.words.assign(1024 * 512, 42);
    assert(!kf::texture_store_upload_tim(&textures, tim_header.data(), tim_header.size()));
    assert(std::ranges::all_of(textures.words, [](u16 word) { return word == 42; }));
    assert(!kf::texture_store_translate_tim(&textures, tim_header.data(), tim_header.size(),
        tim_header.data(), tim_header.size()));
    assert(std::ranges::all_of(textures.words, [](u16 word) { return word == 42; }));
}
