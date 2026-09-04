#ifndef KF_OPEN_SCENE0_H
#define KF_OPEN_SCENE0_H

#include <kf/semantic_types.h>

extern KfCameraPathPoint opening_scene0_camera_path[17];
extern SoundRef opening_scene0_sound;

extern void opening_render_map_cells(void);
extern void opening_render_entities_and_items(void);
extern void opening_scene0_render_frame(
    const VECTOR *position, const SVECTOR *rotation);
extern void opening_scene0_run(void);

#endif
