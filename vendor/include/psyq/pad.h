#ifndef KF_PSYQ_PAD_H
#define KF_PSYQ_PAD_H

/* Version-skewed LIBETC PAD.OBJ public interface recovered from retail. */

#include <kf/lib/types.h>
#include <LIBETC.H>

extern u32 PadInit(s32 identifier);
/* Retail callers pass an ignored identifier to this K&R-style boundary. */
extern u32 PadRead();
#if defined(__cplusplus)
extern u32 PadRead(s32 ignored_identifier);
#endif
extern void PadStop(void);

#endif
