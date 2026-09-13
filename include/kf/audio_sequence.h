#ifndef KF_AUDIO_SEQUENCE_H
#define KF_AUDIO_SEQUENCE_H

#include <kf/audio.h>
#include <psyq/audio.h>

#define AUDIO_SEQUENCE_STOP_AND_CLOSE() do { \
    SsSeqStop(audio_state.sequence_id); \
    SsSeqClose(audio_state.sequence_id); \
    audio_state.sequence_active = KF_AUDIO_SEQUENCE_INACTIVE; \
} while (0)

#endif
