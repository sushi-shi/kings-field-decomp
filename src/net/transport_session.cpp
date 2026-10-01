#include "transport_internal.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace kf::net::detail {
namespace {
void require(bool valid, const char *reason) { if (!valid) throw std::runtime_error(reason); }
Event control(u8 kind, u8 peer, std::string_view text = {}, u32 generation = 0)
{ return {kind, peer, 0, generation, {}, {text.begin(), text.end()}}; }
std::string text(const Json &j, const char *field, std::size_t limit, bool optional = false)
{
    if (optional && !j.contains(field)) return {};
    require(j.contains(field) && j[field].is_string(), "Invalid signaling string");
    auto value = j[field].get<std::string>();
    require(value.size() <= limit && value.find('\0') == std::string::npos, "Invalid signaling string");
    return value;
}
u8 slot(const Json &j, const char *field)
{
    require(j.contains(field) && j[field].is_number_unsigned(), "Invalid peer slot");
    auto value = j[field].get<std::uint64_t>(); require(value < 4, "Invalid peer slot"); return value;
}
Identity identity(std::string_view value)
{
    require(value.size() == 64, "Invalid player identity");
    auto digit = [](char ch) -> u8 {
        if (ch >= '0' && ch <= '9') return ch - '0';
        require(ch >= 'a' && ch <= 'f', "Invalid player identity"); return ch - 'a' + 10;
    };
    Identity result {};
    for (unsigned i = 0; i < result.size(); ++i) result[i] = (digit(value[i * 2]) << 4) | digit(value[i * 2 + 1]);
    require(result != Identity{}, "Invalid player identity"); return result;
}
std::string identity_text(std::span<const u8> bytes)
{
    static constexpr char digits[] = "0123456789abcdef";
    std::string result; for (auto byte : bytes) { result += digits[byte >> 4]; result += digits[byte & 15]; } return result;
}
Json ice_servers(const Json &j)
{
    require(j.contains("iceServers") && j["iceServers"].is_array() && j["iceServers"].size() <= 8, "Invalid ICE configuration");
    Json result = Json::array();
    for (const auto &entry : j["iceServers"]) {
        auto url = text(entry, "urls", 2048);
        require(url.starts_with("stun:") || url.starts_with("stuns:") || url.starts_with("turn:") || url.starts_with("turns:"), "Invalid ICE URL");
        result.push_back({{"urls", url}, {"username", text(entry, "username", 256, true)}, {"credential", text(entry, "credential", 256, true)}});
    }
    return result;
}
Json signal(const Json &j)
{
    if (j.contains("sdp") && j.contains("descriptionType")) {
        auto kind = text(j, "descriptionType", 6); require(kind == "offer" || kind == "answer", "Invalid WebRTC description");
        return {{"sdp", text(j, "sdp", 60000)}, {"descriptionType", kind}};
    }
    return {{"candidate", text(j, "candidate", 4096)}, {"mid", text(j, "mid", 128)}};
}
}

Json parse(std::span<const u8> bytes)
{
    require(bytes.size() <= signal_limit, "Oversized signaling message");
    auto result = Json::parse(bytes.begin(), bytes.end(), [](int depth, Json::parse_event_t, Json &) {
        require(depth <= 32, "Signaling JSON is too deeply nested"); return true;
    });
    require(result.is_object(), "Invalid signaling message"); return result;
}
bool Inbox::live_locked(u8 peer, u32 generation) const
{ return peer < 4 && generation && !stopped && !failed && peers[peer].generation == generation && !peers[peer].closed; }
bool Inbox::live(u8 peer, u32 generation) const
{ std::lock_guard lock(mutex); return live_locked(peer, generation); }
u32 Inbox::generation(u8 peer) const
{ std::lock_guard lock(mutex); return peer < 4 ? peers[peer].generation : 0; }
u32 Inbox::begin(u8 peer)
{
    std::lock_guard lock(mutex); require(peer < 4 && peers[peer].generation != UINT32_MAX, "Peer generation exhausted");
    peers[peer] = {peers[peer].generation + 1, 0, false}; reset[peer] = false;
    std::erase_if(events, [peer](const auto &e) { return e.peer == peer && e.generation; });
    return peers[peer].generation;
}
bool Inbox::can_send(u8 peer, u8 lane, std::size_t size) const
{
    std::lock_guard lock(mutex);
    return peer < 4 && lane < 2 && size > 0 && size <= KF_NET_PACKET_LIMIT &&
        live_locked(peer, peers[peer].generation) && peers[peer].ready == 3;
}
void Inbox::fail_locked(std::string reason)
{
    if (failed || stopped) return;
    failed = true; events.clear(); pending.clear(); events.push_back(control(Error, 0, reason.substr(0, 256)));
}
void Inbox::fail(std::string reason) { std::lock_guard lock(mutex); fail_locked(std::move(reason)); }
void Inbox::end(std::string reason)
{
    std::lock_guard lock(mutex); if (stopped) return;
    stopped = true; events.clear(); pending.clear(); events.push_back(control(Ended, 0, reason.substr(0, 256)));
}
void Inbox::reject_locked(u8 peer, u32 generation, std::string reason)
{
    if (!live_locked(peer, generation)) return;
    peers[peer].closed = true; peers[peer].ready = 0;
    std::erase_if(events, [=](const auto &e) { return e.peer == peer && e.generation == generation; });
    events.push_back(control(Disconnected, peer, reason.substr(0, 256), generation));
}
void Inbox::reject(u8 peer, u32 generation, std::string reason)
{ std::lock_guard lock(mutex); reject_locked(peer, generation, std::move(reason)); }
void Inbox::disconnected(u8 peer, u32 generation)
{
    std::lock_guard lock(mutex); if (!live_locked(peer, generation)) return;
    peers[peer].closed = true; peers[peer].ready = 0;
    events.push_back(control(Disconnected, peer, {}, generation));
}
void Inbox::push_locked(Event event)
{
    if (failed || stopped) return;
    if (event.peer >= 4 || event.lane >= 2 || event.data.size() > KF_NET_PACKET_LIMIT) { fail_locked("Invalid network event"); return; }
    if (event.generation && peers[event.peer].generation != event.generation) return;
    if ((event.kind == Packet || event.kind == Connected) && !live_locked(event.peer, event.generation)) return;
    if (event.kind == Packet && event.lane == 1)
        std::erase_if(events, [&](const auto &e) { return e.kind == Packet && e.lane == 1 && e.peer == event.peer; });
    else if (event.generation == 0 || event.kind == Packet) {
        std::size_t count = 0, bytes = 0;
        for (const auto &e : events)
            if (event.generation == 0 ? e.generation == 0 : e.kind == Packet && e.lane == 0 && e.peer == event.peer) {
                ++count; bytes += e.data.size();
            }
        if (count >= queue_events || bytes + event.data.size() > queue_bytes) {
            if (event.generation) reject_locked(event.peer, event.generation, "Peer packet queue exceeded its limit");
            else fail_locked("Room event queue exceeded its limit");
            return;
        }
    }
    events.push_back(std::move(event));
}
void Inbox::push(Event event) { std::lock_guard lock(mutex); push_locked(std::move(event)); }
void Inbox::callback(u8 kind, u8 peer, u8 lane, u32 generation, std::span<const u8> data)
{
    std::lock_guard lock(mutex);
    if (failed || stopped || (generation && !live_locked(peer, generation))) return;
    if (peer >= 4 || lane >= 2 || data.size() > signal_limit) {
        if (generation) reject_locked(peer, generation, "Invalid peer network callback");
        else fail_locked("Invalid network callback");
        return;
    }
    if (kind == 4) {
        if (!generation || reset[peer]) return;
        auto &state = peers[peer]; const auto old = state.ready; state.ready |= 1 << lane;
        if (old != 3 && state.ready == 3) { auto e = control(Connected, peer, {}, generation); e.identity = identities[peer]; push_locked(std::move(e)); }
        return;
    }
    if (kind == 5) {
        if (!generation || reset[peer]) return;
        peers[peer].closed = true; peers[peer].ready = 0;
        events.push_back(control(Disconnected, peer, {}, generation)); return;
    }
    if (kind == 6) {
        if (!generation || reset[peer]) return;
        if (data.empty() || data.size() > KF_NET_PACKET_LIMIT) { reject_locked(peer, generation, "Invalid data channel packet"); return; }
        push_locked({Packet, peer, lane, generation, {}, {data.begin(), data.end()}}); return;
    }
    if (!(kind <= 3 || kind == 7 || kind == 8)) { fail_locked("Invalid network callback"); return; }
    Raw raw {kind, peer, generation, generation ? int(peer) : -1, false, {data.begin(), data.end()}};
    if (kind == 1) {
        try {
            const auto j = parse(data); const auto type = text(j, "type", 32);
            if (type == "signal") raw.owner = slot(j, "from");
            else if (type == "peer" || type == "peer-left") { raw.owner = slot(j, "slot"); raw.announcement = type == "peer"; }
            else if (type == "host-ready") { raw.owner = 0; raw.announcement = true; }
        } catch (const std::exception &) { /* Full validation runs on the owner thread. */ }
    }
    if (raw.announcement) {
        std::erase_if(pending, [&](const auto &e) { return e.owner == raw.owner; }); reset[raw.owner] = true;
    }
    if (generation && reset[peer]) return;
    if (raw.owner >= 0 && std::any_of(pending.begin(), pending.end(), [&](const auto &e) { return e.kind == 9 && e.owner == raw.owner; })) return;
    std::size_t count = 0, bytes = 0;
    for (const auto &e : pending) if (e.owner == raw.owner) { ++count; bytes += e.data.size(); }
    if (count >= queue_events || bytes + data.size() > queue_bytes) {
        if (raw.owner < 0) fail_locked("Room signaling queue exceeded its limit");
        else {
            std::erase_if(pending, [&](const auto &e) { return e.owner == raw.owner && !e.announcement; });
            pending.push_back({9, static_cast<u8>(raw.owner), reset[raw.owner] ? 0 : generation, raw.owner, false, {}});
        }
        return;
    }
    pending.push_back(std::move(raw));
}
std::optional<Event> Inbox::poll()
{
    std::lock_guard lock(mutex);
    while (!events.empty()) {
        auto event = std::move(events.front()); events.pop_front();
        if (!event.generation || peers[event.peer].generation == event.generation) return event;
    }
    return {};
}
std::optional<Raw> Inbox::raw()
{
    std::lock_guard lock(mutex); if (pending.empty()) return {};
    auto result = std::move(pending.front()); pending.pop_front(); return result;
}
bool Inbox::active() const { std::lock_guard lock(mutex); return !failed && !stopped; }
void Inbox::stop() { std::lock_guard lock(mutex); stopped = true; pending.clear(); events.clear(); }
}

using namespace kf::net::detail;
struct KfNetTransport {
    struct Peer { bool exists {}, described {}, describing {}; unsigned candidate_count {}; std::vector<Json> candidates; };
    std::shared_ptr<Inbox> inbox = std::make_shared<Inbox>();
    std::unique_ptr<Backend, decltype(&backend_close)> backend {nullptr, backend_close};
    KfNetTransportConfig config {};
    Json ice = Json::array();
    std::array<Peer, 4> peers {};
    bool admitted {};

    bool remote(u8 peer) const { return peer < 4 && (config.host ? peer != 0 : peer == 0); }
    void create_peer(u8 peer) {
        const auto generation = inbox->begin(peer); peers[peer] = {}; peers[peer].exists = true;
        require(backend_peer(*backend, peer, generation, config.host, ice), "Cannot create WebRTC peer");
    }
    void description(u8 peer, const Json &message) {
        if (!peers[peer].exists) { if (config.host) return; create_peer(peer); }
        auto &state = peers[peer];
        if (message.contains("sdp")) {
            require(!state.described && !state.describing && (message["descriptionType"] == "answer") == bool(config.host), "Unexpected WebRTC description");
            state.describing = true;
        } else {
            require(state.candidate_count < 64, "Too many ICE candidates"); ++state.candidate_count;
            if (!state.described) { state.candidates.push_back(message); return; }
        }
        require(backend_description(*backend, peer, message), "Invalid WebRTC signal");
    }
    void receive(const Json &j) {
        const auto type = text(j, "type", 32);
        if (type == "created" || type == "joined") {
            auto peer = slot(j, "slot"); auto id = identity(text(j, "identity", 64));
            auto room = text(j, "room", 128), resume = text(j, "resume", 128); auto servers = ice_servers(j);
            require(!admitted && (peer == 0) == bool(config.host) && !room.empty() && !resume.empty(), "Invalid room admission");
            admitted = true; ice = std::move(servers);
            { std::lock_guard lock(inbox->mutex); inbox->resume = resume; }
            auto event = control(Room, peer, room); event.identity = id; inbox->push(std::move(event));
        } else if (type == "lobby") {
            require(admitted && j.at("started").is_boolean() && j.at("members").is_array() &&
                j.at("members").size() == 4, "Invalid lobby state");
            Event event; event.kind = Lobby; event.data.resize(76);
            event.data[0] = j.at("started").get<bool>();
            for (u8 peer = 0; peer < 4; ++peer) {
                const auto &member = j.at("members")[peer];
                if (member.is_null()) continue;
                require(member.is_object() && member.at("ready").is_boolean() && member.at("connected").is_boolean() &&
                    member.at("avatar").is_number_unsigned() && member.at("avatar").get<unsigned>() < 44,
                    "Invalid lobby member");
                event.data[3] |= 1u << peer;
                if (member.at("connected").get<bool>()) event.data[1] |= 1u << peer;
                if (member.at("ready").get<bool>()) event.data[2] |= 1u << peer;
                require(!(event.data[2] & (1u << peer)) || (event.data[1] & (1u << peer)), "Invalid lobby readiness");
                event.data[4 + peer] = member.at("avatar").get<u8>();
                if (member.contains("code")) {
                    const auto code = text(member, "code", 16);
                    require(config.host && peer && code.size() == 16 &&
                        std::all_of(code.begin(), code.end(), [](unsigned char c) {
                            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                                (c >= '0' && c <= '9') || c == '-' || c == '_';
                        }), "Invalid character join code");
                    std::copy(code.begin(), code.end(), event.data.begin() + 8 + peer * 17);
                }
            }
            inbox->push(std::move(event));
        } else if (type == "peer") {
            auto peer = slot(j, "slot"); require(admitted && config.host && remote(peer), "Unexpected peer announcement");
            auto id = identity(text(j, "identity", 64)); auto servers = ice_servers(j); ice = std::move(servers);
            { std::lock_guard lock(inbox->mutex); inbox->identities[peer] = id; }
            create_peer(peer);
        } else if (type == "peer-left" || type == "host-wait") {
            const auto peer = type == "host-wait" ? u8(0) : slot(j, "slot");
            require(admitted && (type == "host-wait" ? !config.host : config.host && remote(peer)), "Unexpected peer departure");
            if (type == "host-wait") inbox->push(control(HostWaiting, 0));
            inbox->disconnected(peer, inbox->generation(peer)); backend_drop(*backend, peer); peers[peer] = {};
        } else if (type == "host-ready") {
            require(admitted && !config.host, "Unexpected host announcement");
            ice = ice_servers(j); inbox->begin(0); backend_drop(*backend, 0); peers[0] = {};
        } else if (type == "signal") {
            const auto peer = slot(j, "from"); require(admitted && remote(peer), "Unexpected peer signal");
            auto message = signal(j); const auto generation = inbox->generation(peer);
            if (generation && !inbox->live(peer, generation)) return;
            try { description(peer, message); }
            catch (const std::exception &error) { inbox->reject(peer, inbox->generation(peer), error.what()); }
        } else if (type == "ended") {
            require(admitted, "Unexpected room end"); inbox->end({});
        } else if (type == "error") {
            auto reason = text(j, "reason", 256); require(!reason.empty(), "Invalid room error");
            require(!j.contains("retryable") || j["retryable"].is_boolean(), "Invalid room error");
            if (j.value("retryable", false)) throw std::runtime_error(reason);
            inbox->end(reason);
        } else throw std::runtime_error("Unexpected room service response");
    }
    void process(const Raw &raw) {
        if (raw.generation && !inbox->live(raw.peer, raw.generation)) return;
        switch (raw.kind) {
        case 0: {
            Json roster = Json::array(); for (auto &id : config.roster) roster.push_back(identity_text(id));
            auto string = [](auto &field) { return std::string(reinterpret_cast<const char *>(field)); };
            const Json request {{"type", config.resume[0] ? "resume" : config.host ? "create" : "join"},
                {"protocol", KF_NET_PROTOCOL_VERSION}, {"resources", string(config.resources)}, {"recipe", string(config.recipe)},
                {"room", string(config.room)}, {"resume", string(config.resume)}, {"credential", string(config.credential)}, {"roster", roster},
                {"lobby", bool(config.lobby)}, {"avatar", config.avatar}, {"rosterAvatars", config.roster_avatars}};
            require(backend_signal(*backend, request.dump()), "Cannot send room request"); break;
        }
        case 1: receive(parse(raw.data)); break;
        case 2: throw std::runtime_error("Room service disconnected");
        case 3: {
            auto message = signal(parse(raw.data)); message["type"] = "signal"; message["to"] = raw.peer;
            require(backend_signal(*backend, message.dump()), "Cannot send WebRTC signal"); break;
        }
        case 7: throw std::runtime_error(std::string(raw.data.begin(), raw.data.end()));
        case 8: {
            auto &peer = peers[raw.peer]; peer.described = true;
            for (auto &message : peer.candidates) require(backend_description(*backend, raw.peer, message), "Cannot apply ICE candidate");
            peer.candidates.clear(); break;
        }
        case 9: {
            require(remote(raw.peer), "Invalid signaling queue owner");
            auto generation = inbox->generation(raw.peer); if (!generation) generation = inbox->begin(raw.peer);
            inbox->reject(raw.peer, generation, "Peer signaling queue exceeded its limit"); break;
        }
        default: throw std::runtime_error("Invalid network event");
        }
    }
    ~KfNetTransport() { inbox->stop(); }
};

KfNetTransport *kf_net_transport_open(const KfNetTransportConfig *config)
{
    if (!config || config->host > 1 || config->lobby > 1 || config->avatar >= 44) return nullptr;
    try {
        auto bounded = [](const auto &field) {
            const auto end = std::find(std::begin(field), std::end(field), 0);
            require(end != std::end(field), "Unterminated config string");
            // Serialization checks UTF-8 before opening any connection.
            (void)Json(std::string(std::begin(field), end)).dump();
        };
        bounded(config->address); bounded(config->room); bounded(config->resources); bounded(config->recipe); bounded(config->resume); bounded(config->credential);
        identity(reinterpret_cast<const char *>(config->credential));
        std::string address(reinterpret_cast<const char *>(config->address));
        require(address.starts_with("ws://") || address.starts_with("wss://"), "Invalid signaling address");
        auto transport = std::make_unique<KfNetTransport>(); transport->config = *config;
        transport->backend.reset(backend_open(transport->inbox, address));
        return transport->backend ? transport.release() : nullptr;
    } catch (const std::exception &) { return nullptr; }
}
void kf_net_transport_close(KfNetTransport *transport) { delete transport; }
KfCodecResult kf_net_transport_lobby(KfNetTransport *transport, u8 start)
{
    if (!transport || !transport->admitted || start > 1 || (start && !transport->config.host)) return KF_CODEC_INVALID;
    return backend_signal(*transport->backend, start ? "{\"type\":\"start\"}" : "{\"type\":\"ready\"}")
        ? KF_CODEC_OK : KF_CODEC_OUTPUT_FULL;
}
KfCodecResult kf_net_transport_poll(KfNetTransport *transport, KfNetTransportEvent *out)
{
    if (!transport || !out) return KF_CODEC_INVALID;
    auto &t = *transport;
    backend_poll(*t.backend);
    while (t.inbox->active()) {
        auto raw = t.inbox->raw(); if (!raw) break;
        try { t.process(*raw); }
        catch (const std::exception &error) {
            if (raw->generation) t.inbox->reject(raw->peer, raw->generation, error.what());
            else t.inbox->fail(error.what());
        }
    }
    for (u8 peer = 0; peer < 4; ++peer) if (t.peers[peer].exists && !t.inbox->live(peer, t.inbox->generation(peer))) {
        backend_drop(*t.backend, peer); t.peers[peer] = {};
    }
    auto event = t.inbox->poll(); if (!event) return KF_CODEC_END;
    out->kind = event->kind; out->peer = event->peer; out->lane = event->lane;
    std::copy(event->identity.begin(), event->identity.end(), out->identity);
    out->size = event->data.size(); std::copy(event->data.begin(), event->data.end(), out->data); return KF_CODEC_OK;
}
KfCodecResult kf_net_transport_send(KfNetTransport *transport, u8 peer, u8 lane, const u8 *bytes, std::size_t size)
{
    if (!transport || !bytes || !transport->remote(peer) || !transport->inbox->can_send(peer, lane, size)) return KF_CODEC_INVALID;
    return backend_send(*transport->backend, peer, lane, {bytes, size}) ? KF_CODEC_OK : KF_CODEC_OUTPUT_FULL;
}
KfCodecResult kf_net_transport_resume(KfNetTransport *transport, u8 *bytes, std::size_t capacity, std::size_t *written)
{
    if (!transport || !bytes || !written) return KF_CODEC_INVALID;
    std::lock_guard lock(transport->inbox->mutex); const auto &resume = transport->inbox->resume;
    if (resume.size() > capacity) return KF_CODEC_OUTPUT_FULL;
    std::copy(resume.begin(), resume.end(), bytes); *written = resume.size(); return KF_CODEC_OK;
}
