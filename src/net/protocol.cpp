#include <kf/net/protocol.hpp>
#include <algorithm>

namespace kf::net {
static KfNetHeader record(MessageHeader header)
{
    return {static_cast<u8>(header.kind), header.epoch, header.sequence, header.generation};
}

static MessageHeader message(KfNetHeader header)
{
    return {static_cast<MessageKind>(header.kind), header.epoch, header.sequence, header.generation};
}

static KfNetInputBundle record(const InputBundle &bundle)
{
    KfNetInputBundle result {record(bundle.header), bundle.count, {}};
    std::copy(bundle.frames.begin(), bundle.frames.end(), result.frames);
    return result;
}

bool packet_header(std::span<const u8> packet, MessageHeader &header)
{
    KfNetHeader result {};
    if (kf_net_header_decode(packet.data(), packet.size(), &result) != KF_CODEC_OK) return false;
    header = message(result);
    return true;
}

bool input_encode(InputBundle bundle, std::vector<u8> &packet)
{
    const auto input = record(bundle);
    packet.resize(message_header_bytes + 1 + KF_NET_INPUT_REDUNDANCY * 20);
    std::size_t written = 0;
    if (kf_net_input_encode(&input, packet.data(), packet.size(), &written) != KF_CODEC_OK) {
        packet.clear();
        return false;
    }
    packet.resize(written);
    return true;
}

bool input_decode(std::span<const u8> packet, InputBundle &bundle)
{
    KfNetInputBundle result {};
    if (kf_net_input_decode(packet.data(), packet.size(), &result) != KF_CODEC_OK) return false;
    bundle.header = message(result.header);
    bundle.count = result.count;
    std::copy(std::begin(result.frames), std::end(result.frames), bundle.frames.begin());
    return true;
}

bool command_encode(Command command, std::vector<u8> &packet)
{
    const KfNetCommand input {record(command.header), static_cast<u8>(command.kind), command.object, command.argument, command.target_generation};
    packet.resize(message_header_bytes + 9);
    std::size_t written = 0;
    if (kf_net_command_encode(&input, packet.data(), packet.size(), &written) != KF_CODEC_OK) {
        packet.clear();
        return false;
    }
    packet.resize(written);
    return true;
}

bool command_decode(std::span<const u8> packet, Command &command)
{
    KfNetCommand result {};
    if (kf_net_command_decode(packet.data(), packet.size(), &result) != KF_CODEC_OK) return false;
    command = {message(result.header), static_cast<CommandKind>(result.kind), result.object, result.argument, result.target_generation};
    return true;
}

bool interaction_encode(const KfNetInteraction &view, std::vector<u8> &packet)
{
    packet.resize(message_header_bytes + 5);
    std::size_t written = 0;
    if (kf_net_interaction_encode(&view, packet.data(), packet.size(), &written) != KF_CODEC_OK) {
        packet.clear();
        return false;
    }
    packet.resize(written);
    return true;
}

bool interaction_decode(std::span<const u8> packet, KfNetInteraction &view)
{
    return kf_net_interaction_decode(packet.data(), packet.size(), &view) == KF_CODEC_OK;
}

std::vector<u8> control_encode(MessageHeader header)
{
    const auto input = record(header);
    std::vector<u8> packet(message_header_bytes);
    std::size_t written = 0;
    if (kf_net_header_encode(&input, packet.data(), packet.size(), &written) != KF_CODEC_OK) return {};
    packet.resize(written);
    return packet;
}

std::vector<u8> snapshot_packet(MessageHeader header, std::span<const u8> payload)
{
    header.kind = MessageKind::Snapshot;
    auto packet = control_encode(header);
    if (packet.empty() || payload.size() > maximum_transfer_bytes) return {};
    packet.insert(packet.end(), payload.begin(), payload.end());
    return packet;
}

std::vector<std::vector<u8>> fragment_encode(MessageHeader header, std::span<const u8> payload)
{
    std::vector<std::vector<u8>> packets;
    if (payload.empty() || payload.size() > maximum_transfer_bytes) return packets;
    header.kind = MessageKind::Fragment;
    const auto input = record(header);
    for (std::size_t offset = 0; offset < payload.size(); offset += fragment_payload_bytes) {
        std::vector<u8> packet(message_header_bytes + 8 + fragment_payload_bytes);
        std::size_t written = 0;
        if (kf_net_fragment_encode(&input, payload.data(), payload.size(), static_cast<u32>(offset),
                                   packet.data(), packet.size(), &written) != KF_CODEC_OK) return {};
        packet.resize(written);
        packets.push_back(std::move(packet));
    }
    return packets;
}

bool fragment_accept(Transfer &transfer, std::span<const u8> packet)
{
    transfer.bytes.resize(maximum_transfer_bytes);
    if (kf_net_fragment_accept(&transfer.state, packet.data(), packet.size(),
                              transfer.bytes.data(), transfer.bytes.size()) != KF_CODEC_OK) {
        transfer = {};
        return false;
    }
    transfer.bytes.resize(transfer.state.received);
    transfer.complete = transfer.state.received == transfer.state.total;
    return true;
}

bool InputInbox::push(const InputBundle &bundle)
{
    const auto input = record(bundle);
    return kf_net_input_push(&state, &input) == KF_CODEC_OK;
}

bool InputInbox::pop(InputFrame &frame)
{
    return kf_net_input_pop(&state, &frame) == KF_CODEC_OK;
}
}
