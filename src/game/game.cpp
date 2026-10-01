#include <kf/game/system.h>
#include <kf/game/world.h>
#include <kf/game/audio.h>
#include <kf/game/resources.h>
#include <kf/game/graphics.h>

#include <kf/lib/overlay.h>
#include <kf/game/player.h>
#include <kf/game/save.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <kf/game/game.h>

enum {
    FRAME_PACER_INTERVAL_TICKS = 3,
    ENDING_MASTER_FADE_STEP_Q8 = 0x80
};

static std::uint64_t frame_pacer_last_tick;

KfOverlayResultWord game_next_overlay_mode;

kf::FrameTask<void> game_wait_frame()
{
    co_await kf::FrameDelay {1};
}

kf::FrameTask<void> game_wait_buttons_released(u32 mask)
{
    while (kf::host_read_buttons() & mask)
        co_await kf::FrameDelay {1};
}

kf::FrameTask<void> game_wait_button_press()
{
    while (!kf::host_read_buttons())
        co_await kf::FrameDelay {1};
}

kf::FrameTask<void> game_present_frame(const KfDisplayState &display)
{
    kf::host_present_frame(display.frame_style);
    co_await kf::FrameDelay {1};
}

kf::FrameTask<void> game_main_loop(WorldState &world, PlayerContext &player)
{
    memset((void *)&game_graphics_runtime, 0, sizeof game_graphics_runtime);
    memset((void *)&world.actors, 0, sizeof world.actors);
    memset((void *)&world.objects, 0, sizeof world.objects);
    memset((void *)&world.effects, 0, sizeof(KfEffectState));
    memset((void *)world.map.events, 0, sizeof world.map.events);
    memset((void *)&player.state, 0, sizeof(KfPlayerState));
    memory_set_allocation_mode(memory_arena, KF_MEMORY_CREATE_ARENA);
    audio_initialize();
    display_initialize();
    item_load_database();
    actor_pool_clear(world);
    map_object_pool_clear(world);
    effect_pool_reset(world);
    map_event_timers_reset(world);
    common_resources_load(world, player);
    game_initialize_session(world, player);
    memory_set_allocation_mode(memory_arena, KF_MEMORY_REBASE_ARENA);
    (co_await map_load_floor_wrapper(world, player));
    frame_pacer_last_tick = kf::host_clock_tick();
    (co_await player_warp_shimmer_at_player(world, player, KF_WARP_SHIMMER_SHRINK_REMOVE));
    game_next_overlay_mode = KF_OVERLAY_MODE_NONE;
    if (!kf::net::application_config.signaling_url.empty()) {
        co_await coop_game_loop(world, player);
        if (game_next_overlay_mode == KF_OVERLAY_MODE_NONE)
            game_next_overlay_mode = KF_OVERLAY_MODE_INTRO;
        else if (game_next_overlay_mode == KF_OVERLAY_MODE_ENDING) {
            auto &local = world.party.members[world.party.local_slot].player;
            co_await player_warp_shimmer_at_player(world, local, KF_WARP_SHIMMER_GROW_KEEP);
            co_await display_play_transition();
            co_await audio_stop_sequence_master_fade(ENDING_MASTER_FADE_STEP_Q8);
        }
        game_shutdown();
        co_return;
    }
    for (;;) {
        (co_await player_update(world, player, {kf::host_read_buttons(), kf::host_take_look()}));
        if (game_next_overlay_mode != KF_OVERLAY_MODE_NONE) {
            break;
        }
        player_update_transform_snapshot(player, &player.presentation.position_snapshot, &player.presentation.rotation_snapshot);
        audio_set_listener_transform(audio_state, &player.presentation.position_snapshot, &player.presentation.rotation_snapshot);
        actor_set_player_transform(world, &player.presentation.position_snapshot, &player.presentation.rotation_snapshot);
        (co_await actor_pool_update(world, player));
        map_object_pool_update(world, player);
        effect_pool_update(world, player);
        map_event_pool_update(world, player);
        (co_await map_ambient_scripts_update(world, player));
        (co_await render_frame(world, player, &player.presentation.position_snapshot, &player.presentation.rotation_snapshot));
        player.state.allow_near_actor_spawn = KF_ACTOR_NEAR_SPAWN_FORBIDDEN;
        if (player_current_map_attribute(world, player)
            == KF_MAP_ATTRIBUTE_WARP) {
            if (!map_cells_equal(player.state.previous_map_cell, player.state.motion_state.map_cell)) {
                if ((co_await player_warp_trigger_update(world, player)) != 0) {
                    game_next_overlay_mode = KF_OVERLAY_MODE_ENDING;
                    (co_await player_warp_shimmer_at_player(world, player, KF_WARP_SHIMMER_GROW_KEEP));
                    (co_await display_play_transition());
                    (co_await audio_stop_sequence_master_fade(ENDING_MASTER_FADE_STEP_Q8));
                    break;
                }
                player.state.previous_map_cell.x = player.state.motion_state.map_cell.x;
                player.state.previous_map_cell.z = player.state.motion_state.map_cell.z;
            }
        } else {
            player.state.previous_map_cell.z = KF_MAP_CELL_COORD_INVALID;
            player.state.previous_map_cell.x = KF_MAP_CELL_COORD_INVALID;
        }
    }
    game_shutdown();
}

void game_shutdown(void)
{
    audio_close_vab(audio_state);
}

kf::FrameTask<void> frame_pacer_wait(void)
{
    const auto now = kf::host_clock_tick();
    const auto target = frame_pacer_last_tick + FRAME_PACER_INTERVAL_TICKS;
    if (now < target)
        co_await kf::FrameDelay {static_cast<unsigned>(target - now)};
    frame_pacer_last_tick = kf::host_clock_tick();
}

void game_reset_module_state(void)
{
    kf::restore_initial_value<frame_pacer_last_tick>();
    kf::restore_initial_value<game_next_overlay_mode>();
}
