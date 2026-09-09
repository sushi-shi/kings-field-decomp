#ifndef KF_AUDIO_H
#define KF_AUDIO_H

/*
 * GAME/OPEN audio glue over the Psy-Q sequence and SPU services. Some policy
 * entry points are overlay-specific, while VAB load/close is shared.
 *
 * These layouts belong to that boundary rather than the semantic catch-all:
 * SoundRef is the packed game sound selector, and KfAudioState is GAME's
 * proved sequence/listener/voice aggregate.
 */

#include <kf/game_types.h>
#include <kf/enum.h>
#include <kf/psyq.h>

enum {
    KF_AUDIO_VOICE_SLOTS = 10,
    KF_AUDIO_MAX_VOLUME = 0x7f,
    KF_AUDIO_DEFAULT_MAX_DISTANCE = 16000,
    KF_AUDIO_DEFAULT_ATTENUATION_DISTANCE = 28000,
    KF_AUDIO_SEQUENCE_CAPACITY = 2,
    KF_AUDIO_TRACKS_PER_SEQUENCE = 1,
    KF_AUDIO_VOICE_INACTIVE = -1,
    KF_AUDIO_VAB_AUTO = -1,
    KF_AUDIO_VAB_UNAVAILABLE = -1,
    KF_SOUND_TONE_INDEX_MASK = 0xf,
    KF_SOUND_PACKED_NOTE_SHIFT = 8
};

KF_ENUM_BEGIN(KfAudioSequenceState, s32)
    KF_AUDIO_SEQUENCE_INACTIVE = 0,
    KF_AUDIO_SEQUENCE_ACTIVE = 1
KF_ENUM_END(KfAudioSequenceState)

KF_ENUM_BEGIN(KfAudioStopMode, s32)
    KF_AUDIO_STOP_IMMEDIATE = 0,
    KF_AUDIO_STOP_FADE = 1
KF_ENUM_END(KfAudioStopMode)

KF_ENUM_BEGIN(KfAudioPlaybackResult, u32)
    KF_AUDIO_NOT_PLAYED = 0,
    KF_AUDIO_PLAYED = 1
KF_ENUM_END(KfAudioPlaybackResult)

typedef struct SoundRef {
    u8 program;
    u8 tone;
    u8 note;
} SoundRef;

typedef struct KfAudioVoiceSlots {
    s16 voice_ids[KF_AUDIO_VOICE_SLOTS];
    s16 vab_ids[KF_AUDIO_VOICE_SLOTS];
    s16 programs[KF_AUDIO_VOICE_SLOTS];
    s16 tones[KF_AUDIO_VOICE_SLOTS];
    s16 notes[KF_AUDIO_VOICE_SLOTS];
} KfAudioVoiceSlots;

typedef struct KfAudioState {
    u8 *vab_header;
    s16 active_vab_id;
    u8 unknown_06[2];
    u8 *sequence_buffer;
    s16 sequence_id;
    u8 unknown_0e[2];
    KfAudioSequenceState sequence_active;
    VECTOR listener_position;
    SVECTOR listener_rotation;
    KfAudioVoiceSlots voice_slots;
} KfAudioState;

extern u8 audio_sequence_table[];
extern KfAudioState audio_state;
extern s32 audio_voice_slot_index;

extern void audio_initialize(void);
extern void audio_shutdown(void);
extern void audio_load_vab(u8 *vab_header, u8 *vab_body);
extern void audio_close_vab(void);
extern void audio_play_current_map_sequence(void);
extern void audio_play_map_sequence(u8 sequence_id);
extern void audio_play_sequence_file(const char *path);
extern void audio_play_voice(
    s16 vab_id, s16 program, s16 tone, s16 note, s16 left_volume, s16 right_volume);
extern KfAudioPlaybackResult audio_play_spatial(
    const SoundRef *sound, const VECTOR *position, s16 volume, s32 max_distance,
    s32 attenuation_distance);
extern KfAudioPlaybackResult audio_play_spatial_default_range(
    const SoundRef *sound, const VECTOR *position, s16 volume);
extern KfAudioPlaybackResult audio_play_spatial_range(
    const SoundRef *sound, const VECTOR *position, s16 volume,
    s32 max_distance, s32 attenuation_distance);
extern void sound_ref_play(const SoundRef *sound, s16 volume);
extern void sound_ref_key_off_bank0(const SoundRef *sound);
extern void audio_set_listener_transform(
    const VECTOR *position_or_null, const SVECTOR *rotation_or_null);
extern void audio_stop_sequence_fade(void);
extern void audio_stop_sequence_master_fade(s32 fade_step);
extern void audio_stop_sequence(KfAudioStopMode mode);

#endif
