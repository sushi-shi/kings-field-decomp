#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_cd.h>
#include <kf/game_render.h>
#include <kf/notify.h>
#include <kf/psyq.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>
#include <kf/tmd.h>

enum {
    ERROR_SCREEN_READ_ATTEMPTS = 50,
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

/* Object-table records follow the 12-byte TMD header of the selected asset. */
#define TMD_OBJECTS(asset) ((KfTmdObject *)((asset) + 1))
/* Packet bodies follow the 4-byte packet header (olen, ilen, flag, mode). */
#define TMD_PACKET_BODY(packet) ((packet) + KF_TMD_PACKET_HEADER_BYTES)

/* tmd_register primitive-mode dispatch table. */
RODATA(0x800121b4, 0x74)

/* System-message TIM path; byte 2 selects an error or pause screen. */
DATA(0x80057b50, 0x7)
char error_screen_path[7] = "\\E0.;1";

DATA(0x80070e98, 0x249cc)
KfGraphicsRuntimeGame game_graphics_runtime;

DATA(0x800a0768, 0x4)
u32 DAT_800a0768;

/* Loads and shows the system-message screen for STAGE as a semi-transparent
 * textured box, then blocks until a controller button is pressed and released.
 * Called by cd_file_load_into on disc failure and by game_main_loop on a save
 * error. */
ADDRESS(0x8001b7b0, 0x308)
void display_show_error_screen(KfSystemScreen stage)
{
    POLY_FT4 prim;
    s32 back;
    s32 attempt;
    s32 brightness;

    DrawSync(0);
    SetPolyFT4(&prim);
    SetSemiTrans(&prim, 1);
    prim.x0 = KF_SYSTEM_SCREEN_LEFT;
    prim.y0 = KF_SYSTEM_SCREEN_TOP;
    prim.x1 = KF_SYSTEM_SCREEN_RIGHT;
    prim.y1 = KF_SYSTEM_SCREEN_TOP;
    prim.x2 = KF_SYSTEM_SCREEN_LEFT;
    prim.y2 = KF_SYSTEM_SCREEN_BOTTOM;
    prim.x3 = KF_SYSTEM_SCREEN_RIGHT;
    prim.y3 = KF_SYSTEM_SCREEN_BOTTOM;
    prim.u0 = 0;
    prim.v0 = 0;
    prim.u1 = KF_SYSTEM_SCREEN_U_SPAN;
    prim.v1 = 0;
    prim.u2 = 0;
    prim.v2 = KF_SYSTEM_SCREEN_V_SPAN;
    prim.u3 = KF_SYSTEM_SCREEN_U_SPAN;
    prim.v3 = KF_SYSTEM_SCREEN_V_SPAN;
    prim.clut = GetClut(0, KF_SYSTEM_SCREEN_CLUT_Y);
    prim.tpage = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        KF_SYSTEM_SCREEN_TPAGE_X, KF_TEXTURE_LOWER_PAGE_Y);

    memcpy(cd_path_buffer, error_screen_path, sizeof error_screen_path);
    cd_path_buffer[SYSTEM_SCREEN_PATH_DIGIT] = KF_ENUM_ENCODE(s32, stage) + '0';
    brightness = SYSTEM_SCREEN_BRIGHTNESS;
    if (CdSearchFile(&cd_search_file, cd_path_buffer) == 0) {
        exit(1);
    }
    if (cd_search_file.size & (KF_CD_SECTOR_BYTES - 1)) {
        cd_search_file.size =
            ((cd_search_file.size >> KF_CD_SECTOR_SHIFT) + 1) << KF_CD_SECTOR_SHIFT;
    }
    cd_read_location.minute = cd_search_file.pos.minute;
    cd_read_location.second = cd_search_file.pos.second;
    cd_read_location.sector = cd_search_file.pos.sector;
    for (attempt = 0; attempt < ERROR_SCREEN_READ_ATTEMPTS; attempt++) {
        s32 result;

        CdControl(CdlSetloc, (u_char *)&cd_read_location, 0);
        CdRead(cd_search_file.size >> KF_CD_SECTOR_SHIFT,
               (u_long *)game_graphics_runtime.display_state.asset_load_buffer, CdlModeSpeed);
        while ((result = CdReadSync(KF_CD_READ_POLL, 0)) > 0) {
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
    AddPrim(game_graphics_runtime.display_state.ordering_table, &prim);
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

ADDRESS(0x8001bab8, 0x2c)
void lighting_set_active_color_matrix(KfGameColorPreset preset)
{
    SetColorMatrix(&color_matrix_table[KF_ENUM_ENCODE(s32, preset)]);
}

ADDRESS(0x8001bae4, 0xb0)
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

ADDRESS(0x8001bb94, 0x14c)
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
    game_graphics_runtime.display_draw_environments[0].r0 = 0;
    game_graphics_runtime.display_draw_environments[0].g0 = 0;
    game_graphics_runtime.display_draw_environments[0].b0 = 0;
    game_graphics_runtime.display_draw_environments[1].r0 = 0;
    game_graphics_runtime.display_draw_environments[1].g0 = 0;
    game_graphics_runtime.display_draw_environments[1].b0 = 0;
    PutDispEnv(&game_graphics_runtime.display_disp_environments[0]);
    SetBackColor(INITIAL_BACK_COLOR, INITIAL_BACK_COLOR, INITIAL_BACK_COLOR);
    lighting_set_active_color_matrix(KF_GAME_COLOR_DEFAULT);
    SetFarColor(0, 0, 0);
    game_graphics_runtime.render_state.fog_near_distance = KF_INITIAL_FOG_NEAR_DISTANCE;
    SetFogNear(KF_INITIAL_FOG_NEAR_DISTANCE, KF_DEFAULT_PROJECTION_DISTANCE);
    render_initialize();
}

ADDRESS(0x8001bce0, 0x2d8)
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
    angles.vx = 0;
    angles.vy = 0;
    angles.vz = 0;
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
    game_graphics_runtime.hud_clut = GetClut(hud_palette_rect.x, hud_palette_rect.y);
    game_graphics_runtime.notification_text_tpage = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        NOTIFICATION_TPAGE_X, KF_TEXTURE_LOWER_PAGE_Y);
    game_graphics_runtime.notification_digit_clut = game_graphics_runtime.notification_text_clut =
        GetClut(notification_palette_rect.x, notification_palette_rect.y);
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

ADDRESS(0x8001bfb8, 0x98)
void display_begin_frame(void)
{
    game_graphics_runtime.display_state.buffer_index = display_next_buffer(game_graphics_runtime.display_state.buffer_index);
    game_graphics_runtime.display_state.primitive_buffer = &game_graphics_runtime.display_state.primitive_buffers[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)];
    game_graphics_runtime.display_state.ordering_table =
        game_graphics_runtime.display_state.ordering_tables[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)].entries;
    ClearOTagR(game_graphics_runtime.display_state.ordering_table, KF_ORDERING_TABLE_LENGTH);
    game_graphics_runtime.display_state.primitive_buffer->cursor = game_graphics_runtime.display_state.primitive_buffer->start;
    DAT_800a0768 = 0;
    game_graphics_runtime.DAT_8009569c = 0;
    game_graphics_runtime.DAT_80095698 = 0;
}

ADDRESS(0x8001c050, 0x98)
void display_present_frame(void)
{
    DrawSync(0);
    VSync(0);
    PutDrawEnv(&game_graphics_runtime.display_draw_environments[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)]);
    PutDispEnv(&game_graphics_runtime.display_disp_environments[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)]);
    DrawOTag(game_graphics_runtime.display_state.ordering_table + (KF_ORDERING_TABLE_LENGTH - 1));
}

ADDRESS(0x8001c0e8, 0x2c)
void tmd_select(KfTmdSlot slot)
{
    game_graphics_runtime.tmd_state.current_asset = game_graphics_runtime.tmd_state.slots[KF_ENUM_ENCODE(u16, slot)];
}

ADDRESS(0x8001c114, 0x24)
KfTmdObject *tmd_get_object(u16 index)
{
    return TMD_OBJECTS(game_graphics_runtime.tmd_state.current_asset) + index;
}

ADDRESS(0x8001c138, 0x10)
void tmd_set_current_vertices(KfPackedSVector *vertices)
{
    game_graphics_runtime.current_tmd_vertices = vertices;
}

ADDRESS(0x8001c148, 0x3c)
void tmd_select_object_vertices(u16 index)
{
    game_graphics_runtime.current_tmd_vertices =
        (KfPackedSVector *)((u8 *)game_graphics_runtime.tmd_state.current_asset
            + KF_TMD_HEADER_BYTES + tmd_get_object(index)->vertex_offset);
}

ADDRESS(0x8001c184, 0x12c)
void render_set_view_transform(
    const VECTOR *position, const SVECTOR *rotation)
{
    SVECTOR angles;

    if (position != 0) {
        game_graphics_runtime.render_state.view_position = *position;
        game_graphics_runtime.render_state.view_cell.x = game_graphics_runtime.render_state.view_position.vx / KF_MAP_TILE_SIZE;
        game_graphics_runtime.render_state.view_cell.z = game_graphics_runtime.render_state.view_position.vz / KF_MAP_TILE_SIZE;
    }
    if (rotation != 0) {
        game_graphics_runtime.render_state.view_rotation = *rotation;
    }
    RotMatrix(&game_graphics_runtime.render_state.view_rotation, &game_graphics_runtime.render_state.view_matrix);
    angles.vz = 0;
    angles.vy = 0;
    angles.vx = game_graphics_runtime.render_state.view_rotation.vx;
    RotMatrix(&angles, &game_graphics_runtime.render_state.pitch_matrix);
}

/*
 * Converts every vertex and normal index of the current asset into a byte
 * offset (8-byte entries) in place.  The ABE bit of the packet mode is ignored.
 */
ADDRESS(0x8001c2b0, 0x300)
void tmd_prepare_primitive_indices(void)
{
    KfTmdObject *object;
    u8 *packet;
    u8 *body;
    KfTmdPrimitive *primitive;
    u16 object_count;
    u16 objects_left;
    u16 primitive_count;
    u16 primitives_left;
    KfTmdPacketHeader header;

    object_count = game_graphics_runtime.tmd_state.current_asset->object_count;
    objects_left = object_count - 1;
    object = TMD_OBJECTS(game_graphics_runtime.tmd_state.current_asset);
    if (object_count == 0) {
        return;
    }
    do {
        primitive_count = object->primitive_count;
        packet = (u8 *)game_graphics_runtime.tmd_state.current_asset + (object->primitive_offset + KF_TMD_HEADER_BYTES);
        primitives_left = primitive_count;
        primitives_left--;
        if (primitive_count != 0) {
            do {
                body = TMD_PACKET_BODY(packet);
                header.word = *(u32 *)packet;
                packet = body + header.bytes.input_length * KF_TMD_WORD_BYTES;
                primitive = (KfTmdPrimitive *)body;
                switch (tmd_packet_kind(header.word)) {
                case KF_TMD_MODE_F3: {
                    primitive->f3.v0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->f3.v1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->f3.v2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->f3.n0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    break;
                }
                case KF_TMD_MODE_G3: {
                    primitive->g3.v0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->g3.v1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->g3.v2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->g3.n0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->g3.n1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->g3.n2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    break;
                }
                case KF_TMD_MODE_FT3: {
                    primitive->ft3.v0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->ft3.v1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->ft3.v2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->ft3.n0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    break;
                }
                case KF_TMD_MODE_GT3: {
                    primitive->gt3.v0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->gt3.v1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->gt3.v2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->gt3.n0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->gt3.n1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->gt3.n2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    break;
                }
                case KF_TMD_MODE_F4: {
                    primitive->f4.v0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->f4.v1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->f4.v2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->f4.v3 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->f4.n0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    break;
                }
                case KF_TMD_MODE_G4: {
                    primitive->g4.v0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->g4.v1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->g4.v2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->g4.v3 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->g4.n0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->g4.n1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->g4.n2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->g4.n3 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    break;
                }
                case KF_TMD_MODE_FT4: {
                    primitive->ft4.v0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->ft4.v1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->ft4.v2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->ft4.v3 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->ft4.n0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    break;
                }
                case KF_TMD_MODE_GT4: {
                    primitive->gt4.v0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->gt4.v1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->gt4.v2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->gt4.v3 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->gt4.n0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->gt4.n1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->gt4.n2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    primitive->gt4.n3 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    break;
                }
                }
            } while (primitives_left-- != 0);
        }
        object++;
    } while (objects_left-- != 0);
}

ADDRESS(0x8001c5b0, 0x3c)
void tmd_register(KfTmdSlot slot, KfTmdHeader *tmd)
{
    game_graphics_runtime.tmd_state.current_asset = game_graphics_runtime.tmd_state.slots[KF_ENUM_ENCODE(u16, slot)] = tmd;
    tmd_prepare_primitive_indices();
}

ADDRESS(0x8001c5ec, 0x20)
void tmd_release_last_allocation(KF_ENUM_PARAM(KfTmdSlot, s32) slot)
{
    memory_release_last();
}

/* Store the full GTE depth; RotTransPers returns depth divided by four. */
ADDRESS(0x8001c60c, 0x9c)
void tmd_project_vertices(s32 count)
{
    KfScreenVertex *projected;
    KfPackedSVector *vertex;
    long perspective;
    long gte_flags;
    long depth;
    long unused_depth;

    projected = ((KfScreenVertex *)game_graphics_runtime.unknown_projection_morph_20318);
    vertex = game_graphics_runtime.current_tmd_vertices;
    for (count--; count != -1; count--) {
        RotTransPers(&vertex->vector, &projected->sxy.word, &perspective, &gte_flags);
        projected->p2 = perspective << KF_TMD_DEFAULT_PERSPECTIVE_SHIFT;
        ReadSZ2(&depth, &unused_depth);
        projected->sz = depth;
        projected++;
        vertex++;
    }
}

ADDRESS(0x8001c6a8, 0xac)
void tmd_project_vertices_shift(s32 count, u8 shift)
{
    KfScreenVertex *projected;
    KfPackedSVector *vertex;
    long perspective;
    long gte_flags;
    long depth;
    long unused_depth;

    projected = ((KfScreenVertex *)game_graphics_runtime.unknown_projection_morph_20318);
    vertex = game_graphics_runtime.current_tmd_vertices;
    for (count--; count != -1; count--) {
        RotTransPers(&vertex->vector, &projected->sxy.word, &perspective, &gte_flags);
        projected->p2 = perspective << KF_TMD_DEFAULT_PERSPECTIVE_SHIFT;
        ReadSZ2(&depth, &unused_depth);
        projected->sz = depth >> shift;
        projected++;
        vertex++;
    }
}

ADDRESS(0x8001c754, 0xa4)
void tmd_transform_vertices(s32 count)
{
    KfScreenVertex *projected;
    KfPackedSVector *vertex;
    VECTOR transformed;
    long gte_flags;
    s32 remaining;

    projected = ((KfScreenVertex *)game_graphics_runtime.unknown_projection_morph_20318);
    vertex = game_graphics_runtime.current_tmd_vertices;
    for (remaining = count - 1; remaining != -1; remaining--) {
        RotTrans(&vertex->vector, &transformed, &gte_flags);
        projected->sxy.vector.vx = transformed.vx;
        projected->sxy.vector.vy = transformed.vy;
        projected->p2 = transformed.vz;
        projected->sz = transformed.vz;
        projected++;
        vertex++;
    }
}
