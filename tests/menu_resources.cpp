#include "../src/game/item.cpp"

#include <cassert>
#include <stdexcept>

namespace kf {
[[noreturn]] void host_fail(const char *message)
{
    throw std::runtime_error(message);
}
}

static void rejects(auto operation, const char *message)
{
    try {
        operation();
        assert(false);
    } catch (const std::runtime_error &error) {
        assert(std::strcmp(error.what(), message) == 0);
    }
}

int main()
{
    // Deliberately unaligned input, signed positions, and distinct material words.
    std::array<u8, 41> storage{};
    auto packet = std::span(storage).subspan(1);
    packet[3] = 9;
    packet[4] = 64;
    packet[5] = 32;
    packet[6] = 16;
    packet[7] = 0x2e;
    for (std::size_t corner = 0; corner < 4; ++corner) {
        const auto at = 8 + corner * 8;
        packet[at] = 0xfe;
        packet[at + 1] = 0xff;
        packet[at + 2] = 0x00;
        packet[at + 3] = 0x80;
        packet[at + 4] = 16;
        packet[at + 5] = 32;
    }
    packet[14] = 0x43;
    packet[22] = 0x21;
    kf::codec::Reader input(packet);
    auto face = menu_data_face(input, MenuTemplateKind::Textured);
    assert(input.remaining() == 0);
    assert(face.material.source.x == 64 && face.material.source.y == 0);
    assert(face.material.source.palette_x == 48 && face.material.source.palette_y == 1);
    assert(face.material.blend == kf::BlendMode::add);
    assert(face.transparency == kf::FaceTransparency::Blend);
    for (const auto &vertex : face.vertices) {
        assert(vertex.x == -2 && vertex.y == -32768);
        assert(vertex.u == 16 / kf::texture_uv_scale && vertex.v == 32 / kf::texture_uv_scale);
        assert(vertex.r == 64 / kf::texture_color_unity);
    }
    packet[7] = 0x2d;
    kf::codec::Reader raw_input(packet);
    face = menu_data_face(raw_input, MenuTemplateKind::Textured);
    assert(face.vertices[0].r == 1 && face.vertices[0].g == 1 && face.vertices[0].b == 1);
    packet[3] = 8;
    rejects([&] {
        kf::codec::Reader invalid(packet);
        menu_data_face(invalid, MenuTemplateKind::Textured);
    }, "Unsupported menu template in COM/STAT.DAT");
    packet[3] = 0;
    rejects([&] {
        kf::codec::Reader invalid(packet);
        menu_data_face(invalid, MenuTemplateKind::Textured);
    }, "Invalid empty menu template in COM/STAT.DAT");
    storage.fill(0);
    kf::codec::Reader empty(packet.first(24));
    menu_data_face(empty, MenuTemplateKind::Solid);
    assert(empty.remaining() == 0);
    packet[3] = 5;
    packet[7] = 0x28;
    packet[8] = 0xff;
    packet[9] = 0xff;
    kf::codec::Reader solid(packet.first(24));
    face = menu_data_face(solid, MenuTemplateKind::Solid);
    assert(face.material.kind == kf::SurfaceKind::Solid && face.vertices[0].x == -1);

    const std::array<u8, 12> sprite{1, 0, 0x43, 0, 7, 1, 9, 2, 0xfe, 0xff, 0, 0x80};
    kf::codec::Reader sprites(sprite), tiles(sprite);
    const auto description = menu_data_sprite(sprites);
    const auto tile = menu_data_tile(tiles);
    assert(description.u == 263 && description.v == 521);
    assert(description.width == -2 && description.height == -32768);
    assert(tile.u == 7 && tile.v == 9 && tile.width == 65534 && tile.height == 32768);
    assert(sprites.remaining() == 0 && tiles.remaining() == 0);

    KfMenuResources resources{};
    rejects([&] { menu_resources_decode({}, resources); }, "Truncated COM/STAT.DAT");
    // Empty authored templates plus all-zero labels/prices are structurally valid.
    std::array<u8, 65536> blank{};
    menu_resources_decode(blank, resources);
    assert(resources.buy_prices[0][0] == 0 && resources.sell_prices[0][0] == 0);
}
