#ifndef KF_GAME_WORLD_H
#define KF_GAME_WORLD_H

#include <kf/game/actor.h>
#include <kf/game/effect.h>
#include <kf/game/collision.h>
#include <kf/lib/map.h>
#include <kf/game/party.h>
#include <kf/game/story.h>
struct CampaignRuntime;
struct PartyPresentation;

// Each simulation owns its pools and mutable terrain. Runtime pointers within
// pools are local bindings, not entity identities or serializable state.
struct WorldState {
    PartyState party {};
    KfNetWorldStory story {};
    u32 epoch = 1;
    KfFloorId floor = KF_FLOOR_1;
    KfMapVariant variant = KF_MAP_VARIANT_DEFAULT;
    bool prediction {};
    kf::RandomStream random {};
    u32 sound_sequence {};
    KfNetWorldSound sounds[KF_WORLD_SOUNDS] {};
    CampaignRuntime *campaign {};
    // Local renderer binding, never cloned or serialized.
    const PartyPresentation *presentation {};
    KfActorState actors {};
    KfEffectState effects {};
    KfMapRuntimeState map {};
    KfMapObjectState objects {};
    KfMapGrid collision_flags {};
    KfMapOrientationGrid cell_orientation {};
    KfMapGrid floor_height {};
    KfMapCollisionGrid collision {};
    KfMapAttributeGrid cell_attribute {};
    KfCollisionTarget collision_target {};

    WorldState() = default;
    WorldState(const WorldState &) = delete;
    WorldState &operator=(const WorldState &) = delete;
};

s32 actor_random_next(WorldState &world);
s32 world_random_next(WorldState &world);

#endif
