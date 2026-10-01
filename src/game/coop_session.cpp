#include <kf/game/game.h>
#include <kf/game/party_runtime.h>
#include <kf/game/snapshot.h>
#include <kf/game/campaign.h>
#include <kf/game/player_actions.h>
#include <kf/platform/avatars.hpp>

namespace {
using namespace kf::net;
static kf::FrameTask<void> player_choose_avatar(PlayerContext &player, u8 avatar)
{
    co_await player_request_action(player, CommandKind::ChooseAvatar, avatar);
}
struct SessionPeer {
    bool connected {}, loaded {};
    Identity identity {};
    InputInbox inputs;
    std::vector<std::vector<u8>> outgoing;
    std::size_t next_fragment {};
    u32 last_input_tick {};
    u32 full_tick {};
    std::vector<u8> reply;
};
struct Session {
    WorldState &world;
    Config config;
    std::unique_ptr<Transport, void (*)(Transport *)> transport {nullptr, transport_close};
    PartyRuntime runtime;
    PartyViewCorrection view_correction;
    PartyPresentation presentation;
    CampaignRuntime campaign;
    PlayerActions actions;
    kf::FrameTask<void> menu;
    u32 menu_previous {};
    u8 story_page {};
    PartyPresence confirmed_presence = PartyPresence::Empty;
    PlayerCommandState commands[party_capacity];
    std::uint64_t command_deadline {};
    SessionPeer peers[party_capacity];
    Transfer incoming;
    std::array<InputFrame, 64> history {};
    u32 next_input = 1;
    u32 last_snapshot_tick {};
    u32 last_sound_sequence {};
    std::uint64_t last_snapshot_time {};
    bool host {}, room_ready {}, received_world {}, ended {};
    bool admitted {}, awaiting_host {};
    unsigned reconnect_attempt {};
    std::uint64_t retry_at {}, reconnect_deadline {}, host_wait_deadline {};
    bool barrier {};
    std::uint64_t barrier_deadline {};
    u8 slot {};
    Identity identity {};
    explicit Session(WorldState &simulation) : world(simulation) {}
    ~Session() { world.presentation = nullptr; party_cancel_tasks(runtime); kf::host_set_session_running(false); }
};

static void session_cancel_menu(Session &session)
{
    session.menu = {};
    session.story_page = 0;
    session.actions.pending = false;
    session.command_deadline = 0;
}

static bool session_confirm_presence(Session &session, const WorldState &world)
{
    const auto &member = world.party.members[session.slot];
    // Called after host simulation or an accepted snapshot, never guest replay.
    if (member.presence != session.confirmed_presence) {
        if (member.presence == PartyPresence::Spectating) {
            kf::host_notice("YOU DIED - WATCHING THE PARTY\nREVIVE AT A HEALING OR SAVE POINT");
            std::fprintf(stdout,"Player %u died; spectating\n",session.slot+1);
            std::fflush(stdout);
        } else if (member.presence == PartyPresence::Living && session.confirmed_presence == PartyPresence::Spectating) {
            kf::host_notice("YOU HAVE BEEN REVIVED");
        }
        session.confirmed_presence = member.presence;
        return true;
    }
    return false;
}

static void session_world_status(const Session &session, const WorldState &world)
{
    const auto &member = world.party.members[session.slot];
    if (world.story.kind == party_story_ending)
        kf::host_online_status("Campaign completed. Synchronizing the ending with the party…");
    else if (!member.connected || (member.presence != PartyPresence::Living && member.presence != PartyPresence::Spectating))
        kf::host_online_status("Waiting for the host to reach a safe entry point.");
    else if (world.story.kind == party_story_boss && world.story.page)
        kf::host_online_status(world.story.ready_mask & (1u << session.slot)
            ? "Waiting for the party to finish reading."
            : "Finish reading, then press a button to continue with the party.");
    else if (session.confirmed_presence == PartyPresence::Spectating)
        kf::host_online_status("You died. Watching the party until revival at a healing or save point.");
    else kf::host_online_status("Connected. Click the game to capture the mouse.");
}

static bool session_queue_world(Session &session, WorldState &world, u8 peer)
{
    auto &remote = session.peers[peer];
    if (!remote.connected || remote.next_fragment < remote.outgoing.size()) return false;
    std::vector<u8> bytes;
    if (!world_snapshot_encode(world, {world.epoch, session.runtime.tick, true}, bytes))
        kf::host_fail("Cannot encode cooperative world state");
    remote.outgoing = fragment_encode({MessageKind::Fragment, world.epoch, session.runtime.tick,
                                      world.party.members[peer].generation}, bytes);
    remote.next_fragment = 0;
    remote.full_tick = session.runtime.tick;
    remote.loaded = false;
    return true;
}

static void session_flush(Session &session)
{
    for (u8 peer = 1; peer < party_capacity; ++peer) {
        auto &remote = session.peers[peer];
        if (!remote.connected) continue;
        if (!remote.reply.empty()) {
            if (transport_send(*session.transport, peer, Channel::Actions, remote.reply)) remote.reply.clear();
            else continue;
        }
        if (remote.next_fragment >= remote.outgoing.size()) continue;
        if (transport_send(*session.transport, peer, Channel::Actions, remote.outgoing[remote.next_fragment]))
            ++remote.next_fragment;
    }
}

static bool session_safe_admission(WorldState &world)
{
    if (world.story.kind) return false;
    auto &host = world.party.members[0].player;
    const auto &entry = floor_entry_cells[kf_enum_encode<unsigned>(world.floor) - 1];
    if (host.state.motion_state.map_cell.x != entry.x || host.state.motion_state.map_cell.z != entry.z) return false;
    for (const auto &actor : world.actors.actors) {
        if (actor.slot_state != KF_ACTOR_SLOT_FREE && actor.lifecycle == KF_ACTOR_LIFECYCLE_ACTIVE &&
            actor_distance_to_point(&actor, host.state.camera_position.vx, KF_COLLISION_IGNORE_HEIGHT,
                host.state.camera_position.vz, 8000, 0, 0) != KF_PROXIMITY_NONE) return false;
    }
    return true;
}

static void session_begin_barrier(Session &session, WorldState &world)
{
    session_cancel_menu(session);
    for (auto &command : session.commands) { command.event = -1; command.return_to_entry = false; }
    party_cancel_tasks(session.runtime);
    ++world.epoch;
    session.barrier = true;
    session.barrier_deadline = kf::host_clock_tick() + 900;
    for (auto &member : world.party.members) member.acknowledged_input = 0;
    for (u8 peer = 1; peer < party_capacity; ++peer) {
        auto &remote = session.peers[peer];
        remote.inputs = {};
        remote.loaded = false;
        remote.outgoing.clear();
        remote.reply.clear();
        remote.next_fragment = 0;
        if (remote.connected)
            transport_send(*session.transport, peer, Channel::Actions,
                control_encode({MessageKind::Waiting, world.epoch, session.runtime.tick, world.party.members[peer].generation}));
    }
}

static void session_publish_barrier(Session &session, WorldState &world)
{
    for (u8 peer = 1; peer < party_capacity; ++peer)
        session_queue_world(session, world, peer);
}

static void session_stop_prediction(Session &session)
{
    session.view_correction = {};
    session.presentation = {};
    session_cancel_menu(session);
    session.received_world = false;
    party_cancel_tasks(session.runtime);
    session.history = {};
    session.incoming = {};
    session.next_input = 1;
}

static void session_reconnect(Session &session, WorldState &world)
{
    if (session.config.resume_token.empty() || session.config.room.empty()) {
        session.ended = true;
        return;
    }
    const auto now = kf::host_clock_tick();
    if (!session.reconnect_deadline) {
        session.reconnect_deadline = now + 900;
        session.reconnect_attempt = 0;
        if (session.host) {
            session_begin_barrier(session, world);
            for (u8 peer = 1; peer < party_capacity; ++peer)
                party_set_connected(world, session.runtime, peer, false);
        } else session_stop_prediction(session);
        std::fprintf(stdout, "Online connection interrupted; reconnecting\n");
        std::fflush(stdout);
        kf::host_online_status("Connection interrupted. Reconnecting…");
    }
    session.room_ready = false;
    session.awaiting_host = false;
    session.retry_at = now + (60u << std::min(session.reconnect_attempt++, 2u));
}

static kf::FrameTask<void> session_menu(WorldState &world, PlayerContext &player)
{
    const auto result = static_cast<s32>(co_await menu_enter_mode(world, player, KF_MENU_MODE_ROOT));
    if (result >= 0) {
        if (result == kf_enum_encode<s32>(KF_ITEM_MIRROR_OF_TRUTH))
            co_await player_use_item(world, player, kf_enum_decode<KfObjectId>(result));
        else co_await player_request_action(player, CommandKind::UseItem, static_cast<u16>(result));
    } else if (result == kf_enum_encode<s32>(KF_MENU_RESULT_RETURN_TO_INTRO)) {
        game_next_overlay_mode = KF_OVERLAY_MODE_INTRO;
    }
}

static kf::FrameTask<void> session_npc_interaction(WorldState &world, PlayerContext &player, u16 event)
{
    if (!(co_await player_request_action(player, CommandKind::Interact, event))) co_return;
    const auto view = player.actions->interaction;
    if (view.stage) co_await talk_show_dialogue_page(kf_enum_decode<KfFloorId>(view.floor),
        view.stage, kf_enum_decode<KfCharacterId>(view.character), view.page);
    if (view.shop) {
        co_await audio_play_map_sequence(player, 2);
        co_await menu_enter_mode(world, player, KF_MENU_MODE_SHOP, kf_enum_decode<KfItemStockBank>(view.shop));
        co_await audio_play_current_map_sequence(player);
    }
    co_await player_request_action(player, CommandKind::Cancel, event);
}

static int session_activation_object(WorldState &world, PlayerContext &player)
{
    for (int index = 0; (index = player_interaction_object(world, player, index)) >= 0; ++index) {
        const auto &object = world.objects.objects[index];
        switch (world.objects.definitions.entries[kf_enum_encode<u8>(object.object_id)].behavior_type) {
        case KF_MAP_OBJECT_OP_LIFT_DOOR:
        case KF_MAP_OBJECT_OP_HINGED_DOOR:
        case KF_MAP_OBJECT_OP_HINGED_DOOR_PARTNER:
        case KF_MAP_OBJECT_OP_RESTORE_POINT:
        case KF_MAP_OBJECT_OP_SAVE_POINT:
        case KF_MAP_OBJECT_OP_SCREEN_IMAGE:
        case KF_MAP_OBJECT_OP_EFFECT_SWITCH: return index;
        case KF_MAP_OBJECT_OP_HINGED_CONTAINER:
        case KF_MAP_OBJECT_OP_ITEM_CONTAINER:
        case KF_MAP_OBJECT_OP_ITEM_PICKUP:
        case KF_MAP_OBJECT_OP_GOLD_PICKUP:
            for (u8 component = 0; component < KF_MAP_CONTAINER_ITEM_COUNT; ++component) {
                PlayerLootOffer offer;
                if (player_loot_offer(world, player, index, component, offer)) return index;
            }
            break;
        default: break;
        }
    }
    return -1;
}

static kf::FrameTask<void> session_object_activation(WorldState &world, PlayerContext &player, u16 index, KfObjectId expected)
{
    switch (world.objects.definitions.entries[kf_enum_encode<u8>(expected)].behavior_type) {
    case KF_MAP_OBJECT_OP_SAVE_POINT:
        if (co_await player_request_action(player, CommandKind::ActivateObject, index, kf_enum_encode<u16>(expected)))
            kf::host_notice("ONLY THE HOST CAN SAVE\nASK THEM TO USE THIS SAVE POINT");
        co_return;
    case KF_MAP_OBJECT_OP_HINGED_CONTAINER:
    case KF_MAP_OBJECT_OP_ITEM_CONTAINER:
    case KF_MAP_OBJECT_OP_ITEM_PICKUP:
    case KF_MAP_OBJECT_OP_GOLD_PICKUP:
        co_await player_loot_interact(world, player, index);
        co_return;
    case KF_MAP_OBJECT_OP_SCREEN_IMAGE: {
        const auto image = world.objects.objects[index].link.fields.link_id;
        if (co_await player_request_action(player, CommandKind::ActivateObject, index, kf_enum_encode<u16>(expected)))
            co_await map_show_object_image(player, expected, image);
        co_return;
    }
    default: break;
    }
    // One request starts both leaves of a paired door on the host. Do not send
    // another for its partner before the resulting snapshot reaches this peer.
    co_await player_request_action(player, CommandKind::ActivateObject, index, kf_enum_encode<u16>(expected));
}

static void session_service_action(Session &session, WorldState &world)
{
    auto &actions = session.actions;
    if (!actions.pending || actions.completed) {
        session.command_deadline = 0;
        return;
    }
    if (!session.command_deadline) session.command_deadline = kf::host_clock_tick() + 300;
    if (kf::host_clock_tick() >= session.command_deadline) {
        session_reconnect(session, world);
        return;
    }
    if (actions.sent) return;
    auto &command = actions.command;
    command.header.epoch = world.epoch;
    command.header.generation = world.party.members[session.slot].generation;
    if (session.host) {
        actions.accepted = party_apply_command(world, session.slot, session.commands[session.slot], command);
        if (actions.accepted && command.kind == CommandKind::Interact)
            actions.interaction = session.commands[session.slot].interaction;
        actions.completed = true;
        session.command_deadline = 0;
    } else {
        std::vector<u8> bytes;
        if (!command_encode(command, bytes)) {
            actions.completed = true;
            return;
        }
        if (transport_send(*session.transport, 0, Channel::Actions, bytes)) {
            actions.sent = true;
            session.command_deadline = kf::host_clock_tick() + 300;
        }
    }
}

static kf::FrameTask<void> session_boss_page(WorldState &world, u8 slot, u8 page)
{
    kf::InputContextScope input_context(kf::InputContext::Menu);
    co_await screen_show_image_until_input(page == 1 ? "TALK/C17/T55171.TIM" : "TALK/C17/T55172.TIM");
    auto &member = world.party.members[slot];
    if ((member.presence == PartyPresence::Living || member.presence == PartyPresence::Spectating) &&
        !(world.story.ready_mask & (1u << slot)))
        co_await player_request_action(member.player, CommandKind::StoryReady, page);
    kf::host_online_status("Waiting for the party to finish reading.");
    while (world.story.kind == party_story_boss && world.story.page == page) {
        display_present_system_screen(127);
        co_await game_wait_frame();
    }
}

static kf::FrameTask<void> session_restore_checkpoint(Session &session, WorldState &world)
{
    WorldSnapshotInfo info;
    if (!world_snapshot_info(session.campaign.checkpoint, info))
        kf::host_fail("Invalid campaign checkpoint");
    const auto epoch = world.epoch;
    if (info.floor != world.floor || info.variant != world.variant) {
        auto &host = world.party.members[0].player;
        animation_cache_release_all();
        host.state.progress_state.current_floor = info.floor;
        host.state.map_variant = info.variant;
        co_await map_load_floor_wrapper(world, host);
    }
    auto restored = world_snapshot_decode(session.campaign.checkpoint, world, info);
    if (!restored) kf::host_fail("Campaign checkpoint is incompatible with loaded resources");
    world_apply_snapshot(*restored, world);
    world.epoch = epoch;
    world.prediction = false;
    for (u8 slot = 0; slot < party_capacity; ++slot) {
        auto &member = world.party.members[slot];
        member.player.prediction = false;
        member.acknowledged_input = 0;
        member.connected = slot == 0 || session.peers[slot].connected;
        if (member.presence == PartyPresence::Empty && member.connected)
            member.presence = PartyPresence::Waiting;
        else if (!member.connected && member.presence == PartyPresence::Living) {
            const auto cell = member.player.state.motion_state.map_cell;
            collision_adjust_cell_occupancy(world, cell.x, cell.z, -1);
            member.presence = PartyPresence::Disconnected;
        }
    }
    player_refresh_weapon_asset(world.party.members[0].player);
    session.runtime.wiped = false;
}

static kf::FrameTask<void> session_travel(Session &session, WorldState &world)
{
    session_begin_barrier(session, world);
    if (co_await party_travel(world)) {
        if (!party_ending_begin(world)) kf::host_fail("Invalid cooperative ending state");
        session_world_status(session, world);
    }
    session_publish_barrier(session, world);
}

static u8 session_pending_return(Session &session, const WorldState &world)
{
    for (u8 slot = 0; slot < party_capacity; ++slot) {
        if (!session.commands[slot].return_to_entry) continue;
        session.commands[slot].return_to_entry = false;
        const auto &member = world.party.members[slot];
        if (member.connected && party_member_alive(member) && member.player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_GREEN_DRAGON_STAFF)])
            return slot;
    }
    return no_player;
}

static kf::FrameTask<void> session_return_to_entry(Session &session, WorldState &world, u8 initiator)
{
    session_begin_barrier(session, world);
    auto &host = world.party.members[0].player;
    VECTOR position;
    player_get_floor_position(world.party.members[initiator].player, position);
    co_await player_warp_shimmer(world, host, KF_WARP_SHIMMER_GROW_REMOVE, &position);
    if (world.floor == KF_FLOOR_5 && world.variant != KF_FLOOR5_ENTRY_VARIANT) {
        if (world.variant == KF_FLOOR5_ALTERNATE_MUSIC_VARIANT)
            co_await audio_play_current_map_sequence(host);
        animation_cache_release_all();
        host.state.map_variant = KF_FLOOR5_ENTRY_VARIANT;
        map_variant_assets_load(world, host);
    }
    party_move_to_floor_entry(world);
    player_get_floor_position(host, position);
    co_await player_warp_shimmer(world, host, KF_WARP_SHIMMER_SHRINK_REMOVE, &position);
    session_publish_barrier(session, world);
}

static void session_host_packet(Session &session, WorldState &world, const Event &event)
{
    if (!event.peer || event.peer >= party_capacity) return;
    auto &remote = session.peers[event.peer];
    auto &member = world.party.members[event.peer];
    MessageHeader header;
    if (!packet_header(event.packet, header) || header.epoch != world.epoch || header.generation != member.generation) return;
    if (header.kind == MessageKind::Loaded && event.channel == Channel::Actions && event.packet.size() == message_header_bytes) {
        if (remote.next_fragment == remote.outgoing.size() && header.sequence == remote.full_tick) {
            remote.loaded = true;
            if (world.story.kind == party_story_ending) world.story.ready_mask |= 1u << event.peer;
            if (!member.connected && party_resume(world, session.runtime, event.peer)) {
                std::fprintf(stdout, "Player %u reconnected\n", event.peer + 1);
                std::fflush(stdout);
            }
        }
    } else if (header.kind == MessageKind::Resync && event.channel == Channel::Actions) {
        session_queue_world(session, world, event.peer);
    } else if (header.kind == MessageKind::Input && event.channel == Channel::State && remote.loaded && member.connected) {
        InputBundle bundle;
        if (input_decode(event.packet, bundle) && remote.inputs.push(bundle))
            remote.last_input_tick = session.runtime.tick;
    } else if (header.kind == MessageKind::Command && event.channel == Channel::Actions && remote.reply.empty()) {
        Command command;
        if (!command_decode(event.packet, command)) return;
        const bool accepted = remote.loaded && !session.barrier &&
            party_apply_command(world, event.peer, session.commands[event.peer], command);
        if (accepted && command.kind == CommandKind::Interact) {
            if (!interaction_encode(session.commands[event.peer].interaction, remote.reply))
                kf::host_fail("Cannot encode interaction response");
        } else remote.reply = control_encode({accepted ? MessageKind::Accepted : MessageKind::Rejected,
            world.epoch, command.header.sequence, member.generation});
    }
}

static void session_send_snapshot(Session &session, WorldState &world)
{
    std::vector<u8> bytes;
    if (!world_snapshot_encode(world, {world.epoch, session.runtime.tick, false}, bytes))
        kf::host_fail("Cannot encode cooperative snapshot");
    for (u8 peer = 1; peer < party_capacity; ++peer) {
        auto &remote = session.peers[peer];
        if (!remote.connected || !remote.loaded) continue;
        auto packet = snapshot_packet({MessageKind::Snapshot, world.epoch, session.runtime.tick,
                                       world.party.members[peer].generation}, bytes);
        if (packet.size() <= maximum_packet_bytes)
            transport_send(*session.transport, peer, Channel::State, packet);
        else
            session_queue_world(session, world, peer);
    }
}

static void session_local_input(Session &session, WorldState &world)
{
    if (world.story.kind) {
        session.runtime.inputs[session.slot] = {};
        session.menu_previous = kf::host_read_buttons();
        kf::host_take_look();
        return;
    }
    PlayerInput input {kf::host_read_buttons(), kf::host_take_look()};
    auto &local = world.party.members[session.slot];
    const auto previous = session.menu_previous;
    session.menu_previous = input.buttons;
    if (session.menu.done() && kf::host_input_context() == kf::InputContext::Gameplay &&
        (session.host || session.received_world) && party_member_alive(local) &&
        local.player.state.weapon_attack_phase == KF_WEAPON_ATTACK_INACTIVE) {
        if ((input.buttons & (kf::Button::Start | kf::Button::Back)) &&
            !(previous & (kf::Button::Start | kf::Button::Back)))
            session.menu = session_menu(world, local.player);
        else if (kf::button_pressed(input.buttons, previous, kf::Button::Confirm)) {
            const int event = player_interaction_event(world, local.player);
            if (event >= 0) session.menu = session_npc_interaction(world, local.player, static_cast<u16>(event));
            else if (!session.host) {
                const int object = session_activation_object(world, local.player);
                if (object >= 0)
                    session.menu = session_object_activation(world, local.player, object, world.objects.objects[object].object_id);
            }
        }
        session.menu.advance();
    }
    if (!session.menu.done() || kf::host_input_context() != kf::InputContext::Gameplay) input = {};
    if (session.host) {
        session.runtime.inputs[0] = input;
        return;
    }
    const auto &member = world.party.members[session.slot];
    if (!session.received_world || member.presence != PartyPresence::Living) return;
    InputFrame frame {session.next_input++, session.runtime.tick, input.buttons,
        std::clamp(input.look.yaw, -1024, 1024), std::clamp(input.look.pitch, -1024, 1024)};
    session.history[frame.sequence % session.history.size()] = frame;
    InputBundle bundle;
    bundle.header = {MessageKind::Input, world.epoch, frame.sequence, member.generation};
    const auto first = frame.sequence > 3 ? frame.sequence - 3 : 1;
    for (u32 sequence = first; sequence <= frame.sequence; ++sequence)
        bundle.frames[bundle.count++] = session.history[sequence % session.history.size()];
    std::vector<u8> packet;
    if (input_encode(bundle, packet)) transport_send(*session.transport, 0, Channel::State, packet);
    session.runtime.inputs[session.slot] = {frame.buttons, {frame.yaw, frame.pitch}};
}

static void session_predict_pending(Session &session, WorldState &world)
{
    const auto ack = world.party.members[session.slot].acknowledged_input;
    // Reconciliation uses the same movement and enemy update functions, with
    // prediction-side damage/progression disabled. Never extrapolate beyond 200ms.
    const auto first = std::max(ack + 1, session.next_input > 4 ? session.next_input - 4 : 1);
    for (u32 sequence = first; sequence < session.next_input; ++sequence) {
        const auto &frame = session.history[sequence % session.history.size()];
        if (frame.sequence != sequence) continue;
        session.runtime.inputs[session.slot] = {frame.buttons, {frame.yaw, frame.pitch}};
        if (!party_predict_tick(world, session.runtime, session.last_snapshot_tick)) break;
    }
}
}

kf::FrameTask<void> coop_game_loop(WorldState &world, PlayerContext &offline)
{
    Session session(world);
    session.host = kf::net::application_config.host;
    if (!session.host) world.presentation = &session.presentation;
    session.config = kf::net::application_config;
    kf::host_set_session_running(true);
    world.party.enabled = true;
    world.party.local_slot = 0;
    if (session.host) {
        auto &member = world.party.members[0];
        member.player = offline;
        member.player.party_slot = 0;
        member.presence = PartyPresence::Living;
        member.connected = true;
        member.generation = 1;
        if (session.config.avatar != 0xff) member.avatar = session.config.avatar;
        world.campaign = &session.campaign;
        if (kf::net::application_config.campaign_slot) {
            if (!campaign_read(static_cast<kf::SaveSlot>(kf::net::application_config.campaign_slot), session.campaign.checkpoint))
                kf::host_fail("Cannot read the selected campaign save");
            co_await session_restore_checkpoint(session, world);
        }
        for (u8 slot = 0; slot < party_capacity; ++slot)
            session.config.roster[slot] = world.party.members[slot].character_id;
    }
    session.transport.reset(kf::net::transport_open(session.config));
    if (!session.transport) kf::host_fail("Cannot open online room transport");
    std::uint64_t next_tick = kf::host_clock_tick();
    while (!session.ended) {
        Event event;
        while (session.transport && transport_poll(*session.transport, event)) {
            if (event.kind == EventKind::Room) {
                if (session.admitted && (event.peer != session.slot || event.text != session.config.room || event.identity != session.identity))
                    kf::host_fail("Reconnect changed the room or character slot");
                session.identity = event.identity;
                if (session.host) {
                    auto &member = world.party.members[0];
                    if (member.character_id != Identity{} && member.character_id != event.identity)
                        kf::host_fail("Campaign belongs to a different multiplayer profile");
                    member.character_id = event.identity;
                    session.config.roster[0] = event.identity;
                    if (session.campaign.checkpoint.empty() && !campaign_capture(world, 0, session.campaign.checkpoint))
                        kf::host_fail("Cannot create the campaign-start checkpoint");
                }
                session.config.room = event.text;
                session.config.resume_token = transport_resume_token(*session.transport);
                if (!session.host && !session.reconnect_deadline)
                    session.reconnect_deadline = kf::host_clock_tick() + 1800;
                session.slot = event.peer;
                session.room_ready = true;
                session.retry_at = 0;
                world.party.local_slot = session.slot;
                if (!session.host && !session.admitted) {
                    world.party.members[session.slot].player = offline;
                    world.party.members[session.slot].player.party_slot = session.slot;
                    world.party.members[session.slot].presence = PartyPresence::Waiting;
                }
                session.admitted = true;
                world.party.members[session.slot].player.actions = &session.actions;
                if (session.host) {
                    if (session.reconnect_deadline) {
                        std::fprintf(stdout, "Online room connection restored; synchronizing party\n");
                        session.reconnect_deadline = 0;
                        session.barrier_deadline = kf::host_clock_tick() + 900;
                    }
                    next_tick = kf::host_clock_tick() + 3;
                }
                std::fprintf(stdout, "Online room: %s; player %u\n", event.text.c_str(), session.slot + 1);
                std::fflush(stdout);
                kf::host_online_room(event.text.c_str(), session.host);
                if (session.host && session.barrier)
                    kf::host_online_status("Reconnecting the party…");
            } else if (event.kind == EventKind::Connected && session.host) {
                if (!event.peer || event.peer >= party_capacity) continue;
                auto &peer = session.peers[event.peer];
                peer = {};
                peer.connected = true;
                peer.identity = event.identity;
                auto &member = world.party.members[event.peer];
                if (event.identity == Identity{} || (member.character_id != Identity{} && member.character_id != event.identity))
                    kf::host_fail("Room service assigned a different player to a saved character");
                member.character_id = event.identity;
                party_set_connected(world, session.runtime, event.peer, false);
                peer.inputs.state.received = peer.inputs.state.consumed = member.acknowledged_input;
                if (member.presence == PartyPresence::Empty) member.presence = PartyPresence::Waiting;
                session_queue_world(session, world, event.peer);
            } else if (event.kind == EventKind::Disconnected && session.host) {
                if (!event.peer || event.peer >= party_capacity) continue;
                session.peers[event.peer] = {};
                session.commands[event.peer].event = -1;
                session.commands[event.peer].return_to_entry = false;
                party_set_connected(world, session.runtime, event.peer, false);
            } else if (event.kind == EventKind::HostWaiting && !session.host) {
                session.awaiting_host = true;
                session.host_wait_deadline = kf::host_clock_tick() + 900;
                session_stop_prediction(session);
                std::fprintf(stdout, "Host connection interrupted; waiting for resynchronization\n");
                std::fflush(stdout);
                kf::host_online_status("The host is reconnecting. Please wait…");
            } else if (event.kind == EventKind::Disconnected && !session.host && !session.awaiting_host) {
                session_reconnect(session, world);
                break;
            } else if (event.kind == EventKind::Packet && session.host) {
                session_host_packet(session, world, event);
            } else if (event.kind == EventKind::Packet && !session.host && event.peer == 0) {
                MessageHeader header;
                if (!packet_header(event.packet, header)) continue;
                if (header.kind == MessageKind::Interaction && event.channel == Channel::Actions) {
                    auto &actions = session.actions;
                    KfNetInteraction view;
                    if (actions.pending && actions.sent && actions.command.kind == CommandKind::Interact &&
                        header.epoch == actions.command.header.epoch && header.generation == actions.command.header.generation &&
                        header.sequence == actions.command.header.sequence && interaction_decode(event.packet, view)) {
                        actions.interaction = view;
                        actions.accepted = actions.completed = true;
                        session.command_deadline = 0;
                    }
                    continue;
                }
                if ((header.kind == MessageKind::Accepted || header.kind == MessageKind::Rejected) &&
                    event.channel == Channel::Actions && event.packet.size() == message_header_bytes) {
                    auto &actions = session.actions;
                    if (actions.pending && actions.sent && header.epoch == actions.command.header.epoch &&
                        header.generation == actions.command.header.generation && header.sequence == actions.command.header.sequence) {
                        actions.accepted = header.kind == MessageKind::Accepted;
                        actions.completed = true;
                        session.command_deadline = 0;
                    }
                    continue;
                }
                if (header.kind == MessageKind::Waiting && event.channel == Channel::Actions && header.epoch >= world.epoch) {
                    session_stop_prediction(session);
                    continue;
                }
                std::span<const u8> bytes;
                if (header.kind == MessageKind::Fragment && event.channel == Channel::Actions) {
                    if (!fragment_accept(session.incoming, event.packet) || !session.incoming.complete) continue;
                    bytes = session.incoming.bytes;
                } else if (header.kind == MessageKind::Snapshot && event.channel == Channel::State) {
                    if (!session.received_world || header.epoch != world.epoch || header.sequence <= session.last_snapshot_tick) continue;
                    bytes = std::span<const u8>(event.packet).subspan(message_header_bytes);
                } else continue;
                WorldSnapshotInfo info;
                if (!world_snapshot_info(bytes, info) || info.epoch != header.epoch || info.tick != header.sequence) continue;
                if (info.epoch < world.epoch || (info.epoch == world.epoch && session.received_world && info.tick < session.last_snapshot_tick)) continue;
                party_cancel_tasks(session.runtime);
                if (info.floor != world.floor || info.variant != world.variant) {
                    if (!info.full) continue;
                    session.presentation = {};
                    session.view_correction = {};
                    session_cancel_menu(session);
                    animation_cache_release_all();
                    auto &local = world.party.members[session.slot].player;
                    local.state.progress_state.current_floor = info.floor;
                    local.state.map_variant = info.variant;
                    co_await map_load_floor_wrapper(world, local);
                }
                auto restored = world_snapshot_decode(bytes, world, info);
                if (!restored) { std::fprintf(stderr, "Rejected invalid world snapshot\n"); continue; }
                const auto restored_id = restored->party.members[session.slot].character_id;
                if (restored_id != Identity{} && restored_id != session.identity) {
                    std::fprintf(stderr, "Rejected snapshot for a different character\n");
                    continue;
                }
                const auto old_generation = world.party.members[session.slot].generation;
                const auto old_epoch = world.epoch;
                const auto old_weapon = world.party.members[session.slot].player.state.equipped_weapon_id;
                const auto old_rewards = world.party.members[session.slot].quest_rewards;
                const auto old_position = world.party.members[session.slot].player.state.camera_position;
                const auto old_rotation = world.party.members[session.slot].player.state.camera_rotation;
                const bool smooth_entities = session.received_world && !info.full && old_epoch == info.epoch &&
                    old_generation == restored->party.members[session.slot].generation && !restored->story.kind;
                if (smooth_entities) party_capture_presentation(session.presentation, world);
                else party_clear_entity_corrections(session.presentation);
                if (!session.received_world || old_epoch != info.epoch ||
                    old_generation != restored->party.members[session.slot].generation) session.presentation = {};
                party_confirm_effects(session.presentation, *restored);
                const bool smooth = session.received_world && !info.full && old_epoch == info.epoch &&
                    old_generation == restored->party.members[session.slot].generation && !restored->story.kind &&
                    party_member_alive(world.party.members[session.slot]) && party_member_alive(restored->party.members[session.slot]);
                world_apply_snapshot(*restored, world);
                party_audio_receive(world, session.last_sound_sequence,
                    !session.received_world || old_epoch != world.epoch ||
                    old_generation != world.party.members[session.slot].generation);
                constexpr auto spell_rewards = static_cast<u32>(PartyReward::Healing) |
                    static_cast<u32>(PartyReward::FireBall) | static_cast<u32>(PartyReward::SanctuaryMagic);
                if (session.received_world && old_generation == world.party.members[session.slot].generation && old_epoch == world.epoch &&
                    (world.party.members[session.slot].quest_rewards & ~old_rewards & spell_rewards))
                    notify_enqueue(KF_NOTIFICATION_MAGIC_LEARNED);
                if (old_generation != world.party.members[session.slot].generation || old_epoch != world.epoch)
                    session_cancel_menu(session);
                session.runtime.tick = info.tick;
                session.last_snapshot_tick = info.tick;
                session.last_snapshot_time = kf::host_clock_tick();
                session.received_world = true;
                session.awaiting_host = false;
                session.host_wait_deadline = 0;
                session.reconnect_deadline = 0;
                session_confirm_presence(session, world);
                session_world_status(session, world);
                if (info.full || old_generation != world.party.members[session.slot].generation || old_epoch != world.epoch) {
                    session.history = {};
                    session.next_input = world.party.members[session.slot].acknowledged_input + 1;
                }
                if (old_weapon != world.party.members[session.slot].player.state.equipped_weapon_id)
                    player_refresh_weapon_asset(world.party.members[session.slot].player);
                session_predict_pending(session, world);
                if (smooth_entities) party_correct_presentation(session.presentation, world);
                if (smooth) {
                    const auto &state = world.party.members[session.slot].player.state;
                    party_correct_view(session.view_correction, old_position, old_rotation, state.camera_position, state.camera_rotation);
                } else session.view_correction = {};
                if (info.full) {
                    transport_send(*session.transport, 0, Channel::Actions,
                        control_encode({MessageKind::Loaded, world.epoch, info.tick, world.party.members[session.slot].generation}));
                    std::fprintf(stdout, "World synchronized: epoch %u, tick %u, floor %u\n", info.epoch, info.tick, kf_enum_encode<unsigned>(info.floor));
                    std::fflush(stdout);
                }
            } else if (event.kind == EventKind::Error) {
                std::fprintf(stderr, "Online transport: %s\n", event.text.c_str());
                session_reconnect(session, world);
                break;
            } else if (event.kind == EventKind::Ended) {
                std::fprintf(stderr, "%s\n", event.text.empty() ? "The host closed the room." : event.text.c_str());
                session.ended = true;
            }
        }
        const auto now = kf::host_clock_tick();
        if (session.ended) break;
        if ((session.reconnect_deadline && now >= session.reconnect_deadline) ||
            (session.awaiting_host && now >= session.host_wait_deadline)) {
            std::fprintf(stderr, "Online session ended: reconnect deadline exceeded\n");
            session.ended = true;
            break;
        }
        if (session.retry_at) {
            session.transport.reset();
            if (now >= session.retry_at) {
                session.retry_at = 0;
                session.transport.reset(transport_open(session.config));
                if (!session.transport) session_reconnect(session, world);
            }
        }
        campaign_poll(world);
        if (session.host && session.room_ready && now > next_tick + 12) {
            session_begin_barrier(session, world);
            session_publish_barrier(session, world);
            next_tick = now;
        }
        if (session.barrier && session.room_ready) {
            bool waiting = false;
            for (u8 peer = 1; peer < party_capacity; ++peer) {
                auto &remote = session.peers[peer];
                if (!remote.connected || remote.loaded) continue;
                if (now >= session.barrier_deadline) {
                    remote.connected = false;
                    party_set_connected(world, session.runtime, peer, false);
                } else waiting = true;
            }
            // Preserve the reconnect opportunity for campaign members whose
            // connection broke before they received the terminal snapshot.
            if (world.story.kind == party_story_ending && now < session.barrier_deadline)
                waiting |= party_ending_waiting(world);
            session.barrier = waiting;
            if (!waiting) session_world_status(session, world);
            next_tick = now + 3;
        }
        if (session.host && session.room_ready && !session.barrier && world.story.kind == party_story_ending) break;
        if (session.room_ready && !session.barrier && !session.awaiting_host) {
            if (session.host || (session.received_world && now - session.last_snapshot_time <= 12 &&
                    session.runtime.tick - session.last_snapshot_tick < party_prediction_ticks))
                party_advance_tasks(world, session.runtime);
            for (u8 slot = 0; slot < party_capacity; ++slot)
                if (!party_member_alive(world.party.members[slot]) || !world.party.members[slot].connected) {
                    session.commands[slot].event = -1;
                    session.commands[slot].return_to_entry = false;
                }
            if (world.story.kind == party_story_boss && world.story.page) {
                if (session.story_page != world.story.page || session.menu.done()) {
                    session_cancel_menu(session);
                    session.story_page = world.story.page;
                    session.menu = session_boss_page(world, session.slot, world.story.page);
                    session_world_status(session, world);
                }
                session_service_action(session, world);
                session.menu.advance();
            }
            else if (world.story.kind || !party_member_alive(world.party.members[session.slot])) session_cancel_menu(session);
            else {
                if (session.story_page) {
                    session_cancel_menu(session);
                    session_world_status(session, world);
                }
                if (session.menu.done() && world.party.members[session.slot].avatar != session.config.avatar &&
                    kf::avatar_mesh(session.config.avatar))
                    session.menu = player_choose_avatar(world.party.members[session.slot].player, session.config.avatar);
                session_service_action(session, world);
                session.menu.advance();
            }
        }
        if (session.room_ready && !session.barrier && now >= next_tick) {
            next_tick = now + 3;
            session_local_input(session, world);
            if (session.host) {
                if (session_safe_admission(world)) {
                    for (u8 peer = 1; peer < party_capacity; ++peer) {
                        auto &member = world.party.members[peer];
                        const bool returning = member.presence == PartyPresence::Disconnected;
                        if (session.peers[peer].connected && session.peers[peer].loaded &&
                            (member.presence == PartyPresence::Waiting || member.presence == PartyPresence::Disconnected) &&
                            party_admit(world, peer, session.peers[peer].identity, returning)) {
                            session_queue_world(session, world, peer);
                            std::fprintf(stdout, "Player %u %s\n", peer + 1, returning ? "returned" : "admitted");
                            std::fflush(stdout);
                        }
                    }
                }
                for (u8 peer = 1; peer < party_capacity; ++peer) {
                    auto &remote = session.peers[peer];
                    InputFrame input;
                    if (remote.inputs.pop(input)) {
                        session.runtime.inputs[peer] = {input.buttons, {input.yaw, input.pitch}};
                        world.party.members[peer].acknowledged_input = input.sequence;
                    } else if (session.runtime.tick - remote.last_input_tick > 4) {
                        session.runtime.inputs[peer] = {};
                    }
                }
                party_tick(world, session.runtime);
                session.campaign.tick = session.runtime.tick;
                const auto returning = session.campaign.writing || world.story.kind ? no_player : session_pending_return(session, world);
                if (session.runtime.wiped && !session.campaign.writing) {
                    session_begin_barrier(session, world);
                    co_await session_restore_checkpoint(session, world);
                    session_publish_barrier(session, world);
                    next_tick = kf::host_clock_tick() + 3;
                } else if (returning != no_player) {
                    co_await session_return_to_entry(session, world, returning);
                    next_tick = kf::host_clock_tick() + 3;
                } else if (party_at_same_entrance(world)) {
                    bool entered = false;
                    for (const auto &member : world.party.members)
                        entered |= party_member_alive(member) && !map_cells_equal(member.player.state.previous_map_cell, member.player.state.motion_state.map_cell);
                    if (entered) {
                        co_await session_travel(session, world);
                        next_tick = kf::host_clock_tick() + 3;
                    }
                }
                for (auto &member : world.party.members) {
                    if (party_member_alive(member) && player_current_map_attribute(world, member.player) != KF_MAP_ATTRIBUTE_WARP)
                        member.player.state.previous_map_cell = {KF_MAP_CELL_COORD_INVALID, KF_MAP_CELL_COORD_INVALID};
                }
                if (session_confirm_presence(session, world)) session_world_status(session, world);
                session_send_snapshot(session, world);
            } else if (session.received_world && now - session.last_snapshot_time <= 12) {
                party_predict_tick(world, session.runtime, session.last_snapshot_tick);
            }
            auto &local = world.party.members[session.slot].player;
            if (!session.host) party_advance_presentation(session.presentation, world);
            if ((session.host || session.received_world) && kf::host_input_context() == kf::InputContext::Gameplay) {
                PlayerContext *view = &party_view_player(world);
                player_update_transform_snapshot(*view, &view->presentation.position_snapshot, &view->presentation.rotation_snapshot);
                if (!session.host && view == &local && !world.story.kind)
                    party_smooth_view(session.view_correction, view->presentation.position_snapshot, view->presentation.rotation_snapshot);
                else if (!session.host && !world.story.kind) {
                    const auto raw = party_player_pose(world.party.members[view->party_slot]);
                    const auto shown = party_present_entity(&session.presentation.players[view->party_slot], raw);
                    view->presentation.position_snapshot += VECTOR {shown.position.vx - raw.position.vx,
                        shown.position.vy - raw.position.vy, shown.position.vz - raw.position.vz};
                    view->presentation.rotation_snapshot += SVECTOR {static_cast<s16>(shown.rotation.vx - raw.rotation.vx),
                        static_cast<s16>(shown.rotation.vy - raw.rotation.vy), static_cast<s16>(shown.rotation.vz - raw.rotation.vz)};
                }
                notify_effect_update();
                if (world.story.kind) party_story_render(world, *view);
                else render_world_frame(world, *view, &view->presentation.position_snapshot, &view->presentation.rotation_snapshot);
                presentation_advance_floor_items();
            }
        }
        if (session.host && session.transport && session.room_ready) session_flush(session);
        if (game_next_overlay_mode != KF_OVERLAY_MODE_NONE) break;
        co_await kf::FrameDelay {1};
    }
    world.campaign = nullptr;
    session_cancel_menu(session);
    for (auto &member : world.party.members) member.player.actions = nullptr;
    // A validated terminal snapshot is final even if signaling or the host
    // disappears after delivery. Ordinary room closure still returns to lobby.
    if (world.story.kind == party_story_ending) {
        game_next_overlay_mode = KF_OVERLAY_MODE_ENDING;
        std::fprintf(stdout, "Campaign completed; entering the ending\n");
        std::fflush(stdout);
    }
}
