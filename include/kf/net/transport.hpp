#pragma once
#include <kf/lib/types.h>
#include <kf/net/identity.hpp>
#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace kf::net {
inline constexpr std::size_t maximum_packet_bytes = 60000;
enum class Channel : u8 { Actions, State };
enum class EventKind : u8 { Room, Connected, Disconnected, Packet, HostWaiting, Ended, Error };
struct Config {
    std::string signaling_url;
    std::string room;
    std::string resources;
    std::string avatar_recipe;
    std::string resume_token;
    std::string credential;
    std::array<Identity, 4> roster {};
    bool host {};
    u8 campaign_slot {};
    u8 avatar = 0xff;
};
// Set once by the launcher; an empty URL selects the original solo game.
extern Config application_config;
struct Event {
    EventKind kind {};
    u8 peer {};
    Channel channel {};
    std::vector<u8> packet;
    std::string text;
    Identity identity {};
};
struct Transport;
Transport *transport_open(const Config &config);
void transport_close(Transport *transport);
bool transport_poll(Transport &transport, Event &event);
bool transport_send(Transport &transport, u8 peer, Channel channel, std::span<const u8> packet);
const std::string &transport_resume_token(const Transport &transport);
}
