#ifndef KF_GAME_PLAYER_ACTIONS_H
#define KF_GAME_PLAYER_ACTIONS_H

#include <kf/platform/frame_task.hpp>
#include <kf/game/world.h>
#include <kf/net/protocol.hpp>
#include <kf/game/menu.h>

// One local menu confirmation at a time; never serialized or replayed after
// reconnect. The session supplies identity, epoch and sequence, not the menu.
struct PlayerActions {
    kf::net::Command command;
    KfNetInteraction interaction {};
    u32 next_sequence = 1;
    int container = -1;
    s16 container_pitch {};
    u32 container_generation {};
    bool pending {}, sent {}, completed {}, accepted {};
};
struct PlayerCommandState {
    u32 last_sequence {};
    bool return_to_entry {};
    int event = -1;
    KfNetInteraction interaction {};
};
kf::FrameTask<bool> player_request_action(PlayerContext &player,
    kf::net::CommandKind kind, u16 object, u16 argument = 0, u32 target_generation = 0);
struct PlayerLootOffer {
    u16 object {}, gold {};
    u8 component {};
    KfObjectId item = KF_OBJECT_NONE;
    u32 generation {};
};
bool player_loot_offer(WorldState &world, const PlayerContext &player, u16 object, u8 component, PlayerLootOffer &offer);
bool player_loot_visible(const WorldState &world, u8 slot, u16 object);
kf::FrameTask<void> player_loot_interact(WorldState &world, PlayerContext &player, u16 object);
bool player_equip_item(PlayerContext &player, KfEquipmentMenuCategory category, KfObjectId item);
bool player_consume_item(PlayerContext &player, KfObjectId item);
bool player_use_world_item(WorldState &world, PlayerContext &player, KfObjectId item);
bool player_use_support_magic(WorldState &world, PlayerContext &player, KfEffectKind magic);
bool player_drop_item(PlayerContext &player, KfObjectId item);
bool player_trade_item(PlayerContext &player, KfItemStockBank shop, KfObjectId item, bool buying);
int player_interaction_event(WorldState &world, const PlayerContext &player);
int player_interaction_object(WorldState &world, const PlayerContext &player, int first = 0);

// Host authorization rechecks current state even if the menu was opened earlier.
bool party_apply_command(WorldState &world, u8 slot, PlayerCommandState &state,
    const kf::net::Command &command);

#endif
