#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/psyq.h>
#include <kf/game_render.h>
#include <kf/game_math.h>
#include <kf/game_asset.h>

enum {
    ACTOR_MODEL_ASSET_MASK = 0xf,
    ACTOR_MODEL_TEXTURE_SHIFT = 4,
    MAP_LIFT_DOOR_DEPTH_BIAS = 180,
    MAP_HINGED_DOOR_DEPTH_BIAS = 15,
    MENU_ITEM_DEPTH_BIAS = 1000
};

/* Transform pooled models into view space and install their derived lighting.
 * The actor descriptor's low nibble selects the asset; its high nibble selects
 * a cached texture page and CLUT when nonzero.
 */

ADDRESS(0x8001e9a4, 0x214)
void render_actor(KfActor *actor)
{
    SVECTOR screen;
    MATRIX rot_y;
    MATRIX model;
    MATRIX light;
    long flag;
    KfTmdObject *object;
    u8 descriptor;
    u16 asset;

    SetRotMatrix(&game_graphics_runtime.render_state.view_matrix);
    SetTransMatrix(&game_graphics_runtime.render_state.view_matrix);
    setVector(&screen,
        actor->position.vx - game_graphics_runtime.render_state.view_position.vx,
        actor->position.vy - game_graphics_runtime.render_state.view_position.vy,
        actor->position.vz - game_graphics_runtime.render_state.view_position.vz);
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    matrix_set_rotation_x(actor->rotation.angles.x, &model);
    matrix_set_rotation_y(-actor->rotation.angles.y, &rot_y);
    MulMatrix2(&rot_y, &model);
    MulMatrix0(&render_light_matrices[KF_RENDER_LIGHT_ACTOR], &model, &light);
    MulMatrix2(&game_graphics_runtime.render_state.view_matrix, &model);
    SetRotMatrix(&model);
    SetTransMatrix(&model);
    SetLightMatrix(&light);

    descriptor = actor_state.definitions.entries[actor->definition_id].model_and_texture;
    asset = descriptor & ACTOR_MODEL_ASSET_MASK;
    asset_registry_select(asset);
    object = tmd_get_object(0);
    if (render_bind_animated_instance(
            &actor->animation_cache, asset, actor->animation_id,
            actor->animation_phase, object->vertex_count) == 0) {
        tmd_select_object_vertices(0);
        tmd_project_vertices(tmd_get_object(0)->vertex_count);
    } else {
        tmd_project_vertices(object->vertex_count);
    }

    descriptor >>= ACTOR_MODEL_TEXTURE_SHIFT;
    if (descriptor-- == 0) {
        render_enqueue_tmd(0, 0);
    } else {
        game_graphics_runtime.active_render_tpage = game_graphics_runtime.effect5_texture_pages[descriptor];
        game_graphics_runtime.active_render_clut = game_graphics_runtime.effect5_texture_cluts[descriptor];
        render_enqueue_model(0, 0);
    }
}

ADDRESS(0x8001ebb8, 0x180)
void render_map_object(KfMapObject *object)
{
    SVECTOR screen;
    MATRIX rot_x;
    MATRIX model;
    MATRIX light;
    long flag;
    KF_ENUM_STORAGE(KfMapObjectId, u16) id;
    s16 depth;

    SetRotMatrix(&game_graphics_runtime.render_state.view_matrix);
    SetTransMatrix(&game_graphics_runtime.render_state.view_matrix);
    setVector(&screen,
        object->position.vx - game_graphics_runtime.render_state.view_position.vx,
        object->position.vy - game_graphics_runtime.render_state.view_position.vy,
        object->position.vz - game_graphics_runtime.render_state.view_position.vz);
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    matrix_set_rotation_x(object->rotation.angles.x, &rot_x);
    matrix_set_rotation_y(object->rotation.angles.y, &model);
    MulMatrix(&model, &rot_x);
    MulMatrix0(&game_graphics_runtime.render_state.light_matrix, &model, &light);
    MulMatrix2(&game_graphics_runtime.render_state.view_matrix, &model);
    SetRotMatrix(&model);
    SetTransMatrix(&model);
    SetLightMatrix(&light);

    id = object->object_id;
    switch (map_object_state.definitions.entries[KF_ENUM_ENCODE(u8, object->object_id)].behavior_type) {
    case KF_MAP_OBJECT_BEHAVIOR_LIFT_DOOR:
    case KF_MAP_OBJECT_BEHAVIOR_03:
        depth = MAP_LIFT_DOOR_DEPTH_BIAS;
        break;
    case KF_MAP_OBJECT_BEHAVIOR_HINGED_DOOR:
    case KF_MAP_OBJECT_BEHAVIOR_HINGED_DOOR_PARTNER:
        depth = MAP_HINGED_DOOR_DEPTH_BIAS;
        break;
    default:
        depth = 0;
        break;
    }
    tmd_select_object_vertices(KF_ENUM_ENCODE(u16, id));
    tmd_project_vertices(tmd_get_object(KF_ENUM_ENCODE(u16, id))->vertex_count);
    render_enqueue_tmd(KF_ENUM_ENCODE(u16, id), depth);
}

ADDRESS(0x8001ed38, 0x58)
void menu_render_item_model(void)
{
    lighting_set_active_color_matrix(KF_GAME_COLOR_DEFAULT);
    SetGeomScreen(KF_DEFAULT_PROJECTION_DISTANCE);
    tmd_select(KF_TMD_SLOT_MENU_ITEM);
    tmd_select_object_vertices(0);
    tmd_project_vertices(tmd_get_object(0)->vertex_count);
    render_enqueue_tmd(0, MENU_ITEM_DEPTH_BIAS);
}
