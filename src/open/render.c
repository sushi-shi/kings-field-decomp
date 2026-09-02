#include <kf/address.h>
#include <kf/semantic_types.h>

/*
 * OPEN.EXE copy of the render translation unit (continued from
 * src/open/render_init.c).  The bodies match src/game/render.c; only the
 * state layouts and the separate ordering-table pointer differ.
 */

extern KfDisplayStateOpen display_state;
extern KfRenderStateOpen render_state;
extern KfTmdStateOpen tmd_state;
extern DRAWENV display_draw_environments[2];
extern DISPENV display_disp_environments[2];
extern u32 *ordering_table;
extern struct KfVec4s *current_tmd_vertices;
extern u32 DAT_80075928;
extern u32 DAT_8006e044;
extern u32 DAT_8006e040;
extern void memory_release_last(void);

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

ADDRESS(0x80016d38, 0x98)
void display_begin_frame(void)
{
    display_state.buffer_index = display_state.buffer_index == 0;
    display_state.primitive_buffer = &display_state.primitive_buffers[display_state.buffer_index];
    ordering_table = display_state.ordering_tables[display_state.buffer_index].entries;
    ClearOTagR(ordering_table, 0x4000);
    display_state.primitive_buffer->cursor = display_state.primitive_buffer->start;
    DAT_80075928 = 0;
    DAT_8006e044 = 0;
    DAT_8006e040 = 0;
}

ADDRESS(0x80016dd0, 0x98)
void display_present_frame(void)
{
    DrawSync(0);
    VSync(0);
    PutDrawEnv(&display_draw_environments[display_state.buffer_index]);
    PutDispEnv(&display_disp_environments[display_state.buffer_index]);
    DrawOTag(ordering_table + 0x3fff);
}

ADDRESS(0x80016e68, 0x2c)
void tmd_select(u16 index)
{
    tmd_state.current_asset = tmd_state.slots[index];
}

ADDRESS(0x80016e94, 0x24)
KfTmdObject *tmd_get_object(u16 index)
{
    return TMD_OBJECTS(tmd_state.current_asset) + index;
}

ADDRESS(0x80016eb8, 0x10)
void tmd_set_current_vertices(struct KfVec4s *vertices)
{
    current_tmd_vertices = vertices;
}

ADDRESS(0x80016ec8, 0x3c)
void tmd_select_object_vertices(u16 index)
{
    current_tmd_vertices =
        (struct KfVec4s *)((u8 *)tmd_state.current_asset + 12 + tmd_get_object(index)->vertex_offset);
}

ADDRESS(0x80016f04, 0x12c)
void render_set_view_transform(struct KfVec4i *position, struct KfVec4s *rotation)
{
    SVECTOR angles;

    if (position != 0) {
        render_state.view_position = *position;
        render_state.view_cell.x = render_state.view_position.x / 2000;
        render_state.view_cell.z = render_state.view_position.z / 2000;
    }
    if (rotation != 0) {
        render_state.view_rotation = *rotation;
        RotMatrix((SVECTOR *)&render_state.view_rotation, (MATRIX *)&render_state.view_matrix);
        angles.vz = 0;
        angles.vy = 0;
    }
    angles.vx = render_state.view_rotation.x;
    RotMatrix(&angles, (MATRIX *)&render_state.pitch_matrix);
}

RODATA(0x80012138, 0x74)

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

ADDRESS(0x80017330, 0x3c)
void tmd_register(u16 index, void *asset)
{
    tmd_state.current_asset = tmd_state.slots[index] = asset;
    tmd_prepare_primitive_indices();
}

ADDRESS(0x8001736c, 0x20)
void tmd_release_last_allocation(void)
{
    memory_release_last();
}
