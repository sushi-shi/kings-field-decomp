#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/*
 * Per-floor scripts, one contiguous run 0x80033ee4..0x800346a8 (GAME.EXE):
 * the shared actor-tile lookup, the floor 1..5 ambient scripts dispatched by
 * map_event_pool_update (map_events.c), and the floor 1..4 action scripts
 * dispatched by map_interaction_dispatch (map_interaction.c). The floor-4
 * ambient and action scripts are empty stubs. They watch the player's cell and
 * facing, drive a per-floor state machine in the persistent world-state block
 * map_world_state_base, spawn/retexture actors and objects, teach magic, and run
 * the full-screen colour-matrix reveal fade. Module boundary is WIP.
 *
 * actor_pool_find_at_tile is the shared helper: it scans the 128-slot actor
 * pool for the first live actor sitting on a given map tile and returns its
 * index.
 */


/* Progress-flag block raised at init and decremented on death restart. */
extern u8 DAT_800652a8[240];
/* Persistent per-floor world-state block (save_system world_state base). */
extern u8 map_world_state_base[4];
/* Camera-path / positional-audio data block; +0x40 ambience, +0x50 matrix. */

extern void audio_play_spatial_default_range(
    const SoundRef *sound, const VECTOR *position, s16 volume);
extern void render_frame(s32 first, s32 second);
extern int rand(void);

/* The two TIM cut-in paths shown by map_ambient_script_floor5. */
RODATA(0x80012a54, 0x28)

/* Scan the actor pool for the first live actor on map tile (tile_x, tile_z). */
ADDRESS(0x80033ee4, 0x80)
s32 actor_pool_find_at_tile(u8 tile_x, u8 tile_z)
{
    KfActor *actor = actor_state.actors;
    s16 index;

    for (index = 0; index < 128; index++, actor++) {
        if (actor->slot_state != 0xff && actor->tile_x == tile_x
            && actor->tile_z == tile_z) {
            return index;
        }
    }
    return -1;
}

/* Floor 1 ambient script. */
ADDRESS(0x80033f64, 0x288)
void map_ambient_script_floor1(void)
{
    if (map_world_state_base[3] == 1) {
        audio_play_spatial_default_range(
            &gameplay_sound_ref_5, (const VECTOR *)&DAT_800561c8[0x40], 0x73);
    }

    switch (map_world_state_base[1]) {
    case 0:
        if (player_state.map_cell.x >= 7 && player_state.map_cell.z >= 31
            && player_state.map_cell.x < 12 && player_state.map_cell.z < 41) {
            map_world_state_base[1] = 1;
        }
        break;
    case 1:
        if (player_state.map_cell.x < 2 || player_state.map_cell.z < 25
            || player_state.map_cell.x >= 14 || player_state.map_cell.z >= 46) {
            s32 actor_index;
            s32 object_index;

            map_world_state_base[1] = 2;
            actor_index = actor_pool_find_at_tile(7, 0x28);
            if (actor_index != -1) {
                actor_state.actors[actor_index].lifecycle = 0;
                actor_initialize_slot(actor_index);
            }
            object_index = map_object_pool_find_near_point(0x5208, 0x105b8, 0xbb8);
            if (object_index != -1) {
                map_object_state.objects[object_index].object_id = 0x5c;
            }
        }
        break;
    }

    switch (map_world_state_base[0]) {
    case 0:
        if (player_state.map_cell.x >= 2 && player_state.map_cell.z >= 27
            && player_state.map_cell.x < 5 && player_state.map_cell.z < 30) {
            map_world_state_base[0] = 1;
        }
        break;
    case 1:
        if (player_state.map_cell.x < 2 || player_state.map_cell.z < 11
            || player_state.map_cell.x >= 28 || player_state.map_cell.z >= 41) {
            s32 object_index;

            map_world_state_base[0] = 2;
            object_index = map_object_pool_find_near_point(0x2328, 0xdea8, 0xbb8);
            if (object_index != -1) {
                map_object_state.objects[object_index].object_id = 0xff;
            }
        }
        break;
    }
}

/* Floor 2 ambient script: random fire-crackle on the second map event. */
ADDRESS(0x800341ec, 0x70)
void map_ambient_script_floor2(void)
{
    if (map_event_pool[1].image_index == 2 && map_event_pool[1].image_dirty < 3
        && rand() < 4000) {
        audio_play_spatial_default_range(
            &gameplay_sound_ref_8, (const VECTOR *)&map_event_pool[1].reference_x, 0x7f);
    }
}

/* Floor 3 ambient script: healing altar that also teaches two spells. */
ADDRESS(0x8003425c, 0x88)
void map_ambient_script_floor3(void)
{
    if (player_state.map_cell.x >= 15 && player_state.map_cell.x < 18
        && player_state.map_cell.z == 0x40) {
        player_restore_vitals_with_color_cycle();
        if (magic_records[2].learned == 0 || magic_records[3].learned == 0) {
            magic_records[2].learned = 1;
            magic_records[3].learned = 1;
            notify_enqueue(1);
        }
    }
}

/* Floor 4 ambient script: empty stub. */
ADDRESS(0x800342e4, 0x8)
void map_ambient_script_floor4(void)
{
}

/* Floor-5 ambient script: a one-time scripted reveal at a fixed cell/heading. */
ADDRESS(0x800342ec, 0xf4)
void map_ambient_script_floor5(void)
{
    u8 *fired = &DAT_8009f846;

    if (*fired == 0 && player_state.map_cell.x >= 38
        && player_state.map_cell.x < 41 && player_state.map_cell.z == 7
        && (u16)player_state.camera_rotation.vy >= 1808
        && (u16)player_state.camera_rotation.vy < 2289) {
        *fired = 1;
        screen_show_image_until_input("TALK\\C17\\T55171.TIM");
        render_frame(0, 0);
        render_frame(0, 0);
        screen_show_image_until_input("TALK\\C17\\T55172.TIM");
        actor_state.definitions[7].action_animations[2] = 2;
        actor_state.definitions[7].action_animations[8] = 3;
        actor_state.definitions[7].action_animations[9] = 3;
        actor_state.definitions[7].action_animations[10] = 3;
        actor_state.definitions[7].action_animations[11] = 1;
        map_apply_copy_region(4);
    }
}

/* Floor-1 action script: reveal a passage once its progress flag is set. */
ADDRESS(0x800343e0, 0x58)
void map_action_script_floor1(void)
{
    if (DAT_800652a8[0x38] != 0 && map_world_state_base[2] == 0) {
        map_world_state_base[2] = 1;
        map_apply_copy_region(1);
        sound_ref_play(&gameplay_sound_ref_7, 0x64);
    }
}

/* Full-screen colour-matrix fade that reveals map event 3, then fades back. */
ADDRESS(0x80034438, 0x184)
void map_reveal_fade(void)
{
    MATRIX saved;
    s32 blend;

    saved = render_state.light_matrix_copy;

    for (blend = 0; blend < 4097; blend += 128) {
        lighting_set_color_matrix(&color_matrix_table[0], &color_matrix_table[3], blend);
        if (blend >= 1025) {
            map_event_pool[3].position_y -= 130;
            map_event_pool[3].rotation += 128;
        } else {
            matrix_interpolate(&saved, (const MATRIX *)&DAT_800561c8[0x50],
                               &render_state.light_matrix_copy, blend << 2);
        }
        render_frame(0, 0);
        frame_pacer_wait();
    }

    map_event_pool[3].state = 3;
    DAT_8009f844 = 1;

    for (blend = 0x1000; blend >= 0; blend -= 256) {
        lighting_set_color_matrix(&color_matrix_table[0], &color_matrix_table[3], blend);
        render_frame(0, 0);
        frame_pacer_wait();
    }

    lighting_set_active_color_matrix(0);
    render_state.light_matrix_copy = saved;
}

/* Floor-2 action script: run the reveal fade when event 3 is fully open. */
ADDRESS(0x800345bc, 0x54)
void map_action_script_floor2(void)
{
    if ((*(u32 *)&map_event_pool[3].image_limit & 0xffffff00) == 0x28010200
        && map_event_pool[3].state == 1) {
        map_reveal_fade();
    }
}

/* Floor-3 action script: teach two spells gated on flags and event state. */
ADDRESS(0x80034610, 0x90)
void map_action_script_floor3(void)
{
    if (DAT_800652a8[0x32] != 0) {
        if (magic_records[7].learned == 0) {
            magic_records[7].learned = 1;
            notify_enqueue(1);
        }
    }
    if ((*(u32 *)&map_event_pool[1].image_limit & 0xffffff00) == 0x28010300) {
        if (magic_records[5].learned == 0) {
            magic_records[5].learned = 1;
            notify_enqueue(1);
        }
    }
}

/* Floor 4 action script: empty stub. */
ADDRESS(0x800346a0, 0x8)
void map_action_script_floor4(void)
{
}
