#ifndef KF_FLOOR_H
#define KF_FLOOR_H

#include <kf/lib/types.h>

typedef s32 KfFloorId; enum {
    KF_FLOOR_1 = 1,
    KF_FLOOR_2 = 2,
    KF_FLOOR_3 = 3,
    KF_FLOOR_4 = 4,
    KF_FLOOR_5 = 5,
    KF_FLOOR_FORCE_RELOAD = 255
};

typedef u8 KfMapVariant; enum {
    KF_MAP_VARIANT_DEFAULT = 0,
    KF_FLOOR5_ENTRY_VARIANT = 1,
    KF_MAP_VARIANT_2 = 2,
    KF_FLOOR5_ALTERNATE_MUSIC_VARIANT = 3
};

#endif
