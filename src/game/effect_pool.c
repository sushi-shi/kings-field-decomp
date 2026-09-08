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

enum {
    EFFECT_WIND_CUTTER_PITCH = 850,
    EFFECT_GROUND_BRANCH_DELAY = 6,
    EFFECT_GROUND_BRANCH_SOUND_VOLUME = 120,
    EFFECT_EMERGING_INITIAL_SCALE = 1500,
    EFFECT_SHORT_SWING_SCALE = 2600,
    EFFECT_ORBIT_SCALE = 2800,
    EFFECT_EXTENDED_SOUND_MAX_DISTANCE = 20000,
    EFFECT_EXTENDED_SOUND_ATTENUATION_DISTANCE = 60000
};

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
    u8 id, u8 type, KfEffectKind kind, const VECTOR *position,
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

        magic = &magic_records[KF_ENUM_ENCODE(u8, record->kind)];

        switch (record->kind) {
        case KF_EFFECT_KIND_FIRE_BALL:
            record->animation_clip = KF_EFFECT_ANIMATION_BILLBOARD;
            record->base_render_id = KF_EFFECT_BILLBOARD_FIRE_BALL;
            record->render_id = KF_EFFECT_BILLBOARD_FIRE_BALL;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            audio_play_spatial_default_range(&magic->sounds[0],
                                             &record->position, KF_AUDIO_MAX_VOLUME);
            break;
        case KF_EFFECT_KIND_WIND_CUTTER:
            record->animation_clip = KF_EFFECT_ANIMATION_BILLBOARD;
            record->base_render_id = KF_EFFECT_BILLBOARD_WIND_CUTTER;
            record->render_id = KF_EFFECT_BILLBOARD_WIND_CUTTER;
            record->rotation.vx = EFFECT_WIND_CUTTER_PITCH;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            if (va[2] != 0) {
                audio_play_spatial_default_range(&magic->sounds[0],
                                                 &record->position, KF_AUDIO_MAX_VOLUME);
            }
            break;
        case KF_EFFECT_KIND_LIGHTNING_BOLT_ALTERNATE:
            record->base_render_id = KF_EFFECT_BILLBOARD_LIGHTNING_BOLT_ALTERNATE;
            record->render_id = KF_EFFECT_BILLBOARD_LIGHTNING_BOLT_ALTERNATE;
            record->kind = KF_EFFECT_KIND_LIGHTNING_BOLT;
            goto initialize_lightning_bolt;
        case KF_EFFECT_KIND_LIGHTNING_BOLT:
            record->base_render_id = KF_EFFECT_BILLBOARD_LIGHTNING_BOLT;
            record->render_id = KF_EFFECT_BILLBOARD_LIGHTNING_BOLT;
        initialize_lightning_bolt:
            record->animation_clip = KF_EFFECT_ANIMATION_BILLBOARD;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            record->control.frames_remaining = *(u16 *)(va + 1);
            if (va[2] != 0) {
                audio_play_spatial_range(
                    &magic_records[KF_ENUM_ENCODE(u8, KF_MAGIC_LIGHTNING_BOLT)].sounds[0],
                    &record->position, KF_AUDIO_MAX_VOLUME,
                    EFFECT_EXTENDED_SOUND_MAX_DISTANCE,
                    EFFECT_EXTENDED_SOUND_ATTENUATION_DISTANCE);
            }
            break;
        case KF_EFFECT_KIND_LIGHTNING_IMPACT_ALTERNATE:
            record->base_render_id = KF_EFFECT_BILLBOARD_LIGHTNING_IMPACT_ALTERNATE;
            record->render_id = KF_EFFECT_BILLBOARD_LIGHTNING_IMPACT_ALTERNATE;
            record->kind = KF_EFFECT_KIND_LIGHTNING_IMPACT;
            goto initialize_lightning_impact;
        case KF_EFFECT_KIND_LIGHTNING_IMPACT:
            record->base_render_id = KF_EFFECT_BILLBOARD_LIGHTNING_IMPACT;
            record->render_id = KF_EFFECT_BILLBOARD_LIGHTNING_IMPACT;
        initialize_lightning_impact:
            record->animation_clip = KF_EFFECT_ANIMATION_BILLBOARD;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            audio_play_spatial_range(
                &magic_records[KF_ENUM_ENCODE(u8, KF_MAGIC_LIGHTNING_BOLT)].sounds[1],
                &record->position, KF_AUDIO_MAX_VOLUME,
                EFFECT_EXTENDED_SOUND_MAX_DISTANCE,
                EFFECT_EXTENDED_SOUND_ATTENUATION_DISTANCE);
            break;
        case KF_EFFECT_KIND_LIGHTNING_RADIAL_BLAST_ALTERNATE:
            record->base_render_id = KF_EFFECT_MODEL_LIGHTNING_RADIAL_BLAST_ALTERNATE;
            record->render_id = KF_EFFECT_MODEL_LIGHTNING_RADIAL_BLAST_ALTERNATE;
            record->kind = KF_EFFECT_KIND_LIGHTNING_RADIAL_BLAST;
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            break;
        case KF_EFFECT_KIND_LIGHTNING_RADIAL_BLAST:
            record->base_render_id = KF_EFFECT_MODEL_LIGHTNING_RADIAL_BLAST;
            record->render_id = KF_EFFECT_MODEL_LIGHTNING_RADIAL_BLAST;
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            break;
        case KF_EFFECT_KIND_GROUND_BRANCH:
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            record->base_render_id = KF_EFFECT_MODEL_GROUND_BRANCH;
            record->render_id = KF_EFFECT_MODEL_GROUND_BRANCH;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            record->scale_y = 0;
            {
                KfEffectGroundBranchRole branch_role = KF_ENUM_DECODE(KfEffectGroundBranchRole, *(u16 *)(va + 1));
                record->control.frames_remaining = EFFECT_GROUND_BRANCH_DELAY;
                record->propagation.branch = branch_role;
            }
            if (record->propagation.branch != KF_EFFECT_GROUND_BRANCH_LEAF) {
                sound_ref_play(&magic->sounds[0], EFFECT_GROUND_BRANCH_SOUND_VOLUME);
            }
            break;
        case KF_EFFECT_KIND_GROUND_BRANCH_VISUAL:
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            record->base_render_id = KF_EFFECT_MODEL_GROUND_BRANCH_VISUAL;
            record->render_id = KF_EFFECT_MODEL_GROUND_BRANCH_VISUAL;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            record->scale_y = 0;
            break;
        case KF_EFFECT_KIND_ACTOR_SPAWNER:
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            record->base_render_id = KF_EFFECT_MODEL_ACTOR_SPAWNER;
            record->render_id = KF_EFFECT_MODEL_ACTOR_SPAWNER;
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
            record->base_render_id = KF_EFFECT_BILLBOARD_SCATTER_PROJECTILE;
            record->render_id = KF_EFFECT_BILLBOARD_SCATTER_PROJECTILE;
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
            record->base_render_id = KF_EFFECT_BILLBOARD_DARKNESS_PROJECTILE;
            record->render_id = KF_EFFECT_BILLBOARD_DARKNESS_PROJECTILE;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            audio_play_spatial_default_range(&magic->sounds[0],
                                             &record->position, KF_AUDIO_MAX_VOLUME);
            break;
        case KF_EFFECT_KIND_CURSE_PROJECTILE:
            record->animation_clip = KF_EFFECT_ANIMATION_BILLBOARD;
            record->base_render_id = KF_EFFECT_BILLBOARD_CURSE_PROJECTILE;
            record->render_id = KF_EFFECT_BILLBOARD_CURSE_PROJECTILE;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            audio_play_spatial_default_range(&magic->sounds[0],
                                             &record->position, KF_AUDIO_MAX_VOLUME);
            break;
        case KF_EFFECT_KIND_EMERGING_PROJECTILE:
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            record->base_render_id = KF_EFFECT_MODEL_EMERGING_PROJECTILE;
            record->render_id = KF_EFFECT_MODEL_EMERGING_PROJECTILE;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            record->phase = KF_EFFECT_PROJECTILE_EMERGE_FIRST;
            record->scale_z = EFFECT_EMERGING_INITIAL_SCALE;
            record->scale_y = EFFECT_EMERGING_INITIAL_SCALE;
            record->scale_x = EFFECT_EMERGING_INITIAL_SCALE;
            record->position.vy += KF_EFFECT_EMERGE_DEPTH;
            break;
        case KF_EFFECT_KIND_MAP_EMITTER_PROJECTILE:
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            record->base_render_id = KF_EFFECT_MODEL_MAP_EMITTER_PROJECTILE;
            record->render_id = KF_EFFECT_MODEL_MAP_EMITTER_PROJECTILE;
            record->rotation = *(const SVECTOR *)va[1];
            break;
        case KF_EFFECT_KIND_MAP_SWITCH:
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            record->base_render_id = KF_EFFECT_MODEL_MAP_SWITCH;
            record->render_id = KF_EFFECT_MODEL_MAP_SWITCH;
            record->rotation = *(const SVECTOR *)va[1];
            break;
        case KF_EFFECT_KIND_LIGHT_NEEDLE:
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            record->base_render_id = KF_EFFECT_MODEL_LIGHT_NEEDLE;
            record->render_id = KF_EFFECT_MODEL_LIGHT_NEEDLE;
            record->rotation = *(const SVECTOR *)va[1];
            record->rotation.vx = -record->rotation.vx;
            if (va[2] != 0) {
                audio_play_spatial_default_range(&magic->sounds[0],
                                                 &record->position, KF_AUDIO_MAX_VOLUME);
            }
            break;
        case KF_EFFECT_KIND_PHYSICAL_PROJECTILE:
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            record->base_render_id = KF_EFFECT_MODEL_PHYSICAL_PROJECTILE;
            record->render_id = KF_EFFECT_MODEL_PHYSICAL_PROJECTILE;
            record->rotation = *(const SVECTOR *)va[1];
            record->rotation.vx = -record->rotation.vx;
            if (va[2] != 0) {
                audio_play_spatial_default_range(&magic->sounds[0],
                                                 &record->position, KF_AUDIO_MAX_VOLUME);
            }
            break;
        case KF_EFFECT_KIND_SWINGING_HAZARD_SHORT:
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            record->base_render_id = KF_EFFECT_MODEL_SWINGING_HAZARD;
            record->render_id = KF_EFFECT_MODEL_SWINGING_HAZARD;
            record->rotation = *(const SVECTOR *)va[1];
            record->rotation.vx = KF_ANGLE_EIGHTH_TURN;
            record->direction.words.z = 0;
            record->direction.words.y = 0;
            record->direction.words.x = 0;
            record->scale_z = EFFECT_SHORT_SWING_SCALE;
            record->scale_y = EFFECT_SHORT_SWING_SCALE;
            record->scale_x = EFFECT_SHORT_SWING_SCALE;
            break;
        case KF_EFFECT_KIND_SWINGING_HAZARD_LONG:
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            record->base_render_id = KF_EFFECT_MODEL_SWINGING_HAZARD;
            record->render_id = KF_EFFECT_MODEL_SWINGING_HAZARD;
            record->rotation = *(const SVECTOR *)va[1];
            record->rotation.vx = KF_ANGLE_EIGHTH_TURN;
            record->direction.words.z = 0;
            record->direction.words.y = 0;
            record->direction.words.x = 0;
            break;
        case KF_EFFECT_KIND_ORBITING_PROJECTILE:
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            record->base_render_id = KF_EFFECT_MODEL_EMERGING_PROJECTILE;
            record->render_id = KF_EFFECT_MODEL_EMERGING_PROJECTILE;
            record->rotation.vz = 0;
            record->rotation.vy = 0;
            record->rotation.vx = 0;
            record->control.orbit_angle = 0;
            record->scale_z = EFFECT_ORBIT_SCALE;
            record->scale_y = EFFECT_ORBIT_SCALE;
            record->scale_x = EFFECT_ORBIT_SCALE;
            record->direction.words.x = record->position.vx >> KF_EFFECT_ORBIT_CENTER_SHIFT;
            record->direction.words.z = record->position.vz >> KF_EFFECT_ORBIT_CENTER_SHIFT;
            record->direction.words.y = (u16)record->position.vy;
            break;
        case KF_EFFECT_KIND_MOONLIGHT_PROJECTILE:
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            record->base_render_id = KF_EFFECT_MODEL_MOONLIGHT_PROJECTILE;
            record->render_id = KF_EFFECT_MODEL_MOONLIGHT_PROJECTILE;
            record->rotation = *(const SVECTOR *)va[1];
            record->control.bytes.high = 0xff;
            record->control.bytes.low = 0xff;
            record->rotation.vx = -record->rotation.vx;
            audio_play_spatial_range(
                &magic_records[KF_ENUM_ENCODE(u8, KF_EFFECT_KIND_RADIAL_BLAST)].sounds[1],
                &record->position, KF_AUDIO_MAX_VOLUME,
                EFFECT_EXTENDED_SOUND_MAX_DISTANCE,
                EFFECT_EXTENDED_SOUND_ATTENUATION_DISTANCE);
            break;
        case KF_EFFECT_KIND_GROUND_TRAIL:
            record->animation_clip = KF_EFFECT_ANIMATION_BILLBOARD;
            record->base_render_id = KF_EFFECT_BILLBOARD_GROUND_TRAIL;
            record->render_id = KF_EFFECT_BILLBOARD_GROUND_TRAIL;
            {
                u8 argument = *(u8 *)(va + 1);
                record->rotation.vz = 0;
                record->rotation.vy = 0;
                record->rotation.vx = 0;
                record->control.parent_effect_index = argument;
            }
            break;
        case KF_EFFECT_KIND_RADIAL_BLAST_ALTERNATE:
            record->base_render_id = KF_EFFECT_MODEL_RADIAL_BLAST_ALTERNATE;
            record->render_id = KF_EFFECT_MODEL_RADIAL_BLAST_ALTERNATE;
            record->kind = KF_EFFECT_KIND_RADIAL_BLAST;
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            if (va[1] != 0) {
                audio_play_spatial_range(
                    &magic_records[KF_ENUM_ENCODE(u8, KF_EFFECT_KIND_RADIAL_BLAST)].sounds[0],
                    &record->position, KF_AUDIO_MAX_VOLUME,
                    EFFECT_EXTENDED_SOUND_MAX_DISTANCE,
                    EFFECT_EXTENDED_SOUND_ATTENUATION_DISTANCE);
            }
            break;
        case KF_EFFECT_KIND_RADIAL_BLAST:
            record->base_render_id = KF_EFFECT_MODEL_RADIAL_BLAST;
            record->render_id = KF_EFFECT_MODEL_RADIAL_BLAST;
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            if (va[1] != 0) {
                audio_play_spatial_range(
                    &magic_records[KF_ENUM_ENCODE(u8, KF_EFFECT_KIND_RADIAL_BLAST)].sounds[0],
                    &record->position, KF_AUDIO_MAX_VOLUME,
                    EFFECT_EXTENDED_SOUND_MAX_DISTANCE,
                    EFFECT_EXTENDED_SOUND_ATTENUATION_DISTANCE);
            }
            break;
        case KF_EFFECT_KIND_HOMING_PROJECTILE_ALTERNATE:
            record->base_render_id = KF_EFFECT_MODEL_HOMING_PROJECTILE_ALTERNATE;
            record->render_id = KF_EFFECT_MODEL_HOMING_PROJECTILE_ALTERNATE;
            record->kind = KF_EFFECT_KIND_HOMING_PROJECTILE;
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            record->rotation = *(const SVECTOR *)va[1];
            record->rotation.vx = -record->rotation.vx;
            record->control.target_mode = KF_ENUM_DECODE(KfEffectHomingMode, *(u8 *)(va + 2));
            record->direction.vector = record->rotation;
            record->scale_y = KF_FIXED12_ONE / 2;
            record->scale_x = KF_FIXED12_ONE / 2;
            if (va[3] != 0) {
                audio_play_spatial_default_range(
                    &magic_records[KF_ENUM_ENCODE(u8, KF_EFFECT_KIND_HOMING_PROJECTILE)].sounds[0],
                    &record->position, KF_AUDIO_MAX_VOLUME);
            }
            break;
        case KF_EFFECT_KIND_HOMING_PROJECTILE:
            record->base_render_id = KF_EFFECT_MODEL_HOMING_PROJECTILE;
            record->render_id = KF_EFFECT_MODEL_HOMING_PROJECTILE;
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            record->rotation = *(const SVECTOR *)va[1];
            record->rotation.vx = -record->rotation.vx;
            record->control.target_mode = KF_ENUM_DECODE(KfEffectHomingMode, *(u8 *)(va + 2));
            record->direction.vector = record->rotation;
            record->scale_y = KF_FIXED12_ONE / 2;
            record->scale_x = KF_FIXED12_ONE / 2;
            if (va[3] != 0) {
                audio_play_spatial_default_range(
                    &magic_records[KF_ENUM_ENCODE(u8, KF_EFFECT_KIND_HOMING_PROJECTILE)].sounds[0],
                    &record->position, KF_AUDIO_MAX_VOLUME);
            }
            break;
        case KF_EFFECT_KIND_WARP_SHIMMER:
            record->animation_clip = KF_EFFECT_ANIMATION_FIRST_CLIP;
            record->base_render_id = KF_EFFECT_MODEL_WARP_SHIMMER;
            record->render_id = KF_EFFECT_MODEL_WARP_SHIMMER;
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
        record->type = KF_EFFECT_FLOOR_DEFORM_TYPE;
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
    current_effect_magic_record = &magic_records[KF_ENUM_ENCODE(u8, record->kind)];
}
