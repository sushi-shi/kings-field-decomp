extern "C" {
#ifndef KF_PSYQ_PAD_H
#define KF_PSYQ_PAD_H

#include <kf/game_types.h>
#include <LIBETC.H>

extern u32 PadInit(s32 identifier);

extern u32 PadRead(s32 ignored_identifier = 0);
extern void PadStop(void);

#endif

}
