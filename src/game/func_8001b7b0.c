#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/psyq_audio.h>

extern KfDisplayState display_state;
extern DRAWENV display_draw_environments[2];
extern CdlFILE cd_search_file;
extern char cd_path_buffer[80];
extern CdlLOC cd_read_location;

extern void tim_upload_images(u_long *tim_data);
extern u32 pad_read();
extern void exit(s32 status);

#define CD_SECTOR_SIZE 0x800
#define CD_SECTOR_SHIFT 11

/* Disc file of the fatal-error message screen; the index-2 digit selects the
 * stage: 0 failed disc search, 1 failed disc read, 2 save-file error. */
DATA(0x80057b50, 0x7)
char error_screen_path[7] = "\\E0.;1";

/* Loads and shows the fatal-error message screen for STAGE as a semi-transparent
 * textured box, then blocks until a controller button is pressed and released.
 * Called by cd_file_load_into on disc failure and by game_main_loop on a save
 * error. */
ADDRESS(0x8001b7b0, 0x308)
void func_8001b7b0(s32 stage)
{
    POLY_FT4 prim;
    s32 back;
    s32 attempt;

    DrawSync(0);
    SetPolyFT4(&prim);
    SetSemiTrans(&prim, 1);
    prim.x0 = 32;
    prim.y0 = 112;
    prim.x1 = 288;
    prim.y1 = 112;
    prim.x2 = 32;
    prim.y2 = 240;
    prim.x3 = 288;
    prim.y3 = 240;
    prim.u0 = 0;
    prim.v0 = 0;
    prim.u1 = 255;
    prim.v1 = 0;
    prim.u2 = 0;
    prim.v2 = 128;
    prim.u3 = 255;
    prim.v3 = 128;
    prim.clut = GetClut(0, 0x1f5);
    prim.tpage = GetTPage(0, 0, 0x3c0, 0x100);

    memcpy(cd_path_buffer, error_screen_path, sizeof error_screen_path);
    cd_path_buffer[2] = stage + '0';
    if (CdSearchFile(&cd_search_file, cd_path_buffer) == 0) {
        exit(1);
    }
    if (cd_search_file.size & (CD_SECTOR_SIZE - 1)) {
        cd_search_file.size =
            ((cd_search_file.size >> CD_SECTOR_SHIFT) + 1) << CD_SECTOR_SHIFT;
    }
    cd_read_location.minute = cd_search_file.pos.minute;
    cd_read_location.second = cd_search_file.pos.second;
    cd_read_location.sector = cd_search_file.pos.sector;
    for (attempt = 0; attempt < 50; attempt++) {
        s32 result;

        CdControl(CdlSetloc, (u_char *)&cd_read_location, 0);
        CdRead(cd_search_file.size >> CD_SECTOR_SHIFT,
               display_state.asset_load_buffer, CdlModeSpeed);
        while ((result = CdReadSync(1, 0)) > 0) {
        }
        if (result == 0) {
            attempt = 100;
        }
    }
    tim_upload_images(display_state.asset_load_buffer);

    back = display_state.buffer_index == 0;
    display_draw_environments[back].isbg = 0;
    display_draw_environments[back].dfe = 0;
    PutDrawEnv(&display_draw_environments[back]);
    display_state.ordering_table = display_state.ordering_tables[back].entries;
    prim.r0 = prim.g0 = prim.b0 = 0x60;
    ClearOTagR(display_state.ordering_table, 0x4000);
    AddPrim(display_state.ordering_table, &prim);
    DrawSync(0);
    DrawOTag(display_state.ordering_table + 0x3fff);
    while (pad_read(1) == 0) {
    }
    while (pad_read(1) != 0) {
    }
    display_draw_environments[back].isbg = 1;
    display_draw_environments[back].dfe = 1;
    DrawSync(0);
}
