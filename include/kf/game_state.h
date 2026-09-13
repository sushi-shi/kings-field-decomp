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
#include <kf/overlay.h>
#include <psyq/sdk.h>

extern u32 DAT_80057d24;
extern u32 DAT_800a0768;
extern KfOverlayResultWord game_exit_code;

enum { KF_GAMEPLAY_SOUND_COUNT = 13 };
extern SoundRef gameplay_sound_refs[KF_GAMEPLAY_SOUND_COUNT];
extern char talk_image_path_template[];

#endif
