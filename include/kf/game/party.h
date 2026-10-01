#ifndef KF_GAME_PARTY_H
#define KF_GAME_PARTY_H

#include <kf/game/player.h>
#include <kf/net/identity.hpp>

struct WorldState;
struct KfActor;
inline constexpr u8 party_capacity = 4;
inline constexpr u8 no_player = 0xff;

enum class PartyPresence : u8 { Empty, Waiting, Living, Spectating, Disconnected };
enum class PartyReward : u32 { KeyOfTheDead = 1, Healing = 2, Harp = 4, FireBall = 8, SanctuaryMagic = 16 };
inline constexpr u32 party_reward_mask = 31;

struct PartyMember {
    PlayerContext player {};
    PartyPresence presence = PartyPresence::Empty;
    kf::net::Identity character_id {};
    u32 generation {};
    u32 acknowledged_input {};
    u32 quest_rewards {};
    u8 avatar = 41;
    u8 loot_claims[KF_MAP_SAVED_FLOOR_COUNT][KF_MAP_OBJECT_CAPACITY] {};
    bool connected {};
};

struct PartyState {
    PartyMember members[party_capacity] {};
    u32 quest_rewards {};
    bool enabled {};
    u8 local_slot {};
};

bool party_member_alive(const PartyMember &member);
PlayerContext &party_view_player(WorldState &world);
void party_complete_quest(WorldState &world, PartyReward reward);
void party_grant_quest_rewards(WorldState &world);
PlayerContext *party_nearest_player(WorldState &world, const VECTOR &position);
PlayerContext *party_actor_target(WorldState &world, KfActor &actor, PlayerContext &nearest);
s32 party_find_overlap(WorldState &world, s32 x, s32 y, s32 z,
                       s32 radius, s32 height, u8 ignored_slot = no_player);
PlayerContext &party_collision_player(WorldState &world, PlayerContext &offline_player, u32 collision);
bool party_melee_hit(WorldState &world, PlayerContext &attacker, const VECTOR &point,
                     s32 radius, s32 height);
void party_award_experience(WorldState &world, PlayerContext &offline_player, s16 amount);
void party_apply_radial_damage(WorldState &world, PlayerContext &offline_player,
    const VECTOR *origin, u32 radius, u16 falloff_q12,
    u16 component0, u16 component1, u16 component2, u16 component3, u16 component4,
    u16 scale_q12, u16 multiplier_tenths);

#endif
