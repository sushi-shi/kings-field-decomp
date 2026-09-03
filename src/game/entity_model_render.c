#include <kf/address.h>
#include <kf/psyq.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/*
 * Pooled 3D-model entity emitters invoked by the frame renderer's pool sweep
 * (render_entities).  Each carries one live pool record into view space through
 * RotTrans, composes a per-axis rotation onto the current view matrix, installs
 * the derived light matrix, projects the selected TMD object's vertices, and
 * hands the primitive run to the shared polygon enqueuer (render_enqueue_tmd).
 *
 * WIP: the two enqueuers reached here (render_enqueue_tmd, the large TMD primitive
 * emitter, and render_enqueue_model, the actor-specific enqueue path) are the render
 * subsystem's core polygon builders and remain unreconstructed; they are
 * reached by their address identities.  The actor descriptor byte lives in the
 * actor definition record (definitions[id].unknown_00[1]); its low nibble
 * selects the asset slot and its high nibble selects a floor-billboard depth
 * pair (DAT_80095038/DAT_80095048).
 *
 * Codegen residues (both structurally exact -- calls, referents, widths, and
 * control flow all match).  render_actor: retail zero-extends the descriptor
 * byte with a redundant `andi 0xff` before the `>> 4` nibble shift (and again
 * before the depth-table index), and keeps `high - 1` in its own register;
 * gcc-2.5.7 provably drops the mask from the lbu-loaded byte and folds the -1
 * into the table addend, which also swaps the descriptor/object callee-saved
 * registers and trims the frame.  render_map_object: retail lowers the four-way
 * behavior dispatch as a sequential comparison tree with the 180 arm falling
 * through and the object-id mask duplicated across the exit blocks; gcc-2.5.7
 * if-converts the `< 4 ? 180 : 0` tail to `negu`/`andi 0xb4` and cross-jumps
 * the mask into one block.  Both are the "gcc-2.5.7 optimizes more than retail"
 * wall documented in docs/patterns/source-shapes-gcc257.md; the sources are the
 * honest shapes and must not be distorted to re-introduce the retail idioms.
 */

/*
 * Floor-billboard depth/scale pairs indexed by the actor descriptor's high
 * nibble, copied into the floor-item render descriptor (DAT_80095058 /
 * DAT_8009505a) just before render_enqueue_model enqueues the billboard.  DAT_80095048
 * is the paired table 18 bytes below DAT_8009505a, so it is reached as an
 * offset from that descriptor field (the shared-base addend the original
 * produced).
 */
extern u16 DAT_80095038[];

extern void lighting_set_active_color_matrix(s32 index);
extern KfTmdObject *tmd_get_object(u16 object_index);
extern u16 *render_bind_animated_instance(void *anchor, u16 asset, u16 tag, u16 variant, u16 count);
extern void render_enqueue_tmd(u16 object_index, s16 depth_bias);

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

    SetRotMatrix((MATRIX *)&render_state.view_matrix);
    SetTransMatrix((MATRIX *)&render_state.view_matrix);
    screen.vx = (u16)actor->position.vx - (u16)render_state.view_position.vx;
    screen.vy = (u16)actor->position.vy - (u16)render_state.view_position.vy;
    screen.vz = (u16)actor->position.vz - (u16)render_state.view_position.vz;
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    matrix_set_rotation_x(actor->rotation.x, &model);
    matrix_set_rotation_y(-actor->rotation.y, &rot_y);
    MulMatrix2(&rot_y, &model);
    MulMatrix0(&render_light_matrices[0], &model, &light);
    MulMatrix2((MATRIX *)&render_state.view_matrix, &model);
    SetRotMatrix(&model);
    SetTransMatrix(&model);
    SetLightMatrix(&light);

    descriptor = actor_state.definitions[actor->definition_id].unknown_00[1];
    asset = descriptor & 0xf;
    asset_registry_select(asset);
    object = tmd_get_object(0);
    if (render_bind_animated_instance(&actor->unknown_34, asset, actor->animation_id,
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
        DAT_8009505a = DAT_80095038[high - 1];
        DAT_80095058 = ((u16 *)((char *)&DAT_8009505a - 18))[high - 1];
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

    SetRotMatrix((MATRIX *)&render_state.view_matrix);
    SetTransMatrix((MATRIX *)&render_state.view_matrix);
    screen.vx = (u16)object->position_x - (u16)render_state.view_position.vx;
    screen.vy = (u16)object->position_y - (u16)render_state.view_position.vy;
    screen.vz = (u16)object->position_z - (u16)render_state.view_position.vz;
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    matrix_set_rotation_x(object->rotation.x, &rot_x);
    matrix_set_rotation_y(object->rotation.y, &model);
    MulMatrix(&model, &rot_x);
    MulMatrix0((MATRIX *)&render_state.light_matrix, &model, &light);
    MulMatrix2((MATRIX *)&render_state.view_matrix, &model);
    SetRotMatrix(&model);
    SetTransMatrix(&model);
    SetLightMatrix(&light);

    id = object->object_id;
    switch (map_object_state.definitions[object->object_id].behavior_type) {
    case 0:
    case 1:
        depth = 15;
        break;
    case 2:
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
void func_8001ed38(void)
{
    lighting_set_active_color_matrix(0);
    SetGeomScreen(0xc8);
    tmd_select(4);
    tmd_select_object_vertices(0);
    tmd_project_vertices(tmd_get_object(0)->vertex_count);
    render_enqueue_tmd(0, 0x3e8);
}
