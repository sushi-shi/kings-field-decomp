#include <kf/net/transport.h>
#include <emscripten.h>

EM_JS_DEPS(transport_strings, "$UTF8ToString");
extern "C" EMSCRIPTEN_KEEPALIVE void kf_transport_event(uint32_t handle, uint8_t kind,
    uint8_t peer, uint8_t lane, uint32_t generation, const uint8_t *data, size_t size)
{
    kf_net_browser_event(handle, kind, peer, lane, generation, data, size);
}

EM_JS(int, kf_browser_open, (uint32_t handle, const char *address), {
    const registry = Module.kfTransports ||= new Map();
    const session = { peers: new Map() };
    session.emit = (kind, peer = 0, lane = 0, generation = 0, data = "") => {
        if (registry.get(handle) !== session) return;
        const bytes = typeof data === 'string' ? new TextEncoder().encode(data) : new Uint8Array(data);
        if (bytes.length > 65536) { session.emit(7, peer, lane, generation, 'Oversized network event'); return; }
        const pointer = _malloc(Math.max(1, bytes.length));
        if (!pointer) return;
        HEAPU8.set(bytes, pointer);
        _kf_transport_event(handle, kind, peer, lane, generation, pointer, bytes.length);
        _free(pointer);
    };
    try { session.socket = new WebSocket(UTF8ToString(address)); }
    catch { return 0; }
    registry.set(handle, session);
    session.socket.onopen = () => session.emit(0);
    session.socket.onmessage = event => {
        if (typeof event.data === 'string') session.emit(1, 0, 0, 0, event.data);
        else session.emit(7, 0, 0, 0, 'Invalid signaling packet');
    };
    session.socket.onclose = () => session.emit(2);
    session.socket.onerror = () => session.emit(7, 0, 0, 0, 'Cannot reach room service');
    return 1;
});

EM_JS(void, kf_browser_close, (uint32_t handle), {
    const session = Module.kfTransports?.get(handle);
    if (!session) return;
    Module.kfTransports.delete(handle);
    session.socket.close();
    for (const peer of session.peers.values()) peer.pc.close();
});

EM_JS(int, kf_browser_signal, (uint32_t handle, const char *text), {
    const socket = Module.kfTransports?.get(handle)?.socket;
    if (!socket || socket.readyState !== WebSocket.OPEN || socket.bufferedAmount > 262144) return 0;
    try { socket.send(UTF8ToString(text)); return 1; } catch { return 0; }
});

EM_JS(int, kf_browser_peer, (uint32_t handle, uint8_t slot, uint32_t generation, int initiator, const char *ice), {
    const session = Module.kfTransports?.get(handle);
    if (!session) return 0;
    try {
        session.peers.get(slot)?.pc.close();
        const peer = { pc: new RTCPeerConnection({ iceServers: JSON.parse(UTF8ToString(ice)) }), channels: [] };
        session.peers.set(slot, peer);
        peer.valid = () => Module.kfTransports?.get(handle) === session && session.peers.get(slot) === peer;
        peer.emit = (kind, lane = 0, data = "") => { if (peer.valid()) session.emit(kind, slot, lane, generation, data); };
        const signal = fields => peer.emit(3, 0, JSON.stringify(fields));
        peer.pc.onicecandidate = event => {
            if (event.candidate) signal({ candidate: event.candidate.candidate, mid: event.candidate.sdpMid ?? '0' });
        };
        peer.pc.onconnectionstatechange = () => {
            if (['failed', 'disconnected', 'closed'].includes(peer.pc.connectionState)) peer.emit(5);
        };
        const attach = channel => {
            const lane = channel.label === 'actions' ? 0 : channel.label === 'state' ? 1 : -1;
            if (lane < 0 || peer.channels[lane] || channel.ordered !== (lane === 0) ||
                channel.maxRetransmits !== (lane === 0 ? null : 0) || channel.maxPacketLifeTime !== null ||
                !channel.negotiated || channel.id !== lane || channel.protocol !== "") {
                channel.close(); peer.emit(7, 0, 'Invalid data channel reliability'); return;
            }
            peer.channels[lane] = channel;
            channel.binaryType = 'arraybuffer';
            channel.onopen = () => peer.emit(4, lane);
            channel.onclose = () => peer.emit(5, lane);
            channel.onerror = () => peer.emit(5, lane);
            channel.onmessage = event => {
                if (event.data instanceof ArrayBuffer && event.data.byteLength > 0 && event.data.byteLength <= 60000)
                    peer.emit(6, lane, event.data);
                else peer.emit(7, lane, 'Invalid data channel packet');
            };
            if (channel.readyState === 'open') peer.emit(4, lane);
        };
        peer.pc.ondatachannel = event => { event.channel.close(); peer.emit(7, 0, 'Unexpected data channel'); };
        attach(peer.pc.createDataChannel('actions', { ordered: true, negotiated: true, id: 0 }));
        attach(peer.pc.createDataChannel('state', { ordered: false, maxRetransmits: 0, negotiated: true, id: 1 }));
        if (initiator) {
            (async () => {
                await peer.pc.setLocalDescription(await peer.pc.createOffer());
                signal({ sdp: peer.pc.localDescription.sdp, descriptionType: 'offer' });
            })().catch(() => peer.emit(7, 0, 'Cannot create WebRTC offer'));
        }
        return 1;
    } catch { return 0; }
});

EM_JS(void, kf_browser_drop_peer, (uint32_t handle, uint8_t slot), {
    const session = Module.kfTransports?.get(handle);
    if (!session) return;
    const peer = session.peers.get(slot);
    session.peers.delete(slot);
    peer?.pc.close();
});

EM_JS(int, kf_browser_description, (uint32_t handle, uint8_t slot, const char *text), {
    const peer = Module.kfTransports?.get(handle)?.peers.get(slot);
    if (!peer) return 0;
    try {
        const message = JSON.parse(UTF8ToString(text));
        (async () => {
            if ('sdp' in message) {
                await peer.pc.setRemoteDescription({ type: message.descriptionType, sdp: message.sdp });
                if (!peer.valid()) return;
                if (message.descriptionType === 'offer') {
                    await peer.pc.setLocalDescription(await peer.pc.createAnswer());
                    peer.emit(3, 0, JSON.stringify({ sdp: peer.pc.localDescription.sdp, descriptionType: 'answer' }));
                }
                peer.emit(8);
            } else await peer.pc.addIceCandidate({ candidate: message.candidate, sdpMid: message.mid });
        })().catch(() => peer.emit(7, 0, 'Invalid WebRTC signal'));
        return 1;
    } catch { return 0; }
});

EM_JS(int, kf_browser_send, (uint32_t handle, uint8_t slot, uint8_t lane, const uint8_t *data, size_t size), {
    const channel = Module.kfTransports?.get(handle)?.peers.get(slot)?.channels[lane];
    if (!channel || channel.readyState !== 'open' || channel.bufferedAmount > (lane ? 60000 : 1048576)) return 0;
    try { channel.send(HEAPU8.slice(data, data + size)); return 1; } catch { return 0; }
});
