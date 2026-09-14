#ifndef KF_ANIMATION_H
#define KF_ANIMATION_H

#include <kf/lib/types.h>
#include <kf/lib/enum.h>

/* Clip indices are relative to the selected asset. Definitions, actors,
 * map events, effects and the vertex cache pass the same index through.
 * NONE disables actor clips, invalidates cache keys, and selects effect
 * billboards. Resource-specific clip meanings belong to the asset. */
KF_ENUM_BEGIN(KfAnimationClip, u8)
    KF_ANIMATION_CLIP_FIRST = 0,
    KF_ANIMATION_CLIP_SECOND = 1,
    KF_ANIMATION_CLIP_THIRD = 2,
    KF_ANIMATION_CLIP_FOURTH = 3,
    KF_ANIMATION_CLIP_NONE = 0xff
KF_ENUM_END(KfAnimationClip)

/* Retail tests zero versus nonzero. REVERSE is the canonical source encoding;
 * shipped keyframes all use FORWARD, so its nonzero encoding is not observed. */
KF_ENUM_BEGIN(KfAnimationBlendDirection, u16)
    KF_ANIMATION_BLEND_FORWARD = 0,
    KF_ANIMATION_BLEND_REVERSE = 1
KF_ENUM_END(KfAnimationBlendDirection)

#endif
