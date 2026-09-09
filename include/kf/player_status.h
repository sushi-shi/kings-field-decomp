#ifndef KF_PLAYER_STATUS_H
#define KF_PLAYER_STATUS_H

#include <kf/game_types.h>
#include <kf/enum.h>

/* Independent status bits shared by hit records and the saved player state. */
KF_ENUM_BEGIN(KfPlayerStatusFlags, u16)
    KF_PLAYER_STATUS_NONE = 0,
    KF_PLAYER_STATUS_CURSE = 1 << 0,
    KF_PLAYER_STATUS_DARKNESS = 1 << 1,
    KF_PLAYER_STATUS_POISON = 1 << 2,
    KF_PLAYER_STATUS_SLOWED = 1 << 3,
    KF_PLAYER_STATUS_FIRE_DEFENSE_BOOST = 1 << 4,
    KF_PLAYER_STATUS_KEEP_UPPER = 0xfff0
KF_ENUM_END(KfPlayerStatusFlags)
KF_ENUM_FLAGS(KfPlayerStatusFlags, u16)

#endif
