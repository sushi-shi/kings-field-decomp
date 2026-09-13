#include <kf/null.h>

#include <kf/map_data.h>
#include <kf/memory.h>
#include <kf/debug.h>
#include <kf/open_render.h>
#include <kf/tmd.h>

void display_begin_frame(void)
{
    open_graphics_runtime.display_state.buffer_index = display_next_buffer(open_graphics_runtime.display_state.buffer_index);
    open_graphics_runtime.display_state.primitive_buffer = &open_graphics_runtime.display_state.primitive_buffers[kf_enum_encode<u8>(open_graphics_runtime.display_state.buffer_index)];
    open_graphics_runtime.ordering_table = open_graphics_runtime.display_state.ordering_tables[kf_enum_encode<u8>(open_graphics_runtime.display_state.buffer_index)].entries;
    ClearOTagR(open_graphics_runtime.ordering_table, KF_ORDERING_TABLE_LENGTH);
    open_graphics_runtime.display_state.primitive_buffer->cursor = open_graphics_runtime.display_state.primitive_buffer->start;
    primitive_allocation_count = 0;
    open_graphics_runtime.DAT_8006e044 = 0;
    open_graphics_runtime.DAT_8006e040 = 0;
}

void display_present_frame(void)
{
    DrawSync(0);
    VSync(0);
    PutDrawEnv(&open_graphics_runtime.display_draw_environments[kf_enum_encode<u8>(open_graphics_runtime.display_state.buffer_index)]);
    PutDispEnv(&open_graphics_runtime.display_disp_environments[kf_enum_encode<u8>(open_graphics_runtime.display_state.buffer_index)]);
    DrawOTag(open_graphics_runtime.ordering_table + (KF_ORDERING_TABLE_LENGTH - 1));
}

void tmd_select(KfTmdSlot slot)
{
    open_graphics_runtime.tmd_state.current_asset = open_graphics_runtime.tmd_state.slots[kf_enum_encode<u16>(slot)];
}

KfTmdObject *tmd_get_object(u16 index)
{
    return TMD_OBJECTS(open_graphics_runtime.tmd_state.current_asset) + index;
}

void tmd_set_current_vertices(SVECTOR *vertices)
{
    open_graphics_runtime.current_tmd_vertices = vertices;
}

void tmd_select_object_vertices(u16 index)
{
    open_graphics_runtime.current_tmd_vertices =
        TMD_OBJECT_VERTICES(open_graphics_runtime.tmd_state.current_asset,
            tmd_get_object(index));
}

void render_set_view_transform(
    const VECTOR *position_or_null, const SVECTOR *rotation_or_null)
{
    SVECTOR angles;

    if (position_or_null != NULL) {
        open_graphics_runtime.render_state.view_position = *position_or_null;
        open_graphics_runtime.render_state.view_cell.x = open_graphics_runtime.render_state.view_position.vx / KF_MAP_TILE_SIZE;
        open_graphics_runtime.render_state.view_cell.z = open_graphics_runtime.render_state.view_position.vz / KF_MAP_TILE_SIZE;
    }
    if (rotation_or_null != NULL) {
        open_graphics_runtime.render_state.view_rotation = *rotation_or_null;
    }
    RotMatrix(&open_graphics_runtime.render_state.view_rotation, &open_graphics_runtime.render_state.view_matrix);
    angles.vz = 0;
    angles.vy = 0;
    angles.vx = open_graphics_runtime.render_state.view_rotation.vx;
    RotMatrix(&angles, &open_graphics_runtime.render_state.pitch_matrix);
}

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

    object_count = open_graphics_runtime.tmd_state.current_asset->object_count;
    objects_left = object_count - 1;
    object = TMD_OBJECTS(open_graphics_runtime.tmd_state.current_asset);
    if (object_count == 0) {
        return;
    }
    do {
        primitive_count = object->primitive_count;
        packet = (u8 *)open_graphics_runtime.tmd_state.current_asset +
            (object->primitive_offset + KF_TMD_HEADER_BYTES);
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
    } while (objects_left--);
}

void tmd_register(KfTmdSlot slot, KfTmdHeader *tmd)
{
    open_graphics_runtime.tmd_state.current_asset = open_graphics_runtime.tmd_state.slots[kf_enum_encode<u16>(slot)] = tmd;
    tmd_prepare_primitive_indices();
}

void tmd_release_last_allocation(KfTmdSlot slot)
{
    memory_release_last();
}

void tmd_project_vertices(s32 count)
{
    KfScreenVertex *out;
    SVECTOR *vertex;
    long perspective;
    long flag;
    long depth;
    long unused_depth;

    if (count >= KF_PROJECTED_VERTEX_CAPACITY) {
        debug_printf_sink("POINT OVER !!!!!!\n");
        return;
    }
    out = open_graphics_runtime.tmd_projected_vertices;
    vertex = open_graphics_runtime.current_tmd_vertices;
    for (count--; count != -1; count--) {
        RotTransPers(vertex, &out->sxy.word, &perspective, &flag);
        out->p2 = perspective << open_graphics_runtime.tmd_projection_shift;
        ReadSZ2(&depth, &unused_depth);
        out->sz = depth;
        out++;
        vertex++;
    }
}

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
        RotTransPers(vertex, &out->sxy.word, &perspective, &flag);
        out->p2 = perspective >> open_graphics_runtime.tmd_projection_shift;
        ReadSZ2(&depth, &unused_depth);
        out->sz = depth;
        out++;
        vertex++;
    }
}

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
        RotTransPers(vertex, &out->sxy.word, &perspective, &flag);
        out->p2 = perspective << KF_TMD_DEFAULT_PERSPECTIVE_SHIFT;
        ReadSZ2(&depth, &unused_depth);
        out->sz = depth >> shift;
        out++;
        vertex++;
    }
}

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
        out->sxy.vector.vx = position.vx;
        out->sxy.vector.vy = position.vy;
        out->p2 = position.vz;
        out->sz = position.vz;
        out++;
        vertex++;
    }
}
