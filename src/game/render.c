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
#define TMD_OBJECTS(asset) ((KfTmdObject *)((u8 *)(asset) + KF_TMD_HEADER_BYTES))
/* Packet bodies follow the 4-byte packet header (olen, ilen, flag, mode). */
#define TMD_PACKET_BODY(packet) ((u8 *)(packet) + KF_TMD_PACKET_HEADER_BYTES)

/* tmd_register primitive-mode dispatch table. */
RODATA(0x800121b4, 0x74)

/* System-message TIM path; byte 2 selects an error or pause screen. */
DATA(0x80057b50, 0x7)
char error_screen_path[7] = "\\E0.;1";

DATA(0x80090ec0, 0xb8)
DRAWENV display_draw_environments[KF_DISPLAY_BUFFER_COUNT];

DATA(0x80090f78, 0x28)
DISPENV display_disp_environments[KF_DISPLAY_BUFFER_COUNT];

/* Loads and shows the system-message screen for STAGE as a semi-transparent
 * textured box, then blocks until a controller button is pressed and released.
 * Called by cd_file_load_into on disc failure and by game_main_loop on a save
 * error. */
ADDRESS(0x8001b7b0, 0x308)
void display_show_error_screen(s32 stage)
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
    cd_path_buffer[SYSTEM_SCREEN_PATH_DIGIT] = stage + '0';
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
               display_state.asset_load_buffer, CdlModeSpeed);
        while ((result = CdReadSync(KF_CD_READ_POLL, 0)) > 0) {
        }
        if (result == 0) {
            attempt = KF_CD_READ_STOP_ATTEMPT;
        }
    }
    tim_upload_images(display_state.asset_load_buffer);

    back = display_state.buffer_index == 0;
    display_draw_environments[back].isbg = 0;
    display_draw_environments[back].dfe = 0;
    PutDrawEnv(&display_draw_environments[back]);
    display_state.ordering_table = display_state.ordering_tables[back].entries;
    setRGB0(&prim, brightness, brightness, brightness);
    ClearOTagR(display_state.ordering_table, KF_ORDERING_TABLE_LENGTH);
    AddPrim(display_state.ordering_table, &prim);
    DrawSync(0);
    DrawOTag(display_state.ordering_table + (KF_ORDERING_TABLE_LENGTH - 1));
    while (PadRead(1) == 0) {
    }
    while (PadRead(1) != 0) {
    }
    display_draw_environments[back].isbg = 1;
    display_draw_environments[back].dfe = 1;
    DrawSync(0);
}

ADDRESS(0x8001bab8, 0x2c)
void lighting_set_active_color_matrix(s32 index)
{
    SetColorMatrix(&color_matrix_table[index]);
}

ADDRESS(0x8001bae4, 0xb0)
void effect5_texture_cache_prepare(KfFloorId floor)
{
    if (floor == KF_FLOOR_5) {
        effect5_texture_pages[0] = GetTPage(
            KF_GPU_TEXTURE_8BIT, KF_GPU_BLEND_AVERAGE,
            EFFECT_TEXTURE_FIRST_PAGE_X, KF_TEXTURE_LOWER_PAGE_Y);
        effect5_texture_pages[1] = GetTPage(
            KF_GPU_TEXTURE_8BIT, KF_GPU_BLEND_AVERAGE,
            EFFECT_TEXTURE_SECOND_PAGE_X, KF_TEXTURE_LOWER_PAGE_Y);
        effect5_texture_pages[2] = GetTPage(
            KF_GPU_TEXTURE_8BIT, KF_GPU_BLEND_AVERAGE,
            EFFECT_TEXTURE_THIRD_PAGE_X, KF_TEXTURE_LOWER_PAGE_Y);
        effect5_texture_cluts[0] = GetClut(0, EFFECT_TEXTURE_CLUT_Y);
        effect5_texture_cluts[1] = GetClut(0, EFFECT_TEXTURE_CLUT_Y);
        effect5_texture_cluts[2] = GetClut(0, EFFECT_TEXTURE_CLUT_Y);
    }
}

ADDRESS(0x8001bb94, 0x14c)
void display_initialize(void)
{
    ResetGraph(KF_GPU_RESET_KEEP_DISPLAY);
    InitGeom();
    SetGeomOffset(KF_DISPLAY_WIDTH / 2, KF_DISPLAY_HEIGHT / 2);
    SetDefDrawEnv(
        &display_draw_environments[0], 0, 0,
        KF_DISPLAY_WIDTH, KF_DISPLAY_HEIGHT);
    SetDefDispEnv(
        &display_disp_environments[0], 0, KF_DISPLAY_HEIGHT,
        KF_DISPLAY_WIDTH, KF_DISPLAY_HEIGHT);
    SetDefDrawEnv(
        &display_draw_environments[1], 0, KF_DISPLAY_HEIGHT,
        KF_DISPLAY_WIDTH, KF_DISPLAY_HEIGHT);
    SetDefDispEnv(
        &display_disp_environments[1], 0, 0,
        KF_DISPLAY_WIDTH, KF_DISPLAY_HEIGHT);
    display_draw_environments[0].dtd = display_draw_environments[1].dtd = 1;
    display_draw_environments[0].isbg = 1;
    display_draw_environments[1].isbg = 1;
    display_draw_environments[0].r0 = 0;
    display_draw_environments[0].g0 = 0;
    display_draw_environments[0].b0 = 0;
    display_draw_environments[1].r0 = 0;
    display_draw_environments[1].g0 = 0;
    display_draw_environments[1].b0 = 0;
    PutDispEnv(&display_disp_environments[0]);
    SetBackColor(INITIAL_BACK_COLOR, INITIAL_BACK_COLOR, INITIAL_BACK_COLOR);
    lighting_set_active_color_matrix(KF_GAME_COLOR_DEFAULT);
    SetFarColor(0, 0, 0);
    render_state.fog_near_distance = KF_INITIAL_FOG_NEAR_DISTANCE;
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

    display_state.buffer_index = KF_DISPLAY_BUFFER_UNINITIALIZED;
    buffer = memory_allocate(KF_DISPLAY_BUFFER_COUNT * PRIMITIVE_BUFFER_BYTES);
    display_state.asset_load_buffer = buffer;
    display_state.primitive_buffers[0].start = buffer;
    buffer += PRIMITIVE_BUFFER_BYTES;
    display_state.primitive_buffers[0].end = buffer;
    display_state.primitive_buffers[1].start = buffer;
    buffer += PRIMITIVE_BUFFER_BYTES;
    display_state.primitive_buffers[1].end = buffer;
    floor_item_count = 0;
    angles.vx = 0;
    angles.vy = 0;
    angles.vz = 0;
    RotMatrix(&angles, &render_state.quadrant_matrices[0]);
    angles.vy = KF_ANGLE_THREE_QUARTER_TURN;
    RotMatrix(&angles, &render_state.quadrant_matrices[3]);
    angles.vy = KF_ANGLE_HALF_TURN;
    RotMatrix(&angles, &render_state.quadrant_matrices[2]);
    angles.vy = KF_ANGLE_QUARTER_TURN;
    RotMatrix(&angles, &render_state.quadrant_matrices[1]);
    render_state.light_matrix.m[0][0] = 3800;
    render_state.light_matrix.m[0][1] = -2800;
    render_state.light_matrix.m[0][2] = 0;
    render_state.light_matrix.m[1][0] = -3000;
    render_state.light_matrix.m[1][1] = -3600;
    render_state.light_matrix.m[1][2] = -3400;
    render_state.light_matrix.m[2][0] = -1300;
    render_state.light_matrix.m[2][1] = 2700;
    render_state.light_matrix.m[2][2] = 800;
    render_state.light_matrix_copy = render_state.light_matrix;
    MulMatrix0(
        &render_state.light_matrix,
        &render_state.quadrant_matrices[0],
        &light_quadrant_matrices[0]);
    MulMatrix0(
        &render_state.light_matrix,
        &render_state.quadrant_matrices[1],
        &light_quadrant_matrices[1]);
    MulMatrix0(
        &render_state.light_matrix,
        &render_state.quadrant_matrices[2],
        &light_quadrant_matrices[2]);
    MulMatrix0(
        &render_state.light_matrix,
        &render_state.quadrant_matrices[3],
        &light_quadrant_matrices[3]);
    floor_item_tpage = GetTPage(
        KF_GPU_TEXTURE_8BIT, KF_GPU_BLEND_AVERAGE,
        FLOOR_ITEM_TPAGE_X, 0);
    floor_item_clut = FLOOR_ITEM_CLUT;
    hud_tpage = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        HUD_TPAGE_X, KF_TEXTURE_LOWER_PAGE_Y);
    hud_clut = GetClut(hud_palette_rect.x, hud_palette_rect.y);
    notification_text_tpage = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        NOTIFICATION_TPAGE_X, KF_TEXTURE_LOWER_PAGE_Y);
    notification_digit_clut = notification_text_clut =
        GetClut(notification_palette_rect.x, notification_palette_rect.y);
    notification_digit_tpage = NOTIFICATION_DIGIT_TPAGE;
    notification_state.control.effect_phase = KF_NOTIFICATION_IDLE;
    notification_state.control.queue_tail = 0;
    notification_state.control.queue_head = 0;
    flag = notification_message_ids;
    count = KF_NOTIFICATION_CAPACITY - 1;
    do {
        *flag++ = KF_NOTIFICATION_NONE;
    } while (count-- != 0);
    pool_reset();
}

ADDRESS(0x8001bfb8, 0x98)
void display_begin_frame(void)
{
    display_state.buffer_index = display_state.buffer_index == 0;
    display_state.primitive_buffer = &display_state.primitive_buffers[display_state.buffer_index];
    display_state.ordering_table =
        display_state.ordering_tables[display_state.buffer_index].entries;
    ClearOTagR(display_state.ordering_table, KF_ORDERING_TABLE_LENGTH);
    display_state.primitive_buffer->cursor = display_state.primitive_buffer->start;
    DAT_800a0768 = 0;
    DAT_8009569c = 0;
    DAT_80095698 = 0;
}

ADDRESS(0x8001c050, 0x98)
void display_present_frame(void)
{
    DrawSync(0);
    VSync(0);
    PutDrawEnv(&display_draw_environments[display_state.buffer_index]);
    PutDispEnv(&display_disp_environments[display_state.buffer_index]);
    DrawOTag(display_state.ordering_table + (KF_ORDERING_TABLE_LENGTH - 1));
}

ADDRESS(0x8001c0e8, 0x2c)
void tmd_select(u16 index)
{
    tmd_state.current_asset = tmd_state.slots[index];
}

ADDRESS(0x8001c114, 0x24)
KfTmdObject *tmd_get_object(u16 index)
{
    return TMD_OBJECTS(tmd_state.current_asset) + index;
}

ADDRESS(0x8001c138, 0x10)
void tmd_set_current_vertices(SVECTOR *vertices)
{
    current_tmd_vertices = vertices;
}

ADDRESS(0x8001c148, 0x3c)
void tmd_select_object_vertices(u16 index)
{
    current_tmd_vertices =
        (SVECTOR *)((u8 *)tmd_state.current_asset
            + KF_TMD_HEADER_BYTES + tmd_get_object(index)->vertex_offset);
}

ADDRESS(0x8001c184, 0x12c)
void render_set_view_transform(
    const VECTOR *position, const SVECTOR *rotation)
{
    SVECTOR angles;

    if (position != 0) {
        render_state.view_position = *position;
        render_state.view_cell.x = render_state.view_position.vx / KF_MAP_TILE_SIZE;
        render_state.view_cell.z = render_state.view_position.vz / KF_MAP_TILE_SIZE;
    }
    if (rotation != 0) {
        render_state.view_rotation = *rotation;
    }
    RotMatrix(&render_state.view_rotation, &render_state.view_matrix);
    angles.vz = 0;
    angles.vy = 0;
    angles.vx = render_state.view_rotation.vx;
    RotMatrix(&angles, &render_state.pitch_matrix);
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
    u16 object_count;
    u16 objects_left;
    u16 primitive_count;
    u16 primitives_left;
    u32 word;

    object_count = (u16)((KfTmdHeader *)tmd_state.current_asset)->object_count;
    objects_left = object_count - 1;
    object = TMD_OBJECTS(tmd_state.current_asset);
    if (object_count == 0) {
        return;
    }
    do {
        primitive_count = (u16)object->primitive_count;
        packet = (u8 *)tmd_state.current_asset + (object->primitive_offset + KF_TMD_HEADER_BYTES);
        primitives_left = primitive_count;
        primitives_left--;
        if (primitive_count != 0) {
            do {
                body = TMD_PACKET_BODY(packet);
                word = *(u32 *)packet;
                packet = body + packet[KF_TMD_ILEN_BYTE] * KF_TMD_WORD_BYTES;
                switch ((word >> KF_TMD_MODE_SHIFT) & KF_TMD_MODE_MASK) {
                case KF_TMD_MODE_F3: {
                    KfTmdF3 *p = (KfTmdF3 *)body;
                    p->v0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->n0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    break;
                }
                case KF_TMD_MODE_G3: {
                    KfTmdG3 *p = (KfTmdG3 *)body;
                    p->v0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->n0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->n1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->n2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    break;
                }
                case KF_TMD_MODE_FT3: {
                    KfTmdFt3 *p = (KfTmdFt3 *)body;
                    p->v0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->n0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    break;
                }
                case KF_TMD_MODE_GT3: {
                    KfTmdGt3 *p = (KfTmdGt3 *)body;
                    p->v0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->n0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->n1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->n2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    break;
                }
                case KF_TMD_MODE_F4: {
                    KfTmdF4 *p = (KfTmdF4 *)body;
                    p->v0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v3 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->n0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    break;
                }
                case KF_TMD_MODE_G4: {
                    KfTmdG4 *p = (KfTmdG4 *)body;
                    p->v0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v3 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->n0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->n1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->n2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->n3 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    break;
                }
                case KF_TMD_MODE_FT4: {
                    KfTmdFt4 *p = (KfTmdFt4 *)body;
                    p->v0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v3 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->n0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    break;
                }
                case KF_TMD_MODE_GT4: {
                    KfTmdGt4 *p = (KfTmdGt4 *)body;
                    p->v0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->v3 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->n0 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->n1 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->n2 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    p->n3 <<= KF_TMD_VECTOR_OFFSET_SHIFT;
                    break;
                }
                }
            } while (primitives_left-- != 0);
        }
        object++;
    } while (objects_left-- != 0);
}

ADDRESS(0x8001c5b0, 0x3c)
void tmd_register(u16 slot, u8 *tmd)
{
    tmd_state.current_asset = tmd_state.slots[slot] = tmd;
    tmd_prepare_primitive_indices();
}

ADDRESS(0x8001c5ec, 0x20)
void tmd_release_last_allocation(s32 slot)
{
    memory_release_last();
}

/* Store the full GTE depth; RotTransPers returns depth divided by four. */
ADDRESS(0x8001c60c, 0x9c)
void tmd_project_vertices(s32 count)
{
    KfScreenVertex *projected;
    SVECTOR *vertex;
    long perspective;
    long gte_flags;
    long depth;
    long unused_depth;

    projected = tmd_projected_vertices;
    vertex = current_tmd_vertices;
    for (count--; count != -1; count--) {
        RotTransPers(vertex, &projected->sxy, &perspective, &gte_flags);
        projected->p2 = (u16)perspective << KF_TMD_DEFAULT_PERSPECTIVE_SHIFT;
        ReadSZ2(&depth, &unused_depth);
        projected->sz = (u16)depth;
        projected++;
        vertex++;
    }
}

ADDRESS(0x8001c6a8, 0xac)
void tmd_project_vertices_shift(s32 count, u8 shift)
{
    KfScreenVertex *projected;
    SVECTOR *vertex;
    long perspective;
    long gte_flags;
    long depth;
    long unused_depth;

    projected = tmd_projected_vertices;
    vertex = current_tmd_vertices;
    for (count--; count != -1; count--) {
        RotTransPers(vertex, &projected->sxy, &perspective, &gte_flags);
        projected->p2 = (u16)perspective << KF_TMD_DEFAULT_PERSPECTIVE_SHIFT;
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
    SVECTOR *vertex;
    VECTOR transformed;
    long gte_flags;
    s32 remaining;

    projected = tmd_projected_vertices;
    vertex = current_tmd_vertices;
    for (remaining = count - 1; remaining != -1; remaining--) {
        RotTrans(vertex, &transformed, &gte_flags);
        ((DVECTOR *)&projected->sxy)->vx = transformed.vx;
        ((DVECTOR *)&projected->sxy)->vy = transformed.vy;
        projected->p2 = transformed.vz;
        projected->sz = transformed.vz;
        projected++;
        vertex++;
    }
}
