#include <kf/net/transport.hpp>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>

int main(int argc, char **argv)
{
    using namespace kf::net;
    if (argc != 2 && argc != 3) return 2;
    Config config;
    config.credential = std::string(64, '1');
    config.signaling_url = argv[1];
    config.resources = "crossplay-integration-test";
    config.avatar_recipe = "test-recipe";
    config.host = argc == 2;
    if (!config.host) config.room = argv[2];
    auto *host = transport_open(config);
    if (!host) return 1;
    std::vector<u8> large(maximum_packet_bytes);
    for (std::size_t i = 0; i < large.size(); ++i) large[i] = i % 251;
    large[0] = 0; large[1] = 1; large[2] = 255; large[3] = 128;
    bool success = false, failed = false;
    unsigned connections = 0;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(65);
    while (!success && !failed && std::chrono::steady_clock::now() < deadline) {
        Event event;
        while (transport_poll(*host, event)) {
            if (event.kind == EventKind::Room) {
                std::printf("ROOM %s\n", event.text.c_str());
                std::fflush(stdout);
            } else if (event.kind == EventKind::Error) {
                std::fprintf(stderr, "%s\n", event.text.c_str());
                failed = true;
            } else if (event.kind == EventKind::Disconnected && !event.text.empty()) {
                std::printf("REJECTED %u %s\n", event.peer, event.text.c_str());
                std::fflush(stdout);
            } else if (event.kind == EventKind::Connected) {
                if (++connections == 2 && std::getenv("KF_TEST_PEER_OVERFLOW")) {
                    std::puts("PAUSED");
                    std::fflush(stdout);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                }
            } else if (event.kind == EventKind::Packet) {
                if (event.channel == Channel::Actions && event.packet == large)
                    failed = !transport_send(*host, event.peer, event.channel, event.packet);
                else if (event.channel == Channel::State && event.packet == std::vector<u8>({2, 1, 255, 128}))
                    failed = !transport_send(*host, event.peer, event.channel, event.packet);
                else if (event.channel == Channel::Actions && event.packet == std::vector<u8>({1, 1, 255, 128})) success = true;
                else failed = true;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    transport_close(host);
    return success && !failed ? 0 : 1;
}
