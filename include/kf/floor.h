#ifndef KF_FLOOR_H
#define KF_FLOOR_H

#include <kf/game_types.h>
#include <kf/enum.h>

KF_ENUM_BEGIN(KfFloorId, s32)
    KF_FLOOR_1 = 1,
    KF_FLOOR_2 = 2,
    KF_FLOOR_3 = 3,
    KF_FLOOR_4 = 4,
    KF_FLOOR_5 = 5,
    KF_FLOOR_FORCE_RELOAD = 255
KF_ENUM_END(KfFloorId)

KF_ENUM_BEGIN(KfMapVariant, u8)
    KF_MAP_VARIANT_DEFAULT = 0,
    KF_FLOOR5_ENTRY_VARIANT = 1,
    KF_MAP_VARIANT_2 = 2,
    KF_FLOOR5_ALTERNATE_MUSIC_VARIANT = 3
KF_ENUM_END(KfMapVariant)

#endif
