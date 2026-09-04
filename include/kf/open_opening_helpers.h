#ifndef KF_OPEN_OPENING_HELPERS_H
#define KF_OPEN_OPENING_HELPERS_H

#include <kf/semantic_types.h>

extern u32 opening_input_action;

extern void opening_poll_input(void);
extern s16 angle_shortest_delta(s32 first, s32 second);

#endif
