#ifndef KF_GAME_EFFECT_H
#define KF_GAME_EFFECT_H

#include <kf/game/audio.h>

#include <array>

enum {
    KF_EFFECT_PROJECTILE_DEFAULT_SPEED = 600,
    KF_EFFECT_LIGHTNING_SPEED = 800,
    KF_EFFECT_WIND_CUTTER_SPEED = 800,
    KF_EFFECT_ACTOR_TARGET_MAX_DISTANCE = 20000,
    KF_EFFECT_ACTOR_TARGET_WIDE_CONE = 0x555
};

#include <kf/game/magic.h>
#include <kf/lib/animation.h>
#include <kf/lib/math.h>

struct KfAnimationCacheRecord;
struct KfCollisionResult;

enum {
    KF_EFFECT_CAPACITY = 48
};

enum class KfEffectBillboardId : u8 {
    KF_EFFECT_BILLBOARD_FIRE_BALL = 0,
    KF_EFFECT_BILLBOARD_FIRE_BALL_FRAME_1 = 1,
    KF_EFFECT_BILLBOARD_FIRE_BALL_FRAME_2 = 2,
    KF_EFFECT_BILLBOARD_FIRE_BALL_FRAME_3 = 3,
    KF_EFFECT_BILLBOARD_FIRE_BALL_FRAME_4 = 4,
    KF_EFFECT_BILLBOARD_WIND_CUTTER = 5,
    KF_EFFECT_BILLBOARD_LIGHTNING_BOLT = 6,
    KF_EFFECT_BILLBOARD_LIGHTNING_BOLT_FRAME_1 = 7,
    KF_EFFECT_BILLBOARD_DARKNESS_PROJECTILE = 8,
    KF_EFFECT_BILLBOARD_SCATTER_PROJECTILE = 9,
    KF_EFFECT_BILLBOARD_CURSE_PROJECTILE = 10,
    KF_EFFECT_BILLBOARD_LIGHTNING_IMPACT = 11,
    KF_EFFECT_BILLBOARD_LIGHTNING_IMPACT_FRAME_1 = 12,
    KF_EFFECT_BILLBOARD_LIGHTNING_IMPACT_FRAME_2 = 13,
    KF_EFFECT_BILLBOARD_GROUND_TRAIL = 14,
    KF_EFFECT_BILLBOARD_GROUND_TRAIL_FRAME_1 = 15,
    KF_EFFECT_BILLBOARD_GROUND_TRAIL_FRAME_2 = 16,
    KF_EFFECT_BILLBOARD_LIGHTNING_BOLT_ALTERNATE = 17,
    KF_EFFECT_BILLBOARD_LIGHTNING_ALTERNATE_FRAME_1 = 18,
    KF_EFFECT_BILLBOARD_LIGHTNING_IMPACT_ALTERNATE = 19,
    KF_EFFECT_BILLBOARD_IMPACT_ALTERNATE_FRAME_1 = 20,
    KF_EFFECT_BILLBOARD_IMPACT_ALTERNATE_FRAME_2 = 21,
    KF_EFFECT_BILLBOARD_NONE = 0xff
}; using enum KfEffectBillboardId;
inline KfEffectBillboardId& operator++(KfEffectBillboardId& value)
    { value = static_cast<KfEffectBillboardId>(static_cast<u8>(value) + 1); return value; }
    inline KfEffectBillboardId operator++(KfEffectBillboardId& value, int)
    { KfEffectBillboardId previous = value; ++value; return previous; }
    inline KfEffectBillboardId& operator--(KfEffectBillboardId& value)
    { value = static_cast<KfEffectBillboardId>(static_cast<u8>(value) - 1); return value; }
    inline KfEffectBillboardId operator--(KfEffectBillboardId& value, int)
    { KfEffectBillboardId previous = value; --value; return previous; }

enum { KF_EFFECT_BILLBOARD_SPRITE_COUNT = 22 };

enum class KfEffectModelId : u8 {
    KF_EFFECT_MODEL_LIGHTNING_RADIAL_BLAST = 0,
    KF_EFFECT_MODEL_GROUND_BRANCH = 1,
    KF_EFFECT_MODEL_GROUND_BRANCH_VISUAL = 2,
    KF_EFFECT_MODEL_ACTOR_SPAWNER = 3,
    KF_EFFECT_MODEL_MAP_EMITTER_PROJECTILE = 4,
    KF_EFFECT_MODEL_MAP_SWITCH = 5,
    KF_EFFECT_MODEL_LIGHT_NEEDLE = 6,
    KF_EFFECT_MODEL_SWINGING_HAZARD = 7,
    KF_EFFECT_MODEL_EMERGING_PROJECTILE = 9,
    KF_EFFECT_MODEL_MOONLIGHT_PROJECTILE = 10,
    KF_EFFECT_MODEL_RADIAL_BLAST = 11,
    KF_EFFECT_MODEL_HOMING_PROJECTILE = 12,
    KF_EFFECT_MODEL_WARP_SHIMMER = 13,
    KF_EFFECT_MODEL_PHYSICAL_PROJECTILE = 14,
    KF_EFFECT_MODEL_LIGHTNING_RADIAL_BLAST_ALTERNATE = 15,
    KF_EFFECT_MODEL_HOMING_PROJECTILE_ALTERNATE = 16,
    KF_EFFECT_MODEL_RADIAL_BLAST_ALTERNATE = 17,
    KF_EFFECT_MODEL_NONE = 0xff
}; using enum KfEffectModelId;

typedef KfEffectKind KfEffectKindArgument;

enum class KfEffectType : u8 {
    KF_EFFECT_TYPE_NONE = 0,
    KF_EFFECT_COLLISION_TARGET_ACTORS = 1,
    KF_EFFECT_COLLISION_TARGET_PLAYER = 2,
    KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER = 3,
    KF_EFFECT_COLLISION_TARGETS_MASK = 3,
    KF_EFFECT_USE_PLAYER_MAGIC = 0x10,
    KF_EFFECT_CLASS_20 = 0x20,
    KF_ACTOR_DAMAGE_CREDIT_PLAYER = 0x10,
    KF_ACTOR_DAMAGE_CREDIT_MASK = 0xf0,
    KF_EFFECT_FLOOR_DEFORM_TYPE = 0xf0,
    KF_EFFECT_SLOT_FREE = 0xff
}; using enum KfEffectType;
constexpr KfEffectType operator|(KfEffectType lhs, KfEffectType rhs)
    { return static_cast<KfEffectType>(static_cast<u8>(lhs) | static_cast<u8>(rhs)); }
    constexpr KfEffectType operator&(KfEffectType lhs, KfEffectType rhs)
    { return static_cast<KfEffectType>(static_cast<u8>(lhs) & static_cast<u8>(rhs)); }
    constexpr KfEffectType operator^(KfEffectType lhs, KfEffectType rhs)
    { return static_cast<KfEffectType>(static_cast<u8>(lhs) ^ static_cast<u8>(rhs)); }
    constexpr KfEffectType operator~(KfEffectType value)
    { return static_cast<KfEffectType>(~static_cast<u8>(value)); }
    inline KfEffectType& operator|=(KfEffectType& lhs, KfEffectType rhs) { return lhs = lhs | rhs; }
    inline KfEffectType& operator&=(KfEffectType& lhs, KfEffectType rhs) { return lhs = lhs & rhs; }
    inline KfEffectType& operator^=(KfEffectType& lhs, KfEffectType rhs) { return lhs = lhs ^ rhs; }

enum class KfEffectHomingMode : u8 {
    KF_EFFECT_HOMING_WANDER = 0xff,
    KF_EFFECT_HOMING_PLAYER = 0xfe
}; using enum KfEffectHomingMode;

enum class KfEffectGroundBranchRole : u16 {
    KF_EFFECT_GROUND_BRANCH_ROOT = 0,
    KF_EFFECT_GROUND_BRANCH_QUARTER_TURN = 1,
    KF_EFFECT_GROUND_BRANCH_THREE_QUARTER_TURN = 2,
    KF_EFFECT_GROUND_BRANCH_LEAF = 0xff
}; using enum KfEffectGroundBranchRole;

enum {
    KF_EFFECT_GROUND_BRANCH_TIMER_DONE = 0xff
};

enum class KfEffectPhase : u8 {
    KF_EFFECT_PHASE_INIT = 0,
    KF_EFFECT_FLOOR_DEFORM_ADVANCE = 0,
    KF_EFFECT_FLOOR_DEFORM_HOLD = 1,
    KF_EFFECT_FLOOR_DEFORM_REVERSE = 2,
    KF_EFFECT_PROJECTILE_TRAVEL = 0,
    KF_EFFECT_PROJECTILE_IMPACT_FIRST = 1,
    KF_EFFECT_PROJECTILE_DISSIPATE_FIRST = 50,
    KF_EFFECT_PROJECTILE_DISSIPATE_END = 60,
    KF_EFFECT_PROJECTILE_EMERGE_FIRST = 100,
    KF_EFFECT_PROJECTILE_EMERGE_LAST = 119,
    KF_EFFECT_PROJECTILE_FALL = 120,
    KF_EFFECT_PROJECTILE_SHRINK = 121,
    KF_EFFECT_PROJECTILE_LAUNCH_WRAP = 0xff,
    KF_EFFECT_GROUND_TRAIL_WAIT_FOR_PARENT = 0,
    KF_EFFECT_GROUND_TRAIL_SHRINK = 1,
    KF_EFFECT_HAZARD_RUNNING = 0,
    KF_EFFECT_HAZARD_RELEASE_REQUEST = 1,
    KF_EFFECT_HAZARD_RISE_FIRST = 10,
    KF_EFFECT_SHORT_SWING_PHASE_LIMIT = 40,
    KF_EFFECT_LONG_SWING_PHASE_LIMIT = 60,
    KF_EFFECT_ORBIT_PHASE_LIMIT = 40,
    KF_EFFECT_FIRE_BALL_IMPACT_END = 5,
    KF_EFFECT_PROJECTILE_IMPACT_END = 10,
    KF_EFFECT_MOONLIGHT_TRAVEL_LAST = 10,
    KF_EFFECT_MOONLIGHT_TRAIL_EMIT_PHASE = 2,
    KF_EFFECT_MOONLIGHT_IMPACT_FIRST = 20,
    KF_EFFECT_MOONLIGHT_IMPACT_LAST = 23,
    KF_EFFECT_RADIAL_BLAST_PHASE_END = 13,
    KF_EFFECT_HOMING_INITIAL_PHASE_LAST = 4,
    KF_EFFECT_HOMING_TRACKING_PHASE = 20,
    KF_EFFECT_LIGHTNING_IMPACT_PHASE_LAST = 9,
    KF_EFFECT_LIGHTNING_IMPACT_EMIT_FIRST = 3,
    KF_EFFECT_LIGHTNING_IMPACT_EMIT_SECOND = 5,
    KF_EFFECT_LIGHTNING_IMPACT_EMIT_LAST = 7,
    KF_EFFECT_LIGHTNING_BLAST_PHASE_LAST = 7,
    KF_EFFECT_GROUND_BRANCH_GROW_END = 16,
    KF_EFFECT_GROUND_BRANCH_ROOT_HOLD_BASE = 16,
    KF_EFFECT_GROUND_BRANCH_SIDE_HOLD_BASE = 26,
    KF_EFFECT_GROUND_BRANCH_LEAF_HOLD_BASE = 36,
    KF_EFFECT_GROUND_BRANCH_SHRINK_FIRST = 48,
    KF_EFFECT_GROUND_BRANCH_PHASE_END = 64,
    KF_EFFECT_GROUND_VISUAL_SHRINK_FIRST = 4,
    KF_EFFECT_GROUND_VISUAL_PHASE_END = 8,
    KF_EFFECT_ACTOR_SPAWNER_TRAVEL_FIRST = 17,
    KF_EFFECT_ACTOR_SPAWNER_TRAVEL_LAST = 40,
    KF_EFFECT_ACTOR_SPAWNER_WAIT_FIRST = 117,
    KF_EFFECT_ACTOR_SPAWNER_CREATE_PHASE = 132,
    KF_EFFECT_ACTOR_SPAWNER_SHRINK_FIRST = 149,
    KF_EFFECT_ACTOR_SPAWNER_PHASE_END = 165
}; using enum KfEffectPhase;
inline KfEffectPhase& operator++(KfEffectPhase& value)
    { value = static_cast<KfEffectPhase>(static_cast<u8>(value) + 1); return value; }
    inline KfEffectPhase operator++(KfEffectPhase& value, int)
    { KfEffectPhase previous = value; ++value; return previous; }
    inline KfEffectPhase& operator--(KfEffectPhase& value)
    { value = static_cast<KfEffectPhase>(static_cast<u8>(value) - 1); return value; }
    inline KfEffectPhase operator--(KfEffectPhase& value, int)
    { KfEffectPhase previous = value; --value; return previous; }

enum {
    KF_EFFECT_EMERGE_Y_STEP = 175,
    KF_EFFECT_EMERGE_DEPTH =
        (kf_enum_encode<u8>(KF_EFFECT_PROJECTILE_FALL) - kf_enum_encode<u8>(KF_EFFECT_PROJECTILE_EMERGE_FIRST)) * KF_EFFECT_EMERGE_Y_STEP,

    KF_EFFECT_ORBIT_CENTER_SHIFT = 8
};

enum {
    KF_EFFECT_SWING_PROBE_SHORT = 0,
    KF_EFFECT_SWING_PROBE_LONG = 1,
    KF_EFFECT_SWING_PROBE_COUNT = 2
};

typedef struct KfFloorDeformSegment {
    u8 column;
    u8 row;
    u8 column_step;
    u8 row_step;
    u8 cell_count;
    u8 start_height;
    u8 end_height;
} KfFloorDeformSegment;

// One byte selects what an effect draws: a billboard sprite while its
// animation clip is KF_ANIMATION_CLIP_NONE, otherwise a model.
// KF_EFFECT_MODEL_NONE draws nothing.
struct KfEffectRenderId {
    u8 value;

    KfEffectRenderId() = default;
    constexpr KfEffectRenderId(KfEffectBillboardId id) : value(kf_enum_encode<u8>(id)) {}
    constexpr KfEffectRenderId(KfEffectModelId id) : value(kf_enum_encode<u8>(id)) {}
    constexpr KfEffectBillboardId billboard() const { return kf_enum_decode<KfEffectBillboardId>(value); }
    constexpr KfEffectModelId model() const { return kf_enum_decode<KfEffectModelId>(value); }
};

struct KfFloorDeformation {
    s16 first_segment;
    s16 segment_count;
    s16 progress_per_update;
    s16 cell_stagger;
    s16 progress;
    u16 updates_remaining;
    u16 sweep_updates;
    s32 hold_countdown;
};

typedef struct KfEffectRecord {
    KfEffectType type;
    KfEffectKind kind;
    KfEffectRenderId base_render_id;
    KfEffectRenderId render_id;
    KfAnimationClip animation_clip;
    KfAudioPlaybackResult sound_played;
    u8 id;
    KfEffectPhase phase;
    u16 animation_phase;
    u16 unknown_0a;
    VECTOR position;
    SVECTOR rotation;
    u16 scale_x;
    u16 scale_y;
    u16 scale_z;
    u16 unknown_2a;
    SVECTOR direction;
    struct KfAnimationCacheRecord *animation_cache;
    // Kind-specific state. Each spawner initializes the fields its kind uses.
    u16 frames_remaining; // lightning bolt, scatter, fire wall, actor spawner
    u16 pulse_base_scale; // scatter
    u16 generations_remaining; // scatter
    KfEffectGroundBranchRole branch_role; // fire wall
    u8 parent_effect_index; // ground trail
    KfEffectHomingMode target_mode; // homing projectile
    s16 pitch_velocity; // swinging hazard
    u16 orbit_angle; // orbiting projectile
    SVECTOR orbit_center; // orbiting projectile; x and z in KF_EFFECT_ORBIT_CENTER_SHIFT steps
    KfFloorDeformation floor_deformation;
} KfEffectRecord;

constexpr bool effect_has_visual(const KfEffectRecord &effect)
{
    return effect.animation_clip == KF_ANIMATION_CLIP_NONE
        ? effect.render_id.billboard() != KF_EFFECT_BILLBOARD_NONE
        : effect.render_id.model() != KF_EFFECT_MODEL_NONE;
}

typedef struct KfEffectState {
    KfMagicTable magic;
    std::array<KfEffectRecord, KF_EFFECT_CAPACITY> records;
    KfMagicRecord *current_magic;
    KfEffectRecord *current_record;
} KfEffectState;

extern std::array<SVECTOR, KF_EFFECT_SWING_PROBE_COUNT> effect_swing_probe_offsets;
extern KfEffectState effect_state;

enum class KfEffectSoundRequest : s32 {
    KF_EFFECT_SOUND_SILENT = 0,
    KF_EFFECT_SOUND_PLAY = 1
}; using enum KfEffectSoundRequest;

constexpr KfEffectSoundRequest effect_sound_request(bool requested)
{
    return requested ? KF_EFFECT_SOUND_PLAY : KF_EFFECT_SOUND_SILENT;
}

enum class KfEffectVariant { Normal, Alternate };

extern KfEffectRecord *effect_pool_find_free(void);
KfEffectRecord *effect_spawn_fire_ball(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction);
KfEffectRecord *effect_spawn_wind_cutter(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, KfEffectSoundRequest sound);
KfEffectRecord *effect_spawn_lightning_bolt(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, KfEffectVariant variant, s32 duration, KfEffectSoundRequest sound);
KfEffectRecord *effect_spawn_lightning_impact(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, KfEffectVariant variant);
KfEffectRecord *effect_spawn_lightning_radial_blast(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, KfEffectVariant variant);
KfEffectRecord *effect_spawn_fire_wall(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, KfEffectGroundBranchRole role);
KfEffectRecord *effect_spawn_ground_branch_visual(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction);
KfEffectRecord *effect_spawn_actor_spawner(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, s32 duration);
KfEffectRecord *effect_spawn_scatter_projectile(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, s32 generations, s32 duration, s32 scale);
KfEffectRecord *effect_spawn_darkness_projectile(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction);
KfEffectRecord *effect_spawn_curse_projectile(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction);
KfEffectRecord *effect_spawn_emerging_projectile(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction);
KfEffectRecord *effect_spawn_map_emitter_projectile(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, const SVECTOR &rotation);
KfEffectRecord *effect_spawn_map_switch(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, const SVECTOR &rotation);
KfEffectRecord *effect_spawn_light_needle(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, const SVECTOR &rotation, KfEffectSoundRequest sound);
KfEffectRecord *effect_spawn_physical_projectile(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, const SVECTOR &rotation, KfEffectSoundRequest sound);
KfEffectRecord *effect_spawn_swinging_hazard_short(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, const SVECTOR &rotation);
KfEffectRecord *effect_spawn_swinging_hazard_long(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, const SVECTOR &rotation);
KfEffectRecord *effect_spawn_orbiting_projectile(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction);
KfEffectRecord *effect_spawn_moonlight_projectile(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, const SVECTOR &rotation);
KfEffectRecord *effect_spawn_ground_trail(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, u8 parent_index);
KfEffectRecord *effect_spawn_radial_blast(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, KfEffectVariant variant, KfEffectSoundRequest sound);
KfEffectRecord *effect_spawn_homing_projectile(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction, KfEffectVariant variant, const SVECTOR &rotation, KfEffectHomingMode target, KfEffectSoundRequest sound);
KfEffectRecord *effect_spawn_warp_shimmer(u8 id, KfEffectType type, const VECTOR &position,
    const SVECTOR &direction);
extern KfEffectRecord *effect_pool_spawn_floor_deformation(
    s16 first_segment, s16 segment_count, s16 progress_per_update, s16 cell_stagger,
    u16 sweep_updates, s32 hold_countdown);
extern void effect_pool_set_current(KfEffectRecord *effect);
extern void effect_pool_reset(void);
extern void effect_pool_update(void);
extern void effect_update_dispatch(void);
extern int effect_magic_power(KfEffectRecord *effect);
extern void effect_update_swinging_hazard(SVECTOR *probe_offset, KfEffectPhase phase_limit);
extern void effect_update_orbiting_projectile(s32 orbit_radius, KfEffectPhase phase_limit);
extern void effect_floor_deform_line(s32 segment_index, s32 progress_start, s32 progress_step);
extern void effect_scatter_triple(SVECTOR *velocity);
extern void effect_rotate_scale_offset_y(SVECTOR *offset, VECTOR *output, s16 angle, s32 scale);
extern void effect_spawn_ground_trail(u8 id, KfEffectRecord *parent_effect, s16 angle, s32 distance);
extern void effect_spawn_ground_branch(u8 id, KfEffectRecord *parent_effect, s16 angle_offset,
    KfEffectGroundBranchRole branch_role);
extern KfCollisionResult effect_map_collision(VECTOR *position, s32 radius);

#endif // KF_GAME_EFFECT_H
