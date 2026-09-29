extern "C" {
#ifndef KF_PSYQ_AUDIO_H
#define KF_PSYQ_AUDIO_H

#include <psyq/cd.h>
#include <LIBSND.H>

extern void Snd_play(short sequence, short track);
extern void Snd_stop(short sequence, short track);

#endif

}
