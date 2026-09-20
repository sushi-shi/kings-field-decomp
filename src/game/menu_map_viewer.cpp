#include <kf/game/graphics.h>

#include <kf/game/menu.h>
#include <kf/game/game.h>
#include <kf/lib/render_face.h>

enum {
    MENU_MAP_PIXELS_PER_CELL = 2,
    MENU_MAP_MARKER_SPAN = 4,
    MENU_MAP_WATCHMAN_SET = 1,
    MENU_MAP_DEFAULT_SET = 2,
    MENU_MAP_BRIGHTNESS = 64,
    MENU_MAP_IMAGE_CLUT = 0x7d40,
    MENU_MAP_IMAGE_TPAGE = 0x1f,
    MENU_MAP_MARKER_CLUT = 0x7c40,
    MENU_MAP_MARKER_TPAGE = 0x1b,
    MENU_MAP_IMAGE_SPAN = 219,
    MENU_MAP_IMAGE_LEFT_X = 50,
    MENU_MAP_IMAGE_TOP_Y = 10,
    MENU_MAP_MARKER_ORIGIN_X = 58,
    MENU_MAP_MARKER_ORIGIN_Y = 216
};

static constexpr unsigned map_image_path_capacity = 16;
static constexpr unsigned map_image_set_offset = 5, map_image_floor_offset = 6;

void menu_map_viewer(KfObjectId item_id)
{
    s32 frame = 0;
    kf::DrawFace background{};
    kf::DrawFace marker{};
    char path[map_image_path_capacity] = "MAP/M00.";
    u8 *buffer;
    s32 map_number;

    map_number = MENU_MAP_DEFAULT_SET;
    if (item_id == KF_ITEM_WATCHMAN_MAP)
        map_number = MENU_MAP_WATCHMAN_SET;
    path[map_image_set_offset] = map_number + '0';
    path[map_image_floor_offset] = kf_enum_encode<u8>(player_state.progress_state.current_floor) + '0';

    buffer = game_graphics_runtime.display_state.asset_load_buffer;
    std::size_t image_size;
    if (resource_file_load_into(buffer,
            game_graphics_runtime.display_state.asset_load_capacity, path, &image_size) != KF_RESOURCE_LOADED)
        return;
    tim_upload_images(buffer, image_size);

    background.material = render_texture_material(MENU_MAP_IMAGE_TPAGE, MENU_MAP_IMAGE_CLUT);
    background.transparency = kf::FaceTransparency::Blend;
    background.depth = MENU_CONTENT_OT_DEPTH;
    render_face_uv_rectangle(&background, 0, 0, MENU_MAP_IMAGE_SPAN, MENU_MAP_IMAGE_SPAN);
    render_face_rectangle(&background, MENU_MAP_IMAGE_LEFT_X, MENU_MAP_IMAGE_TOP_Y,
        MENU_MAP_IMAGE_LEFT_X + MENU_MAP_IMAGE_SPAN, MENU_MAP_IMAGE_TOP_Y + MENU_MAP_IMAGE_SPAN);

    marker.material = render_texture_material(MENU_MAP_MARKER_TPAGE, MENU_MAP_MARKER_CLUT);
    marker.depth = MENU_MARKER_OT_DEPTH;
    render_face_uv_rectangle(&marker, 0, 0, MENU_MAP_MARKER_SPAN, MENU_MAP_MARKER_SPAN);
    const s32 marker_x = player_state.motion_state.fields.map_cell.coords.x * MENU_MAP_PIXELS_PER_CELL + MENU_MAP_MARKER_ORIGIN_X;
    const s32 marker_y = MENU_MAP_MARKER_ORIGIN_Y - player_state.motion_state.fields.map_cell.coords.z * MENU_MAP_PIXELS_PER_CELL;
    render_face_rectangle(&marker, marker_x, marker_y,
        marker_x + MENU_MAP_MARKER_SPAN, marker_y + MENU_MAP_MARKER_SPAN);
    for (unsigned i = 0; i < 4; ++i) {
        auto &image_vertex = background.vertices[i];
        auto &marker_vertex = marker.vertices[i];
        image_vertex.r = image_vertex.g = image_vertex.b = MENU_MAP_BRIGHTNESS / kf::texture_color_unity;
        marker_vertex.r = marker_vertex.g = marker_vertex.b = MENU_MAP_BRIGHTNESS / kf::texture_color_unity;
        image_vertex.a = marker_vertex.a = 1;
    }

    for (;;) {
        menu_frame_begin();
        kf::host_enqueue_face(marker);
        kf::host_enqueue_face(background);
        menu_enqueue_background();
        menu_present_frame();
        if (frame < MENU_PANEL_INPUT_RELEASE_FRAME) {
            frame++;
        } else if (frame == MENU_PANEL_INPUT_RELEASE_FRAME) {
            kf::host_wait_buttons_released();
            frame++;
        } else {
            if (kf::host_read_buttons() == 0)
                continue;
            kf::host_wait_buttons_released();
            return;
        }
    }
}
