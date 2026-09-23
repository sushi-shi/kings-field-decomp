#ifndef KF_ANIMATION_H
#define KF_ANIMATION_H

#include <kf/lib/types.h>

typedef u8 KfAnimationClip; enum {
    KF_ANIMATION_CLIP_FIRST = 0,
    KF_ANIMATION_CLIP_SECOND = 1,
    KF_ANIMATION_CLIP_THIRD = 2,
    KF_ANIMATION_CLIP_FOURTH = 3,
    KF_ANIMATION_CLIP_NONE = 0xff
};

typedef u16 KfAnimationBlendDirection; enum {
    KF_ANIMATION_BLEND_FORWARD = 0,
    KF_ANIMATION_BLEND_REVERSE = 1
};

#endif
