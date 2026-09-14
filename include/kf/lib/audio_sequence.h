#ifndef KF_AUDIO_SEQUENCE_H
#define KF_AUDIO_SEQUENCE_H

#include <kf/lib/audio.h>
#include <psyq/audio.h>

/* Call after the active-state check and any fade. Reload the sequence ID
 * between SDK calls, then publish the inactive state after closing it. */
#define AUDIO_SEQUENCE_STOP_AND_CLOSE() do { \
    SsSeqStop(audio_state.sequence_id); \
    SsSeqClose(audio_state.sequence_id); \
    audio_state.sequence_active = KF_AUDIO_SEQUENCE_INACTIVE; \
} while (0)

#endif
