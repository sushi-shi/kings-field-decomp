#ifndef KF_AUDIO_SEQUENCE_PARSE_H
#define KF_AUDIO_SEQUENCE_PARSE_H

/*
 * game.audio_sequence_parse (0x80046a94-0x800476e8): loads a sequence into a
 * track slot and parses its VAB-bound track data.
 */

#include <kf/game_types.h>

extern void audio_sequence_load_track(s16 slot, s16 vab_id, u8 *sequence);
extern s32 audio_sequence_parse_track(s16 slot, s32 index, s16 vab_id, u8 *data);

#endif
