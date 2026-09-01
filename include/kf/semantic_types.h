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
typedef char KfPitchYaw_size_is_4[(sizeof(struct KfPitchYaw) == 4) ? 1 : -1];
typedef char KfEulerAngles_size_is_6[
    (sizeof(struct KfEulerAngles) == 6) ? 1 : -1];
typedef char KfSaveSlotSummary_size_is_24[
    (sizeof(KfSaveSlotSummary) == 0x18) ? 1 : -1];
typedef char KfSaveDirectory_size_is_128[
    (sizeof(KfSaveDirectory) == 0x80) ? 1 : -1];
typedef char KfSaveHeader_size_is_640[
    (sizeof(KfSaveHeader) == 0x280) ? 1 : -1];
typedef char KfSavePayload_size_is_9600[
    (sizeof(KfSavePayload) == 0x2580) ? 1 : -1];

#endif
