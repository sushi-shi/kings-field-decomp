#include <kf/address.h>
#include <kf/semantic_types.h>

/*
 * Per-entity billboard/model emitters invoked by the frame renderer's pool
 * sweep (render_entities).  Each transforms one live entry into view space, binds
 * the matching asset, and hands the result to the shared sprite/model draw
 * helpers.
 *
 * WIP: the pool record layouts and the sprite descriptor tables in the
 * 0x80055afc load-data blob are unresolved, so entity fields and the tables are
 * reached through byte views until their owners are reconstructed.
 *
 * Codegen residue (render_floor_item): retail zero-extends the masked facing nibble
 * with a redundant `andi 0xff` after `andi 0xf0` before its `beqz`/subtract; the
 * value provably fits a byte, so the rebuilt gcc-2.5.7 probe elides the second
 * mask (the inverse of the byte-field cases in docs/patterns/source-shapes-
 * gcc257.md).  The single extra instruction is the only divergence.
 */

extern KfRenderState render_state;

/*
 * Sprite-descriptor blob; ed90 indexes the 12-byte records four bytes in and
 * eedc's billboard path indexes them 0x58 bytes in.
 */
extern u8 DAT_80055afc[];

extern void matrix_set_rotation_y(s16 angle, MATRIX *matrix);
extern void matrix_set_rotation_yxz(const struct KfEulerAngles *angles, MATRIX *matrix);
extern void render_enqueue_sprite(char *descriptor, s16 screen_scale, s32 flag);
extern void asset_registry_select(u16 index);
extern KfTmdObject *tmd_get_object(u16 index);
extern void tmd_select_object_vertices(u16 object_index);
extern void tmd_project_vertices(s32 count);
extern u16 *render_bind_animated_instance(void *anchor, u16 asset, u16 tag, u16 variant, u16 count);
extern void render_enqueue_tmd(u16 arg0, s16 arg1);

/*
 * One record of the floor-decoration pool (24-byte stride).  Only the transform
 * inputs and the animation cursor are resolved.
 */
typedef struct KfFloorSprite {
    u16 sprite_id;    /* +0 */
    u8 orientation;   /* +2: high nibble facing step, low nibble frame count */
    u8 unknown_03;    /* +3 */
    u16 position_x;   /* +4 */
    u16 unknown_06;   /* +6 */
    u16 position_y;   /* +8 */
    u16 unknown_0a;   /* +10 */
    u16 position_z;   /* +12 */
    u8 unknown_0e[6]; /* +14 */
    u8 anim_frame;    /* +20 */
    u8 unknown_15[3]; /* +21 */
} KfFloorSprite;

/*
 * Emits one floor decoration.  The record's world position is carried into the
 * view via RotTrans (its result lands directly in the model matrix's t column),
 * then either a facing rotation is composed onto the view matrix or the render
 * pitch matrix is used verbatim.  The sprite frame advances and wraps against
 * the low nibble of the orientation byte.
 */
ADDRESS(0x8001ed90, 0x14c)
void render_floor_item(KfFloorSprite *sprite)
{
    SVECTOR screen;
    MATRIX model;
    long flag;
    u8 facing;
    u32 next_frame;
    u32 frame_count;
    s16 screen_scale;

    SetRotMatrix((MATRIX *)&render_state.view_matrix);
    SetTransMatrix((MATRIX *)&render_state.view_matrix);
    screen.vx = sprite->position_x - (u16)render_state.view_position.vx;
    screen.vy = sprite->position_y - (u16)render_state.view_position.vy;
    screen.vz = sprite->position_z - (u16)render_state.view_position.vz;
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    facing = sprite->orientation & 0xf0;
    if (facing != 0) {
        matrix_set_rotation_y((facing - 16) << 6, &model);
        MulMatrix2((MATRIX *)&render_state.view_matrix, &model);
        SetRotMatrix(&model);
        screen_scale = 0x96;
    } else {
        SetRotMatrix((MATRIX *)&render_state.pitch_matrix);
        screen_scale = 0xc8;
    }
    SetTransMatrix(&model);
    render_enqueue_sprite(
        (char *)&DAT_80055afc[4 + (sprite->sprite_id + sprite->anim_frame) * 12],
        screen_scale, 1);
    next_frame = sprite->anim_frame + 1;
    frame_count = sprite->orientation;
    sprite->anim_frame = next_frame;
    if ((next_frame & 0xff) >= (frame_count & 0xf)) {
        sprite->anim_frame = 0;
    }
}

/*
 * One record of the actor-billboard pool (60-byte stride).  Only the transform
 * inputs, the mode/id selectors, and the projection anchor are resolved.
 */
typedef struct KfActorSprite {
    u8 unknown_00[3];
    u8 sprite_id;                  /* +3: 0xff marks an empty slot */
    u8 mode;                       /* +4: 0xff selects the billboard sprite path */
    u8 unknown_05[3];
    u16 asset_variant;             /* +8 */
    u8 unknown_0a[2];
    u16 position_x;                /* +12 */
    u16 unknown_0e;
    u16 position_y;                /* +16 */
    u16 unknown_12;
    u16 position_z;                /* +20 */
    u8 unknown_16[6];
    struct KfEulerAngles rotation; /* +28 */
    u8 unknown_22[2];
    s16 scale_x;                   /* +36 */
    s16 scale_y;                   /* +38 */
    s16 scale_z;                   /* +40 */
    u8 unknown_2a[10];
    u8 anchor[4];                  /* +52 */
    u8 unknown_38[4];
} KfActorSprite;

/*
 * Emits one pooled actor.  Empty slots (id 0xff) are skipped.  The record is
 * carried into the view, oriented from its Euler angles and scaled in place.
 * A mode of 0xff draws a fixed billboard sprite through render_enqueue_sprite against
 * the render pitch matrix; otherwise the asset that follows the id by 30 is
 * bound, tested for visibility, and projected against the view matrix.
 */
ADDRESS(0x8001eedc, 0x1e8)
void render_actor_sprite(KfActorSprite *actor)
{
    SVECTOR screen;
    VECTOR scale;
    MATRIX model;
    long flag;
    u16 asset;
    KfTmdObject *object;

    if (actor->sprite_id == 0xff) {
        return;
    }
    SetRotMatrix((MATRIX *)&render_state.view_matrix);
    SetTransMatrix((MATRIX *)&render_state.view_matrix);
    screen.vx = actor->position_x - (u16)render_state.view_position.vx;
    screen.vy = actor->position_y - (u16)render_state.view_position.vy;
    screen.vz = actor->position_z - (u16)render_state.view_position.vz;
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    matrix_set_rotation_yxz(&actor->rotation, &model);
    scale.vx = actor->scale_x;
    scale.vy = actor->scale_y;
    scale.vz = actor->scale_z;
    ScaleMatrix(&model, &scale);
    if (actor->mode == 0xff) {
        MulMatrix2((MATRIX *)&render_state.pitch_matrix, &model);
        SetRotMatrix(&model);
        SetTransMatrix(&model);
        render_enqueue_sprite((char *)&DAT_80055afc[0x58 + actor->sprite_id * 12], 0, 0);
    } else {
        MulMatrix2((MATRIX *)&render_state.view_matrix, &model);
        SetRotMatrix(&model);
        SetTransMatrix(&model);
        asset = actor->sprite_id + 30;
        asset_registry_select(asset);
        object = tmd_get_object(0);
        if (render_bind_animated_instance(
                &actor->anchor, asset, actor->mode, actor->asset_variant,
                object->vertex_count) == 0) {
            tmd_select_object_vertices(0);
            tmd_project_vertices(tmd_get_object(0)->vertex_count);
        } else {
            tmd_project_vertices(object->vertex_count);
        }
        render_enqueue_tmd(0, 100);
    }
}
