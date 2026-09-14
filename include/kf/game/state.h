#ifndef KF_GAME_STATE_H
#define KF_GAME_STATE_H

#include <kf/lib/audio.h>
#include <kf/lib/types.h>
#include <kf/lib/overlay.h>
#include <psyq/sdk.h>

extern u32 DAT_80057d24;
extern u32 DAT_800a0768;
extern KfOverlayResultWord game_next_overlay_mode;

enum { KF_GAMEPLAY_SOUND_COUNT = 13 };
extern SoundRef gameplay_sound_refs[KF_GAMEPLAY_SOUND_COUNT];
extern char talk_image_path_template[];

#endif
