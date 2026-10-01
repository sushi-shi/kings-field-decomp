#include <kf/net/transport.hpp>
#include <kf/net/world.h>
#include <emscripten.h>
#include <algorithm>
#include <cstdlib>
#include <memory>

EM_JS_DEPS(crossplay_strings, "$UTF8ToString,$stringToNewUTF8");

EM_JS(char *, test_query, (const char *name), {
    return stringToNewUTF8(new URLSearchParams(location.search).get(UTF8ToString(name)) ?? "");
});
EM_JS(void, test_result, (int success, const char *detail), {
    document.body.dataset.result = success ? 'passed' : 'failed';
    document.body.dataset.detail = UTF8ToString(detail);
});
EM_JS(void, test_status, (const char *name, const char *value), {
    document.body.dataset[UTF8ToString(name)] = UTF8ToString(value);
});
EM_JS(int, test_release, (), {
    return !new URLSearchParams(location.search).has('hold') || document.body.dataset.release === 'yes';
});
EM_JS(int, test_pause, (), {
    const mode = new URLSearchParams(location.search).get('overflow');
    return mode === 'packets' ? 1 : mode === 'signals' && document.body.dataset.requestPause === 'yes' ? 2 : 0;
});

static bool world_codec_roundtrip()
{
    auto source = std::make_unique<KfNetWorld>();
    auto restored = std::make_unique<KfNetWorld>();
    source->header = {1, 42, 1, 1, 0};
    for (auto &actor : source->actors) actor.slot_state = 255;
    for (auto &effect : source->effects) effect.slot_type = 255;
    for (auto &object : source->objects) object.object_id = 255;
    for (auto &event : source->events) event.state = 255;
    source->floor_height[9999] = 73;
    KfNetWorldLimits limits {};
    std::fill(std::begin(limits.asset_clips), std::end(limits.asset_clips), -1);
    std::vector<u8> bytes(KF_NET_TRANSFER_LIMIT);
    std::size_t size = 0;
    if (kf_net_world_encode(source.get(), &limits, bytes.data(), bytes.size(), &size) != KF_CODEC_OK ||
        kf_net_world_decode(bytes.data(), size, &limits, restored.get()) != KF_CODEC_OK ||
        restored->header.tick != 42 || restored->floor_height[9999] != 73) return false;
    // Schema 11 ends with a 35-byte story record. Corrupt the final RLE run's
    // length, not a zero-valued cell or the padded C struct's layout.
    bytes[size - 35 - 3] = bytes[size - 35 - 2] = 0;
    return kf_net_world_decode(bytes.data(), size, &limits, restored.get()) == KF_CODEC_INVALID &&
        restored->header.tick == 42 && restored->floor_height[9999] == 73;
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
    const auto deadline=emscripten_get_now()+5000;
    while (!ended && emscripten_get_now()<deadline) {
        Event event;
        while (transport_poll(*client,event)) {
            ended=true;
            success=event.kind==EventKind::Ended && event.text=="Room unavailable";
        }
        emscripten_sleep(2);
    }
    transport_close(client);
    return success;
}

int main()
{
    using namespace kf::net;
    if (!world_codec_roundtrip()) { test_result(0, "Browser world codec failed"); return 1; }
    Config config;
    config.credential = std::string(64, '2');
    auto *url = test_query("signal"), *room = test_query("room"), *host = test_query("host");
    config.signaling_url = url;
    config.room = room;
    config.host = *host == '1';
    std::free(url); std::free(room); std::free(host);
    config.resources = "crossplay-integration-test";
    config.avatar_recipe = "test-recipe";
    if (!rejected_resume(config)) { test_result(0,"Resume rejection lost its terminal reason"); return 1; }
    auto *client = transport_open(config);
    if (!client) { test_result(0, "Cannot open browser transport"); return 1; }
    // Relay selection can outlast direct candidate checks and DTLS backoff.
    const auto deadline = emscripten_get_now() + 60000;
    bool ready = false, reliable = false, state = false, success = false;
    bool paused = false;
    u8 remote = 0;
    std::vector<u8> packet(maximum_packet_bytes);
    for (std::size_t i = 0; i < packet.size(); ++i) packet[i] = i % 251;
    packet[0] = 0; packet[1] = 1; packet[2] = 255; packet[3] = 128;
    double next_state = 0;
    while (emscripten_get_now() < deadline && !success) {
        if (!paused && ready && test_pause() == 2) {
            paused = true;
            test_status("paused", "yes");
            emscripten_sleep(1500);
        }
        Event event;
        while (transport_poll(*client, event)) {
            if (event.kind == EventKind::Room) test_status("room", event.text.c_str());
            if (event.kind == EventKind::Disconnected && !event.text.empty()) test_status("rejected", event.text.c_str());
            if (event.kind == EventKind::Error) {
                test_result(0, event.text.c_str());
                transport_close(client);
                return 1;
            }
            if (event.kind == EventKind::Connected && !ready) {
                remote = event.peer;
                ready = transport_send(*client, remote, Channel::Actions, packet);
                if (ready) test_status("ready", "yes");
            } else if (event.kind == EventKind::Connected && !paused && test_pause() == 1) {
                paused = true;
                test_status("paused", "yes");
                emscripten_sleep(1500);
            }
            if (event.kind == EventKind::Packet && event.channel == Channel::Actions && event.packet == packet) reliable = true;
            if (event.kind == EventKind::Packet && event.channel == Channel::State && event.packet == std::vector<u8>({2, 1, 255, 128})) state = true;
        }
        if (ready && !state && emscripten_get_now() >= next_state) {
            transport_send(*client, remote, Channel::State, std::vector<u8> {2, 1, 255, 128});
            next_state = emscripten_get_now() + 100;
        }
        if (reliable && state && test_release()) success = transport_send(*client, remote, Channel::Actions, std::vector<u8> {1, 1, 255, 128});
        emscripten_sleep(2);
    }
    emscripten_sleep(100);
    test_result(success, success ? "Browser rejection reason, world codec and native WebRTC: 60 KB reliable packets and state channel passed" : "WebRTC deadline exceeded");
    transport_close(client);
    return success ? 0 : 1;
}
