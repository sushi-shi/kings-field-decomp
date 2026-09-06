#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_player.h>
#include <kf/game_collision.h>
#include <kf/game.h>

DATA(0x80056248, 0x20)
static MATRIX actor_transform_color_matrix = {
    {{250, 100, 500}, {250, 100, 500}, {250, 100, 500}}, {0, 0, 0}
};

/*
 * Player warp / floor-transition band 0x80036618..0x80036d3c (GAME.EXE).
 *
 * player_warp_shimmer runs the warp shimmer: it spawns four type-0x15 effects at the
 * player position, animates their intensity/rotation over 48 frames (playing
 * gameplay_sound_ref_6 on frame 8), and, unless mode 2 keeps them, releases the
 * pool records afterwards. mode 0/2 fade the shimmer in (delta +0x100 from 0),
 * mode 1 fades it out (delta -0x100 from 0x2000).
 *
 * player_warp_change_floor is the change-floor warp: shimmer out, reload the world through
 * map_unload_floor/map_load_floor, record the new floor/variant, snap the camera to
 * the centre of its 2000-unit cell, and shimmer back in. player_warp_same_floor is the
 * same-floor teleport: shimmer out, drop the old broad-phase occupancy, swap the
 * map variant and its assets, move to an explicit cell, and shimmer back in.
 *
 * player_warp_trigger_update runs every frame from game_main_loop: it dispatches on the
 * current floor (jump table at 0x80012c14) and, when the player's current map
 * cell matches a scripted trigger, performs the corresponding warp.
 *
 * actor_transform_definition5_to6 is the colour-fade transition called for
 * floor 4 actor definition 5: it sets map events 1 and 2 to state 3, fades
 * the GTE colour matrix toward the global target while raising and spinning
 * the actor, changes its definition to 6, then reverses the fade and motion.
 *
 * The KfEffectRecord layout is modelled in kf/game_effect.h; the shimmer
 * reuses the record's rotation_y halfword as its rotation phase and scale_y as
 * its fade intensity.
 */

#define EFFECT_ROTATION_PHASE(e) ((e)->rotation_y)
#define EFFECT_INTENSITY(e) ((e)->scale_y)

ADDRESS(0x80036618, 0x238)
void player_warp_shimmer(s32 mode, VECTOR *position)
{
    KfEffectRecord *effects[4];
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
    case 0:
    case 2:
        intensity = 0;
        intensity_delta = 0x100;
        break;
    case 1:
        intensity = 0x2000;
        intensity_delta = -0x100;
        break;
    }

    scratch.position.vx = position->vx;
    scratch.position.vz = position->vz;
    scratch.position.vy = position->vy;
    display_flip_buffer_index();
    cursor = effects;
    for (i = 3; i != -1; i--) {
        effect = effect_pool_construct(0xa, 0x11, 0x15, position, &scratch.direction);
        EFFECT_INTENSITY(effect) = intensity;
        *cursor++ = effect;
    }

    display_flip_buffer_index();
    render_frame(&player_state.camera_position, &player_state.camera_rotation);
    display_flip_buffer_index();
    render_frame(&player_state.camera_position, &player_state.camera_rotation);

    for (frame = 0; frame < 48; frame++) {
        cursor = effects;
        if (frame == 8) {
            sound_ref_play(&gameplay_sound_ref_6, 0x7f);
        }
        for (i = 0; i < 4; i++) {
            effect = *cursor++;

            if (i * 8 < frame) {
                u16 current_intensity = EFFECT_INTENSITY(effect);

                if (current_intensity < 8193) {
                    EFFECT_INTENSITY(effect) = intensity_delta + current_intensity;
                }
            }
            EFFECT_ROTATION_PHASE(effect) =
                (EFFECT_ROTATION_PHASE(effect) + 512) & 0xfff;
        }
        render_frame(&player_state.camera_position, &player_state.camera_rotation);
        frame_pacer_wait();
    }

    if (mode_value != 2) {
        cursor = effects;
        for (i = 3; i != -1; i--) {
            effect = *cursor++;
            effect->type = 0xff;
        }
    }
}

ADDRESS(0x80036850, 0x15c)
void player_warp_change_floor(s32 floor, u32 variant)
{
    VECTOR position;

    position.vx = player_state.camera_position.vx;
    position.vz = player_state.camera_position.vz;
    position.vy = player_state.floor_height;
    player_warp_shimmer(0, &position);
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
    player_warp_shimmer(1, &position);
}

ADDRESS(0x800369ac, 0x144)
void player_warp_same_floor(u32 variant, s32 cell_x, s32 cell_z)
{
    VECTOR position;
    u8 previous_variant;

    position.vx = player_state.camera_position.vx;
    position.vz = player_state.camera_position.vz;
    position.vy = player_state.floor_height;
    player_warp_shimmer(0, &position);
    collision_adjust_cell_occupancy(player_state.map_cell.x,
                                    player_state.map_cell.z, -1);
    pool_release_all();
    previous_variant = player_state.map_variant;
    player_state.map_variant = variant;
    map_variant_assets_load();
    if (player_state.progress_state.current_floor == 5) {
        if (player_state.map_variant == 3 || previous_variant == 3) {
            audio_play_current_map_sequence();
        }
    }
    player_state.camera_position.vx = cell_x * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    position.vx = player_state.camera_position.vx;
    player_state.camera_position.vz = cell_z * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    position.vz = player_state.camera_position.vz;
    player_sync_position_to_map();
    position.vy = player_state.floor_height;
    player_warp_shimmer(1, &position);
}

/* player_warp_trigger_update scripted-trigger jump table (current floor 1..5). */
RODATA(0x80012c14, 0x14)

ADDRESS(0x80036af0, 0x24c)
u32 player_warp_trigger_update(void)
{
    u32 cell;
    s32 destination_floor;
    u8 destination_variant = 0;

    /* The aligned word spans pitch_step and map_cell; mask out pitch_step. */
    switch (player_state.progress_state.current_floor) {
    case 1:
        cell = *(u32 *)&player_state.motion_state.pitch_step & 0xffff0000;
        if (cell == 0x1d380000) {
            destination_floor = 2;
change_floor:
            player_warp_change_floor(destination_floor, destination_variant);
        } else if (cell == 0x190b0000) {
            destination_floor = 3;
            goto change_floor;
        } else if (cell == 0x27230000) {
            goto change_to_floor4;
        } else if (cell == 0x0f020000) {
            if (boss_defeat_complete) {
                return 1;
            }
        }
        break;
    case 2:
        cell = *(u32 *)&player_state.motion_state.pitch_step & 0xffff0000;
        if (cell == 0x1d380000) {
            destination_floor = 1;
            goto change_floor;
        } else if (cell == 0x1c120000) {
            destination_floor = 3;
            goto change_floor;
        }
        break;
    case 3:
        cell = *(u32 *)&player_state.motion_state.pitch_step & 0xffff0000;
        if (cell == 0x190b0000) {
            destination_floor = 1;
            goto change_floor;
        } else if (cell == 0x1c120000) {
            destination_floor = 2;
            goto change_floor;
        } else if (cell == 0x07160000 || cell == 0x2b5c0000) {
change_to_floor4:
            destination_floor = 4;
            goto change_floor;
        }
        break;
    case 4:
        cell = *(u32 *)&player_state.motion_state.pitch_step & 0xffff0000;
        if (cell == 0x27230000) {
            destination_floor = 1;
            goto change_floor;
        } else if (cell == 0x07160000) {
            destination_floor = 3;
            goto change_floor;
        } else if (cell == 0x27450000) {
            destination_floor = 5;
            destination_variant = 1;
            goto change_floor;
        } else if (cell == 0x2b5c0000) {
            destination_floor = 3;
            goto change_floor;
        }
        break;
    case 5:
        cell = *(u32 *)&player_state.motion_state.pitch_step & 0xffff0000;
        if (cell == 0x27450000) {
            goto change_to_floor4;
        } else if (cell == 0x463d0000) {
            player_warp_same_floor(2, 0x12, 0x25);
        } else if (cell == 0x12250000) {
            player_warp_same_floor(1, 0x46, 0x3d);
        } else if (cell == 0x05180000) {
            player_warp_same_floor(3, 0x27, 0x2f);
        } else if (cell == 0x272f0000) {
            if (!boss_defeat_complete) {
                player_warp_same_floor(2, 5, 0x19);
            } else {
                return 1;
            }
        } else if (cell == 0x05250000) {
            player_warp_same_floor(1, 0xe, 0x4f);
        } else if (cell == 0x0e4f0000) {
            player_warp_same_floor(2, 5, 0x25);
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

    map_event_pool[1].state = 3;
    map_event_pool[2].state = 3;
    ReadColorMatrix(&saved);
    for (blend = 0; blend < 4097; blend += 64) {
        lighting_set_color_matrix(&saved, &actor_transform_color_matrix, blend);
        actor->position.vy += 40;
        actor->rotation.y += 64;
        render_frame(0, 0);
        frame_pacer_wait();
    }
    actor->definition_id = 6;
    for (blend = 4096; blend >= 0; blend -= 64) {
        lighting_set_color_matrix(&saved, &actor_transform_color_matrix, blend);
        actor->position.vy -= 40;
        actor->rotation.y -= 64;
        render_frame(0, 0);
        frame_pacer_wait();
    }
    lighting_set_active_color_matrix(KF_GAME_COLOR_DEFAULT);
}
