#include <kf/net/transport.hpp>
#include <kf/net/transport.h>
#include <algorithm>

namespace kf::net {
Config application_config;
struct Transport {
    KfNetTransport *handle {};
    KfNetTransportEvent event {};
    mutable std::string resume;
};

template<std::size_t N>
static bool string_copy(u8 (&output)[N], const std::string &input)
{
    if (input.size() >= N || input.find('\0') != std::string::npos) return false;
    std::copy(input.begin(), input.end(), output);
    return true;
}

Transport *transport_open(const Config &config)
{
    KfNetTransportConfig input {};
    if (!string_copy(input.address, config.signaling_url) || !string_copy(input.room, config.room) ||
        !string_copy(input.resources, config.resources) || !string_copy(input.recipe, config.avatar_recipe) ||
        !string_copy(input.resume, config.resume_token) ||
        !string_copy(input.credential, config.credential)) return nullptr;
    for (unsigned slot = 0; slot < config.roster.size(); ++slot)
        std::copy(config.roster[slot].begin(), config.roster[slot].end(), input.roster[slot]);
    input.host = config.host;
    auto transport = std::make_unique<Transport>();
    transport->handle = kf_net_transport_open(&input);
    return transport->handle ? transport.release() : nullptr;
}

void transport_close(Transport *transport)
{
    if (!transport) return;
    kf_net_transport_close(transport->handle);
    delete transport;
}

bool transport_poll(Transport &transport, Event &event)
{
    auto &result = transport.event;
    if (kf_net_transport_poll(transport.handle, &result) != KF_CODEC_OK) return false;
    event = {};
    event.kind = static_cast<EventKind>(result.kind);
    event.peer = result.peer;
    event.channel = static_cast<Channel>(result.lane);
    std::copy(std::begin(result.identity), std::end(result.identity), event.identity.begin());
    if (event.kind == EventKind::Packet) event.packet.assign(result.data, result.data + result.size);
    else event.text.assign(result.data, result.data + result.size);
    return true;
}

bool transport_send(Transport &transport, u8 peer, Channel channel, std::span<const u8> packet)
{
    return kf_net_transport_send(transport.handle, peer, static_cast<u8>(channel), packet.data(), packet.size()) == KF_CODEC_OK;
}

const std::string &transport_resume_token(const Transport &transport)
{
    u8 bytes[128];
    std::size_t size = 0;
    if (kf_net_transport_resume(transport.handle, bytes, sizeof bytes, &size) == KF_CODEC_OK)
        transport.resume.assign(bytes, bytes + size);
    return transport.resume;
}
}
