#ifndef KF_SEMANTIC_TYPES_H
#define KF_SEMANTIC_TYPES_H

#include <kf/game_types.h>

/*
 * Layout identities supported by the semantic inventory. Their original
 * source spellings and translation-unit ownership remain unknown.
 */
struct KfVecXZs {
    s16 x;
    s16 z;
};

struct KfVec3s {
    s16 x;
    s16 y;
    s16 z;
};

struct KfVec3i {
    s32 x;
    s32 y;
    s32 z;
};

struct KfVec4s {
    s16 x;
    s16 y;
    s16 z;
    s16 pad;
};

struct KfVec4i {
    s32 x;
    s32 y;
    s32 z;
    s32 pad;
};

struct KfPitchYaw {
    s16 pitch;
    s16 yaw;
};

struct KfEulerAngles {
    s16 x;
    s16 y;
    s16 z;
};

/*
 * Optional output from the world collision query.  The query copies a
 * transform from the selected player, actor, map object, or map event and
 * records the selected object's collision radius.  The final six bytes are
 * retained because the next independently referenced state begins at +0x20;
 * their meaning is not yet known.
 */
typedef struct KfCollisionTarget {
    struct KfVec4i position;
    struct KfVec4s rotation;
    u16 radius;
    u8 unknown_1a[0x06];
} KfCollisionTarget;

typedef struct SoundRef {
    u8 program;
    u8 tone;
    u8 note;
} SoundRef;

typedef struct KfAudioVoiceSlots {
    s16 voice_ids[10];
    s16 vab_ids[10];
    s16 programs[10];
    s16 tones[10];
    s16 notes[10];
} KfAudioVoiceSlots;

/*
 * GAME.EXE keeps twelve 0x98-byte actor definitions immediately before a
 * pool of 128 0x48-byte live actors.  Only fields exercised by the reviewed
 * core routines are named; the remaining bytes deliberately stay opaque.
 */
typedef struct KfActorDefinition {
    u8 unknown_00[0x11];
    s8 hit_action;
    s8 death_action;
    u8 unknown_13[0x67];
    u16 collision_radius;
    u16 collision_height;
    u16 awareness_distance;
    u16 initial_health;
    u8 unknown_82[0x02];
    u16 experience_reward;
    u16 attack_components[3];
    u16 defenses[5];
    u8 unknown_96[0x02];
} KfActorDefinition;

typedef struct KfActorActionProfile {
    s16 far_distance;
    s16 far_weight;
    s16 near_distance;
    s16 middle_weight;
    s16 near_weight;
} KfActorActionProfile;

typedef struct KfActorPlacement {
    u8 bytes[0x10];
} KfActorPlacement;

typedef struct KfActor {
    u8 slot_state;
    u8 definition_id;
    u8 variant;
    u8 heading_quadrant;
    u8 tile_x;
    u8 tile_z;
    u8 lifecycle;
    u8 unknown_07;
    u8 action;
    u8 unknown_09;
    u8 animation_id;
    u8 vertical_state;
    u8 unknown_0c[2];
    s16 local_x;
    s16 local_z;
    u16 animation_phase;
    u16 health;
    s16 cell_x;
    s16 cell_z;
    s16 unknown_1a;
    struct KfVec3i position;
    u32 unknown_28;
    struct KfEulerAngles rotation;
    u16 unknown_32;
    u32 unknown_34;
    s8 action_timer;
    u8 collision_state;
    s16 movement_yaw;
    s16 animation_step;
    s16 vertical_velocity;
    s16 movement_x;
    s16 movement_z;
    s16 movement_y;
    u8 unknown_46[2];
} KfActor;

typedef struct KfMapCell {
    u8 z;
    u8 x;
} KfMapCell;

typedef struct KfMapCopyRegion {
    u8 source_x;
    u8 source_z;
    u8 destination_x;
    u8 destination_z;
    u8 width;
    u8 height;
} KfMapCopyRegion;

typedef struct KfMapObjectPlacement {
    u8 bytes[0x14];
} KfMapObjectPlacement;

typedef struct KfMapObjectDefinition {
    u8 behavior_type;
    u8 unknown_01;
    u16 collision_radius;
    u16 interaction_radius;
    u8 unknown_06[2];
} KfMapObjectDefinition;

typedef struct KfMapObject {
    u8 object_id;
    u8 unknown_01;
    u16 cell_x;
    u16 cell_z;
    u8 unknown_06[2];
    s32 position_x;
    s32 position_y;
    s32 position_z;
    u8 unknown_14[4];
    struct KfEulerAngles rotation;
    u16 unknown_1e;
    u8 link_id;
    u8 action_parameter;
    u16 spawn_sequence;
    s16 vertical_velocity;
    u8 unknown_26[2];
    u8 action;
    u8 unknown_29;
    u16 action_timer;
} KfMapObject;

/*
 * Cutscene camera paths use 0x1c-byte serialized points and a 0x64-byte
 * runtime interpolator.  The fourth vector lane and two trailing halfwords
 * are retained because their meanings are not yet evidenced.
 */
typedef struct KfCameraPathPoint {
    struct KfVec4i position;
    struct KfVec4s rotation;
    s16 speed;
    s16 unknown_1a;
} KfCameraPathPoint;

typedef struct KfCameraPathState {
    const KfCameraPathPoint *points;
    struct KfVec4i position;
    struct KfVec4s rotation;
    struct KfVec4i position_fixed;
    struct KfVec4i rotation_fixed;
    struct KfVec4i position_delta;
    struct KfVec4i rotation_delta;
    s16 point_index;
    s16 unknown_5e;
    s32 frames_remaining;
} KfCameraPathState;

/*
 * Map-event definitions are expanded from a 0x18-byte resource record into
 * one of eight 0x44-byte runtime slots.  Only fields supported by the loader
 * and reviewed update routines are named.
 */
typedef struct KfMapEventDefinition {
    u8 state;
    u8 kind;
    u8 variant;
    u8 cell_z;
    u8 cell_x;
    u8 unknown_05[5];
    u8 image_limit;
    u8 unknown_0b;
    u8 unknown_0c;
    u8 unknown_0d;
    s16 position_z_offset;
    s16 position_x_offset;
    u16 initial_rotation;
    u16 radius;
    u16 unknown_16;
} KfMapEventDefinition;

typedef struct KfMapEvent {
    u8 state;
    u8 kind;
    u8 variant;
    u8 unknown_03[5];
    u8 image_limit;
    u8 image_index;
    u8 image_dirty;
    u8 image_delay;
    u8 unknown_0c;
    u8 unknown_0d;
    u8 unknown_0e;
    u8 unknown_0f;
    u8 unknown_10;
    u8 unknown_11;
    u16 rotation_phase;
    s32 position_x;
    s32 position_z;
    u16 cell_x;
    u16 cell_z;
    u16 radius;
    u16 unknown_22;
    s32 reference_x;
    s32 position_y;
    s32 reference_z;
    u8 unknown_30[4];
    u16 unknown_34;
    u16 rotation;
    u16 unknown_38;
    u8 unknown_3a[6];
    u16 rotation_target;
    u16 unknown_42;
} KfMapEvent;

typedef struct KfPlayerProgressState {
    u8 level;
    u8 unknown_01;
    u8 current_floor;
    u8 highest_floor;
} KfPlayerProgressState;

/*
 * Record zero seeds a new player's values.  Later records provide the next
 * absolute HP/MP values, stat steps, and cumulative experience threshold.
 * After level 40 the game extrapolates from the final two records.
 */
typedef struct KfPlayerLevelGrowth {
    u16 maximum_hp;
    u16 maximum_mp;
    u16 physical_power_step;
    u16 magic_step;
    u32 experience_threshold;
} KfPlayerLevelGrowth;

/*
 * The HUD, save summary, adjustment helpers, and full-restoration effect all
 * agree on these four adjacent unsigned halfwords.  Linked bytes do not
 * establish whether the original object had external or internal linkage.
 */
typedef struct KfPlayerVitals {
    u16 maximum_hp;
    u16 current_hp;
    u16 maximum_mp;
    u16 current_mp;
} KfPlayerVitals;

typedef struct KfPlayerAttackChargeState {
    u16 current;
    u16 committed;
} KfPlayerAttackChargeState;

/*
 * The player input loop ramps the two signed movement components, derives the
 * unsigned horizontal speed, and independently ramps the yaw and pitch steps.
 * The grouped clear routine proves the complete five-halfword extent.
 */
typedef struct KfPlayerMotionState {
    s16 strafe_velocity;
    s16 forward_velocity;
    u16 movement_speed;
    s16 yaw_step;
    s16 pitch_step;
} KfPlayerMotionState;

/*
 * Equipped weapons use a 0x2c-byte runtime-loaded record.  Reviewed combat
 * and attack code establishes only the fields below; the remaining resource
 * bytes deliberately retain opaque identities.
 */
typedef struct KfWeaponRecord {
    u8 unknown_00;
    u8 charge_rate;
    u16 attack_components[5];
    u8 unknown_0c[0x06];
    u16 attack_z_offset;
    u8 unknown_14[0x12];
    u16 mirrored_angle;
    u8 unknown_28[0x04];
} KfWeaponRecord;

/*
 * The memory-card file starts with the standard 0x200-byte PlayStation save
 * header (one header frame plus three icon frames).  King's Field appends a
 * 0x80-byte directory and stores each serialized game slot in 0x2580 bytes.
 * The six summary fields are known to be 32-bit values, but their individual
 * gameplay meanings are still under investigation.
 */
typedef struct KfSaveSlotSummary {
    u32 fields[6];
} KfSaveSlotSummary;

typedef struct KfSaveDirectory {
    u8 slot_ids[4];
    u8 reserved[0x1c];
    KfSaveSlotSummary summaries[4];
} KfSaveDirectory;

typedef struct KfSaveHeader {
    u8 playstation_header[0x200];
    KfSaveDirectory directory;
} KfSaveHeader;

typedef struct KfSavePayload {
    u8 bytes[0x2580];
} KfSavePayload;

typedef char KfVecXZs_size_is_4[(sizeof(struct KfVecXZs) == 4) ? 1 : -1];
typedef char KfVec3s_size_is_6[(sizeof(struct KfVec3s) == 6) ? 1 : -1];
typedef char KfVec3i_size_is_12[(sizeof(struct KfVec3i) == 12) ? 1 : -1];
typedef char KfVec4s_size_is_8[(sizeof(struct KfVec4s) == 8) ? 1 : -1];
typedef char KfVec4i_size_is_16[(sizeof(struct KfVec4i) == 16) ? 1 : -1];
typedef char KfPitchYaw_size_is_4[(sizeof(struct KfPitchYaw) == 4) ? 1 : -1];
typedef char KfEulerAngles_size_is_6[
    (sizeof(struct KfEulerAngles) == 6) ? 1 : -1];
typedef char KfCollisionTarget_size_is_32[
    (sizeof(KfCollisionTarget) == 0x20) ? 1 : -1];
typedef char SoundRef_size_is_3[(sizeof(SoundRef) == 3) ? 1 : -1];
typedef char KfAudioVoiceSlots_size_is_100[
    (sizeof(KfAudioVoiceSlots) == 0x64) ? 1 : -1];
#define KF_OFFSET_OF(type, member) ((u32)&(((type *)0)->member))
typedef char KfAudioVoiceSlots_vab_ids_offset_is_20[
    (KF_OFFSET_OF(KfAudioVoiceSlots, vab_ids) == 0x14) ? 1 : -1];
typedef char KfAudioVoiceSlots_programs_offset_is_40[
    (KF_OFFSET_OF(KfAudioVoiceSlots, programs) == 0x28) ? 1 : -1];
typedef char KfAudioVoiceSlots_tones_offset_is_60[
    (KF_OFFSET_OF(KfAudioVoiceSlots, tones) == 0x3c) ? 1 : -1];
typedef char KfAudioVoiceSlots_notes_offset_is_80[
    (KF_OFFSET_OF(KfAudioVoiceSlots, notes) == 0x50) ? 1 : -1];
typedef char KfCameraPathPoint_size_is_28[
    (sizeof(KfCameraPathPoint) == 0x1c) ? 1 : -1];
typedef char KfCameraPathPoint_speed_offset_is_24[
    (KF_OFFSET_OF(KfCameraPathPoint, speed) == 0x18) ? 1 : -1];
typedef char KfCameraPathState_size_is_100[
    (sizeof(KfCameraPathState) == 0x64) ? 1 : -1];
typedef char KfCameraPathState_rotation_offset_is_20[
    (KF_OFFSET_OF(KfCameraPathState, rotation) == 0x14) ? 1 : -1];
typedef char KfCameraPathState_position_fixed_offset_is_28[
    (KF_OFFSET_OF(KfCameraPathState, position_fixed) == 0x1c) ? 1 : -1];
typedef char KfCameraPathState_position_delta_offset_is_60[
    (KF_OFFSET_OF(KfCameraPathState, position_delta) == 0x3c) ? 1 : -1];
typedef char KfCameraPathState_point_index_offset_is_92[
    (KF_OFFSET_OF(KfCameraPathState, point_index) == 0x5c) ? 1 : -1];
typedef char KfCameraPathState_frames_remaining_offset_is_96[
    (KF_OFFSET_OF(KfCameraPathState, frames_remaining) == 0x60) ? 1 : -1];
typedef char KfMapEventDefinition_size_is_24[
    (sizeof(KfMapEventDefinition) == 0x18) ? 1 : -1];
typedef char KfMapEventDefinition_initial_rotation_offset_is_18[
    (KF_OFFSET_OF(KfMapEventDefinition, initial_rotation) == 0x12) ? 1 : -1];
typedef char KfMapEventDefinition_radius_offset_is_20[
    (KF_OFFSET_OF(KfMapEventDefinition, radius) == 0x14) ? 1 : -1];
typedef char KfMapEvent_size_is_68[
    (sizeof(KfMapEvent) == 0x44) ? 1 : -1];
typedef char KfMapEvent_rotation_phase_offset_is_18[
    (KF_OFFSET_OF(KfMapEvent, rotation_phase) == 0x12) ? 1 : -1];
typedef char KfMapEvent_position_x_offset_is_20[
    (KF_OFFSET_OF(KfMapEvent, position_x) == 0x14) ? 1 : -1];
typedef char KfMapEvent_cell_x_offset_is_28[
    (KF_OFFSET_OF(KfMapEvent, cell_x) == 0x1c) ? 1 : -1];
typedef char KfMapEvent_radius_offset_is_32[
    (KF_OFFSET_OF(KfMapEvent, radius) == 0x20) ? 1 : -1];
typedef char KfMapEvent_rotation_offset_is_54[
    (KF_OFFSET_OF(KfMapEvent, rotation) == 0x36) ? 1 : -1];
typedef char KfMapEvent_rotation_target_offset_is_64[
    (KF_OFFSET_OF(KfMapEvent, rotation_target) == 0x40) ? 1 : -1];
typedef char KfPlayerProgressState_size_is_4[
    (sizeof(KfPlayerProgressState) == 4) ? 1 : -1];
typedef char KfPlayerProgressState_current_floor_offset_is_2[
    (KF_OFFSET_OF(KfPlayerProgressState, current_floor) == 2) ? 1 : -1];
typedef char KfPlayerLevelGrowth_size_is_12[
    (sizeof(KfPlayerLevelGrowth) == 0x0c) ? 1 : -1];
typedef char KfPlayerLevelGrowth_experience_threshold_offset_is_8[
    (KF_OFFSET_OF(KfPlayerLevelGrowth, experience_threshold) == 8) ? 1 : -1];
typedef char KfPlayerVitals_size_is_8[
    (sizeof(KfPlayerVitals) == 8) ? 1 : -1];
typedef char KfPlayerVitals_current_hp_offset_is_2[
    (KF_OFFSET_OF(KfPlayerVitals, current_hp) == 2) ? 1 : -1];
typedef char KfPlayerVitals_maximum_mp_offset_is_4[
    (KF_OFFSET_OF(KfPlayerVitals, maximum_mp) == 4) ? 1 : -1];
typedef char KfPlayerVitals_current_mp_offset_is_6[
    (KF_OFFSET_OF(KfPlayerVitals, current_mp) == 6) ? 1 : -1];
typedef char KfPlayerAttackChargeState_size_is_4[
    (sizeof(KfPlayerAttackChargeState) == 4) ? 1 : -1];
typedef char KfPlayerAttackChargeState_committed_offset_is_2[
    (KF_OFFSET_OF(KfPlayerAttackChargeState, committed) == 2) ? 1 : -1];
typedef char KfMapCell_x_offset_is_1[
    (KF_OFFSET_OF(KfMapCell, x) == 1) ? 1 : -1];
typedef char KfPlayerMotionState_movement_speed_offset_is_4[
    (KF_OFFSET_OF(KfPlayerMotionState, movement_speed) == 4) ? 1 : -1];
typedef char KfPlayerMotionState_yaw_step_offset_is_6[
    (KF_OFFSET_OF(KfPlayerMotionState, yaw_step) == 6) ? 1 : -1];
typedef char KfPlayerMotionState_pitch_step_offset_is_8[
    (KF_OFFSET_OF(KfPlayerMotionState, pitch_step) == 8) ? 1 : -1];
typedef char KfWeaponRecord_charge_rate_offset_is_1[
    (KF_OFFSET_OF(KfWeaponRecord, charge_rate) == 1) ? 1 : -1];
typedef char KfWeaponRecord_attack_components_offset_is_2[
    (KF_OFFSET_OF(KfWeaponRecord, attack_components) == 2) ? 1 : -1];
typedef char KfWeaponRecord_attack_z_offset_is_18[
    (KF_OFFSET_OF(KfWeaponRecord, attack_z_offset) == 0x12) ? 1 : -1];
typedef char KfWeaponRecord_mirrored_angle_offset_is_38[
    (KF_OFFSET_OF(KfWeaponRecord, mirrored_angle) == 0x26) ? 1 : -1];
typedef char KfCollisionTarget_rotation_offset_is_16[
    (KF_OFFSET_OF(KfCollisionTarget, rotation) == 0x10) ? 1 : -1];
typedef char KfCollisionTarget_radius_offset_is_24[
    (KF_OFFSET_OF(KfCollisionTarget, radius) == 0x18) ? 1 : -1];
#undef KF_OFFSET_OF
typedef char KfSaveSlotSummary_size_is_24[
    (sizeof(KfSaveSlotSummary) == 0x18) ? 1 : -1];
typedef char KfSaveDirectory_size_is_128[
    (sizeof(KfSaveDirectory) == 0x80) ? 1 : -1];
typedef char KfSaveHeader_size_is_640[
    (sizeof(KfSaveHeader) == 0x280) ? 1 : -1];
typedef char KfSavePayload_size_is_9600[
    (sizeof(KfSavePayload) == 0x2580) ? 1 : -1];
typedef char KfActorDefinition_size_is_152[
    (sizeof(KfActorDefinition) == 0x98) ? 1 : -1];
typedef char KfActor_size_is_72[(sizeof(KfActor) == 0x48) ? 1 : -1];
typedef char KfActorActionProfile_size_is_10[
    (sizeof(KfActorActionProfile) == 0x0a) ? 1 : -1];
typedef char KfActorPlacement_size_is_16[
    (sizeof(KfActorPlacement) == 0x10) ? 1 : -1];
typedef char KfMapCell_size_is_2[(sizeof(KfMapCell) == 0x02) ? 1 : -1];
typedef char KfMapCopyRegion_size_is_6[
    (sizeof(KfMapCopyRegion) == 0x06) ? 1 : -1];
typedef char KfMapObjectPlacement_size_is_20[
    (sizeof(KfMapObjectPlacement) == 0x14) ? 1 : -1];
typedef char KfMapObjectDefinition_size_is_8[
    (sizeof(KfMapObjectDefinition) == 0x08) ? 1 : -1];
typedef char KfMapObject_size_is_44[
    (sizeof(KfMapObject) == 0x2c) ? 1 : -1];
typedef char KfPlayerMotionState_size_is_10[
    (sizeof(KfPlayerMotionState) == 0x0a) ? 1 : -1];
typedef char KfWeaponRecord_size_is_44[
    (sizeof(KfWeaponRecord) == 0x2c) ? 1 : -1];

#endif
