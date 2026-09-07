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

#endif
