#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/memory.h>
#include <kf/debug.h>
#include <kf/open_render.h>
#include <kf/tmd.h>

/*
 * OPEN.EXE display, TMD, and projection bodies following render initialization.
 * Shared instruction shapes with the GAME renderer support common source
 * lineage; OPEN-specific state layouts and call paths remain explicit.
 */

/* Object-table records follow the 12-byte TMD header of the selected asset. */
#define TMD_OBJECTS(asset) ((KfTmdObject *)((u8 *)(asset) + 12))
/* Packet bodies follow the 4-byte packet header (olen, ilen, flag, mode). */
#define TMD_PACKET_BODY(packet) ((packet) + 4)

#define TMD_F3 0x20
#define TMD_FT3 0x24
#define TMD_F4 0x28
#define TMD_FT4 0x2c
#define TMD_G3 0x30
#define TMD_GT3 0x34
#define TMD_G4 0x38
#define TMD_GT4 0x3c

ADDRESS(0x80016d38, 0x98)
void display_begin_frame(void)
{
    open_graphics_runtime.display_state.buffer_index = open_graphics_runtime.display_state.buffer_index == 0;
    open_graphics_runtime.display_state.primitive_buffer = &open_graphics_runtime.display_state.primitive_buffers[open_graphics_runtime.display_state.buffer_index];
    open_graphics_runtime.ordering_table = open_graphics_runtime.display_state.ordering_tables[open_graphics_runtime.display_state.buffer_index].entries;
    ClearOTagR(open_graphics_runtime.ordering_table, KF_ORDERING_TABLE_LENGTH);
    open_graphics_runtime.display_state.primitive_buffer->cursor = open_graphics_runtime.display_state.primitive_buffer->start;
    primitive_allocation_count = 0;
    open_graphics_runtime.DAT_8006e044 = 0;
    open_graphics_runtime.DAT_8006e040 = 0;
}

ADDRESS(0x80016dd0, 0x98)
void display_present_frame(void)
{
    DrawSync(0);
    VSync(0);
    PutDrawEnv(&open_graphics_runtime.display_draw_environments[open_graphics_runtime.display_state.buffer_index]);
    PutDispEnv(&open_graphics_runtime.display_disp_environments[open_graphics_runtime.display_state.buffer_index]);
    DrawOTag(open_graphics_runtime.ordering_table + (KF_ORDERING_TABLE_LENGTH - 1));
}

ADDRESS(0x80016e68, 0x2c)
void tmd_select(u16 index)
{
    open_graphics_runtime.tmd_state.current_asset = open_graphics_runtime.tmd_state.slots[index];
}

ADDRESS(0x80016e94, 0x24)
KfTmdObject *tmd_get_object(u16 index)
{
    return TMD_OBJECTS(open_graphics_runtime.tmd_state.current_asset) + index;
}

ADDRESS(0x80016eb8, 0x10)
void tmd_set_current_vertices(SVECTOR *vertices)
{
    open_graphics_runtime.current_tmd_vertices = vertices;
}

ADDRESS(0x80016ec8, 0x3c)
void tmd_select_object_vertices(u16 index)
{
    open_graphics_runtime.current_tmd_vertices =
        (SVECTOR *)((u8 *)open_graphics_runtime.tmd_state.current_asset + 12 + tmd_get_object(index)->vertex_offset);
}

ADDRESS(0x80016f04, 0x12c)
void render_set_view_transform(
    const VECTOR *position, const SVECTOR *rotation)
{
    SVECTOR angles;

    if (position != 0) {
        open_graphics_runtime.render_state.view_position = *position;
        open_graphics_runtime.render_state.view_cell.x = open_graphics_runtime.render_state.view_position.vx / KF_MAP_TILE_SIZE;
        open_graphics_runtime.render_state.view_cell.z = open_graphics_runtime.render_state.view_position.vz / KF_MAP_TILE_SIZE;
    }
    if (rotation != 0) {
        open_graphics_runtime.render_state.view_rotation = *rotation;
    }
    RotMatrix(&open_graphics_runtime.render_state.view_rotation, &open_graphics_runtime.render_state.view_matrix);
    angles.vz = 0;
    angles.vy = 0;
    angles.vx = open_graphics_runtime.render_state.view_rotation.vx;
    RotMatrix(&angles, &open_graphics_runtime.render_state.pitch_matrix);
}

RODATA(0x80012138, 0x88)

/*
 * Converts every vertex and normal index of the current asset into a byte
 * offset (8-byte entries) in place.  The ABE bit of the packet mode is ignored.
 */
ADDRESS(0x80017030, 0x300)
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

    object_count = (u16)((KfTmdHeader *)open_graphics_runtime.tmd_state.current_asset)->object_count;
    objects_left = object_count - 1;
    object = TMD_OBJECTS(open_graphics_runtime.tmd_state.current_asset);
    if (object_count == 0) {
        return;
    }
    do {
        primitive_count = (u16)object->primitive_count;
        packet = (u8 *)open_graphics_runtime.tmd_state.current_asset + (object->primitive_offset + 12);
        primitives_left = primitive_count;
        primitives_left--;
        if (primitive_count != 0) {
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

ADDRESS(0x80017330, 0x3c)
void tmd_register(u16 slot, u8 *tmd)
{
    open_graphics_runtime.tmd_state.current_asset = open_graphics_runtime.tmd_state.slots[slot] = tmd;
    tmd_prepare_primitive_indices();
}

ADDRESS(0x8001736c, 0x20)
void tmd_release_last_allocation(s32 slot)
{
    memory_release_last();
}

ADDRESS(0x8001738c, 0xcc)
void tmd_project_vertices(s32 count)
{
    KfScreenVertex *out;
    SVECTOR *vertex;
    long perspective;
    long flag;
    long depth;
    long unused_depth;

    if (count >= KF_OPEN_PROJECTED_VERTEX_CAPACITY) {
        debug_printf_sink("POINT OVER !!!!!!\n");
        return;
    }
    out = open_graphics_runtime.tmd_projected_vertices;
    vertex = open_graphics_runtime.current_tmd_vertices;
    for (count--; count != -1; count--) {
        RotTransPers(vertex, &out->sxy, &perspective, &flag);
        out->p2 = perspective << open_graphics_runtime.tmd_projection_shift;
        ReadSZ2(&depth, &unused_depth);
        out->sz = (u16)depth;
        out++;
        vertex++;
    }
}

ADDRESS(0x80017458, 0xa4)
void tmd_project_vertices_perspective_right(s32 count)
{
    KfScreenVertex *out;
    SVECTOR *vertex;
    long perspective;
    long flag;
    long depth;
    long unused_depth;

    out = open_graphics_runtime.tmd_projected_vertices;
    vertex = open_graphics_runtime.current_tmd_vertices;
    for (count--; count != -1; count--) {
        RotTransPers(vertex, &out->sxy, &perspective, &flag);
        out->p2 = perspective >> open_graphics_runtime.tmd_projection_shift;
        ReadSZ2(&depth, &unused_depth);
        out->sz = (u16)depth;
        out++;
        vertex++;
    }
}

ADDRESS(0x800174fc, 0xac)
void tmd_project_vertices_shift(s32 count, u8 shift)
{
    KfScreenVertex *out;
    SVECTOR *vertex;
    long perspective;
    long flag;
    long depth;
    long unused_depth;

    out = open_graphics_runtime.tmd_projected_vertices;
    vertex = open_graphics_runtime.current_tmd_vertices;
    for (count--; count != -1; count--) {
        RotTransPers(vertex, &out->sxy, &perspective, &flag);
        out->p2 = (u16)perspective << 1;
        ReadSZ2(&depth, &unused_depth);
        out->sz = depth >> shift;
        out++;
        vertex++;
    }
}

ADDRESS(0x800175a8, 0xa4)
void tmd_transform_vertices(s32 count)
{
    KfScreenVertex *out;
    SVECTOR *vertex;
    VECTOR position;
    long flag;

    out = open_graphics_runtime.tmd_projected_vertices;
    vertex = open_graphics_runtime.current_tmd_vertices;
    for (count--; count != -1; count--) {
        RotTrans(vertex, &position, &flag);
        ((DVECTOR *)&out->sxy)->vx = position.vx;
        ((DVECTOR *)&out->sxy)->vy = position.vy;
        out->p2 = position.vz;
        out->sz = position.vz;
        out++;
        vertex++;
    }
}
