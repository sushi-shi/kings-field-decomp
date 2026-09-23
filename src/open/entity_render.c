#include <kf/lib/math.h>
#include <kf/open/resources.h>
#include <kf/lib/map_data.h>
#include <kf/open/opening_render.h>
#include <kf/open/render.h>
#include <kf/open/scene0.h>
#include <psyq/sdk.h>
#include <kf/lib/graphics.h>

enum {
    OPENING_MODEL_DEPTH_BIAS = -100,
    OPENING_MODEL_YAW_STEP = 64,
    ENDING_TRANSLATING_MODEL_DEPTH_BIAS = 1000,
    ENDING_ROTATING_MODEL_DEPTH_BIAS = 10000
};

KfSpriteQuad floor_item_sprites[KF_FLOOR_ITEM_SPRITE_COUNT] = {
    {0x90, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0xd0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},
    {0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},
    {0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},
};

void opening_entity_render(KfOpeningEntity *entity)
{
    VECTOR scale;
    SVECTOR screen;
    MATRIX model;
    MATRIX light;
    long flag;
    u16 object_id;
    s16 depth;

    SetRotMatrix(&open_graphics_runtime.render_state.view_matrix);
    SetTransMatrix(&open_graphics_runtime.render_state.view_matrix);
    setVector(&screen,
        entity->position.vx - open_graphics_runtime.render_state.view_position.vx,
        entity->position.vy - open_graphics_runtime.render_state.view_position.vy,
        entity->position.vz - open_graphics_runtime.render_state.view_position.vz);

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
        tmd_select_object_vertices(((u16)(object_id)));
        tmd_project_vertices_perspective_right(
            tmd_get_object(((u16)(object_id)))->vertex_count);
        render_enqueue_tmd(((u16)(object_id)), 0);
        return;
    case KF_OPENING_ENDING_ORANGE_DISK:
        depth = ENDING_TRANSLATING_MODEL_DEPTH_BIAS;
        goto render_alternate;
    case KF_OPENING_ENDING_STARFIELD:
        depth = ENDING_ROTATING_MODEL_DEPTH_BIAS;
    render_alternate:
        tmd_select_object_vertices(((u16)(object_id)));
        tmd_project_vertices(tmd_get_object(((u16)(object_id)))->vertex_count);
        render_enqueue_unlit_triangles(((u16)(object_id)), depth);
        return;
    default:
        depth = 0;
        break;
    }

    tmd_select_object_vertices(((u16)(object_id)));
    tmd_project_vertices(tmd_get_object(((u16)(object_id)))->vertex_count);
    render_enqueue_tmd(((u16)(object_id)), depth);
}

#include "../lib/floor_item_render.inc"

void opening_render_entities_and_items(void)
{
    const KfCellWindow *window = open_graphics_runtime.active_cell_window;
    u16 origin_z = open_graphics_runtime.render_state.view_cell.z - window->origin_z;
    u16 origin_x = open_graphics_runtime.render_state.view_cell.x - window->origin_x;
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
        open_graphics_runtime.floor_item_state.material.color.b = KF_FLOOR_ITEM_RENDER_BRIGHTNESS;
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
