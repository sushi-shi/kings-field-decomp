#include <kf/open/resources.h>
#include <kf/open/controller.h>
#include <kf/lib/overlay.h>
#include <kf/lib/memory.h>
void opening_helpers_reset_module_state(void);
void camera_path_reset_module_state(void);
void opening_scenes_reset_module_state(void);
void opening_fade_reset_module_state(void);
void opening_controller_reset_module_state(void);
void resources_reset_module_state(void);
void render_init_reset_module_state(void);
void render_tmd_reset_module_state(void);
void render_map_reset_module_state(void);
void render_sprite_reset_module_state(void);
void render_map_cells_reset_module_state(void);
void entity_render_reset_module_state(void);
void opening_entity_pool_reset_module_state(void);
void audio_reset_module_state(void);

static void restore_module_initial_state()
{
    opening_helpers_reset_module_state();
    camera_path_reset_module_state();
    opening_scenes_reset_module_state();
    opening_fade_reset_module_state();
    opening_controller_reset_module_state();
    memory_destroy_arena(memory_arena);
    resources_reset_module_state();
    render_init_reset_module_state();
    render_tmd_reset_module_state();
    render_map_reset_module_state();
    render_sprite_reset_module_state();
    render_map_cells_reset_module_state();
    entity_render_reset_module_state();
    opening_entity_pool_reset_module_state();
    audio_reset_module_state();
    format_reset_module_state();
}

extern "C" void kf_run_opening(kf::AppMode mode) {
    restore_module_initial_state();
    opening_run(mode == kf::AppMode::Ending ? KF_OVERLAY_MODE_ENDING : KF_OVERLAY_MODE_INTRO);
    memory_destroy_arena(memory_arena);
}

KfMemoryArena memory_arena;
