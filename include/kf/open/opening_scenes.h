#ifndef KF_OPEN_OPENING_SCENES_H
#define KF_OPEN_OPENING_SCENES_H

#include <kf/lib/audio.h>
#include <kf/lib/map.h>
#include <kf/lib/render_types.h>

enum { KF_OPENING_SCENE3_OVERLAY_COUNT = 2 };

enum {
    KF_OPENING_SCENE0_CAMERA_POINT_COUNT = 17,
    KF_OPENING_SCENE3_CAMERA_POINT_COUNT = 3,
    KF_OPENING_ENDING_CAMERA_POINT_COUNT = 9
};

typedef s16 KfOpeningCylinderTransitionMode; enum {
    KF_OPENING_CYLINDER_TRANSITION_GROW = 0,
    KF_OPENING_CYLINDER_TRANSITION_REMOVE = 1,
    KF_OPENING_CYLINDER_TRANSITION_SHRINK = 2,
    KF_OPENING_CYLINDER_TRANSITION_CREATE = 3
};

extern KfCameraPathPoint opening_scene0_camera_path[KF_OPENING_SCENE0_CAMERA_POINT_COUNT];
extern KfCameraPathPoint opening_scene3_camera_path[KF_OPENING_SCENE3_CAMERA_POINT_COUNT];
extern KfCameraPathPoint opening_ending_camera_path[KF_OPENING_ENDING_CAMERA_POINT_COUNT];
extern SoundRef opening_scene0_sound;
extern KfScreenRect opening_scene3_overlay_rects[KF_OPENING_SCENE3_OVERLAY_COUNT];
extern u8 opening_scene3_overlay_uv[KF_QUAD_TEX_DESCRIPTOR_BYTES];
extern CVECTOR opening_scene3_overlay_color;

extern void opening_scene0_run(void);
extern void opening_scene1_draw_fade(u8 shade);
extern void opening_scene1_run(void);
extern void opening_cylinder_transition(KfOpeningCylinderTransitionMode transition_mode, const VECTOR *position);
extern void opening_scene3_run(void);
extern void opening_ending_scene_run(void);
extern void opening_ending_scroll_run(void);

#endif
