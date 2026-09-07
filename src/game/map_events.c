#include <kf/address.h>
#include <kf/game_actor.h>
#include <kf/map_data.h>
#include <kf/game_map.h>
#include <kf/game_collision.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

enum {
    MAP_AMBIENT_COUNTDOWN_RELOAD = 10,
    MAP_EVENT_WANDER_TURN_STEP = 70,
    MAP_EVENT_WANDER_VECTOR_SCALE = 20,
    MAP_EVENT_WANDER_TURN_RANDOM_LIMIT = 1584,
    MAP_EVENT_RANDOM_YAW_SHIFT = 3,
    MAP_EVENT_LOOP_SOUND_MAX_DISTANCE = 18000,
    MAP_EVENT_LOOP_SOUND_ATTENUATION_DISTANCE = 50000
};

/*
 * Map-event runtime band 0x80035708..0x80035e14 (GAME.EXE).
 *
 * map_event_pool_update is the per-frame driver called by game_main_loop: it walks the
 * eight-record map_event_pool, dispatches each active event on its behavior
 * (wander or looping animation), advances dialogue page delays on a gated tick,
 * then runs the per-floor ambient scripts on their own countdown.
 * map_world_state_persist serialises the live event, actor,
 * and map-object state into the map_world_state_base world-state block per floor and is
 * invoked on death restart, floor teleport, and from map_unload_floor.
 *
 * map_runtime_state owns the events, current-event pointer, timers and saved
 * world block. The event pool starts 556 bytes before the saved block.
 */

DATA(0x8009db88, 0x2360)
KfMapRuntimeState map_runtime_state;

/* map_event_pool_update current-floor dispatch jump table (cases 1..5). */
RODATA(0x80012be4, 0x14)

/* Reset the dialogue tick gate and ambient-script countdown at floor start. */
ADDRESS(0x800356e8, 0x20)
void map_event_timers_reset(void)
{
    map_dialogue_advance_gate = KF_DIALOGUE_GATE_RELOAD;
    map_ambient_script_countdown = MAP_AMBIENT_COUNTDOWN_RELOAD;
}

ADDRESS(0x80035708, 0x1d8)
void map_event_update_wander(void)
{
    KfMapEvent *event = current_map_event;
    struct KfVecXZs forward;
    VECTOR point;
    s16 heading;

    collision_adjust_cell_occupancy(event->cell_x, event->cell_z, -1);

    heading = angle_approach(event->rotation, event->rotation_target, MAP_EVENT_WANDER_TURN_STEP);
    event->rotation = heading;
    angle_to_forward_xz(heading, &forward);
    vector2s_scale_shift11(MAP_EVENT_WANDER_VECTOR_SCALE, &forward);

    point.vx = forward.x + event->reference_x;
    point.vz = forward.z + event->reference_z;

    if (collision_query_world(
            point.vx, KF_COLLISION_IGNORE_HEIGHT, point.vz, event->radius, 0,
            KF_COLLISION_SKIP_MAP_EVENTS | (0x80 << KF_COLLISION_CELL_FLAG_SHIFT))
            == (u32)KF_COLLISION_NONE) {
        event->reference_x = point.vx;
        event->reference_z = point.vz;
        event->cell_x = point.vx / KF_MAP_TILE_SIZE;
        event->cell_z = point.vz / KF_MAP_TILE_SIZE;
        event->collision_turn_pending = 0;
        if (event->rotation == event->rotation_target && rand() < MAP_EVENT_WANDER_TURN_RANDOM_LIMIT) {
            event->rotation_target = rand() >> MAP_EVENT_RANDOM_YAW_SHIFT;
        }
    } else {
        if (event->collision_turn_pending == 0 || event->rotation == event->rotation_target) {
            event->rotation_target = rand() >> MAP_EVENT_RANDOM_YAW_SHIFT;
            event->collision_turn_pending = 1;
        }
    }

    event->animation_phase =
        (event->animation_phase + KF_MAP_EVENT_ANIMATION_WANDER_STEP)
        & KF_MAP_EVENT_ANIMATION_PHASE_MASK;
    collision_adjust_cell_occupancy(event->cell_x, event->cell_z, 1);
}

ADDRESS(0x800358e0, 0x8c)
void map_event_update_animation_loop(void)
{
    KfMapEvent *event = current_map_event;

    event->animation_phase =
        (event->animation_phase + KF_MAP_EVENT_ANIMATION_LOOP_STEP)
        & KF_MAP_EVENT_ANIMATION_PHASE_MASK;

    if (player_state.progress_state.current_floor == KF_FLOOR_5
            && event == &map_event_pool[0]
            && map_event_pool[0].animation_phase < KF_MAP_EVENT_ANIMATION_LOOP_STEP) {
        audio_play_spatial_range(&gameplay_sound_ref_10,
            (const VECTOR *)&map_event_pool[0].reference_x,
            KF_AUDIO_MAX_VOLUME, MAP_EVENT_LOOP_SOUND_MAX_DISTANCE, MAP_EVENT_LOOP_SOUND_ATTENUATION_DISTANCE);
    }
}

ADDRESS(0x8003596c, 0x1f0)
void map_event_pool_update(void)
{
    KfMapEvent *event = map_event_pool;
    u16 index = KF_MAP_EVENT_CAPACITY - 1;

    do {
        KF_ENUM_PROMOTED(KfMapEventState) state = event->state;

        if (state == KF_MAP_EVENT_ACTIVE) {
            map_event_set_current(event);

            if (event->behavior == KF_MAP_EVENT_BEHAVIOR_WANDER) {
                goto call_wander;
            }
            if (event->behavior == KF_MAP_EVENT_BEHAVIOR_ANIMATION_LOOP) {
                goto call_animation_loop;
            }
            goto advance_dialogue;
        call_wander:
            map_event_update_wander();
            goto advance_dialogue;
        call_animation_loop:
            map_event_update_animation_loop();
        advance_dialogue:
            if (map_dialogue_advance_gate == 0 && event->dialogue_page_delay != 0) {
                event->dialogue_page_delay--;
                if (event->dialogue_page_delay == 0) {
                    s32 limit = event->dialogue_pages.last_page[event->dialogue_stage - 1];
                    event->dialogue_page++;
                    if (event->dialogue_page >= limit) {
                        event->dialogue_page = limit;
                    }
                }
            }
        }

        event++;
    } while (index-- != 0);

    {
        u16 *gate = &map_dialogue_advance_gate;
        u16 current = *gate;

        *gate = current - 1;
        if (current == 0) {
            *gate = KF_DIALOGUE_GATE_RELOAD;
        }
    }

    if (map_ambient_script_countdown-- == 0) {
        map_ambient_script_countdown = MAP_AMBIENT_COUNTDOWN_RELOAD;
        switch (player_state.progress_state.current_floor) {
        case KF_FLOOR_1:
            map_ambient_script_floor1();
            break;
        case KF_FLOOR_2:
            map_ambient_script_floor2();
            break;
        case KF_FLOOR_3:
            map_ambient_script_floor3();
            break;
        case KF_FLOOR_4:
            map_ambient_script_floor4();
            break;
        case KF_FLOOR_5:
            map_ambient_script_floor5();
            break;
        }
    }
}

ADDRESS(0x80035b5c, 0x2b8)
void map_world_state_persist(void)
{
    u8 *base = MAP_WORLD_STATE_BYTES;
    u8 *out;
    u8 *count_slot;
    KfMapEvent *event;
    KfActor *actor;
    KfMapObject *object;
    KfMapObjectDefinition *definitions;
    s32 i;
    s32 active;

    out = base - (KF_MAP_SAVED_FLOOR_BYTES - KF_MAP_SAVED_RECORDS_OFFSET)
        + KF_MAP_SAVED_FLOOR_BYTES * KF_ENUM_ENCODE(u8, player_state.progress_state.current_floor);
    *out++ = 1;

    event = map_runtime_state.events;
    for (i = 0; i < KF_MAP_EVENT_CAPACITY; i++, event++) {
        *out++ = KF_ENUM_ENCODE(u8, event->state);
        *out++ = event->dialogue_stage_limit;
        *out++ = event->dialogue_stage;
        *out++ = event->dialogue_page;
        *out++ = event->dialogue_pages.last_page[event->dialogue_stage - 1];
        *out++ = event->dialogue_page_delay;
        *out++ = event->unknown_0d;
    }

    count_slot = out++;
    active = 0;
    actor = &actor_state.actors[0];
    for (i = 0; i < KF_ACTOR_CAPACITY; i++, actor++) {
        if (actor->slot_state == KF_ACTOR_SLOT_PERSISTENT || actor->slot_state == KF_ACTOR_SLOT_HOMEBOUND) {
            active++;
            *out++ = i;
            if (actor->lifecycle == KF_ACTOR_LIFECYCLE_DISABLED) {
                *out++ = KF_ENUM_ENCODE(u8, KF_ACTOR_LIFECYCLE_DISABLED);
            } else {
                *out++ = KF_ENUM_ENCODE(u8, KF_ACTOR_LIFECYCLE_DORMANT);
            }
        }
    }
    *count_slot = active;

    object = &map_object_state.objects[0];
    for (i = 0; i < KF_MAP_OBJECT_CAPACITY; i++, object++) {
        *out++ = object->object_id;
    }

    count_slot = out++;
    active = 0;
    object = &map_object_state.objects[0];
    definitions = map_object_state.definitions;
    for (i = 0; i < KF_MAP_OBJECT_EFFECT_FIRST; i++, object++) {
        u8 id = object->object_id;
        u8 behavior;

        if (id == KF_MAP_OBJECT_FREE) {
            continue;
        }

        behavior = definitions[id].behavior_type;
        if ((behavior == KF_MAP_OBJECT_BEHAVIOR_NONE
                || behavior == KF_MAP_OBJECT_BEHAVIOR_SCREEN_IMAGE
                || behavior == KF_MAP_OBJECT_BEHAVIOR_ITEM_PICKUP
                || behavior == KF_MAP_OBJECT_BEHAVIOR_SAVE_POINT
                || behavior == KF_MAP_OBJECT_BEHAVIOR_GOLD_PICKUP)
                && object->action == KF_MAP_OBJECT_ACTION_IDLE) {
            continue;
        }

        active++;
        *out++ = i;
        {
            u8 *link = (u8 *)&object->link;
            s32 k = sizeof(object->link) - 1;

            do {
                *out++ = *link++;
            } while (--k != -1);
        }
    }
    *count_slot = active;

    object = &map_object_state.objects[KF_MAP_OBJECT_GOLD_DROP_FIRST];
    for (i = 0; i < KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY; i++, object++) {
        *out++ = (u8)object->cell_x;
        *out++ = (u8)object->cell_z;
        *out++ = object->link.link_id;
        *out++ = (u8)(*(const u16 *)&object->link >> 8);
    }

    object = &map_object_state.objects[KF_MAP_OBJECT_DEFINITION_DROP_FIRST];
    for (i = 0; i < 2 * KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY; i++, object++) {
        *out++ = (u8)object->cell_x;
        *out++ = (u8)object->cell_z;
        *out++ = (u8)((u16)object->rotation.y >> KF_MAP_SAVED_YAW_SHIFT);
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
