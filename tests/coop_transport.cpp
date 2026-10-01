#include <kf/net/transport.hpp>
#include <array>
#include <chrono>
#include <cstdio>
#include <thread>

static std::vector<u8> payload(u8 marker, unsigned lane)
{
    std::vector<u8> bytes(lane ? 4 : kf::net::maximum_packet_bytes);
    for (std::size_t i = 0; i < bytes.size(); ++i) bytes[i] = i % 251;
    bytes[0] = static_cast<u8>(lane); bytes[1] = marker; bytes[2] = 255; bytes[3] = 128;
    return bytes;
}

static bool rejected_resume(kf::net::Config config)
{
    using namespace kf::net;
    config.host=false;
    config.room="expired-room";
    config.resume_token=std::string(32,'x');
    auto *client=transport_open(config);
    if (!client) return false;
    bool ended=false, success=false;
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
    while (!ended && std::chrono::steady_clock::now()<deadline) {
        Event event;
        while (transport_poll(*client,event)) {
            ended=true;
            success=event.kind==EventKind::Ended && event.text=="Room unavailable";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    transport_close(client);
    if (!success) std::fprintf(stderr,"Resume rejection lost its terminal reason\n");
    return success;
}

int main(int argc, char **argv)
{
    using namespace kf::net;
    if (argc != 2) return 2;
    Config config;
    config.credential = std::string(64, '1');
    config.signaling_url = argv[1];
    config.resources = "transport-integration-test";
    config.avatar_recipe = "test-recipe";
    config.host = true;
    if (!rejected_resume(config)) return 1;
    std::array<Transport *, 4> clients {};
    clients[0] = transport_open(config);
    bool sent[4][2] {}, received[4][2] {}, echoed[4][2] {};
    bool ready[4][4] {};
    u8 slots[4] {};
    bool failed = !clients[0];
    bool reconnected = false;
    bool host_reconnected = false;
    u8 returning_slot = 0;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(20);
    while (!failed && std::chrono::steady_clock::now() < deadline) {
        for (unsigned index = 0; index < 4; ++index) {
            if (!clients[index]) continue;
            Event event;
            while (transport_poll(*clients[index], event)) {
                if (event.kind == EventKind::Error) {
                    std::fprintf(stderr, "peer %u: %s\n", index, event.text.c_str());
                    failed = true;
                } else if (event.kind == EventKind::Room && index == 0) {
                    config.host = false;
                    config.room = event.text;
                    for (unsigned guest = 1; guest < 4 && !host_reconnected; ++guest) {
                        config.credential = std::string(64, '1' + guest);
                        clients[guest] = transport_open(config);
                        if (!clients[guest]) failed = true;
                    }
                } else if (event.kind == EventKind::Room) {
                    slots[index] = event.peer;
                    if (reconnected && index == 1 && event.peer != returning_slot) {
                        std::fprintf(stderr, "Resume changed the player's room slot\n");
                        failed = true;
                    }
                }
                else if (event.kind == EventKind::Connected) ready[index][event.peer] = true;
                else if (event.kind == EventKind::Disconnected) ready[index][event.peer] = false;
                else if (event.kind == EventKind::Packet) {
                    const u8 marker = index ? slots[index] : event.peer;
                    const auto lane = static_cast<unsigned>(event.channel);
                    if (event.packet != payload(marker, lane)) {
                        std::fprintf(stderr, "Binary packet corrupted or routed to another participant\n");
                        failed = true;
                    }
                    if (index == 0) {
                        received[event.peer][lane] = true;
                        if (!transport_send(*clients[0], event.peer, event.channel, event.packet)) failed = true;
                    } else echoed[index][lane] = true;
                }
            }
        }
        for (u8 guest = 1; guest < 4; ++guest) {
            for (unsigned lane = 0; lane < 2; ++lane) {
                if ((!sent[guest][lane] || (lane && !echoed[guest][lane])) && slots[guest] && ready[0][slots[guest]] && ready[guest][0]) {
                    sent[guest][lane] = transport_send(*clients[guest], 0, static_cast<Channel>(lane), payload(slots[guest], lane));
                }
            }
        }
        if (echoed[1][0] && echoed[2][0] && echoed[3][0] && echoed[1][1] && echoed[2][1] && echoed[3][1]) {
            if (host_reconnected) break;
            if (reconnected) {
                config.host = true;
                config.credential = std::string(64, '1');
                config.resume_token = transport_resume_token(*clients[0]);
                if (config.resume_token.empty()) { failed = true; break; }
                transport_close(clients[0]);
                clients[0] = transport_open(config);
                if (!clients[0]) { failed = true; break; }
                for (u8 guest = 1; guest < 4; ++guest) {
                    ready[0][slots[guest]] = ready[guest][0] = false;
                    for (unsigned lane = 0; lane < 2; ++lane)
                        sent[guest][lane] = received[slots[guest]][lane] = echoed[guest][lane] = false;
                }
                host_reconnected = true;
                continue;
            }
            config.credential = std::string(64, '2');
            config.resume_token = transport_resume_token(*clients[1]);
            if (config.resume_token.empty()) { failed = true; break; }
            returning_slot = slots[1];
            transport_close(clients[1]);
            clients[1] = transport_open(config);
            if (!clients[1]) { failed = true; break; }
            slots[1] = 0;
            ready[0][returning_slot] = ready[1][0] = false;
            for (unsigned lane = 0; lane < 2; ++lane) {
                sent[1][lane] = received[returning_slot][lane] = echoed[1][lane] = false;
            }
            reconnected = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    const bool success = !failed && reconnected && host_reconnected && received[1][0] && received[2][0] && received[3][0] &&
        echoed[1][0] && echoed[2][0] && echoed[3][0] && echoed[1][1] && echoed[2][1] && echoed[3][1];
    for (auto *client : clients) transport_close(client);
    if (!success) std::fprintf(stderr, "Four-party WebRTC exchange did not complete\n");
    return success ? 0 : 1;
}
