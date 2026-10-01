#include <kf/game/game.h>
#include <kf/game/player_actions.h>
#include <kf/game/avatar.h>
#include <kf/platform/avatars.hpp>

kf::FrameTask<bool> player_request_action(PlayerContext &player,
    kf::net::CommandKind kind, u16 object, u16 argument, u32 target_generation)
{
    auto *actions = player.actions;
    if (!actions || actions->pending || !actions->next_sequence) co_return false;
    actions->command = {{kf::net::MessageKind::Command, 0, actions->next_sequence++, 0}, kind, object, argument, target_generation};
    actions->interaction = {};
    actions->pending = true;
    actions->sent = actions->completed = actions->accepted = false;
    while (!actions->completed) co_await kf::FrameDelay {1};
    const bool accepted = actions->accepted;
    actions->pending = false;
    if (!accepted) co_await menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
    co_return accepted;
}

bool player_equip_item(PlayerContext &player, KfEquipmentMenuCategory category, KfObjectId item)
{
    KfObjectId first, end;
    KfEquipmentSlot slot;
    switch (category) {
    case KF_EQUIP_MENU_WEAPON: first = KF_ITEM_SHORT_SWORD; end = KF_ITEM_IRON_MASK; slot = KF_EQUIPMENT_SLOT_REFRESH_ONLY; break;
    case KF_EQUIP_MENU_HEAD: first = KF_ITEM_IRON_MASK; end = KF_ITEM_BREASTPLATE; slot = KF_EQUIPMENT_SLOT_HEAD; break;
    case KF_EQUIP_MENU_BODY: first = KF_ITEM_BREASTPLATE; end = KF_ITEM_SMALL_SHIELD; slot = KF_EQUIPMENT_SLOT_BODY; break;
    case KF_EQUIP_MENU_SHIELD: first = KF_ITEM_SMALL_SHIELD; end = KF_ITEM_GAUNTLET; slot = KF_EQUIPMENT_SLOT_SHIELD; break;
    case KF_EQUIP_MENU_ARM: first = KF_ITEM_GAUNTLET; end = KF_ITEM_IRON_BOOTS; slot = KF_EQUIPMENT_SLOT_ARM; break;
    case KF_EQUIP_MENU_LEG: first = KF_ITEM_IRON_BOOTS; end = KF_ITEM_GOLD_COIN; slot = KF_EQUIPMENT_SLOT_LEG; break;
    case KF_EQUIP_MENU_ACCESSORY: first = KF_ITEM_LIGHT_RING; end = KF_ITEM_GOLD_CROSS; slot = KF_EQUIPMENT_SLOT_ACCESSORY; break;
    default: return false;
    }
    if (item != KF_OBJECT_NONE && (item < first || item >= end ||
        !player.item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(item)])) return false;
    if ((category == KF_EQUIP_MENU_ARM || category == KF_EQUIP_MENU_LEG) &&
        player.state.equipped_body_armor_id == KF_ITEM_FULL_PLATE) return false;
    if (category == KF_EQUIP_MENU_WEAPON) player_equip_weapon(player, item);
    else {
        player_set_equipment_slot(player, item, slot);
        if (category == KF_EQUIP_MENU_BODY && item == KF_ITEM_FULL_PLATE) {
            player_set_equipment_slot(player, KF_OBJECT_NONE, KF_EQUIPMENT_SLOT_ARM);
            player_set_equipment_slot(player, KF_OBJECT_NONE, KF_EQUIPMENT_SLOT_LEG);
        }
    }
    return true;
}

bool player_consume_item(PlayerContext &player, KfObjectId item)
{
    if (item < KF_ITEM_VERDITE || item >= KF_ITEM_LIGHT_RING) return false;
    auto &count = player.item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(item)];
    if (!count) return false;
    --count;
    switch (item) {
    case KF_ITEM_VERDITE:
        player.state.magic_training += KF_PLAYER_TRAINING_POINTS_PER_GAIN;
        player_increment_magic_training(player);
        break;
    case KF_ITEM_MEDICINAL_HERB: player_adjust_hp(player, 25); break;
    case KF_ITEM_ANTIDOTE_HERB:
        player_adjust_hp(player, 10);
        player.state.status_effect_flags &= KF_PLAYER_STATUS_CURSE | KF_PLAYER_STATUS_DARKNESS | KF_PLAYER_STATUS_SLOWED;
        break;
    case KF_ITEM_RECOVERY_MEDICINE:
        player_adjust_hp(player, 80);
        player.state.status_effect_flags &= KF_PLAYER_STATUS_CURSE | KF_PLAYER_STATUS_DARKNESS;
        break;
    case KF_ITEM_DRAGON_KING_GRASS_LEAF:
        player_adjust_hp(player, 150);
        player.state.status_effect_flags = KF_PLAYER_STATUS_NONE;
        break;
    case KF_ITEM_DRAGON_KING_GRASS_FRUIT:
        player_adjust_hp(player, 300);
        player.state.status_effect_flags = KF_PLAYER_STATUS_NONE;
        player.state.vitals.current_mp = player.state.vitals.maximum_mp;
        break;
    default: break;
    }
    return true;
}

bool player_use_support_magic(WorldState &world, PlayerContext &player, KfEffectKind magic)
{
    if (magic < KF_MAGIC_HEALING || magic >= KF_MAGIC_LIGHTNING_BOLT ||
        player.learned_magic[kf_enum_encode<u8>(magic)] != KF_MAGIC_LEARNED) return false;
    const auto cost = world.effects.magic.entries[kf_enum_encode<u8>(magic)].mp_cost;
    if (player.state.vitals.current_mp < cost) return false;
    player.state.vitals.current_mp -= cost;
    player.cast_pose_ticks = avatar_cast_ticks;
    switch (magic) {
    case KF_MAGIC_HEALING: player_adjust_hp(player, player.state.magic); break;
    case KF_MAGIC_DISPOISON: player.state.status_effect_flags &= KF_PLAYER_STATUS_CURSE | KF_PLAYER_STATUS_DARKNESS; break;
    case KF_MAGIC_RESIST_FIRE:
        player.state.status_effect_flags |= KF_PLAYER_STATUS_FIRE_DEFENSE_BOOST;
        player_apply_fire_defense_boost(player);
        break;
    case KF_MAGIC_BLESS:
        player.state.status_effect_flags &= KF_PLAYER_STATUS_POISON | KF_PLAYER_STATUS_SLOWED;
        player_adjust_hp(player, player.state.magic * 3);
        break;
    default: break;
    }
    return true;
}

bool player_drop_item(PlayerContext &player, KfObjectId item)
{
    if (kf_enum_encode<unsigned>(item) >= KF_ITEM_COUNT) return false;
    auto &count = player.item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(item)];
    if (count <= (player_item_is_equipped(player, item) ? 1 : 0)) return false;
    --count;
    return true;
}

bool player_trade_item(PlayerContext &player, KfItemStockBank shop, KfObjectId item, bool buying)
{
    if (shop < KF_ITEM_STOCK_FIRST_SHOP || shop > KF_ITEM_STOCK_SECOND_SHOP ||
        kf_enum_encode<unsigned>(item) >= KF_ITEM_COUNT) return false;
    const auto id = kf_enum_encode<u8>(item);
    const auto bank = kf_enum_encode<u8>(shop);
    auto &owned = player.item_stock[0][id];
    if (buying) {
        const auto price = item_buy_prices[id][bank - 1];
        if (!player.item_stock[bank][id] || owned >= KF_ITEM_STACK_CAPACITY || player.state.gold < price) return false;
        player.state.gold -= price;
        ++owned;
        // The original shops replenish everything except their one gold cross.
        if (item == KF_ITEM_GOLD_CROSS) --player.item_stock[bank][id];
    } else {
        const auto price = item_sell_prices[id][bank - 1];
        if (item >= KF_ITEM_GOLD_CROSS || owned <= (player_item_is_equipped(player, item) ? 1 : 0) ||
            player.state.gold > std::numeric_limits<u32>::max() - price) return false;
        --owned;
        player.state.gold += price;
    }
    return true;
}

int player_interaction_event(WorldState &world, const PlayerContext &player)
{
    const auto probe = vector_yaw_probe_xz(player.state.camera_position,
        player.state.camera_rotation.vy, MAP_INTERACTION_PROBE_DISTANCE);
    return map_event_pool_find_overlap(world, probe.x, probe.z, MAP_INTERACTION_RADIUS_PADDING);
}

int player_interaction_object(WorldState &world, const PlayerContext &player, int first)
{
    if (first < 0 || first >= KF_MAP_OBJECT_CAPACITY) return -1;
    const auto probe = vector_yaw_probe_xz(player.state.camera_position,
        player.state.camera_rotation.vy, MAP_INTERACTION_PROBE_DISTANCE);
    return map_object_pool_find_interaction_from(world, first, probe.x, probe.z, MAP_INTERACTION_RADIUS_PADDING);
}

static bool player_activate_object(WorldState &world, PlayerContext &player, u16 index, KfObjectId expected)
{
    if (index >= KF_MAP_OBJECT_CAPACITY || player_interaction_event(world, player) >= 0 ||
        player_interaction_object(world, player, index) != index) return false;
    auto &object = world.objects.objects[index];
    if (object.object_id != expected) return false;
    const auto &definition = world.objects.definitions.entries[kf_enum_encode<u8>(object.object_id)];
    switch (definition.behavior_type) {
    case KF_MAP_OBJECT_OP_LIFT_DOOR:
    case KF_MAP_OBJECT_OP_HINGED_DOOR:
    case KF_MAP_OBJECT_OP_HINGED_DOOR_PARTNER:
        if (!angle_within_tolerance(player.state.camera_rotation.vy, object.rotation.angles.y, MAP_DOOR_FACING_TOLERANCE) &&
            !angle_within_tolerance(player.state.camera_rotation.vy, object.rotation.angles.y + KF_ANGLE_HALF_TURN, MAP_DOOR_FACING_TOLERANCE))
            return false;
        if (object.action != KF_MAP_OBJECT_OP_NONE ||
            (object.link.fields.link_id != KF_MAP_LINK_NONE && definition.behavior_type != KF_MAP_OBJECT_OP_HINGED_DOOR_PARTNER))
            return false;
        if (definition.behavior_type != KF_MAP_OBJECT_OP_LIFT_DOOR)
            return map_start_hinged_door_pair(world, &object, &definition, index);
        map_object_start_action_if_idle(&object, definition.behavior_type);
        return true;
    case KF_MAP_OBJECT_OP_EFFECT_SWITCH:
        if (object.link.fields.link_id == KF_MAP_LINK_NONE || object.action_timer == KF_MAP_OBJECT_SWITCH_FORWARD) return false;
        object.action_timer = KF_MAP_OBJECT_SWITCH_FORWARD;
        return true;
    case KF_MAP_OBJECT_OP_RESTORE_POINT: {
        if (object.link.fields.link_id != KF_MAP_LINK_NONE) return false;
        auto task = player_restore_vitals_with_color_cycle(world, player);
        task.advance();
        if (!task.done()) kf::host_fail("Online restoration unexpectedly suspended");
        return true;
    }
    case KF_MAP_OBJECT_OP_SCREEN_IMAGE:
        return map_object_image_valid(object.object_id, object.link.fields.link_id);
    case KF_MAP_OBJECT_OP_SAVE_POINT:
        // A guest may inspect the point, but only the host owns campaign storage.
        return true;
    default: return false;
    }
}

bool player_loot_offer(WorldState &world, const PlayerContext &player, u16 index, u8 component, PlayerLootOffer &offer)
{
    if (!world.party.enabled || player.party_slot >= party_capacity || index >= KF_MAP_OBJECT_CAPACITY ||
        component >= KF_MAP_CONTAINER_ITEM_COUNT || player_interaction_event(world, player) >= 0 ||
        player_interaction_object(world, player, index) != index) return false;
    const auto floor = kf_enum_encode<unsigned>(world.floor) - 1;
    if (floor >= KF_MAP_SAVED_FLOOR_COUNT ||
        (world.party.members[player.party_slot].loot_claims[floor][index] & (1u << component))) return false;
    const auto &object = world.objects.objects[index];
    if (!object.generation || kf_enum_encode<unsigned>(object.object_id) >= KF_MAP_OBJECT_DEFINITION_COUNT) return false;
    PlayerLootOffer result {index, 0, component, KF_OBJECT_NONE, object.generation};
    switch (world.objects.definitions.entries[kf_enum_encode<u8>(object.object_id)].behavior_type) {
    case KF_MAP_OBJECT_OP_HINGED_CONTAINER:
        if (object.link.hinged_container.link_id != KF_MAP_LINK_NONE ||
            !angle_within_tolerance(player.state.camera_rotation.vy, object.rotation.angles.y, KF_ANGLE_EIGHTH_TURN)) return false;
        result.item = object.link.hinged_container.item_ids[component];
        break;
    case KF_MAP_OBJECT_OP_ITEM_CONTAINER: result.item = object.link.item_ids[component]; break;
    case KF_MAP_OBJECT_OP_ITEM_PICKUP:
        if (component) return false;
        result.item = object.object_id;
        break;
    case KF_MAP_OBJECT_OP_GOLD_PICKUP:
        if (component || !object.link.gold_amount) return false;
        result.item = KF_ITEM_GOLD_COIN;
        result.gold = object.link.gold_amount;
        break;
    default: return false;
    }
    if (kf_enum_encode<unsigned>(result.item) >= KF_ITEM_COUNT) return false;
    offer = result;
    return true;
}

bool player_loot_visible(const WorldState &world, u8 slot, u16 index)
{
    if (!world.party.enabled || slot >= party_capacity || index >= KF_MAP_OBJECT_CAPACITY) return true;
    const auto &object = world.objects.objects[index];
    if (kf_enum_encode<unsigned>(object.object_id) >= KF_MAP_OBJECT_DEFINITION_COUNT) return false;
    const auto behavior = world.objects.definitions.entries[kf_enum_encode<u8>(object.object_id)].behavior_type;
    if (behavior != KF_MAP_OBJECT_OP_ITEM_PICKUP && behavior != KF_MAP_OBJECT_OP_GOLD_PICKUP) return true;
    const auto floor = kf_enum_encode<unsigned>(world.floor) - 1;
    return floor < KF_MAP_SAVED_FLOOR_COUNT && !(world.party.members[slot].loot_claims[floor][index] & 1);
}

static bool player_take_loot(WorldState &world, PlayerContext &player, const kf::net::Command &command)
{
    const auto component = static_cast<u8>(command.argument & 255);
    const auto expected = kf_enum_decode<KfObjectId>(command.argument >> 8);
    PlayerLootOffer offer;
    if (!player_loot_offer(world, player, command.object, component, offer) ||
        offer.generation != command.target_generation || offer.item != expected) return false;
    if (offer.gold) {
        if (player.state.gold > std::numeric_limits<u32>::max() - offer.gold) return false;
        player.state.gold += offer.gold;
    } else {
        auto &count = player.item_stock[0][kf_enum_encode<u8>(offer.item)];
        if (count >= KF_ITEM_STACK_CAPACITY) return false;
        ++count;
    }
    world.party.members[player.party_slot].loot_claims[kf_enum_encode<unsigned>(world.floor) - 1][command.object] |= 1u << component;
    if (world.floor == KF_FLOOR_1) map_action_script_floor1(world, player);
    if (world.floor == KF_FLOOR_3) map_action_script_floor3(world, player);
    return true;
}

kf::FrameTask<void> player_loot_interact(WorldState &world, PlayerContext &player, u16 index)
{
    if (!player.actions || index >= KF_MAP_OBJECT_CAPACITY) co_return;
    auto &actions = *player.actions;
    struct ContainerView {
        PlayerActions &actions;
        ~ContainerView() { actions.container = -1; actions.container_pitch = 0; }
    } view {actions};
    bool opened = false;
    for (u8 component = 0; component < KF_MAP_CONTAINER_ITEM_COUNT; ++component) {
        PlayerLootOffer offer;
        if (!player_loot_offer(world, player, index, component, offer)) continue;
        if (!opened && world.objects.definitions.entries[kf_enum_encode<u8>(world.objects.objects[index].object_id)].behavior_type == KF_MAP_OBJECT_OP_HINGED_CONTAINER) {
            opened = true;
            actions.container = index;
            actions.container_generation = offer.generation;
            audio_play_spatial_default_range(player, &gameplay_sound_refs[KF_GAMEPLAY_SOUND_CONTAINER_OPEN],
                &world.objects.objects[index].position, KF_AUDIO_MAX_VOLUME);
            while (actions.container_pitch > -KF_ANGLE_QUARTER_TURN) {
                actions.container_pitch -= 32;
                co_await kf::FrameDelay {3};
            }
        }
        if (!offer.gold) {
            const auto result = kf_enum_decode<KfMenuResult>(co_await menu_enter_mode(world, player, KF_MENU_MODE_ITEM_PICKUP, offer.item));
            if (result == KF_MENU_RESULT_STACK_FULL) notify_enqueue(KF_NOTIFICATION_CANNOT_CARRY_MORE);
            if (result != KF_MENU_RESULT_ACCEPTED) continue;
        }
        if (co_await player_request_action(player, kf::net::CommandKind::TakeLoot, offer.object,
                (kf_enum_encode<u16>(offer.item) << 8) | offer.component, offer.generation)) {
            if (offer.gold) notify_enqueue(KF_NOTIFICATION_GOLD, offer.gold);
        }
    }
}

bool party_apply_command(WorldState &world, u8 slot, PlayerCommandState &state, const kf::net::Command &command)
{
    PartyAudioScope audio(world);
    using namespace kf::net;
    if (!world.party.enabled || world.prediction || slot >= party_capacity ||
        command.header.kind != MessageKind::Command || command.header.epoch != world.epoch ||
        !command.header.sequence || command.header.sequence <= state.last_sequence) return false;
    auto &member = world.party.members[slot];
    if (command.header.generation != member.generation) return false;
    state.last_sequence = command.header.sequence;
    auto &player = member.player;
    if (command.kind == CommandKind::StoryReady)
        return !command.argument && !command.target_generation && party_story_ready(world, slot, command.object);
    if (world.story.kind || !member.connected || !party_member_alive(member) ||
        player.state.weapon_attack_phase != KF_WEAPON_ATTACK_INACTIVE) return false;
    if (command.kind == CommandKind::TakeLoot)
        return player.party_slot == slot && player_take_loot(world, player, command);
    if (command.target_generation) return false;
    // Check wide wire values before converting to the game's byte enums.
    if (command.object > 255 || command.argument > 255) return false;
    const auto item = kf_enum_decode<KfObjectId>(command.object);
    const auto magic = kf_enum_decode<KfEffectKind>(command.object);
    switch (command.kind) {
    case CommandKind::ChooseAvatar:
        if (command.argument || !kf::avatar_mesh(command.object)) return false;
        member.avatar = static_cast<u8>(command.object);
        return true;
    case CommandKind::ActivateObject:
        return player_activate_object(world, player, command.object, kf_enum_decode<KfObjectId>(command.argument));
    case CommandKind::Interact: {
        if (command.argument || command.object >= KF_MAP_EVENT_CAPACITY ||
            player_interaction_event(world, player) != command.object) return false;
        auto &event = world.map.events[command.object];
        if (!event.dialogue.stage || event.dialogue.stage > KF_DIALOGUE_STAGE_COUNT ||
            event.dialogue.page > 9 || kf_enum_encode<unsigned>(event.character_id) > 99 ||
            (event.behavior == KF_MAP_EVENT_BEHAVIOR_SHOP &&
             (kf_enum_encode<unsigned>(event.character_id) < 1 || kf_enum_encode<unsigned>(event.character_id) > 2))) return false;
        DialoguePage dialogue;
        auto task = map_event_interact(world, player, &event, &dialogue);
        task.advance();
        if (!task.done()) kf::host_fail("Captured dialogue unexpectedly suspended");
        if (world.floor == KF_FLOOR_3) map_action_script_floor3(world, player);
        state.event = command.object;
        state.interaction = {{static_cast<u8>(MessageKind::Interaction), world.epoch, command.header.sequence, member.generation},
            kf_enum_encode<u8>(world.floor), dialogue.stage, kf_enum_encode<u8>(dialogue.character), dialogue.page,
            event.behavior == KF_MAP_EVENT_BEHAVIOR_SHOP ? kf_enum_encode<u8>(event.character_id) : u8 {0}};
        return true;
    }
    case CommandKind::Cancel:
        if (command.argument || state.event != command.object || state.interaction.header.epoch != world.epoch ||
            state.interaction.header.generation != member.generation) return false;
        if (auto &dialogue = world.map.events[state.event].dialogue;
            state.interaction.stage && dialogue.stage == state.interaction.stage && dialogue.page == state.interaction.page && !dialogue.page_delay) {
            dialogue.page_delay = KF_DIALOGUE_PAGE_DELAY_TICKS;
            party_story_begin(world, player, command.object);
        }
        state.event = -1;
        if (world.floor == KF_FLOOR_3) map_action_script_floor3(world, player);
        return true;
    case CommandKind::Buy:
    case CommandKind::Sell:
        if (state.event < 0 || state.interaction.header.epoch != world.epoch ||
            state.interaction.header.generation != member.generation || !state.interaction.shop ||
            state.interaction.shop != command.argument || player_interaction_event(world, player) != state.event ||
            world.map.events[state.event].behavior != KF_MAP_EVENT_BEHAVIOR_SHOP ||
            kf_enum_encode<u8>(world.map.events[state.event].character_id) != state.interaction.shop) return false;
        return player_trade_item(player, kf_enum_decode<KfItemStockBank>(command.argument), item, command.kind == CommandKind::Buy);
    case CommandKind::Equip:
        return player_equip_item(player, kf_enum_decode<KfEquipmentMenuCategory>(command.argument), item);
    case CommandKind::SelectMagic:
        if (command.argument || (magic != KF_MAGIC_NONE &&
            (kf_enum_encode<unsigned>(magic) < kf_enum_encode<unsigned>(KF_MAGIC_LIGHTNING_BOLT) ||
             kf_enum_encode<unsigned>(magic) >= KF_MAGIC_PLAYER_COUNT ||
             player.learned_magic[kf_enum_encode<u8>(magic)] != KF_MAGIC_LEARNED))) return false;
        player_select_magic(world, player, magic);
        return true;
    case CommandKind::UseItem:
        if (command.argument) return false;
        if (item == KF_ITEM_GREEN_DRAGON_STAFF) {
            if (!player.item_stock[0][kf_enum_encode<u8>(item)] || state.return_to_entry) return false;
            state.return_to_entry = true;
            return true;
        }
        return player_use_world_item(world, player, item);
    case CommandKind::UseMagic: return !command.argument && player_use_support_magic(world, player, magic);
    case CommandKind::DropItem: return !command.argument && player_drop_item(player, item);
    default: return false;
    }
}
