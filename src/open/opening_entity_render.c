#include <kf/address.h>
#include <kf/open_opening_render.h>
#include <kf/open_render.h>
#include <kf/psyq.h>

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

    SetRotMatrix(&render_state.view_matrix);
    SetTransMatrix(&render_state.view_matrix);
    screen.vx = (u16)entity->position.vx - (u16)render_state.view_position.vx;
    screen.vy = (u16)entity->position.vy - (u16)render_state.view_position.vy;
    screen.vz = (u16)entity->position.vz - (u16)render_state.view_position.vz;
    /* RotTrans writes the three translation words, not a VECTOR pad word. */
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    matrix_set_rotation_yxz(&entity->rotation, &model);
    scale.vx = entity->scale.vx;
    scale.vy = entity->scale.vy;
    scale.vz = entity->scale.vz;
    ScaleMatrix(&model, &scale);
    MulMatrix0(&render_state.light_matrix, &model, &light);
    MulMatrix2(&render_state.view_matrix, &model);
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
        func_8001764c(object_id, 0);
        return;
    case 26:
        depth = 1000;
        goto render_alternate;
    case 27:
        depth = 10000;
    render_alternate:
        tmd_select_object_vertices(object_id);
        tmd_project_vertices(tmd_get_object(object_id)->vertex_count);
        func_80018344(object_id, depth);
        return;
    default:
        depth = 0;
        break;
    }

    tmd_select_object_vertices(object_id);
    tmd_project_vertices(tmd_get_object(object_id)->vertex_count);
    func_8001764c(object_id, depth);
}
