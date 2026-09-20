#include <kf/renderer/renderer.hpp>
#include <GLES3/gl3.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <limits>

namespace kf {
namespace {
constexpr int display_aspect_width = 4;
constexpr int display_aspect_height = 3;
// Uniform encoding shared with the shader-local names below.
constexpr GLint shader_texture_solid = 0;
constexpr GLint shader_texture_modulated = 1;
constexpr GLint shader_texture_raw = 2;
}

// Triangles are backend-private: original producers submit whole faces, which
// are sorted before this representation is constructed.
struct DrawTriangle {
    Vertex vertices[3];
    u32 material;
    BlendMode blend;
    SurfaceKind surface;
    TextureColorMode color_mode;
    bool dither;
};
struct DrawList {
    const DrawTriangle *triangles;
    std::size_t count;
    const TextureId *textures;
    std::size_t texture_count;
    FrameStyle style{};
};
static GLuint shader(GLenum type, const char *source, char *error, std::size_t size) {
    const auto object = glCreateShader(type);
    glShaderSource(object, 1, &source, nullptr);
    glCompileShader(object);
    GLint good = 0;
    glGetShaderiv(object, GL_COMPILE_STATUS, &good);
    if (!good) {
        glGetShaderInfoLog(object, static_cast<GLsizei>(size), nullptr, error);
        glDeleteShader(object);
        return 0;
    }
    return object;
}
bool renderer_init(Renderer *renderer, char *error, std::size_t size) {
    const auto vertex = shader(GL_VERTEX_SHADER, R"(#version 300 es
layout(location=0) in vec2 position;
layout(location=1) in vec2 texcoord;
layout(location=2) in vec4 color;
uniform int texture_mode;
out vec2 uv; out vec4 tint;
const int texture_solid=0;
const float rgb8_max=255.0;
const float modulation_unity=128.0;
void main() {
    gl_Position=vec4(position.x/160.0-1.0,1.0-position.y/120.0,0.0,1.0);
    uv=texcoord;
    tint=vec4(floor(color.rgb*(texture_mode==texture_solid ? rgb8_max : modulation_unity)+0.5),color.a);
}
)",
                               error, size);
    if (!vertex)
        return false;
    const auto fragment = shader(GL_FRAGMENT_SHADER, R"(#version 300 es
precision highp float;
precision highp int;
uniform sampler2D image;
uniform sampler2D backdrop;
uniform int texture_mode;
uniform int dither_enabled;
uniform int blend_mode;
in vec2 uv; in vec4 tint;
out vec4 pixel;
// BlendMode and the texture-mode uniform use these integer encodings.
const int texture_solid=0, texture_modulated=1, texture_raw=2;
const int blend_opaque=0, blend_average=1, blend_add=2,
    blend_subtract=3, blend_add_quarter=4;
const float rgb8_max=255.0;
const int rgb5_max=31, rgb_expansion_shift=3, rgb_replication_shift=2;
const int modulation_shift=4;
const int dither_period=4, dither_mask=dither_period-1;
const int last_framebuffer_row=239;
// Texture uploads mark STP texels with alpha 128 and opaque texels with 255.
const float stp_alpha_threshold=0.75;
const int dither_offsets[dither_period*dither_period]=int[dither_period*dither_period](
    -4,0,-3,1, 2,-2,3,-1, -3,1,-4,0, 3,-1,2,-2);
void main() {
    vec4 sample_color=texture(image,uv);
    if(sample_color.a==0.0) discard;
    ivec3 shade=ivec3(clamp(floor(tint.rgb),0.0,rgb8_max));
    ivec3 texel=ivec3(floor(sample_color.rgb*float(rgb5_max)+0.5));
    ivec3 foreground=shade;
    if(texture_mode==texture_modulated) foreground=(texel*shade)>>modulation_shift;
    if(texture_mode==texture_raw) foreground=texel<<rgb_expansion_shift;
    ivec2 destination=ivec2(gl_FragCoord.xy);
    int offset=0;
    if(dither_enabled!=0)
        offset=dither_offsets[((last_framebuffer_row-destination.y)&dither_mask)*dither_period+(destination.x&dither_mask)];
    foreground=clamp((foreground+ivec3(offset))>>rgb_expansion_shift,ivec3(0),ivec3(rgb5_max));
    // A textured face blends only its STP texels. Black texels with STP are
    // visible; only a zero texture word was discarded above.
    if(blend_mode!=blend_opaque && (texture_mode==texture_solid || sample_color.a<stp_alpha_threshold)) {
        ivec3 background=ivec3(floor(texelFetch(backdrop,destination,0).rgb*float(rgb5_max)+0.5));
        if(blend_mode==blend_average) foreground=(background+foreground)>>1;
        if(blend_mode==blend_add) foreground=background+foreground;
        if(blend_mode==blend_subtract) foreground=background-foreground;
        if(blend_mode==blend_add_quarter) foreground=background+(foreground>>2);
        foreground=clamp(foreground,ivec3(0),ivec3(rgb5_max));
    }
    ivec3 expanded=(foreground<<rgb_expansion_shift)|(foreground>>rgb_replication_shift);
    pixel=vec4(vec3(expanded)/rgb8_max,1.0);
}
)",
                                 error, size);
    if (!fragment) {
        glDeleteShader(vertex);
        return false;
    }
    renderer->program = glCreateProgram();
    glAttachShader(renderer->program, vertex);
    glAttachShader(renderer->program, fragment);
    glLinkProgram(renderer->program);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    GLint good = 0;
    glGetProgramiv(renderer->program, GL_LINK_STATUS, &good);
    if (!good) {
        glGetProgramInfoLog(renderer->program, static_cast<GLsizei>(size), nullptr, error);
        renderer_release(renderer);
        return false;
    }
    glGenVertexArrays(1, &renderer->vao);
    glBindVertexArray(renderer->vao);
    glGenBuffers(1, &renderer->buffer);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, u)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, r)));
    glGenTextures(1, &renderer->color_texture);
    glBindTexture(GL_TEXTURE_2D, renderer->color_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, render_width, render_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenFramebuffers(1, &renderer->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, renderer->framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           renderer->color_texture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::snprintf(error, size, "Cannot create 320x240 render target.");
        renderer_release(renderer);
        return false;
    }
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glGenTextures(1, &renderer->blend_texture);
    glBindTexture(GL_TEXTURE_2D, renderer->blend_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, render_width, render_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    u8 white[] = {color8_max, color8_max, color8_max, color8_max};
    const Image solid{1, 1, {white, sizeof white, sizeof white}};
    renderer->white_texture = renderer_upload(&solid);
    if (!renderer->white_texture) {
        std::snprintf(error, size, "Cannot create solid-color material.");
        renderer_release(renderer);
        return false;
    }
    glUseProgram(renderer->program);
    glUniform1i(glGetUniformLocation(renderer->program, "image"), 0);
    glUniform1i(glGetUniformLocation(renderer->program, "backdrop"), 1);
    return glGetError() == GL_NO_ERROR;
}
void renderer_release(Renderer *renderer) {
    texture_store_release(&renderer->textures);
    renderer_delete_texture(renderer->white_texture);
    glDeleteProgram(renderer->program);
    glDeleteBuffers(1, &renderer->buffer);
    glDeleteVertexArrays(1, &renderer->vao);
    glDeleteTextures(1, &renderer->color_texture);
    glDeleteTextures(1, &renderer->blend_texture);
    glDeleteFramebuffers(1, &renderer->framebuffer);
    *renderer = {};
}
TextureId renderer_upload(const Image *image) {
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(image->width),
                 static_cast<GLsizei>(image->height), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 image->rgba.data);
    if (glGetError() != GL_NO_ERROR) {
        glDeleteTextures(1, &texture);
        return 0;
    }
    return texture;
}
void renderer_delete_texture(TextureId texture) {
    glDeleteTextures(1, &texture);
}
static float clear_channel(float value) {
    const auto channel = static_cast<u32>(std::clamp(std::round(value * color8_max), 0.0f, color8_scale)) >> color5_color8_shift;
    return static_cast<float>((channel << color5_color8_shift) | (channel >> color5_replication_shift)) / color8_scale;
}
static void renderer_draw(const Renderer *renderer, const DrawList *draws, int width, int height) {
    glBindFramebuffer(GL_FRAMEBUFFER, renderer->framebuffer);
    glViewport(0, 0, render_width, render_height);
    const auto &style = draws->style;
    const auto left = std::clamp<std::int64_t>(style.clip_x, 0, render_width);
    const auto top = std::clamp<std::int64_t>(style.clip_y, 0, render_height);
    const auto right = std::clamp<std::int64_t>(std::int64_t(style.clip_x) + style.clip_width, left, render_width);
    const auto bottom = std::clamp<std::int64_t>(std::int64_t(style.clip_y) + style.clip_height, top, render_height);
    glEnable(GL_SCISSOR_TEST);
    glScissor(left, render_height - bottom, right - left, bottom - top);
    glDisable(GL_DITHER);
    glDisable(GL_BLEND);
    if (style.clear == FrameClear::Clear) {
        glClearColor(clear_channel(style.red), clear_channel(style.green), clear_channel(style.blue), 1);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glUseProgram(renderer->program);
    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->buffer);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, renderer->blend_texture);
    const auto texture_mode = glGetUniformLocation(renderer->program, "texture_mode");
    const auto dither_enabled = glGetUniformLocation(renderer->program, "dither_enabled");
    const auto blend_mode = glGetUniformLocation(renderer->program, "blend_mode");
    for (std::size_t i = 0; i < draws->count; ++i) {
        const auto *draw = &draws->triangles[i];
        if (draw->material >= draws->texture_count || !draws->textures[draw->material])
            continue;
        if (draw->blend != BlendMode::opaque) {
            const auto &a = draw->vertices[0];
            const auto &b = draw->vertices[1];
            const auto &c = draw->vertices[2];
            const auto x0 = static_cast<GLint>(std::clamp(std::floor(std::min({a.x, b.x, c.x})),
                static_cast<float>(left), static_cast<float>(right)));
            const auto x1 = static_cast<GLint>(std::clamp(std::ceil(std::max({a.x, b.x, c.x})),
                static_cast<float>(left), static_cast<float>(right)));
            const auto y0 = static_cast<GLint>(std::clamp(std::floor(std::min({a.y, b.y, c.y})),
                static_cast<float>(top), static_cast<float>(bottom)));
            const auto y1 = static_cast<GLint>(std::clamp(std::ceil(std::max({a.y, b.y, c.y})),
                static_cast<float>(top), static_cast<float>(bottom)));
            if (x1 <= x0 || y1 <= y0)
                continue;
            // Snapshot only this triangle's clipped bounds. Reading the attached
            // color target directly in a shader is undefined on GLES/WebGL.
            glActiveTexture(GL_TEXTURE1);
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, x0, render_height - y1, x0, render_height - y1, x1 - x0, y1 - y0);
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, draws->textures[draw->material]);
        glBufferData(GL_ARRAY_BUFFER, sizeof draw->vertices, draw->vertices, GL_STREAM_DRAW);
        glUniform1i(texture_mode, draw->surface == SurfaceKind::Solid ? shader_texture_solid :
            draw->color_mode == TextureColorMode::Modulated ? shader_texture_modulated : shader_texture_raw);
        glUniform1i(dither_enabled, draw->dither ? 1 : 0);
        glUniform1i(blend_mode, static_cast<GLint>(draw->blend));
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
    renderer_present_retained(renderer, width, height);
}

void renderer_present_retained(const Renderer *renderer, int width, int height) {
    if (width <= 0 || height <= 0)
        return;
    glDisable(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    int target_width = width, target_height = width * display_aspect_height / display_aspect_width;
    if (target_height > height) {
        target_height = height;
        target_width = height * display_aspect_width / display_aspect_height;
    }
    const int x = (width - target_width) / 2, y = (height - target_height) / 2;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer->framebuffer);
    glBlitFramebuffer(0, 0, render_width, render_height, x, y, x + target_width, y + target_height,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

struct FaceOrder {
    std::size_t index;
    s32 depth;
};
static int compare_faces(const void *lhs, const void *rhs) {
    const auto &a = *static_cast<const FaceOrder *>(lhs);
    const auto &b = *static_cast<const FaceOrder *>(rhs);
    if (a.depth != b.depth)
        return a.depth > b.depth ? -1 : 1;
    // Original equal-depth insertion was LIFO. Order whole faces before splitting
    // quads, so another face can never be interleaved between their triangles.
    return a.index == b.index ? 0 : a.index > b.index ? -1 : 1;
}

static unsigned append_face_triangle(DrawTriangle *triangles, const DrawFace &face,
    unsigned a, unsigned b, unsigned c, u32 material, BlendMode blend) {
    const auto &va = face.vertices[a];
    const auto &vb = face.vertices[b];
    const auto &vc = face.vertices[c];
    const float width = std::max({va.x, vb.x, vc.x}) - std::min({va.x, vb.x, vc.x});
    const float height = std::max({va.y, vb.y, vc.y}) - std::min({va.y, vb.y, vc.y});
    // Preserve the original rasterizer's per-triangle size rejection before
    // viewport clipping. Saturated behind-camera faces can otherwise fill the
    // view. Check each quad half independently without changing face order.
    constexpr float maximum_triangle_width = 1023;
    constexpr float maximum_triangle_height = 511;
    if (width > maximum_triangle_width || height > maximum_triangle_height)
        return 0;
    const bool dither = face.material.kind == SurfaceKind::Texture
        ? face.material.color_mode == TextureColorMode::Modulated : face.shading == FaceShading::Gouraud;
    triangles[0] = {{va, vb, vc}, material, blend, face.material.kind, face.material.color_mode, dither};
    return 1;
}

bool renderer_draw_faces(Renderer *renderer, const FaceList *faces, int width, int height) {
    const auto count = faces->count;
    if ((count && !faces->faces) || count > std::numeric_limits<u32>::max() ||
        count > std::numeric_limits<std::size_t>::max() / 2 / sizeof(DrawTriangle))
        return false;
    auto *order = static_cast<FaceOrder *>(std::malloc(count * sizeof(FaceOrder)));
    auto *triangles = static_cast<DrawTriangle *>(std::malloc(count * 2 * sizeof(DrawTriangle)));
    auto *textures = static_cast<TextureId *>(std::malloc(count * sizeof(TextureId)));
    bool good = !count || (order && triangles && textures);
    std::size_t triangle_count = 0;
    // The frame starts with an explicit blend policy; previous-frame texture
    // selection must not change a leading untextured translucent face.
    auto preceding_blend = faces->style.initial_blend;
    if (good && count) {
        for (std::size_t i = 0; i < count; ++i)
            order[i] = {i, faces->faces[i].depth};
        std::qsort(order, count, sizeof(FaceOrder), compare_faces);
        for (std::size_t i = 0; i < count; ++i) {
            const auto &face = faces->faces[order[i].index];
            if (face.shape != FaceShape::Triangle && face.shape != FaceShape::Quad) {
                good = false;
                break;
            }
            const bool textured = face.material.kind == SurfaceKind::Texture;
            if (textured)
                preceding_blend = face.material.blend;
            textures[i] = textured ? texture_store_resolve(&renderer->textures, face.material.source)
                                   : renderer->white_texture;
            if (!textures[i]) {
                good = false;
                break;
            }
            const auto blend = face.transparency == FaceTransparency::Blend ? preceding_blend : BlendMode::opaque;
            // Sort original whole faces first. All output from each face
            // stays adjacent, preserving equal-depth LIFO and blending order.
            triangle_count += append_face_triangle(triangles + triangle_count, face,
                0, 1, 2, static_cast<u32>(i), blend);
            if (face.shape == FaceShape::Quad)
                triangle_count += append_face_triangle(triangles + triangle_count, face,
                    1, 3, 2, static_cast<u32>(i), blend);
        }
    }
    if (good) {
        const DrawList draws{triangles, triangle_count, textures, count, faces->style};
        renderer_draw(renderer, &draws, width, height);
    }
    std::free(order);
    std::free(triangles);
    std::free(textures);
    return good;
}
} // namespace kf
