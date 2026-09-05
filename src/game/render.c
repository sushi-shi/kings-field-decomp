#include <kf/address.h>
#include <kf/game_cd.h>
#include <kf/game_render.h>
#include <kf/notify.h>
#include <kf/psyq.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>
#include <kf/tmd.h>

#define CD_SECTOR_SIZE 0x800
#define CD_SECTOR_SHIFT 11

/* Object-table records follow the 12-byte TMD header of the selected asset. */
#define TMD_OBJECTS(asset) ((KfTmdObject *)((u8 *)(asset) + 12))
/* Packet bodies follow the 4-byte packet header (olen, ilen, flag, mode). */
#define TMD_PACKET_BODY(packet) ((u8 *)(packet) + 4)

#define TMD_F3 0x20
#define TMD_FT3 0x24
#define TMD_F4 0x28
#define TMD_FT4 0x2c
#define TMD_G3 0x30
#define TMD_GT3 0x34
#define TMD_G4 0x38
#define TMD_GT4 0x3c

/* tmd_register primitive-mode dispatch table. */
RODATA(0x800121b4, 0x74)

/* Disc file of the fatal-error message screen; the index-2 digit selects the
 * stage: 0 failed disc search, 1 failed disc read, 2 save-file error. */
DATA(0x80057b50, 0x7)
char error_screen_path[7] = "\\E0.;1";

/* Loads and shows the fatal-error message screen for STAGE as a semi-transparent
 * textured box, then blocks until a controller button is pressed and released.
 * Called by cd_file_load_into on disc failure and by game_main_loop on a save
 * error. */
ADDRESS(0x8001b7b0, 0x308)
void display_show_error_screen(s32 stage)
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
void effect5_texture_cache_prepare(s32 mode)
{
    if (mode == 5) {
        effect5_texture_pages[0] = GetTPage(1, 0, 0x140, 0x100);
        effect5_texture_pages[1] = GetTPage(1, 0, 0x180, 0x100);
        effect5_texture_pages[2] = GetTPage(1, 0, 0x340, 0x100);
        effect5_texture_cluts[0] = GetClut(0, 0x1eb);
        effect5_texture_cluts[1] = GetClut(0, 0x1eb);
        effect5_texture_cluts[2] = GetClut(0, 0x1eb);
    }
}

ADDRESS(0x8001bb94, 0x14c)
void display_initialize(void)
{
    ResetGraph(3);
    InitGeom();
    SetGeomOffset(160, 120);
    SetDefDrawEnv(&display_draw_environments[0], 0, 0, 320, 240);
    SetDefDispEnv(&display_disp_environments[0], 0, 240, 320, 240);
    SetDefDrawEnv(&display_draw_environments[1], 0, 240, 320, 240);
    SetDefDispEnv(&display_disp_environments[1], 0, 0, 320, 240);
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
    SetBackColor(60, 60, 60);
    lighting_set_active_color_matrix(0);
    SetFarColor(0, 0, 0);
    render_state.fog_near_distance = 0x2af8;
    SetFogNear(0x2af8, 200);
    render_initialize();
}

ADDRESS(0x8001bce0, 0x2d8)
void render_initialize(void)
{
    SVECTOR angles;
    u8 *flag;
    u8 count;
    u8 *buffer;

    display_state.buffer_index = 0xff;
    buffer = memory_allocate(0x32c80);
    display_state.asset_load_buffer = buffer;
    display_state.primitive_buffers[0].start = buffer;
    buffer += 0x19640;
    display_state.primitive_buffers[0].end = buffer;
    display_state.primitive_buffers[1].start = buffer;
    buffer += 0x19640;
    display_state.primitive_buffers[1].end = buffer;
    floor_item_count = 0;
    angles.vx = 0;
    angles.vy = 0;
    angles.vz = 0;
    RotMatrix(&angles, &render_state.quadrant_matrices[0]);
    angles.vy = 0xc00;
    RotMatrix(&angles, &render_state.quadrant_matrices[3]);
    angles.vy = 0x800;
    RotMatrix(&angles, &render_state.quadrant_matrices[2]);
    angles.vy = 0x400;
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
    DAT_8009508e = GetTPage(1, 0, 0x380, 0);
    DAT_8009508c = 0x7a40;
    DAT_80095062 = GetTPage(0, 0, 0x380, 0x100);
    DAT_80095060 = GetClut(DAT_80055dac, DAT_80055dae);
    DAT_80095068 = GetTPage(0, 0, 0x340, 0x100);
    DAT_8009506a = DAT_80095066 = GetClut(DAT_80055db4, DAT_80055db6);
    DAT_8009506c = 0x1c;
    notification_state.effect_phase = 0;
    notification_state.queue_tail = 0;
    notification_state.queue_head = 0;
    flag = notification_message_ids;
    count = 7;
    do {
        *flag++ = 0xff;
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
    ClearOTagR(display_state.ordering_table, 0x4000);
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
    DrawOTag(display_state.ordering_table + 0x3fff);
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
        (SVECTOR *)((u8 *)tmd_state.current_asset + 12 + tmd_get_object(index)->vertex_offset);
}

ADDRESS(0x8001c184, 0x12c)
void render_set_view_transform(
    const VECTOR *position, const SVECTOR *rotation)
{
    SVECTOR angles;

    if (position != 0) {
        render_state.view_position = *position;
        render_state.view_cell.x = render_state.view_position.vx / 2000;
        render_state.view_cell.z = render_state.view_position.vz / 2000;
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
        packet = (u8 *)tmd_state.current_asset + (object->primitive_offset + 12);
        if (primitive_count != 0) {
            primitives_left = primitive_count;
            primitives_left--;
            do {
                body = TMD_PACKET_BODY(packet);
                word = *(u32 *)packet;
                packet = body + packet[1] * 4;
            switch ((word >> 24) & 0xfd) {
                case TMD_F3: {
                    KfTmdF3 *p = (KfTmdF3 *)body;
                    p->v0 <<= 3;
                    p->v1 <<= 3;
                    p->v2 <<= 3;
                    p->n0 <<= 3;
                    break;
                }
                case TMD_G3: {
                    KfTmdG3 *p = (KfTmdG3 *)body;
                    p->v0 <<= 3;
                    p->v1 <<= 3;
                    p->v2 <<= 3;
                    p->n0 <<= 3;
                    p->n1 <<= 3;
                    p->n2 <<= 3;
                    break;
                }
                case TMD_FT3: {
                    KfTmdFt3 *p = (KfTmdFt3 *)body;
                    p->v0 <<= 3;
                    p->v1 <<= 3;
                    p->v2 <<= 3;
                    p->n0 <<= 3;
                    break;
                }
                case TMD_GT3: {
                    KfTmdGt3 *p = (KfTmdGt3 *)body;
                    p->v0 <<= 3;
                    p->v1 <<= 3;
                    p->v2 <<= 3;
                    p->n0 <<= 3;
                    p->n1 <<= 3;
                    p->n2 <<= 3;
                    break;
                }
                case TMD_F4: {
                    KfTmdF4 *p = (KfTmdF4 *)body;
                    p->v0 <<= 3;
                    p->v1 <<= 3;
                    p->v2 <<= 3;
                    p->v3 <<= 3;
                    p->n0 <<= 3;
                    break;
                }
                case TMD_G4: {
                    KfTmdG4 *p = (KfTmdG4 *)body;
                    p->v0 <<= 3;
                    p->v1 <<= 3;
                    p->v2 <<= 3;
                    p->v3 <<= 3;
                    p->n0 <<= 3;
                    p->n1 <<= 3;
                    p->n2 <<= 3;
                    p->n3 <<= 3;
                    break;
                }
                case TMD_FT4: {
                    KfTmdFt4 *p = (KfTmdFt4 *)body;
                    p->v0 <<= 3;
                    p->v1 <<= 3;
                    p->v2 <<= 3;
                    p->v3 <<= 3;
                    p->n0 <<= 3;
                    break;
                }
                case TMD_GT4: {
                    KfTmdGt4 *p = (KfTmdGt4 *)body;
                    p->v0 <<= 3;
                    p->v1 <<= 3;
                    p->v2 <<= 3;
                    p->v3 <<= 3;
                    p->n0 <<= 3;
                    p->n1 <<= 3;
                    p->n2 <<= 3;
                    p->n3 <<= 3;
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
        projected->p2 = (u16)perspective << 1;
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
        projected->p2 = (u16)perspective << 1;
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
