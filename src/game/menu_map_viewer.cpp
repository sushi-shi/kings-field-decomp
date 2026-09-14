#include <kf/game/graphics.h>

#include <kf/game/menu.h>
#include <kf/game/game.h>

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

void menu_map_viewer(KfObjectId item_id)
{
    s32 frame = 0;
    POLY_FT4 poly_bg[KF_DISPLAY_BUFFER_COUNT];
    POLY_FT4 poly_marker[KF_DISPLAY_BUFFER_COUNT];
    char path[16] = "MAP\\M00.";
    u8 *buffer;
    s32 map_number;

    map_number = MENU_MAP_DEFAULT_SET;
    if (item_id == KF_ITEM_WATCHMAN_MAP)
        map_number = MENU_MAP_WATCHMAN_SET;
    path[5] = map_number + '0';
    path[6] = kf_enum_encode<u8>(player_state.progress_state.current_floor) + '0';

    buffer = game_graphics_runtime.display_state.primitive_buffer->cursor;
    if (cd_file_load_into((void *)buffer, path) != KF_RESOURCE_LOADED)
        return;
    tim_upload_images(buffer);

    SetPolyFT4(&poly_bg[0]);
    SetSemiTrans((void *)(&poly_bg[0]), 1);
    setRGB0(&poly_bg[0], MENU_MAP_BRIGHTNESS, MENU_MAP_BRIGHTNESS, MENU_MAP_BRIGHTNESS);
    poly_bg[0].clut = MENU_MAP_IMAGE_CLUT;
    poly_bg[0].tpage = MENU_MAP_IMAGE_TPAGE;
    setUVWH(&poly_bg[0], 0, 0, MENU_MAP_IMAGE_SPAN, MENU_MAP_IMAGE_SPAN);
    setXYWH(&poly_bg[0],
        MENU_MAP_IMAGE_LEFT_X,
        MENU_MAP_IMAGE_TOP_Y,
        MENU_MAP_IMAGE_SPAN,
        MENU_MAP_IMAGE_SPAN);
    poly_bg[1] = poly_bg[0];

    SetPolyFT4(&poly_marker[0]);
    setRGB0(&poly_marker[0], MENU_MAP_BRIGHTNESS, MENU_MAP_BRIGHTNESS, MENU_MAP_BRIGHTNESS);
    poly_marker[0].clut = MENU_MAP_MARKER_CLUT;
    poly_marker[0].tpage = MENU_MAP_MARKER_TPAGE;
    setUVWH(&poly_marker[0], 0, 0, MENU_MAP_MARKER_SPAN, MENU_MAP_MARKER_SPAN);
    setXYWH(&poly_marker[0],
        player_state.motion_state.fields.map_cell.coords.x * MENU_MAP_PIXELS_PER_CELL + MENU_MAP_MARKER_ORIGIN_X,
        MENU_MAP_MARKER_ORIGIN_Y - player_state.motion_state.fields.map_cell.coords.z * MENU_MAP_PIXELS_PER_CELL,
        MENU_MAP_MARKER_SPAN,
        MENU_MAP_MARKER_SPAN);
    poly_marker[1] = poly_marker[0];

    for (;;) {
        menu_frame_begin();
        AddPrim((void *)(game_graphics_runtime.display_state.ordering_table + MENU_MARKER_OT_DEPTH),
                (void *)(&poly_marker[kf_enum_encode<u8>(game_graphics_runtime.display_state.buffer_index)]));
        AddPrim((void *)(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH),
                (void *)(&poly_bg[kf_enum_encode<u8>(game_graphics_runtime.display_state.buffer_index)]));
        MENU_ENQUEUE_BACKGROUND();
        menu_present_frame();
        if (frame < MENU_PANEL_INPUT_RELEASE_FRAME) {
            frame++;
        } else if (frame == MENU_PANEL_INPUT_RELEASE_FRAME) {
            while (PadRead(1) != 0)
                ;
            frame++;
        } else {
            if (PadRead(1) == 0)
                continue;
            while (PadRead(1) != 0)
                ;
            return;
        }
    }
}
