#ifndef KF_PSYQ_PAD_H
#define KF_PSYQ_PAD_H

/* Version-skewed LIBETC PAD.OBJ public interface recovered from retail. */

#include <kf/game_types.h>

extern u32 PadInit(s32 identifier);
/* Retail callers pass an ignored identifier to this K&R-style boundary. */
extern u32 PadRead();
extern void PadStop(void);

#endif
