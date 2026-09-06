#include <kf/address.h>
#include <kf/game_actor.h>
#include <kf/map_data.h>
#include <kf/game_map.h>
#include <kf/game_collision.h>
#include <kf/notify.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

/* Floor-specific ambient/action scripts and their shared interaction dispatch.
 * The original module boundary remains WIP. */

KF_ENUM_BEGIN(KfMapWeaponTransformPhase, s32)
    MAP_WEAPON_TRANSFORM_SPIN_UP = 0,
    MAP_WEAPON_TRANSFORM_SPIN_DOWN = 1
KF_ENUM_END(KfMapWeaponTransformPhase)

enum {
    MAP_WEAPON_TRANSFORM_HOLD_UPDATES = 40,
    MAP_WEAPON_TRANSFORM_SWAP_COUNTDOWN = 20,
    MAP_INTERACTION_RADIUS_PADDING = 800,
    MAP_DOOR_FACING_TOLERANCE = 341,
    MAP_CONTAINER_ITEM_COUNT = 4,
    MAP_CONTAINER_ITEM_NONE = 255
};

/* Word at event+8: ignored stage cap, stage, page, gated delay (little endian). */
#define MAP_DIALOGUE_TRIGGER_MASK 0xffffff00
#define MAP_DIALOGUE_STARTED(stage) \
    ((KF_DIALOGUE_PAGE_DELAY_TICKS << 24) | (KF_DIALOGUE_FIRST_PAGE << 16) \
     | ((stage) << 8))

DATA(0x800561d0, 0x38)
static KfCameraPathPoint map_floor5_camera_path[2] = {
    {{173000, -11500, 85000, 0}, {0, KF_ANGLE_HALF_TURN, 0, 0}, 100, 0},
    {{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0}
};

DATA(0x80056208, 0x10)
static VECTOR map_floor1_sound_position = {65000, -10000, 25000, 0};

DATA(0x80056218, 0x20)
static MATRIX map_reveal_light_matrix = {
    {{0, -KF_FIXED12_ONE, 0}, {0, -KF_FIXED12_ONE, 0}, {0, -KF_FIXED12_ONE, 0}}, {0, 0, 0}
};

DATA(0x80056238, 0x10)
static char map_screen_image_path[16] = "KAN\\B0\\K000.TIM";

/* Two cut-in path literals, then object-behaviour and floor dispatch tables. */
RODATA(0x80012a54, 0x18c)

/* Scan the actor pool for the first live actor on map tile (tile_x, tile_z). */
ADDRESS(0x80033ee4, 0x80)
s32 actor_pool_find_at_tile(u8 tile_x, u8 tile_z)
{
    KfActor *actor = actor_state.actors;
    s16 index;

    for (index = 0; index < KF_ACTOR_CAPACITY; index++, actor++) {
        if (actor->slot_state != KF_ACTOR_SLOT_FREE && actor->tile_x == tile_x
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
    if (map_floor1_script.revival_enabled == KF_MAP_SCRIPT_SET) {
        audio_play_spatial_default_range(
            &gameplay_sound_ref_5, &map_floor1_sound_position, 0x73);
    }

    switch (map_floor1_script.actor_activation_stage) {
    case KF_MAP_TRIGGER_AWAIT_ENTRY:
        if (player_state.map_cell.x >= 7 && player_state.map_cell.z >= 31
            && player_state.map_cell.x < 12 && player_state.map_cell.z < 41) {
            map_floor1_script.actor_activation_stage = KF_MAP_TRIGGER_AWAIT_EXIT;
        }
        break;
    case KF_MAP_TRIGGER_AWAIT_EXIT:
        if (player_state.map_cell.x < 2 || player_state.map_cell.z < 25
            || player_state.map_cell.x >= 14 || player_state.map_cell.z >= 46) {
            s32 actor_index;
            s32 object_index;

            map_floor1_script.actor_activation_stage = KF_MAP_TRIGGER_COMPLETE;
            actor_index = actor_pool_find_at_tile(7, 0x28);
            if (actor_index != -1) {
                actor_state.actors[actor_index].lifecycle = KF_ACTOR_LIFECYCLE_DORMANT;
                actor_initialize_slot(actor_index);
            }
            object_index = map_object_pool_find_near_point(0x5208, 0x105b8, 0xbb8);
            if (object_index != -1) {
                map_object_state.objects[object_index].object_id = 0x5c;
            }
        }
        break;
    }

    switch (map_floor1_script.object_removal_stage) {
    case KF_MAP_TRIGGER_AWAIT_ENTRY:
        if (player_state.map_cell.x >= 2 && player_state.map_cell.z >= 27
            && player_state.map_cell.x < 5 && player_state.map_cell.z < 30) {
            map_floor1_script.object_removal_stage = KF_MAP_TRIGGER_AWAIT_EXIT;
        }
        break;
    case KF_MAP_TRIGGER_AWAIT_EXIT:
        if (player_state.map_cell.x < 2 || player_state.map_cell.z < 11
            || player_state.map_cell.x >= 28 || player_state.map_cell.z >= 41) {
            s32 object_index;

            map_floor1_script.object_removal_stage = KF_MAP_TRIGGER_COMPLETE;
            object_index = map_object_pool_find_near_point(0x2328, 0xdea8, 0xbb8);
            if (object_index != -1) {
                map_object_state.objects[object_index].object_id = KF_MAP_OBJECT_FREE;
            }
        }
        break;
    }
}

/* Floor 2 ambient sound during the first two pages of event 1's stage 2. */
ADDRESS(0x800341ec, 0x70)
void map_ambient_script_floor2(void)
{
    if (map_event_pool[1].dialogue_stage == 2 && map_event_pool[1].dialogue_page < 3
        && rand() < 4000) {
        audio_play_spatial_default_range(
            &gameplay_sound_ref_8, (const VECTOR *)&map_event_pool[1].reference_x, KF_AUDIO_MAX_VOLUME);
    }
}

/* Floor 3: restore vitals and teach two spells within the fixed map region. */
ADDRESS(0x8003425c, 0x88)
void map_ambient_script_floor3(void)
{
    if (player_state.map_cell.x >= 15 && player_state.map_cell.x < 18
        && player_state.map_cell.z == 0x40) {
        player_restore_vitals_with_color_cycle();
        if (magic_records[KF_MAGIC_RESIST_FIRE].learned == 0 || magic_records[KF_MAGIC_BLESS].learned == 0) {
            magic_records[KF_MAGIC_RESIST_FIRE].learned = 1;
            magic_records[KF_MAGIC_BLESS].learned = 1;
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
    KfMapScriptFlag *encounter_started = &map_floor5_script.boss_encounter_started;

    if (*encounter_started == KF_MAP_SCRIPT_UNSET && player_state.map_cell.x >= 38
        && player_state.map_cell.x < 41 && player_state.map_cell.z == 7
        && (u16)player_state.camera_rotation.vy >= 1808
        && (u16)player_state.camera_rotation.vy < 2289) {
        *encounter_started = KF_MAP_SCRIPT_SET;
        screen_show_image_until_input("TALK\\C17\\T55171.TIM");
        render_frame(0, 0);
        render_frame(0, 0);
        screen_show_image_until_input("TALK\\C17\\T55172.TIM");
        actor_state.definitions[7].action_animations[KF_ACTOR_ANIM_SLOT_MELEE] = 2;
        actor_state.definitions[7].action_animations[KF_ACTOR_ANIM_SLOT_EFFECT0] = 3;
        actor_state.definitions[7].action_animations[KF_ACTOR_ANIM_SLOT_EFFECT1] = 3;
        actor_state.definitions[7].action_animations[KF_ACTOR_ANIM_SLOT_EFFECT2] = 3;
        actor_state.definitions[7].action_animations[KF_ACTOR_ANIM_SLOT_MULTI_HIT_ATTACK] = 1;
        map_apply_copy_region(4);
    }
}

/* Floor-1 action script: reveal a passage once its progress flag is set. */
ADDRESS(0x800343e0, 0x58)
void map_action_script_floor1(void)
{
    if (item_stock[0][KF_ITEM_DRAGON_CHALICE] != 0
        && map_floor1_script.passage_opened == KF_MAP_SCRIPT_UNSET) {
        map_floor1_script.passage_opened = KF_MAP_SCRIPT_SET;
        map_apply_copy_region(1);
        sound_ref_play(&gameplay_sound_ref_7, 0x64);
    }
}

/* Raise and rotate event 3 into a white fade, disable it, then fade back. */
ADDRESS(0x80034438, 0x184)
void map_reveal_fade(void)
{
    MATRIX saved;
    s32 blend;

    saved = render_state.light_matrix_copy;

    for (blend = 0; blend < KF_FIXED12_ONE + 1; blend += 128) {
        lighting_set_color_matrix(&color_matrix_table[KF_GAME_COLOR_DEFAULT], &color_matrix_table[KF_GAME_COLOR_WHITE], blend);
        if (blend >= KF_FIXED12_ONE / 4 + 1) {
            map_event_pool[3].position_y -= 130;
            map_event_pool[3].rotation += 128;
        } else {
            matrix_interpolate(&saved, &map_reveal_light_matrix,
                               &render_state.light_matrix_copy, blend << 2);
        }
        render_frame(0, 0);
        frame_pacer_wait();
    }

    map_event_pool[3].state = KF_MAP_EVENT_DISABLED;
    map_floor5_script.character_arrived = KF_MAP_SCRIPT_SET;

    for (blend = KF_FIXED12_ONE; blend >= 0; blend -= 256) {
        lighting_set_color_matrix(&color_matrix_table[KF_GAME_COLOR_DEFAULT], &color_matrix_table[KF_GAME_COLOR_WHITE], blend);
        render_frame(0, 0);
        frame_pacer_wait();
    }

    lighting_set_active_color_matrix(KF_GAME_COLOR_DEFAULT);
    render_state.light_matrix_copy = saved;
}

/* Floor-2 action script: fade after event 3's first stage-2 dialogue page. */
ADDRESS(0x800345bc, 0x54)
void map_action_script_floor2(void)
{
    if ((*(u32 *)&map_event_pool[3].dialogue_stage_limit & MAP_DIALOGUE_TRIGGER_MASK)
            == MAP_DIALOGUE_STARTED(2)
        && map_event_pool[3].state == KF_MAP_EVENT_ACTIVE) {
        map_reveal_fade();
    }
}

/* Floor-3 action script: teach two spells gated on flags and event state. */
ADDRESS(0x80034610, 0x90)
void map_action_script_floor3(void)
{
    if (item_stock[0][KF_ITEM_WIND_BLADE_BRACELET] != 0) {
        if (magic_records[KF_ENUM_ENCODE(u8, KF_MAGIC_WIND_CUTTER)].learned == 0) {
            magic_records[KF_ENUM_ENCODE(u8, KF_MAGIC_WIND_CUTTER)].learned = 1;
            notify_enqueue(1);
        }
    }
    if ((*(u32 *)&map_event_pool[1].dialogue_stage_limit & MAP_DIALOGUE_TRIGGER_MASK)
            == MAP_DIALOGUE_STARTED(3)) {
        if (magic_records[KF_ENUM_ENCODE(u8, KF_MAGIC_FIRE_BALL)].learned == 0) {
            magic_records[KF_ENUM_ENCODE(u8, KF_MAGIC_FIRE_BALL)].learned = 1;
            notify_enqueue(1);
        }
    }
}

/* Floor 4 action script: empty stub. */
ADDRESS(0x800346a0, 0x8)
void map_action_script_floor4(void)
{
}

ADDRESS(0x800346a8, 0x38c)
void map_floor5_transition_cutscene(void)
{
    MATRIX color_matrix;
    KfCameraPathState path;
    KfMapObject *effect;
    SVECTOR direction;
    VECTOR spawn;
    s32 grid_height;
    s32 spin;
    s32 hold;
    KfMapWeaponTransformPhase phase;

    if (player_state.equipped_weapon_id == KF_ITEM_DRAGON_SWORD) {
        player_equip_weapon(KF_ITEM_NONE);
    }
    item_stock[0][KF_ITEM_DRAGON_SWORD] = 0;
    collision_adjust_cell_occupancy(player_state.map_cell.x, player_state.map_cell.z, -1);

    camera_path_begin(&path, map_floor5_camera_path);
    for (;;) {
        camera_path_step(&path, 0);
        if (path.frames_remaining == KF_CAMERA_PATH_FINISHED) {
            break;
        }
        render_frame(&path.position, &path.rotation);
    }

    player_state.camera_position = path.position;
    player_state.camera_rotation = path.rotation;
    player_state.map_cell.x = player_state.camera_position.vx / KF_MAP_TILE_SIZE;
    player_state.map_cell.z = player_state.camera_position.vz / KF_MAP_TILE_SIZE;
    collision_adjust_cell_occupancy(player_state.map_cell.x, player_state.map_cell.z, 1);

    ReadColorMatrix(&color_matrix);
    effect = map_object_effect_pool_acquire(
        KF_MAP_OBJECT_PLACEMENT_DROP_FIRST, KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY, map_object_effect_sequence_180);
    effect->object_id = KF_ITEM_DRAGON_SWORD;
    effect->cell_x = 85;
    effect->cell_z = 40;
    effect->position_x = effect->cell_x * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    effect->position_z = effect->cell_z * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    grid_height = map_floor_height_grid[effect->cell_z][effect->cell_x];
    effect->rotation.z = 0;
    effect->rotation.x = 0;
    effect->rotation.y = KF_ANGLE_HALF_TURN;
    effect->action = KF_MAP_OBJECT_ACTION_IDLE;
    effect->position_y = -(grid_height * KF_MAP_HEIGHT_STEP) - 1300;

    spin = 0;
    hold = 0;
    phase = MAP_WEAPON_TRANSFORM_SPIN_UP;
    for (;;) {
        switch (phase) {
        case MAP_WEAPON_TRANSFORM_SPIN_UP:
            effect->rotation.y += spin;
            if (hold != 0) {
                hold -= 1;
                if (hold == 1) {
                    phase = MAP_WEAPON_TRANSFORM_SPIN_DOWN;
                } else if (hold == MAP_WEAPON_TRANSFORM_SWAP_COUNTDOWN) {
                    spawn = *(VECTOR *)&effect->position_x;
                    spawn.vy -= 600;
                    effect_pool_construct(
                        0, KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,
                        KF_EFFECT_KIND_RADIAL_BLAST, &spawn, &direction, 1);
                    effect->object_id = KF_ITEM_MOONLIGHT_SWORD;
                }
            } else if (spin < 240) {
                spin += 1;
            } else {
                hold = MAP_WEAPON_TRANSFORM_HOLD_UPDATES;
            }
            break;
        case MAP_WEAPON_TRANSFORM_SPIN_DOWN:
            effect->rotation.y += spin;
            if (spin > 0) {
                spin -= 1;
            } else {
                map_object_start_action_if_idle(effect, KF_MAP_OBJECT_ACTION_FALL_AND_TIP);
                effect->link.vertical_velocity = 0;
                goto done;
            }
            break;
        default:
            break;
        }
        effect_pool_sweep();
        render_frame(0, 0);
    }
done:
    ;
}

ADDRESS(0x80034a34, 0x4c)
void map_action_script_floor5(void)
{
    if ((*(u32 *)&map_event_pool[1].dialogue_stage_limit & MAP_DIALOGUE_TRIGGER_MASK)
            == MAP_DIALOGUE_STARTED(5)) {
        map_floor5_transition_cutscene();
        map_floor5_script.weapon_transformed = KF_MAP_SCRIPT_SET;
    }
}

ADDRESS(0x80034a80, 0x2d4)
void map_event_interact(KfMapEvent *event)
{
    switch (event->character_id) {
    case 3:
        if (item_stock[0][KF_ITEM_GOLD_CROSS] != 0 && map_event_pool[2].dialogue_stage == 1
            && map_event_pool[2].dialogue_page < 3) {
            item_stock[0][KF_ITEM_KEY_OF_THE_DEAD] = 1;
            map_event_pool[2].dialogue_pages.last_page[0] = 7;
            item_stock[0][KF_ITEM_GOLD_CROSS]--;
            talk_show_dialogue_page(player_state.progress_state.current_floor,
                                    event->dialogue_stage, event->character_id, 3);
            map_event_pool[2].dialogue_page = 4;
            map_event_pool[2].dialogue_page_delay = 0;
            map_event_pool[2].dialogue_stage_limit = 5;
            map_event_refresh_dialogue_stage(&map_event_pool[2]);
            return;
        }
        break;
    case 8:
        if (item_stock[0][KF_ITEM_MIRROR_OF_TRUTH] != 0 && map_event_pool[2].dialogue_stage == 2
            && map_event_pool[2].dialogue_page < 2) {
            magic_records[KF_MAGIC_HEALING].learned = 1;
            item_stock[0][KF_ITEM_MIRROR_OF_TRUTH]--;
            notify_enqueue(1);
            map_event_pool[2].dialogue_pages.last_page[1] = 7;
            talk_show_dialogue_page(player_state.progress_state.current_floor,
                                    event->dialogue_stage, event->character_id, 2);
            map_event_pool[2].dialogue_page = 3;
            map_event_pool[2].dialogue_page_delay = 0;
            map_event_pool[2].dialogue_stage_limit = 5;
            map_event_refresh_dialogue_stage(&map_event_pool[2]);
            return;
        }
        break;
    case 7:
        if (item_stock[0][KF_ITEM_DRAGON_KING_GRASS_FRUIT] != 0 && map_event_pool[1].dialogue_stage == 2
            && map_event_pool[1].dialogue_page < 2) {
            item_stock[0][KF_ITEM_HARP] = 1;
            map_event_pool[1].dialogue_pages.last_page[1] = 5;
            item_stock[0][KF_ITEM_DRAGON_KING_GRASS_FRUIT]--;
            talk_show_dialogue_page(player_state.progress_state.current_floor,
                                    event->dialogue_stage, event->character_id, 2);
            map_event_pool[1].dialogue_page = 3;
            map_event_pool[1].dialogue_page_delay = 0;
            map_event_pool[1].dialogue_stage_limit = 2;
            map_event_refresh_dialogue_stage(&map_event_pool[1]);
            return;
        }
        break;
    case 12:
        map_object_pool_clear_link(0x37);
        break;
    default:
        break;
    }

    if (event->dialogue_stage_limit != 0) {
        if (event->dialogue_pages.last_page[event->dialogue_stage - 1] != 0) {
            talk_show_dialogue_page(player_state.progress_state.current_floor,
                                    event->dialogue_stage, event->character_id, event->dialogue_page);
            if (event->dialogue_page_delay == 0) {
                event->dialogue_page_delay = KF_DIALOGUE_PAGE_DELAY_TICKS;
            }
        }
    }
}

ADDRESS(0x80034d54, 0x90)
void map_show_screen_image(s32 group, s32 index)
{
    char *directory_floor = &map_screen_image_path[5];

    map_screen_image_path[8] = group + '0';
    *directory_floor = player_state.progress_state.current_floor + '0';
    map_screen_image_path[9] = index / 10 + '0';
    map_screen_image_path[10] = index % 10 + '0';
    screen_show_image_until_input(map_screen_image_path);
}

ADDRESS(0x80034de4, 0x904)
void map_interaction_dispatch(const VECTOR *position, SVECTOR *rotation)
{
    s32 sound_x;
    s32 sound_z;
    s32 index;
    s32 slot;
    s32 item_index;
    s32 result;
    KfItemPickupResult pickup_result;
    s32 neighbor_index;
    u16 saved_pitch;
    u8 found_item;
    u8 *item_id;
    KfMapEvent *event;
    KfMapObject *object;
    KfMapObject *neighbor;
    KfMapObjectDefinition *definition;
    KfMapObjectDefinition *neighbor_definition;

    sound_x = position->vx - (rsin(rotation->vy) * 1500 >> KF_FIXED12_BITS);
    sound_z = position->vz + (rcos(rotation->vy) * 1500 >> KF_FIXED12_BITS);
    switch (map_cell_attribute_grid[sound_z / KF_MAP_TILE_SIZE][sound_x / KF_MAP_TILE_SIZE]) {
    case 0x3a:
        notify_enqueue(0xc);
        break;
    case 0x3f:
        notify_enqueue(0x17);
        break;
    case 0x5d:
        notify_enqueue(0x18);
        break;
    case 0x45:
        notify_enqueue(0x11);
        break;
    default:
        break;
    }

    sound_x = position->vx - (rsin(rotation->vy) * 1000 >> KF_FIXED12_BITS);
    sound_z = position->vz + (rcos(rotation->vy) * 1000 >> KF_FIXED12_BITS);
    if (notification_state.control.effect_phase == KF_NOTIFICATION_IDLE) {
        index = map_event_pool_find_overlap(sound_x, sound_z, MAP_INTERACTION_RADIUS_PADDING);
        if (index != -1) {
            event = &map_event_pool[index];
            switch (event->behavior) {
            case KF_MAP_EVENT_BEHAVIOR_SHOP:
                event->animation_phase = 0;
                event->animation_clip = 0;
                map_event_advance_animation_blocking(event, KF_MAP_EVENT_ANIMATION_TALK_POSE, KF_MAP_EVENT_ANIMATION_TALK_STEP);
                audio_play_map_sequence(2);
                map_event_interact(event);
                menu_enter_mode(KF_MENU_MODE_SHOP, event->character_id);
                audio_play_current_map_sequence();
                map_event_advance_animation_blocking(event, KF_MAP_EVENT_ANIMATION_PHASE_MASK, KF_MAP_EVENT_ANIMATION_TALK_STEP);
                event->animation_clip = 0;
                event->animation_phase = 0;
                player_clear_motion();
                break;
            case KF_MAP_EVENT_BEHAVIOR_WANDER:
                map_event_interact(event);
                player_clear_motion();
                break;
            case KF_MAP_EVENT_BEHAVIOR_ANIMATION_LOOP:
                map_event_advance_animation_blocking(event, KF_MAP_EVENT_ANIMATION_PHASE_MASK, KF_MAP_EVENT_ANIMATION_FINISH_STEP);
                result = asset_registry_entries[event->model_index]->animation_clip_count < 2;
                if (result == 0) {
                    event->animation_phase = 0;
                    event->animation_clip = 1;
                    map_event_advance_animation_blocking(event, KF_MAP_EVENT_ANIMATION_TALK_POSE, KF_MAP_EVENT_ANIMATION_TALK_STEP);
                }
                map_event_interact(event);
                if (result == 0) {
                    map_event_advance_animation_blocking(event, KF_MAP_EVENT_ANIMATION_PHASE_MASK, KF_MAP_EVENT_ANIMATION_TALK_STEP);
                }
                event->animation_clip = 0;
                event->animation_phase = 0;
                player_clear_motion();
                break;
            default:
                break;
            }
        }
        return;
    }

    for (slot = 0;; slot++) {
        index = map_object_pool_find_interaction_from(slot, sound_x, sound_z, MAP_INTERACTION_RADIUS_PADDING);
        if (index == -1) {
            break;
        }
        object = &map_object_state.objects[index];
        definition = &map_object_state.definitions[object->object_id];
        switch (definition->behavior_type) {
        case KF_MAP_OBJECT_BEHAVIOR_HINGED_CONTAINER:
            if (object->link.link_id != KF_MAP_LINK_NONE) {
                notify_enqueue(object->link.linked_notification);
                continue;
            }
            if (!angle_within_tolerance(rotation->vy, object->rotation.y, KF_ANGLE_EIGHTH_TURN)) {
                break;
            }

            item_index = MAP_CONTAINER_ITEM_COUNT - 1;
            while (object->link.action_parameter == KF_MAP_OBJECT_PARAMETER_NONE) {
                item_index--;
                if ((s16)item_index == -1) {
                    goto notify_default;
                }
            }

            audio_play_spatial_default_range(
                &gameplay_sound_ref_2, (const VECTOR *)&object->position_x, KF_AUDIO_MAX_VOLUME);
            saved_pitch = rotation->vx;
            while (object->rotation.x >= -(KF_ANGLE_QUARTER_TURN - 1)) {
                if ((u16)(rotation->vx - 191) >= 1858) {
                    rotation->vx += 16;
                }
                object->rotation.x -= 32;
                render_frame(position, rotation);
                frame_pacer_wait();
            }

            item_index = MAP_CONTAINER_ITEM_COUNT - 1;
            item_id = &object->link.action_parameter;
            for (;;) {
                if (*item_id != MAP_CONTAINER_ITEM_NONE) {
                    pickup_result = KF_ENUM_DECODE(KfItemPickupResult, menu_enter_mode(KF_MENU_MODE_ITEM_PICKUP, *item_id));
                    if (pickup_result == KF_ITEM_PICKUP_ACQUIRED) {
                        *item_id = MAP_CONTAINER_ITEM_NONE;
                    } else if (pickup_result == KF_ITEM_PICKUP_STACK_FULL) {
                        notify_enqueue(0x10);
                    }
                }
                item_index--;
                if ((s16)item_index == -1) {
                    break;
                }
                item_id++;
            }
            object->rotation.x = 0;
            rotation->vx = saved_pitch;
            break;

        case KF_MAP_OBJECT_BEHAVIOR_ITEM_CONTAINER:
            item_id = &object->link.link_id;
            found_item = 0;
            item_index = MAP_CONTAINER_ITEM_COUNT - 1;
            for (;;) {
                if (*item_id != MAP_CONTAINER_ITEM_NONE) {
                    found_item = 1;
                    pickup_result = KF_ENUM_DECODE(KfItemPickupResult, menu_enter_mode(KF_MENU_MODE_ITEM_PICKUP, *item_id));
                    if (pickup_result == KF_ITEM_PICKUP_ACQUIRED) {
                        *item_id = MAP_CONTAINER_ITEM_NONE;
                    } else if (pickup_result == KF_ITEM_PICKUP_STACK_FULL) {
                        notify_enqueue(0x10);
                    }
                }
                item_index--;
                if ((s16)item_index == -1) {
                    break;
                }
                item_id++;
            }
            if (found_item == 0) {
                goto notify_default;
            }
            continue;

        case KF_MAP_OBJECT_BEHAVIOR_LIFT_DOOR:
            if (!angle_within_tolerance(rotation->vy, object->rotation.y, MAP_DOOR_FACING_TOLERANCE)
                && !angle_within_tolerance(
                    rotation->vy, object->rotation.y + KF_ANGLE_HALF_TURN, MAP_DOOR_FACING_TOLERANCE)) {
                break;
            }
            if (object->action != KF_MAP_OBJECT_ACTION_IDLE) {
                break;
            }
            if (object->link.link_id != KF_MAP_LINK_NONE) {
                goto notify_default;
            }
            map_object_start_action_if_idle(object, KF_MAP_OBJECT_ACTION_LIFT_DOOR);
            continue;

        case KF_MAP_OBJECT_BEHAVIOR_HINGED_DOOR:
        case KF_MAP_OBJECT_BEHAVIOR_HINGED_DOOR_PARTNER:
            if (!angle_within_tolerance(rotation->vy, object->rotation.y, MAP_DOOR_FACING_TOLERANCE)
                && !angle_within_tolerance(
                    rotation->vy, object->rotation.y + KF_ANGLE_HALF_TURN, MAP_DOOR_FACING_TOLERANCE)) {
                break;
            }
            if (object->link.link_id != KF_MAP_LINK_NONE && definition->behavior_type == KF_MAP_OBJECT_BEHAVIOR_HINGED_DOOR) {
                goto notify_default;
            }

            neighbor_index = 0;
            for (;;) {
                neighbor_index = map_object_pool_find_interaction_from(
                    neighbor_index, object->position_x, object->position_z, 6000);
                if (neighbor_index == -1) {
                    object->link.action_parameter = KF_MAP_OBJECT_PARAMETER_NONE;
                    break;
                }
                if (neighbor_index != index) {
                    neighbor = &map_object_state.objects[neighbor_index];
                    neighbor_definition =
                        &map_object_state.definitions[neighbor->object_id];
                    if (neighbor_definition->behavior_type < KF_MAP_OBJECT_BEHAVIOR_LIFT_DOOR) {
                        if (neighbor->link.link_id != KF_MAP_LINK_NONE
                            && neighbor_definition->behavior_type == KF_MAP_OBJECT_BEHAVIOR_HINGED_DOOR) {
                            goto notify_default;
                        }
                        map_object_start_action_if_idle(
                            neighbor, neighbor_definition->behavior_type);
                        object->link.action_parameter = neighbor_index;
                        neighbor->link.action_parameter = index;
                        break;
                    }
                }
                neighbor_index++;
            }
            map_object_start_action_if_idle(object, definition->behavior_type);
            continue;

        case KF_MAP_OBJECT_BEHAVIOR_ITEM_PICKUP:
            pickup_result = KF_ENUM_DECODE(KfItemPickupResult, menu_enter_mode(KF_MENU_MODE_ITEM_PICKUP, object->object_id));
            if (pickup_result == KF_ITEM_PICKUP_ACQUIRED) {
                object->object_id = KF_MAP_OBJECT_FREE;
            } else if (pickup_result == KF_ITEM_PICKUP_STACK_FULL) {
                notify_enqueue(0x10);
                continue;
            }
            break;

        case KF_MAP_OBJECT_BEHAVIOR_GOLD_PICKUP:
            result = object->link.link_id | object->link.action_parameter << 8;
            notify_enqueue(KF_NOTIFICATION_GOLD, result);
            player_state.gold += result;
            object->object_id = KF_MAP_OBJECT_FREE;
            break;

        case KF_MAP_OBJECT_BEHAVIOR_EFFECT_SWITCH:
            if (object->link.link_id == KF_MAP_LINK_NONE) {
                goto notify_default;
            }
            object->action_timer = KF_MAP_OBJECT_SWITCH_FORWARD;
            break;

        case KF_MAP_OBJECT_BEHAVIOR_RESTORE_POINT:
            if (object->link.link_id != KF_MAP_LINK_NONE) {
                goto notify_default;
            }
            player_restore_vitals_with_color_cycle();
            continue;

        case KF_MAP_OBJECT_BEHAVIOR_SCREEN_IMAGE:
            if (notification_state.control.effect_phase != KF_NOTIFICATION_IDLE) {
                break;
            }
            if (object->object_id == 0x82) {
                result = 0;
            } else {
                if (object->object_id != 0x83) {
                    return;
                }
                result = 1;
            }
            map_show_screen_image(result, object->link.link_id);
            player_clear_motion();
            continue;

        case KF_MAP_OBJECT_BEHAVIOR_SAVE_POINT:
            map_world_state_persist();
            menu_save_confirm();
            continue;

        default:
notify_default:
            notify_enqueue(object->link.default_notification);
            break;
        }
    }

    switch (player_state.progress_state.current_floor) {
    case 1:
        map_action_script_floor1();
        break;
    case 2:
        map_action_script_floor2();
        break;
    case 3:
        map_action_script_floor3();
        break;
    case 4:
        map_action_script_floor4();
        break;
    case 5:
        map_action_script_floor5();
        break;
    default:
        break;
    }
}
