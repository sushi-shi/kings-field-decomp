ADDRESS_AT("OPEN", 0x80013b5c, 0x114)
void sprite_add_f4(
    const KfScreenRect *rectangle, const CVECTOR *color, u16 ot_index)
{
    POLY_F4 *prim = (POLY_F4 *)KF_GRAPHICS_RUNTIME.display_state.primitive_buffer->cursor;

    KF_GRAPHICS_RUNTIME.display_state.primitive_buffer->cursor += sizeof(POLY_F4);
    SetPolyF4(prim);
    setXYWH(prim, rectangle->x, rectangle->y, rectangle->w, rectangle->h);
    setRGB0(prim, color->r, color->g, color->b);
    AddPrim(
        (void *)(&KF_ACTIVE_ORDERING_TABLE[ot_index & KF_ORDERING_TABLE_INDEX_MASK]),
        (void *)prim);
}
