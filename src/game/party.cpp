#include <kf/platform/prelude.h>
#include <kf/game/world.h>
#include <kf/game/party.h>
#include <kf/game/notify.h>

s32 world_random_next(WorldState &world)
{
    return world.party.enabled ? kf::random_next(world.random) : kf::random_next();
}

s32 player_random_next(PlayerContext &player)
{
    return player.party_slot < party_capacity ? kf::random_next(player.random) : kf::random_next();
}

void party_grant_quest_rewards(WorldState &world)
{
    if (!world.party.enabled || world.prediction) return;
    for (auto &member : world.party.members) {
        if (member.presence == PartyPresence::Empty || member.presence == PartyPresence::Waiting) continue;
        auto &player = member.player;
        bool learned = false;
        const auto magic = [&](KfEffectKind spell) {
            auto &known = player.learned_magic[kf_enum_encode<u8>(spell)];
            learned |= known != KF_MAGIC_LEARNED;
            known = KF_MAGIC_LEARNED;
        };
        for (const auto reward : {PartyReward::KeyOfTheDead, PartyReward::Healing, PartyReward::Harp,
                                  PartyReward::FireBall, PartyReward::SanctuaryMagic}) {
            const auto bit = static_cast<u32>(reward);
            if (!(world.party.quest_rewards & bit) || (member.quest_rewards & bit)) continue;
            switch (reward) {
            case PartyReward::KeyOfTheDead:
            case PartyReward::Harp: {
                const auto item = reward == PartyReward::KeyOfTheDead ? KF_ITEM_KEY_OF_THE_DEAD : KF_ITEM_HARP;
                auto &count = player.item_stock[0][kf_enum_encode<u8>(item)];
                // Keep a full stack's reward pending until there is room.
                if (count >= KF_ITEM_STACK_CAPACITY) continue;
                ++count;
                break;
            }
            case PartyReward::Healing: magic(KF_MAGIC_HEALING); break;
            case PartyReward::FireBall: magic(KF_MAGIC_FIRE_BALL); break;
            case PartyReward::SanctuaryMagic: magic(KF_MAGIC_RESIST_FIRE); magic(KF_MAGIC_BLESS); break;
            }
            member.quest_rewards |= bit;
        }
        if (learned && player.local_view) notify_enqueue(KF_NOTIFICATION_MAGIC_LEARNED);
    }
}

void party_complete_quest(WorldState &world, PartyReward reward)
{
    if (!world.party.enabled || world.prediction) return;
    world.party.quest_rewards |= static_cast<u32>(reward);
    party_grant_quest_rewards(world);
}

bool party_member_alive(const PartyMember &member)
{
    return member.presence == PartyPresence::Living && member.player.state.vitals.current_hp != 0;
}

PlayerContext &party_view_player(WorldState &world)
{
    auto &local = world.party.members[world.party.local_slot];
    if (local.presence != PartyPresence::Living)
        for (auto &candidate : world.party.members)
            if (party_member_alive(candidate)) return candidate.player;
    return local.player;
}

static std::uint64_t party_distance_squared(const VECTOR &position, const PlayerContext &player)
{
    const std::int64_t x = static_cast<std::int64_t>(position.vx) - player.state.camera_position.vx;
    const std::int64_t z = static_cast<std::int64_t>(position.vz) - player.state.camera_position.vz;
    return x * x + z * z;
}

PlayerContext *party_nearest_player(WorldState &world, const VECTOR &position)
{
    PlayerContext *nearest = nullptr;
    auto distance = std::numeric_limits<std::uint64_t>::max();
    for (auto &member : world.party.members) {
        if (!party_member_alive(member)) continue;
        const auto candidate = party_distance_squared(position, member.player);
        if (candidate < distance) {
            distance = candidate;
            nearest = &member.player;
        }
    }
    return nearest;
}

PlayerContext *party_actor_target(WorldState &world, KfActor &actor, PlayerContext &nearest)
{
    if (actor.target_player_slot < party_capacity) {
        auto &current = world.party.members[actor.target_player_slot];
        const bool attacking = actor.action == KF_ACTOR_ACTION_MELEE_ATTACK ||
            actor.action == KF_ACTOR_ACTION_JUMP_ATTACK || actor.action == KF_ACTOR_ACTION_SPECIAL_ATTACK ||
            actor.action == KF_ACTOR_ACTION_MULTI_HIT_ATTACK || actor.action == KF_ACTOR_ACTION_EFFECT0 ||
            actor.action == KF_ACTOR_ACTION_EFFECT1 || actor.action == KF_ACTOR_ACTION_EFFECT2;
        if (current.generation == actor.target_player_generation) {
            // Lock a committed attack; losing a target does not retarget its hit.
            if (attacking) return &current.player;
            const auto current_distance = party_distance_squared(actor.position, current.player);
            const auto nearest_distance = party_distance_squared(actor.position, nearest);
            if (party_member_alive(current) && current_distance <= nearest_distance + nearest_distance / 2)
                return &current.player;
        } else if (attacking) {
            actor_set_action(&actor, KF_ACTOR_ACTION_IDLE);
        }
    }
    actor.target_player_slot = nearest.party_slot;
    actor.target_player_generation = world.party.members[nearest.party_slot].generation;
    return &nearest;
}

s32 party_find_overlap(WorldState &world, s32 x, s32 y, s32 z,
                       s32 radius, s32 height, u8 ignored_slot)
{
    s32 nearest = KF_PROXIMITY_NONE;
    s32 distance = radius;
    for (u8 slot = 0; slot < party_capacity; ++slot) {
        auto &member = world.party.members[slot];
        if (slot == ignored_slot || !party_member_alive(member)) continue;
        const auto candidate = player_distance_to_point(member.player, x, y, z, distance, height);
        if (candidate == KF_PROXIMITY_NONE) continue;
        nearest = slot;
        distance = candidate;
    }
    return nearest;
}

PlayerContext &party_collision_player(WorldState &world, PlayerContext &offline_player, KfCollisionResult collision)
{
    if (!world.party.enabled) return offline_player;
    const auto slot = collision.detail;
    if (slot >= party_capacity)
        kf::host_fail("Invalid party collision target");
    return world.party.members[slot].player;
}

bool party_melee_hit(WorldState &world, PlayerContext &attacker, const VECTOR &point,
                     s32 radius, s32 height)
{
    if (!world.party.enabled) return false;
    const auto slot = party_find_overlap(world, point.vx, point.vy, point.vz,
        radius + KF_COLLISION_PLAYER_RADIUS, height, attacker.party_slot);
    if (slot == KF_PROXIMITY_NONE) return false;
    // Tenfold melee scaling makes a charged starting sword hit remove 10 of
    // a fresh character's 30 HP. Scale before truncating partial charge to HP.
    constexpr u32 friendly_melee_scale = 10;
    const auto charge = std::min<u32>(attacker.state.attack_charge_state.committed, KF_ACTOR_DAMAGE_SCALE_ONE);
    // Use the same defense, charge and status path as other incoming hits.
    // Party membership survives damage; this path awards neither XP nor loot.
    player_apply_damage(world.party.members[slot].player,
        attacker.state.cutting_attack, attacker.state.striking_attack,
        attacker.state.piercing_attack, KF_PLAYER_STATUS_NONE,
        attacker.state.holy_attack, attacker.state.fire_attack,
        static_cast<u16>(charge * KF_FIXED12_ONE * friendly_melee_scale /
                         KF_ACTOR_DAMAGE_SCALE_ONE), KF_PLAYER_DAMAGE_MULTIPLIER_ONE);
    return true;
}

void party_award_experience(WorldState &world, PlayerContext &offline_player, s16 amount)
{
    if (!world.party.enabled) {
        player_add_experience(offline_player, amount);
        return;
    }
    for (auto &member : world.party.members) {
        if (member.connected && (member.presence == PartyPresence::Living ||
                                 member.presence == PartyPresence::Spectating))
            player_add_experience(member.player, amount);
    }
}

void party_apply_radial_damage(WorldState &world, PlayerContext &offline_player,
    const VECTOR *origin, u32 radius, u16 falloff_q12,
    u16 component0, u16 component1, u16 component2, u16 component3, u16 component4,
    u16 scale_q12, u16 multiplier_tenths)
{
    if (!world.party.enabled) {
        player_apply_radial_damage(offline_player, origin, radius, falloff_q12,
            component0, component1, component2, component3, component4, scale_q12, multiplier_tenths);
        return;
    }
    for (auto &member : world.party.members) {
        if (!party_member_alive(member)) continue;
        player_apply_radial_damage(member.player, origin, radius, falloff_q12,
            component0, component1, component2, component3, component4, scale_q12, multiplier_tenths);
    }
}
