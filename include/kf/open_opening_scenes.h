#ifndef KF_OPEN_OPENING_SCENES_H
#define KF_OPEN_OPENING_SCENES_H

#include <kf/audio.h>
#include <kf/game_map.h>

enum { KF_OPENING_SCENE3_OVERLAY_COUNT = 2 };

KF_ENUM_BEGIN(KfOpeningTransitionMode, s16)
    KF_OPENING_TRANSITION_GROW = 0,
    KF_OPENING_TRANSITION_REMOVE = 1,
    KF_OPENING_TRANSITION_SHRINK = 2,
    KF_OPENING_TRANSITION_CREATE = 3
KF_ENUM_END(KfOpeningTransitionMode)

extern KfCameraPathPoint opening_scene0_camera_path[17];
extern KfCameraPathPoint opening_scene3_camera_path[3];
extern KfCameraPathPoint opening_ending_camera_path[9];
extern SoundRef opening_scene0_sound;
extern u16 opening_scene3_overlay_rects[KF_OPENING_SCENE3_OVERLAY_COUNT][4];
extern u8 opening_scene3_overlay_uv[8];
extern u8 opening_scene3_overlay_color[4];

extern void opening_scene0_run(void);
extern void opening_scene1_draw_fade(u8 shade);
extern void opening_scene1_run(void);
extern void opening_entity_transition(KfOpeningTransitionMode mode, const VECTOR *position);
extern void opening_scene3_run(void);
extern void opening_ending_scene_run(void);
extern void opening_ending_scroll_run(void);

#endif
