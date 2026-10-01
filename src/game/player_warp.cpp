#include <kf/game/system.h>
#include <kf/game/world.h>
#include <kf/game/audio.h>
#include <kf/lib/null.h>
#include <kf/game/graphics.h>
#include <kf/lib/bool.h>

#include <kf/game/map_data.h>
#include <kf/game/player.h>
#include <kf/game/collision.h>
#include <kf/game/game.h>

static MATRIX actor_transform_color_matrix = {
    {{250, 100, 500}, {250, 100, 500}, {250, 100, 500}}, {0, 0, 0}
};

enum {
    WARP_SHIMMER_OWNER_ID = 10,
    WARP_SHIMMER_SOUND_FRAME = 8,
    ACTOR_TRANSFORM_BLEND_INTERVALS = 64,
    ACTOR_TRANSFORM_Y_STEP = 40
};

namespace {
struct WarpCell { u8 x, z; };
constexpr WarpCell floor1_floor2_cell = {29, 56};
constexpr WarpCell floor1_floor3_cell = {25, 11};
constexpr WarpCell floor1_floor4_cell = {39, 35};
constexpr WarpCell floor1_exit_cell = {15, 2};
constexpr WarpCell floor2_floor3_cell = {28, 18};
constexpr WarpCell floor3_floor4_cell = {7, 22};
constexpr WarpCell floor3_floor4_alternate_cell = {43, 92};
constexpr WarpCell floor4_floor5_cell = {39, 69};
constexpr WarpCell floor5_entry_gate = {70, 61};
constexpr WarpCell floor5_inner_gate = {18, 37};
constexpr WarpCell floor5_ending_gate = {5, 24};
constexpr WarpCell floor5_ending_return = {5, 25};
constexpr WarpCell floor5_ending_arrival = {39, 47};
constexpr WarpCell floor5_inner_return = {5, 37};
constexpr WarpCell floor5_entry_return = {14, 79};
}

kf::FrameTask<void> player_warp_shimmer(WorldState &world, PlayerContext &player, KfWarpShimmerMode shimmer_mode, VECTOR *position)
{
    kf::InputContextScope input_context(kf::InputContext::Scripted);
    KfEffectRecord *effects[KF_CYLINDER_TRANSITION_COUNT];
    KfEffectRecord **cursor;
    KfEffectRecord *effect;
    struct {
        VECTOR position;
        SVECTOR direction;
    } scratch {};
    s16 scale_y;
    s16 scale_y_step;
    s16 frame;
    s16 i;
    KfEnumStorage<KfWarpShimmerMode, s16> mode_value = shimmer_mode;

    switch (mode_value) {
    case KF_WARP_SHIMMER_GROW_REMOVE:
    case KF_WARP_SHIMMER_GROW_KEEP:
        scale_y = 0;
        scale_y_step = KF_CYLINDER_TRANSITION_SCALE_STEP;
        break;
    case KF_WARP_SHIMMER_SHRINK_REMOVE:
        scale_y = KF_CYLINDER_TRANSITION_TALL_SCALE;
        scale_y_step = -KF_CYLINDER_TRANSITION_SCALE_STEP;
        break;
    }

    scratch.position.vx = position->vx;
    scratch.position.vz = position->vz;
    scratch.position.vy = position->vy;
    display_flip_buffer_index();
    cursor = effects;
    for (i = KF_CYLINDER_TRANSITION_COUNT - 1; i != -1; i--) {
        effect = effect_pool_construct(world, player,
            WARP_SHIMMER_OWNER_ID,
            KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS,
            KF_EFFECT_KIND_WARP_SHIMMER, position, &scratch.direction);
        if (effect) effect->scale_y = scale_y;
        *cursor++ = effect;
    }

    display_flip_buffer_index();
    (co_await render_frame(world, player, &player.state.camera_position, &player.state.camera_rotation));
    display_flip_buffer_index();
    (co_await render_frame(world, player, &player.state.camera_position, &player.state.camera_rotation));

    for (frame = 0; frame < KF_CYLINDER_TRANSITION_FRAMES; frame++) {
        cursor = effects;
        if (frame == WARP_SHIMMER_SOUND_FRAME) {
            sound_ref_play(audio_playback(player), &gameplay_sound_refs[KF_GAMEPLAY_SOUND_WARP_SHIMMER], KF_AUDIO_MAX_VOLUME);
        }
        for (i = 0; i < KF_CYLINDER_TRANSITION_COUNT; i++) {
            effect = *cursor++;
            if (!effect) continue;

            if (i * KF_CYLINDER_TRANSITION_STAGGER_FRAMES < frame) {
                u16 current_scale_y = effect->scale_y;

                if (current_scale_y < KF_CYLINDER_TRANSITION_TALL_SCALE + 1) {
                    effect->scale_y = scale_y_step + current_scale_y;
                }
            }
            effect->rotation.vector.vy =
                (effect->rotation.vector.vy + KF_CYLINDER_TRANSITION_YAW_STEP)
                & KF_ANGLE_WRAP_MASK;
        }
        (co_await render_frame(world, player, &player.state.camera_position, &player.state.camera_rotation));
    }

    if (mode_value != KF_WARP_SHIMMER_GROW_KEEP) {
        cursor = effects;
        for (i = KF_CYLINDER_TRANSITION_COUNT - 1; i != -1; i--) {
            effect = *cursor++;
            if (effect) effect->type = KF_EFFECT_SLOT_FREE;
        }
    }

}

kf::FrameTask<void> player_warp_change_floor(WorldState &world, PlayerContext &player, KfFloorId floor, KfMapVariant map_variant)
{
    VECTOR position;

    player_get_floor_position(player, position);
    (co_await player_warp_shimmer(world, player, KF_WARP_SHIMMER_GROW_REMOVE, &position));
    map_unload_floor(world, player);
    player.state.progress_state.current_floor = floor;
    player.state.map_variant = map_variant;
    if (player.state.progress_state.highest_floor < floor) {
        player.state.progress_state.highest_floor = floor;
    }
    (co_await map_load_floor(world, player));
    player.state.camera_position.vx =
        player.state.camera_position.vx / KF_MAP_TILE_SIZE * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    position.vx = player.state.camera_position.vx;
    player.state.camera_position.vz =
        player.state.camera_position.vz / KF_MAP_TILE_SIZE * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    position.vz = player.state.camera_position.vz;
    // Loading already synchronized height, motion and occupancy. Centering
    // within that same cell must not register the player a second time.
    position.vy = player.state.foot_height;
    (co_await player_warp_shimmer(world, player, KF_WARP_SHIMMER_SHRINK_REMOVE, &position));
}

kf::FrameTask<void> player_warp_same_floor(WorldState &world, PlayerContext &player, KfMapVariant map_variant, s32 cell_x, s32 cell_z)
{
    VECTOR position;
    KfMapVariant previous_variant;

    player_get_floor_position(player, position);
    (co_await player_warp_shimmer(world, player, KF_WARP_SHIMMER_GROW_REMOVE, &position));
    collision_adjust_cell_occupancy(world, player.state.motion_state.map_cell.x,
                                    player.state.motion_state.map_cell.z, -1);
    animation_cache_release_all();
    previous_variant = player.state.map_variant;
    player.state.map_variant = map_variant;
    map_variant_assets_load(world, player);
    if (player.state.progress_state.current_floor == KF_FLOOR_5) {
        if (player.state.map_variant == KF_FLOOR5_ALTERNATE_MUSIC_VARIANT
            || previous_variant == KF_FLOOR5_ALTERNATE_MUSIC_VARIANT) {
            (co_await audio_play_current_map_sequence(player));
        }
    }
    player.state.camera_position.vx = cell_x * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    position.vx = player.state.camera_position.vx;
    player.state.camera_position.vz = cell_z * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    position.vz = player.state.camera_position.vz;
    player_sync_position_to_map(world, player);
    position.vy = player.state.foot_height;
    (co_await player_warp_shimmer(world, player, KF_WARP_SHIMMER_SHRINK_REMOVE, &position));
}

static constexpr bool warp_cell_matches(KfMapCellCoordinates cell, WarpCell warp)
{
    return cell.x == warp.x && cell.z == warp.z;
}

kf::FrameTask<KfBoolU32> player_warp_trigger_update(WorldState &world, PlayerContext &player)
{
    const auto cell = player.state.motion_state.map_cell;

    switch (player.state.progress_state.current_floor) {
    case KF_FLOOR_FORCE_RELOAD:
        // This resource-load sentinel identifies no floor exit.
        break;
    case KF_FLOOR_1:
        if (warp_cell_matches(cell, floor1_floor2_cell)) {
            (co_await player_warp_change_floor(world, player, KF_FLOOR_2, KF_MAP_VARIANT_DEFAULT));
        } else if (warp_cell_matches(cell, floor1_floor3_cell)) {
            (co_await player_warp_change_floor(world, player, KF_FLOOR_3, KF_MAP_VARIANT_DEFAULT));
        } else if (warp_cell_matches(cell, floor1_floor4_cell)) {
            (co_await player_warp_change_floor(world, player, KF_FLOOR_4, KF_MAP_VARIANT_DEFAULT));
        } else if (warp_cell_matches(cell, floor1_exit_cell)) {
            if (map_floor_script(world, KF_FLOOR_5).floor5.boss_defeat != KF_MAP_SCRIPT_UNSET) {
                co_return true;
            }
        }
        break;
    case KF_FLOOR_2:
        if (warp_cell_matches(cell, floor1_floor2_cell)) {
            (co_await player_warp_change_floor(world, player, KF_FLOOR_1, KF_MAP_VARIANT_DEFAULT));
        } else if (warp_cell_matches(cell, floor2_floor3_cell)) {
            (co_await player_warp_change_floor(world, player, KF_FLOOR_3, KF_MAP_VARIANT_DEFAULT));
        }
        break;
    case KF_FLOOR_3:
        if (warp_cell_matches(cell, floor1_floor3_cell)) {
            (co_await player_warp_change_floor(world, player, KF_FLOOR_1, KF_MAP_VARIANT_DEFAULT));
        } else if (warp_cell_matches(cell, floor2_floor3_cell)) {
            (co_await player_warp_change_floor(world, player, KF_FLOOR_2, KF_MAP_VARIANT_DEFAULT));
        } else if (warp_cell_matches(cell, floor3_floor4_cell) || warp_cell_matches(cell, floor3_floor4_alternate_cell)) {
            (co_await player_warp_change_floor(world, player, KF_FLOOR_4, KF_MAP_VARIANT_DEFAULT));
        }
        break;
    case KF_FLOOR_4:
        if (warp_cell_matches(cell, floor1_floor4_cell)) {
            (co_await player_warp_change_floor(world, player, KF_FLOOR_1, KF_MAP_VARIANT_DEFAULT));
        } else if (warp_cell_matches(cell, floor3_floor4_cell)) {
            (co_await player_warp_change_floor(world, player, KF_FLOOR_3, KF_MAP_VARIANT_DEFAULT));
        } else if (warp_cell_matches(cell, floor4_floor5_cell)) {
            (co_await player_warp_change_floor(world, player, KF_FLOOR_5, KF_FLOOR5_ENTRY_VARIANT));
        } else if (warp_cell_matches(cell, floor3_floor4_alternate_cell)) {
            (co_await player_warp_change_floor(world, player, KF_FLOOR_3, KF_MAP_VARIANT_DEFAULT));
        }
        break;
    case KF_FLOOR_5:
        if (warp_cell_matches(cell, floor4_floor5_cell)) {
            (co_await player_warp_change_floor(world, player, KF_FLOOR_4, KF_MAP_VARIANT_DEFAULT));
        } else if (warp_cell_matches(cell, floor5_entry_gate)) {
            (co_await player_warp_same_floor(world, player, KF_MAP_VARIANT_2, floor5_inner_gate.x, floor5_inner_gate.z));
        } else if (warp_cell_matches(cell, floor5_inner_gate)) {
            (co_await player_warp_same_floor(world, player, KF_FLOOR5_ENTRY_VARIANT, floor5_entry_gate.x, floor5_entry_gate.z));
        } else if (warp_cell_matches(cell, floor5_ending_gate)) {
            (co_await player_warp_same_floor(world, player, KF_FLOOR5_ALTERNATE_MUSIC_VARIANT, floor5_ending_arrival.x, floor5_ending_arrival.z));
        } else if (warp_cell_matches(cell, floor5_ending_arrival)) {
            if (map_floor_script(world, KF_FLOOR_5).floor5.boss_defeat == KF_MAP_SCRIPT_UNSET) {
                (co_await player_warp_same_floor(world, player, KF_MAP_VARIANT_2, floor5_ending_return.x, floor5_ending_return.z));
            } else {
                co_return true;
            }
        } else if (warp_cell_matches(cell, floor5_inner_return)) {
            (co_await player_warp_same_floor(world, player, KF_FLOOR5_ENTRY_VARIANT, floor5_entry_return.x, floor5_entry_return.z));
        } else if (warp_cell_matches(cell, floor5_entry_return)) {
            (co_await player_warp_same_floor(world, player, KF_MAP_VARIANT_2, floor5_inner_return.x, floor5_inner_return.z));
        }
        break;
    }
    co_return false;
}

static constexpr unsigned floor4_transform_hidden_events[] = {1, 2};

bool actor_step_transform(WorldState &world, KfActor &actor)
{
    if (actor.transform_step == 0) {
        for (const auto slot : floor4_transform_hidden_events)
            world.map.events[slot].state = KF_MAP_EVENT_DISABLED;
    }
    if (actor.transform_step <= ACTOR_TRANSFORM_BLEND_INTERVALS) {
        actor.position.vy += ACTOR_TRANSFORM_Y_STEP;
        actor.rotation.angles.y += KF_ANGLE_FULL_TURN / ACTOR_TRANSFORM_BLEND_INTERVALS;
    } else {
        actor.definition_id = KF_FLOOR4_TRANSFORM_RESULT_DEFINITION;
        actor.position.vy -= ACTOR_TRANSFORM_Y_STEP;
        actor.rotation.angles.y -= KF_ANGLE_FULL_TURN / ACTOR_TRANSFORM_BLEND_INTERVALS;
    }
    return ++actor.transform_step == 2 * (ACTOR_TRANSFORM_BLEND_INTERVALS + 1);
}

kf::FrameTask<void> actor_transform_definition5_to6(WorldState &world, PlayerContext &player, KfActor *actor)
{
    kf::InputContextScope input_context(kf::InputContext::Scripted);
    MATRIX saved;
    s32 blend;

    world.map.events[floor4_transform_hidden_events[0]].state = KF_MAP_EVENT_DISABLED;
    world.map.events[floor4_transform_hidden_events[1]].state = KF_MAP_EVENT_DISABLED;
    saved = game_graphics_runtime.render_state.lighting.color_matrix;

    for (blend = 0; blend < KF_FIXED12_ONE + 1; blend += KF_FIXED12_ONE / ACTOR_TRANSFORM_BLEND_INTERVALS) {
        lighting_set_color_matrix(game_graphics_runtime.render_state, &saved, &actor_transform_color_matrix, blend);
        actor->position.vy += ACTOR_TRANSFORM_Y_STEP;
        actor->rotation.angles.y += KF_ANGLE_FULL_TURN / ACTOR_TRANSFORM_BLEND_INTERVALS;
        (co_await render_frame(world, player, NULL, NULL));
    }
    actor->definition_id = KF_FLOOR4_TRANSFORM_RESULT_DEFINITION;
    for (blend = KF_FIXED12_ONE; blend >= 0; blend -= KF_FIXED12_ONE / ACTOR_TRANSFORM_BLEND_INTERVALS) {
        lighting_set_color_matrix(game_graphics_runtime.render_state, &saved, &actor_transform_color_matrix, blend);
        actor->position.vy -= ACTOR_TRANSFORM_Y_STEP;
        actor->rotation.angles.y -= KF_ANGLE_FULL_TURN / ACTOR_TRANSFORM_BLEND_INTERVALS;
        (co_await render_frame(world, player, NULL, NULL));
    }
    lighting_set_active_color_matrix(KF_GAME_COLOR_DEFAULT);

}

void player_warp_reset_module_state(void)
{
    kf::restore_initial_value<actor_transform_color_matrix>();
}
