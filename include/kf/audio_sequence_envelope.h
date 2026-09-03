#ifndef KF_AUDIO_SEQUENCE_ENVELOPE_H
#define KF_AUDIO_SEQUENCE_ENVELOPE_H

/*
 * game.audio_sequence_envelope (0x8004ae30-0x8004b6a4): sequence envelope
 * controls - fade in, pitch slide, and per-track key-off.
 */

#include <kf/game_types.h>

extern void audio_sequence_fade_in(s16 sequence, s16 track);
extern void audio_sequence_pitch_slide(s16 sequence, s16 track);
extern void audio_sequence_track_key_off(s16 sequence, s16 track);

#endif
