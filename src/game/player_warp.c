#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_player.h>
#include <kf/game_collision.h>
#include <kf/game.h>

DATA(0x80056248, 0x20)
static MATRIX actor_transform_color_matrix = {
    {{250, 100, 500}, {250, 100, 500}, {250, 100, 500}}, {0, 0, 0}
};

enum {
    WARP_SHIMMER_COUNT = 4,
    WARP_SHIMMER_OWNER_ID = 10,
    WARP_SHIMMER_TYPE = 0x11,
    WARP_SHIMMER_KIND = 0x15,
    WARP_SHIMMER_TALL_SCALE = 0x2000,
    WARP_SHIMMER_SCALE_STEP = 0x100,
    WARP_SHIMMER_YAW_STEP = 512,
    WARP_SHIMMER_FRAMES = 48,
    WARP_SHIMMER_SOUND_FRAME = 8,
    WARP_SHIMMER_STAGGER_FRAMES = 8,
    WARP_DEFAULT_VARIANT = 0,
    WARP_CELL_X_SHIFT = 24,
    WARP_CELL_Z_SHIFT = 16,
    ACTOR_TRANSFORM_RESULT_DEFINITION = 6
};

#define WARP_CELL_KEY_MASK 0xffff0000
#define WARP_CELL_KEY(x, z) \
    (((u32)(x) << WARP_CELL_X_SHIFT) | ((u32)(z) << WARP_CELL_Z_SHIFT))

/* The shimmer reuses rotation_y as phase and scale_y as intensity. */
#define EFFECT_ROTATION_PHASE(e) ((e)->rotation.vy)
#define EFFECT_INTENSITY(e) ((e)->scale_y)

ADDRESS(0x80036618, 0x238)
void player_warp_shimmer(s32 mode, VECTOR *position)
{
    KfEffectRecord *effects[WARP_SHIMMER_COUNT];
    KfEffectRecord **cursor;
    KfEffectRecord *effect;
    struct {
        VECTOR position;
        SVECTOR direction;
    } scratch;
    s16 intensity;
    s16 intensity_delta;
    s16 frame;
    s16 i;
    s16 mode_value = mode;

    switch (mode_value) {
    case KF_WARP_SHIMMER_GROW_REMOVE:
    case KF_WARP_SHIMMER_GROW_KEEP:
        intensity = 0;
        intensity_delta = WARP_SHIMMER_SCALE_STEP;
        break;
    case KF_WARP_SHIMMER_SHRINK_REMOVE:
        intensity = WARP_SHIMMER_TALL_SCALE;
        intensity_delta = -WARP_SHIMMER_SCALE_STEP;
        break;
    }

    scratch.position.vx = position->vx;
    scratch.position.vz = position->vz;
    scratch.position.vy = position->vy;
    display_flip_buffer_index();
    cursor = effects;
    for (i = WARP_SHIMMER_COUNT - 1; i != -1; i--) {
        effect = effect_pool_construct(WARP_SHIMMER_OWNER_ID, WARP_SHIMMER_TYPE,
                                       WARP_SHIMMER_KIND, position, &scratch.direction);
        EFFECT_INTENSITY(effect) = intensity;
        *cursor++ = effect;
    }

    display_flip_buffer_index();
    render_frame(&player_state.camera_position, &player_state.camera_rotation);
    display_flip_buffer_index();
    render_frame(&player_state.camera_position, &player_state.camera_rotation);

    for (frame = 0; frame < WARP_SHIMMER_FRAMES; frame++) {
        cursor = effects;
        if (frame == WARP_SHIMMER_SOUND_FRAME) {
            sound_ref_play(&gameplay_sound_ref_6, KF_AUDIO_MAX_VOLUME);
        }
        for (i = 0; i < WARP_SHIMMER_COUNT; i++) {
            effect = *cursor++;

            if (i * WARP_SHIMMER_STAGGER_FRAMES < frame) {
                u16 current_intensity = EFFECT_INTENSITY(effect);

                if (current_intensity < WARP_SHIMMER_TALL_SCALE + 1) {
                    EFFECT_INTENSITY(effect) = intensity_delta + current_intensity;
                }
            }
            EFFECT_ROTATION_PHASE(effect) =
                (EFFECT_ROTATION_PHASE(effect) + WARP_SHIMMER_YAW_STEP)
                & KF_ANGLE_WRAP_MASK;
        }
        render_frame(&player_state.camera_position, &player_state.camera_rotation);
        frame_pacer_wait();
    }

    if (mode_value != KF_WARP_SHIMMER_GROW_KEEP) {
        cursor = effects;
        for (i = WARP_SHIMMER_COUNT - 1; i != -1; i--) {
            effect = *cursor++;
            effect->type = KF_EFFECT_SLOT_FREE;
        }
    }
}

ADDRESS(0x80036850, 0x15c)
void player_warp_change_floor(KfFloorId floor, u32 variant)
{
    VECTOR position;

    position.vx = player_state.camera_position.vx;
    position.vz = player_state.camera_position.vz;
    position.vy = player_state.floor_height;
    player_warp_shimmer(KF_WARP_SHIMMER_GROW_REMOVE, &position);
    map_unload_floor();
    player_state.progress_state.current_floor = floor;
    player_state.map_variant = variant;
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

ADDRESS(0x800369ac, 0x144)
void player_warp_same_floor(u32 variant, s32 cell_x, s32 cell_z)
{
    VECTOR position;
    u8 previous_variant;

    position.vx = player_state.camera_position.vx;
    position.vz = player_state.camera_position.vz;
    position.vy = player_state.floor_height;
    player_warp_shimmer(KF_WARP_SHIMMER_GROW_REMOVE, &position);
    collision_adjust_cell_occupancy(player_state.map_cell.x,
                                    player_state.map_cell.z, -1);
    pool_release_all();
    previous_variant = player_state.map_variant;
    player_state.map_variant = variant;
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

/* player_warp_trigger_update scripted-trigger jump table (current floor 1..5). */
RODATA(0x80012c14, 0x14)

ADDRESS(0x80036af0, 0x24c)
u32 player_warp_trigger_update(void)
{
    u32 cell;
    KfFloorId destination_floor;
    u8 destination_variant = WARP_DEFAULT_VARIANT;

    /* The aligned word spans pitch_step and map_cell; mask out pitch_step. */
    switch (player_state.progress_state.current_floor) {
    case KF_FLOOR_1:
        cell = *(u32 *)&player_state.motion_state.pitch_step & WARP_CELL_KEY_MASK;
        if (cell == WARP_CELL_KEY(29, 56)) {
            destination_floor = KF_FLOOR_2;
change_floor:
            player_warp_change_floor(destination_floor, destination_variant);
        } else if (cell == WARP_CELL_KEY(25, 11)) {
            destination_floor = KF_FLOOR_3;
            goto change_floor;
        } else if (cell == WARP_CELL_KEY(39, 35)) {
            goto change_to_floor4;
        } else if (cell == WARP_CELL_KEY(15, 2)) {
            if (boss_defeat_complete != KF_MAP_SCRIPT_UNSET) {
                return 1;
            }
        }
        break;
    case KF_FLOOR_2:
        cell = *(u32 *)&player_state.motion_state.pitch_step & WARP_CELL_KEY_MASK;
        if (cell == WARP_CELL_KEY(29, 56)) {
            destination_floor = KF_FLOOR_1;
            goto change_floor;
        } else if (cell == WARP_CELL_KEY(28, 18)) {
            destination_floor = KF_FLOOR_3;
            goto change_floor;
        }
        break;
    case KF_FLOOR_3:
        cell = *(u32 *)&player_state.motion_state.pitch_step & WARP_CELL_KEY_MASK;
        if (cell == WARP_CELL_KEY(25, 11)) {
            destination_floor = KF_FLOOR_1;
            goto change_floor;
        } else if (cell == WARP_CELL_KEY(28, 18)) {
            destination_floor = KF_FLOOR_2;
            goto change_floor;
        } else if (cell == WARP_CELL_KEY(7, 22) || cell == WARP_CELL_KEY(43, 92)) {
change_to_floor4:
            destination_floor = KF_FLOOR_4;
            goto change_floor;
        }
        break;
    case KF_FLOOR_4:
        cell = *(u32 *)&player_state.motion_state.pitch_step & WARP_CELL_KEY_MASK;
        if (cell == WARP_CELL_KEY(39, 35)) {
            destination_floor = KF_FLOOR_1;
            goto change_floor;
        } else if (cell == WARP_CELL_KEY(7, 22)) {
            destination_floor = KF_FLOOR_3;
            goto change_floor;
        } else if (cell == WARP_CELL_KEY(39, 69)) {
            destination_floor = KF_FLOOR_5;
            destination_variant = KF_FLOOR5_ENTRY_VARIANT;
            goto change_floor;
        } else if (cell == WARP_CELL_KEY(43, 92)) {
            destination_floor = KF_FLOOR_3;
            goto change_floor;
        }
        break;
    case KF_FLOOR_5:
        cell = *(u32 *)&player_state.motion_state.pitch_step & WARP_CELL_KEY_MASK;
        if (cell == WARP_CELL_KEY(39, 69)) {
            goto change_to_floor4;
        } else if (cell == WARP_CELL_KEY(70, 61)) {
            player_warp_same_floor(2, 18, 37);
        } else if (cell == WARP_CELL_KEY(18, 37)) {
            player_warp_same_floor(KF_FLOOR5_ENTRY_VARIANT, 70, 61);
        } else if (cell == WARP_CELL_KEY(5, 24)) {
            player_warp_same_floor(KF_FLOOR5_ALTERNATE_MUSIC_VARIANT, 39, 47);
        } else if (cell == WARP_CELL_KEY(39, 47)) {
            if (boss_defeat_complete == KF_MAP_SCRIPT_UNSET) {
                player_warp_same_floor(2, 5, 25);
            } else {
                return 1;
            }
        } else if (cell == WARP_CELL_KEY(5, 37)) {
            player_warp_same_floor(KF_FLOOR5_ENTRY_VARIANT, 14, 79);
        } else if (cell == WARP_CELL_KEY(14, 79)) {
            player_warp_same_floor(2, 5, 37);
        }
        break;
    }
    return 0;
}

ADDRESS(0x80036d3c, 0xf4)
void actor_transform_definition5_to6(KfActor *actor)
{
    MATRIX saved;
    s32 blend;

    map_event_pool[1].state = KF_MAP_EVENT_DISABLED;
    map_event_pool[2].state = KF_MAP_EVENT_DISABLED;
    ReadColorMatrix(&saved);
    /* Both blend endpoints execute: 65 motion updates per phase.
     * Y moves 40 world units per update; its design rationale is unresolved.
     * Blend and yaw advance by 1/64 of their full ranges, then reverse. */
    for (blend = 0; blend < KF_FIXED12_ONE + 1; blend += KF_FIXED12_ONE / 64) {
        lighting_set_color_matrix(&saved, &actor_transform_color_matrix, blend);
        actor->position.vy += 40;
        actor->rotation.y += KF_ANGLE_FULL_TURN / 64;
        render_frame(0, 0);
        frame_pacer_wait();
    }
    actor->definition_id = ACTOR_TRANSFORM_RESULT_DEFINITION;
    for (blend = KF_FIXED12_ONE; blend >= 0; blend -= KF_FIXED12_ONE / 64) {
        lighting_set_color_matrix(&saved, &actor_transform_color_matrix, blend);
        actor->position.vy -= 40;
        actor->rotation.y -= KF_ANGLE_FULL_TURN / 64;
        render_frame(0, 0);
        frame_pacer_wait();
    }
    lighting_set_active_color_matrix(KF_GAME_COLOR_DEFAULT);
}
