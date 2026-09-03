#ifndef KF_GAME_AUDIO_H
#define KF_GAME_AUDIO_H

/*
 * Audio / sound prototypes (game glue over the sequence engine).
 *
 * Generated during extern-crutch removal: declarations that were duplicated
 * as `extern` across src/game/*.c now live here once. DAT_/func_ spellings
 * remain unresolved WIP identities. Byte-neutral: a declaration never changes
 * codegen.
 */

#include <kf/semantic_types.h>

extern void audio_close_vab(void);
extern void audio_initialize(void);
extern void audio_load_vab(u8 *vab_header, u8 *vab_body);
extern void audio_play_current_map_sequence(void);
extern void audio_play_map_sequence(u8 sequence_id);
extern u32 audio_play_spatial( const SoundRef *sound, const VECTOR *position, s16 volume, s32 max_distance, s32 attenuation_distance);
extern void audio_play_voice( s16 vab_id, s16 program, s16 tone, s16 note, s16 left_volume, s16 right_volume);
extern void audio_sequence_fade_in(s16 sequence, s16 track);
extern void audio_sequence_load_track(s16 slot, s16 vab_id, u8 *sequence);
extern s32 audio_sequence_parse_track(s16 slot, s32 index, s16 vab_id, u8 *data);
extern void audio_sequence_pitch_slide(s16 sequence, s16 track);
extern void audio_sequence_track_key_off(s16 sequence, s16 track);
extern void audio_set_listener_transform( const VECTOR *position_or_null, const SVECTOR *rotation_or_null);
extern void audio_shutdown(void);
extern void audio_stop_sequence_fade(void);
extern void audio_stop_sequence_master_fade(s32 fade_step);
extern void sound_ref_play(const SoundRef *sound, s16 volume);

#endif
