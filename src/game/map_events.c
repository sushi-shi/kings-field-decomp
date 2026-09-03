#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/*
 * Map-event runtime band 0x80035708..0x80035e14 (GAME.EXE).
 *
 * map_event_pool_update is the per-frame driver called by game_main_loop: it walks the
 * eight-record map_event_pool, dispatches each active event on its unknown_0e
 * kind (1 -> map_event_update_wander wander, 2 -> map_event_update_spinner spinner), advances the
 * image-animation counters, then runs two global countdowns that fire the
 * per-floor ambient scripts. map_world_state_persist serialises the live event, actor,
 * and map-object state into the map_world_state_base world-state block per floor and is
 * invoked on death restart, floor teleport, and from map_unload_floor.
 *
 * map_event_pool, current_map_event, map_event_animation_gate/b2/b4 and the per-floor save
 * records are one contiguous BSS aggregate in the original; map_event_update_spinner and
 * map_world_state_persist reach the pool through a single map_world_state_base base register,
 * which separate globals cannot reproduce (documented residue).
 */

/* Event-animation gate: nonzero three frames in four. */
/* Ambient floor-script countdown, reloaded to 10. */
/* Start of the persistent world-state block (save_system world_state base). */
extern u32 map_world_state_base;

extern void collision_adjust_cell_occupancy(u16 cell_x, u16 cell_z, s32 delta);
extern void audio_play_spatial_range(
    const SoundRef *sound, const VECTOR *position, s16 volume,
    s32 max_distance, s32 attenuation_distance);
extern int rand(void);

/* Per-floor ambient-event scripts dispatched by current_floor. */

/* map_event_pool_update current-floor dispatch jump table (cases 1..5). */
RODATA(0x80012be4, 0x14)

/* Reset the event-animation gate and ambient-script countdown at floor start. */
ADDRESS(0x800356e8, 0x20)
void map_event_timers_reset(void)
{
    map_event_animation_gate = 3;
    map_ambient_script_countdown = 10;
}

ADDRESS(0x80035708, 0x1d8)
void map_event_update_wander(void)
{
    KfMapEvent *event = current_map_event;
    struct KfVecXZs forward;
    VECTOR point;
    s16 heading;

    collision_adjust_cell_occupancy(event->cell_x, event->cell_z, -1);

    heading = angle_approach(event->rotation, event->rotation_target, 0x46);
    event->rotation = heading;
    angle_to_forward_xz(heading, &forward);
    vector2s_scale_shift11(0x14, (s16 *)&forward);

    point.vx = forward.x + event->reference_x;
    point.vz = forward.z + event->reference_z;

    if (collision_query_world(point.vx, 0xffff, point.vz, event->radius, 0, 0x8040) == (u32)-1) {
        event->reference_x = point.vx;
        event->reference_z = point.vz;
        event->cell_x = point.vx / 2000;
        event->cell_z = point.vz / 2000;
        event->unknown_10 = 0;
        if (event->rotation == event->rotation_target && rand() < 1584) {
            event->rotation_target = rand() >> 3;
        }
    } else {
        if (event->unknown_10 == 0 || event->rotation == event->rotation_target) {
            event->rotation_target = rand() >> 3;
            event->unknown_10 = 1;
        }
    }

    event->rotation_phase = (event->rotation_phase + 110) & 0xfff;
    collision_adjust_cell_occupancy(event->cell_x, event->cell_z, 1);
}

ADDRESS(0x800358e0, 0x8c)
void map_event_update_spinner(void)
{
    KfMapEvent *event = current_map_event;

    event->rotation_phase = (event->rotation_phase + 200) & 0xfff;

    if (player_state.progress_state.current_floor == 5
            && event == &map_event_pool[0]
            && map_event_pool[0].rotation_phase < 200) {
        audio_play_spatial_range(&gameplay_sound_ref_10,
            (const VECTOR *)&map_event_pool[0].reference_x,
            0x7f, 0x4650, 0xc350);
    }
}

ADDRESS(0x8003596c, 0x1f0)
void map_event_pool_update(void)
{
    KfMapEvent *event = map_event_pool;
    u16 index = 7;

    do {
        s32 state = event->state;

        if (state == 1) {
            map_event_set_current(event);

            if (event->unknown_0e == state) {
                goto call_wander;
            }
            if (event->unknown_0e == 2) {
                goto call_spinner;
            }
            goto advance_image;
        call_wander:
            map_event_update_wander();
            goto advance_image;
        call_spinner:
            map_event_update_spinner();
        advance_image:
            if (map_event_animation_gate == 0 && event->image_delay != 0) {
                event->image_delay--;
                if (event->image_delay == 0) {
                    s32 limit = event->tag.bytes[event->image_index - 1];
                    event->image_dirty++;
                    if (event->image_dirty >= limit) {
                        event->image_dirty = limit;
                    }
                }
            }
        }

        event++;
    } while (index-- != 0);

    {
        u16 *gate = &map_event_animation_gate;
        u16 current = *gate;

        *gate = current - 1;
        if (current == 0) {
            *gate = 3;
        }
    }

    if (map_ambient_script_countdown-- == 0) {
        map_ambient_script_countdown = 10;
        switch (player_state.progress_state.current_floor) {
        case 1:
            map_ambient_script_floor1();
            break;
        case 2:
            map_ambient_script_floor2();
            break;
        case 3:
            map_ambient_script_floor3();
            break;
        case 4:
            map_ambient_script_floor4();
            break;
        case 5:
            map_ambient_script_floor5();
            break;
        }
    }
}

ADDRESS(0x80035b5c, 0x2b8)
void map_world_state_persist(void)
{
    u8 *base = (u8 *)&map_world_state_base;
    u8 *out;
    u8 *count_slot;
    KfMapEvent *event;
    KfActor *actor;
    KfMapObject *object;
    s32 i;
    s32 active;

    out = base - 1690 + 1700 * player_state.progress_state.current_floor;
    *out++ = 1;

    event = (KfMapEvent *)(base - 556);
    for (i = 0; i < 8; i++, event++) {
        *out++ = event->state;
        *out++ = event->image_limit;
        *out++ = event->image_index;
        *out++ = event->image_dirty;
        *out++ = event->tag.bytes[event->image_index - 1];
        *out++ = event->image_delay;
        *out++ = event->unknown_0d;
    }

    count_slot = out++;
    active = 0;
    actor = &actor_state.actors[0];
    for (i = 0; i < 128; i++, actor++) {
        if (actor->slot_state == 1 || actor->slot_state == 3) {
            active++;
            *out++ = i;
            if (actor->lifecycle == 3) {
                *out++ = 3;
            } else {
                *out++ = 0;
            }
        }
    }
    *count_slot = active;

    object = &map_object_state.objects[0];
    for (i = 0; i < 190; i++, object++) {
        *out++ = object->object_id;
    }

    count_slot = out++;
    active = 0;
    object = &map_object_state.objects[0];
    for (i = 0; i < 160; i++, object++) {
        u8 id = object->object_id;
        u8 behavior;

        if (id == 0xff) {
            continue;
        }

        behavior = map_object_state.definitions[id].behavior_type;
        if ((behavior == 0xff || behavior == 0xd || behavior == 0x40
                || behavior == 0xe || behavior == 0x41)
                && object->action == 0xff) {
            continue;
        }

        active++;
        *out++ = i;
        {
            u8 *link = (u8 *)&object->link;
            s32 k = 7;

            do {
                *out++ = *link++;
            } while (--k != -1);
        }
    }
    *count_slot = active;

    object = &map_object_state.objects[160];
    for (i = 0; i < 10; i++, object++) {
        *out++ = (u8)object->cell_x;
        *out++ = (u8)object->cell_z;
        *out++ = object->link.link_id;
        *out++ = (u8)(*(const u16 *)&object->link >> 8);
    }

    object = &map_object_state.objects[170];
    for (i = 0; i < 20; i++, object++) {
        *out++ = (u8)object->cell_x;
        *out++ = (u8)object->cell_z;
        *out++ = (u8)((u16)object->rotation.y >> 4);
    }
}

/* Tear down the live floor: release pooled allocations, close the map VAB, and
 * persist the world-state block. Called on floor teleport and death restart. */
ADDRESS(0x80035e14, 0x30)
void map_unload_floor(void)
{
    pool_release_all();
    audio_close_vab();
    map_world_state_persist();
}
