#pragma once
#include <kf/lib/codec.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    KF_NET_PROTOCOL_VERSION = 15,
    KF_NET_HEADER_BYTES = 19,
    KF_NET_PACKET_LIMIT = 60000,
    KF_NET_TRANSFER_LIMIT = 131072,
    KF_NET_FRAGMENT_BYTES = 16000,
    KF_NET_INPUT_REDUNDANCY = 4,
    KF_NET_INPUT_QUEUE = 32
};

/* Semantic values at the ABI boundary. Their layout is never the wire format. */
typedef struct KfNetHeader {
    uint8_t kind;
    uint32_t epoch, sequence, generation;
} KfNetHeader;
typedef struct KfNetInputFrame {
    uint32_t sequence, tick, buttons;
    int32_t yaw, pitch;
} KfNetInputFrame;
typedef struct KfNetInputBundle {
    KfNetHeader header;
    uint8_t count;
    KfNetInputFrame frames[KF_NET_INPUT_REDUNDANCY];
} KfNetInputBundle;
typedef struct KfNetCommand {
    KfNetHeader header;
    uint8_t kind;
    uint16_t object, argument;
    uint32_t target_generation;
} KfNetCommand;
typedef struct KfNetInteraction {
    KfNetHeader header;
    uint8_t floor, stage, character, page, shop;
} KfNetInteraction;
typedef struct KfNetFragment {
    KfNetHeader header;
    uint32_t total, offset, payload_offset, payload_size;
} KfNetFragment;
typedef struct KfNetInputInbox {
    KfNetInputFrame pending[KF_NET_INPUT_QUEUE];
    uint32_t received, consumed;
    uint8_t begin, count;
} KfNetInputInbox;
typedef struct KfNetTransfer {
    uint32_t epoch, sequence, generation, total, received;
} KfNetTransfer;

/* Invalid input never partially updates a decoded output or input queue.
 * Buffers/records are caller-owned, correctly aligned, valid and disjoint.
 * These codec calls are internal to the network implementation; gameplay uses
 * the session's typed commands/events, not packet or socket callbacks. */
KfCodecResult kf_net_header_decode(const uint8_t *, size_t, KfNetHeader *);
KfCodecResult kf_net_header_encode(const KfNetHeader *, uint8_t *, size_t, size_t *);
KfCodecResult kf_net_input_decode(const uint8_t *, size_t, KfNetInputBundle *);
KfCodecResult kf_net_input_encode(const KfNetInputBundle *, uint8_t *, size_t, size_t *);
KfCodecResult kf_net_command_decode(const uint8_t *, size_t, KfNetCommand *);
KfCodecResult kf_net_command_encode(const KfNetCommand *, uint8_t *, size_t, size_t *);
KfCodecResult kf_net_interaction_decode(const uint8_t *, size_t, KfNetInteraction *);
KfCodecResult kf_net_interaction_encode(const KfNetInteraction *, uint8_t *, size_t, size_t *);
KfCodecResult kf_net_fragment_decode(const uint8_t *, size_t, KfNetFragment *);
KfCodecResult kf_net_fragment_encode(const KfNetHeader *, const uint8_t *, size_t,
                                   uint32_t, uint8_t *, size_t, size_t *);
KfCodecResult kf_net_input_push(KfNetInputInbox *, const KfNetInputBundle *);
KfCodecResult kf_net_input_pop(KfNetInputInbox *, KfNetInputFrame *);
/* Ordered reliable fragments only. Failure clears the transfer state.
 * Output storage may contain partial bytes; consume only when received == total. */
KfCodecResult kf_net_fragment_accept(KfNetTransfer *, const uint8_t *, size_t,
                                   uint8_t *, size_t);

#ifdef __cplusplus
}
#endif
