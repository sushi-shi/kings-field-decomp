#include <kf/address.h>
#include <kf/game_effect.h>
#include <kf/game.h>

typedef char effect_state_alignment[__alignof__(KfEffectState) == 4 ? 1 : -1];
typedef char effect_state_records_offset[
    (u32)&((KfEffectState *)0)->records == 0x1e0 ? 1 : -1];
typedef char effect_state_current_magic_offset[
    (u32)&((KfEffectState *)0)->current_magic == 0xd20 ? 1 : -1];
typedef char effect_state_current_record_offset[
    (u32)&((KfEffectState *)0)->current_record == 0xd24 ? 1 : -1];

DATA(0x8009ce60, 0xd28)
KfEffectState effect_state;

/* The whole shared state is cleared at startup. The selector reaches the magic
 * rows by a member-relative offset from the current-record pointer slot. */

ADDRESS(0x80036f00, 0x44)
KfEffectRecord *effect_pool_find_free(void)
{
    KfEffectRecord *record = effect_pool_records;
    u16 i = KF_EFFECT_CAPACITY;

    do {
        if (record->type == KF_EFFECT_SLOT_FREE) {
            return record;
        }
        record++;
    } while (--i != 0);
    return 0;
}

RODATA(0x80012c28, 0xb4)

ADDRESS(0x80036f44, 0x82c)
KfEffectRecord *effect_pool_construct(
    u8 id, u8 type, u8 kind, const VECTOR *position,
    const SVECTOR *direction, ...)
{
    s32 *va = (s32 *)&direction;   /* variadic stack base: va[1]=arg6, va[2]=arg7, va[3]=arg8 */
    KfEffectRecord *record = effect_pool_find_free();
    KfMagicRecord *magic;

    if (record != 0) {
        record->type = type;
        record->kind = kind;
        record->position = *position;
        record->direction.vector = *direction;
        record->phase = 0;
        record->id = id;
        record->scale_z = KF_FIXED12_ONE;
        record->scale_y = KF_FIXED12_ONE;
        record->scale_x = KF_FIXED12_ONE;
        record->visual.animation_phase = 0;
        record->sound_played = 0;

        magic = &magic_records[record->kind];

        switch (record->kind) {
        case KF_EFFECT_KIND_FIRE_BALL:
            record->animation_clip = KF_EFFECT_ANIMATION_BILLBOARD;
            record->base_render_id = 0;
            record->render_id = 0;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            audio_play_spatial_default_range(&magic->sounds[0],
                                             &record->position, KF_AUDIO_MAX_VOLUME);
            break;
        case KF_EFFECT_KIND_WIND_CUTTER:
            record->animation_clip = KF_EFFECT_ANIMATION_BILLBOARD;
            record->base_render_id = 5;
            record->render_id = 5;
            record->rotation.vx = 0x352;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            if (va[2] != 0) {
                audio_play_spatial_default_range(&magic->sounds[0],
                                                 &record->position, KF_AUDIO_MAX_VOLUME);
            }
            break;
        case 0x17:
            record->base_render_id = 0x11;
            record->render_id = 0x11;
            record->kind = KF_EFFECT_KIND_LIGHTNING_BOLT;
            goto initialize_kind_04;
        case KF_EFFECT_KIND_LIGHTNING_BOLT:
            record->base_render_id = 6;
            record->render_id = 6;
        initialize_kind_04:
            record->animation_clip = KF_EFFECT_ANIMATION_BILLBOARD;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            record->control.frames_remaining = *(u16 *)(va + 1);
            if (va[2] != 0) {
                audio_play_spatial_range(
                    &magic_records[KF_MAGIC_LIGHTNING_BOLT].sounds[0],
                    &record->position, KF_AUDIO_MAX_VOLUME, 0x4e20, 0xea60);
            }
            break;
        case 0x29:
            record->base_render_id = 0x13;
            record->render_id = 0x13;
            record->kind = 0x20;
            goto initialize_kind_20;
        case 0x20:
            record->base_render_id = 0xb;
            record->render_id = 0xb;
        initialize_kind_20:
            record->animation_clip = KF_EFFECT_ANIMATION_BILLBOARD;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            audio_play_spatial_range(
                &magic_records[KF_MAGIC_LIGHTNING_BOLT].sounds[1],
                &record->position, KF_AUDIO_MAX_VOLUME, 0x4e20, 0xea60);
            break;
        case 0x2a:
            record->base_render_id = 0xf;
            record->render_id = 0xf;
            record->kind = 0x21;
            record->animation_clip = 0;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            break;
        case 0x21:
            record->base_render_id = 0;
            record->render_id = 0;
            record->animation_clip = 0;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            break;
        case KF_EFFECT_KIND_GROUND_BRANCH:
            record->animation_clip = 0;
            record->base_render_id = 1;
            record->render_id = 1;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            record->scale_y = 0;
            {
                u16 argument = *(u16 *)(va + 1);
                record->control.frames_remaining = 6;
                record->propagation.branch = argument;
            }
            if (record->propagation.branch != KF_EFFECT_GROUND_BRANCH_LEAF) {
                sound_ref_play(&magic->sounds[0], 0x78);
            }
            break;
        case 0x22:
            record->animation_clip = 0;
            record->base_render_id = 2;
            record->render_id = 2;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            record->scale_y = 0;
            break;
        case KF_EFFECT_KIND_ACTOR_SPAWNER:
            record->animation_clip = 0;
            record->base_render_id = 3;
            record->render_id = 3;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            record->scale_z = 0;
            record->scale_y = 0;
            record->scale_x = 0;
            record->control.frames_remaining = *(u16 *)(va + 1);
            audio_play_spatial_default_range(&magic->sounds[0],
                                             &record->position, KF_AUDIO_MAX_VOLUME);
            break;
        case KF_EFFECT_KIND_SCATTER_PROJECTILE:
            record->animation_clip = KF_EFFECT_ANIMATION_BILLBOARD;
            record->base_render_id = 9;
            record->render_id = 9;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            record->propagation.generations_remaining = *(u16 *)(va + 1);
            record->control.frames_remaining = *(u16 *)(va + 2);
            record->scale_x = record->scale_y = record->scale_z =
                record->visual.pulse_base_scale = *(u16 *)(va + 3);
            audio_play_spatial_default_range(&magic->sounds[0],
                                             &record->position, KF_AUDIO_MAX_VOLUME);
            break;
        case KF_EFFECT_KIND_DARKNESS_PROJECTILE:
            record->animation_clip = KF_EFFECT_ANIMATION_BILLBOARD;
            record->base_render_id = 8;
            record->render_id = 8;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            audio_play_spatial_default_range(&magic->sounds[0],
                                             &record->position, KF_AUDIO_MAX_VOLUME);
            break;
        case KF_EFFECT_KIND_CURSE_PROJECTILE:
            record->animation_clip = KF_EFFECT_ANIMATION_BILLBOARD;
            record->base_render_id = 0xa;
            record->render_id = 0xa;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            audio_play_spatial_default_range(&magic->sounds[0],
                                             &record->position, KF_AUDIO_MAX_VOLUME);
            break;
        case KF_EFFECT_KIND_EMERGING_PROJECTILE:
            record->animation_clip = 0;
            record->base_render_id = 9;
            record->render_id = 9;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            record->phase = KF_EFFECT_PROJECTILE_EMERGE_FIRST;
            record->scale_z = 0x5dc;
            record->scale_y = 0x5dc;
            record->scale_x = 0x5dc;
            record->position.vy += 3500;
            break;
        case 0xe:
            record->animation_clip = 0;
            record->base_render_id = 4;
            record->render_id = 4;
            record->rotation = *(const SVECTOR *)va[1];
            break;
        case 0x30:
            record->animation_clip = 0;
            record->base_render_id = 5;
            record->render_id = 5;
            record->rotation = *(const SVECTOR *)va[1];
            break;
        case KF_EFFECT_KIND_LIGHT_NEEDLE:
            record->animation_clip = 0;
            record->base_render_id = 6;
            record->render_id = 6;
            record->rotation = *(const SVECTOR *)va[1];
            record->rotation.vx = -record->rotation.vx;
            if (va[2] != 0) {
                audio_play_spatial_default_range(&magic->sounds[0],
                                                 &record->position, KF_AUDIO_MAX_VOLUME);
            }
            break;
        case 0x16:
            record->animation_clip = 0;
            record->base_render_id = 0xe;
            record->render_id = 0xe;
            record->rotation = *(const SVECTOR *)va[1];
            record->rotation.vx = -record->rotation.vx;
            if (va[2] != 0) {
                audio_play_spatial_default_range(&magic->sounds[0],
                                                 &record->position, KF_AUDIO_MAX_VOLUME);
            }
            break;
        case KF_EFFECT_KIND_SWINGING_HAZARD_SHORT:
            record->animation_clip = 0;
            record->base_render_id = 7;
            record->render_id = 7;
            record->rotation = *(const SVECTOR *)va[1];
            record->rotation.vx = KF_ANGLE_EIGHTH_TURN;
            record->direction.words.z = 0;
            record->direction.words.y = 0;
            record->direction.words.x = 0;
            record->scale_z = 0xa28;
            record->scale_y = 0xa28;
            record->scale_x = 0xa28;
            break;
        case KF_EFFECT_KIND_SWINGING_HAZARD_LONG:
            record->animation_clip = 0;
            record->base_render_id = 7;
            record->render_id = 7;
            record->rotation = *(const SVECTOR *)va[1];
            record->rotation.vx = KF_ANGLE_EIGHTH_TURN;
            record->direction.words.z = 0;
            record->direction.words.y = 0;
            record->direction.words.x = 0;
            break;
        case KF_EFFECT_KIND_ORBITING_PROJECTILE:
            record->animation_clip = 0;
            record->base_render_id = 9;
            record->render_id = 9;
            record->rotation.vz = 0;
            record->rotation.vy = 0;
            record->rotation.vx = 0;
            record->control.orbit_angle = 0;
            record->scale_z = 0xaf0;
            record->scale_y = 0xaf0;
            record->scale_x = 0xaf0;
            record->direction.words.x = record->position.vx >> 8;
            record->direction.words.z = record->position.vz >> 8;
            record->direction.words.y = (u16)record->position.vy;
            break;
        case 0x24:
            record->animation_clip = 0;
            record->base_render_id = 0xa;
            record->render_id = 0xa;
            record->rotation = *(const SVECTOR *)va[1];
            record->control.bytes.high = 0xff;
            record->control.bytes.low = 0xff;
            record->rotation.vx = -record->rotation.vx;
            audio_play_spatial_range(
                &magic_records[KF_EFFECT_KIND_RADIAL_BLAST].sounds[1],
                &record->position, KF_AUDIO_MAX_VOLUME, 0x4e20, 0xea60);
            break;
        case KF_EFFECT_KIND_GROUND_TRAIL:
            record->animation_clip = KF_EFFECT_ANIMATION_BILLBOARD;
            record->base_render_id = 0xe;
            record->render_id = 0xe;
            {
                u8 argument = *(u8 *)(va + 1);
                record->rotation.vz = 0;
                record->rotation.vy = 0;
                record->rotation.vx = 0;
                record->control.parent_effect_index = argument;
            }
            break;
        case 0x2c:
            record->base_render_id = 0x11;
            record->render_id = 0x11;
            record->kind = KF_EFFECT_KIND_RADIAL_BLAST;
            record->animation_clip = 0;
            if (va[1] != 0) {
                audio_play_spatial_range(
                    &magic_records[KF_EFFECT_KIND_RADIAL_BLAST].sounds[0],
                    &record->position, KF_AUDIO_MAX_VOLUME, 0x4e20, 0xea60);
            }
            break;
        case KF_EFFECT_KIND_RADIAL_BLAST:
            record->base_render_id = 0xb;
            record->render_id = 0xb;
            record->animation_clip = 0;
            if (va[1] != 0) {
                audio_play_spatial_range(
                    &magic_records[KF_EFFECT_KIND_RADIAL_BLAST].sounds[0],
                    &record->position, KF_AUDIO_MAX_VOLUME, 0x4e20, 0xea60);
            }
            break;
        case 0x18:
            record->base_render_id = 0x10;
            record->render_id = 0x10;
            record->kind = KF_EFFECT_KIND_HOMING_PROJECTILE;
            record->animation_clip = 0;
            record->rotation = *(const SVECTOR *)va[1];
            record->rotation.vx = -record->rotation.vx;
            record->control.target_mode = *(u8 *)(va + 2);
            record->direction.vector = record->rotation;
            record->scale_y = 0x800;
            record->scale_x = 0x800;
            if (va[3] != 0) {
                audio_play_spatial_default_range(
                    &magic_records[KF_EFFECT_KIND_HOMING_PROJECTILE].sounds[0],
                    &record->position, KF_AUDIO_MAX_VOLUME);
            }
            break;
        case KF_EFFECT_KIND_HOMING_PROJECTILE:
            record->base_render_id = 0xc;
            record->render_id = 0xc;
            record->animation_clip = 0;
            record->rotation = *(const SVECTOR *)va[1];
            record->rotation.vx = -record->rotation.vx;
            record->control.target_mode = *(u8 *)(va + 2);
            record->direction.vector = record->rotation;
            record->scale_y = 0x800;
            record->scale_x = 0x800;
            if (va[3] != 0) {
                audio_play_spatial_default_range(
                    &magic_records[KF_EFFECT_KIND_HOMING_PROJECTILE].sounds[0],
                    &record->position, KF_AUDIO_MAX_VOLUME);
            }
            break;
        case 0x15:
            record->animation_clip = 0;
            record->base_render_id = 0xd;
            record->render_id = 0xd;
            record->scale_y = 0;
            record->scale_z = 0x1800;
            record->scale_x = 0x1800;
            record->rotation.vx = 0;
            record->rotation.vy = player_state.camera_rotation.vy;
            record->rotation.vz = 0;
            break;
        default:
            record->type = KF_EFFECT_SLOT_FREE;
            break;
        }
    }

    return record;
}

ADDRESS(0x80037770, 0xac)
KfEffectRecord *effect_pool_spawn_typed(
    u16 first_segment, u16 segment_count, u16 progress_per_update, u16 cell_stagger,
    s32 sweep_updates, s32 hold_countdown)
{
    KfEffectRecord *record = effect_pool_find_free();
    if (record != 0) {
        record->rotation.vx = first_segment;
        record->rotation.vy = segment_count;
        record->rotation.vz = progress_per_update;
        record->position.vx = sweep_updates;
        record->position.vy = hold_countdown;
        record->direction.words.y = cell_stagger;
        record->base_render_id = KF_EFFECT_RENDER_NONE;
        record->render_id = KF_EFFECT_RENDER_NONE;
        record->kind = KF_EFFECT_KIND_FLOOR_DEFORMATION;
        record->type = 0xf0;
        record->phase = KF_EFFECT_FLOOR_DEFORM_ADVANCE;
        record->direction.words.x = sweep_updates;
        record->direction.words.z = 0;
    }
    return record;
}

ADDRESS(0x8003781c, 0x34)
void effect_pool_set_current(KfEffectRecord *record)
{
    current_effect = record;
    current_effect_magic_record = &magic_records[record->kind];
}
