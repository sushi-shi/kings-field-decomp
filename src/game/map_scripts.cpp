#include <kf/null.h>
#include <kf/bool.h>
#include <kf/game_graphics.h>

#include <kf/game_actor.h>
#include <kf/map_data.h>
#include <kf/game_map.h>
#include <kf/game_collision.h>
#include <kf/notify.h>
#include <psyq/libc.h>
#include <kf/game.h>

enum class KfMapWeaponTransformPhase : s32 {
    MAP_WEAPON_TRANSFORM_SPIN_UP = 0,
    MAP_WEAPON_TRANSFORM_SPIN_DOWN = 1
}; using enum KfMapWeaponTransformPhase;

enum class KfMapImageGroup : s32 {
    MAP_IMAGE_GROUP_SIGNBOARD = 0,
    MAP_IMAGE_GROUP_INSCRIPTION = 1
}; using enum KfMapImageGroup;

enum {
    MAP_WEAPON_TRANSFORM_HOLD_UPDATES = 40,
    MAP_WEAPON_TRANSFORM_SWAP_COUNTDOWN = 20,
    MAP_SHOP_SEQUENCE_INDEX = 2,
    MAP_FLOOR3_DIALOGUE_DOOR_LINK = 0x37
};

enum {
    MAP_FLOOR1_AMBIENT_VOLUME = 115,
    MAP_SCRIPT_OBJECT_SEARCH_PADDING = 3000,
    MAP_FLOOR2_AMBIENT_RANDOM_LIMIT = 4000,
    MAP_BOSS_REVEAL_YAW_MIN = 1808,
    MAP_BOSS_REVEAL_YAW_END = 2289,
    MAP_PASSAGE_OPEN_SOUND_VOLUME = 100,
    MAP_REVEAL_FADE_IN_STEP = 128,
    MAP_REVEAL_LIGHT_BLEND_SHIFT = 2,
    MAP_REVEAL_RISE_STEP = 130,
    MAP_REVEAL_YAW_STEP = 128,
    MAP_REVEAL_FADE_OUT_STEP = 256,
    MAP_WEAPON_TRANSFORM_HEIGHT = 1300,
    MAP_WEAPON_TRANSFORM_BLAST_HEIGHT = 600,
    MAP_WEAPON_TRANSFORM_MAX_YAW_STEP = 240,
    MAP_WEAPON_TRANSFORM_YAW_ACCELERATION = 1,
    MAP_ATTRIBUTE_PROBE_DISTANCE = 1500,
    MAP_CONTAINER_CAMERA_PITCH_SPAN = KF_ANGLE_HALF_TURN - KF_PLAYER_CAMERA_PITCH_LIMIT + 1,
    MAP_CONTAINER_CAMERA_PITCH_STEP = 16,
    MAP_CONTAINER_OPEN_PITCH_STEP = 32,
    MAP_DOOR_PARTNER_SEARCH_PADDING = 6000
};

#define MAP_DIALOGUE_TRIGGER_MASK 0xffffff00
#define MAP_DIALOGUE_STARTED(stage) \
    ((KF_DIALOGUE_PAGE_DELAY_TICKS << 24) | (KF_DIALOGUE_FIRST_PAGE << 16) \
     | ((stage) << 8))

static KfCameraPathPoint map_floor5_camera_path[2] = {
    {{173000, -11500, 85000, 0}, {0, KF_ANGLE_HALF_TURN, 0, 0}, 100, 0},
    {{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0}
};

static VECTOR map_floor1_sound_position = {65000, -10000, 25000, 0};

static MATRIX map_reveal_light_matrix = {
    {{0, -KF_FIXED12_ONE, 0}, {0, -KF_FIXED12_ONE, 0}, {0, -KF_FIXED12_ONE, 0}}, {0, 0, 0}
};

static char map_screen_image_path[16] = "KAN\\B0\\K000.TIM";

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

void map_ambient_script_floor1(void)
{
    if (map_floor1_script.revival_enabled == KF_MAP_SCRIPT_SET) {
        audio_play_spatial_default_range(
            &gameplay_sound_refs[5], &map_floor1_sound_position, MAP_FLOOR1_AMBIENT_VOLUME);
    }

    switch (map_floor1_script.actor_activation_stage) {
    case KF_MAP_TRIGGER_AWAIT_ENTRY:
        if (player_state.motion_state.fields.map_cell.coords.x >= 7 && player_state.motion_state.fields.map_cell.coords.z >= 31
            && player_state.motion_state.fields.map_cell.coords.x < 12 && player_state.motion_state.fields.map_cell.coords.z < 41) {
            map_floor1_script.actor_activation_stage = KF_MAP_TRIGGER_AWAIT_EXIT;
        }
        break;
    case KF_MAP_TRIGGER_AWAIT_EXIT:
        if (player_state.motion_state.fields.map_cell.coords.x < 2 || player_state.motion_state.fields.map_cell.coords.z < 25
            || player_state.motion_state.fields.map_cell.coords.x >= 14 || player_state.motion_state.fields.map_cell.coords.z >= 46) {
            s32 actor_index;
            s32 object_index;

            map_floor1_script.actor_activation_stage = KF_MAP_TRIGGER_COMPLETE;
            actor_index = actor_pool_find_at_tile(7, 0x28);
            if (actor_index != -1) {
                actor_state.actors[actor_index].lifecycle = KF_ACTOR_LIFECYCLE_DORMANT;
                actor_initialize_slot(actor_index);
            }
            object_index = map_object_pool_find_near_point(0x5208, 0x105b8, MAP_SCRIPT_OBJECT_SEARCH_PADDING);
            if (object_index != -1) {
                map_object_state.objects[object_index].object_id = KF_MAP_OBJECT_BROKEN_STONE_CROSS;
            }
        }
        break;
    }

    switch (map_floor1_script.object_removal_stage) {
    case KF_MAP_TRIGGER_AWAIT_ENTRY:
        if (player_state.motion_state.fields.map_cell.coords.x >= 2 && player_state.motion_state.fields.map_cell.coords.z >= 27
            && player_state.motion_state.fields.map_cell.coords.x < 5 && player_state.motion_state.fields.map_cell.coords.z < 30) {
            map_floor1_script.object_removal_stage = KF_MAP_TRIGGER_AWAIT_EXIT;
        }
        break;
    case KF_MAP_TRIGGER_AWAIT_EXIT:
        if (player_state.motion_state.fields.map_cell.coords.x < 2 || player_state.motion_state.fields.map_cell.coords.z < 11
            || player_state.motion_state.fields.map_cell.coords.x >= 28 || player_state.motion_state.fields.map_cell.coords.z >= 41) {
            s32 object_index;

            map_floor1_script.object_removal_stage = KF_MAP_TRIGGER_COMPLETE;
            object_index = map_object_pool_find_near_point(0x2328, 0xdea8, MAP_SCRIPT_OBJECT_SEARCH_PADDING);
            if (object_index != -1) {
                map_object_state.objects[object_index].object_id = KF_OBJECT_NONE;
            }
        }
        break;
    }
}

void map_ambient_script_floor2(void)
{
    if (map_event_pool[1].dialogue.fields.stage == 2 && map_event_pool[1].dialogue.fields.page < 3
        && rand() < MAP_FLOOR2_AMBIENT_RANDOM_LIMIT) {
        audio_play_spatial_default_range(
            &gameplay_sound_refs[8], &map_event_pool[1].reference_position, KF_AUDIO_MAX_VOLUME);
    }
}

void map_ambient_script_floor3(void)
{
    if (player_state.motion_state.fields.map_cell.coords.x >= 15 && player_state.motion_state.fields.map_cell.coords.x < 18
        && player_state.motion_state.fields.map_cell.coords.z == 0x40) {
        player_restore_vitals_with_color_cycle();
        if (magic_records[kf_enum_encode<u8>(KF_MAGIC_RESIST_FIRE)].learned == KF_MAGIC_UNLEARNED || magic_records[kf_enum_encode<u8>(KF_MAGIC_BLESS)].learned == KF_MAGIC_UNLEARNED) {
            magic_records[kf_enum_encode<u8>(KF_MAGIC_RESIST_FIRE)].learned = KF_MAGIC_LEARNED;
            magic_records[kf_enum_encode<u8>(KF_MAGIC_BLESS)].learned = KF_MAGIC_LEARNED;
            notify_enqueue(KF_NOTIFICATION_MAGIC_LEARNED);
        }
    }
}

void map_ambient_script_floor4(void)
{
}

void map_ambient_script_floor5(void)
{
    KfMapScriptFlag *encounter_started = &map_floor5_script.boss_encounter_started;

    if (*encounter_started == KF_MAP_SCRIPT_UNSET && player_state.motion_state.fields.map_cell.coords.x >= 38
        && player_state.motion_state.fields.map_cell.coords.x < 41 && player_state.motion_state.fields.map_cell.coords.z == 7
        && (u16)player_state.camera_rotation.vy >= MAP_BOSS_REVEAL_YAW_MIN
        && (u16)player_state.camera_rotation.vy < MAP_BOSS_REVEAL_YAW_END) {
        *encounter_started = KF_MAP_SCRIPT_SET;
        screen_show_image_until_input("TALK\\C17\\T55171.TIM");
        render_frame(NULL, NULL);
        render_frame(NULL, NULL);
        screen_show_image_until_input("TALK\\C17\\T55172.TIM");
        actor_state.definitions.entries[7].action_animations[KF_ACTOR_ANIM_SLOT_MELEE] = KF_ANIMATION_CLIP_THIRD;
        actor_state.definitions.entries[7].action_animations[KF_ACTOR_ANIM_SLOT_EFFECT0] = KF_ANIMATION_CLIP_FOURTH;
        actor_state.definitions.entries[7].action_animations[KF_ACTOR_ANIM_SLOT_EFFECT1] = KF_ANIMATION_CLIP_FOURTH;
        actor_state.definitions.entries[7].action_animations[KF_ACTOR_ANIM_SLOT_EFFECT2] = KF_ANIMATION_CLIP_FOURTH;
        actor_state.definitions.entries[7].action_animations[KF_ACTOR_ANIM_SLOT_MULTI_HIT_ATTACK] = KF_ANIMATION_CLIP_SECOND;
        map_apply_copy_region(KF_MAP_COPY_FLOOR5_BOSS_ENCOUNTER);
    }
}

void map_action_script_floor1(void)
{
    if (item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(KF_ITEM_DRAGON_CHALICE)] != 0
        && map_floor1_script.passage_opened == KF_MAP_SCRIPT_UNSET) {
        map_floor1_script.passage_opened = KF_MAP_SCRIPT_SET;
        map_apply_copy_region(KF_MAP_COPY_FLOOR1_PASSAGE);
        sound_ref_play(&gameplay_sound_refs[7], MAP_PASSAGE_OPEN_SOUND_VOLUME);
    }
}

void map_reveal_fade(void)
{
    MATRIX saved;
    s32 blend;

    saved = game_graphics_runtime.render_state.light_matrix_copy;

    for (blend = 0; blend < KF_FIXED12_ONE + 1; blend += MAP_REVEAL_FADE_IN_STEP) {
        lighting_set_color_matrix(&color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_DEFAULT)], &color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_WHITE)], blend);
        if (blend >= KF_FIXED12_ONE / 4 + 1) {
            map_event_pool[3].reference_position.vy -= MAP_REVEAL_RISE_STEP;
            map_event_pool[3].rotation.vy += MAP_REVEAL_YAW_STEP;
        } else {
            matrix_interpolate(&saved, &map_reveal_light_matrix,
                               &game_graphics_runtime.render_state.light_matrix_copy, blend << MAP_REVEAL_LIGHT_BLEND_SHIFT);
        }
        render_frame(NULL, NULL);
        frame_pacer_wait();
    }

    map_event_pool[3].state = KF_MAP_EVENT_DISABLED;
    map_floor5_script.character_arrived = KF_MAP_SCRIPT_SET;

    for (blend = KF_FIXED12_ONE; blend >= 0; blend -= MAP_REVEAL_FADE_OUT_STEP) {
        lighting_set_color_matrix(&color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_DEFAULT)], &color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_WHITE)], blend);
        render_frame(NULL, NULL);
        frame_pacer_wait();
    }

    lighting_set_active_color_matrix(KF_GAME_COLOR_DEFAULT);
    game_graphics_runtime.render_state.light_matrix_copy = saved;
}

void map_action_script_floor2(void)
{
    if ((map_event_pool[3].dialogue.word & MAP_DIALOGUE_TRIGGER_MASK)
            == MAP_DIALOGUE_STARTED(2)
        && map_event_pool[3].state == KF_MAP_EVENT_ACTIVE) {
        map_reveal_fade();
    }
}

void map_action_script_floor3(void)
{
    if (item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(KF_ITEM_WIND_BLADE_BRACELET)] != 0) {
        if (magic_records[kf_enum_encode<u8>(KF_MAGIC_WIND_CUTTER)].learned == KF_MAGIC_UNLEARNED) {
            magic_records[kf_enum_encode<u8>(KF_MAGIC_WIND_CUTTER)].learned = KF_MAGIC_LEARNED;
            notify_enqueue(KF_NOTIFICATION_MAGIC_LEARNED);
        }
    }
    if ((map_event_pool[1].dialogue.word & MAP_DIALOGUE_TRIGGER_MASK)
            == MAP_DIALOGUE_STARTED(3)) {
        if (magic_records[kf_enum_encode<u8>(KF_MAGIC_FIRE_BALL)].learned == KF_MAGIC_UNLEARNED) {
            magic_records[kf_enum_encode<u8>(KF_MAGIC_FIRE_BALL)].learned = KF_MAGIC_LEARNED;
            notify_enqueue(KF_NOTIFICATION_MAGIC_LEARNED);
        }
    }
}

void map_action_script_floor4(void)
{
}

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
        player_equip_weapon(KF_OBJECT_NONE);
    }
    item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(KF_ITEM_DRAGON_SWORD)] = 0;
    collision_adjust_cell_occupancy(player_state.motion_state.fields.map_cell.coords.x, player_state.motion_state.fields.map_cell.coords.z, -1);

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
    player_state.motion_state.fields.map_cell.coords.x = player_state.camera_position.vx / KF_MAP_TILE_SIZE;
    player_state.motion_state.fields.map_cell.coords.z = player_state.camera_position.vz / KF_MAP_TILE_SIZE;
    collision_adjust_cell_occupancy(player_state.motion_state.fields.map_cell.coords.x, player_state.motion_state.fields.map_cell.coords.z, 1);

    ReadColorMatrix(&color_matrix);
    effect = map_object_effect_pool_acquire(
        KF_MAP_OBJECT_PLACEMENT_DROP_FIRST, KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY, map_object_state.effect_sequence_180);
    effect->object_id = KF_ITEM_DRAGON_SWORD;
    effect->cell_x = 85;
    effect->cell_z = 40;
    effect->position.vx = effect->cell_x * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    effect->position.vz = effect->cell_z * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    grid_height = map_floor_height_grid.cells[effect->cell_z][effect->cell_x];
    effect->rotation.angles.z = 0;
    effect->rotation.angles.x = 0;
    effect->rotation.angles.y = KF_ANGLE_HALF_TURN;
    effect->action = KF_MAP_OBJECT_OP_NONE;
    effect->position.vy = -(grid_height * KF_MAP_HEIGHT_STEP) - MAP_WEAPON_TRANSFORM_HEIGHT;

    spin = 0;
    hold = 0;
    phase = MAP_WEAPON_TRANSFORM_SPIN_UP;
    for (;;) {
        switch (phase) {
        case MAP_WEAPON_TRANSFORM_SPIN_UP:
            effect->rotation.angles.y += spin;
            if (hold != 0) {
                hold -= 1;
                if (hold == 1) {
                    phase = MAP_WEAPON_TRANSFORM_SPIN_DOWN;
                } else if (hold == MAP_WEAPON_TRANSFORM_SWAP_COUNTDOWN) {
                    spawn = effect->position;
                    spawn.vy -= MAP_WEAPON_TRANSFORM_BLAST_HEIGHT;
                    effect_pool_construct(
                        0, KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,
                        KF_EFFECT_KIND_RADIAL_BLAST, &spawn, &direction, KfEffectSoundArguments{KF_EFFECT_SOUND_PLAY});
                    effect->object_id = KF_ITEM_MOONLIGHT_SWORD;
                }
            } else if (spin < MAP_WEAPON_TRANSFORM_MAX_YAW_STEP) {
                spin += MAP_WEAPON_TRANSFORM_YAW_ACCELERATION;
            } else {
                hold = MAP_WEAPON_TRANSFORM_HOLD_UPDATES;
            }
            break;
        case MAP_WEAPON_TRANSFORM_SPIN_DOWN:
            effect->rotation.angles.y += spin;
            if (spin > 0) {
                spin -= MAP_WEAPON_TRANSFORM_YAW_ACCELERATION;
            } else {
                map_object_start_action_if_idle(effect, KF_MAP_OBJECT_OP_FALL_AND_TIP);
                effect->link.fields.vertical_velocity = 0;
                return;
            }
            break;
        default:
            break;
        }
        effect_pool_sweep();
        render_frame(NULL, NULL);
    }
}

void map_action_script_floor5(void)
{
    if ((map_event_pool[1].dialogue.word & MAP_DIALOGUE_TRIGGER_MASK)
            == MAP_DIALOGUE_STARTED(5)) {
        map_floor5_transition_cutscene();
        map_floor5_script.weapon_transformed = KF_MAP_SCRIPT_SET;
    }
}

void map_event_interact(KfMapEvent *event)
{
    switch (event->character_id) {
    case KF_CHARACTER_KEY_OF_THE_DEAD_EXCHANGE:
        if (item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(KF_ITEM_GOLD_CROSS)] != 0 && map_event_pool[2].dialogue.fields.stage == 1
            && map_event_pool[2].dialogue.fields.page < 3) {
            item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(KF_ITEM_KEY_OF_THE_DEAD)] = 1;
            map_event_pool[2].dialogue_pages.last_page[0] = 7;
            item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(KF_ITEM_GOLD_CROSS)]--;
            talk_show_dialogue_page(player_state.progress_state.current_floor,
                                    event->dialogue.fields.stage, event->character_id, 3);
            map_event_pool[2].dialogue.fields.page = 4;
            map_event_pool[2].dialogue.fields.page_delay = 0;
            map_event_pool[2].dialogue.fields.stage_limit = 5;
            map_event_refresh_dialogue_stage(&map_event_pool[2]);
            return;
        }
        break;
    case KF_CHARACTER_HEALING_EXCHANGE:
        if (item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(KF_ITEM_MIRROR_OF_TRUTH)] != 0 && map_event_pool[2].dialogue.fields.stage == 2
            && map_event_pool[2].dialogue.fields.page < 2) {
            magic_records[kf_enum_encode<u8>(KF_MAGIC_HEALING)].learned = KF_MAGIC_LEARNED;
            item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(KF_ITEM_MIRROR_OF_TRUTH)]--;
            notify_enqueue(KF_NOTIFICATION_MAGIC_LEARNED);
            map_event_pool[2].dialogue_pages.last_page[1] = 7;
            talk_show_dialogue_page(player_state.progress_state.current_floor,
                                    event->dialogue.fields.stage, event->character_id, 2);
            map_event_pool[2].dialogue.fields.page = 3;
            map_event_pool[2].dialogue.fields.page_delay = 0;
            map_event_pool[2].dialogue.fields.stage_limit = 5;
            map_event_refresh_dialogue_stage(&map_event_pool[2]);
            return;
        }
        break;
    case KF_CHARACTER_HARP_EXCHANGE:
        if (item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(KF_ITEM_DRAGON_KING_GRASS_FRUIT)] != 0 && map_event_pool[1].dialogue.fields.stage == 2
            && map_event_pool[1].dialogue.fields.page < 2) {
            item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(KF_ITEM_HARP)] = 1;
            map_event_pool[1].dialogue_pages.last_page[1] = 5;
            item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(KF_ITEM_DRAGON_KING_GRASS_FRUIT)]--;
            talk_show_dialogue_page(player_state.progress_state.current_floor,
                                    event->dialogue.fields.stage, event->character_id, 2);
            map_event_pool[1].dialogue.fields.page = 3;
            map_event_pool[1].dialogue.fields.page_delay = 0;
            map_event_pool[1].dialogue.fields.stage_limit = 2;
            map_event_refresh_dialogue_stage(&map_event_pool[1]);
            return;
        }
        break;
    case KF_CHARACTER_FLOOR3_DOOR_UNLOCKER:
        map_object_pool_clear_link(MAP_FLOOR3_DIALOGUE_DOOR_LINK);
        break;
    default:
        break;
    }

    if (event->dialogue.fields.stage_limit != 0) {
        if (event->dialogue_pages.last_page[event->dialogue.fields.stage - 1] != 0) {
            talk_show_dialogue_page(player_state.progress_state.current_floor,
                                    event->dialogue.fields.stage, event->character_id, event->dialogue.fields.page);
            if (event->dialogue.fields.page_delay == 0) {
                event->dialogue.fields.page_delay = KF_DIALOGUE_PAGE_DELAY_TICKS;
            }
        }
    }
}

void map_show_screen_image(KfMapImageGroup group, s32 index)
{
    char *directory_floor = &map_screen_image_path[5];

    *directory_floor = kf_enum_encode<u8>(player_state.progress_state.current_floor) + '0';
    map_screen_image_path[8] = kf_enum_encode<s32>(group) + '0';
    map_screen_image_path[9] = index / 10 + '0';
    map_screen_image_path[10] = index % 10 + '0';
    screen_show_image_until_input(directory_floor - 5);
}

void map_interaction_dispatch(const VECTOR *position, SVECTOR *rotation)
{
    s32 sound_x;
    s32 sound_z;
    s32 index;
    s32 result;
    KfMenuResult pickup_result;
    s32 neighbor_index;
    u16 saved_pitch;
    KfBool8 found_item;
    KfMapEvent *event;
    KfMapObject *object;
    KfMapObject *neighbor;
    KfMapObjectDefinition *definition;
    KfMapObjectDefinition *neighbor_definition;

    VECTOR_YAW_PROBE_XZ(sound_x, sound_z, *position, *rotation, MAP_ATTRIBUTE_PROBE_DISTANCE);
    switch (map_cell_attribute_grid.cells[sound_z / KF_MAP_TILE_SIZE][sound_x / KF_MAP_TILE_SIZE]) {
    case KF_MAP_ATTRIBUTE_PITFALL:
        notify_enqueue(KF_NOTIFICATION_PITFALL);
        break;
    case KF_MAP_ATTRIBUTE_POISON_HOLE:
        notify_enqueue(KF_NOTIFICATION_POISON_HOLE);
        break;
    case KF_MAP_ATTRIBUTE_BOTTOMLESS_PIT:
        notify_enqueue(KF_NOTIFICATION_BOTTOMLESS_PIT);
        break;
    case KF_MAP_ATTRIBUTE_HIDDEN_DOOR:
        notify_enqueue(KF_NOTIFICATION_HIDDEN_DOOR);
        break;
    default:
        break;
    }

    VECTOR_YAW_PROBE_XZ(sound_x, sound_z, *position, *rotation, MAP_INTERACTION_PROBE_DISTANCE);
    if (game_graphics_runtime.notification_state.control.effect_phase == KF_NOTIFICATION_IDLE
        && (index = map_event_pool_find_overlap(
                sound_x, sound_z, MAP_INTERACTION_RADIUS_PADDING)) != -1) {
        event = &map_event_pool[index];
        switch (event->behavior) {
            case KF_MAP_EVENT_BEHAVIOR_SHOP:
                event->animation_phase = 0;
                event->animation_clip = KF_ANIMATION_CLIP_FIRST;
                map_event_advance_animation_blocking(event, KF_MAP_EVENT_ANIMATION_TALK_POSE, KF_MAP_EVENT_ANIMATION_TALK_STEP);
                audio_play_map_sequence(MAP_SHOP_SEQUENCE_INDEX);
                map_event_interact(event);
                menu_enter_mode(KF_MENU_MODE_SHOP, kf_enum_decode<KfItemStockBank>(kf_enum_encode<u8>(event->character_id)));
                audio_play_current_map_sequence();
                map_event_advance_animation_blocking(event, KF_MAP_EVENT_ANIMATION_PHASE_MASK, KF_MAP_EVENT_ANIMATION_TALK_STEP);
                goto clear_event_phase;
            case KF_MAP_EVENT_BEHAVIOR_ANIMATION_LOOP:
                result = game_graphics_runtime.asset_registry_entries[
                    event->model_index + KF_ASSET_MAP_EVENT_FIRST]->animation_clip_count;
                map_event_advance_animation_blocking(event, KF_MAP_EVENT_ANIMATION_PHASE_MASK, KF_MAP_EVENT_ANIMATION_FINISH_STEP);
                result = result < 2;
                if (result == 0) {
                    event->animation_phase = 0;
                    event->animation_clip = KF_ANIMATION_CLIP_SECOND;
                    map_event_advance_animation_blocking(event, KF_MAP_EVENT_ANIMATION_TALK_POSE, KF_MAP_EVENT_ANIMATION_TALK_STEP);
                }
                map_event_interact(event);
                if (result == 0) {
                    map_event_advance_animation_blocking(event, KF_MAP_EVENT_ANIMATION_PHASE_MASK, KF_MAP_EVENT_ANIMATION_TALK_STEP);
                }
                event->animation_clip = KF_ANIMATION_CLIP_FIRST;
clear_event_phase:
                event->animation_phase = 0;
                player_clear_motion();
                break;
            case KF_MAP_EVENT_BEHAVIOR_WANDER:
                map_event_interact(event);
                player_clear_motion();
                break;
            default:
                break;
        }
    } else {
        for (index = 0;; index++) {
            index = map_object_pool_find_interaction_from(
                index, sound_x, sound_z, MAP_INTERACTION_RADIUS_PADDING);
            if (index == -1) {
                break;
            }
            object = &map_object_state.objects[index];
            definition = &map_object_state.definitions.entries[
                kf_enum_encode<u8>(object->object_id)];
            switch (definition->behavior_type) {
            case KF_MAP_OBJECT_OP_HINGED_CONTAINER: {
                s16 item_index;
                KfObjectId *item_id;

                if (object->link.fields.link_id != KF_MAP_LINK_NONE) {
                    goto notify_linked;
                }
                if (!angle_within_tolerance(rotation->vy, object->rotation.angles.y, KF_ANGLE_EIGHTH_TURN)) {
                    break;
                }

                item_index = KF_MAP_CONTAINER_ITEM_COUNT - 1;
                for (;;) {
                    KfObjectId item_parameter = object->link.hinged_container.item_ids[0];
                    s32 item_scan_end;

                    if (item_parameter != KF_OBJECT_NONE) {
                        break;
                    }
                    item_scan_end = -1;
                    if (--item_index == item_scan_end) {
                        goto notify_default;
                    }
                }

                saved_pitch = rotation->vx;
                audio_play_spatial_default_range(
                    &gameplay_sound_refs[2], &object->position, KF_AUDIO_MAX_VOLUME);
                while (object->rotation.angles.x >= -(KF_ANGLE_QUARTER_TURN - 1)) {
                    u16 current_pitch = rotation->vx;
                    u16 relative_pitch = current_pitch;

                    relative_pitch -= KF_PLAYER_CAMERA_PITCH_LIMIT;
                    if (relative_pitch >= MAP_CONTAINER_CAMERA_PITCH_SPAN) {
                        rotation->vx = current_pitch + MAP_CONTAINER_CAMERA_PITCH_STEP;
                    }
                    object->rotation.angles.x -= MAP_CONTAINER_OPEN_PITCH_STEP;
                    render_frame(position, rotation);
                    frame_pacer_wait();
                }

                item_id = object->link.hinged_container.item_ids;
                item_index = KF_MAP_CONTAINER_ITEM_COUNT - 1;
                for (;;) {
                    if (*item_id != KF_OBJECT_NONE) {
                        pickup_result = kf_enum_decode<KfMenuResult>(menu_enter_mode(KF_MENU_MODE_ITEM_PICKUP, *item_id));
                        switch (pickup_result) {
                        case KF_MENU_RESULT_ACCEPTED:
                            *item_id = KF_OBJECT_NONE;
                            break;
                        case KF_MENU_RESULT_STACK_FULL:
                            notify_enqueue(KF_NOTIFICATION_CANNOT_CARRY_MORE);
                            break;
                        default:
                            break;
                        }
                    }
                    item_index--;
                    if (item_index == -1) {
                        break;
                    }
                    item_id++;
                }
                object->rotation.angles.x = 0;
                rotation->vx = saved_pitch;
                break;

notify_linked:
                notify_enqueue(object->link.fields.linked_notification);
                continue;

            }

            case KF_MAP_OBJECT_OP_ITEM_CONTAINER: {
                s16 item_index;
                KfObjectId *item_id;

                item_id = object->link.item_ids;
                found_item = KF_FALSE;
                item_index = KF_MAP_CONTAINER_ITEM_COUNT - 1;
                for (;;) {
                    if (*item_id != KF_OBJECT_NONE) {
                        found_item = KF_TRUE;
                        pickup_result = kf_enum_decode<KfMenuResult>(menu_enter_mode(KF_MENU_MODE_ITEM_PICKUP, *item_id));
                        switch (pickup_result) {
                        case KF_MENU_RESULT_ACCEPTED:
                            *item_id = KF_OBJECT_NONE;
                            break;
                        case KF_MENU_RESULT_STACK_FULL:
                            notify_enqueue(KF_NOTIFICATION_CANNOT_CARRY_MORE);
                            break;
                        default:
                            break;
                        }
                    }
                    item_index--;
                    if (item_index == -1) {
                        break;
                    }
                    item_id++;
                }
                if (found_item != KF_FALSE) {
                    break;
                }
                goto notify_default;

            }

            case KF_MAP_OBJECT_OP_LIFT_DOOR:
                if (!angle_within_tolerance(rotation->vy, object->rotation.angles.y, MAP_DOOR_FACING_TOLERANCE)
                    && !angle_within_tolerance(
                        rotation->vy, object->rotation.angles.y + KF_ANGLE_HALF_TURN, MAP_DOOR_FACING_TOLERANCE)) {
                    break;
                }
                if (object->action != KF_MAP_OBJECT_OP_NONE) {
                    break;
                }
                if (object->link.fields.link_id != KF_MAP_LINK_NONE) {
                    notify_enqueue(object->link.fields.default_notification);
                    break;
                }
                map_object_start_action_if_idle(object, KF_MAP_OBJECT_OP_LIFT_DOOR);
                continue;

            case KF_MAP_OBJECT_OP_HINGED_DOOR:
            case KF_MAP_OBJECT_OP_HINGED_DOOR_PARTNER:
                if (!angle_within_tolerance(rotation->vy, object->rotation.angles.y, MAP_DOOR_FACING_TOLERANCE)
                    && !angle_within_tolerance(
                        rotation->vy, object->rotation.angles.y + KF_ANGLE_HALF_TURN, MAP_DOOR_FACING_TOLERANCE)) {
                    break;
                }
                if (object->link.fields.link_id != KF_MAP_LINK_NONE && definition->behavior_type == KF_MAP_OBJECT_OP_HINGED_DOOR) {
                    notify_enqueue(object->link.fields.default_notification);
                    break;
                }

                neighbor_index = 0;
                for (;;) {
                    neighbor_index = map_object_pool_find_interaction_from(
                        neighbor_index, object->position.vx, object->position.vz, MAP_DOOR_PARTNER_SEARCH_PADDING);
                    if (neighbor_index == -1) {
                        break;
                    }
                    if (neighbor_index != index) {
                        neighbor = &map_object_state.objects[neighbor_index];
                        neighbor_definition =
                            &map_object_state.definitions.entries[kf_enum_encode<u8>(neighbor->object_id)];
                        if (neighbor_definition->behavior_type == KF_MAP_OBJECT_OP_HINGED_DOOR
                            || neighbor_definition->behavior_type == KF_MAP_OBJECT_OP_HINGED_DOOR_PARTNER) {
                            if (neighbor->link.fields.link_id != KF_MAP_LINK_NONE
                                && neighbor_definition->behavior_type == KF_MAP_OBJECT_OP_HINGED_DOOR) {
                                goto notify_default;
                            }
                            map_object_start_action_if_idle(
                                neighbor, neighbor_definition->behavior_type);
                            object->link.fields.action_parameter.object_index = neighbor_index;
                            neighbor->link.fields.action_parameter.object_index = index;
                            goto start_paired_door;
                        }
                    }
                    neighbor_index++;
                }
                object->link.fields.action_parameter.object_index = KF_MAP_OBJECT_PARAMETER_NONE;
start_paired_door:
                map_object_start_action_if_idle(object, definition->behavior_type);
                continue;

            case KF_MAP_OBJECT_OP_ITEM_PICKUP:
                pickup_result = kf_enum_decode<KfMenuResult>(menu_enter_mode(KF_MENU_MODE_ITEM_PICKUP, object->object_id));
                switch (pickup_result) {
                case KF_MENU_RESULT_ACCEPTED:
                    object->object_id = KF_OBJECT_NONE;
                    break;
                case KF_MENU_RESULT_STACK_FULL:
                    notify_enqueue(KF_NOTIFICATION_CANNOT_CARRY_MORE);
                    continue;
                default:
                    break;
                }
                break;

            case KF_MAP_OBJECT_OP_GOLD_PICKUP:
                result = object->link.gold_amount;
                notify_enqueue(KF_NOTIFICATION_GOLD, result);
                result += player_state.gold;
                player_state.gold = result;
                object->object_id = KF_OBJECT_NONE;
                break;

            case KF_MAP_OBJECT_OP_EFFECT_SWITCH:
                if (object->link.fields.link_id == KF_MAP_LINK_NONE) {
                    notify_enqueue(object->link.fields.default_notification);
                } else {
                    object->action_timer = KF_MAP_OBJECT_SWITCH_FORWARD;
                }
                break;

            case KF_MAP_OBJECT_OP_RESTORE_POINT:
                if (object->link.fields.link_id != KF_MAP_LINK_NONE) {
                    notify_enqueue(object->link.fields.default_notification);
                    break;
                }
                player_restore_vitals_with_color_cycle();
                continue;

            case KF_MAP_OBJECT_OP_SCREEN_IMAGE: {
                KfMapImageGroup image_group;

                if (game_graphics_runtime.notification_state.control.effect_phase != KF_NOTIFICATION_IDLE) {
                    break;
                }
                if (object->object_id == KF_MAP_OBJECT_SIGNBOARD) {
                    image_group = MAP_IMAGE_GROUP_SIGNBOARD;
                } else {
                    if (object->object_id != KF_MAP_OBJECT_INSCRIPTION_PANEL) {
                        return;
                    }
                    image_group = MAP_IMAGE_GROUP_INSCRIPTION;
                }
                map_show_screen_image(image_group, object->link.fields.link_id);
                player_clear_motion();
                continue;
            }

            case KF_MAP_OBJECT_OP_SAVE_POINT:
                map_world_state_persist();
                menu_save_confirm();
                continue;

            default:
notify_default:
                notify_enqueue(object->link.fields.default_notification);
                break;
            }
        }
    }

    switch (player_state.progress_state.current_floor) {
    case KF_FLOOR_1:
        map_action_script_floor1();
        break;
    case KF_FLOOR_2:
        map_action_script_floor2();
        break;
    case KF_FLOOR_3:
        map_action_script_floor3();
        break;
    case KF_FLOOR_4:
        map_action_script_floor4();
        break;
    case KF_FLOOR_5:
        map_action_script_floor5();
        break;
    default:
        break;
    }
}
