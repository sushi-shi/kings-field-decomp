void render_floor_item(KfFloorItem *item, const MATRIX *lights)
{
    SVECTOR screen;
    MATRIX model;
    KfFloorItemFacing facing;
    s16 depth_bias;

    setVector(&screen,
        item->position_x - KF_GRAPHICS_RUNTIME.render_state.view_position.vx,
        item->position_y - KF_GRAPHICS_RUNTIME.render_state.view_position.vy,
        item->position_z - KF_GRAPHICS_RUNTIME.render_state.view_position.vz);
    kf::render_place_model(model, KF_GRAPHICS_RUNTIME.render_state.view_matrix, screen);
    facing = item->facing;
    if (facing != KF_FLOOR_ITEM_FACING_BILLBOARD) {
        matrix_set_rotation_y(
            (kf_enum_encode<u16>(facing) - kf_enum_encode<u8>(KF_FLOOR_ITEM_FACING_ZERO_YAW)) << KF_FLOOR_ITEM_FACING_TO_ANGLE_SHIFT,
            &model);
        kf::matrix_multiply_rotation(KF_GRAPHICS_RUNTIME.render_state.view_matrix, model, model);
        depth_bias = KF_FLOOR_ITEM_FIXED_FACING_DEPTH_BIAS;
    } else {
        memcpy(model.m, KF_GRAPHICS_RUNTIME.render_state.pitch_matrix.m, sizeof model.m);
        depth_bias = KF_FLOOR_ITEM_BILLBOARD_DEPTH_BIAS;
    }
    render_enqueue_sprite(&floor_item_sprites[kf_enum_encode<u16>(item->base_sprite_index) + item->animation_frame], depth_bias, KF_SPRITE_DEPTH_CUE_BOOSTED, lights, &model, KF_GRAPHICS_RUNTIME.render_state.projection);
    floor_item_advance_frame(item);
}
