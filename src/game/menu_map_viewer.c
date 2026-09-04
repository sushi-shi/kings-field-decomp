#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

extern u32 pad_read();

RODATA(0x800122e4, 0xc)

/* Display the current-floor map image and player-position marker. */
ADDRESS(0x80022d7c, 0x400)
void menu_map_viewer(s32 item_code)
{
    s32 frame = 0;
    POLY_FT4 poly_bg[2];
    POLY_FT4 poly_marker[2];
    char path[16] = "MAP\\M00.";
    u8 *buffer;
    s32 map_number;

    map_number = 2;
    if (item_code == 0x37)
        map_number = 1;
    path[5] = map_number + '0';
    path[6] = player_state.progress_state.current_floor + '0';

    buffer = display_state.primitive_buffer->cursor;
    if (cd_file_load_into(buffer, path) != 0)
        return;
    tim_upload_images((u_long *)buffer);

    SetPolyFT4(&poly_bg[0]);
    SetSemiTrans(&poly_bg[0], 1);
    poly_bg[0].r0 = 0x40;
    poly_bg[0].g0 = 0x40;
    poly_bg[0].b0 = 0x40;
    poly_bg[0].clut = 0x7d40;
    poly_bg[0].tpage = 0x1f;
    poly_bg[0].u0 = 0;
    poly_bg[0].v0 = 0;
    poly_bg[0].u1 = 0xdb;
    poly_bg[0].v1 = 0;
    poly_bg[0].u2 = 0;
    poly_bg[0].v2 = 0xdb;
    poly_bg[0].u3 = 0xdb;
    poly_bg[0].v3 = 0xdb;
    poly_bg[0].x0 = 0x32;
    poly_bg[0].y0 = 0xa;
    poly_bg[0].x1 = 0x10d;
    poly_bg[0].y1 = 0xa;
    poly_bg[0].x2 = 0x32;
    poly_bg[0].y2 = 0xe5;
    poly_bg[0].x3 = 0x10d;
    poly_bg[0].y3 = 0xe5;
    poly_bg[1] = poly_bg[0];

    SetPolyFT4(&poly_marker[0]);
    poly_marker[0].r0 = 0x40;
    poly_marker[0].g0 = 0x40;
    poly_marker[0].b0 = 0x40;
    poly_marker[0].clut = 0x7c40;
    poly_marker[0].tpage = 0x1b;
    poly_marker[0].u0 = 0;
    poly_marker[0].v0 = 0;
    poly_marker[0].u1 = 4;
    poly_marker[0].v1 = 0;
    poly_marker[0].u2 = 0;
    poly_marker[0].v2 = 4;
    poly_marker[0].u3 = 4;
    poly_marker[0].v3 = 4;
    poly_marker[0].x0 = player_state.map_cell.x * 2 + 58;
    poly_marker[0].y0 = 0xd8 - player_state.map_cell.z * 2;
    poly_marker[0].x1 = player_state.map_cell.x * 2 + 62;
    poly_marker[0].y1 = 0xd8 - player_state.map_cell.z * 2;
    poly_marker[0].x2 = player_state.map_cell.x * 2 + 58;
    poly_marker[0].y2 = 0xdc - player_state.map_cell.z * 2;
    poly_marker[0].x3 = player_state.map_cell.x * 2 + 62;
    poly_marker[0].y3 = 0xdc - player_state.map_cell.z * 2;
    poly_marker[1] = poly_marker[0];

    for (;;) {
        menu_frame_begin();
        AddPrim(display_state.ordering_table + 500,
                &poly_marker[display_state.buffer_index]);
        AddPrim(display_state.ordering_table + 1000,
                &poly_bg[display_state.buffer_index]);
        AddPrim(display_state.ordering_table + 3000,
                &DAT_800580e8[display_state.buffer_index][3]);
        AddPrim(display_state.ordering_table + 3000,
                &DAT_800580e8[display_state.buffer_index][2]);
        AddPrim(display_state.ordering_table + 3000,
                &DAT_800580e8[display_state.buffer_index][1]);
        AddPrim(display_state.ordering_table + 3000,
                &DAT_800580e8[display_state.buffer_index][0]);
        menu_present_frame();
        if (frame < 2) {
            frame++;
        } else if (frame == 2) {
            while (pad_read(1) != 0)
                ;
            frame++;
        } else {
            if (pad_read(1) == 0)
                continue;
            while (pad_read(1) != 0)
                ;
            return;
        }
    }
}
