#include <kf/lib/null.h>
#include <kf/game/graphics.h>
#include <kf/lib/bool.h>

#include <kf/lib/map_data.h>
#include <kf/game/player.h>
#include <kf/game/collision.h>
#include <kf/game/game.h>

static MATRIX actor_transform_color_matrix = {
    {{250, 100, 500}, {250, 100, 500}, {250, 100, 500}}, {0, 0, 0}
};

enum {
    WARP_SHIMMER_OWNER_ID = 10,
    WARP_SHIMMER_SOUND_FRAME = 8,
    WARP_CELL_X_SHIFT = 24,
    WARP_CELL_Z_SHIFT = 16,
    ACTOR_TRANSFORM_BLEND_INTERVALS = 64,
    ACTOR_TRANSFORM_Y_STEP = 40
};

#define WARP_CELL_KEY_MASK 0xffff0000
#define WARP_CELL_KEY(x, z) \
    (((u32)(x) << WARP_CELL_X_SHIFT) | ((u32)(z) << WARP_CELL_Z_SHIFT))


namespace {
constexpr u32 floor1_floor2_cell = WARP_CELL_KEY(29, 56);
constexpr u32 floor1_floor3_cell = WARP_CELL_KEY(25, 11);
constexpr u32 floor1_floor4_cell = WARP_CELL_KEY(39, 35);
constexpr u32 floor1_exit_cell = WARP_CELL_KEY(15, 2);
constexpr u32 floor2_floor3_cell = WARP_CELL_KEY(28, 18);
constexpr u32 floor3_floor4_cell = WARP_CELL_KEY(7, 22);
constexpr u32 floor3_floor4_alternate_cell = WARP_CELL_KEY(43, 92);
constexpr u32 floor4_floor5_cell = WARP_CELL_KEY(39, 69);
struct WarpCell { u8 x, z; };
constexpr WarpCell floor5_entry_gate = {70, 61};
constexpr WarpCell floor5_inner_gate = {18, 37};
constexpr WarpCell floor5_ending_gate = {5, 24};
constexpr WarpCell floor5_ending_return = {5, 25};
constexpr WarpCell floor5_ending_arrival = {39, 47};
constexpr WarpCell floor5_inner_return = {5, 37};
constexpr WarpCell floor5_entry_return = {14, 79};
}

void player_warp_shimmer(KfWarpShimmerMode shimmer_mode, VECTOR *position)
{
    const auto input_context = kf::host_set_input_context(kf::InputContext::Scripted);
    KfEffectRecord *effects[KF_CYLINDER_TRANSITION_COUNT];
    KfEffectRecord **cursor;
    KfEffectRecord *effect;
    struct {
        VECTOR position;
        SVECTOR direction;
    } scratch;
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
        effect = effect_pool_construct(
            WARP_SHIMMER_OWNER_ID,
            KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS,
            KF_EFFECT_KIND_WARP_SHIMMER, position, &scratch.direction);
        effect->scale_y = scale_y;
        *cursor++ = effect;
    }

    display_flip_buffer_index();
    render_frame(&player_state.camera_position, &player_state.camera_rotation);
    display_flip_buffer_index();
    render_frame(&player_state.camera_position, &player_state.camera_rotation);

    for (frame = 0; frame < KF_CYLINDER_TRANSITION_FRAMES; frame++) {
        cursor = effects;
        if (frame == WARP_SHIMMER_SOUND_FRAME) {
            sound_ref_play(&gameplay_sound_refs[KF_GAMEPLAY_SOUND_WARP_SHIMMER], KF_AUDIO_MAX_VOLUME);
        }
        for (i = 0; i < KF_CYLINDER_TRANSITION_COUNT; i++) {
            effect = *cursor++;

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
        render_frame(&player_state.camera_position, &player_state.camera_rotation);
    }

    if (mode_value != KF_WARP_SHIMMER_GROW_KEEP) {
        cursor = effects;
        for (i = KF_CYLINDER_TRANSITION_COUNT - 1; i != -1; i--) {
            effect = *cursor++;
            effect->type = KF_EFFECT_SLOT_FREE;
        }
    }
    kf::host_set_input_context(input_context);
}

void player_warp_change_floor(KfFloorId floor, KfMapVariant map_variant)
{
    VECTOR position;

    PLAYER_FLOOR_POSITION(position);
    player_warp_shimmer(KF_WARP_SHIMMER_GROW_REMOVE, &position);
    map_unload_floor();
    player_state.progress_state.current_floor = floor;
    player_state.map_variant = map_variant;
    if (player_state.progress_state.highest_floor < floor) {
        player_state.progress_state.highest_floor = floor;
    }
    map_load_floor();
    player_state.camera_position.vx =
        player_state.camera_position.vx / KF_MAP_TILE_SIZE * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    position.vx = player_state.camera_position.vx;
    player_state.camera_position.vz =
        player_state.camera_position.vz / KF_MAP_TILE_SIZE * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    position.vz = player_state.camera_position.vz;
    player_sync_position_to_map();
    position.vy = player_state.floor_height;
    player_warp_shimmer(KF_WARP_SHIMMER_SHRINK_REMOVE, &position);
}

void player_warp_same_floor(KfMapVariant map_variant, s32 cell_x, s32 cell_z)
{
    VECTOR position;
    KfMapVariant previous_variant;

    PLAYER_FLOOR_POSITION(position);
    player_warp_shimmer(KF_WARP_SHIMMER_GROW_REMOVE, &position);
    collision_adjust_cell_occupancy(player_state.motion_state.fields.map_cell.coords.x,
                                    player_state.motion_state.fields.map_cell.coords.z, -1);
    pool_release_all();
    previous_variant = player_state.map_variant;
    player_state.map_variant = map_variant;
    map_variant_assets_load();
    if (player_state.progress_state.current_floor == KF_FLOOR_5) {
        if (player_state.map_variant == KF_FLOOR5_ALTERNATE_MUSIC_VARIANT
            || previous_variant == KF_FLOOR5_ALTERNATE_MUSIC_VARIANT) {
            audio_play_current_map_sequence();
        }
    }
    player_state.camera_position.vx = cell_x * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    position.vx = player_state.camera_position.vx;
    player_state.camera_position.vz = cell_z * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    position.vz = player_state.camera_position.vz;
    player_sync_position_to_map();
    position.vy = player_state.floor_height;
    player_warp_shimmer(KF_WARP_SHIMMER_SHRINK_REMOVE, &position);
}

KfBoolU32 player_warp_trigger_update(void)
{
    u32 cell;
    KfFloorId destination_floor;
    KfMapVariant destination_variant = KF_MAP_VARIANT_DEFAULT;

    switch (player_state.progress_state.current_floor) {
    case KF_FLOOR_FORCE_RELOAD:
        // This resource-load sentinel identifies no floor exit.
        break;
    case KF_FLOOR_1:
        cell = player_state.motion_state.words[2] & WARP_CELL_KEY_MASK;
        if (cell == floor1_floor2_cell) {
            destination_floor = KF_FLOOR_2;
change_floor:
            player_warp_change_floor(destination_floor, destination_variant);
        } else if (cell == floor1_floor3_cell) {
            destination_floor = KF_FLOOR_3;
            goto change_floor;
        } else if (cell == floor1_floor4_cell) {
            goto change_to_floor4;
        } else if (cell == floor1_exit_cell) {
            if (boss_defeat_complete != KF_MAP_SCRIPT_UNSET) {
                return KF_TRUE;
            }
        }
        break;
    case KF_FLOOR_2:
        cell = player_state.motion_state.words[2] & WARP_CELL_KEY_MASK;
        if (cell == floor1_floor2_cell) {
            destination_floor = KF_FLOOR_1;
            goto change_floor;
        } else if (cell == floor2_floor3_cell) {
            destination_floor = KF_FLOOR_3;
            goto change_floor;
        }
        break;
    case KF_FLOOR_3:
        cell = player_state.motion_state.words[2] & WARP_CELL_KEY_MASK;
        if (cell == floor1_floor3_cell) {
            destination_floor = KF_FLOOR_1;
            goto change_floor;
        } else if (cell == floor2_floor3_cell) {
            destination_floor = KF_FLOOR_2;
            goto change_floor;
        } else if (cell == floor3_floor4_cell || cell == floor3_floor4_alternate_cell) {
change_to_floor4:
            destination_floor = KF_FLOOR_4;
            goto change_floor;
        }
        break;
    case KF_FLOOR_4:
        cell = player_state.motion_state.words[2] & WARP_CELL_KEY_MASK;
        if (cell == floor1_floor4_cell) {
            destination_floor = KF_FLOOR_1;
            goto change_floor;
        } else if (cell == floor3_floor4_cell) {
            destination_floor = KF_FLOOR_3;
            goto change_floor;
        } else if (cell == floor4_floor5_cell) {
            destination_floor = KF_FLOOR_5;
            destination_variant = KF_FLOOR5_ENTRY_VARIANT;
            goto change_floor;
        } else if (cell == floor3_floor4_alternate_cell) {
            destination_floor = KF_FLOOR_3;
            goto change_floor;
        }
        break;
    case KF_FLOOR_5:
        cell = player_state.motion_state.words[2] & WARP_CELL_KEY_MASK;
        if (cell == floor4_floor5_cell) {
            goto change_to_floor4;
        } else if (cell == WARP_CELL_KEY(floor5_entry_gate.x, floor5_entry_gate.z)) {
            player_warp_same_floor(KF_MAP_VARIANT_2, floor5_inner_gate.x, floor5_inner_gate.z);
        } else if (cell == WARP_CELL_KEY(floor5_inner_gate.x, floor5_inner_gate.z)) {
            player_warp_same_floor(KF_FLOOR5_ENTRY_VARIANT, floor5_entry_gate.x, floor5_entry_gate.z);
        } else if (cell == WARP_CELL_KEY(floor5_ending_gate.x, floor5_ending_gate.z)) {
            player_warp_same_floor(KF_FLOOR5_ALTERNATE_MUSIC_VARIANT, floor5_ending_arrival.x, floor5_ending_arrival.z);
        } else if (cell == WARP_CELL_KEY(floor5_ending_arrival.x, floor5_ending_arrival.z)) {
            if (boss_defeat_complete == KF_MAP_SCRIPT_UNSET) {
                player_warp_same_floor(KF_MAP_VARIANT_2, floor5_ending_return.x, floor5_ending_return.z);
            } else {
                return KF_TRUE;
            }
        } else if (cell == WARP_CELL_KEY(floor5_inner_return.x, floor5_inner_return.z)) {
            player_warp_same_floor(KF_FLOOR5_ENTRY_VARIANT, floor5_entry_return.x, floor5_entry_return.z);
        } else if (cell == WARP_CELL_KEY(floor5_entry_return.x, floor5_entry_return.z)) {
            player_warp_same_floor(KF_MAP_VARIANT_2, floor5_inner_return.x, floor5_inner_return.z);
        }
        break;
    }
    return KF_FALSE;
}

static constexpr unsigned floor4_transform_hidden_events[] = {1, 2};

void actor_transform_definition5_to6(KfActor *actor)
{
    const auto input_context = kf::host_set_input_context(kf::InputContext::Scripted);
    MATRIX saved;
    s32 blend;

    map_event_pool[floor4_transform_hidden_events[0]].state = KF_MAP_EVENT_DISABLED;
    map_event_pool[floor4_transform_hidden_events[1]].state = KF_MAP_EVENT_DISABLED;
    saved = game_graphics_runtime.render_state.lighting.color_matrix;

    for (blend = 0; blend < KF_FIXED12_ONE + 1; blend += KF_FIXED12_ONE / ACTOR_TRANSFORM_BLEND_INTERVALS) {
        lighting_set_color_matrix(&saved, &actor_transform_color_matrix, blend);
        actor->position.vy += ACTOR_TRANSFORM_Y_STEP;
        actor->rotation.angles.y += KF_ANGLE_FULL_TURN / ACTOR_TRANSFORM_BLEND_INTERVALS;
        render_frame(NULL, NULL);
    }
    actor->definition_id = KF_FLOOR4_TRANSFORM_RESULT_DEFINITION;
    for (blend = KF_FIXED12_ONE; blend >= 0; blend -= KF_FIXED12_ONE / ACTOR_TRANSFORM_BLEND_INTERVALS) {
        lighting_set_color_matrix(&saved, &actor_transform_color_matrix, blend);
        actor->position.vy -= ACTOR_TRANSFORM_Y_STEP;
        actor->rotation.angles.y -= KF_ANGLE_FULL_TURN / ACTOR_TRANSFORM_BLEND_INTERVALS;
        render_frame(NULL, NULL);
    }
    lighting_set_active_color_matrix(KF_GAME_COLOR_DEFAULT);
    kf::host_set_input_context(input_context);
}


void player_warp_reset_module_state(void)
{
    kf::restore_initial_value<actor_transform_color_matrix>();
}
