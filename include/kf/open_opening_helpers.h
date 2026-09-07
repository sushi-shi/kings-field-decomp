#ifndef KF_OPEN_OPENING_HELPERS_H
#define KF_OPEN_OPENING_HELPERS_H

#include <kf/game_types.h>
#include <kf/enum.h>

KF_ENUM_BEGIN(KfOpeningInputAction, u32)
    KF_OPENING_INPUT_NONE = 0,
    KF_OPENING_INPUT_ADVANCE = 1,
    KF_OPENING_INPUT_SKIP = 2
KF_ENUM_END(KfOpeningInputAction)

extern KfOpeningInputAction opening_input_action;

extern void opening_poll_input(void);
extern s16 angle_shortest_delta(s32 first, s32 second);

#endif
