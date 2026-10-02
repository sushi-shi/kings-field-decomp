#include <kf/platform/prelude.h>
#include <kf/game/audio.h>
#include <kf/game/effect.h>
#include <kf/game/game.h>
#include <kf/lib/null.h>

enum {
    EFFECT_WIND_CUTTER_PITCH = 850,
    EFFECT_GROUND_BRANCH_DELAY = 6,
    EFFECT_GROUND_BRANCH_SOUND_VOLUME = 120,
    EFFECT_EMERGING_INITIAL_SCALE = 1500,
    EFFECT_SHORT_SWING_SCALE = 2600,
    EFFECT_ORBIT_SCALE = 2800,
    EFFECT_WARP_HORIZONTAL_SCALE = 0x1800
};

KfEffectState effect_state;

KfEffectRecord *effect_pool_find_free(void)
{
    for (auto &record : effect_state.records) {
        if (record.type == KF_EFFECT_SLOT_FREE) {
            return &record;
        }
    }
    return NULL;
}

static KfEffectRecord *effect_begin(u8 id, KfEffectType type, KfEffectKind kind,
    const VECTOR &position, const SVECTOR &direction)
{
    auto *record = effect_pool_find_free();
    if (!record)
        return nullptr;
    record->type = type;
    record->kind = kind;
    record->position = position;
    record->direction = direction;
    record->phase = KF_EFFECT_PHASE_INIT;
    record->id = id;
    record->scale_z = KF_FIXED12_ONE;
    record->scale_y = KF_FIXED12_ONE;
    record->scale_x = KF_FIXED12_ONE;
    record->animation_phase = 0;
    record->sound_played = KF_AUDIO_NOT_PLAYED;

    return record;
}

KfEffectRecord *effect_spawn_fire_ball(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction)
{
    auto *record = effect_begin(id, type, KF_MAGIC_FIRE_BALL, position, direction);
    if (!record)
        return nullptr;
    const auto &magic = effect_state.magic.entries[kf_enum_encode<u8>(KF_MAGIC_FIRE_BALL)];
    record->animation_clip = KF_ANIMATION_CLIP_NONE;
    record->base_render_id = KF_EFFECT_BILLBOARD_FIRE_BALL;
    record->render_id = KF_EFFECT_BILLBOARD_FIRE_BALL;
    record->rotation = {};
    audio_play_spatial_default_range(&magic.sounds[0],
                                     &record->position, KF_AUDIO_MAX_VOLUME);
    return record;
}

KfEffectRecord *effect_spawn_wind_cutter(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, KfEffectSoundRequest sound)
{
    auto *record = effect_begin(id, type, KF_MAGIC_WIND_CUTTER, position, direction);
    if (!record)
        return nullptr;
    const auto &magic = effect_state.magic.entries[kf_enum_encode<u8>(KF_MAGIC_WIND_CUTTER)];
    record->animation_clip = KF_ANIMATION_CLIP_NONE;
    record->base_render_id = KF_EFFECT_BILLBOARD_WIND_CUTTER;
    record->render_id = KF_EFFECT_BILLBOARD_WIND_CUTTER;
    record->rotation = {EFFECT_WIND_CUTTER_PITCH, 0, 0};
    if (sound != KF_EFFECT_SOUND_SILENT) {
        audio_play_spatial_default_range(&magic.sounds[0],
                                         &record->position, KF_AUDIO_MAX_VOLUME);
    }
    return record;
}

KfEffectRecord *effect_spawn_lightning_bolt(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, KfEffectVariant variant, s32 duration, KfEffectSoundRequest sound)
{
    const auto kind = variant == KfEffectVariant::Alternate ? KF_EFFECT_KIND_LIGHTNING_BOLT_ALTERNATE : KF_MAGIC_LIGHTNING_BOLT;
    auto *record = effect_begin(id, type, kind, position, direction);
    if (!record)
        return nullptr;
    record->base_render_id = record->kind == KF_EFFECT_KIND_LIGHTNING_BOLT_ALTERNATE
        ? KF_EFFECT_BILLBOARD_LIGHTNING_BOLT_ALTERNATE : KF_EFFECT_BILLBOARD_LIGHTNING_BOLT;
    record->render_id = record->base_render_id;
    record->kind = KF_MAGIC_LIGHTNING_BOLT;
    record->animation_clip = KF_ANIMATION_CLIP_NONE;
    record->rotation = {};
    record->frames_remaining = duration;
    if (sound != KF_EFFECT_SOUND_SILENT) {
        audio_play_spatial_range(
            &effect_state.magic.entries[kf_enum_encode<u8>(KF_MAGIC_LIGHTNING_BOLT)].sounds[0],
            &record->position, KF_AUDIO_MAX_VOLUME,
            KF_AUDIO_EXTENDED_MAX_DISTANCE,
            KF_AUDIO_EXTENDED_ATTENUATION_DISTANCE);
    }
    return record;
}

KfEffectRecord *effect_spawn_lightning_impact(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, KfEffectVariant variant)
{
    const auto kind = variant == KfEffectVariant::Alternate ? KF_EFFECT_KIND_LIGHTNING_IMPACT_ALTERNATE : KF_EFFECT_KIND_LIGHTNING_IMPACT;
    auto *record = effect_begin(id, type, kind, position, direction);
    if (!record)
        return nullptr;
    record->base_render_id = record->kind == KF_EFFECT_KIND_LIGHTNING_IMPACT_ALTERNATE
        ? KF_EFFECT_BILLBOARD_LIGHTNING_IMPACT_ALTERNATE : KF_EFFECT_BILLBOARD_LIGHTNING_IMPACT;
    record->render_id = record->base_render_id;
    record->kind = KF_EFFECT_KIND_LIGHTNING_IMPACT;
    record->animation_clip = KF_ANIMATION_CLIP_NONE;
    record->rotation = {};
    audio_play_spatial_range(
        &effect_state.magic.entries[kf_enum_encode<u8>(KF_MAGIC_LIGHTNING_BOLT)].sounds[1],
        &record->position, KF_AUDIO_MAX_VOLUME,
        KF_AUDIO_EXTENDED_MAX_DISTANCE,
        KF_AUDIO_EXTENDED_ATTENUATION_DISTANCE);
    return record;
}

KfEffectRecord *effect_spawn_lightning_radial_blast(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, KfEffectVariant variant)
{
    const auto kind = variant == KfEffectVariant::Alternate ? KF_EFFECT_KIND_LIGHTNING_RADIAL_BLAST_ALTERNATE : KF_EFFECT_KIND_LIGHTNING_RADIAL_BLAST;
    auto *record = effect_begin(id, type, kind, position, direction);
    if (!record)
        return nullptr;
    record->kind = KF_EFFECT_KIND_LIGHTNING_RADIAL_BLAST;
    record->base_render_id = variant == KfEffectVariant::Alternate
        ? KF_EFFECT_MODEL_LIGHTNING_RADIAL_BLAST_ALTERNATE : KF_EFFECT_MODEL_LIGHTNING_RADIAL_BLAST;
    record->render_id = record->base_render_id;
    record->animation_clip = KF_ANIMATION_CLIP_FIRST;
    record->rotation = {};
    return record;
}

KfEffectRecord *effect_spawn_fire_wall(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, KfEffectGroundBranchRole role)
{
    auto *record = effect_begin(id, type, KF_MAGIC_FIRE_WALL, position, direction);
    if (!record)
        return nullptr;
    const auto &magic = effect_state.magic.entries[kf_enum_encode<u8>(KF_MAGIC_FIRE_WALL)];
    record->animation_clip = KF_ANIMATION_CLIP_FIRST;
    record->base_render_id = KF_EFFECT_MODEL_GROUND_BRANCH;
    record->render_id = KF_EFFECT_MODEL_GROUND_BRANCH;
    record->rotation = {};
    record->scale_y = 0;
    {
        KfEffectGroundBranchRole branch_role = role;
        record->frames_remaining = EFFECT_GROUND_BRANCH_DELAY;
        record->branch_role = branch_role;
    }
    if (record->branch_role != KF_EFFECT_GROUND_BRANCH_LEAF) {
        sound_ref_play(audio_playback(), &magic.sounds[0], EFFECT_GROUND_BRANCH_SOUND_VOLUME);
    }
    return record;
}

KfEffectRecord *effect_spawn_ground_branch_visual(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction)
{
    auto *record = effect_begin(id, type, KF_EFFECT_KIND_GROUND_BRANCH_VISUAL, position, direction);
    if (!record)
        return nullptr;
    record->animation_clip = KF_ANIMATION_CLIP_FIRST;
    record->base_render_id = KF_EFFECT_MODEL_GROUND_BRANCH_VISUAL;
    record->render_id = KF_EFFECT_MODEL_GROUND_BRANCH_VISUAL;
    record->rotation = {};
    record->scale_y = 0;
    return record;
}

KfEffectRecord *effect_spawn_actor_spawner(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, s32 duration)
{
    auto *record = effect_begin(id, type, KF_EFFECT_KIND_ACTOR_SPAWNER, position, direction);
    if (!record)
        return nullptr;
    const auto &magic = effect_state.magic.entries[kf_enum_encode<u8>(KF_EFFECT_KIND_ACTOR_SPAWNER)];
    record->animation_clip = KF_ANIMATION_CLIP_FIRST;
    record->base_render_id = KF_EFFECT_MODEL_ACTOR_SPAWNER;
    record->render_id = KF_EFFECT_MODEL_ACTOR_SPAWNER;
    record->rotation = {};
    record->scale_z = 0;
    record->scale_y = 0;
    record->scale_x = 0;
    record->frames_remaining = duration;
    audio_play_spatial_default_range(&magic.sounds[0],
                                     &record->position, KF_AUDIO_MAX_VOLUME);
    return record;
}

KfEffectRecord *effect_spawn_scatter_projectile(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, s32 generations, s32 duration, s32 scale)
{
    auto *record = effect_begin(id, type, KF_EFFECT_KIND_SCATTER_PROJECTILE, position, direction);
    if (!record)
        return nullptr;
    const auto &magic = effect_state.magic.entries[kf_enum_encode<u8>(KF_EFFECT_KIND_SCATTER_PROJECTILE)];
    record->animation_clip = KF_ANIMATION_CLIP_NONE;
    record->base_render_id = KF_EFFECT_BILLBOARD_SCATTER_PROJECTILE;
    record->render_id = KF_EFFECT_BILLBOARD_SCATTER_PROJECTILE;
    record->rotation = {};
    record->generations_remaining = generations;
    record->frames_remaining = duration;
    record->scale_x = record->scale_y = record->scale_z =
        record->pulse_base_scale = scale;
    audio_play_spatial_default_range(&magic.sounds[0],
                                     &record->position, KF_AUDIO_MAX_VOLUME);
    return record;
}

KfEffectRecord *effect_spawn_darkness_projectile(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction)
{
    auto *record = effect_begin(id, type, KF_EFFECT_KIND_DARKNESS_PROJECTILE, position, direction);
    if (!record)
        return nullptr;
    const auto &magic = effect_state.magic.entries[kf_enum_encode<u8>(KF_EFFECT_KIND_DARKNESS_PROJECTILE)];
    record->animation_clip = KF_ANIMATION_CLIP_NONE;
    record->base_render_id = KF_EFFECT_BILLBOARD_DARKNESS_PROJECTILE;
    record->render_id = KF_EFFECT_BILLBOARD_DARKNESS_PROJECTILE;
    record->rotation = {};
    audio_play_spatial_default_range(&magic.sounds[0],
                                     &record->position, KF_AUDIO_MAX_VOLUME);
    return record;
}

KfEffectRecord *effect_spawn_curse_projectile(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction)
{
    auto *record = effect_begin(id, type, KF_EFFECT_KIND_CURSE_PROJECTILE, position, direction);
    if (!record)
        return nullptr;
    const auto &magic = effect_state.magic.entries[kf_enum_encode<u8>(KF_EFFECT_KIND_CURSE_PROJECTILE)];
    record->animation_clip = KF_ANIMATION_CLIP_NONE;
    record->base_render_id = KF_EFFECT_BILLBOARD_CURSE_PROJECTILE;
    record->render_id = KF_EFFECT_BILLBOARD_CURSE_PROJECTILE;
    record->rotation = {};
    audio_play_spatial_default_range(&magic.sounds[0],
                                     &record->position, KF_AUDIO_MAX_VOLUME);
    return record;
}

KfEffectRecord *effect_spawn_emerging_projectile(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction)
{
    auto *record = effect_begin(id, type, KF_EFFECT_KIND_EMERGING_PROJECTILE, position, direction);
    if (!record)
        return nullptr;
    record->animation_clip = KF_ANIMATION_CLIP_FIRST;
    record->base_render_id = KF_EFFECT_MODEL_EMERGING_PROJECTILE;
    record->render_id = KF_EFFECT_MODEL_EMERGING_PROJECTILE;
    record->rotation = {};
    record->phase = KF_EFFECT_PROJECTILE_EMERGE_FIRST;
    record->scale_z = EFFECT_EMERGING_INITIAL_SCALE;
    record->scale_y = EFFECT_EMERGING_INITIAL_SCALE;
    record->scale_x = EFFECT_EMERGING_INITIAL_SCALE;
    record->position.vy += KF_EFFECT_EMERGE_DEPTH;
    return record;
}

KfEffectRecord *effect_spawn_map_emitter_projectile(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, const SVECTOR &rotation)
{
    auto *record = effect_begin(id, type, KF_EFFECT_KIND_MAP_EMITTER_PROJECTILE, position, direction);
    if (!record)
        return nullptr;
    record->animation_clip = KF_ANIMATION_CLIP_FIRST;
    record->base_render_id = KF_EFFECT_MODEL_MAP_EMITTER_PROJECTILE;
    record->render_id = KF_EFFECT_MODEL_MAP_EMITTER_PROJECTILE;
    record->rotation = rotation;
    return record;
}

KfEffectRecord *effect_spawn_map_switch(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, const SVECTOR &rotation)
{
    auto *record = effect_begin(id, type, KF_EFFECT_KIND_MAP_SWITCH, position, direction);
    if (!record)
        return nullptr;
    record->animation_clip = KF_ANIMATION_CLIP_FIRST;
    record->base_render_id = KF_EFFECT_MODEL_MAP_SWITCH;
    record->render_id = KF_EFFECT_MODEL_MAP_SWITCH;
    record->rotation = rotation;
    return record;
}

KfEffectRecord *effect_spawn_light_needle(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, const SVECTOR &rotation, KfEffectSoundRequest sound)
{
    auto *record = effect_begin(id, type, KF_MAGIC_LIGHT_NEEDLE, position, direction);
    if (!record)
        return nullptr;
    const auto &magic = effect_state.magic.entries[kf_enum_encode<u8>(KF_MAGIC_LIGHT_NEEDLE)];
    record->animation_clip = KF_ANIMATION_CLIP_FIRST;
    record->base_render_id = KF_EFFECT_MODEL_LIGHT_NEEDLE;
    record->render_id = KF_EFFECT_MODEL_LIGHT_NEEDLE;
    record->rotation = rotation;
    record->rotation.vx = -record->rotation.vx;
    if (sound != KF_EFFECT_SOUND_SILENT) {
        audio_play_spatial_default_range(&magic.sounds[0],
                                         &record->position, KF_AUDIO_MAX_VOLUME);
    }
    return record;
}

KfEffectRecord *effect_spawn_physical_projectile(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, const SVECTOR &rotation, KfEffectSoundRequest sound)
{
    auto *record = effect_begin(id, type, KF_EFFECT_KIND_PHYSICAL_PROJECTILE, position, direction);
    if (!record)
        return nullptr;
    const auto &magic = effect_state.magic.entries[kf_enum_encode<u8>(KF_EFFECT_KIND_PHYSICAL_PROJECTILE)];
    record->animation_clip = KF_ANIMATION_CLIP_FIRST;
    record->base_render_id = KF_EFFECT_MODEL_PHYSICAL_PROJECTILE;
    record->render_id = KF_EFFECT_MODEL_PHYSICAL_PROJECTILE;
    record->rotation = rotation;
    record->rotation.vx = -record->rotation.vx;
    if (sound != KF_EFFECT_SOUND_SILENT) {
        audio_play_spatial_default_range(&magic.sounds[0],
                                         &record->position, KF_AUDIO_MAX_VOLUME);
    }
    return record;
}

KfEffectRecord *effect_spawn_swinging_hazard_short(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, const SVECTOR &rotation)
{
    auto *record = effect_begin(id, type, KF_EFFECT_KIND_SWINGING_HAZARD_SHORT, position, direction);
    if (!record)
        return nullptr;
    record->animation_clip = KF_ANIMATION_CLIP_FIRST;
    record->base_render_id = KF_EFFECT_MODEL_SWINGING_HAZARD;
    record->render_id = KF_EFFECT_MODEL_SWINGING_HAZARD;
    record->rotation = rotation;
    record->rotation.vx = KF_ANGLE_EIGHTH_TURN;
    record->pitch_velocity = 0;
    record->scale_z = EFFECT_SHORT_SWING_SCALE;
    record->scale_y = EFFECT_SHORT_SWING_SCALE;
    record->scale_x = EFFECT_SHORT_SWING_SCALE;
    return record;
}

KfEffectRecord *effect_spawn_swinging_hazard_long(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, const SVECTOR &rotation)
{
    auto *record = effect_begin(id, type, KF_EFFECT_KIND_SWINGING_HAZARD_LONG, position, direction);
    if (!record)
        return nullptr;
    record->animation_clip = KF_ANIMATION_CLIP_FIRST;
    record->base_render_id = KF_EFFECT_MODEL_SWINGING_HAZARD;
    record->render_id = KF_EFFECT_MODEL_SWINGING_HAZARD;
    record->rotation = rotation;
    record->rotation.vx = KF_ANGLE_EIGHTH_TURN;
    record->pitch_velocity = 0;
    return record;
}

KfEffectRecord *effect_spawn_orbiting_projectile(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction)
{
    auto *record = effect_begin(id, type, KF_EFFECT_KIND_ORBITING_PROJECTILE, position, direction);
    if (!record)
        return nullptr;
    record->animation_clip = KF_ANIMATION_CLIP_FIRST;
    record->base_render_id = KF_EFFECT_MODEL_EMERGING_PROJECTILE;
    record->render_id = KF_EFFECT_MODEL_EMERGING_PROJECTILE;
    record->rotation = {};
    record->orbit_angle = 0;
    record->scale_z = EFFECT_ORBIT_SCALE;
    record->scale_y = EFFECT_ORBIT_SCALE;
    record->scale_x = EFFECT_ORBIT_SCALE;
    record->orbit_center = {
        static_cast<s16>(record->position.vx >> KF_EFFECT_ORBIT_CENTER_SHIFT),
        static_cast<s16>(record->position.vy),
        static_cast<s16>(record->position.vz >> KF_EFFECT_ORBIT_CENTER_SHIFT)};
    return record;
}

KfEffectRecord *effect_spawn_moonlight_projectile(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, const SVECTOR &rotation)
{
    auto *record = effect_begin(id, type, KF_EFFECT_KIND_MOONLIGHT_PROJECTILE, position, direction);
    if (!record)
        return nullptr;
    record->animation_clip = KF_ANIMATION_CLIP_FIRST;
    record->base_render_id = KF_EFFECT_MODEL_MOONLIGHT_PROJECTILE;
    record->render_id = KF_EFFECT_MODEL_MOONLIGHT_PROJECTILE;
    record->rotation = rotation;
    record->rotation.vx = -record->rotation.vx;
    audio_play_spatial_range(
        &effect_state.magic.entries[kf_enum_encode<u8>(KF_EFFECT_KIND_RADIAL_BLAST)].sounds[1],
        &record->position, KF_AUDIO_MAX_VOLUME,
        KF_AUDIO_EXTENDED_MAX_DISTANCE,
        KF_AUDIO_EXTENDED_ATTENUATION_DISTANCE);
    return record;
}

KfEffectRecord *effect_spawn_ground_trail(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, u8 parent_index)
{
    auto *record = effect_begin(id, type, KF_EFFECT_KIND_GROUND_TRAIL, position, direction);
    if (!record)
        return nullptr;
    record->animation_clip = KF_ANIMATION_CLIP_NONE;
    record->base_render_id = KF_EFFECT_BILLBOARD_GROUND_TRAIL;
    record->render_id = KF_EFFECT_BILLBOARD_GROUND_TRAIL;
    {
        u8 argument = parent_index;
        record->rotation = {};
        record->parent_effect_index = argument;
    }
    return record;
}

KfEffectRecord *effect_spawn_radial_blast(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, KfEffectVariant variant, KfEffectSoundRequest sound)
{
    const auto kind = variant == KfEffectVariant::Alternate ? KF_EFFECT_KIND_RADIAL_BLAST_ALTERNATE : KF_EFFECT_KIND_RADIAL_BLAST;
    auto *record = effect_begin(id, type, kind, position, direction);
    if (!record)
        return nullptr;
    record->kind = KF_EFFECT_KIND_RADIAL_BLAST;
    record->base_render_id = variant == KfEffectVariant::Alternate
        ? KF_EFFECT_MODEL_RADIAL_BLAST_ALTERNATE : KF_EFFECT_MODEL_RADIAL_BLAST;
    record->render_id = record->base_render_id;
    record->animation_clip = KF_ANIMATION_CLIP_FIRST;
    if (sound != KF_EFFECT_SOUND_SILENT) {
        audio_play_spatial_range(
            &effect_state.magic.entries[kf_enum_encode<u8>(KF_EFFECT_KIND_RADIAL_BLAST)].sounds[0],
            &record->position, KF_AUDIO_MAX_VOLUME,
            KF_AUDIO_EXTENDED_MAX_DISTANCE,
            KF_AUDIO_EXTENDED_ATTENUATION_DISTANCE);
    }
    return record;
}

KfEffectRecord *effect_spawn_homing_projectile(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, KfEffectVariant variant, const SVECTOR &rotation, KfEffectHomingMode target, KfEffectSoundRequest sound)
{
    const auto kind = variant == KfEffectVariant::Alternate ? KF_EFFECT_KIND_HOMING_PROJECTILE_ALTERNATE : KF_EFFECT_KIND_HOMING_PROJECTILE;
    auto *record = effect_begin(id, type, kind, position, direction);
    if (!record)
        return nullptr;
    record->kind = KF_EFFECT_KIND_HOMING_PROJECTILE;
    record->base_render_id = variant == KfEffectVariant::Alternate
        ? KF_EFFECT_MODEL_HOMING_PROJECTILE_ALTERNATE : KF_EFFECT_MODEL_HOMING_PROJECTILE;
    record->render_id = record->base_render_id;
    record->animation_clip = KF_ANIMATION_CLIP_FIRST;
    record->rotation = rotation;
    record->rotation.vx = -record->rotation.vx;
    record->target_mode = target;
    record->direction = record->rotation;
    record->scale_y = KF_FIXED12_ONE / 2;
    record->scale_x = KF_FIXED12_ONE / 2;
    if (sound != KF_EFFECT_SOUND_SILENT) {
        audio_play_spatial_default_range(
            &effect_state.magic.entries[kf_enum_encode<u8>(KF_EFFECT_KIND_HOMING_PROJECTILE)].sounds[0],
            &record->position, KF_AUDIO_MAX_VOLUME);
    }
    return record;
}

KfEffectRecord *effect_spawn_warp_shimmer(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction)
{
    auto *record = effect_begin(id, type, KF_EFFECT_KIND_WARP_SHIMMER, position, direction);
    if (!record)
        return nullptr;
    record->animation_clip = KF_ANIMATION_CLIP_FIRST;
    record->base_render_id = KF_EFFECT_MODEL_WARP_SHIMMER;
    record->render_id = KF_EFFECT_MODEL_WARP_SHIMMER;
    record->scale_y = 0;
    record->scale_z = EFFECT_WARP_HORIZONTAL_SCALE;
    record->scale_x = EFFECT_WARP_HORIZONTAL_SCALE;
    record->rotation = {0, player_state.camera_rotation.vy, 0};
    return record;
}

KfEffectRecord *effect_pool_spawn_floor_deformation(
    s16 first_segment, s16 segment_count, s16 progress_per_update, s16 cell_stagger,
    u16 sweep_updates, s32 hold_countdown)
{
    KfEffectRecord *record = effect_pool_find_free();
    if (record != NULL) {
        record->floor_deformation = {
            .first_segment = first_segment,
            .segment_count = segment_count,
            .progress_per_update = progress_per_update,
            .cell_stagger = cell_stagger,
            .progress = 0,
            .updates_remaining = sweep_updates,
            .sweep_updates = sweep_updates,
            .hold_countdown = hold_countdown,
        };
        record->base_render_id = KF_EFFECT_MODEL_NONE;
        record->render_id = KF_EFFECT_MODEL_NONE;
        record->kind = KF_EFFECT_KIND_FLOOR_DEFORMATION;
        record->type = KF_EFFECT_FLOOR_DEFORM_TYPE;
        record->phase = KF_EFFECT_FLOOR_DEFORM_ADVANCE;
    }
    return record;
}

void effect_pool_set_current(KfEffectRecord *effect)
{
    effect_state.current_record = effect;
    effect_state.current_magic = kf_enum_encode<u8>(effect->kind) < KF_MAGIC_RECORD_COUNT
        ? &effect_state.magic.entries[kf_enum_encode<u8>(effect->kind)] : nullptr;
}

void effect_pool_reset_module_state(void)
{
    kf::restore_initial_value<effect_state>();
}
