#ifndef KF_OVERLAY_H
#define KF_OVERLAY_H

#include <kf/lib/enum.h>
#include <kf/lib/types.h>

enum class KfOverlayMode : s32 {
    KF_OVERLAY_MODE_NONE = 0,
    KF_OVERLAY_MODE_INTRO = 1,
    KF_OVERLAY_MODE_ENDING = 0xfe
}; using enum KfOverlayMode;

typedef KfEnumStorage<KfOverlayMode, u32> KfOverlayResultWord;

#endif
