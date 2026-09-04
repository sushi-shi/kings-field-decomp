#ifndef KF_PSYQ_AUDIO_H
#define KF_PSYQ_AUDIO_H
/* Sound / CD-ROM SDK headers, guarded (they ship without guards). Kept out of
   kf/psyq.h so their symbols are not pulled into every translation unit. */
#include <kf/psyq_cd.h>
#include <LIBSND.H>

/* Internal LIBSND entry points linked from PLAY.OBJ and STOP.OBJ. */
extern void Snd_play(short sequence, short track);
extern void Snd_stop(short sequence, short track);

#endif
