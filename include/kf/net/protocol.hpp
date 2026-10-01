#pragma once
#include <kf/net/codec.h>
#include <kf/lib/types.h>
#include <array>
#include <span>
#include <vector>

namespace kf::net {
inline constexpr std::size_t maximum_transfer_bytes = KF_NET_TRANSFER_LIMIT;
inline constexpr std::size_t fragment_payload_bytes = KF_NET_FRAGMENT_BYTES;
enum class MessageKind : u8 { Input = 1, Snapshot, Fragment, Command, Loaded, Resync, Waiting, End, Accepted, Rejected, Interaction };
enum class CommandKind : u8 { Interact = 1, UseItem, Equip, SelectMagic, Buy, Sell, Save, Cancel, UseMagic, DropItem, ActivateObject, TakeLoot, ChooseAvatar, StoryReady };
struct MessageHeader {
    MessageKind kind {};
    u32 epoch {};
    u32 sequence {};
    u32 generation {};
};
inline constexpr std::size_t message_header_bytes = KF_NET_HEADER_BYTES;
using InputFrame = KfNetInputFrame;
struct InputBundle {
    MessageHeader header {MessageKind::Input};
    u8 count {};
    std::array<InputFrame, 4> frames {};
};
struct Command {
    MessageHeader header {MessageKind::Command};
    CommandKind kind {};
    u16 object {}, argument {};
    u32 target_generation {};
};
bool packet_header(std::span<const u8> packet, MessageHeader &header);
bool input_encode(InputBundle bundle, std::vector<u8> &packet);
bool input_decode(std::span<const u8> packet, InputBundle &bundle);
bool command_encode(Command command, std::vector<u8> &packet);
bool command_decode(std::span<const u8> packet, Command &command);
bool interaction_encode(const KfNetInteraction &view, std::vector<u8> &packet);
bool interaction_decode(std::span<const u8> packet, KfNetInteraction &view);
std::vector<u8> control_encode(MessageHeader header);
std::vector<u8> snapshot_packet(MessageHeader header, std::span<const u8> payload);
std::vector<std::vector<u8>> fragment_encode(MessageHeader header, std::span<const u8> payload);
struct Transfer {
    std::vector<u8> bytes;
    KfNetTransfer state {};
    bool complete {};
};
// One bounded, reliable, ordered transfer per peer. Any malformed fragment
// clears the partial transfer. State datagrams never enter this assembler.
bool fragment_accept(Transfer &transfer, std::span<const u8> packet);
struct InputInbox {
    KfNetInputInbox state {};
    bool push(const InputBundle &bundle);
    bool pop(InputFrame &frame);
};
}
