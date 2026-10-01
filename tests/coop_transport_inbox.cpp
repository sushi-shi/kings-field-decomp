#include "../src/net/transport_internal.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <source_location>
#include <string_view>

namespace {
using namespace kf::net::detail;
void require(bool value, const char *message, std::source_location where = std::source_location::current())
{
    if (!value) { std::fprintf(stderr, "%s:%u: %s\n", where.file_name(), where.line(), message); std::abort(); }
}
std::span<const u8> bytes(std::string_view text)
{ return {reinterpret_cast<const u8 *>(text.data()), text.size()}; }
std::string text(const Event &event) { return {event.data.begin(), event.data.end()}; }
Event take(Inbox &inbox, u8 kind, u8 peer = 0)
{
    auto event = inbox.poll(); require(event && event->kind == kind && event->peer == peer, "unexpected transport event");
    return std::move(*event);
}
void empty(Inbox &inbox) { require(!inbox.poll(), "unexpected extra transport event"); }
void ready(Inbox &inbox, u8 peer, u32 generation)
{ inbox.callback(4, peer, 0, generation, {}); inbox.callback(4, peer, 1, generation, {}); }
void packet(Inbox &inbox, u8 peer, u32 generation, u8 value, u8 lane = 0)
{ inbox.callback(6, peer, lane, generation, {&value, 1}); }
void json(Inbox &inbox, const Json &message)
{ const auto wire = message.dump(); inbox.callback(1, 0, 0, 0, bytes(wire)); }

void stale_generations_and_readiness()
{
    Inbox inbox;
    inbox.identities[1].fill(0xab);
    auto old = inbox.begin(1);
    inbox.callback(4, 1, 0, old, {}); empty(inbox);
    require(!inbox.can_send(1, 0, 1), "one data channel admitted the peer");
    inbox.callback(4, 1, 1, old, {});
    packet(inbox, 1, old, 1);
    auto current = inbox.begin(1); empty(inbox);
    ready(inbox, 1, old); packet(inbox, 1, old, 2); inbox.disconnected(1, old);
    require(!inbox.can_send(1, 0, 1), "stale open callback readied replacement"); empty(inbox);
    ready(inbox, 1, current); ready(inbox, 1, current);
    auto event = take(inbox, Connected, 1);
    require(std::all_of(event.identity.begin(), event.identity.end(), [](u8 b) { return b == 0xab; }), "peer identity lost");
    require(inbox.can_send(1, 0, 1) && inbox.can_send(1, 1, KF_NET_PACKET_LIMIT), "ready channels cannot send");
    require(!inbox.can_send(1, 0, 0) && !inbox.can_send(1, 0, KF_NET_PACKET_LIMIT + 1) &&
        !inbox.can_send(1, 2, 1) && !inbox.can_send(4, 0, 1), "invalid send bounds accepted");
    inbox.disconnected(1, old); inbox.reject(1, old, "late failure"); empty(inbox);
    inbox.disconnected(1, current); ready(inbox, 1, current); packet(inbox, 1, current, 3);
    require(!inbox.can_send(1, 0, 1), "closed generation reopened"); take(inbox, Disconnected, 1); empty(inbox);
}
void close_keeps_accepted_packets()
{
    Inbox inbox; const auto generation = inbox.begin(1); ready(inbox, 1, generation);
    packet(inbox, 1, generation, 42); inbox.callback(5, 1, 0, generation, {});
    packet(inbox, 1, generation, 99); inbox.callback(5, 1, 1, generation, {});
    take(inbox, Connected, 1); require(take(inbox, Packet, 1).data == std::vector<u8>{42}, "close discarded accepted packet");
    take(inbox, Disconnected, 1); empty(inbox);
}
void rejection_isolates_peer()
{
    Inbox inbox; inbox.resume = "room-resume";
    const auto bad = inbox.begin(1), good = inbox.begin(2);
    ready(inbox, 1, bad); packet(inbox, 1, bad, 1); ready(inbox, 2, good); packet(inbox, 2, good, 2);
    inbox.reject(1, bad, "invalid peer"); ready(inbox, 1, bad); packet(inbox, 1, bad, 99);
    require(inbox.active() && !inbox.live(1, bad) && inbox.can_send(2, 0, 1) && inbox.resume == "room-resume",
        "peer rejection poisoned the room");
    take(inbox, Connected, 2); require(take(inbox, Packet, 2).data == std::vector<u8>{2}, "healthy packet lost");
    require(text(take(inbox, Disconnected, 1)) == "invalid peer", "peer rejection reason lost"); empty(inbox);
    const auto replacement = inbox.begin(1); ready(inbox, 1, replacement); inbox.reject(1, bad, "stale error");
    require(inbox.can_send(1, 0, 1), "stale error poisoned replacement"); take(inbox, Connected, 1); empty(inbox);
}
void oversized_callbacks_are_peer_local()
{
    const std::vector<u8> huge(signal_limit + 1, 0xa5);
    for (unsigned size : {0u, unsigned(KF_NET_PACKET_LIMIT + 1), unsigned(signal_limit + 1)}) {
        Inbox inbox; const auto old = inbox.begin(1), good = inbox.begin(2);
        const auto current = inbox.begin(1); ready(inbox, 1, current); ready(inbox, 2, good);
        take(inbox, Connected, 1); take(inbox, Connected, 2);
        // Oversized and invalid-lane callbacks from replaced connections must
        // be discarded before validating fields against the live room.
        inbox.callback(6, 1, 0, old, huge); inbox.callback(6, 1, 255, old, huge);
        require(inbox.active() && inbox.can_send(1, 0, 1), "stale malformed callback damaged replacement"); empty(inbox);
        packet(inbox, 2, good, 7);
        inbox.callback(6, 1, 0, current, std::span(huge).first(size));
        require(inbox.active() && !inbox.live(1, current) && inbox.can_send(2, 0, 1), "oversized packet ended healthy room");
        require(take(inbox, Packet, 2).data == std::vector<u8>{7}, "oversized peer packet discarded healthy queue");
        take(inbox, Disconnected, 1); empty(inbox);
    }
    Inbox room; room.callback(1, 0, 0, 0, huge); require(!room.active(), "oversized room message accepted"); take(room, Error); empty(room);
}
void independent_count_budgets_and_state()
{
    Inbox inbox; const auto a = inbox.begin(1), b = inbox.begin(2); ready(inbox, 1, a); ready(inbox, 2, b);
    take(inbox, Connected, 1); take(inbox, Connected, 2);
    for (unsigned value = 0; value < queue_events; ++value) { packet(inbox, 1, a, value); packet(inbox, 2, b, value); }
    for (unsigned value = 0; value < 200; ++value) packet(inbox, 2, b, value, 1);
    require(inbox.can_send(1, 0, 1) && inbox.can_send(2, 0, 1), "peers shared a reliable allowance");
    inbox.push({Room, 0, 0, 0, {}, {'r'}}); packet(inbox, 1, a, 255);
    require(inbox.active() && !inbox.live(1, a) && inbox.can_send(2, 0, 1), "reliable count overflow escaped peer");
    for (unsigned value = 0; value < queue_events; ++value) {
        auto event = take(inbox, Packet, 2); require(event.lane == 0 && event.data == std::vector<u8>{static_cast<u8>(value)}, "reliable queue order changed");
    }
    auto state = take(inbox, Packet, 2); require(state.lane == 1 && state.data == std::vector<u8>{199}, "state was not independently coalesced");
    take(inbox, Room); take(inbox, Disconnected, 1); empty(inbox);
    auto replacement = inbox.begin(1); for (unsigned i = 0; i < queue_events; ++i) packet(inbox, 1, replacement, 3);
    auto newest = inbox.begin(1); packet(inbox, 1, newest, 8); inbox.reject(1, replacement, "late overflow");
    require(inbox.live(1, newest) && take(inbox, Packet, 1).data == std::vector<u8>{8}, "replacement retained stale budget"); empty(inbox);
}
void independent_byte_budgets()
{
    Inbox inbox; const auto a = inbox.begin(1), b = inbox.begin(2); std::vector<u8> payload(KF_NET_PACKET_LIMIT, 2);
    for (unsigned i = 0; i < 34; ++i) inbox.callback(6, 2, 0, b, payload);
    for (unsigned i = 0; i < 40; ++i) inbox.callback(6, 1, 0, a, payload);
    require(inbox.active() && !inbox.live(1, a) && inbox.live(2, b), "reliable byte overflow escaped peer");
    for (unsigned i = 0; i < 34; ++i) require(take(inbox, Packet, 2).data == payload, "healthy byte allowance lost");
    take(inbox, Disconnected, 1); empty(inbox);
}
void signaling_owner_budgets_and_announcements()
{
    Inbox inbox; const auto old = inbox.begin(1); inbox.resume = "keep-token";
    json(inbox, { {"type", "peer"}, {"slot", 1} });
    for (unsigned i = 0; i < 40; ++i) json(inbox, {{"type", "signal"}, {"from", 1}, {"sdp", std::string(60000, 'a')}, {"descriptionType", "answer"}});
    json(inbox, {{"type", "signal"}, {"from", 2}, {"candidate", "healthy"}, {"mid", "0"}});
    json(inbox, {{"type", "created"}, {"room", "same-room"}});
    ready(inbox, 1, old); packet(inbox, 1, old, 7);
    require(inbox.active() && inbox.resume == "keep-token", "peer signaling flood ended room"); empty(inbox);
    auto announcement = inbox.raw(); require(announcement && announcement->announcement && announcement->owner == 1, "overflow erased peer announcement");
    auto marker = inbox.raw(); require(marker && marker->kind == 9 && marker->owner == 1 && marker->generation == 0, "missing deferred owner rejection");
    auto healthy = inbox.raw(); require(healthy && healthy->owner == 2 && healthy->kind == 1, "signaling flood erased healthy owner");
    auto room = inbox.raw(); require(room && room->owner == -1 && room->kind == 1, "signaling flood erased room control");
    require(!inbox.raw(), "signaling flood retained excess raw events");
    // A later announcement replaces both pending old signals and rejection.
    json(inbox, {{"type", "peer"}, {"slot", 1}});
    for (unsigned i = 0; i <= queue_events; ++i) json(inbox, {{"type", "signal"}, {"from", 1}, {"candidate", "x"}, {"mid", "0"}});
    json(inbox, {{"type", "peer"}, {"slot", 1}});
    auto fresh = inbox.raw(); require(fresh && fresh->announcement && fresh->owner == 1 && !inbox.raw(), "replacement retained stale signaling rejection");
    const auto current = inbox.begin(1); ready(inbox, 1, current); take(inbox, Connected, 1); empty(inbox);
}
void room_budget_and_terminal_events()
{
    Inbox room;
    for (unsigned i = 0; i <= queue_events; ++i) room.callback(0, 0, 0, 0, {});
    require(!room.active() && !room.raw(), "unbounded room signaling queue"); take(room, Error); empty(room);
    Inbox inbox; const auto generation = inbox.begin(1); ready(inbox, 1, generation); packet(inbox, 1, generation, 42);
    inbox.end("Room unavailable"); inbox.fail("late socket failure"); inbox.callback(5, 1, 0, generation, {}); inbox.callback(2, 0, 0, 0, {});
    require(!inbox.active() && !inbox.can_send(1, 0, 1), "ended room remained sendable");
    require(text(take(inbox, Ended)) == "Room unavailable", "terminal reason overwritten by late callback"); empty(inbox);
    Inbox stopped; stopped.stop(); stopped.callback(0, 0, 0, 0, {}); empty(stopped); require(!stopped.raw(), "closed transport retained callback");
}
void bounded_json()
{
    const std::string good = R"({"type":"signal","from":0,"candidate":"x","mid":"0"})";
    require(parse(bytes(good))["candidate"] == "x", "valid JSON failed");
    for (const auto &bad : {std::string("[]"), std::string("null"), std::string("{"),
            std::string(signal_limit + 1, ' '), std::string("{\"x\":") + std::string(40, '[') + "0" + std::string(40, ']') + "}"}) {
        bool rejected = false; try { (void)parse(bytes(bad)); } catch (const std::exception &) { rejected = true; }
        require(rejected, "invalid/deep/oversized signaling JSON accepted");
    }
}
}

// This target links transport_session.cpp without a network backend. Calling
// negotiation here is a test error; queue tests never open sockets or clients.
namespace kf::net::detail {
Backend *backend_open(std::shared_ptr<Inbox>, const std::string &) { std::abort(); }
void backend_close(Backend *) { std::abort(); }
bool backend_signal(Backend &, const std::string &) { std::abort(); }
bool backend_peer(Backend &, u8, u32, bool, const Json &) { std::abort(); }
void backend_drop(Backend &, u8) { std::abort(); }
bool backend_description(Backend &, u8, const Json &) { std::abort(); }
bool backend_send(Backend &, u8, u8, std::span<const u8>) { std::abort(); }
void backend_poll(Backend &) { std::abort(); }
}
int main()
{
    stale_generations_and_readiness(); close_keeps_accepted_packets(); rejection_isolates_peer();
    oversized_callbacks_are_peer_local(); independent_count_budgets_and_state(); independent_byte_budgets();
    signaling_owner_budgets_and_announcements(); room_budget_and_terminal_events(); bounded_json();
    std::puts("Transport Inbox generations, queue bounds, peer isolation, and terminal callbacks passed");
}
