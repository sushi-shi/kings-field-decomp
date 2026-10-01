import { createServer } from 'node:http';
import { createHash, createHmac, randomBytes, timingSafeEqual } from 'node:crypto';
import { createReadStream } from 'node:fs';
import { stat } from 'node:fs/promises';
import { join } from 'node:path';
import { pathToFileURL } from 'node:url';
import { WebSocketServer, WebSocket } from 'ws';

const token = () => randomBytes(24).toString('base64url');
const equalToken = (a, b) => typeof a === 'string' && /^[A-Za-z0-9_-]{32}$/.test(a) && a.length === b.length &&
  timingSafeEqual(Buffer.from(a), Buffer.from(b));
const text = (value, limit) => typeof value === 'string' && value.length <= limit;
const identityText = value => typeof value === 'string' && /^[0-9a-f]{64}$/.test(value);
const emptyIdentity = '0'.repeat(64);
const identityFor = credential => identityText(credential) && credential !== emptyIdentity
  ? createHash('sha256').update(Buffer.from(credential, 'hex')).digest('hex') : null;

export function createRoomService(options = {}) {
  if (Boolean(options.turnUrl) !== Boolean(options.turnSecret))
    throw new Error('TURN_URL and TURN_SECRET must be configured together');
  const turnUrls = options.turnUrl ? options.turnUrl.split(',').map(url => url.trim()) : [];
  if (turnUrls.length > 7 || turnUrls.some(url =>
    url.length > 2048 || !/^turns?:[^\s,\x00-\x1f\x7f]+$/.test(url)))
    throw new Error('TURN_URL must contain one to seven comma-separated TURN URLs');
  const rooms = new Map();
  const webFiles = new Map([
    ['/', ['kings-field.html', 'text/html; charset=utf-8']],
    ['/kings-field.html', ['kings-field.html', 'text/html; charset=utf-8']],
    ['/kings-field.js', ['kings-field.js', 'text/javascript; charset=utf-8']],
    ['/kings-field.wasm', ['kings-field.wasm', 'application/wasm']],
  ]);
  const server = createServer(async (request, response) => {
    let pathname;
    try { pathname = new URL(request.url, 'http://localhost').pathname; }
    catch { response.writeHead(400); response.end(); return; }
    if (pathname === '/health') {
      response.writeHead(200, { 'content-type': 'text/plain' });
      response.end('ok\n');
      return;
    }
    const file = options.webRoot && webFiles.get(pathname);
    if (file && ['GET', 'HEAD'].includes(request.method)) {
      try {
        const path = join(options.webRoot, file[0]);
        const info = await stat(path);
        if (!info.isFile()) throw new Error('Missing web artifact');
        response.writeHead(200, { 'content-type': file[1], 'content-length': info.size,
          'cache-control': 'no-cache', 'x-content-type-options': 'nosniff' });
        if (request.method === 'HEAD') response.end();
        else {
          const stream = createReadStream(path);
          stream.on('error', () => response.destroy());
          response.on('close', () => stream.destroy());
          stream.pipe(response);
        }
        return;
      } catch { /* An absent build is a 404, never a directory listing. */ }
    }
    response.writeHead(404, { 'content-type': 'text/plain' });
    response.end('not found\n');
  });
  const sockets = new WebSocketServer({ server, maxPayload: 65536, perMessageDeflate: false });
  const send = (socket, value, sender = socket) => {
    if (socket?.readyState !== WebSocket.OPEN) return;
    // Reserve one maximum frame for room notifications, including peer-left
    // after a flooding sender is closed. Forwarded traffic cannot spend it.
    const limit = sender === socket ? 327680 : 262144;
    if (socket.bufferedAmount > limit) {
      // A host sending to a slow guest must also lose only that guest.
      (sender.slot === 0 ? socket : sender).close(1008, 'Signaling backlog');
      return;
    }
    socket.send(JSON.stringify(value));
  };
  const fail = (socket, reason, retryable = false) => send(socket, { type: 'error', reason, retryable });
  const iceServers = () => {
    const servers = [];
    if (options.stunUrl) servers.push({ urls: options.stunUrl });
    if (options.turnUrl && options.turnSecret) {
      const username = `${Math.floor(Date.now() / 1000) + 3600}:${randomBytes(8).toString('hex')}`;
      const credential = createHmac('sha1', options.turnSecret).update(username).digest('base64');
      for (const urls of turnUrls) servers.push({ urls, username, credential });
    }
    return servers;
  };
  const closeRoom = room => {
    if (rooms.get(room.id) !== room) return;
    rooms.delete(room.id);
    clearTimeout(room.expiry);
    for (const member of room.members) if (member?.socket) {
      send(member.socket, { type: 'ended' });
      member.socket.close(1000, 'Host left');
    }
  };
  const attach = (socket, room, slot, resumed = false) => {
    const member = room.members[slot];
    member.socket?.close(1000, 'Reconnected');
    member.socket = socket;
    socket.room = room;
    socket.slot = slot;
    if (slot === 0) clearTimeout(room.expiry);
    send(socket, { type: slot === 0 ? 'created' : 'joined', room: room.id,
      slot, resume: member.resume, identity: member.identity, iceServers: iceServers(), resumed });
    // A waiting host/guest can outlive its admission credentials. Deliver fresh
    // credentials before announcing the connection that will consume them.
    if (slot === 0) {
      for (let guest = 1; guest < 4; ++guest) if (room.members[guest]?.socket) {
        send(socket, { type: 'peer', slot: guest, identity: room.members[guest].identity,
          iceServers: iceServers() });
        send(room.members[guest].socket, { type: 'host-ready', iceServers: iceServers() });
      }
    } else send(room.members[0].socket, { type: 'peer', slot, identity: member.identity,
      iceServers: iceServers() });
  };
  sockets.on('connection', socket => {
    // Rejected sockets can still receive frames during the close handshake.
    socket.on('error', () => {});
    if (sockets.clients.size > (options.maxClients ?? 1024)) {
      socket.close(1013, 'Service full'); return;
    }
    socket.alive = true;
    socket.messages = 0;
    socket.rateStart = Date.now();
    socket.on('pong', () => { socket.alive = true; });
    socket.on('message', (bytes, binary) => {
      if (binary) { socket.close(1008, 'Expected JSON signaling'); return; }
      if (Date.now() - socket.rateStart >= 1000) { socket.rateStart = Date.now(); socket.messages = 0; }
      if (++socket.messages > 100) { socket.close(1008, 'Signaling rate exceeded'); return; }
      let message;
      try { message = JSON.parse(bytes.toString()); } catch { fail(socket, 'Invalid JSON'); return; }
      if (!message || typeof message !== 'object' || Array.isArray(message)) return fail(socket, 'Invalid message');
      if (message.type === 'create' && !socket.room) {
        if (rooms.size >= (options.maxRooms ?? 256)) return fail(socket, 'Service full', true);
        if (!Number.isInteger(message.protocol) || !text(message.resources, 128) || !text(message.recipe, 128))
          return fail(socket, 'Missing compatibility information');
        const identity = identityFor(message.credential);
        if (!identity) return fail(socket, 'Invalid player credential');
        const roster = message.roster ?? Array(4).fill(emptyIdentity);
        if (!Array.isArray(roster) || roster.length !== 4 || !roster.every(identityText) ||
            (roster[0] !== emptyIdentity && roster[0] !== identity) ||
            new Set([identity, ...roster.slice(1).filter(id => id !== emptyIdentity)]).size !==
              1 + roster.slice(1).filter(id => id !== emptyIdentity).length)
          return fail(socket, 'Campaign belongs to a different player or has an invalid roster');
        const room = { id: randomBytes(12).toString('base64url'), protocol: message.protocol,
          resources: message.resources, recipe: message.recipe,
          members: roster.map(id => id === emptyIdentity ? null : { identity: id, resume: token() }) };
        room.members[0] = { identity, resume: token() };
        rooms.set(room.id, room);
        return attach(socket, room, 0);
      }
      if ((message.type === 'join' || message.type === 'resume') && !socket.room) {
        const room = rooms.get(message.room);
        if (!room) return fail(socket, 'Room unavailable');
        const identity = identityFor(message.credential);
        if (!identity) return fail(socket, 'Invalid player credential');
        if (message.type === 'resume') {
          if (message.protocol !== room.protocol || message.resources !== room.resources || message.recipe !== room.recipe)
            return fail(socket, 'Incompatible game resources or protocol');
          const slot = room.members.findIndex(member => member && member.identity === identity && equalToken(message.resume, member.resume));
          if (slot < 0) return fail(socket, 'Invalid reconnect token');
          return attach(socket, room, slot, true);
        }
        if (message.protocol !== room.protocol || message.resources !== room.resources || message.recipe !== room.recipe)
          return fail(socket, 'Incompatible game resources or protocol');
        if (!room.members[0].socket) return fail(socket, 'Host reconnecting', true);
        const returning = room.members.findIndex(member => member?.identity === identity);
        if (returning === 0) return fail(socket, 'The host cannot join as a guest');
        if (returning > 0) return attach(socket, room, returning, true);
        // Reserve disconnected slots for the same participant until the host
        // explicitly removes them; reconnects cannot become another character.
        const slot = [1, 2, 3].find(index => !room.members[index]);
        if (slot === undefined) return fail(socket, 'Party full');
        room.members[slot] = { identity, resume: token() };
        return attach(socket, room, slot);
      }
      const room = socket.room;
      if (!room || rooms.get(room.id) !== room || room.members[socket.slot]?.socket !== socket)
        return fail(socket, 'Join a room first');
      if (message.type === 'leave') {
        if (socket.slot === 0) return closeRoom(room);
        socket.close(1000, 'Left party');
        return;
      }
      if (message.type !== 'signal' || !Number.isInteger(message.to) || message.to < 0 || message.to > 3 ||
          message.to === socket.slot || (socket.slot !== 0 && message.to !== 0))
        return fail(socket, 'Invalid signaling route');
      if (text(message.sdp, 60000) && ['offer', 'answer'].includes(message.descriptionType))
        send(room.members[message.to]?.socket, { type: 'signal', from: socket.slot,
          sdp: message.sdp, descriptionType: message.descriptionType }, socket);
      else if (text(message.candidate, 4096) && text(message.mid, 128))
        send(room.members[message.to]?.socket, { type: 'signal', from: socket.slot,
          candidate: message.candidate, mid: message.mid }, socket);
      else fail(socket, 'Invalid signal');
    });
    socket.on('close', () => {
      const room = socket.room;
      if (!room || rooms.get(room.id) !== room || room.members[socket.slot]?.socket !== socket) return;
      room.members[socket.slot].socket = null;
      if (socket.slot === 0) {
        for (const member of room.members.slice(1)) send(member?.socket, { type: 'host-wait' });
        room.expiry = setTimeout(() => closeRoom(room), options.hostGraceMs ?? 15000);
      } else send(room.members[0].socket, { type: 'peer-left', slot: socket.slot });
    });
  });
  const heartbeat = setInterval(() => {
    for (const socket of sockets.clients) {
      if (!socket.alive) { socket.terminate(); continue; }
      socket.alive = false;
      socket.ping();
    }
  }, 10000);
  heartbeat.unref();
  return {
    server,
    close: async () => {
      clearInterval(heartbeat);
      for (const room of rooms.values()) clearTimeout(room.expiry);
      rooms.clear();
      for (const socket of sockets.clients) socket.terminate();
      await new Promise(resolve => sockets.close(resolve));
      await new Promise(resolve => server.close(resolve));
    },
  };
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  const service = createRoomService({ webRoot: process.env.WEB_ROOT, stunUrl: process.env.STUN_URL,
    turnUrl: process.env.TURN_URL, turnSecret: process.env.TURN_SECRET });
  service.server.listen(Number(process.env.PORT ?? 8787), process.env.BIND ?? '127.0.0.1');
  for (const signal of ['SIGINT', 'SIGTERM']) process.on(signal, () => void service.close());
}
