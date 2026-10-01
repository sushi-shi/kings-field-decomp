#include "transport_internal.h"
#include <rtc/rtc.hpp>

#include <chrono>

namespace kf::net::detail {
using Clock = std::chrono::steady_clock;
struct Backend {
    struct Peer {
        std::shared_ptr<rtc::PeerConnection> connection;
        std::array<std::shared_ptr<rtc::DataChannel>, 2> channels;
        std::array<std::size_t, 2> last_buffered {};
        std::array<Clock::time_point, 2> last_progress {};
        u32 generation {};
    };
    std::shared_ptr<Inbox> inbox;
    std::shared_ptr<rtc::WebSocket> socket;
    std::array<Peer, 4> peers;
};
namespace {
auto callback(const std::shared_ptr<Inbox> &inbox, u8 peer = 0, u32 generation = 0)
{
    return [weak = std::weak_ptr(inbox), peer, generation](u8 kind, u8 lane = 0, std::span<const u8> data = {}) {
        if (auto state = weak.lock()) state->callback(kind, peer, lane, generation, data);
    };
}
std::span<const u8> bytes(const std::string &value)
{ return {reinterpret_cast<const u8 *>(value.data()), value.size()}; }
}
Backend *backend_open(std::shared_ptr<Inbox> inbox, const std::string &address)
{
    auto backend = std::make_unique<Backend>(); backend->inbox = std::move(inbox);
    rtc::WebSocketConfiguration config; config.maxMessageSize = signal_limit;
    config.connectionTimeout = std::chrono::seconds(10);
    backend->socket = std::make_shared<rtc::WebSocket>(config);
    const auto emit = callback(backend->inbox);
    backend->socket->onOpen([emit] { emit(0); });
    backend->socket->onClosed([emit] { emit(2); });
    backend->socket->onError([emit](std::string) { emit(7, 0, bytes("Cannot reach room service")); });
    backend->socket->onMessage([emit](rtc::message_variant message) {
        if (auto text = std::get_if<std::string>(&message)) emit(1, 0, bytes(*text));
        else emit(7, 0, bytes("Invalid signaling packet"));
    });
    backend->socket->open(address);
    return backend.release();
}
void backend_drop(Backend &backend, u8 peer)
{
    auto &state = backend.peers[peer];
    for (auto &channel : state.channels) if (channel) { channel->resetCallbacks(); channel->close(); }
    if (state.connection) { state.connection->resetCallbacks(); state.connection->close(); }
    state = {};
}
void backend_close(Backend *backend)
{
    if (!backend) return;
    for (u8 peer = 0; peer < 4; ++peer) backend_drop(*backend, peer);
    backend->socket->resetCallbacks(); backend->socket->close(); delete backend;
}
bool backend_signal(Backend &backend, const std::string &message)
{
    if (!backend.socket->isOpen() || backend.socket->bufferedAmount() > 262144) return false;
    try { backend.socket->send(message); return true; } catch (const std::exception &) { return false; }
}
bool backend_peer(Backend &backend, u8 peer, u32 generation, bool initiator, const Json &ice)
{
    backend_drop(backend, peer);
    try {
        rtc::Configuration config; config.disableAutoNegotiation = true; config.maxMessageSize = KF_NET_PACKET_LIMIT;
        for (const auto &entry : ice) {
            rtc::IceServer server(entry.at("urls").get<std::string>());
            server.username = entry.at("username").get<std::string>(); server.password = entry.at("credential").get<std::string>();
            config.iceServers.push_back(std::move(server));
        }
        auto &state = backend.peers[peer]; state.generation = generation;
        state.connection = std::make_shared<rtc::PeerConnection>(config);
        const auto emit = callback(backend.inbox, peer, generation);
        state.connection->onLocalDescription([emit](rtc::Description description) {
            const Json message {{"sdp", std::string(description)}, {"descriptionType", description.typeString()}};
            emit(3, 0, bytes(message.dump()));
        });
        state.connection->onLocalCandidate([emit](rtc::Candidate candidate) {
            const Json message {{"candidate", std::string(candidate)}, {"mid", candidate.mid()}};
            emit(3, 0, bytes(message.dump()));
        });
        state.connection->onStateChange([emit](rtc::PeerConnection::State state) {
            if (state == rtc::PeerConnection::State::Failed || state == rtc::PeerConnection::State::Disconnected || state == rtc::PeerConnection::State::Closed) emit(5);
        });
        state.connection->onDataChannel([emit](std::shared_ptr<rtc::DataChannel> channel) {
            channel->close(); emit(7, 0, bytes("Unexpected data channel"));
        });
        for (u8 lane = 0; lane < 2; ++lane) {
            rtc::DataChannelInit init; init.negotiated = true; init.id = lane; init.reliability.unordered = lane == 1;
            if (lane) init.reliability.maxRetransmits = 0;
            auto &channel = state.channels[lane]; channel = state.connection->createDataChannel(lane ? "state" : "actions", init);
            channel->onOpen([emit, lane] { emit(4, lane); });
            channel->onClosed([emit, lane] { emit(5, lane); });
            channel->onError([emit, lane](std::string) { emit(5, lane); });
            channel->onMessage([emit, lane](rtc::message_variant message) {
                if (auto binary = std::get_if<rtc::binary>(&message))
                    emit(6, lane, {reinterpret_cast<const u8 *>(binary->data()), binary->size()});
                else emit(7, lane, bytes("Invalid data channel packet"));
            });
            if (channel->isOpen()) emit(4, lane);
        }
        if (initiator) state.connection->setLocalDescription(rtc::Description::Type::Offer);
        return true;
    } catch (const std::exception &) { backend_drop(backend, peer); return false; }
}
bool backend_description(Backend &backend, u8 peer, const Json &message)
{
    auto &state = backend.peers[peer]; if (!state.connection) return false;
    try {
        if (message.contains("sdp")) {
            const auto kind = message.at("descriptionType").get<std::string>();
            state.connection->setRemoteDescription(rtc::Description(message.at("sdp").get<std::string>(), kind));
            if (kind == "offer") state.connection->setLocalDescription(rtc::Description::Type::Answer);
            backend.inbox->callback(8, peer, 0, state.generation, {});
        } else state.connection->addRemoteCandidate(rtc::Candidate(message.at("candidate").get<std::string>(), message.at("mid").get<std::string>()));
        return true;
    } catch (const std::exception &) { return false; }
}
bool backend_send(Backend &backend, u8 peer, u8 lane, std::span<const u8> bytes)
{
    auto &state = backend.peers[peer]; auto &channel = state.channels[lane];
    if (!channel || !channel->isOpen()) return false;
    const auto buffered = channel->bufferedAmount();
    if (!buffered || buffered < state.last_buffered[lane]) state.last_progress[lane] = Clock::now();
    if (buffered + bytes.size() > (lane ? 2 : 32) * KF_NET_PACKET_LIMIT) {
        if (!lane) backend.inbox->reject(peer, state.generation, "Peer reliable send queue exceeded its limit");
        return false;
    }
    try {
        // libdatachannel returns false when it accepted a message into its send
        // buffer. Retrying that message would duplicate reliable actions.
        channel->send(reinterpret_cast<const std::byte *>(bytes.data()), bytes.size());
        state.last_buffered[lane] = channel->bufferedAmount();
        return true;
    } catch (const std::exception &) {
        if (!lane) backend.inbox->reject(peer, state.generation, "Peer reliable send failed");
        return false;
    }
}
void backend_poll(Backend &backend)
{
    for (u8 peer = 0; peer < 4; ++peer) {
        auto &state = backend.peers[peer]; auto &channel = state.channels[0];
        if (channel) {
            const auto buffered = channel->bufferedAmount();
            if (!buffered || buffered < state.last_buffered[0]) state.last_progress[0] = Clock::now();
            state.last_buffered[0] = buffered;
            if (buffered && Clock::now() - state.last_progress[0] > std::chrono::seconds(2))
                backend.inbox->reject(peer, state.generation, "Peer reliable send stalled");
        }
    }
}
}
