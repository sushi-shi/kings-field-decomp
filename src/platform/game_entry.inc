#include <kf/game/game.h>
#include <kf/lib/overlay.h>
#include "game_state.inc"

extern "C" kf::AppMode kf_run_game() {
    restore_module_initial_state();
    game_main_loop();
    pool_release_all();
    memory_destroy_arena();
    switch (static_cast<KfOverlayMode>(game_next_overlay_mode)) {
    case KF_OVERLAY_MODE_INTRO: return kf::AppMode::Opening;
    case KF_OVERLAY_MODE_ENDING: return kf::AppMode::Ending;
    default: kf::host_fail("Game returned without a valid application transition.");
    }
}
