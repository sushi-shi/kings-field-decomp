#ifndef KF_GAME_PARTY_RUNTIME_H
#define KF_GAME_PARTY_RUNTIME_H

#include <kf/game/world.h>

// Local continuations are deliberately outside snapshots and campaign saves.
struct PartyRuntime {
    kf::FrameTask<void> controls[party_capacity];
    kf::FrameTask<void> ambient;
    PlayerInput inputs[party_capacity] {};
    u32 disconnected_at[party_capacity] {};
    u32 tick {};
    bool wiped {};
};

inline constexpr u32 party_prediction_ticks = 4; // 200 ms at the 20 Hz simulation rate.
bool party_predict_tick(WorldState &world, PartyRuntime &runtime, u32 authoritative_tick);

// Presentation only: never changes collision, aiming or serialized state.
struct PartyViewCorrection {
    VECTOR position {};
    SVECTOR rotation {};
    unsigned remaining {};
};
void party_correct_view(PartyViewCorrection &correction, const VECTOR &before, const SVECTOR &before_rotation,
    const VECTOR &after, const SVECTOR &after_rotation);
void party_smooth_view(PartyViewCorrection &correction, VECTOR &position, SVECTOR &rotation);

struct PartyEntityPose {
    VECTOR position {};
    SVECTOR rotation {};
    u32 generation {};
    u16 kind {};
    bool active {};
};
struct PartyEntityView {
    PartyEntityPose previous;
    PartyViewCorrection correction;
};
struct PartyEffectAppearance {
    KfEffectRenderId render_id {};
    KfAnimationClip animation_clip {};
    u16 animation_phase {};
    u16 scale_x {}, scale_y {}, scale_z {};
};
struct PartyEffectVisual {
    u32 generation {}, owner_generation {}, displayed_age {};
    u8 owner_slot {};
    KfEffectKind kind {};
    KfEffectRenderId base_render_id {};
    bool confirmed {}, impact_confirmed {}, displayed {}, finished {};
    PartyEffectAppearance authoritative, displayed_appearance;
};
struct PartyPresentation {
    PartyEntityView players[party_capacity];
    PartyEntityView actors[KF_WORLD_ACTORS];
    PartyEntityView effects[KF_WORLD_EFFECTS];
    PartyEntityView objects[KF_WORLD_OBJECTS];
    PartyEntityView events[KF_WORLD_EVENTS];
    // Renderer cache only; no animation allocation or simulation pointers.
    mutable PartyEffectVisual effect_visuals[KF_WORLD_EFFECTS];
};
PartyEntityPose party_player_pose(const PartyMember &member);
PartyEntityPose party_actor_pose(const KfActor &actor);
PartyEntityPose party_effect_pose(const KfEffectRecord &effect);
PartyEntityPose party_object_pose(const KfMapObject &object);
PartyEntityPose party_event_pose(const KfMapEvent &event);
void party_capture_presentation(PartyPresentation &presentation, const WorldState &world);
void party_correct_presentation(PartyPresentation &presentation, const WorldState &world);
void party_clear_entity_corrections(PartyPresentation &presentation);
void party_confirm_effects(PartyPresentation &presentation, const WorldState &world);
void party_advance_presentation(PartyPresentation &presentation, const WorldState &world);
bool party_present_effect(PartyEffectVisual *view, const KfEffectRecord &effect, PartyEffectAppearance &appearance);
PartyEntityPose party_present_entity(const PartyEntityView *view, PartyEntityPose pose);

void party_cancel_tasks(PartyRuntime &runtime);
void party_advance_tasks(WorldState &world, PartyRuntime &runtime);
void party_settle_deaths(WorldState &world, PartyRuntime &runtime);
void party_tick(WorldState &world, PartyRuntime &runtime);
void party_set_connected(WorldState &world, PartyRuntime &runtime, u8 slot, bool connected);
bool party_resume(WorldState &world, PartyRuntime &runtime, u8 slot);
bool party_at_same_entrance(WorldState &world);
// The session owns the load barrier and must cancel player tasks before travel.
kf::FrameTask<bool> party_travel(WorldState &world);
bool party_admit(WorldState &world, u8 slot, const kf::net::Identity &character_id, bool returning);
void party_revive_spectators(WorldState &world, const PlayerContext &at);
void party_move_to_floor_entry(WorldState &world);

#endif
