#include "../src/lib/tmd.cpp"
#include "../src/lib/render_face.cpp"
#include "../src/renderer/lighting.cpp"
#include "../src/cutscene/render_tmd.cpp"
#include "../src/cutscene/render_map.cpp"
#include "../src/cutscene/render_unlit.cpp"
#include "../src/cutscene/opening_entity_pool.cpp"
#include <cassert>
#include <cstring>
#include <vector>
#include <string_view>

KfGraphicsRuntimeOpen open_graphics_runtime {};
KfMapGrid cutscene_map_floor_height_grid {};
static unsigned submitted;
namespace kf {
[[noreturn]] void host_fail(const char *message)
{
    std::fprintf(stderr, "%s\n", message);
    std::exit(77);
}
void host_enqueue_face(const DrawFace &) { ++submitted; }
}

// Supply a visible projected quad so all packet indices and normals are consumed.
void cutscene_tmd_project_vertices(s32, const MATRIX *, const kf::Projection &)
{
    auto &vertices = open_graphics_runtime.tmd_projected_vertices;
    vertices[0] = {{0, 0}, 800, 0};
    vertices[1] = {{100, 0}, 800, 0};
    vertices[2] = {{0, 100}, 800, 0};
    vertices[3] = {{100, 100}, 800, 0};
}

static void word(std::vector<u8> &bytes, std::size_t offset, u32 value)
{
    for (unsigned i = 0; i < 4; ++i)
        bytes[offset + i] = value >> (8 * i);
}

int main(int argc, char **argv)
{
    assert(argc == 3);
    const std::string_view consumer = argv[1], scenario = argv[2];
    if (consumer == "placements") {
        KfMapObjectPlacement entries[2] {};
        entries[1].object_id = 0xff;
        std::size_t size = sizeof entries;
        if (scenario == "truncated") size = sizeof(entries[0]) - 1;
        if (scenario == "unterminated") size = sizeof(entries[0]);
        if (scenario == "outside-grid") entries[0].tile_x = KF_MAP_COLUMNS;
        opening_entity_pool_load_placements({reinterpret_cast<const u8 *>(entries), size}, 0);
        assert(opening_entity_state.entities[0].object_id == static_cast<KfOpeningModelId>(0));
        assert(opening_entity_state.entities[1].object_id == KF_OPENING_ENTITY_FREE);
        return 0;
    }
    const MATRIX lights {};
    for (unsigned mode = 0x20; mode <= 0x3e; mode += 2) {
        const bool textured = mode & 4, gouraud = mode & 16;
        const unsigned corners = mode & 8 ? 4 : 3;
        const unsigned indices = textured ? corners * 4 : 4;
        const unsigned body_size = (indices + (gouraud ? corners * 4 : (corners + 1) * 2) + 3) & ~3u;
        constexpr unsigned normal_offset = 12 + 28, packet_offset = normal_offset + 8;
        std::vector<u8> bytes(packet_offset + 4 + body_size);
        word(bytes, 8, 1);
        word(bytes, 16, 4);
        word(bytes, 20, normal_offset - 12);
        word(bytes, 24, 1);
        word(bytes, 28, packet_offset - 12);
        word(bytes, 32, 1);
        word(bytes, packet_offset, (mode << 24) | (body_size / 4 << 8));
        for (unsigned i = 0; i < corners; ++i)
            bytes[packet_offset + 4 + indices + (gouraud ? i * 4 + 2 : i * 2 + 2)] = i;
        if (scenario == "truncated") bytes.pop_back();
        if (scenario == "short-layout") bytes[packet_offset + 1] = 0;
        if (scenario == "vertex") bytes[packet_offset + 4 + indices + 2] = 4;
        if (scenario == "normal") bytes[packet_offset + 4 + indices] = 1;
        open_graphics_runtime.tmd_state.current_tmd = tmd_resource_view(bytes.data(), bytes.size());
        cutscene_tmd_project_vertices(4, &lights, {});
        submitted = 0;
        if (consumer == "map") cutscene_render_enqueue_map(0, &lights, &lights, {});
        else if (consumer == "unlit") render_enqueue_unlit_triangles(0, 0);
        else cutscene_render_enqueue_tmd(0, 0, &lights);
        const bool supported = consumer == "map" ? (mode == 0x24 || mode == 0x2c)
            : consumer == "unlit" ? (mode == 0x20 || mode == 0x24)
            : !(textured && (mode & 2));
        assert(submitted == static_cast<unsigned>(supported));
    }
}
