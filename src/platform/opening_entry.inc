#include <kf/open/controller.h>
#include <kf/lib/overlay.h>
#include <kf/lib/memory.h>
#include "opening_state.inc"

extern "C" void kf_run_opening(kf::AppMode mode) {
    restore_module_initial_state();
    opening_run(mode == kf::AppMode::Ending ? KF_OVERLAY_MODE_ENDING : KF_OVERLAY_MODE_INTRO);
    memory_destroy_arena();
}
