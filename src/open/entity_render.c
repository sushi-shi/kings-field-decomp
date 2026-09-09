#include <kf/address.h>
#include <kf/game_math.h>
#include <kf/open_resources.h>
#include <kf/map_data.h>
#include <kf/open_opening_render.h>
#include <kf/open_render.h>
#include <kf/open_scene0.h>
#include <kf/psyq.h>

enum {
    OPENING_MODEL_DEPTH_BIAS = -100,
    OPENING_MODEL_YAW_STEP = 64,
    ENDING_TRANSLATING_MODEL_DEPTH_BIAS = 1000,
    ENDING_ROTATING_MODEL_DEPTH_BIAS = 10000,
    FLOOR_ITEM_RENDER_BRIGHTNESS = 180
};

DATA(0x800358e0, 0x54)
KfSpriteQuad floor_item_sprites[KF_FLOOR_ITEM_SPRITE_COUNT] = {
    {0x90, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0xd0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},
    {0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},
    {0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},
};

/* Shared sweep caller and transform setup support this WIP contiguous module. */
RODATA(0x80012238, 0x44)

ADDRESS(0x80018ecc, 0x228)
void opening_entity_render(KfOpeningEntity *entity)
{
    VECTOR scale;
    SVECTOR screen;
    MATRIX model;
    MATRIX light;
    long flag;
    KF_ENUM_STORAGE(KfOpeningModelId, u16) object_id;
    s16 depth;

    SetRotMatrix(&open_graphics_runtime.render_state.view_matrix);
    SetTransMatrix(&open_graphics_runtime.render_state.view_matrix);
    setVector(&screen,
        (u16)entity->position.vx - (u16)open_graphics_runtime.render_state.view_position.vx,
        (u16)entity->position.vy - (u16)open_graphics_runtime.render_state.view_position.vy,
        (u16)entity->position.vz - (u16)open_graphics_runtime.render_state.view_position.vz);
    /* RotTrans writes the three translation words, not a VECTOR pad word. */
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    matrix_set_rotation_yxz(&entity->rotation, &model);
    copyVector(&scale, &entity->scale);
    ScaleMatrix(&model, &scale);
    MulMatrix0(&open_graphics_runtime.render_state.light_matrix, &model, &light);
    MulMatrix2(&open_graphics_runtime.render_state.view_matrix, &model);
    SetRotMatrix(&model);
    SetTransMatrix(&model);
    SetLightMatrix(&light);

    object_id = entity->object_id;
    depth = 0;
    switch (entity->object_id) {
    case KF_OPENING_SCENE0_DECREASING_YAW_MODEL:
    case KF_OPENING_SCENE0_INCREASING_YAW_MODEL:
    case KF_OPENING_SCENE3_INCREASING_YAW_MODEL:
    case KF_OPENING_SCENE3_DECREASING_YAW_MODEL:
    case KF_OPENING_TAPERED_COLUMN:
        depth = OPENING_MODEL_DEPTH_BIAS;
        break;
    case KF_OPENING_GREEN_CRYSTAL_INCREASING_YAW:
    case KF_OPENING_PINK_CRYSTAL_INCREASING_YAW:
        entity->rotation.y = (entity->rotation.y + OPENING_MODEL_YAW_STEP) & KF_ANGLE_WRAP_MASK;
        break;
    case KF_OPENING_GREEN_CRYSTAL_DECREASING_YAW:
    case KF_OPENING_PINK_CRYSTAL_DECREASING_YAW:
        entity->rotation.y = (entity->rotation.y - OPENING_MODEL_YAW_STEP) & KF_ANGLE_WRAP_MASK;
        break;
    case KF_OPENING_CASTLE_MOUNTAIN_BACKDROP:
        tmd_select_object_vertices(KF_ENUM_ENCODE(u16, object_id));
        tmd_project_vertices_perspective_right(
            tmd_get_object(KF_ENUM_ENCODE(u16, object_id))->vertex_count);
        render_enqueue_tmd(KF_ENUM_ENCODE(u16, object_id), 0);
        return;
    case KF_OPENING_ENDING_ORANGE_DISK:
        depth = ENDING_TRANSLATING_MODEL_DEPTH_BIAS;
        goto render_alternate;
    case KF_OPENING_ENDING_STARFIELD:
        depth = ENDING_ROTATING_MODEL_DEPTH_BIAS;
    render_alternate:
        tmd_select_object_vertices(KF_ENUM_ENCODE(u16, object_id));
        tmd_project_vertices(tmd_get_object(KF_ENUM_ENCODE(u16, object_id))->vertex_count);
        render_enqueue_unlit_triangles(KF_ENUM_ENCODE(u16, object_id), depth);
        return;
    default:
        depth = 0;
        break;
    }

    tmd_select_object_vertices(KF_ENUM_ENCODE(u16, object_id));
    tmd_project_vertices(tmd_get_object(KF_ENUM_ENCODE(u16, object_id))->vertex_count);
    render_enqueue_tmd(KF_ENUM_ENCODE(u16, object_id), depth);
}

ADDRESS(0x800190f4, 0x14c)
void render_floor_item(KfFloorItem *item)
{
    SVECTOR screen;
    MATRIX model;
    long flag;
    KF_ENUM_PARAM(KfFloorItemFacing, u16) facing;
    u32 next_frame;
    u32 frame_count;
    s16 depth_bias;

    SetRotMatrix(&open_graphics_runtime.render_state.view_matrix);
    SetTransMatrix(&open_graphics_runtime.render_state.view_matrix);
    setVector(&screen,
        (u16)item->position_x - (u16)open_graphics_runtime.render_state.view_position.vx,
        (u16)item->position_y - (u16)open_graphics_runtime.render_state.view_position.vy,
        (u16)item->position_z - (u16)open_graphics_runtime.render_state.view_position.vz);
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    facing = floor_item_facing(item->facing_and_frame_count);
    if (KF_ENUM_ENCODE(u8, facing) != KF_ENUM_ENCODE(u8, KF_FLOOR_ITEM_FACING_BILLBOARD)) {
        matrix_set_rotation_y(
            (KF_ENUM_ENCODE(u16, facing) - KF_ENUM_ENCODE(u8, KF_FLOOR_ITEM_FACING_ZERO_YAW)) << KF_FLOOR_ITEM_FACING_TO_ANGLE_SHIFT,
            &model);
        MulMatrix2(&open_graphics_runtime.render_state.view_matrix, &model);
        SetRotMatrix(&model);
        depth_bias = KF_FLOOR_ITEM_FIXED_FACING_DEPTH_BIAS;
    } else {
        SetRotMatrix(&open_graphics_runtime.render_state.pitch_matrix);
        depth_bias = KF_FLOOR_ITEM_BILLBOARD_DEPTH_BIAS;
    }
    SetTransMatrix(&model);
    render_enqueue_sprite(
        &floor_item_sprites[KF_ENUM_ENCODE(u16, item->base_sprite_index) + item->animation_frame], depth_bias, KF_SPRITE_DEPTH_CUE_BOOSTED);
    next_frame = item->animation_frame + 1;
    frame_count = KF_ENUM_ENCODE(u8, item->facing_and_frame_count);
    item->animation_frame = next_frame;
    if ((next_frame & 0xff) >= (frame_count & KF_FLOOR_ITEM_FRAME_COUNT_MASK)) {
        item->animation_frame = 0;
    }
}

ADDRESS(0x80019240, 0x298)
void opening_render_entities_and_items(void)
{
    const KfCellWindow *window = open_graphics_runtime.active_cell_window;
    u16 origin_z = (u16)open_graphics_runtime.render_state.view_cell.z - window->origin_z;
    u16 origin_x = (u16)open_graphics_runtime.render_state.view_cell.x - window->origin_x;
    KfOpeningEntity *entity;
    KfFloorItem *item;
    u16 *material_tpage;
    u16 row;
    u16 col;
    s16 remaining;

    tmd_select(KF_TMD_SLOT_ENTITIES);
    entity = opening_entity_state.entities;
    for (remaining = KF_OPENING_ENTITY_CAPACITY - 1; remaining != -1; remaining--) {
        if (entity->object_id < KF_OPENING_ENTITY_MODEL_LIMIT) {
            const KfCellWindow *grid = open_graphics_runtime.active_cell_window;

            row = entity->cell_z - origin_z;
            if (row < grid->height) {
                col = entity->cell_x - origin_x;

                if (col < grid->width && grid->cells[row * grid->width + col] != KF_CELL_WINDOW_HIDDEN) {
                    opening_entity_render(entity);
                }
            }
        }
        entity++;
    }

    SetLightMatrix(&floor_item_light_matrix);
    material_tpage = &open_graphics_runtime.floor_item_state.material.tpage;
    open_graphics_runtime.floor_item_state.material.color.r = open_graphics_runtime.floor_item_state.material.color.g =
        open_graphics_runtime.floor_item_state.material.color.b = FLOOR_ITEM_RENDER_BRIGHTNESS;
    *material_tpage = open_graphics_runtime.floor_item_state.texture_tpage;
    open_graphics_runtime.floor_item_state.material.clut = open_graphics_runtime.floor_item_state.texture_clut;
    item = open_graphics_runtime.floor_item_state.items;
    remaining = open_graphics_runtime.floor_item_state.count;
    while (--remaining != -1) {
        const KfCellWindow *grid = open_graphics_runtime.active_cell_window;

        row = item->position_z / KF_MAP_TILE_SIZE - origin_z;
        if (row < grid->height) {
            col = item->position_x / KF_MAP_TILE_SIZE - origin_x;

            if (col < grid->width
                    && grid->cells[row * grid->width + col]
                        != KF_CELL_WINDOW_HIDDEN) {
                render_floor_item(item);
            }
        }
        item++;
    }
}
