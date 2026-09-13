#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_map.h>
#include <psyq/libc.h>
#include <kf/game.h>

enum {
    MAP_RESTORE_POSITION_RANDOM_BITS = 15
};

/*
 * Map-load band 0x80035e44..0x800365f8 (GAME.EXE).
 *
 * map_restore_floor_state is the per-floor world-state RESTORE routine: the exact inverse
 * of map_world_state_persist (map_events.c), which serialises the live event, actor, and
 * map-object state into the map_world_state_base world-state block. It reads the same
 * 1700-byte per-floor record (base - 1690 + 1700 * current_floor), and when its
 * marker byte is 1 it rebuilds the eight map events, the live-actor lifecycle
 * overrides, the 190 map-object ids, the linked-object payloads, and the two
 * effect-object pools (objects[160..169] and objects[170..189]). The common
 * tail dispatches a per-floor scripted setup on the current floor (1..5).
 *
 * map_runtime_state owns both the event pool and per-floor saved records;
 * the pool starts 556 bytes before the saved block.
 *
 * map_refresh_dialogue_stages refreshes the stage of every active map event.
 * map_load_floor loads the current floor:
 * map_resources_load, the world-state restore, the event refresh, effect5_texture_cache_prepare,
 * then copies colour_matrix_table[3] into the render lighting matrix.
 */

/* map_restore_floor_state per-floor scripted-setup jump table (floors 1..5). */
RODATA(0x80012bfc, 0x14)

ADDRESS(0x80035e44, 0x69c)
void map_restore_floor_state(void)
{
    u8 *base = (u8 *)&map_runtime_state.world_state;
    u8 *in;
    KfMapEvent *event;
    KfMapObject *object;
    s32 i;
    s32 index;

    {
        s32 floor_offset = KF_MAP_SAVED_FLOOR_BYTES
            * KF_ENUM_ENCODE(u8, player_state.progress_state.current_floor);
        u8 *records_base =
            base - (KF_MAP_SAVED_FLOOR_BYTES - KF_MAP_SAVED_RECORDS_OFFSET);

        in = records_base + floor_offset;
    }
    if (*in++ == 1) {
        event = map_runtime_state.events;
        for (i = 0; i < KF_MAP_EVENT_CAPACITY; i++, event++) {
            event->state = KF_ENUM_DECODE(KfMapEventState, *in++);
            event->dialogue.fields.stage_limit = *in++;
            event->dialogue.fields.stage = *in++;
            event->dialogue.fields.page = *in++;
            event->dialogue_pages.last_page[event->dialogue.fields.stage - 1] = *in++;
            event->dialogue.fields.page_delay = *in++;
            event->unknown_0d = *in++;
        }

        i = *in++;
        if (--i != -1) {
            KfActor *actors = actor_state.actors;

            do {
                index = *in++;
                actors[index].lifecycle = KF_ENUM_DECODE(KfActorLifecycle, *in++);
            } while (--i != -1);
        }

        object = &map_object_state.objects[0];
        for (i = 0; i < KF_MAP_OBJECT_CAPACITY; i++, object++) {
            object->object_id = KF_ENUM_DECODE(KfObjectId, *in++);
        }

        i = *in++;
        while (--i != -1) {
            u8 *link;
            s32 k;

            index = *in++;
            object = &map_object_state.objects[index];
            link = (u8 *)&object->link;
            k = sizeof(object->link) - 1;
            do {
                *link++ = *in++;
            } while (--k != -1);
        }

        object = &map_object_state.objects[KF_MAP_OBJECT_GOLD_DROP_FIRST];
        for (i = 0; i < KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY; i++, object++) {
            object->cell_x = *in++;
            object->cell_z = *in++;
            object->position.vx =
                object->cell_x * KF_MAP_TILE_SIZE + ((rand() * KF_MAP_TILE_SIZE) >> MAP_RESTORE_POSITION_RANDOM_BITS);
            object->position.vz =
                object->cell_z * KF_MAP_TILE_SIZE + ((rand() * KF_MAP_TILE_SIZE) >> MAP_RESTORE_POSITION_RANDOM_BITS);
            object->position.vy =
                -(map_floor_height_grid.cells[object->cell_z][object->cell_x] * KF_MAP_HEIGHT_STEP);
            object->rotation.angles.z = 0;
            object->rotation.angles.y = 0;
            object->rotation.angles.x = 0;
            object->link.gold_amount = *in++;
            object->link.gold_amount |= *in++ << 8;
            object->link.fields.spawn.sequence = 0;
            object->link.fields.vertical_velocity = 0;
        }

        object = &map_object_state.objects[KF_MAP_OBJECT_DEFINITION_DROP_FIRST];
        for (i = 0; i < 2 * KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY; i++, object++) {
            object->cell_x = *in++;
            object->cell_z = *in++;
            object->position.vx =
                object->cell_x * KF_MAP_TILE_SIZE + ((rand() * KF_MAP_TILE_SIZE) >> MAP_RESTORE_POSITION_RANDOM_BITS);
            object->position.vz =
                object->cell_z * KF_MAP_TILE_SIZE + ((rand() * KF_MAP_TILE_SIZE) >> MAP_RESTORE_POSITION_RANDOM_BITS);
            object->position.vy =
                -(map_floor_height_grid.cells[object->cell_z][object->cell_x] * KF_MAP_HEIGHT_STEP);
            if (object->object_id < KF_MAP_DROP_TIP_ID_END) {
                object->rotation.angles.x = KF_ANGLE_QUARTER_TURN;
            } else if (object->object_id < KF_MAP_DROP_SPIN_ID_END) {
                object->rotation.angles.x = 0;
            }
            object->rotation.angles.z = 0;
            object->rotation.angles.y = *in++ << KF_MAP_SAVED_YAW_SHIFT;
            object->link.gold_amount = 0;
            object->link.fields.spawn.sequence = 0;
            object->link.fields.vertical_velocity = 0;
        }
    }

    switch (player_state.progress_state.current_floor) {
    case KF_FLOOR_1:
        if (map_floor1_script.passage_opened == KF_MAP_SCRIPT_SET) {
            map_apply_copy_region(KF_MAP_COPY_FLOOR1_PASSAGE);
        }
        if (map_floor1_script.actor_activation_stage != KF_MAP_TRIGGER_COMPLETE) {
            index = actor_pool_find_at_tile(7, 0x28);
            if (index != -1) {
                actor_state.actors[index].lifecycle = KF_ACTOR_LIFECYCLE_DISABLED;
            }
        }
        if (map_floor5_script.weapon_transformed == KF_MAP_SCRIPT_SET) {
            map_object_pool_clear_link(KF_MAP_LINK_WEAPON_TRANSFORM_DOORS);
        }
        break;
    case KF_FLOOR_2:
        if (map_floor5_script.weapon_transformed == KF_MAP_SCRIPT_SET) {
            map_object_pool_clear_link(KF_MAP_LINK_WEAPON_TRANSFORM_DOORS);
        }
        if (player_state.progress_state.highest_floor >= KF_FLOOR_3) {
            map_event_pool[0].state = KF_MAP_EVENT_DISABLED;
        }
        break;
    case KF_FLOOR_3:
        if (map_floor5_script.weapon_transformed == KF_MAP_SCRIPT_SET) {
            map_object_pool_clear_link(KF_MAP_LINK_WEAPON_TRANSFORM_DOORS);
        }
        if (map_floor3_script.revealed_piece_count == KF_MAP_FLOOR3_REQUIRED_REVEALS) {
            map_apply_copy_region(KF_MAP_COPY_FLOOR3_REVEAL_FIRST);
            map_apply_copy_region(KF_MAP_COPY_FLOOR3_REVEAL_SECOND);
        }
        break;
    case KF_FLOOR_4:
        break;
    case KF_FLOOR_5:
        if (map_floor5_script.character_arrived == KF_MAP_SCRIPT_SET) {
            map_event_pool[1].state = KF_MAP_EVENT_ACTIVE;
        }
        if (map_floor5_script.boss_encounter_started == KF_MAP_SCRIPT_UNSET) {
            actor_state.definitions.entries[7].action_animations[KF_ACTOR_ANIM_SLOT_MELEE] = KF_ANIMATION_CLIP_NONE;
            actor_state.definitions.entries[7].action_animations[KF_ACTOR_ANIM_SLOT_EFFECT0] = KF_ANIMATION_CLIP_NONE;
            actor_state.definitions.entries[7].action_animations[KF_ACTOR_ANIM_SLOT_EFFECT1] = KF_ANIMATION_CLIP_NONE;
            actor_state.definitions.entries[7].action_animations[KF_ACTOR_ANIM_SLOT_EFFECT2] = KF_ANIMATION_CLIP_NONE;
            actor_state.definitions.entries[7].action_animations[KF_ACTOR_ANIM_SLOT_MULTI_HIT_ATTACK] = KF_ANIMATION_CLIP_NONE;
        } else {
            map_apply_copy_region(KF_MAP_COPY_FLOOR5_BOSS_ENCOUNTER);
        }
        if (item_stock[KF_ENUM_ENCODE(u8, KF_ITEM_STOCK_PLAYER)][KF_ENUM_ENCODE(u8, KF_ITEM_DRAGON_SWORD)] != 0 || item_stock[KF_ENUM_ENCODE(u8, KF_ITEM_STOCK_PLAYER)][KF_ENUM_ENCODE(u8, KF_ITEM_MOONLIGHT_SWORD)] != 0
                || map_floor5_script.weapon_transformed == KF_MAP_SCRIPT_SET) {
            map_object_pool_clear_link(KF_MAP_LINK_FLOOR5_SWORD_DOOR);
        }
        if (boss_defeat_complete != KF_MAP_SCRIPT_UNSET) {
            map_object_pool_trigger_link(KF_MAP_LINK_BOSS_EMITTERS);
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
void map_refresh_dialogue_stages(void)
{
    KfMapEvent *event = map_event_pool;
    u16 index = KF_MAP_EVENT_CAPACITY - 1;

    do {
        if (event->state == KF_MAP_EVENT_ACTIVE) {
            map_event_refresh_dialogue_stage(event);
        }
        event++;
    } while (index-- != 0);
}

ADDRESS(0x80036554, 0xa4)
void map_load_floor(void)
{
    map_resources_load(player_state.progress_state.current_floor,
                       player_state.map_variant);
    map_restore_floor_state();
    map_refresh_dialogue_stages();
    effect5_texture_cache_prepare(player_state.progress_state.current_floor);
    game_graphics_runtime.render_state.effect_color_matrix = color_matrix_table[KF_ENUM_ENCODE(s32, KF_GAME_COLOR_WHITE)];
}

ADDRESS(0x800365f8, 0x20)
void map_load_floor_wrapper(void)
{
    /* The wrapper's distinct purpose is unresolved; it adds no visible work. */
    map_load_floor();
}
