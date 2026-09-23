#include <kf/game/actor.h>
#include <kf/lib/map_data.h>
#include <kf/lib/map.h>
#include <kf/game/collision.h>
#include <psyq/libc.h>
#include <kf/game/game.h>

enum {
    MAP_AMBIENT_COUNTDOWN_RELOAD = 10,
    MAP_EVENT_WANDER_TURN_STEP = 70,
    MAP_EVENT_WANDER_VECTOR_SCALE = 20,
    MAP_EVENT_WANDER_TURN_RANDOM_LIMIT = 1584,
    MAP_EVENT_LOOP_SOUND_MAX_DISTANCE = 18000,
    MAP_EVENT_LOOP_SOUND_ATTENUATION_DISTANCE = 50000
};

KfMapRuntimeState map_runtime_state;

void map_event_timers_reset(void)
{
    map_runtime_state.dialogue_advance_gate = KF_DIALOGUE_GATE_RELOAD;
    map_runtime_state.ambient_script_countdown = MAP_AMBIENT_COUNTDOWN_RELOAD;
}

void map_event_update_wander(void)
{
    KfMapEvent *event = map_runtime_state.current_event;
    struct KfVecXZs forward;
    VECTOR point;
    s16 heading;

    collision_adjust_cell_occupancy(event->cell_x, event->cell_z, -1);

    heading = angle_approach(event->rotation.vy, event->rotation_target, MAP_EVENT_WANDER_TURN_STEP);
    event->rotation.vy = heading;
    angle_to_forward_xz(heading, &forward);
    vector2s_scale_shift11(MAP_EVENT_WANDER_VECTOR_SCALE, &forward);

    point.vx = forward.x + event->reference_position.vx;
    point.vz = forward.z + event->reference_position.vz;

    if (collision_query_world(
            point.vx, KF_COLLISION_IGNORE_HEIGHT, point.vz, event->radius, 0,
            KF_COLLISION_SKIP_MAP_EVENTS
                | (KF_COLLISION_CELL_BLOCKS_WANDER << KF_COLLISION_CELL_FLAG_SHIFT))
            == KF_COLLISION_NONE) {
        event->reference_position.vx = point.vx;
        event->reference_position.vz = point.vz;
        event->cell_x = point.vx / KF_MAP_TILE_SIZE;
        event->cell_z = point.vz / KF_MAP_TILE_SIZE;
        event->collision_turn_pending = KF_MAP_EVENT_COLLISION_TURN_NONE;
        if (event->rotation.vy == event->rotation_target && rand() < MAP_EVENT_WANDER_TURN_RANDOM_LIMIT) {
            event->rotation_target = rand() >> KF_RANDOM_ANGLE_SHIFT;
        }
    } else {
        if (event->collision_turn_pending == KF_MAP_EVENT_COLLISION_TURN_NONE
            || event->rotation.vy == event->rotation_target) {
            event->rotation_target = rand() >> KF_RANDOM_ANGLE_SHIFT;
            event->collision_turn_pending = KF_MAP_EVENT_COLLISION_TURN_PENDING;
        }
    }

    event->animation_phase =
        (event->animation_phase + KF_MAP_EVENT_ANIMATION_WANDER_STEP)
        & KF_MAP_EVENT_ANIMATION_PHASE_MASK;
    collision_adjust_cell_occupancy(event->cell_x, event->cell_z, 1);
}

void map_event_update_animation_loop(void)
{
    KfMapEvent *event = map_runtime_state.current_event;

    event->animation_phase =
        (event->animation_phase + KF_MAP_EVENT_ANIMATION_LOOP_STEP)
        & KF_MAP_EVENT_ANIMATION_PHASE_MASK;

    if (player_state.progress_state.current_floor == KF_FLOOR_5
            && event == &map_runtime_state.events[0]
            && map_runtime_state.events[0].animation_phase < KF_MAP_EVENT_ANIMATION_LOOP_STEP) {
        audio_play_spatial_range(&gameplay_sound_refs[KF_GAMEPLAY_SOUND_FLOOR5_EVENT_LOOP],
            &map_runtime_state.events[0].reference_position,
            KF_AUDIO_MAX_VOLUME, MAP_EVENT_LOOP_SOUND_MAX_DISTANCE, MAP_EVENT_LOOP_SOUND_ATTENUATION_DISTANCE);
    }
}

void map_event_pool_update(void)
{
    KfMapEvent *event = map_runtime_state.events;
    u16 index = KF_MAP_EVENT_CAPACITY - 1;

    do {
        KfMapEventState state = event->state;

        if (state == KF_MAP_EVENT_ACTIVE) {
            map_event_set_current(event);

            switch (event->behavior) {
            case KF_MAP_EVENT_BEHAVIOR_WANDER:
                map_event_update_wander();
                break;
            case KF_MAP_EVENT_BEHAVIOR_ANIMATION_LOOP:
                map_event_update_animation_loop();
                break;
            }
            if (map_runtime_state.dialogue_advance_gate == 0 && event->dialogue.fields.page_delay != 0) {
                event->dialogue.fields.page_delay--;
                if (event->dialogue.fields.page_delay == 0) {
                    s32 limit = event->dialogue_pages.last_page[event->dialogue.fields.stage - 1];
                    event->dialogue.fields.page++;
                    if (event->dialogue.fields.page >= limit) {
                        event->dialogue.fields.page = limit;
                    }
                }
            }
        }

        event++;
    } while (index-- != 0);

    {
        u16 *gate = &map_runtime_state.dialogue_advance_gate;
        u16 current = *gate;

        *gate = current - 1;
        if (current == 0) {
            *gate = KF_DIALOGUE_GATE_RELOAD;
        }
    }

    if (map_runtime_state.ambient_script_countdown-- == 0) {
        map_runtime_state.ambient_script_countdown = MAP_AMBIENT_COUNTDOWN_RELOAD;
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

void map_world_state_persist(void)
{
    u8 *out;
    u8 *count_slot;
    KfMapEvent *event;
    KfActor *actor;
    KfMapObject *object;
    KfMapObjectDefinition *definitions;
    s32 i;
    s32 active;

    out = map_runtime_state.world_state.floors[
        kf_enum_encode<u8>(player_state.progress_state.current_floor) - 1].records;
    *out++ = 1;

    event = map_runtime_state.events;
    for (i = 0; i < KF_MAP_EVENT_CAPACITY; i++, event++) {
        *out++ = kf_enum_encode<u8>(event->state);
        *out++ = event->dialogue.fields.stage_limit;
        *out++ = event->dialogue.fields.stage;
        *out++ = event->dialogue.fields.page;
        *out++ = event->dialogue_pages.last_page[event->dialogue.fields.stage - 1];
        *out++ = event->dialogue.fields.page_delay;
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
                *out++ = kf_enum_encode<u8>(KF_ACTOR_LIFECYCLE_DISABLED);
            } else {
                *out++ = kf_enum_encode<u8>(KF_ACTOR_LIFECYCLE_DORMANT);
            }
        }
    }
    *count_slot = active;

    object = &map_object_state.objects[0];
    for (i = 0; i < KF_MAP_OBJECT_CAPACITY; i++, object++) {
        *out++ = kf_enum_encode<u8>(object->object_id);
    }

    count_slot = out++;
    active = 0;
    object = &map_object_state.objects[0];
    definitions = map_object_state.definitions.entries;
    for (i = 0; i < KF_MAP_OBJECT_EFFECT_FIRST; i++, object++) {
        KfObjectId id = object->object_id;
        KfMapObjectOperation behavior;

        if (id == KF_OBJECT_NONE) {
            continue;
        }

        behavior = definitions[kf_enum_encode<u8>(id)].behavior_type;
        if ((behavior == KF_MAP_OBJECT_OP_NONE
                || behavior == KF_MAP_OBJECT_OP_SCREEN_IMAGE
                || behavior == KF_MAP_OBJECT_OP_ITEM_PICKUP
                || behavior == KF_MAP_OBJECT_OP_SAVE_POINT
                || behavior == KF_MAP_OBJECT_OP_GOLD_PICKUP)
                && object->action == KF_MAP_OBJECT_OP_NONE) {
            continue;
        }

        active++;
        *out++ = i;
        {
            const u8 *link = (const u8 *)&object->link;
            s32 k = sizeof(object->link) - 1;

            for (; k != -1; k--) {
                *out++ = *link++;
            }
        }
    }
    *count_slot = active;

    object = &map_object_state.objects[KF_MAP_OBJECT_GOLD_DROP_FIRST];
    for (i = 0; i < KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY; i++, object++) {
        *out++ = object->cell_x;
        *out++ = object->cell_z;
        *out++ = object->link.fields.link_id;
        *out++ = object->link.gold_amount >> 8;
    }

    object = &map_object_state.objects[KF_MAP_OBJECT_DEFINITION_DROP_FIRST];
    for (i = 0; i < 2 * KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY; i++, object++) {
        *out++ = object->cell_x;
        *out++ = object->cell_z;
        *out++ = (u16)object->rotation.angles.y >> KF_MAP_SAVED_YAW_SHIFT;
    }
}

void map_unload_floor(void)
{
    pool_release_all();
    audio_close_vab();
    map_world_state_persist();
}
