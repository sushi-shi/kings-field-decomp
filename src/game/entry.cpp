#include <kf/game/resources.h>
#include <kf/game/game.h>
#include <kf/lib/overlay.h>
void display_play_transition_reset_module_state(void);
void game_reset_module_state(void);
void equipment_reset_module_state(void);
void player_death_reset_module_state(void);
void player_core_reset_module_state(void);
void player_use_item_reset_module_state(void);
void player_update_reset_module_state(void);
void collision_reset_module_state(void);
void resource_file_reset_module_state(void);
void resources_reset_module_state(void);
void render_reset_module_state(void);
void render_enqueuers_reset_module_state(void);
void render_map_cells_reset_module_state(void);
void entity_render_reset_module_state(void);
void geometry_render_reset_module_state(void);
void render_frame_reset_module_state(void);
void item_reset_module_state(void);
void menu_runtime_reset_module_state(void);
void save_system_reset_module_state(void);
void actor_reset_module_state(void);
void actor_pool_reset_module_state(void);
void map_object_pool_reset_module_state(void);
void map_object_reset_module_state(void);
void audio_reset_module_state(void);
void map_scripts_reset_module_state(void);
void map_events_reset_module_state(void);
void player_warp_reset_module_state(void);
void effect_pool_reset_module_state(void);
void effect_map_collision_reset_module_state(void);
void effect_update_reset_module_state(void);
void effect_dispatch_reset_module_state(void);
void debug_text_reset_module_state(void);

static void restore_module_initial_state()
{
    display_play_transition_reset_module_state();
    game_reset_module_state();
    equipment_reset_module_state();
    player_death_reset_module_state();
    player_core_reset_module_state();
    player_use_item_reset_module_state();
    player_update_reset_module_state();
    collision_reset_module_state();
    memory_destroy_arena(memory_arena);
    resource_file_reset_module_state();
    resources_reset_module_state();
    render_reset_module_state();
    render_enqueuers_reset_module_state();
    render_map_cells_reset_module_state();
    entity_render_reset_module_state();
    geometry_render_reset_module_state();
    render_frame_reset_module_state();
    item_reset_module_state();
    menu_runtime_reset_module_state();
    save_system_reset_module_state();
    actor_reset_module_state();
    actor_pool_reset_module_state();
    map_object_pool_reset_module_state();
    map_object_reset_module_state();
    audio_reset_module_state();
    map_scripts_reset_module_state();
    map_events_reset_module_state();
    player_warp_reset_module_state();
    effect_pool_reset_module_state();
    effect_map_collision_reset_module_state();
    effect_update_reset_module_state();
    effect_dispatch_reset_module_state();
    debug_text_reset_module_state();
    format_reset_module_state();
}

extern "C" kf::AppMode kf_run_game() {
    restore_module_initial_state();
    game_main_loop();
    pool_release_all();
    memory_destroy_arena(memory_arena);
    switch (static_cast<KfOverlayMode>(game_next_overlay_mode)) {
    case KF_OVERLAY_MODE_INTRO: return kf::AppMode::Opening;
    case KF_OVERLAY_MODE_ENDING: return kf::AppMode::Ending;
    default: kf::host_fail("Game returned without a valid application transition.");
    }
}

KfMemoryArena memory_arena;
