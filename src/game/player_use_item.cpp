#include <kf/game/system.h>
#include <kf/game/world.h>
#include <kf/game/audio.h>
#include <kf/lib/null.h>
#include <kf/lib/bool.h>

#include <kf/game/player.h>
#include <kf/game/game.h>
#include <kf/game/player_actions.h>
static constexpr unsigned enemy_image_number_offset = 7;
static constexpr unsigned enemy_image_path_capacity = 14, person_image_path_capacity = 15;
static constexpr unsigned person_image_number_offset = 8;

enum {
    PLAYER_KEY_UNLOCK_VOLUME = 110,
    PLAYER_ILLUSION_STAFF_TIMER_RELOAD = 1000,
    PLAYER_MIRROR_TARGET_DISTANCE = 6000,
    PLAYER_MIRROR_ANGLE_TOLERANCE = KF_ANGLE_FULL_TURN / 12,
    PLAYER_HARP_PROGRESS_PER_UPDATE = 150,
    PLAYER_HARP_CELL_STAGGER = 800,
    PLAYER_HARP_FLOOR2_FIRST_SEGMENT = 0,
    PLAYER_HARP_FLOOR2_SEGMENT_COUNT = 4,
    PLAYER_HARP_FLOOR2_SWEEP_UPDATES = 43,
    PLAYER_HARP_FLOOR2_HOLD_COUNTDOWN = 70,
    PLAYER_HARP_FLOOR3_FIRST_SEGMENT = 4,
    PLAYER_HARP_FLOOR3_SEGMENT_COUNT = 1,
    PLAYER_HARP_FLOOR3_SWEEP_UPDATES = 88,
    PLAYER_HARP_FLOOR3_HOLD_COUNTDOWN = 270
};

char enemy_info_image_path_template[enemy_image_path_capacity] = "ENE0/EI00.TIM";

char person_image_path_template[person_image_path_capacity] = "PRSN/PER00.TIM";

kf::FrameTask<void> actor_show_info_image(WorldState &world, PlayerContext &player, const KfActor *actor)
{
    (co_await render_frame(world, player, NULL, NULL));
    (co_await render_frame(world, player, NULL, NULL));
    enemy_info_image_path_template[3] = '0' + kf_enum_encode<u8>(player.state.progress_state.current_floor);
    enemy_info_image_path_template[enemy_image_number_offset] = '0' + actor->definition_id / 10;
    enemy_info_image_path_template[enemy_image_number_offset + 1] = '0' + actor->definition_id % 10;
    (co_await screen_show_image_until_input(enemy_info_image_path_template));
}

kf::FrameTask<void> map_event_show_person_image(WorldState &world, PlayerContext &player, const KfMapEvent *event)
{
    (co_await render_frame(world, player, NULL, NULL));
    (co_await render_frame(world, player, NULL, NULL));
    person_image_path_template[person_image_number_offset] = '0' + kf_enum_encode<u8>(event->character_id) / 10;
    person_image_path_template[person_image_number_offset + 1] = '0' + kf_enum_encode<u8>(event->character_id) % 10;
    (co_await screen_show_image_until_input(person_image_path_template));
}

bool player_use_world_item(WorldState &world, PlayerContext &player, KfObjectId item_id)
{
    KfMapObject *object;
    KfEffectRecord *record;
    s32 index;
    s16 slot;
    KfBool8 used = false;
    bool feedback = false;
    const auto notify = [&](KfNotificationId id) {
        feedback = true;
        if (player.local_view) notify_enqueue(id);
    };

    if (world.prediction || player.prediction || kf_enum_encode<unsigned>(item_id) >= KF_ITEM_COUNT ||
        !player.item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(item_id)]) return false;
    if (player_consume_item(player, item_id)) return true;

    const auto reach = vector_yaw_probe_xz(player.state.camera_position,
        player.state.camera_rotation.vy, MAP_INTERACTION_PROBE_DISTANCE);
    index = 0;
    switch (item_id) {
    default:
        // Unusable items reach the existing "nothing happens" notification.
        break;
    case KF_ITEM_KEY_OF_THE_DEAD:
    case KF_ITEM_RAITO_FAMILY_KEY:
    case KF_ITEM_DUNGEON_KEY:
    case KF_ITEM_SORCERER_KEY:
        for (;;) {
            index = map_object_pool_find_interaction_from(world,
                index, reach.x, reach.z, MAP_INTERACTION_RADIUS_PADDING);
            if (index == -1) {
                break;
            }
            object = &world.objects.objects[index];
            switch (object->object_id) {
            default:
                // Only the listed doors and lids accept keys.
                break;
            case KF_MAP_OBJECT_BEVELED_WOODEN_LID:
            case KF_MAP_OBJECT_FLAT_WOODEN_LID:
            case KF_MAP_OBJECT_STONE_CONTAINER_LID:
            case KF_MAP_OBJECT_GRAVESTONE:
            case KF_MAP_OBJECT_LIFTING_GATE:
            case KF_MAP_OBJECT_PORTCULLIS:
            case KF_MAP_OBJECT_HINGED_DOOR:
            case KF_MAP_OBJECT_HINGED_DOOR_PARTNER:
            case KF_MAP_OBJECT_TALL_HINGED_DOOR:
            case KF_MAP_OBJECT_TALL_HINGED_DOOR_PARTNER:
                if (object->link.fields.link_id == KF_MAP_LINK_NONE) {
                    notify(KF_NOTIFICATION_NOTHING_HAPPENS);
                } else if (object->object_id != KF_MAP_OBJECT_GRAVESTONE
                           || angle_within_tolerance(
                               player.state.camera_rotation.vy, KF_ANGLE_HALF_TURN - object->rotation.angles.y, MAP_DOOR_FACING_TOLERANCE)) {
                    if (object->link.fields.link_id == kf_enum_encode<u8>(item_id)) {
                        used = true;
                        object->link.fields.link_id = KF_MAP_LINK_NONE;
                        sound_ref_play(audio_playback(player), &gameplay_sound_refs[KF_GAMEPLAY_SOUND_KEY_UNLOCK], PLAYER_KEY_UNLOCK_VOLUME);
                        if (object->object_id == KF_MAP_OBJECT_GRAVESTONE) {
                            sound_ref_play(audio_playback(player), &gameplay_sound_refs[KF_GAMEPLAY_SOUND_STONE_PASSAGE], KF_AUDIO_MAX_VOLUME);
                        }
                    } else {
                        notify(KF_NOTIFICATION_KEY_DOES_NOT_FIT);
                    }
                }
                break;
            }
            index++;
        }
        break;
    case KF_ITEM_DRAGON_CHALICE:
    case KF_ITEM_WATER_SEAL_STONE:
    case KF_ITEM_EARTH_SEAL_STONE:
    case KF_ITEM_FIRE_SEAL_STONE:
    case KF_ITEM_WIND_SEAL_STONE:
        for (;;) {
            index = map_object_pool_find_interaction_from(world,
                index, reach.x, reach.z, MAP_INTERACTION_RADIUS_PADDING);
            if (index == -1) {
                break;
            }
            object = &world.objects.objects[index];
            if (object->object_id == kf_enum_decode<KfObjectId>(kf_enum_encode<u8>(item_id))) {
                if (object->link.fields.link_id == KF_MAP_LINK_NONE) {
                    notify(KF_NOTIFICATION_NOTHING_HAPPENS);
                } else {
                    player.item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(object->object_id)] = 0;
                    used = true;
                    map_object_pool_trigger_link(world, object->link.fields.link_id);
                    object->link.fields.link_id = KF_MAP_LINK_NONE;
                }
            }
            index++;
        }
        break;
    case KF_ITEM_HARP:
        record = world.effects.records;
        for (slot = KF_EFFECT_CAPACITY - 1; slot != -1; slot--, record++) {
            if (record->type == KF_EFFECT_SLOT_FREE) {
                continue;
            }
            if (record->kind == KF_EFFECT_KIND_FLOOR_DEFORMATION) {
                notify(KF_NOTIFICATION_NOTHING_HAPPENS);
                return false;
            }
        }
        if (player.state.progress_state.current_floor == KF_FLOOR_2) {
            record = effect_pool_spawn_floor_deformation(world,
                PLAYER_HARP_FLOOR2_FIRST_SEGMENT, PLAYER_HARP_FLOOR2_SEGMENT_COUNT,
                PLAYER_HARP_PROGRESS_PER_UPDATE, PLAYER_HARP_CELL_STAGGER,
                PLAYER_HARP_FLOOR2_SWEEP_UPDATES, PLAYER_HARP_FLOOR2_HOLD_COUNTDOWN);
        } else if (player.state.progress_state.current_floor == KF_FLOOR_3) {
            record = effect_pool_spawn_floor_deformation(world,
                PLAYER_HARP_FLOOR3_FIRST_SEGMENT, PLAYER_HARP_FLOOR3_SEGMENT_COUNT,
                PLAYER_HARP_PROGRESS_PER_UPDATE, PLAYER_HARP_CELL_STAGGER,
                PLAYER_HARP_FLOOR3_SWEEP_UPDATES, PLAYER_HARP_FLOOR3_HOLD_COUNTDOWN);
        } else {
            break;
        }
        if (!record) {
            notify(KF_NOTIFICATION_NOTHING_HAPPENS);
            return false;
        }
        sound_ref_play(audio_playback(player), &gameplay_sound_refs[KF_GAMEPLAY_SOUND_HARP], KF_AUDIO_MAX_VOLUME);
        used = true;
        break;
    case KF_ITEM_ILLUSION_STAFF:
        player.state.illusion_staff_timer = PLAYER_ILLUSION_STAFF_TIMER_RELOAD;
        if (player.item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(KF_ITEM_ILLUSION_STAFF)] != 0) {
            player.item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(KF_ITEM_ILLUSION_STAFF)]--;
        }
        return true;
    }
    if (!used && !feedback) notify(KF_NOTIFICATION_NOTHING_HAPPENS);
    return used;
}

kf::FrameTask<void> player_use_item(WorldState &world, PlayerContext &player, KfObjectId item_id)
{
    if (kf_enum_encode<unsigned>(item_id) >= KF_ITEM_COUNT || !player.item_stock[0][kf_enum_encode<u8>(item_id)]) co_return;
    s32 distance;
    KfActor *actor;
    KfMapEvent *event;
    switch (item_id) {
    case KF_ITEM_GREEN_DRAGON_STAFF:
        co_await player_warp_to_floor_entry(world, player);
        co_return;
    case KF_ITEM_MIRROR_OF_TRUTH:
        actor = actor_pool_find_target_in_cone(world,
            &player.state.camera_position,
            player.state.camera_rotation.vy,
            PLAYER_MIRROR_TARGET_DISTANCE,
            PLAYER_MIRROR_ANGLE_TOLERANCE,
            &distance);
        if (actor != NULL) {
            (co_await actor_show_info_image(world, player, actor));
            co_return;
        }
        event = map_event_pool_find_target_in_cone(world,
            &player.state.camera_position,
            player.state.camera_rotation.vy,
            PLAYER_MIRROR_TARGET_DISTANCE,
            PLAYER_MIRROR_ANGLE_TOLERANCE,
            &distance);
        if (event == NULL) {
            break;
        }
        (co_await map_event_show_person_image(world, player, event));
        co_return;
    default:
        player_use_world_item(world, player, item_id);
        co_return;
    }
    if (player.local_view) {
        notify_enqueue(KF_NOTIFICATION_NOTHING_HAPPENS);
    }
}

void player_use_item_reset_module_state(void)
{
    kf::restore_initial_value<enemy_info_image_path_template>();
    kf::restore_initial_value<person_image_path_template>();
}
