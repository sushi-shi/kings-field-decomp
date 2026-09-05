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
extern s16 DAT_80055dac;
extern s16 DAT_80055dae;
extern s16 DAT_80055db4;
extern s16 DAT_80055db6;
extern u16 DAT_80055e9c; /* fixed fallback cell window */
extern u8 DAT_800561c8[0x70];
extern char DAT_80056238[16];
extern u8 DAT_80056247[];
extern MATRIX DAT_80056248;
extern u16 DAT_80057b72;
extern void *DAT_80057d08;
extern u32 DAT_80057d24;
extern u8 DAT_80058010[8];
extern void *DAT_80058018; /* saved handler */
extern int DAT_8005b270[]; /* jmp_buf */
extern void *DAT_8005b274; /* live abort handler slot (jmp_buf + 4) */
extern int DAT_80063278; /* installed abort handler */
extern u8 DAT_800652a8[240]; /* inventory and progress flag block */
extern u16 DAT_80095060;
extern u16 DAT_80095062;
extern u8 DAT_80095064;
extern u16 DAT_80095066;
extern u16 DAT_80095068;
extern u16 DAT_8009506a;
extern u16 DAT_8009506c;
extern u16 DAT_8009508c;
extern u16 DAT_8009508e;
extern u32 DAT_80095698;
extern u32 DAT_8009569c;
extern u32 DAT_8009a740;
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
