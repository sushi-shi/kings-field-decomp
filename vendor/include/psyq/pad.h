#ifndef KF_PSYQ_PAD_H
#define KF_PSYQ_PAD_H

#include <kf/lib/types.h>
#include <LIBETC.H>

extern u32 PadInit(s32 identifier);

extern u32 PadRead();
extern void PadStop(void);

#endif
