#ifndef KF_AUDIO_SEQUENCE_H
#define KF_AUDIO_SEQUENCE_H

#include <kf/lib/audio.h>

#define AUDIO_SEQUENCE_STOP_AND_CLOSE() do { \
    kf::sound_sequence_release(audio_state.sequence); \
    audio_state.sequence = nullptr; \
    audio_state.sequence_active = KF_AUDIO_SEQUENCE_INACTIVE; \
} while (0)

#endif
