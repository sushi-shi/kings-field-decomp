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
#include <kf/psyq.h>

typedef struct SoundRef {
    u8 program;
    u8 tone;
    u8 note;
} SoundRef;

typedef struct KfAudioVoiceSlots {
    s16 voice_ids[10];
    s16 vab_ids[10];
    s16 programs[10];
    s16 tones[10];
    s16 notes[10];
} KfAudioVoiceSlots;

typedef struct KfAudioState {
    u8 *vab_header;
    s16 active_vab_id;
    u8 unknown_06[2];
    u8 *sequence_buffer;
    s16 sequence_id;
    u8 unknown_0e[2];
    s32 sequence_active;
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
extern u32 audio_play_spatial(
    const SoundRef *sound, const VECTOR *position, s16 volume, s32 max_distance,
    s32 attenuation_distance);
extern u32 audio_play_spatial_default_range(
    const SoundRef *sound, const VECTOR *position, s16 volume);
extern u32 audio_play_spatial_range(
    const SoundRef *sound, const VECTOR *position, s16 volume,
    s32 max_distance, s32 attenuation_distance);
extern void sound_ref_play(const SoundRef *sound, s16 volume);
extern void audio_set_listener_transform(
    const VECTOR *position_or_null, const SVECTOR *rotation_or_null);
extern void audio_stop_sequence_fade(void);
extern void audio_stop_sequence_master_fade(s32 fade_step);
extern void audio_stop_sequence(s32 fade);

#endif
