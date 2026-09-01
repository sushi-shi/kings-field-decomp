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

typedef char KfVecXZs_size_is_4[(sizeof(struct KfVecXZs) == 4) ? 1 : -1];
typedef char KfVec3s_size_is_6[(sizeof(struct KfVec3s) == 6) ? 1 : -1];
typedef char KfVec3i_size_is_12[(sizeof(struct KfVec3i) == 12) ? 1 : -1];
typedef char KfPitchYaw_size_is_4[(sizeof(struct KfPitchYaw) == 4) ? 1 : -1];
typedef char KfEulerAngles_size_is_6[
    (sizeof(struct KfEulerAngles) == 6) ? 1 : -1];

#endif
