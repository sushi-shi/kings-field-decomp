#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/open_opening_render.h>
#include <kf/open_render.h>
#include <kf/open_scene0.h>
#include <kf/psyq.h>

DATA(0x800358e0, 0x54)
KfSpriteQuad floor_item_sprites[7] = {
    {0x90, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0xd0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},
    {0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},
    {0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},
};

DATA(0x800359e4, 0x20)
MATRIX floor_item_light_matrix = {
    {{0, 0, 0x1000}, {0, 0, 0x1000}, {0, 0, 0}}, {0, 0, 0},
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
    u16 object_id;
    s16 depth;

    SetRotMatrix(&open_graphics_runtime.render_state.view_matrix);
    SetTransMatrix(&open_graphics_runtime.render_state.view_matrix);
    screen.vx = (u16)entity->position.vx - (u16)open_graphics_runtime.render_state.view_position.vx;
    screen.vy = (u16)entity->position.vy - (u16)open_graphics_runtime.render_state.view_position.vy;
    screen.vz = (u16)entity->position.vz - (u16)open_graphics_runtime.render_state.view_position.vz;
    /* RotTrans writes the three translation words, not a VECTOR pad word. */
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    matrix_set_rotation_yxz(&entity->rotation, &model);
    scale.vx = entity->scale.vx;
    scale.vy = entity->scale.vy;
    scale.vz = entity->scale.vz;
    ScaleMatrix(&model, &scale);
    MulMatrix0(&open_graphics_runtime.render_state.light_matrix, &model, &light);
    MulMatrix2(&open_graphics_runtime.render_state.view_matrix, &model);
    SetRotMatrix(&model);
    SetTransMatrix(&model);
    SetLightMatrix(&light);

    object_id = entity->object_id;
    depth = 0;
    switch (entity->object_id) {
    case 11:
    case 12:
    case 13:
    case 14:
    case 20:
        depth = -100;
        break;
    case 21:
    case 22:
        entity->rotation.y = (entity->rotation.y + 64) & 0xfff;
        break;
    case 23:
    case 24:
        entity->rotation.y = (entity->rotation.y - 64) & 0xfff;
        break;
    case 25:
        tmd_select_object_vertices(object_id);
        tmd_project_vertices_perspective_right(
            tmd_get_object(object_id)->vertex_count);
        render_enqueue_tmd(object_id, 0);
        return;
    case 26:
        depth = 1000;
        goto render_alternate;
    case 27:
        depth = 10000;
    render_alternate:
        tmd_select_object_vertices(object_id);
        tmd_project_vertices(tmd_get_object(object_id)->vertex_count);
        render_enqueue_unlit_triangles(object_id, depth);
        return;
    default:
        depth = 0;
        break;
    }

    tmd_select_object_vertices(object_id);
    tmd_project_vertices(tmd_get_object(object_id)->vertex_count);
    render_enqueue_tmd(object_id, depth);
}

ADDRESS(0x800190f4, 0x14c)
void render_floor_item(KfFloorItem *item)
{
    SVECTOR screen;
    MATRIX model;
    long flag;
    u8 facing;
    u32 next_frame;
    u32 frame_count;
    s16 depth_bias;

    SetRotMatrix(&open_graphics_runtime.render_state.view_matrix);
    SetTransMatrix(&open_graphics_runtime.render_state.view_matrix);
    screen.vx = (u16)item->position_x - (u16)open_graphics_runtime.render_state.view_position.vx;
    screen.vy = (u16)item->position_y - (u16)open_graphics_runtime.render_state.view_position.vy;
    screen.vz = (u16)item->position_z - (u16)open_graphics_runtime.render_state.view_position.vz;
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    facing = item->facing_and_frame_count & 0xf0;
    if (facing != 0) {
        matrix_set_rotation_y((facing - 16) << 6, &model);
        MulMatrix2(&open_graphics_runtime.render_state.view_matrix, &model);
        SetRotMatrix(&model);
        depth_bias = 150;
    } else {
        SetRotMatrix(&open_graphics_runtime.render_state.pitch_matrix);
        depth_bias = 200;
    }
    SetTransMatrix(&model);
    render_enqueue_sprite(
        &floor_item_sprites[item->item_id + item->animation_frame], depth_bias, 1);
    next_frame = item->animation_frame + 1;
    frame_count = item->facing_and_frame_count;
    item->animation_frame = next_frame;
    if ((next_frame & 0xff) >= (frame_count & 0xf)) {
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
    s16 remaining;

    tmd_select(1);
    entity = opening_entity_state.entities;
    for (remaining = 31; remaining != -1; remaining--) {
        if (entity->object_id < 32) {
            u16 row = entity->cell_z - origin_z;
            const KfCellWindow *grid = open_graphics_runtime.active_cell_window;

            if (row < grid->height) {
                u16 col = entity->cell_x - origin_x;

                if (col < grid->width && grid->cells[row * grid->width + col] != 0) {
                    opening_entity_render(entity);
                }
            }
        }
        entity++;
    }

    SetLightMatrix(&floor_item_light_matrix);
    open_graphics_runtime.floor_item_state.material.color.r = open_graphics_runtime.floor_item_state.material.color.g =
        open_graphics_runtime.floor_item_state.material.color.b = 180;
    open_graphics_runtime.floor_item_state.material.tpage = open_graphics_runtime.floor_item_state.texture_tpage;
    open_graphics_runtime.floor_item_state.material.clut = open_graphics_runtime.floor_item_state.texture_clut;
    item = open_graphics_runtime.floor_item_state.items;
    remaining = open_graphics_runtime.floor_item_state.count;
    while (--remaining != -1) {
        u16 row = item->position_z / KF_MAP_TILE_SIZE - origin_z;
        const KfCellWindow *grid = open_graphics_runtime.active_cell_window;

        if (row < grid->height) {
            u16 col = item->position_x / KF_MAP_TILE_SIZE - origin_x;

            if (col < grid->width && grid->cells[row * grid->width + col] != 0) {
                render_floor_item(item);
            }
        }
        item++;
    }
}
