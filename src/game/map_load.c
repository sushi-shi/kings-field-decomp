#include <kf/address.h>
#include <kf/semantic_types.h>

/*
 * Map-load band 0x80035e44..0x800365f8 (GAME.EXE).
 *
 * func_80035e44 is the per-floor world-state RESTORE routine: the exact inverse
 * of func_80035b5c (map_events.c), which serialises the live event, actor, and
 * map-object state into the DAT_8009ddb4 world-state block. It reads the same
 * 1700-byte per-floor record (base - 1690 + 1700 * current_floor), and when its
 * marker byte is 1 it rebuilds the eight map events, the live-actor lifecycle
 * overrides, the 190 map-object ids, the linked-object payloads, and the two
 * effect-object pools (objects[160..169] and objects[170..189]). The common
 * tail dispatches a per-floor scripted setup on the current floor (1..5).
 *
 * map_event_pool and the per-floor records share one contiguous BSS aggregate
 * reached through a single DAT_8009ddb4 base register (the event pool is
 * base - 556); separate globals cannot reproduce that base, so a register/
 * scheduling residue is expected here exactly as documented for func_80035b5c.
 *
 * func_800364e0 walks the eight-record map_event_pool and refreshes the image
 * of every active (state == 1) event. func_80036554 loads the current floor:
 * map_resources_load, the world-state restore, the event refresh, func_8001bae4,
 * then copies colour_matrix_table[3] into the render lighting matrix.
 */

extern KfPlayerState player_state;
extern KfMapEvent map_event_pool[8];
extern KfActorState actor_state;
extern KfMapObjectState map_object_state;
extern KfRenderState render_state;
extern MATRIX color_matrix_table[7];
extern u8 map_floor_height_grid[100][100];

/* Start of the per-floor persistent world-state block (see func_80035b5c). */
extern u32 DAT_8009ddb4;
extern u8 DAT_8009eafc;
extern u8 DAT_8009f844;
extern u8 DAT_8009f845;
extern u8 DAT_8009f846;
extern u8 boss_defeat_complete;
/* Player inventory / key-item flag array. */
extern u8 DAT_800652a8[240];

extern int rand(void);
extern void map_resources_load(u8 floor, u8 map_variant);
extern void map_event_refresh_image_for_progress(KfMapEvent *event);
extern void func_8001bae4(s32 floor);
extern void map_apply_copy_region(u8 region_id);
extern s32 actor_pool_find_at_tile(u8 tile_x, u8 tile_z);
extern void map_object_pool_clear_link(u8 link_id);
extern void map_object_pool_trigger_link(u8 link_id);
extern void actor_pool_begin_death_by_definition(u16 definition_id);

/* func_80035e44 per-floor scripted-setup jump table (floors 1..5). */
RODATA(0x80012bfc, 0x14)

ADDRESS(0x80035e44, 0x69c)
void func_80035e44(void)
{
    u8 *base = (u8 *)&DAT_8009ddb4;
    u8 *in;
    KfMapEvent *event;
    KfMapObject *object;
    s32 i;
    s32 count;

    in = base - 1690 + 1700 * player_state.progress_state.current_floor;
    if (*in++ == 1) {
        event = (KfMapEvent *)(base - 556);
        for (i = 0; i < 8; i++, event++) {
            event->state = *in++;
            event->image_limit = *in++;
            event->image_index = *in++;
            event->image_dirty = *in++;
            event->tag.bytes[event->image_index - 1] = *in++;
            event->image_delay = *in++;
            event->unknown_0d = *in++;
        }

        count = *in++;
        while (count-- != 0) {
            i = *in++;
            actor_state.actors[i].lifecycle = *in++;
        }

        object = &map_object_state.objects[0];
        for (i = 0; i < 190; i++, object++) {
            object->object_id = *in++;
        }

        count = *in++;
        while (count-- != 0) {
            u8 *link;
            s32 k;

            object = &map_object_state.objects[*in++];
            link = (u8 *)&object->link;
            k = 7;
            do {
                *link++ = *in++;
            } while (--k != -1);
        }

        object = &map_object_state.objects[160];
        for (i = 0; i < 10; i++, object++) {
            object->cell_x = *in++;
            object->cell_z = *in++;
            object->position_x =
                object->cell_x * 2000 + ((rand() * 2000) >> 15);
            object->position_z =
                object->cell_z * 2000 + ((rand() * 2000) >> 15);
            object->position_y =
                -(map_floor_height_grid[object->cell_z][object->cell_x] * 100);
            object->rotation.x = 0;
            object->rotation.y = 0;
            object->rotation.z = 0;
            *(u16 *)&object->link = *in++;
            *(u16 *)&object->link |= *in++ << 8;
            object->link.spawn_sequence = 0;
            object->link.vertical_velocity = 0;
        }

        object = &map_object_state.objects[170];
        for (i = 0; i < 20; i++, object++) {
            object->cell_x = *in++;
            object->cell_z = *in++;
            object->position_x =
                object->cell_x * 2000 + ((rand() * 2000) >> 15);
            object->position_z =
                object->cell_z * 2000 + ((rand() * 2000) >> 15);
            object->position_y =
                -(map_floor_height_grid[object->cell_z][object->cell_x] * 100);
            if (object->object_id < 43) {
                object->rotation.x = 0x400;
            } else if (object->object_id < 48) {
                object->rotation.x = 0;
            }
            object->rotation.z = 0;
            *(u16 *)&object->link = 0;
            object->link.spawn_sequence = 0;
            object->link.vertical_velocity = 0;
            object->rotation.y = *in++ << 4;
        }
    }

    switch (player_state.progress_state.current_floor) {
    case 1:
        if (((u8 *)&DAT_8009ddb4)[2] == 1) {
            map_apply_copy_region(1);
        }
        if (((u8 *)&DAT_8009ddb4)[1] != 2) {
            i = actor_pool_find_at_tile(7, 0x28);
            if (i != -1) {
                actor_state.actors[i].lifecycle = 3;
            }
        }
        if (DAT_8009f845 == 1) {
            map_object_pool_clear_link(0x33);
        }
        break;
    case 2:
        if (DAT_8009f845 == 1) {
            map_object_pool_clear_link(0x33);
        }
        if (player_state.progress_state.highest_floor >= 3) {
            map_event_pool[0].state = 3;
        }
        break;
    case 3:
        if (DAT_8009f845 == 1) {
            map_object_pool_clear_link(0x33);
        }
        if (DAT_8009eafc == 4) {
            map_apply_copy_region(2);
            map_apply_copy_region(3);
        }
        break;
    case 5:
        if (DAT_8009f844 == 1) {
            map_event_pool[1].state = 1;
        }
        if (DAT_8009f846 == 0) {
            ((u8 *)&actor_state)[0x438] = 0xff;
            ((u8 *)&actor_state)[0x43e] = 0xff;
            ((u8 *)&actor_state)[0x43f] = 0xff;
            ((u8 *)&actor_state)[0x440] = 0xff;
            ((u8 *)&actor_state)[0x441] = 0xff;
        } else {
            map_apply_copy_region(4);
        }
        if (DAT_800652a8[0xa] != 0 || DAT_800652a8[0xb] != 0
                || DAT_8009f845 == 1) {
            map_object_pool_clear_link(0x34);
        }
        if (boss_defeat_complete != 0) {
            map_object_pool_trigger_link(0xd);
            actor_pool_begin_death_by_definition(0);
            actor_pool_begin_death_by_definition(2);
            actor_pool_begin_death_by_definition(3);
            actor_pool_begin_death_by_definition(4);
        }
        break;
    default:
        break;
    }
}

ADDRESS(0x800364e0, 0x74)
void func_800364e0(void)
{
    KfMapEvent *event = map_event_pool;
    u16 index = 7;

    do {
        if (event->state == 1) {
            map_event_refresh_image_for_progress(event);
        }
        event++;
    } while (index-- != 0);
}

ADDRESS(0x80036554, 0xa4)
void func_80036554(void)
{
    map_resources_load(player_state.progress_state.current_floor,
                       player_state.map_variant);
    func_80035e44();
    func_800364e0();
    func_8001bae4(player_state.progress_state.current_floor);
    render_state.unknown_80 = color_matrix_table[3];
}
