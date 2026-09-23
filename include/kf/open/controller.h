#ifndef KF_OPEN_CONTROLLER_H
#define KF_OPEN_CONTROLLER_H

#include <kf/lib/types.h>
#include <kf/lib/overlay.h>

enum { KF_OPENING_INITIAL_TIM_PATH_BYTES = 6 };

extern char opening_initial_tim_path[KF_OPENING_INITIAL_TIM_PATH_BYTES];

extern void opening_run(KfOverlayMode overlay_mode);

#endif
