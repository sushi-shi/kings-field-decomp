#pragma once
#include <kf/lib/audio.h>

extern KfAudioState audio_state;
extern s32 audio_voice_slot_index;
KfAudioPlayback audio_playback();

extern void audio_initialize(void);
extern void audio_load_vab(KfAudioBankResource resource);
extern void audio_play_sequence_file(const char *path);
extern KfAudioPlaybackResult audio_play_spatial(
    const SoundRef *sound, const VECTOR *position, s16 volume, s32 max_distance,
    s32 attenuation_distance);
extern void audio_stop_sequence(KfAudioStopMode stop_mode);
