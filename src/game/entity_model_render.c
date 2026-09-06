#include <kf/address.h>
#include <kf/psyq.h>
#include <kf/game_render.h>
#include <kf/game_math.h>
#include <kf/game_asset.h>

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
    int high;

    SetRotMatrix(&render_state.view_matrix);
    SetTransMatrix(&render_state.view_matrix);
    screen.vx = (u16)actor->position.vx - (u16)render_state.view_position.vx;
    screen.vy = (u16)actor->position.vy - (u16)render_state.view_position.vy;
    screen.vz = (u16)actor->position.vz - (u16)render_state.view_position.vz;
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    matrix_set_rotation_x(actor->rotation.x, &model);
    matrix_set_rotation_y(-actor->rotation.y, &rot_y);
    MulMatrix2(&rot_y, &model);
    MulMatrix0(&render_light_matrices[0], &model, &light);
    MulMatrix2(&render_state.view_matrix, &model);
    SetRotMatrix(&model);
    SetTransMatrix(&model);
    SetLightMatrix(&light);

    descriptor = actor_state.definitions[actor->definition_id].model_and_texture;
    asset = descriptor & 0xf;
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

    high = descriptor >> 4;
    if (high == 0) {
        render_enqueue_tmd(0, 0);
    } else {
        active_render_tpage = effect5_texture_pages[high - 1];
        active_render_clut = effect5_texture_cluts[high - 1];
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
    u16 id;
    s16 depth;

    SetRotMatrix(&render_state.view_matrix);
    SetTransMatrix(&render_state.view_matrix);
    screen.vx = (u16)object->position_x - (u16)render_state.view_position.vx;
    screen.vy = (u16)object->position_y - (u16)render_state.view_position.vy;
    screen.vz = (u16)object->position_z - (u16)render_state.view_position.vz;
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    matrix_set_rotation_x(object->rotation.x, &rot_x);
    matrix_set_rotation_y(object->rotation.y, &model);
    MulMatrix(&model, &rot_x);
    MulMatrix0(&render_state.light_matrix, &model, &light);
    MulMatrix2(&render_state.view_matrix, &model);
    SetRotMatrix(&model);
    SetTransMatrix(&model);
    SetLightMatrix(&light);

    id = object->object_id;
    switch (map_object_state.definitions[object->object_id].behavior_type) {
    case KF_MAP_OBJECT_BEHAVIOR_HINGED_DOOR:
    case KF_MAP_OBJECT_BEHAVIOR_HINGED_DOOR_PARTNER:
        depth = 15;
        break;
    case KF_MAP_OBJECT_BEHAVIOR_LIFT_DOOR:
    case 3:
        depth = 180;
        break;
    default:
        depth = 0;
        break;
    }
    tmd_select_object_vertices(id);
    tmd_project_vertices(tmd_get_object(id)->vertex_count);
    render_enqueue_tmd(id, depth);
}

ADDRESS(0x8001ed38, 0x58)
void menu_render_item_model(void)
{
    lighting_set_active_color_matrix(KF_GAME_COLOR_DEFAULT);
    SetGeomScreen(0xc8);
    tmd_select(KF_TMD_SLOT_MENU_ITEM);
    tmd_select_object_vertices(0);
    tmd_project_vertices(tmd_get_object(0)->vertex_count);
    render_enqueue_tmd(0, 0x3e8);
}
