#ifndef KF_AUDIO_H
#define KF_AUDIO_H

#include <kf/lib/types.h>
#include <kf/lib/enum.h>
#include <kf/lib/sound_types.h>
#include <kf/lib/geometry_types.h>
#include <kf/audio/sound.hpp>

enum {
    KF_AUDIO_VOICE_SLOTS = 10,
    KF_AUDIO_MAX_VOLUME = 0x7f,
    KF_AUDIO_DEFAULT_MAX_DISTANCE = 16000,
    KF_AUDIO_DEFAULT_ATTENUATION_DISTANCE = 28000,
    KF_AUDIO_EXTENDED_MAX_DISTANCE = 20000,
    KF_AUDIO_EXTENDED_ATTENUATION_DISTANCE = 60000,
    KF_SOUND_TONE_INDEX_MASK = 0xf
};

enum class KfAudioSequenceState : s32 {
    KF_AUDIO_SEQUENCE_INACTIVE = 0,
    KF_AUDIO_SEQUENCE_ACTIVE = 1
}; using enum KfAudioSequenceState;

enum class KfAudioStopMode : s32 {
    KF_AUDIO_STOP_IMMEDIATE = 0,
    KF_AUDIO_STOP_FADE = 1
}; using enum KfAudioStopMode;

enum class KfAudioPlaybackResult : u32 {
    KF_AUDIO_NOT_PLAYED = 0,
    KF_AUDIO_PLAYED = 1
}; using enum KfAudioPlaybackResult;

typedef struct KfAudioVoiceSlots {
    kf::SoundVoice voice_ids[KF_AUDIO_VOICE_SLOTS];
} KfAudioVoiceSlots;

typedef struct KfAudioState {
    kf::SoundBank *bank;
    u8 *sequence_buffer;
    kf::MusicSequence *sequence;
    KfAudioSequenceState sequence_active;
    VECTOR listener_position;
    SVECTOR listener_rotation;
    KfAudioVoiceSlots voice_slots;
} KfAudioState;

extern KfAudioState audio_state;
extern s32 audio_voice_slot_index;

static inline void audio_reset_voice_slots(void)
{
    s32 index;
    kf::SoundVoice inactive_voice_id;

    inactive_voice_id = kf::no_sound_voice;
    index = KF_AUDIO_VOICE_SLOTS - 1;
    do {
        audio_state.voice_slots.voice_ids[index] = inactive_voice_id;
    } while (--index >= 0);
}

static inline void audio_key_on_next_slot(
    kf::SoundBank *bank, s16 program, s16 tone, s16 note, s16 left_volume, s16 right_volume)
{
    audio_voice_slot_index++;
    if (audio_voice_slot_index == KF_AUDIO_VOICE_SLOTS) {
        audio_voice_slot_index = 0;
    }
    kf::sound_voice_release(audio_state.voice_slots.voice_ids[audio_voice_slot_index]);
    audio_state.voice_slots.voice_ids[audio_voice_slot_index] =
        kf::sound_voice_play(bank, program, tone, note, left_volume, right_volume);
}

extern void audio_initialize(void);
extern void audio_shutdown(void);
extern void audio_load_vab(const u8 *header, std::size_t header_size, const u8 *body, std::size_t body_size);
extern void audio_load_vab_resource(const u8 *data, std::size_t size);
extern void audio_close_vab(void);
extern void audio_play_current_map_sequence(void);
extern void audio_play_map_sequence(u8 sequence_id);
extern void audio_play_sequence_file(const char *path);
extern void audio_play_voice(
    kf::SoundBank *bank, s16 program, s16 tone, s16 note, s16 left_volume, s16 right_volume);
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
extern void audio_stop_sequence(KfAudioStopMode stop_mode);

#endif
