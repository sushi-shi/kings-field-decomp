#ifndef KF_OPEN_CONTROLLER_H
#define KF_OPEN_CONTROLLER_H

#include <kf/lib/types.h>
#include <kf/cutscene/playback.h>

enum { KF_OPENING_INITIAL_TIM_PATH_BYTES = 7 };

extern char opening_initial_tim_path[KF_OPENING_INITIAL_TIM_PATH_BYTES];

extern void opening_run(Cutscene scene);

#endif
