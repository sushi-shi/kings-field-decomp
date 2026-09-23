#include <kf/lib/null.h>
#include <kf/game/graphics.h>

#include <kf/lib/map_data.h>
#include <kf/game/cd.h>
#include <kf/game/render.h>
#include <kf/game/notify.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <kf/game/game.h>
#include <kf/lib/tmd.h>
#include <kf/lib/graphics.h>

enum {
    RENDER_PALETTE_HUD,
    RENDER_PALETTE_NOTIFICATION,
    RENDER_PALETTE_COUNT,
    SYSTEM_SCREEN_READ_ATTEMPTS = 50,
    PRIMITIVE_BUFFER_BYTES = 0x19640,
    INITIAL_BACK_COLOR = 60,
    SYSTEM_SCREEN_BRIGHTNESS = 96,
    SYSTEM_SCREEN_PATH_DIGIT = 2,
    EFFECT_TEXTURE_FIRST_PAGE_X = 320,
    EFFECT_TEXTURE_SECOND_PAGE_X = 384,
    EFFECT_TEXTURE_THIRD_PAGE_X = 832,
    EFFECT_TEXTURE_CLUT_Y = 491,
    FLOOR_ITEM_TPAGE_X = 896,
    FLOOR_ITEM_CLUT = 0x7a40,
    HUD_TPAGE_X = 896,
    NOTIFICATION_TPAGE_X = 832,
    NOTIFICATION_DIGIT_TPAGE = 0x1c
};

static RECT render_palette_rects[RENDER_PALETTE_COUNT] = {{0, 500, 16, 1}, {0, 499, 16, 1}};

MATRIX color_matrix_table[KF_GAME_COLOR_PRESET_COUNT] = {
    {{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},
    {{{3000, 1000, 4000}, {200, 70, 400}, {200, 70, 400}}, {0, 0, 0}},
    {{{1000, 350, 2000}, {1000, 350, 2000}, {3000, 1000, 4000}}, {0, 0, 0}},
    {{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},
    {{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},
    {{{0, 0, 0}, {4095, 4095, 4095}, {0, 0, 0}}, {0, 0, 0}},
    {{{0, 0, 0}, {0, 0, 0}, {4095, 4095, 4095}}, {0, 0, 0}},
};

char system_screen_path[7] = "\\E0.;1";

KfGraphicsRuntimeGame game_graphics_runtime;

u32 DAT_800a0768;

void display_show_system_screen(KfSystemScreen screen)
{
    POLY_FT4 prim;
    s32 back;
    s32 attempt;
    s32 brightness;

    DrawSync(0);
    SetPolyFT4(&prim);
    SetSemiTrans((void *)&prim, 1);
    setXY4(&prim,
        KF_SYSTEM_SCREEN_LEFT, KF_SYSTEM_SCREEN_TOP,
        KF_SYSTEM_SCREEN_RIGHT, KF_SYSTEM_SCREEN_TOP,
        KF_SYSTEM_SCREEN_LEFT, KF_SYSTEM_SCREEN_BOTTOM,
        KF_SYSTEM_SCREEN_RIGHT, KF_SYSTEM_SCREEN_BOTTOM);
    setUVWH(&prim, 0, 0, KF_SYSTEM_SCREEN_U_SPAN, KF_SYSTEM_SCREEN_V_SPAN);
    prim.clut = GetClut(0, KF_SYSTEM_SCREEN_CLUT_Y);
    prim.tpage = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        KF_SYSTEM_SCREEN_TPAGE_X, KF_TEXTURE_LOWER_PAGE_Y);

    memcpy((void *)cd_path_buffer, (const void *)system_screen_path, sizeof system_screen_path);
    cd_path_buffer[SYSTEM_SCREEN_PATH_DIGIT] = ((s32)(screen)) + '0';
    brightness = SYSTEM_SCREEN_BRIGHTNESS;
    if (CdSearchFile(&cd_search_file, cd_path_buffer) == NULL) {
        exit(1);
    }
    if (cd_search_file.size & (KF_CD_SECTOR_BYTES - 1)) {
        cd_search_file.size =
            ((cd_search_file.size >> KF_CD_SECTOR_SHIFT) + 1) << KF_CD_SECTOR_SHIFT;
    }
    CD_LOCATION_COPY(cd_read_location, cd_search_file.pos);
    for (attempt = 0; attempt < SYSTEM_SCREEN_READ_ATTEMPTS; attempt++) {
        s32 result;

        CdControl(CdlSetloc, (u_char *)&cd_read_location, NULL);
        CdRead(cd_search_file.size >> KF_CD_SECTOR_SHIFT,
               (u_long *)game_graphics_runtime.display_state.asset_load_buffer, CdlModeSpeed);
        while ((result = CdReadSync(KF_CD_READ_POLL, NULL)) > 0) {
        }
        if (result == 0) {
            attempt = KF_CD_READ_STOP_ATTEMPT;
        }
    }
    tim_upload_images(game_graphics_runtime.display_state.asset_load_buffer);

    back = game_graphics_runtime.display_state.buffer_index == KF_DISPLAY_BUFFER_FIRST;
    game_graphics_runtime.display_draw_environments[back].isbg = 0;
    game_graphics_runtime.display_draw_environments[back].dfe = 0;
    PutDrawEnv(&game_graphics_runtime.display_draw_environments[back]);
    game_graphics_runtime.display_state.ordering_table = game_graphics_runtime.display_state.ordering_tables[back].entries;
    setRGB0(&prim, brightness, brightness, brightness);
    ClearOTagR(game_graphics_runtime.display_state.ordering_table, KF_ORDERING_TABLE_LENGTH);
    AddPrim((void *)game_graphics_runtime.display_state.ordering_table, (void *)&prim);
    DrawSync(0);
    DrawOTag(game_graphics_runtime.display_state.ordering_table + (KF_ORDERING_TABLE_LENGTH - 1));
    while (PadRead(1) == 0) {
    }
    while (PadRead(1) != 0) {
    }
    game_graphics_runtime.display_draw_environments[back].isbg = 1;
    game_graphics_runtime.display_draw_environments[back].dfe = 1;
    DrawSync(0);
}

#include "../lib/lighting_color.inc"

void effect5_texture_cache_prepare(KfFloorId floor)
{
    if (floor == KF_FLOOR_5) {
        game_graphics_runtime.effect5_texture_pages[0] = GetTPage(
            KF_GPU_TEXTURE_8BIT, KF_GPU_BLEND_AVERAGE,
            EFFECT_TEXTURE_FIRST_PAGE_X, KF_TEXTURE_LOWER_PAGE_Y);
        game_graphics_runtime.effect5_texture_pages[1] = GetTPage(
            KF_GPU_TEXTURE_8BIT, KF_GPU_BLEND_AVERAGE,
            EFFECT_TEXTURE_SECOND_PAGE_X, KF_TEXTURE_LOWER_PAGE_Y);
        game_graphics_runtime.effect5_texture_pages[2] = GetTPage(
            KF_GPU_TEXTURE_8BIT, KF_GPU_BLEND_AVERAGE,
            EFFECT_TEXTURE_THIRD_PAGE_X, KF_TEXTURE_LOWER_PAGE_Y);
        game_graphics_runtime.effect5_texture_cluts[0] = GetClut(0, EFFECT_TEXTURE_CLUT_Y);
        game_graphics_runtime.effect5_texture_cluts[1] = GetClut(0, EFFECT_TEXTURE_CLUT_Y);
        game_graphics_runtime.effect5_texture_cluts[2] = GetClut(0, EFFECT_TEXTURE_CLUT_Y);
    }
}

void display_initialize(void)
{
    ResetGraph(KF_GPU_RESET_KEEP_DISPLAY);
    InitGeom();
    SetGeomOffset(KF_DISPLAY_WIDTH / 2, KF_DISPLAY_HEIGHT / 2);
    SetDefDrawEnv(
        &game_graphics_runtime.display_draw_environments[0], 0, 0,
        KF_DISPLAY_WIDTH, KF_DISPLAY_HEIGHT);
    SetDefDispEnv(
        &game_graphics_runtime.display_disp_environments[0], 0, KF_DISPLAY_HEIGHT,
        KF_DISPLAY_WIDTH, KF_DISPLAY_HEIGHT);
    SetDefDrawEnv(
        &game_graphics_runtime.display_draw_environments[1], 0, KF_DISPLAY_HEIGHT,
        KF_DISPLAY_WIDTH, KF_DISPLAY_HEIGHT);
    SetDefDispEnv(
        &game_graphics_runtime.display_disp_environments[1], 0, 0,
        KF_DISPLAY_WIDTH, KF_DISPLAY_HEIGHT);
    game_graphics_runtime.display_draw_environments[0].dtd = game_graphics_runtime.display_draw_environments[1].dtd = 1;
    game_graphics_runtime.display_draw_environments[0].isbg = 1;
    game_graphics_runtime.display_draw_environments[1].isbg = 1;
    setRGB0(&game_graphics_runtime.display_draw_environments[0], 0, 0, 0);
    setRGB0(&game_graphics_runtime.display_draw_environments[1], 0, 0, 0);
    PutDispEnv(&game_graphics_runtime.display_disp_environments[0]);
    SetBackColor(INITIAL_BACK_COLOR, INITIAL_BACK_COLOR, INITIAL_BACK_COLOR);
    lighting_set_active_color_matrix(KF_GAME_COLOR_DEFAULT);
    SetFarColor(0, 0, 0);
    game_graphics_runtime.render_state.fog_near_distance = KF_INITIAL_FOG_NEAR_DISTANCE;
    SetFogNear(KF_INITIAL_FOG_NEAR_DISTANCE, KF_DEFAULT_PROJECTION_DISTANCE);
    render_initialize();
}

void render_initialize(void)
{
    SVECTOR angles;
    KfNotificationId *flag;
    u8 count;
    u8 *buffer;

    game_graphics_runtime.display_state.buffer_index = KF_DISPLAY_BUFFER_UNINITIALIZED;
    buffer = (u8 *)memory_allocate(KF_DISPLAY_BUFFER_COUNT * PRIMITIVE_BUFFER_BYTES);
    game_graphics_runtime.display_state.asset_load_buffer = buffer;
    game_graphics_runtime.display_state.primitive_buffers[0].start = buffer;
    buffer += PRIMITIVE_BUFFER_BYTES;
    game_graphics_runtime.display_state.primitive_buffers[0].end = buffer;
    game_graphics_runtime.display_state.primitive_buffers[1].start = buffer;
    buffer += PRIMITIVE_BUFFER_BYTES;
    game_graphics_runtime.display_state.primitive_buffers[1].end = buffer;
    game_graphics_runtime.floor_item_count = 0;
    setVector(&angles, 0, 0, 0);
    RotMatrix(&angles, &game_graphics_runtime.render_state.quadrant_matrices[0]);
    angles.vy = KF_ANGLE_THREE_QUARTER_TURN;
    RotMatrix(&angles, &game_graphics_runtime.render_state.quadrant_matrices[3]);
    angles.vy = KF_ANGLE_HALF_TURN;
    RotMatrix(&angles, &game_graphics_runtime.render_state.quadrant_matrices[2]);
    angles.vy = KF_ANGLE_QUARTER_TURN;
    RotMatrix(&angles, &game_graphics_runtime.render_state.quadrant_matrices[1]);
    game_graphics_runtime.render_state.light_matrix.m[0][0] = 3800;
    game_graphics_runtime.render_state.light_matrix.m[0][1] = -2800;
    game_graphics_runtime.render_state.light_matrix.m[0][2] = 0;
    game_graphics_runtime.render_state.light_matrix.m[1][0] = -3000;
    game_graphics_runtime.render_state.light_matrix.m[1][1] = -3600;
    game_graphics_runtime.render_state.light_matrix.m[1][2] = -3400;
    game_graphics_runtime.render_state.light_matrix.m[2][0] = -1300;
    game_graphics_runtime.render_state.light_matrix.m[2][1] = 2700;
    game_graphics_runtime.render_state.light_matrix.m[2][2] = 800;
    game_graphics_runtime.render_state.light_matrix_copy = game_graphics_runtime.render_state.light_matrix;
    MulMatrix0(
        &game_graphics_runtime.render_state.light_matrix,
        &game_graphics_runtime.render_state.quadrant_matrices[0],
        &game_graphics_runtime.light_quadrant_matrices[0]);
    MulMatrix0(
        &game_graphics_runtime.render_state.light_matrix,
        &game_graphics_runtime.render_state.quadrant_matrices[1],
        &game_graphics_runtime.light_quadrant_matrices[1]);
    MulMatrix0(
        &game_graphics_runtime.render_state.light_matrix,
        &game_graphics_runtime.render_state.quadrant_matrices[2],
        &game_graphics_runtime.light_quadrant_matrices[2]);
    MulMatrix0(
        &game_graphics_runtime.render_state.light_matrix,
        &game_graphics_runtime.render_state.quadrant_matrices[3],
        &game_graphics_runtime.light_quadrant_matrices[3]);
    game_graphics_runtime.floor_item_tpage = GetTPage(
        KF_GPU_TEXTURE_8BIT, KF_GPU_BLEND_AVERAGE,
        FLOOR_ITEM_TPAGE_X, 0);
    game_graphics_runtime.floor_item_clut = FLOOR_ITEM_CLUT;
    game_graphics_runtime.hud_tpage = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        HUD_TPAGE_X, KF_TEXTURE_LOWER_PAGE_Y);
    game_graphics_runtime.hud_clut = GetClut(render_palette_rects[RENDER_PALETTE_HUD].x, render_palette_rects[RENDER_PALETTE_HUD].y);
    game_graphics_runtime.notification_text_tpage = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        NOTIFICATION_TPAGE_X, KF_TEXTURE_LOWER_PAGE_Y);
    game_graphics_runtime.notification_digit_clut = game_graphics_runtime.notification_text_clut =
        GetClut(render_palette_rects[RENDER_PALETTE_NOTIFICATION].x, render_palette_rects[RENDER_PALETTE_NOTIFICATION].y);
    game_graphics_runtime.notification_digit_tpage = NOTIFICATION_DIGIT_TPAGE;
    game_graphics_runtime.notification_state.control.effect_phase = KF_NOTIFICATION_IDLE;
    game_graphics_runtime.notification_state.control.queue_tail = 0;
    game_graphics_runtime.notification_state.control.queue_head = 0;
    flag = game_graphics_runtime.notification_message_ids;
    count = KF_NOTIFICATION_CAPACITY - 1;
    do {
        *flag++ = KF_NOTIFICATION_NONE;
    } while (count-- != 0);
    pool_reset();
}

#include "../lib/display_frame.inc"

#include "../lib/tmd.inc"

void tmd_project_vertices(s32 count)
{
    KfScreenVertex *projected;
    SVECTOR *vertex;
    long perspective;
    long gte_flags;
    long depth;
    long unused_depth;

    projected = game_graphics_runtime.tmd_projected_vertices;
    vertex = game_graphics_runtime.current_tmd_vertices;
    for (count--; count != -1; count--) {
        RotTransPers(vertex, &projected->sxy.word, &perspective, &gte_flags);
        projected->p2 = perspective << KF_TMD_DEFAULT_PERSPECTIVE_SHIFT;
        ReadSZ2(&depth, &unused_depth);
        projected->sz = depth;
        projected++;
        vertex++;
    }
}

#include "../lib/tmd_transform.inc"
