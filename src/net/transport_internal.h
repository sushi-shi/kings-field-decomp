#ifndef KF_NET_TRANSPORT_INTERNAL_H
#define KF_NET_TRANSPORT_INTERNAL_H

#include <kf/net/transport.h>
#include <nlohmann/json.hpp>

#include <array>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace kf::net::detail {
using Json = nlohmann::json;
using Identity = std::array<u8, 32>;
inline constexpr std::size_t signal_limit = 65536, queue_bytes = 2 * 1024 * 1024, queue_events = 128;
enum EventKind : u8 { Room, Connected, Disconnected, Packet, HostWaiting, Ended, Error, Lobby };
struct Event {
    u8 kind {}, peer {}, lane {};
    u32 generation {};
    Identity identity {};
    std::vector<u8> data;
};
struct Raw {
    u8 kind {}, peer {};
    u32 generation {};
    int owner = -1;
    bool announcement {};
    std::vector<u8> data;
};
struct PeerState { u32 generation {}; u8 ready {}; bool closed {}; };
// Callbacks only copy into bounded queues. The application thread owns negotiation.
struct Inbox {
    mutable std::mutex mutex;
    std::deque<Event> events;
    std::deque<Raw> pending;
    std::array<PeerState, 4> peers {};
    std::array<Identity, 4> identities {};
    std::array<bool, 4> reset {};
    bool failed {}, stopped {};
    std::string resume;

    bool live(u8 peer, u32 generation) const;
    u32 generation(u8 peer) const;
    u32 begin(u8 peer);
    bool can_send(u8 peer, u8 lane, std::size_t size) const;
    void fail(std::string reason);
    void end(std::string reason);
    void reject(u8 peer, u32 generation, std::string reason);
    void disconnected(u8 peer, u32 generation);
    void push(Event event);
    void callback(u8 kind, u8 peer, u8 lane, u32 generation, std::span<const u8> data);
    std::optional<Event> poll();
    std::optional<Raw> raw();
    bool active() const;
    void stop();
private:
    bool live_locked(u8 peer, u32 generation) const;
    void fail_locked(std::string reason);
    void reject_locked(u8 peer, u32 generation, std::string reason);
    void push_locked(Event event);
};
Json parse(std::span<const u8> bytes);
struct Backend;
Backend *backend_open(std::shared_ptr<Inbox> inbox, const std::string &address);
void backend_close(Backend *backend);
bool backend_signal(Backend &, const std::string &message);
bool backend_peer(Backend &, u8 peer, u32 generation, bool initiator, const Json &ice);
void backend_drop(Backend &, u8 peer);
bool backend_description(Backend &, u8 peer, const Json &message);
bool backend_send(Backend &, u8 peer, u8 lane, std::span<const u8> bytes);
void backend_poll(Backend &);
}
#endif // KF_NET_TRANSPORT_INTERNAL_H
