#ifndef KF_PSYQ_AUDIO_H
#define KF_PSYQ_AUDIO_H
/* Sound / CD-ROM SDK headers, guarded (they ship without guards). Kept out of
   kf/psyq.h so their symbols are not pulled into every translation unit. */
#include <kf/psyq.h>
#include <LIBSND.H>
#include <LIBCD.H>

/* Internal LIBSND entry point linked from PLAY.OBJ. */
extern void Snd_play(short sequence, short track);

#endif
