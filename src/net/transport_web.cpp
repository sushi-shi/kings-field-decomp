#include "transport_internal.h"
#include <map>

extern "C" {
int kf_browser_open(u32, const char *);
void kf_browser_close(u32);
int kf_browser_signal(u32, const char *);
int kf_browser_peer(u32, u8, u32, int, const char *);
void kf_browser_drop_peer(u32, u8);
int kf_browser_description(u32, u8, const char *);
int kf_browser_send(u32, u8, u8, const u8 *, std::size_t);
}
namespace kf::net::detail {
struct Backend { u32 id; std::shared_ptr<Inbox> inbox; };
static std::map<u32, std::weak_ptr<Inbox>> sessions;
static u32 next_id = 1;
Backend *backend_open(std::shared_ptr<Inbox> inbox, const std::string &address)
{
    if (next_id == UINT32_MAX) return nullptr;
    const auto id = next_id++; sessions[id] = inbox;
    if (!kf_browser_open(id, address.c_str())) { sessions.erase(id); return nullptr; }
    return new Backend {id, std::move(inbox)};
}
void backend_close(Backend *backend)
{ if (backend) { sessions.erase(backend->id); kf_browser_close(backend->id); delete backend; } }
bool backend_signal(Backend &backend, const std::string &message)
{ return kf_browser_signal(backend.id, message.c_str()); }
bool backend_peer(Backend &backend, u8 peer, u32 generation, bool initiator, const Json &ice)
{ return kf_browser_peer(backend.id, peer, generation, initiator, ice.dump().c_str()); }
void backend_drop(Backend &backend, u8 peer) { kf_browser_drop_peer(backend.id, peer); }
bool backend_description(Backend &backend, u8 peer, const Json &message)
{ return kf_browser_description(backend.id, peer, message.dump().c_str()); }
bool backend_send(Backend &backend, u8 peer, u8 lane, std::span<const u8> bytes)
{ return kf_browser_send(backend.id, peer, lane, bytes.data(), bytes.size()); }
void backend_poll(Backend &) {}
}
void kf_net_browser_event(u32 handle, u8 kind, u8 peer, u8 lane, u32 generation, const u8 *data, std::size_t size)
{
    using namespace kf::net::detail;
    const auto found = sessions.find(handle); if (found == sessions.end()) return;
    if (auto inbox = found->second.lock()) {
        if (!data && size) {
            if (generation) inbox->reject(peer, generation, "Invalid browser event");
            else inbox->fail("Invalid browser event");
        }
        else inbox->callback(kind, peer, lane, generation, {data, size});
    }
}
