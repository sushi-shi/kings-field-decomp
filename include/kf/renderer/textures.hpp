#pragma once
#include <kf/platform/assets.hpp>

namespace kf {
enum class TextureFormat : u8 { Indexed4, Indexed8, Direct16 };
struct TextureSource {
    u16 x, y, palette_x, palette_y;
    TextureFormat format;
};
struct TextureEntry {
    TextureSource source;
    u32 texture;
    bool dirty;
};
struct TextureStore {
    // Authored TIM texels and palettes share this coordinate space. It is not a
    // framebuffer, command memory, or an address space exposed to gameplay.
    u16 *words;
    TextureEntry *entries;
    std::size_t count, capacity;
};
bool texture_store_upload_tim(TextureStore *store, const u8 *bytes, std::size_t size);
bool texture_decode(Image *image, TextureSource source, const u16 *words, std::size_t count);
u32 texture_store_resolve(TextureStore *store, TextureSource source);
void texture_store_release(TextureStore *store);
}
