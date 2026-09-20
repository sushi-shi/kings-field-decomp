void opening_helpers_reset_module_state(void);
void camera_path_reset_module_state(void);
void opening_scenes_reset_module_state(void);
void opening_fade_reset_module_state(void);
void opening_controller_reset_module_state(void);
void memory_reset_module_state(void);
void resources_reset_module_state(void);
void render_init_reset_module_state(void);
void render_tmd_reset_module_state(void);
void render_map_reset_module_state(void);
void render_sprite_reset_module_state(void);
void render_map_cells_reset_module_state(void);
void entity_render_reset_module_state(void);
void opening_entity_pool_reset_module_state(void);
void audio_reset_module_state(void);
void audio_play_voice_reset_module_state(void);
void format_reset_module_state(void);

static void restore_module_initial_state()
{
    opening_helpers_reset_module_state();
    camera_path_reset_module_state();
    opening_scenes_reset_module_state();
    opening_fade_reset_module_state();
    opening_controller_reset_module_state();
    memory_reset_module_state();
    resources_reset_module_state();
    render_init_reset_module_state();
    render_tmd_reset_module_state();
    render_map_reset_module_state();
    render_sprite_reset_module_state();
    render_map_cells_reset_module_state();
    entity_render_reset_module_state();
    opening_entity_pool_reset_module_state();
    audio_reset_module_state();
    audio_play_voice_reset_module_state();
    format_reset_module_state();
}
