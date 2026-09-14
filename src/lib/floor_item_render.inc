ADDRESS_AT("GAME", 0x8001ed90, 0x14c)
ADDRESS_AT("OPEN", 0x800190f4, 0x14c)
void render_floor_item(KfFloorItem *item)
{
    SVECTOR screen;
    MATRIX model;
    long flag;
    KF_ENUM_PARAM(KfFloorItemFacing, u16) facing;
    s16 depth_bias;

    SetRotMatrix(&KF_GRAPHICS_RUNTIME.render_state.view_matrix);
    SetTransMatrix(&KF_GRAPHICS_RUNTIME.render_state.view_matrix);
    setVector(&screen,
        item->position_x - KF_GRAPHICS_RUNTIME.render_state.view_position.vx,
        item->position_y - KF_GRAPHICS_RUNTIME.render_state.view_position.vy,
        item->position_z - KF_GRAPHICS_RUNTIME.render_state.view_position.vz);
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    facing = floor_item_facing(item->facing_and_frame_count);
    if (KF_ENUM_ENCODE(u8, facing) != KF_ENUM_ENCODE(u8, KF_FLOOR_ITEM_FACING_BILLBOARD)) {
        matrix_set_rotation_y(
            (KF_ENUM_ENCODE(u16, facing) - KF_ENUM_ENCODE(u8, KF_FLOOR_ITEM_FACING_ZERO_YAW)) << KF_FLOOR_ITEM_FACING_TO_ANGLE_SHIFT,
            &model);
        MulMatrix2(&KF_GRAPHICS_RUNTIME.render_state.view_matrix, &model);
        SetRotMatrix(&model);
        depth_bias = KF_FLOOR_ITEM_FIXED_FACING_DEPTH_BIAS;
    } else {
        SetRotMatrix(&KF_GRAPHICS_RUNTIME.render_state.pitch_matrix);
        depth_bias = KF_FLOOR_ITEM_BILLBOARD_DEPTH_BIAS;
    }
    SetTransMatrix(&model);
    render_enqueue_sprite(
        &floor_item_sprites[KF_ENUM_ENCODE(u16, item->base_sprite_index) + item->animation_frame],
        depth_bias, KF_SPRITE_DEPTH_CUE_BOOSTED);
    floor_item_advance_frame(item);
}
