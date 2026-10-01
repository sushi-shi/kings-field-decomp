#include <kf/game/snapshot.h>
#include <kf/game/game.h>
#include <kf/game/graphics.h>
#include <kf/game/avatar.h>
#include <kf/net/world.h>
#include "snapshot_fields.hpp"
static_assert(party_reward_mask == KF_WORLD_QUEST_REWARD_MASK);
static_assert(avatar_cast_ticks == KF_WORLD_CAST_POSE_TICKS);

void player_rebind_records(WorldState &world, PlayerContext &player)
{
    auto &p = player.state;
    p.selected_magic_record = p.selected_magic_id == KF_MAGIC_NONE ? nullptr
        : &world.effects.magic.entries[kf_enum_encode<u8>(p.selected_magic_id)];
    p.equipped_weapon_record = p.equipped_weapon_id == KF_OBJECT_NONE ? nullptr
        : &weapon_records.entries[kf_enum_encode<u8>(p.equipped_weapon_id)];
    const auto armor = [](KfObjectId id) -> KfArmorRecord * {
        return id == KF_OBJECT_NONE ? nullptr
            : &armor_records.entries[kf_enum_encode<u8>(id) - kf_enum_encode<u8>(KF_ITEM_IRON_MASK)];
    };
    p.equipped_head_armor_record = armor(p.equipped_head_armor_id);
    p.equipped_body_armor_record = armor(p.equipped_body_armor_id);
    p.equipped_shield_record = armor(p.equipped_shield_id);
    p.equipped_arm_armor_record = armor(p.equipped_arm_armor_id);
    p.equipped_leg_armor_record = armor(p.equipped_leg_armor_id);
}

void world_clone_simulation(const WorldState &source, WorldState &destination)
{
    destination.epoch = source.epoch;
    destination.floor = source.floor;
    destination.variant = source.variant;
    destination.prediction = true;
    destination.random = source.random;
    destination.sound_sequence = source.sound_sequence;
    std::copy(std::begin(source.sounds), std::end(source.sounds), std::begin(destination.sounds));
    destination.party = source.party;
    destination.story = source.story;
    destination.actors = source.actors;
    destination.effects = source.effects;
    destination.objects = source.objects;
    destination.map = source.map;
    destination.collision_flags = source.collision_flags;
    destination.cell_orientation = source.cell_orientation;
    destination.floor_height = source.floor_height;
    destination.collision = source.collision;
    destination.cell_attribute = source.cell_attribute;
    destination.actors.current = nullptr;
    destination.actors.current_definition = nullptr;
    destination.actors.player_target = nullptr;
    destination.effects.current_magic = nullptr;
    destination.effects.current_record = nullptr;
    destination.map.current_event = nullptr;
    // Asset storage belongs to the presentation/loader, never a prediction copy.
    destination.map.variant_asset_buffer = nullptr;
    for (auto &actor : destination.actors.actors) actor.animation_cache = nullptr;
    for (auto &effect : destination.effects.records) effect.animation_cache = nullptr;
    for (auto &event : destination.map.events) event.animation_cache = nullptr;
    for (auto &member : destination.party.members) {
        member.player.prediction = true;
        member.player.actions = nullptr;
        member.player.state.weapon_animation_cache = nullptr;
        member.player.state.weapon_asset_buffer = nullptr;
        if (member.presence == PartyPresence::Living || member.presence == PartyPresence::Spectating ||
            member.presence == PartyPresence::Disconnected)
            player_rebind_records(destination, member.player);
    }
}

void world_apply_snapshot(const WorldState &source, WorldState &destination)
{
    // Cache records retain pointers to their owning slots. Release before
    // replacing those slots; preserving a dangling owner would corrupt a later frame.
    animation_cache_release_all();
    const auto slot = destination.party.local_slot;
    const auto effects = destination.party.members[slot].player.state.audio_effects_enabled;
    const auto music = destination.party.members[slot].player.state.audio_music_enabled;
    const auto gauges = destination.party.members[slot].player.state.hud_gauges_enabled;
    const auto compass = destination.party.members[slot].player.state.compass_enabled;
    PlayerActions *actions[party_capacity];
    for (u8 index = 0; index < party_capacity; ++index)
        actions[index] = destination.party.members[index].player.actions;
    auto *weapon_buffer = destination.party.members[slot].player.state.weapon_asset_buffer;
    auto *variant_buffer = destination.map.variant_asset_buffer;
    world_clone_simulation(source, destination);
    destination.party.enabled = true;
    destination.party.local_slot = slot;
    destination.map.variant_asset_buffer = variant_buffer;
    destination.party.members[slot].player.state.weapon_asset_buffer = weapon_buffer;
    destination.party.members[slot].player.state.audio_effects_enabled = effects;
    destination.party.members[slot].player.state.audio_music_enabled = music;
    destination.party.members[slot].player.state.hud_gauges_enabled = gauges;
    destination.party.members[slot].player.state.compass_enabled = compass;
    for (u8 index = 0; index < party_capacity; ++index) {
        destination.party.members[index].player.local_view = index == slot;
        destination.party.members[index].player.actions = actions[index];
    }
}

static KfNetWorldLimits snapshot_limits(const WorldState &world)
{
    KfNetWorldLimits limits {};
    for (std::size_t i = 0; i < KF_WORLD_ASSETS; ++i) {
        const auto *asset = game_graphics_runtime.asset_registry_entries[i];
        limits.asset_clips[i] = asset ? asset->animation_clip_count : -1;
    }
    for (std::size_t i = 0; i < KF_WORLD_ACTOR_DEFINITIONS; ++i)
        limits.actor_assets[i] = world.actors.definitions.entries[i].model_and_texture & 15;
    for (std::size_t i = 0; i < KF_WORLD_OBJECT_DEFINITIONS; ++i)
        limits.object_operations[i] = kf_enum_encode<u8>(world.objects.definitions.entries[i].behavior_type);
    return limits;
}

template<bool Restore>
static void snapshot_world(WorldState &world, KfNetWorld &record)
{
    snapshot_field<Restore>(world.random.state, record.random_state);
    snapshot_field<Restore>(world.sound_sequence, record.sound_sequence);
    snapshot_field<Restore>(world.sounds, record.sounds);
    snapshot_field<Restore>(world.story.kind, record.story.kind);
    snapshot_field<Restore>(world.story.initiator, record.story.initiator);
    snapshot_field<Restore>(world.story.effect, record.story.effect);
    snapshot_field<Restore>(world.story.tick, record.story.tick);
    snapshot_field<Restore>(world.story.object, record.story.object);
    snapshot_field<Restore>(world.story.generation, record.story.generation);
    snapshot_field<Restore>(world.story.camera_x, record.story.camera_x);
    snapshot_field<Restore>(world.story.camera_y, record.story.camera_y);
    snapshot_field<Restore>(world.story.camera_z, record.story.camera_z);
    snapshot_field<Restore>(world.story.pitch, record.story.pitch);
    snapshot_field<Restore>(world.story.yaw, record.story.yaw);
    snapshot_field<Restore>(world.story.roll, record.story.roll);
    snapshot_field<Restore>(world.story.page, record.story.page);
    snapshot_field<Restore>(world.story.ready_mask, record.story.ready_mask);
    snapshot_field<Restore>(world.epoch, record.header.epoch);
    snapshot_field<Restore>(world.floor, record.header.floor);
    snapshot_field<Restore>(world.variant, record.header.variant);
    snapshot_field<Restore>(world.party.quest_rewards, record.quest_rewards);
    for (std::size_t i = 0; i < party_capacity; ++i) {
        auto &member = world.party.members[i];
        auto &saved = record.members[i];
        snapshot_field<Restore>(member.presence, saved.presence);
        for (unsigned byte = 0; byte < member.character_id.size(); ++byte)
            snapshot_field<Restore>(member.character_id[byte], saved.character_id[byte]);
        snapshot_field<Restore>(member.generation, saved.generation);
        snapshot_field<Restore>(member.acknowledged_input, saved.acknowledged_input);
        snapshot_field<Restore>(member.quest_rewards, saved.quest_rewards);
        snapshot_field<Restore>(member.connected, saved.connected);
        snapshot_field<Restore>(member.avatar, saved.avatar);
        snapshot_field<Restore>(member.loot_claims, saved.loot_claims);
        if (saved.presence >= 2) snapshot_player<Restore>(member.player, saved.player);
    }
    for (std::size_t i = 0; i < KF_WORLD_ACTORS; ++i)
        snapshot_actor<Restore>(world.actors.actors[i], record.actors[i]);
    for (std::size_t i = 0; i < KF_WORLD_ACTOR_DEFINITIONS; ++i)
        snapshot_field<Restore>(world.actors.definitions.entries[i].action_animations, record.action_animations[i]);
    for (std::size_t i = 0; i < KF_WORLD_EFFECTS; ++i)
        snapshot_effect<Restore>(world.effects.records[i], record.effects[i]);
    for (std::size_t i = 0; i < KF_WORLD_OBJECTS; ++i)
        snapshot_object<Restore>(world.objects.objects[i], record.objects[i]);
    for (std::size_t i = 0; i < KF_WORLD_EVENTS; ++i)
        snapshot_event<Restore>(world.map.events[i], record.events[i]);
    snapshot_field<Restore>(world.objects.gold_drop_sequence, record.gold_drop_sequence);
    snapshot_field<Restore>(world.objects.definition_drop_sequence, record.definition_drop_sequence);
    snapshot_field<Restore>(world.objects.placement_drop_sequence, record.placement_drop_sequence);
    snapshot_field<Restore>(world.map.dialogue_advance_gate, record.dialogue_advance_gate);
    snapshot_field<Restore>(world.map.ambient_script_countdown, record.ambient_script_countdown);
    for (std::size_t i = 0; i < KF_WORLD_FLOORS; ++i) {
        snapshot_field<Restore>(world.map.world_state.floors[i].script.bytes, record.floors[i].script);
        if (record.header.full) snapshot_field<Restore>(world.map.world_state.floors[i].records, record.floors[i].records);
    }
    snapshot_field<Restore>(world.collision_flags.linear, record.collision_flags);
    snapshot_field<Restore>(world.cell_orientation.linear, record.cell_orientation);
    snapshot_field<Restore>(world.floor_height.linear, record.floor_height);
    snapshot_field<Restore>(world.collision.linear, record.collision);
    snapshot_field<Restore>(world.cell_attribute.linear, record.cell_attribute);
}

static WorldSnapshotInfo snapshot_info(const KfNetWorldHeader &header)
{
    return {header.epoch, header.tick, header.full != 0,
        kf_enum_decode<KfFloorId>(header.floor), kf_enum_decode<KfMapVariant>(header.variant)};
}

bool world_snapshot_info(std::span<const u8> bytes, WorldSnapshotInfo &info)
{
    KfNetWorldHeader header {};
    if (kf_net_world_info(bytes.data(), bytes.size(), &header) != KF_CODEC_OK) return false;
    info = snapshot_info(header);
    return true;
}

bool world_snapshot_encode(WorldState &world, WorldSnapshotInfo info, std::vector<u8> &bytes)
{
    auto record = std::make_unique<KfNetWorld>();
    record->header.full = info.full;
    record->header.tick = info.tick;
    snapshot_world<false>(world, *record);
    const auto limits = snapshot_limits(world);
    bytes.resize(world_snapshot_limit);
    std::size_t written = 0;
    if (kf_net_world_encode(record.get(), &limits, bytes.data(), bytes.size(), &written) != KF_CODEC_OK) {
        bytes.clear();
        return false;
    }
    bytes.resize(written);
    return true;
}

std::unique_ptr<WorldState> world_snapshot_decode(std::span<const u8> bytes,
    const WorldState &base, WorldSnapshotInfo &info)
{
    auto record = std::make_unique<KfNetWorld>();
    const auto limits = snapshot_limits(base);
    if (kf_net_world_decode(bytes.data(), bytes.size(), &limits, record.get()) != KF_CODEC_OK ||
        record->header.floor != kf_enum_encode<int>(base.floor) ||
        record->header.variant != kf_enum_encode<u8>(base.variant)) return nullptr;
    auto world = std::make_unique<WorldState>();
    world_clone_simulation(base, *world);
    snapshot_world<true>(*world, *record);
    for (auto &member : world->party.members) {
        if (member.presence != PartyPresence::Empty && member.presence != PartyPresence::Waiting)
            player_rebind_records(*world, member.player);
    }
    info = snapshot_info(record->header);
    return world;
}
