#ifndef KF_SEMANTIC_TYPES_H
#define KF_SEMANTIC_TYPES_H

#include <kf/game_types.h>
#include <kf/psyq.h>

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
 * Each display buffer owns one primitive allocation interval. Frame begin
 * selects one of two 0x0c-byte records and resets cursor to start.
 */
typedef struct KfPrimitiveBuffer {
    u8 *start;
    u8 *end;
    u8 *cursor;
} KfPrimitiveBuffer;

/*
 * CD file record used by the game's own file table (cd_file_table): the
 * older libcd CdlFILE layout with a 12-byte name, 20 bytes per entry.
 */
typedef struct KfCdFileEntry {
    u8 minute;
    u8 second;
    u8 sector;
    u8 track;
    u32 size;
    u8 name[12];
} KfCdFileEntry;

/*
 * 12-byte header of an unlinked TMD payload.  The format stores the object
 * count as a 32-bit word; the game reads only its low halfword (lhu).
 */
typedef struct KfTmdHeader {
    u32 id;
    u32 flags;
    u16 object_count;
    u16 object_count_high;
} KfTmdHeader;

/*
 * Standard 0x1c-byte object-table record in an unlinked TMD payload.  The
 * primitive count is read as a halfword (lhu) by the index preparation loop.
 */
typedef struct KfTmdObject {
    u32 vertex_offset;
    u32 vertex_count;
    u32 normal_offset;
    u32 normal_count;
    u32 primitive_offset;
    u16 primitive_count;
    u16 primitive_count_high;
    s32 scale;
} KfTmdObject;

/*
 * TMD primitive packet bodies that follow the 4-byte packet header
 * (olen, ilen, flag, mode).  n0..n3 are normal indices, v0..v3 vertex
 * indices, and tu/tv texture coordinates.  Layouts follow the Psy-Q TMD format.
 */
typedef struct KfTmdF3 {
    u8 r;
    u8 g;
    u8 b;
    u8 mode;
    u16 n0;
    u16 v0;
    u16 v1;
    u16 v2;
} KfTmdF3;

typedef struct KfTmdG3 {
    u8 r;
    u8 g;
    u8 b;
    u8 mode;
    u16 n0;
    u16 v0;
    u16 n1;
    u16 v1;
    u16 n2;
    u16 v2;
} KfTmdG3;

typedef struct KfTmdF4 {
    u8 r;
    u8 g;
    u8 b;
    u8 mode;
    u16 n0;
    u16 v0;
    u16 v1;
    u16 v2;
    u16 v3;
    u16 pad;
} KfTmdF4;

typedef struct KfTmdG4 {
    u8 r;
    u8 g;
    u8 b;
    u8 mode;
    u16 n0;
    u16 v0;
    u16 n1;
    u16 v1;
    u16 n2;
    u16 v2;
    u16 n3;
    u16 v3;
} KfTmdG4;

typedef struct KfTmdFt3 {
    u8 tu0;
    u8 tv0;
    u16 cba;
    u8 tu1;
    u8 tv1;
    u16 tsb;
    u8 tu2;
    u8 tv2;
    u16 pad;
    u16 n0;
    u16 v0;
    u16 v1;
    u16 v2;
} KfTmdFt3;

typedef struct KfTmdGt3 {
    u8 tu0;
    u8 tv0;
    u16 cba;
    u8 tu1;
    u8 tv1;
    u16 tsb;
    u8 tu2;
    u8 tv2;
    u16 pad;
    u16 n0;
    u16 v0;
    u16 n1;
    u16 v1;
    u16 n2;
    u16 v2;
} KfTmdGt3;

typedef struct KfTmdFt4 {
    u8 tu0;
    u8 tv0;
    u16 cba;
    u8 tu1;
    u8 tv1;
    u16 tsb;
    u8 tu2;
    u8 tv2;
    u16 pad0;
    u8 tu3;
    u8 tv3;
    u16 pad1;
    u16 n0;
    u16 v0;
    u16 v1;
    u16 v2;
    u16 v3;
    u16 pad2;
} KfTmdFt4;

typedef struct KfTmdGt4 {
    u8 tu0;
    u8 tv0;
    u16 cba;
    u8 tu1;
    u8 tv1;
    u16 tsb;
    u8 tu2;
    u8 tv2;
    u16 pad0;
    u8 tu3;
    u8 tv3;
    u16 pad1;
    u16 n0;
    u16 v0;
    u16 n1;
    u16 v1;
    u16 n2;
    u16 v2;
    u16 n3;
    u16 v3;
} KfTmdGt4;

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
/*
 * Per-action tables are indexed by KF_ACTOR_ACTION_INDEX(action): the hit
 * action (5) and death action (6) occupy entries 3 and 4, and the eight
 * effect actions occupy entries 8..15 (actor_update_effect_action).
 */
typedef struct KfActorDefinition {
    u8 unknown_00[0x03];
    u8 status_effect;
    u8 status_effect_chance;
    u8 action_parameters[8];
    u8 move_speed;
    u8 action_animations[16];
    u8 turn_rate;
    SoundRef sounds[3];
    struct KfVec3s attachment_offsets[2];
    s16 unknown_34;
    s16 unknown_36;
    u8 unknown_38[2];
    u16 action_animation_steps[16];
    u16 action_animation_phases[16];
    u16 collision_radius;
    u16 collision_height;
    u16 awareness_distance;
    u16 initial_health;
    u16 unknown_82;
    u16 experience_reward;
    u16 attack_components[3];
    u16 defenses[5];
    u16 unknown_96;
} KfActorDefinition;

#define KF_ACTOR_ACTION_INDEX(action) ((action) - 2)

typedef struct KfActorActionProfile {
    s16 far_distance;
    s16 far_weight;
    s16 near_distance;
    s16 middle_weight;
    s16 near_weight;
} KfActorActionProfile;

/* 16-byte actor placement record from the map's MIXA.DAT stream. */
typedef struct KfActorPlacement {
    u8 slot_state;
    u8 definition_flags;
    u8 heading_quadrant;
    u8 tile_z;
    u8 tile_x;
    u8 unknown_05;
    u8 unknown_06;
    u8 unknown_07[3];
    s16 local_z;
    s16 local_x;
    u8 unknown_0e[2];
} KfActorPlacement;

typedef struct KfActor {
    u8 slot_state;
    u8 definition_id;
    u8 variant;
    u8 heading_quadrant;
    u8 tile_z;
    u8 tile_x;
    u8 lifecycle;
    u8 unknown_07;
    u8 action;
    u8 unknown_09;
    u8 animation_id;
    u8 vertical_state;
    u8 unknown_0c[2];
    s16 local_z;
    s16 local_x;
    u16 animation_phase;
    u16 health;
    u16 cell_x;
    u16 cell_z;
    s16 unknown_1a;
    struct KfVec4i position;
    struct KfEulerAngles rotation;
    u16 unknown_32;
    u32 unknown_34;
    u8 action_timer;
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

/* Effect pool record; only the 60-byte stride and two fields are known. */
typedef struct KfEffectRecord {
    u8 unknown_00[7];
    u8 unknown_07;
    u16 unknown_08;
    u8 unknown_0a[0x32];
} KfEffectRecord;

typedef struct KfMapObjectLink {
    u8 link_id;
    u8 action_parameter;
    u16 spawn_sequence;
    s16 vertical_velocity;
    u8 unknown_06[2];
} KfMapObjectLink;

typedef struct KfMapObjectPlacement {
    u8 object_id;
    u8 unknown_01;
    u8 tile_z;
    u8 tile_x;
    u16 yaw;
    s16 local_z;
    s16 local_x;
    s16 local_y;
    KfMapObjectLink link;
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
    KfMapObjectLink link;
    u8 action;
    u8 unknown_29;
    u16 action_timer;
} KfMapObject;

/*
 * Serialized floor-item placement record (12 bytes) from the map resource
 * stream, and the runtime floor-item entry (24 bytes) the loader expands it
 * into.  The tile bytes index map_floor_height_grid[tile_z][tile_x]; the world
 * position is tile*2000 plus the signed local offset, and the height byte times
 * -100 sinks the item onto the floor.
 */
typedef struct KfFloorItemPlacement {
    u16 item_id;
    u8 unknown_02;
    u8 unknown_03;
    u8 tile_z;
    u8 tile_x;
    s16 local_z;
    s16 local_x;
    s16 local_y;
} KfFloorItemPlacement;

typedef struct KfFloorItem {
    u16 item_id;
    u8 unknown_02;
    u8 unknown_03;
    s32 position_x;
    s32 position_y;
    s32 position_z;
    u8 unknown_10[4];
    u8 flicker;
    u8 unknown_15[3];
} KfFloorItem;

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
/*
 * Five opaque bytes that map_event_pool_load copies from a definition into its
 * runtime slot as one block (an unaligned word plus a byte), which only a
 * struct assignment produces.
 */
typedef struct KfMapEventTag {
    u8 bytes[5];
} KfMapEventTag;

typedef struct KfMapEventDefinition {
    u8 state;
    u8 kind;
    u8 variant;
    u8 cell_z;
    u8 cell_x;
    KfMapEventTag tag;
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
    KfMapEventTag tag;
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
    s16 rotation;
    u16 unknown_38;
    u8 unknown_3a[6];
    s16 rotation_target;
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
/* Armor and accessory record (item ids 13..54 index armor_records[id - 13]). */
typedef struct KfArmorRecord {
    u16 unknown_00;
    u16 defense_component0;
    u16 defense_component1;
    u16 defense_component2;
    u16 status_effect2_resistance;
    u16 defense_component3;
    u16 defense_component4;
    u16 hp_regen_interval;
    u16 hp_drain_interval;
    u8 unknown_12[0x0a];
} KfArmorRecord;

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
    u16 hp_regen_interval;
    u16 mp_regen_interval;
    u8 unknown_10[0x02];
    u16 attack_z_offset;
    u8 unknown_14[0x12];
    u16 mirrored_angle;
    u8 unknown_28[0x04];
} KfWeaponRecord;

typedef struct KfMagicRecord {
    u8 learned;
    u8 charge_rate;
    u8 unknown_02[0x06];
    u16 unknown_08;
    u16 unknown_0a;
    u8 unknown_0c[0x04];
    u16 mp_cost;
    u8 unknown_12[0x02];
} KfMagicRecord;

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

/*
 * The write/read pair copies four ranges verbatim.  The two word-aligned
 * ranges are moved with aligned word loops and the 0xf0-byte range with a
 * runtime-alignment-checked loop, which fixes the member alignments; the
 * meanings of the serialized bytes remain unresolved.
 */
typedef struct KfSavePayload {
    u32 player_state[56];
    u8 unknown_0e0[556];
    u32 world_state[2125];
    u8 unknown_2440[240];
    u8 magic_flags[24];
    u8 unknown_2548[56];
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
typedef char KfPrimitiveBuffer_size_is_12[
    (sizeof(KfPrimitiveBuffer) == 0x0c) ? 1 : -1];
typedef char KfPrimitiveBuffer_end_offset_is_4[
    (KF_OFFSET_OF(KfPrimitiveBuffer, end) == 0x04) ? 1 : -1];
typedef char KfPrimitiveBuffer_cursor_offset_is_8[
    (KF_OFFSET_OF(KfPrimitiveBuffer, cursor) == 0x08) ? 1 : -1];
typedef char KfTmdObject_size_is_28[
    (sizeof(KfTmdObject) == 0x1c) ? 1 : -1];
typedef char KfTmdObject_primitive_offset_offset_is_16[
    (KF_OFFSET_OF(KfTmdObject, primitive_offset) == 0x10) ? 1 : -1];
typedef char KfTmdObject_primitive_count_offset_is_20[
    (KF_OFFSET_OF(KfTmdObject, primitive_count) == 0x14) ? 1 : -1];
typedef char KfCdFileEntry_size_is_20[(sizeof(KfCdFileEntry) == 0x14) ? 1 : -1];
typedef char KfTmdHeader_size_is_12[(sizeof(KfTmdHeader) == 0xc) ? 1 : -1];
typedef char KfTmdF3_size_is_12[(sizeof(KfTmdF3) == 0xc) ? 1 : -1];
typedef char KfTmdG3_size_is_16[(sizeof(KfTmdG3) == 0x10) ? 1 : -1];
typedef char KfTmdF4_size_is_16[(sizeof(KfTmdF4) == 0x10) ? 1 : -1];
typedef char KfTmdG4_size_is_20[(sizeof(KfTmdG4) == 0x14) ? 1 : -1];
typedef char KfTmdFt3_size_is_20[(sizeof(KfTmdFt3) == 0x14) ? 1 : -1];
typedef char KfTmdGt3_size_is_24[(sizeof(KfTmdGt3) == 0x18) ? 1 : -1];
typedef char KfTmdFt4_size_is_28[(sizeof(KfTmdFt4) == 0x1c) ? 1 : -1];
typedef char KfTmdGt4_size_is_32[(sizeof(KfTmdGt4) == 0x20) ? 1 : -1];
typedef char KfTmdObject_scale_offset_is_24[
    (KF_OFFSET_OF(KfTmdObject, scale) == 0x18) ? 1 : -1];
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
typedef char KfArmorRecord_size_is_28[(sizeof(KfArmorRecord) == 28) ? 1 : -1];
typedef char KfMagicRecord_size_is_20[(sizeof(KfMagicRecord) == 20) ? 1 : -1];
typedef char KfEffectRecord_size_is_60[(sizeof(KfEffectRecord) == 60) ? 1 : -1];
typedef char KfMapObjectLink_size_is_8[(sizeof(KfMapObjectLink) == 8) ? 1 : -1];
typedef char KfMapObjectPlacement_size_is_20[
    (sizeof(KfMapObjectPlacement) == 0x14) ? 1 : -1];
typedef char KfMapObjectDefinition_size_is_8[
    (sizeof(KfMapObjectDefinition) == 0x08) ? 1 : -1];
typedef char KfMapObject_size_is_44[
    (sizeof(KfMapObject) == 0x2c) ? 1 : -1];
typedef char KfFloorItemPlacement_size_is_12[
    (sizeof(KfFloorItemPlacement) == 0x0c) ? 1 : -1];
typedef char KfFloorItem_size_is_24[
    (sizeof(KfFloorItem) == 0x18) ? 1 : -1];
typedef char KfPlayerMotionState_size_is_10[
    (sizeof(KfPlayerMotionState) == 0x0a) ? 1 : -1];
typedef char KfWeaponRecord_size_is_44[
    (sizeof(KfWeaponRecord) == 0x2c) ? 1 : -1];

/* === game-math-death layouts === */
/* === end game-math-death === */

/* === player layouts === */

/*
 * The saved player object. save_file_write_slot copies exactly 0xe0 bytes
 * from 0x800a0780 into the save payload with the compiler's inline struct
 * loop, and the player routines address these members from a registered
 * base; both prove one object rather than adjacent globals. Field names keep
 * their former identity spellings; unknown_ spans remain opaque.
 */
typedef struct KfPlayerState {
    s32 experience;
    s32 next_level_experience;
    KfPlayerProgressState progress_state;
    u8 map_variant;
    u8 unknown_0d;
    u8 weapon_charge_delay;
    u8 unknown_0f;
    KfPlayerVitals vitals;
    KfPlayerAttackChargeState attack_charge_state;
    u16 magic_charge;
    u16 physical_power_training;
    u16 magic_training;
    u16 base_physical_power;
    u16 base_magic;
    u16 physical_power;
    u16 magic;
    u16 status_effect_flags;
    u32 unknown_2c;
    u16 attack_component0;
    u16 attack_component1;
    u16 attack_component2;
    u16 attack_component3;
    u16 attack_component4;
    u8 unknown_3a[2];
    u16 damage_defense_component0;
    u16 damage_defense_component1;
    u16 damage_defense_component2;
    u16 status_effect2_resistance;
    u16 damage_defense_component3;
    u16 damage_defense_component4;
    s16 status_effect0_timer;
    s16 status_effect1_timer;
    s16 status_effect2_timer;
    s16 status_effect3_timer;
    s16 status_effect4_timer;
    s16 light_effect_timer;
    u8 unknown_54[4];
    u32 unknown_58;
    u8 selected_magic_id;
    u8 unknown_5d[3];
    KfMagicRecord *selected_magic_record;
    u8 equipped_weapon_id;
    u8 unknown_65[3];
    const KfWeaponRecord *equipped_weapon_record;
    u8 *weapon_asset_buffer;
    s16 weapon_attack_phase;
    u8 unknown_72[2];
    u32 unknown_74;
    u8 unknown_78;
    u8 unknown_79;
    u8 weapon_attack_fully_charged;
    u8 unknown_7b[1];
    KfArmorRecord *equipped_shield_record;
    KfArmorRecord *equipped_head_armor_record;
    KfArmorRecord *equipped_body_armor_record;
    KfArmorRecord *equipped_arm_armor_record;
    KfArmorRecord *equipped_leg_armor_record;
    u8 equipped_shield_id;
    u8 equipped_head_armor_id;
    u8 equipped_body_armor_id;
    u8 equipped_arm_armor_id;
    u8 equipped_leg_armor_id;
    u8 equipped_accessory_id;
    u8 audio_effects_enabled;
    u8 audio_music_enabled;
    u8 unknown_98[2];
    struct KfVec4s view_rotation_offset;
    u8 update_state;
    u8 unknown_a3;
    struct KfVec4i camera_position;
    s32 floor_height;
    struct KfVec4s camera_rotation;
    KfPlayerMotionState motion_state;
    KfMapCell map_cell;
    KfMapCell previous_map_cell;
    u8 unknown_ce[6];
    s16 view_bob_offset;
    u16 view_bob_phase;
    u16 death_camera_pitch_step;
    s16 death_visual_blend;
    s16 vertical_velocity;
    u8 vertical_state;
    u8 unknown_df[1];
} KfPlayerState;

typedef char KfPlayerState_size_is_0xe0[(sizeof(KfPlayerState) == 0xe0) ? 1 : -1];

/*
 * Per-floor entry cell (one-based floor number). player_warp_to_floor_entry
 * copies byte 0 into previous_map_cell.x and byte 1 into .z, so this table
 * stores x first, unlike KfMapCell.
 */
typedef struct KfFloorEntryCell {
    u8 x;
    u8 z;
} KfFloorEntryCell;

typedef char KfFloorEntryCell_size_is_2[(sizeof(KfFloorEntryCell) == 2) ? 1 : -1];

/* === end player === */
/* === actor layouts === */

/*
 * The actor system state is one object: actor routines address a definition
 * as the actor array base minus 0x720 (`addiu ...,-1824`) and the current
 * actor pointer as the array base plus 0x241c (`addiu ...,9244`), arithmetic
 * the compiler only emits inside one aggregate. The members after the arrays
 * are the retail order of the formerly separate identities.
 */
typedef struct KfActorState {
    KfActorDefinition definitions[12];
    KfActor actors[128];
    struct KfVec4i player_position;
    struct KfVec4s player_rotation;
    KfActorDefinition *current_definition;
    KfActor *current;
    u16 current_index;
    u16 current_definition_id;
    KfActor *player_target;
} KfActorState;

typedef char KfActorState_size_is_0x2b48[
    (sizeof(KfActorState) == 0x2b48) ? 1 : -1];
#define KF_ACTOR_STATE_OFFSET_OF(member) ((u32)&(((KfActorState *)0)->member))
typedef char KfActorState_current_offset_is_0x2b3c[
    (KF_ACTOR_STATE_OFFSET_OF(current) == 0x2b3c) ? 1 : -1];
typedef char KfActorState_player_target_offset_is_0x2b44[
    (KF_ACTOR_STATE_OFFSET_OF(player_target) == 0x2b44) ? 1 : -1];
#undef KF_ACTOR_STATE_OFFSET_OF

/* === end actor === */
/* === map-audio layouts === */

/*
 * One aggregate holds the audio runtime state at 0x80095868. audio_initialize
 * derives the voice-id slot address from the sequence-buffer field with a
 * plain `addiu 54`, which a compiler can only do inside one object; the former
 * separate identities are its fields.
 */
typedef struct KfAudioState {
    u8 *vab_header;
    s16 active_vab_id;
    u8 unknown_06[2];
    u8 *sequence_buffer;
    s16 sequence_id;
    u8 unknown_0e[2];
    s32 sequence_active;
    struct KfVec4i listener_position;
    struct KfVec4s listener_rotation;
    KfAudioVoiceSlots voice_slots;
} KfAudioState;

typedef char KfAudioState_size_is_144[(sizeof(KfAudioState) == 0x90) ? 1 : -1];

/* === end map-audio === */

/* One reversed ordering table of 0x4000 entries (ClearOTagR/DrawOTag). */
typedef struct KfOrderingTable {
    u32 entries[0x4000];
} KfOrderingTable;

typedef char KfOrderingTable_size_is_0x10000[(sizeof(KfOrderingTable) == 0x10000) ? 1 : -1];


/* === display_state layout === */
/*
 * Double-buffered display state. display_begin_frame addresses the primitive
 * buffers (+8) and the ordering tables (+0x24) from the buffer index through
 * one base register, so the block is one object in the original source.
 */
typedef struct KfDisplayState {
    u8 buffer_index;
    u8 unknown_01[3];
    void *asset_load_buffer;
    KfPrimitiveBuffer primitive_buffers[2];
    KfPrimitiveBuffer *primitive_buffer;
    KfOrderingTable ordering_tables[2];
    u32 *ordering_table;
} KfDisplayState;

typedef char KfDisplayState_size_is_0x20028[(sizeof(KfDisplayState) == 0x20028) ? 1 : -1];
/* === end display_state === */


/* === tmd_state layout === */
/*
 * Registered TMD slots and the selected asset. tmd_register addresses the slot
 * table 32 bytes below the current-asset pointer through one base register.
 */
typedef struct KfTmdState {
    u8 *slots[8];
    void *current_asset;
} KfTmdState;

typedef char KfTmdState_size_is_0x24[(sizeof(KfTmdState) == 0x24) ? 1 : -1];
/* === end tmd_state === */


/* === render_state layout === */
/*
 * Render state. render_set_view_transform reaches the view and pitch matrices
 * from the rotation (-180, -148) and render_initialize reaches the quadrant
 * matrices from the light matrix (+128..+224) through one base register, so
 * the block is one object in the original source. The light products that
 * follow (light_quadrant_matrices) are addressed absolutely there, so they are
 * a separate object. Matrix roles follow the GTE calls that fill them; the
 * light matrix values are three direction rows.
 */
typedef struct KfRenderState {
    struct KfMatrix view_matrix;
    struct KfMatrix pitch_matrix;
    struct KfMatrix light_matrix;
    struct KfMatrix light_matrix_copy;
    struct KfMatrix unknown_80;
    s32 fog_near_distance;
    struct KfVec4i view_position;
    struct KfVec4s view_rotation;
    struct KfVecXZs view_cell;
    struct KfMatrix quadrant_matrices[4];
} KfRenderState;

typedef char KfRenderState_size_is_0x140[(sizeof(KfRenderState) == 0x140) ? 1 : -1];
/* === end render_state === */


/* === display_state layout === */
/*
 * OPEN.EXE display state.  Same members as KfDisplayState except that the
 * ordering-table pointer lives outside the block (ordering_table).
 */
typedef struct KfDisplayStateOpen {
    u8 buffer_index;
    u8 unknown_01[3];
    void *asset_load_buffer;
    KfPrimitiveBuffer primitive_buffers[2];
    KfPrimitiveBuffer *primitive_buffer;
    KfOrderingTable ordering_tables[2];
} KfDisplayStateOpen;

typedef char KfDisplayStateOpen_size_is_0x20024[(sizeof(KfDisplayStateOpen) == 0x20024) ? 1 : -1];
/* === end display_state === */


/* === tmd_state layout === */
/* OPEN.EXE TMD state: two asset slots and the selected asset. */
typedef struct KfTmdStateOpen {
    u8 *slots[2];
    void *current_asset;
} KfTmdStateOpen;

typedef char KfTmdStateOpen_size_is_0xc[(sizeof(KfTmdStateOpen) == 0xc) ? 1 : -1];
/* === end tmd_state === */


/* === render_state layout === */
/*
 * OPEN.EXE render state.  Same roles as KfRenderState without the
 * light_matrix_copy and unknown_80 members.
 */
typedef struct KfRenderStateOpen {
    struct KfMatrix view_matrix;
    struct KfMatrix pitch_matrix;
    struct KfMatrix light_matrix;
    s32 fog_near_distance;
    struct KfVec4i view_position;
    struct KfVec4s view_rotation;
    struct KfVecXZs view_cell;
    struct KfMatrix quadrant_matrices[4];
} KfRenderStateOpen;

typedef char KfRenderStateOpen_size_is_0x100[(sizeof(KfRenderStateOpen) == 0x100) ? 1 : -1];
/* === end render_state === */


/* === map_object_state layout === */
/*
 * Map object state: the 160 definitions and the 190-object pool.
 * map_object_pool_clear_link reaches the definitions from the pool base
 * register (-1280), so the two arrays are one object in the original source.
 */
typedef struct KfMapObjectState {
    KfMapObjectDefinition definitions[160];
    KfMapObject objects[190];
} KfMapObjectState;

typedef char KfMapObjectState_size_is_0x25a8[(sizeof(KfMapObjectState) == 0x25a8) ? 1 : -1];
/* === end map_object_state === */

#endif
