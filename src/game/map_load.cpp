#include <kf/game/graphics.h>

#include <kf/lib/map_data.h>
#include <kf/lib/map.h>
#include <psyq/libc.h>
#include <kf/game/game.h>

enum {
    MAP_RESTORE_POSITION_RANDOM_BITS = 15
};

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
            * kf_enum_encode<u8>(player_state.progress_state.current_floor);
        u8 *records_base =
            base - (KF_MAP_SAVED_FLOOR_BYTES - KF_MAP_SAVED_RECORDS_OFFSET);

        in = records_base + floor_offset;
    }
    if (*in++ == 1) {
        event = map_runtime_state.events;
        for (i = 0; i < KF_MAP_EVENT_CAPACITY; i++, event++) {
            event->state = kf_enum_decode<KfMapEventState>(*in++);
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
                actors[index].lifecycle = kf_enum_decode<KfActorLifecycle>(*in++);
            } while (--i != -1);
        }

        object = &map_object_state.objects[0];
        for (i = 0; i < KF_MAP_OBJECT_CAPACITY; i++, object++) {
            object->object_id = kf_enum_decode<KfObjectId>(*in++);
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
        if (item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(KF_ITEM_DRAGON_SWORD)] != 0 || item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(KF_ITEM_MOONLIGHT_SWORD)] != 0
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

void map_load_floor(void)
{
    map_resources_load(player_state.progress_state.current_floor,
                       player_state.map_variant);
    map_restore_floor_state();
    map_refresh_dialogue_stages();
    effect5_texture_cache_prepare(player_state.progress_state.current_floor);
    game_graphics_runtime.render_state.effect_color_matrix = color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_WHITE)];
}

void map_load_floor_wrapper(void)
{

    map_load_floor();
}
