#pragma once
#include <kf/platform/assets.hpp>
#include <kf/renderer/textures.hpp>

namespace kf {
using TextureId = u32;
struct Vertex {
    float x, y, u, v;
    float r, g, b, a;
};
// Values are shared with the pixel shader's blend-mode uniform.
enum class BlendMode : u8 { opaque = 0, average = 1, add = 2, subtract = 3, add_quarter = 4 };
enum class FrameClear : u8 { Clear, Retain };
struct FrameStyle {
    FrameClear clear = FrameClear::Clear;
    BlendMode initial_blend = BlendMode::average;
    float red = 0, green = 0, blue = 0;
    int clip_x = 0, clip_y = 0, clip_width = 320, clip_height = 240;
};
enum class FaceShape : u8 { Triangle = 3, Quad = 4 };
enum class SurfaceKind : u8 { Solid, Texture };
enum class FaceTransparency : u8 { Opaque, Blend };
enum class FaceShading : u8 { Flat, Gouraud };
enum class TextureColorMode : u8 { Modulated, Raw };
struct FaceMaterial {
    SurfaceKind kind;
    TextureSource source;
    BlendMode blend;
    TextureColorMode color_mode = TextureColorMode::Modulated;
};
struct DrawFace {
    Vertex vertices[4];
    FaceShape shape;
    FaceMaterial material;
    FaceTransparency transparency;
    s32 depth;
    FaceShading shading = FaceShading::Flat;
};
struct FaceList {
    const DrawFace *faces;
    std::size_t count;
    FrameStyle style{};
};
struct Renderer {
    u32 program, vao, buffer, framebuffer, color_texture, blend_texture;
    TextureId white_texture;
    TextureStore textures;
};
bool renderer_init(Renderer *renderer, char *error, std::size_t error_size);
void renderer_release(Renderer *renderer);
TextureId renderer_upload(const Image *image);
void renderer_delete_texture(TextureId texture);
void renderer_present_retained(const Renderer *renderer, int width, int height);
bool renderer_draw_faces(Renderer *renderer, const FaceList *faces, int width, int height);
} // namespace kf
