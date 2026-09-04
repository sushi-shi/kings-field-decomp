#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

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
    RotMatrix(&angles, (MATRIX *)&render_state.quadrant_matrices[0]);
    angles.vy = 0xc00;
    RotMatrix(&angles, (MATRIX *)&render_state.quadrant_matrices[3]);
    angles.vy = 0x800;
    RotMatrix(&angles, (MATRIX *)&render_state.quadrant_matrices[2]);
    angles.vy = 0x400;
    RotMatrix(&angles, (MATRIX *)&render_state.quadrant_matrices[1]);
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
        (MATRIX *)&render_state.light_matrix,
        (MATRIX *)&render_state.quadrant_matrices[0],
        (MATRIX *)&light_quadrant_matrices[0]);
    MulMatrix0(
        (MATRIX *)&render_state.light_matrix,
        (MATRIX *)&render_state.quadrant_matrices[1],
        (MATRIX *)&light_quadrant_matrices[1]);
    MulMatrix0(
        (MATRIX *)&render_state.light_matrix,
        (MATRIX *)&render_state.quadrant_matrices[2],
        (MATRIX *)&light_quadrant_matrices[2]);
    MulMatrix0(
        (MATRIX *)&render_state.light_matrix,
        (MATRIX *)&render_state.quadrant_matrices[3],
        (MATRIX *)&light_quadrant_matrices[3]);
    DAT_8009508e = GetTPage(1, 0, 0x380, 0);
    DAT_8009508c = 0x7a40;
    DAT_80095062 = GetTPage(0, 0, 0x380, 0x100);
    DAT_80095060 = GetClut(DAT_80055dac, DAT_80055dae);
    DAT_80095068 = GetTPage(0, 0, 0x340, 0x100);
    DAT_8009506a = DAT_80095066 = GetClut(DAT_80055db4, DAT_80055db6);
    DAT_8009506c = 0x1c;
    notification_effect_phase = 0;
    notification_queue_tail = 0;
    notification_queue_head = 0;
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
void render_set_view_transform(VECTOR *position, SVECTOR *rotation)
{
    SVECTOR angles;

    if (position != 0) {
        render_state.view_position = *position;
        render_state.view_cell.x = render_state.view_position.vx / 2000;
        render_state.view_cell.z = render_state.view_position.vz / 2000;
    }
    if (rotation != 0) {
        render_state.view_rotation = *rotation;
        RotMatrix((SVECTOR *)&render_state.view_rotation, (MATRIX *)&render_state.view_matrix);
        angles.vz = 0;
        angles.vy = 0;
    }
    angles.vx = render_state.view_rotation.vx;
    RotMatrix(&angles, (MATRIX *)&render_state.pitch_matrix);
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

    object_count = ((KfTmdHeader *)tmd_state.current_asset)->object_count;
    if (object_count == 0) {
        return;
    }
    object = TMD_OBJECTS(tmd_state.current_asset);
    objects_left = object_count - 1;
    do {
        primitive_count = object->primitive_count;
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
void tmd_register(u16 index, void *asset)
{
    tmd_state.current_asset = tmd_state.slots[index] = asset;
    tmd_prepare_primitive_indices();
}

ADDRESS(0x8001c5ec, 0x20)
void tmd_release_last_allocation(s32 slot)
{
    memory_release_last();
}
