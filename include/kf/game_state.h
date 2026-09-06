#ifndef KF_GAME_STATE_H
#define KF_GAME_STATE_H

/*
 * Shared declarations whose semantic owner headers remain unresolved.
 * Recovered families live in their own headers. Load-image data is defined
 * with DATA() in its owning translation unit; unresolved BSS ownership and
 * DAT_ identities remain curated WIP.
 */

#include <kf/audio.h>
#include <kf/game_types.h>
#include <kf/psyq.h>

extern u8 DAT_80055afc[];
extern u32 DAT_80057d24;
extern u32 DAT_80095698;
extern u32 DAT_8009569c;
extern u32 DAT_800a0768;
extern u32 game_exit_code;
extern const SoundRef gameplay_sound_ref_0;
extern const SoundRef gameplay_sound_ref_1;
extern SoundRef gameplay_sound_ref_10;
extern const SoundRef gameplay_sound_ref_11;
extern const SoundRef gameplay_sound_ref_12;
extern SoundRef gameplay_sound_ref_2;
extern const SoundRef gameplay_sound_ref_3;
extern SoundRef gameplay_sound_ref_4;
extern const SoundRef gameplay_sound_ref_5;
extern SoundRef gameplay_sound_ref_6;
extern const SoundRef gameplay_sound_ref_7;
extern const SoundRef gameplay_sound_ref_8;
extern char talk_image_path_template[];

#endif
