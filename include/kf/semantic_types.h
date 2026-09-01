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

typedef struct SoundRef {
    u8 bank;
    u8 program;
    u8 note;
} SoundRef;

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
    u16 death_sound;
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

typedef struct KfMapObject {
    u8 object_id;
    u8 unknown_01;
    u16 cell_x;
    u16 cell_z;
    u8 unknown_06[2];
    s32 position_x;
    s32 position_y;
    s32 position_z;
    u8 unknown_14[6];
    u16 yaw;
    u8 unknown_1c[4];
    u32 unknown_20;
    u32 unknown_24;
    u8 state_28;
    u8 unknown_29[3];
} KfMapObject;

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
typedef char SoundRef_size_is_3[(sizeof(SoundRef) == 3) ? 1 : -1];
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
typedef char KfMapCopyRegion_size_is_6[
    (sizeof(KfMapCopyRegion) == 0x06) ? 1 : -1];
typedef char KfMapObjectPlacement_size_is_20[
    (sizeof(KfMapObjectPlacement) == 0x14) ? 1 : -1];
typedef char KfMapObject_size_is_44[
    (sizeof(KfMapObject) == 0x2c) ? 1 : -1];

#endif
