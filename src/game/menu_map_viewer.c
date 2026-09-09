#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

enum {
    MENU_MAP_MARKER_OT_DEPTH = 500,
    MENU_MAP_IMAGE_OT_DEPTH = 1000,
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

RODATA(0x800122e4, 0x9)

/* Display the current-floor map image and player-position marker. */
ADDRESS(0x80022d7c, 0x400)
void menu_map_viewer(KF_ENUM_PARAM(KfItemId, s32) item_code)
{
    s32 frame = 0;
    POLY_FT4 poly_bg[KF_DISPLAY_BUFFER_COUNT];
    POLY_FT4 poly_marker[KF_DISPLAY_BUFFER_COUNT];
    char path[16] = "MAP\\M00.";
    u8 *buffer;
    s32 map_number;

    map_number = MENU_MAP_DEFAULT_SET;
    if (item_code == KF_ITEM_WATCHMAN_MAP)
        map_number = MENU_MAP_WATCHMAN_SET;
    path[5] = map_number + '0';
    path[6] = KF_ENUM_ENCODE(u8, player_state.progress_state.current_floor) + '0';

    buffer = game_graphics_runtime.display_state.primitive_buffer->cursor;
    if (cd_file_load_into(buffer, path) != KF_RESOURCE_LOADED)
        return;
    tim_upload_images((u_long *)buffer);

    SetPolyFT4(&poly_bg[0]);
    SetSemiTrans(&poly_bg[0], 1);
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
        AddPrim(game_graphics_runtime.display_state.ordering_table + MENU_MAP_MARKER_OT_DEPTH,
                &poly_marker[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)]);
        AddPrim(game_graphics_runtime.display_state.ordering_table + MENU_MAP_IMAGE_OT_DEPTH,
                &poly_bg[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)]);
        AddPrim(game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
                &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][3]);
        AddPrim(game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
                &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][2]);
        AddPrim(game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
                &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][1]);
        AddPrim(game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
                &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][0]);
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
