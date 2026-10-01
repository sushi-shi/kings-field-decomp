#include <kf/net/protocol.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>

static void require(bool value, const char *message) {
    if (!value) { std::fprintf(stderr, "%s\n", message); std::abort(); }
}

static void abi_rejections(const std::vector<u8> &packet)
{
    KfNetInputBundle output {};
    output.header.epoch = 1234;
    output.frames[0].yaw = -765;
    std::array<u8, sizeof(output)> before;
    std::memcpy(before.data(), &output, sizeof(output));
    for (std::size_t size = 0; size < packet.size(); ++size) {
        require(kf_net_input_decode(packet.data(), size, &output) == KF_CODEC_INVALID,
                "C ABI accepted truncated input");
        require(std::memcmp(before.data(), &output, sizeof(output)) == 0,
                "C ABI partially modified rejected output");
    }
    require(kf_net_input_decode(nullptr, packet.size(), &output) == KF_CODEC_INVALID,
            "C ABI accepted null input");
    require(kf_net_input_decode(packet.data(), packet.size(), nullptr) == KF_CODEC_INVALID,
            "C ABI accepted null output");
    require(kf_net_input_decode(packet.data(), std::numeric_limits<std::size_t>::max(), &output) == KF_CODEC_INVALID,
            "C ABI accepted unbounded input length");
    alignas(KfNetInputBundle) std::array<u8, sizeof(KfNetInputBundle) + 1> unaligned {};
    require(kf_net_input_decode(packet.data(), packet.size(),
                reinterpret_cast<KfNetInputBundle *>(unaligned.data() + 1)) == KF_CODEC_INVALID,
            "C ABI accepted misaligned output");
    require(kf_net_input_decode(packet.data(), packet.size(), &output) == KF_CODEC_OK,
            "C ABI rejected valid input");
    std::vector<u8> buffer(packet.size() + 1, 0xa5);
    std::size_t written = 1234;
    require(kf_net_input_encode(&output, buffer.data(), packet.size() - 1, &written) == KF_CODEC_OUTPUT_FULL &&
            written == 1234 && buffer[packet.size() - 1] == 0xa5,
            "Short output buffer corrupted storage or result length");
    for (const auto offset : {0u, 4u, 6u, 19u}) {
        auto invalid = packet;
        invalid[offset] = 0xff;
        require(kf_net_input_decode(invalid.data(), invalid.size(), &output) == KF_CODEC_INVALID,
                "Invalid magic, version, message kind or count accepted");
    }
}

static void queue_capacity()
{
    using namespace kf::net;
    InputInbox inbox;
    InputBundle bundle;
    bundle.header = {MessageKind::Input, 1, 0, 1};
    bundle.count = 1;
    for (u32 sequence = 1; sequence <= KF_NET_INPUT_QUEUE; ++sequence) {
        bundle.header.sequence = sequence;
        bundle.frames[0].sequence = sequence;
        require(inbox.push(bundle), "Input queue filled early");
    }
    bundle.header.sequence = bundle.frames[0].sequence = KF_NET_INPUT_QUEUE + 1;
    require(!inbox.push(bundle) && inbox.state.received == KF_NET_INPUT_QUEUE,
            "Input queue overflow advanced acknowledgement");
    InputFrame frame {};
    require(inbox.pop(frame) && frame.sequence == 1 && inbox.push(bundle), "Cannot reuse queue capacity");
    for (u32 sequence = 2; sequence <= KF_NET_INPUT_QUEUE + 1; ++sequence)
        require(inbox.pop(frame) && frame.sequence == sequence, "Queue wrap changed input ordering");
    require(!inbox.pop(frame), "Queue wrap left stale input");
}

int main() {
    using namespace kf::net;
    InputBundle input;
    input.header = {MessageKind::Input, 4, 3, 7};
    input.count = 3;
    for (u32 i = 0; i < input.count; ++i) input.frames[i] = {i + 1, i + 20, 0x1000, 10, -20};
    std::vector<u8> packet;
    require(input_encode(input, packet), "Cannot encode controls");
    abi_rejections(packet);
    queue_capacity();
    const std::array<u8, 19> header_bytes {0x4b, 0x46, 0x4e, 0x31, 15, 0, 1, 4, 0, 0, 0, 3, 0, 0, 0, 7, 0, 0, 0};
    require(std::memcmp(packet.data(), header_bytes.data(), header_bytes.size()) == 0,
            "Protocol wire header changed");
    auto old_version = packet;
    old_version[4] = KF_NET_PROTOCOL_VERSION - 1;
    MessageHeader unsupported;
    require(!packet_header(old_version, unsupported), "Previous protocol version bypassed compatibility validation");
    Command ready {{MessageKind::Command,1,1,1},CommandKind::StoryReady,1,0}, ready_decoded;
    std::vector<u8> ready_bytes;
    require(command_encode(ready,ready_bytes) && command_decode(ready_bytes,ready_decoded) && ready_decoded.object == 1,
        "Story acknowledgement did not round trip");
    ready.object = 3;
    require(!command_encode(ready,ready_bytes),"Unknown story page accepted");
    ready.object = 2;
    ready.argument = 1;
    require(!command_encode(ready,ready_bytes),"Story acknowledgement accepted extra arguments");
    ready.argument = 0;
    ready.target_generation = 1;
    require(!command_encode(ready,ready_bytes),"Story acknowledgement accepted an object generation");
    InputBundle decoded;
    require(input_decode(packet, decoded) && decoded.header.generation == 7 && decoded.frames[2].pitch == -20,
            "Input wire round trip failed");
    for (std::size_t size = 0; size < packet.size(); ++size)
        require(!input_decode(std::span<const u8>(packet).first(size), decoded), "Truncated input accepted");
    packet.push_back(0);
    require(!input_decode(packet, decoded), "Trailing input accepted");
    input.frames[1].sequence = 1;
    require(!input_encode(input, packet), "Unordered input accepted");
    input.frames[1].sequence = 2;
    input.frames[0].yaw = 100000;
    require(!input_encode(input, packet), "Unbounded look accepted");
    input.frames[0].yaw = 0;
    InputInbox inbox;
    require(inbox.push(input) && inbox.push(input) && inbox.state.count == 3, "Redundant input duplicated commands");
    auto invalid = input;
    invalid.frames[0].sequence = 4;
    invalid.frames[1].sequence = 2000;
    invalid.frames[2].sequence = 2001;
    invalid.header.sequence = 2001;
    require(!inbox.push(invalid) && inbox.state.count == 3 && inbox.state.received == 3, "Invalid bundle partially committed");
    InputFrame frame;
    for (u32 i = 1; i <= 3; ++i)
        require(inbox.pop(frame) && frame.sequence == i && inbox.state.consumed == i, "Input consumption lost ordering");
    require(!inbox.pop(frame), "Input queue underflow");

    std::vector<u8> payload(maximum_transfer_bytes);
    for (std::size_t i = 0; i < payload.size(); ++i) payload[i] = i % 251;
    const auto fragments = fragment_encode({MessageKind::Fragment, 9, 10, 1}, payload);
    require(fragments.size() > 2, "Large snapshot not fragmented");
    Transfer transfer;
    for (const auto &fragment : fragments) require(fragment_accept(transfer, fragment), "Valid fragment rejected");
    require(transfer.complete && transfer.bytes == payload, "Fragmented snapshot corrupt");
    require(!fragment_accept(transfer, fragments.back()) && transfer.bytes.empty(), "Duplicate tail accepted");
    require(fragment_accept(transfer, fragments.front()), "Cannot restart transfer");
    require(!fragment_accept(transfer, fragments[2]) && transfer.bytes.empty(), "Fragment gap retained partial world");
    auto bad = fragments.front();
    bad[message_header_bytes] = 0xff;
    bad[message_header_bytes + 3] = 0x7f;
    require(!fragment_accept(transfer, bad) && transfer.bytes.empty(), "Oversized allocation accepted");
    auto changed = fragments[1];
    changed[7] ^= 1;
    require(fragment_accept(transfer, fragments.front()) && !fragment_accept(transfer, changed), "Cross-epoch transfer accepted");
    changed = fragments[1];
    changed[15] ^= 1;
    require(fragment_accept(transfer, fragments.front()) && !fragment_accept(transfer, changed) && transfer.bytes.empty(),
            "Cross-generation transfer accepted");
    KfNetTransfer state {};
    std::array<u8, 16> short_output {};
    require(kf_net_fragment_accept(&state, fragments.front().data(), fragments.front().size(),
                                  short_output.data(), short_output.size()) == KF_CODEC_OUTPUT_FULL && state.total == 0,
            "Fragment wrote beyond destination capacity");
    Command command {{MessageKind::Command, 4, 1, 7}, CommandKind::Equip, 3, 2}, result;
    require(command_encode(command, packet) && command_decode(packet, result) && result.object == 3,
            "Reliable command wire round trip failed");
    command.kind = CommandKind::ActivateObject;
    require(command_encode(command, packet) && command_decode(packet, result) && result.kind == command.kind,
            "Object activation command was rejected by the codec");
    packet[message_header_bytes] = 14;
    require(!command_decode(packet, result), "Unknown command operation was accepted");
    command.kind = CommandKind::TakeLoot;
    command.target_generation = 0x12345678;
    command.argument = (60 << 8) | 3;
    require(command_encode(command, packet) && command_decode(packet, result) &&
        result.target_generation == command.target_generation && result.argument == command.argument,
        "Pickup command lost its object generation or component");
    command.argument = (60 << 8) | 4;
    require(!command_encode(command, packet), "Pickup accepted an invalid container component");
    command.argument = 60 << 8;
    command.target_generation = 0;
    require(!command_encode(command, packet), "Pickup accepted an absent object generation");
    for (const auto kind : {MessageKind::Accepted, MessageKind::Rejected}) {
        MessageHeader response;
        const auto bytes = control_encode({kind, 4, 19, 7});
        require(bytes.size() == message_header_bytes && packet_header(bytes, response) &&
                response.kind == kind && response.sequence == 19 && response.generation == 7,
                "Host action result lost command correlation");
    }
    KfNetInteraction view {{static_cast<u8>(MessageKind::Interaction), 4, 21, 7}, 1, 2, 3, 4, 1}, restored {};
    require(interaction_encode(view, packet) && interaction_decode(packet, restored) &&
        restored.page == 4 && restored.shop == 1 && restored.header.sequence == 21,
        "Interaction response lost its dialogue, shop or request identity");
    for (std::size_t size = 0; size < packet.size(); ++size)
        require(!interaction_decode(std::span(packet).first(size), restored), "Truncated interaction was accepted");
    for (const auto offset : {19u, 20u, 21u, 22u, 23u}) {
        auto invalid = packet;
        invalid[offset] = 255;
        restored.page = 7;
        require(!interaction_decode(invalid, restored) && restored.page == 7, "Invalid interaction partially updated output");
    }
}
