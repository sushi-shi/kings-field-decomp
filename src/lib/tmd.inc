ADDRESS_AT("GAME", 0x8001c0e8, 0x2c)
ADDRESS_AT("OPEN", 0x80016e68, 0x2c)
void tmd_select(KfTmdSlot slot)
{
    KF_GRAPHICS_RUNTIME.tmd_state.current_asset = KF_GRAPHICS_RUNTIME.tmd_state.slots[KF_ENUM_ENCODE(u16, slot)];
}

ADDRESS_AT("GAME", 0x8001c114, 0x24)
ADDRESS_AT("OPEN", 0x80016e94, 0x24)
KfTmdObject *tmd_get_object(u16 index)
{
    return TMD_OBJECTS(KF_GRAPHICS_RUNTIME.tmd_state.current_asset) + index;
}

ADDRESS_AT("GAME", 0x8001c138, 0x10)
ADDRESS_AT("OPEN", 0x80016eb8, 0x10)
void tmd_set_current_vertices(SVECTOR *vertices)
{
    KF_GRAPHICS_RUNTIME.current_tmd_vertices = vertices;
}

ADDRESS_AT("GAME", 0x8001c148, 0x3c)
ADDRESS_AT("OPEN", 0x80016ec8, 0x3c)
void tmd_select_object_vertices(u16 index)
{
    KF_GRAPHICS_RUNTIME.current_tmd_vertices =
        TMD_OBJECT_VERTICES(KF_GRAPHICS_RUNTIME.tmd_state.current_asset,
            tmd_get_object(index));
}

ADDRESS_AT("GAME", 0x8001c184, 0x12c)
ADDRESS_AT("OPEN", 0x80016f04, 0x12c)
void render_set_view_transform(
    const VECTOR *position_or_null, const SVECTOR *rotation_or_null)
{
    SVECTOR angles;

    if (position_or_null != NULL) {
        KF_GRAPHICS_RUNTIME.render_state.view_position = *position_or_null;
        KF_GRAPHICS_RUNTIME.render_state.view_cell.x = KF_GRAPHICS_RUNTIME.render_state.view_position.vx / KF_MAP_TILE_SIZE;
        KF_GRAPHICS_RUNTIME.render_state.view_cell.z = KF_GRAPHICS_RUNTIME.render_state.view_position.vz / KF_MAP_TILE_SIZE;
    }
    if (rotation_or_null != NULL) {
        KF_GRAPHICS_RUNTIME.render_state.view_rotation = *rotation_or_null;
    }
    RotMatrix(&KF_GRAPHICS_RUNTIME.render_state.view_rotation, &KF_GRAPHICS_RUNTIME.render_state.view_matrix);
    angles.vz = 0;
    angles.vy = 0;
    angles.vx = KF_GRAPHICS_RUNTIME.render_state.view_rotation.vx;
    RotMatrix(&angles, &KF_GRAPHICS_RUNTIME.render_state.pitch_matrix);
}

ADDRESS_AT("GAME", 0x8001c2b0, 0x300)
ADDRESS_AT("OPEN", 0x80017030, 0x300)
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

    object_count = KF_GRAPHICS_RUNTIME.tmd_state.current_asset->object_count;
    objects_left = object_count - 1;
    object = TMD_OBJECTS(KF_GRAPHICS_RUNTIME.tmd_state.current_asset);
    if (object_count == 0) {
        return;
    }
    do {
        primitive_count = object->primitive_count;
        packet = (u8 *)KF_GRAPHICS_RUNTIME.tmd_state.current_asset + (object->primitive_offset + KF_TMD_HEADER_BYTES);
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

ADDRESS_AT("GAME", 0x8001c5b0, 0x3c)
ADDRESS_AT("OPEN", 0x80017330, 0x3c)
void tmd_register(KfTmdSlot slot, KfTmdHeader *tmd)
{
    KF_GRAPHICS_RUNTIME.tmd_state.current_asset = KF_GRAPHICS_RUNTIME.tmd_state.slots[KF_ENUM_ENCODE(u16, slot)] = tmd;
    tmd_prepare_primitive_indices();
}

ADDRESS_AT("GAME", 0x8001c5ec, 0x20)
ADDRESS_AT("OPEN", 0x8001736c, 0x20)
void tmd_release_last_allocation(KF_ENUM_PARAM(KfTmdSlot, s32) slot)
{
    memory_release_last();
}
