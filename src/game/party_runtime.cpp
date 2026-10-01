#include <kf/game/game.h>
#include <kf/game/party_runtime.h>

bool party_predict_tick(WorldState &world, PartyRuntime &runtime, u32 authoritative_tick)
{
    if (!world.prediction || runtime.tick < authoritative_tick ||
        runtime.tick - authoritative_tick >= party_prediction_ticks) return false;
    party_tick(world, runtime);
    return true;
}

void party_correct_view(PartyViewCorrection &correction, const VECTOR &before, const SVECTOR &before_rotation,
    const VECTOR &after, const SVECTOR &after_rotation)
{
    const auto x = std::int64_t(before.vx) + correction.position.vx - after.vx;
    const auto y = std::int64_t(before.vy) + correction.position.vy - after.vy;
    const auto z = std::int64_t(before.vz) + correction.position.vz - after.vz;
    const auto angle = [](int old, int offset, int next) {
        return ((old + offset - next + KF_ANGLE_HALF_TURN) & KF_ANGLE_WRAP_MASK) - KF_ANGLE_HALF_TURN;
    };
    const int pitch = angle(before_rotation.vx, correction.rotation.vx, after_rotation.vx);
    const int yaw = angle(before_rotation.vy, correction.rotation.vy, after_rotation.vy);
    const int roll = angle(before_rotation.vz, correction.rotation.vz, after_rotation.vz);
    correction = {};
    if (!x && !y && !z && !pitch && !yaw && !roll) return;
    // Teleports and large corrections snap immediately rather than sweeping through walls.
    constexpr int limit = KF_MAP_TILE_SIZE / 2;
    if (std::abs(x) > limit || std::abs(y) > limit || std::abs(z) > limit ||
        x*x + y*y + z*z > limit*limit || std::abs(pitch) > KF_ANGLE_EIGHTH_TURN ||
        std::abs(yaw) > KF_ANGLE_EIGHTH_TURN || std::abs(roll) > KF_ANGLE_EIGHTH_TURN) return;
    correction.position = {static_cast<s32>(x), static_cast<s32>(y), static_cast<s32>(z)};
    correction.rotation = VECTOR{pitch, yaw, roll}.narrowed();
    correction.remaining = party_prediction_ticks;
}

static void party_decay_correction(PartyViewCorrection &correction)
{
    if (!correction.remaining) return;
    const int frames = correction.remaining--;
    const int remaining = correction.remaining;
    correction.position.vx = correction.position.vx * remaining / frames;
    correction.position.vy = correction.position.vy * remaining / frames;
    correction.position.vz = correction.position.vz * remaining / frames;
    correction.rotation.vx = correction.rotation.vx * remaining / frames;
    correction.rotation.vy = correction.rotation.vy * remaining / frames;
    correction.rotation.vz = correction.rotation.vz * remaining / frames;
}

void party_smooth_view(PartyViewCorrection &correction, VECTOR &position, SVECTOR &rotation)
{
    party_decay_correction(correction);
    position += correction.position;
    rotation += correction.rotation;
}

PartyEntityPose party_player_pose(const PartyMember &member)
{
    const auto &player = member.player.state;
    return {{player.camera_position.vx, player.foot_height, player.camera_position.vz},
        player.camera_rotation, member.generation, member.avatar, party_member_alive(member)};
}

PartyEntityPose party_actor_pose(const KfActor &actor)
{
    return {actor.position, actor.rotation.vector, actor.generation, actor.definition_id,
        actor.slot_state != KF_ACTOR_SLOT_FREE && actor.lifecycle == KF_ACTOR_LIFECYCLE_ACTIVE};
}

PartyEntityPose party_effect_pose(const KfEffectRecord &effect)
{
    return {effect.position, effect.rotation.vector, effect.generation,
        static_cast<u16>(kf_enum_encode<u8>(effect.kind) | (effect.owner_player_slot << 8)),
        effect.type != KF_EFFECT_SLOT_FREE && effect.render_id.model != KF_EFFECT_MODEL_NONE};
}

PartyEntityPose party_object_pose(const KfMapObject &object)
{
    return {object.position, object.rotation.vector, object.generation, kf_enum_encode<u8>(object.object_id),
        object.object_id < KF_MAP_OBJECT_RENDER_ID_END};
}

PartyEntityPose party_event_pose(const KfMapEvent &event)
{
    return {event.reference_position, event.rotation, 0,
        static_cast<u16>(kf_enum_encode<u8>(event.character_id) | (event.model_index << 8)),
        event.state == KF_MAP_EVENT_ACTIVE};
}

static bool party_same_entity(const PartyEntityPose &a, const PartyEntityPose &b)
{
    return a.active && b.active && a.generation == b.generation && a.kind == b.kind;
}

template<class Visit>
static void party_visit_poses(PartyPresentation &presentation, const WorldState &world, Visit visit)
{
    for (u8 slot = 0; slot < party_capacity; ++slot) {
        auto pose = party_player_pose(world.party.members[slot]);
        pose.active &= slot != world.party.local_slot;
        visit(presentation.players[slot], pose);
    }
    for (unsigned i = 0; i < KF_WORLD_ACTORS; ++i) visit(presentation.actors[i], party_actor_pose(world.actors.actors[i]));
    for (unsigned i = 0; i < KF_WORLD_EFFECTS; ++i) visit(presentation.effects[i], party_effect_pose(world.effects.records[i]));
    for (unsigned i = 0; i < KF_WORLD_OBJECTS; ++i) visit(presentation.objects[i], party_object_pose(world.objects.objects[i]));
    for (unsigned i = 0; i < KF_WORLD_EVENTS; ++i) visit(presentation.events[i], party_event_pose(world.map.events[i]));
}

void party_capture_presentation(PartyPresentation &presentation, const WorldState &world)
{
    party_visit_poses(presentation, world, [](PartyEntityView &view, const PartyEntityPose &pose) {
        if (!party_same_entity(view.previous, pose)) view.correction = {};
        view.previous = pose;
    });
}

void party_correct_presentation(PartyPresentation &presentation, const WorldState &world)
{
    party_visit_poses(presentation, world, [](PartyEntityView &view, const PartyEntityPose &pose) {
        if (party_same_entity(view.previous, pose))
            party_correct_view(view.correction, view.previous.position, view.previous.rotation, pose.position, pose.rotation);
        else view.correction = {};
        view.previous = pose;
    });
}

void party_clear_entity_corrections(PartyPresentation &presentation)
{
    for (auto &view : presentation.players) view = {};
    for (auto &view : presentation.actors) view = {};
    for (auto &view : presentation.effects) view = {};
    for (auto &view : presentation.objects) view = {};
    for (auto &view : presentation.events) view = {};
}

static bool party_effect_is_impact(const KfEffectRecord &effect)
{
    switch (effect.kind) {
    case KF_EFFECT_KIND_RADIAL_BLAST:
    case KF_EFFECT_KIND_LIGHTNING_IMPACT:
    case KF_EFFECT_KIND_LIGHTNING_RADIAL_BLAST:
    case KF_EFFECT_KIND_GROUND_BRANCH_VISUAL:
        return true;
    case KF_MAGIC_FIRE_BALL:
    case KF_MAGIC_LIGHTNING_BOLT:
    case KF_MAGIC_WIND_CUTTER:
    case KF_MAGIC_LIGHT_NEEDLE:
    case KF_EFFECT_KIND_SCATTER_PROJECTILE:
    case KF_EFFECT_KIND_DARKNESS_PROJECTILE:
    case KF_EFFECT_KIND_CURSE_PROJECTILE:
    case KF_EFFECT_KIND_MAP_EMITTER_PROJECTILE:
    case KF_EFFECT_KIND_PHYSICAL_PROJECTILE:
    case KF_EFFECT_KIND_EMERGING_PROJECTILE:
        return effect.phase != KF_EFFECT_PROJECTILE_TRAVEL &&
            (effect.phase < KF_EFFECT_PROJECTILE_EMERGE_FIRST || effect.phase >= KF_EFFECT_PROJECTILE_FALL);
    default:
        return false;
    }
}

static PartyEffectAppearance party_effect_appearance(const KfEffectRecord &effect)
{
    return {effect.render_id, effect.animation_clip, effect.visual.animation_phase,
        effect.scale_x, effect.scale_y, effect.scale_z};
}

static bool party_same_effect(const PartyEffectVisual &view, const KfEffectRecord &effect)
{
    return view.confirmed && view.generation == effect.generation && view.kind == effect.kind &&
        view.owner_slot == effect.owner_player_slot && view.owner_generation == effect.owner_player_generation &&
        view.base_render_id.model == effect.base_render_id.model;
}

void party_confirm_effects(PartyPresentation &presentation, const WorldState &world)
{
    for (unsigned i = 0; i < KF_WORLD_EFFECTS; ++i) {
        auto &view = presentation.effect_visuals[i];
        const auto &effect = world.effects.records[i];
        if (effect.type == KF_EFFECT_SLOT_FREE) { view = {}; continue; }
        if (!party_same_effect(view, effect)) {
            view = {};
            view.generation = effect.generation;
            view.owner_generation = effect.owner_player_generation;
            view.owner_slot = effect.owner_player_slot;
            view.kind = effect.kind;
            view.base_render_id = effect.base_render_id;
            view.confirmed = true;
        }
        view.impact_confirmed = party_effect_is_impact(effect);
        view.authoritative = party_effect_appearance(effect);
    }
}

bool party_present_effect(PartyEffectVisual *view, const KfEffectRecord &effect, PartyEffectAppearance &appearance)
{
    if (effect.type == KF_EFFECT_SLOT_FREE) return false;
    appearance = party_effect_appearance(effect);
    if (view && party_effect_is_impact(effect)) {
        // A speculative child may occupy a different slot from the host's child.
        // Only authoritative identities may start a one-shot impact animation.
        if (!party_same_effect(*view, effect)) return false;
        if (!view->impact_confirmed) appearance = view->authoritative;
        else {
            if (view->finished) return false;
            if (!view->displayed || effect.age >= view->displayed_age) {
                view->displayed_appearance = appearance;
                view->displayed_age = effect.age;
                view->displayed = true;
            } else appearance = view->displayed_appearance;
        }
    }
    return appearance.render_id.model != KF_EFFECT_MODEL_NONE;
}

void party_advance_presentation(PartyPresentation &presentation, const WorldState &world)
{
    for (auto &view : presentation.players) party_decay_correction(view.correction);
    for (auto &view : presentation.actors) party_decay_correction(view.correction);
    for (auto &view : presentation.effects) party_decay_correction(view.correction);
    for (auto &view : presentation.objects) party_decay_correction(view.correction);
    for (auto &view : presentation.events) party_decay_correction(view.correction);
    for (unsigned i = 0; i < KF_WORLD_EFFECTS; ++i) {
        auto &view = presentation.effect_visuals[i];
        const auto &effect = world.effects.records[i];
        if (view.impact_confirmed && view.displayed &&
            (effect.type == KF_EFFECT_SLOT_FREE || !party_same_effect(view, effect))) view.finished = true;
    }
}

PartyEntityPose party_present_entity(const PartyEntityView *view, PartyEntityPose pose)
{
    if (view && party_same_entity(view->previous, pose)) {
        pose.position += view->correction.position;
        pose.rotation += view->correction.rotation;
    }
    return pose;
}

static void party_reset_motion(PlayerContext &player)
{
    player.cast_pose_ticks = 0;
    player.previous_input = 0;
    player.state.motion_state.strafe_velocity = 0;
    player.state.motion_state.forward_velocity = 0;
    player.state.motion_state.yaw_step = 0;
    player.state.motion_state.pitch_step = 0;
    player.state.weapon_attack_phase = KF_WEAPON_ATTACK_INACTIVE;
    player.state.weapon_magic_shots_remaining = 0;
}

static void party_clear_status(PlayerContext &player)
{
    player.state.status_effect_flags = KF_PLAYER_STATUS_NONE;
    player.state.curse_timer = player.state.darkness_timer = player.state.poison_timer =
        player.state.slowed_timer = player.state.fire_defense_timer = KF_PLAYER_STATUS_TIMER_INACTIVE;
    player.state.illusion_staff_timer = KF_ILLUSION_STAFF_INACTIVE;
    player.state.view_rotation_offset = {};
    player.state.view_bob_offset = 0;
}

void party_cancel_tasks(PartyRuntime &runtime)
{
    runtime.ambient = {};
    for (auto &task : runtime.controls) task = {};
    for (auto &input : runtime.inputs) input = {};
}

void party_settle_deaths(WorldState &world, PartyRuntime &runtime)
{
    bool admitted = false, living = false;
    for (u8 slot = 0; slot < party_capacity; ++slot) {
        auto &member = world.party.members[slot];
        if (member.presence == PartyPresence::Living && member.player.state.vitals.current_hp == 0) {
            runtime.controls[slot] = {};
            runtime.inputs[slot] = {};
            const auto cell = member.player.state.motion_state.map_cell;
            collision_adjust_cell_occupancy(world, cell.x, cell.z, -1);
            party_reset_motion(member.player);
            member.presence = PartyPresence::Spectating;
        }
        admitted |= member.connected && (member.presence == PartyPresence::Living ||
                                         member.presence == PartyPresence::Spectating);
        living |= party_member_alive(member);
    }
    runtime.wiped = admitted && !living;
}

void party_advance_tasks(WorldState &world, PartyRuntime &runtime)
{
    PartyAudioScope audio(world);
    if (world.story.kind) { party_cancel_tasks(runtime); return; }
    party_settle_deaths(world, runtime);
    for (auto &task : runtime.controls) task.advance();
    runtime.ambient.advance();
    party_settle_deaths(world, runtime);
}

void party_set_connected(WorldState &world, PartyRuntime &runtime, u8 slot, bool connected)
{
    if (slot >= party_capacity) return;
    auto &member = world.party.members[slot];
    if (member.connected == connected) return;
    member.connected = connected;
    runtime.disconnected_at[slot] = runtime.tick;
    runtime.inputs[slot] = {};
    runtime.controls[slot] = {};
    party_reset_motion(member.player);
}

bool party_resume(WorldState &world, PartyRuntime &runtime, u8 slot)
{
    if (slot >= party_capacity) return false;
    party_settle_deaths(world, runtime);
    auto &member = world.party.members[slot];
    if (member.presence != PartyPresence::Living && member.presence != PartyPresence::Spectating)
        return false;
    // A live body still in its grace period resumes in place. A death during
    // that period remains a death; transport recovery never revives anyone.
    party_set_connected(world, runtime, slot, true);
    return true;
}

void party_tick(WorldState &world, PartyRuntime &runtime)
{
    PartyAudioScope audio(world);
    ++runtime.tick;
    if (world.story.kind) { party_story_tick(world); return; }
    party_grant_quest_rewards(world);
    party_settle_deaths(world, runtime);
    for (u8 slot = 0; slot < party_capacity; ++slot) {
        auto &member = world.party.members[slot];
        if (!party_member_alive(member)) continue;
        if (!member.connected && runtime.tick - runtime.disconnected_at[slot] >= 100) {
            const auto cell = member.player.state.motion_state.map_cell;
            collision_adjust_cell_occupancy(world, cell.x, cell.z, -1);
            member.presence = PartyPresence::Disconnected;
            continue;
        }
        if (runtime.controls[slot].done()) {
            runtime.controls[slot] = player_update(world, member.player,
                member.connected ? runtime.inputs[slot] : PlayerInput {});
            runtime.controls[slot].advance();
        }
        // Look deltas are impulses; held buttons may span several ticks.
        runtime.inputs[slot].look = {};
        if (member.player.state.vitals.current_hp != 0)
            player_tick_effects(world, member.player);
    }
    party_settle_deaths(world, runtime);
    auto &local = world.party.members[world.party.local_slot].player;
    player_update_transform_snapshot(local, &local.presentation.position_snapshot, &local.presentation.rotation_snapshot);
    if (!world.prediction)
        party_audio_update_listener(world);
    auto actors = actor_pool_update(world, local);
    actors.advance();
    if (!actors.done()) kf::host_fail("A cooperative actor update suspended across a simulation tick");
    map_object_pool_update(world, local);
    effect_pool_update(world, local);
    map_event_pool_update(world, local);
    if (!world.prediction && runtime.ambient.done()) {
        runtime.ambient = map_ambient_scripts_update(world, local);
        runtime.ambient.advance();
    }
    for (auto &member : world.party.members)
        member.player.state.allow_near_actor_spawn = KF_ACTOR_NEAR_SPAWN_FORBIDDEN;
    party_settle_deaths(world, runtime);
}

bool party_at_same_entrance(WorldState &world)
{
    if (world.story.kind) return false;
    const PlayerContext *first = nullptr;
    for (auto &member : world.party.members) {
        if (!party_member_alive(member)) continue;
        if (!member.connected || player_current_map_attribute(world, member.player) != KF_MAP_ATTRIBUTE_WARP)
            return false;
        if (first && !map_cells_equal(first->state.motion_state.map_cell, member.player.state.motion_state.map_cell))
            return false;
        first = &member.player;
    }
    return first != nullptr;
}

kf::FrameTask<bool> party_travel(WorldState &world)
{
    if (world.prediction || !party_at_same_entrance(world)) co_return false;
    auto &host = world.party.members[0].player;
    if (!party_member_alive(world.party.members[0])) {
        for (auto &member : world.party.members) {
            if (!party_member_alive(member)) continue;
            host.state.camera_position = member.player.state.camera_position;
            host.state.camera_rotation = member.player.state.camera_rotation;
            host.state.motion_state.map_cell = member.player.state.motion_state.map_cell;
            host.state.foot_height = member.player.state.foot_height;
            break;
        }
        // The original warp routine moves the host record and removes its old
        // occupancy. A spectator needs a temporary entry to balance that removal.
        const auto cell = host.state.motion_state.map_cell;
        collision_adjust_cell_occupancy(world, cell.x, cell.z, 1);
    }
    for (u8 slot = 1; slot < party_capacity; ++slot) {
        const auto &member = world.party.members[slot];
        if (party_member_alive(member)) {
            const auto cell = member.player.state.motion_state.map_cell;
            collision_adjust_cell_occupancy(world, cell.x, cell.z, -1);
        }
    }
    if (co_await player_warp_trigger_update(world, host)) {
        // An ending leaves the map in place; undo the temporary travel occupancy.
        for (u8 slot = 0; slot < party_capacity; ++slot) {
            const auto &member = world.party.members[slot];
            const auto cell = member.player.state.motion_state.map_cell;
            if (!slot && !party_member_alive(member)) collision_adjust_cell_occupancy(world, cell.x, cell.z, -1);
            if (slot && party_member_alive(member)) collision_adjust_cell_occupancy(world, cell.x, cell.z, 1);
        }
        co_return true;
    }
    for (u8 slot = 0; slot < party_capacity; ++slot) {
        auto &member = world.party.members[slot];
        if (member.presence == PartyPresence::Empty || member.presence == PartyPresence::Waiting) continue;
        auto &player = member.player;
        player.state.progress_state.current_floor = world.floor;
        player.state.map_variant = world.variant;
        player.state.progress_state.highest_floor = std::max<KfFloorId>(player.state.progress_state.highest_floor, world.floor);
        player.state.camera_position = host.state.camera_position;
        player.state.camera_rotation = host.state.camera_rotation;
        player.state.motion_state.map_cell = host.state.motion_state.map_cell;
        player.state.previous_map_cell = host.state.motion_state.map_cell;
        party_reset_motion(player);
        if (slot && party_member_alive(member)) player_sync_position_to_map(world, player);
    }
    if (!party_member_alive(world.party.members[0])) {
        const auto cell = host.state.motion_state.map_cell;
        collision_adjust_cell_occupancy(world, cell.x, cell.z, -1);
    }
    co_return false;
}

bool party_admit(WorldState &world, u8 slot, const kf::net::Identity &character_id, bool returning)
{
    if (slot >= party_capacity || character_id == kf::net::Identity{} ||
        (world.floor == KF_FLOOR_5 && world.variant != KF_FLOOR5_ENTRY_VARIANT)) return false;
    for (u8 other = 0; other < party_capacity; ++other)
        if (other != slot && world.party.members[other].character_id == character_id) return false;
    auto &member = world.party.members[slot];
    if (member.presence == PartyPresence::Living || member.presence == PartyPresence::Spectating) return false;
    if (returning && member.character_id != character_id) return false;
    auto &player = member.player;
    if (!returning) {
        std::memset(member.loot_claims, 0, sizeof member.loot_claims);
        member.quest_rewards = 0;
        member.avatar = 41;
        player = {};
        player.random.state = world.epoch + (slot + 1u) * 2654435761u;
        player.local_view = slot == world.party.local_slot;
        player_initialize_character(world, player);
        for (unsigned magic = 0; magic < KF_MAGIC_PLAYER_COUNT; ++magic)
            player.learned_magic[magic] = world.effects.magic.entries[magic].learned;
    }
    player.party_slot = slot;
    player.local_view = slot == world.party.local_slot;
    player.prediction = false;
    player.state.progress_state.current_floor = world.floor;
    player.state.progress_state.highest_floor = std::max<KfFloorId>(player.state.progress_state.highest_floor, world.floor);
    for (const auto &other : world.party.members)
        if (other.presence != PartyPresence::Empty && other.presence != PartyPresence::Waiting)
            player.state.progress_state.highest_floor = std::max<KfFloorId>(player.state.progress_state.highest_floor,
                other.player.state.progress_state.highest_floor);
    player.state.map_variant = world.variant;
    const auto &entry = floor_entry_cells[kf_enum_encode<unsigned>(world.floor) - 1];
    player.state.camera_position.vx = entry.x * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    player.state.camera_position.vz = entry.z * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    player.state.camera_rotation = {};
    if (!returning) party_clear_status(player);
    player.state.update_state = KF_PLAYER_UPDATE_NORMAL;
    if (!returning) {
        player.state.vitals.current_hp = player.state.vitals.maximum_hp;
        player.state.vitals.current_mp = player.state.vitals.maximum_mp;
    }
    party_reset_motion(player);
    player_sync_position_to_map(world, player);
    player.state.previous_map_cell = player.state.motion_state.map_cell;
    member.character_id = character_id;
    ++member.generation;
    member.presence = player.state.vitals.current_hp ? PartyPresence::Living : PartyPresence::Spectating;
    if (member.presence == PartyPresence::Spectating)
        collision_adjust_cell_occupancy(world, player.state.motion_state.map_cell.x, player.state.motion_state.map_cell.z, -1);
    member.connected = true;
    party_grant_quest_rewards(world);
    return true;
}

void party_move_to_floor_entry(WorldState &world)
{
    if (!world.party.enabled || world.prediction) return;
    const auto &entry = floor_entry_cells[kf_enum_encode<unsigned>(world.floor) - 1];
    for (auto &member : world.party.members) {
        if (member.presence == PartyPresence::Empty || member.presence == PartyPresence::Waiting) continue;
        auto &player = member.player;
        const bool living = party_member_alive(member);
        if (living) {
            const auto cell = player.state.motion_state.map_cell;
            collision_adjust_cell_occupancy(world, cell.x, cell.z, -1);
        }
        player.state.camera_position.vx = entry.x * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
        player.state.camera_position.vz = entry.z * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
        player.state.progress_state.current_floor = world.floor;
        player.state.map_variant = world.variant;
        party_reset_motion(player);
        player_sync_position_to_map(world, player);
        player.state.previous_map_cell = player.state.motion_state.map_cell;
        if (!living) collision_adjust_cell_occupancy(world, entry.x, entry.z, -1);
    }
}

void party_revive_spectators(WorldState &world, const PlayerContext &at)
{
    for (auto &member : world.party.members) {
        if (member.presence != PartyPresence::Spectating || !member.connected) continue;
        auto &player = member.player;
        player.state.camera_position = at.state.camera_position;
        player.state.camera_rotation = at.state.camera_rotation;
        player_sync_position_to_map(world, player);
        party_clear_status(player);
        player.state.update_state = KF_PLAYER_UPDATE_NORMAL;
        player.state.vitals.current_hp = player.state.vitals.maximum_hp;
        player.state.vitals.current_mp = player.state.vitals.maximum_mp;
        party_reset_motion(player);
        member.presence = PartyPresence::Living;
    }
}
