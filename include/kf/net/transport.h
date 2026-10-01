#pragma once
#include <kf/net/codec.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KfNetTransport KfNetTransport;
typedef struct KfNetTransportConfig {
    uint8_t address[2049], room[129], resources[129], recipe[129], resume[129];
    uint8_t credential[65], roster[4][32];
    uint8_t host;
} KfNetTransportConfig;
typedef struct KfNetTransportEvent {
    uint8_t kind, peer, lane;
    uint8_t identity[32];
    uint32_t size;
    uint8_t data[KF_NET_PACKET_LIMIT];
} KfNetTransportEvent;

/* One owner calls open/send/poll/close. The transport owns connections and queues.
 * Config strings are UTF-8, NUL terminated. Events are copied to caller storage.
 * Transport handles are invalid after close; no callbacks enter gameplay. */
KfNetTransport *kf_net_transport_open(const KfNetTransportConfig *);
void kf_net_transport_close(KfNetTransport *);
KfCodecResult kf_net_transport_poll(KfNetTransport *, KfNetTransportEvent *);
KfCodecResult kf_net_transport_send(KfNetTransport *, uint8_t, uint8_t, const uint8_t *, size_t);
KfCodecResult kf_net_transport_resume(KfNetTransport *, uint8_t *, size_t, size_t *);

/* Browser bridge only: integer session IDs reject callbacks after close. */
void kf_net_browser_event(uint32_t, uint8_t, uint8_t, uint8_t, uint32_t, const uint8_t *, size_t);

#ifdef __cplusplus
}
#endif
