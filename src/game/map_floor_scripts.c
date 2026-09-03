#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/*
 * Per-floor ambient event scripts, band 0x80033ee4..0x800342e3 (GAME.EXE).
 *
 * func_8003596c (map_events.c) runs one of these every ~10 frames, indexed by
 * player_state.progress_state.current_floor (floor 1 -> func_80033f64, floor 2
 * -> func_800341ec, floor 3 -> func_8003425c). They watch the player's current
 * map cell and facing angle and drive a small per-floor state machine held in
 * the persistent world-state block DAT_8009ddb4, spawning actors, retexturing
 * map objects, teaching magic, and playing positional ambience.
 *
 * actor_pool_find_at_tile is the shared helper: it scans the 128-slot actor pool for the
 * first live actor sitting on a given map tile and returns its index.
 */

extern KfMagicRecord magic_records[24];

/* Persistent per-floor world-state block (save_system world_state base). */
extern u8 DAT_8009ddb4[4];
/* Camera-path / positional-audio data block; +0x40 is an ambience anchor. */

extern void audio_play_spatial_default_range(
    const SoundRef *sound, const VECTOR *position, s16 volume);
extern void notify_enqueue(s32 arg0);
extern int rand(void);

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
void func_80033f64(void)
{
    if (DAT_8009ddb4[3] == 1) {
        audio_play_spatial_default_range(
            &gameplay_sound_ref_5, (const VECTOR *)&DAT_800561c8[0x40], 0x73);
    }

    switch (DAT_8009ddb4[1]) {
    case 0:
        if (player_state.map_cell.x >= 7 && player_state.map_cell.z >= 31
            && player_state.map_cell.x < 12 && player_state.map_cell.z < 41) {
            DAT_8009ddb4[1] = 1;
        }
        break;
    case 1:
        if (player_state.map_cell.x < 2 || player_state.map_cell.z < 25
            || player_state.map_cell.x >= 14 || player_state.map_cell.z >= 46) {
            s32 actor_index;
            s32 object_index;

            DAT_8009ddb4[1] = 2;
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

    switch (DAT_8009ddb4[0]) {
    case 0:
        if (player_state.map_cell.x >= 2 && player_state.map_cell.z >= 27
            && player_state.map_cell.x < 5 && player_state.map_cell.z < 30) {
            DAT_8009ddb4[0] = 1;
        }
        break;
    case 1:
        if (player_state.map_cell.x < 2 || player_state.map_cell.z < 11
            || player_state.map_cell.x >= 28 || player_state.map_cell.z >= 41) {
            s32 object_index;

            DAT_8009ddb4[0] = 2;
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
void func_800341ec(void)
{
    if (map_event_pool[1].image_index == 2 && map_event_pool[1].image_dirty < 3
        && rand() < 4000) {
        audio_play_spatial_default_range(
            &gameplay_sound_ref_8, (const VECTOR *)&map_event_pool[1].reference_x, 0x7f);
    }
}

/* Floor 3 ambient script: healing altar that also teaches two spells. */
ADDRESS(0x8003425c, 0x88)
void func_8003425c(void)
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
