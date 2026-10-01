import { test } from 'node:test';
import assert from 'node:assert/strict';
import { once } from 'node:events';
import { randomBytes, createHash, createHmac } from 'node:crypto';
import { mkdtemp, writeFile, rm } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { connect as connectTcp } from 'node:net';
import WebSocket from 'ws';
import { createRoomService } from './server.mjs';

async function setup(t, options = {}) {
  const service = createRoomService(options);
  service.server.listen(0, '127.0.0.1');
  await once(service.server, 'listening');
  t.after(() => service.close());
  return `ws://127.0.0.1:${service.server.address().port}`;
}

async function connect(url, credential = randomBytes(32).toString('hex')) {
  const socket = new WebSocket(url);
  const queue = [], pending = [];
  socket.on('message', bytes => {
    const message = JSON.parse(bytes.toString());
    const index = pending.findIndex(wait => wait.type === message.type);
    if (index < 0) queue.push(message);
    else pending.splice(index, 1)[0].resolve(message);
  });
  await once(socket, 'open');
  return {
    socket,
    credential,
    send: message => socket.send(JSON.stringify({ credential, ...message })),
    next: type => {
      const index = queue.findIndex(message => message.type === type);
      if (index >= 0) return Promise.resolve(queue.splice(index, 1)[0]);
      return new Promise((resolve, reject) => {
        const timer = setTimeout(() => reject(new Error(`Missing ${type}`)), 2000);
        pending.push({ type, resolve: value => { clearTimeout(timer); resolve(value); } });
      });
    },
  };
}

const compatibility = { protocol: 8, resources: 'retail-test-hash', recipe: 'avatar-test-recipe' };
async function lobby(client, predicate) {
  for (let attempt = 0; attempt < 20; ++attempt) {
    const state = await client.next('lobby');
    if (predicate(state)) return state;
  }
  assert.fail('Expected lobby state was not published');
}

test('host waits in a visible roster and starts the ready party together', async t => {
  const url = await setup(t);
  const host = await connect(url);
  host.send({type:'create', ...compatibility, lobby:true, avatar:41});
  const room = await host.next('created');
  assert.equal((await host.next('lobby')).started, false);
  host.send({type:'ready'});
  const guest = await connect(url);
  guest.send({type:'join', room:room.room, ...compatibility, avatar:1});
  await guest.next('joined');
  const waiting = await lobby(host, state => state.members[1]?.connected);
  assert.equal(waiting.members[1].avatar, 1);
  assert.equal(waiting.members[1].ready, false);
  host.send({type:'start'});
  assert.equal((await lobby(host, state => !!state.members[1])).started, false);
  guest.send({type:'start'});
  assert.match((await guest.next('error')).reason, /Only the host/);
  guest.send({type:'ready'});
  await lobby(host, state => state.members[1]?.ready);
  host.send({type:'start'});
  for (const client of [host,guest]) assert.equal((await lobby(client, state => state.started)).started, true);
});

test('saved character codes transfer a character to another device and reserve absent characters', async t => {
  const url = await setup(t);
  const host = await connect(url);
  const originalCredential = randomBytes(32).toString('hex');
  const savedIdentity = createHash('sha256').update(Buffer.from(originalCredential, 'hex')).digest('hex');
  host.send({type:'create', ...compatibility, lobby:true,
    roster:['0'.repeat(64), savedIdentity, 'b'.repeat(64), '0'.repeat(64)], rosterAvatars:[41,1,19,41]});
  const room = await host.next('created');
  const saved = await host.next('lobby');
  const code = saved.members[1].code;
  assert.match(code, /^[A-Za-z0-9_-]{16}$/);
  assert.equal(saved.members[1].connected, false);
  host.send({type:'ready'});
  await lobby(host, state => state.members[0].ready);
  host.send({type:'start'});
  assert.equal((await lobby(host, state => state.started)).started, true, 'Absent characters must not prevent a solo start');
  const returning = await connect(url);
  returning.send({type:'join', room:code, ...compatibility, avatar:5});
  const admission = await returning.next('joined');
  assert.equal(admission.room, room.room);
  assert.equal(admission.slot, 1);
  assert.equal(admission.identity, savedIdentity);
  const returned = await returning.next('lobby');
  assert.equal(returned.members[1].avatar, 1, 'Claiming a saved character must preserve its avatar');
  assert.ok(returned.members.every(member => !member || !('code' in member)), 'Character codes leaked to a guest');
  const previousOwner = await connect(url, originalCredential);
  previousOwner.send({type:'join', room:room.room, ...compatibility});
  assert.match((await previousOwner.next('error')).reason, /character code/, 'Original profile allocated a duplicate saved identity');
  const duplicate = await connect(url);
  duplicate.send({type:'join', room:code, ...compatibility});
  assert.match((await duplicate.next('error')).reason, /already connected/);
  const newcomer = await connect(url);
  newcomer.send({type:'join', room:room.room, ...compatibility, avatar:26});
  assert.equal((await newcomer.next('joined')).slot, 3, 'New player replaced an absent saved character');
  const extra = await connect(url);
  extra.send({type:'join', room:room.room, ...compatibility});
  assert.match((await extra.next('error')).reason, /Party full/);
  returning.socket.close();
  await lobby(host, state => !state.members[1].connected);
  duplicate.send({type:'join', room:code, ...compatibility});
  assert.equal((await duplicate.next('joined')).identity, savedIdentity);
  host.send({type:'leave'});
  await duplicate.next('ended');
  const expired = await connect(url);
  expired.send({type:'join', room:code, ...compatibility});
  assert.equal((await expired.next('error')).reason, 'Room unavailable');
});

test('departing lobby newcomers free a slot while saved characters remain reserved', async t => {
  const url = await setup(t);
  const host = await connect(url);
  host.send({type:'create', ...compatibility, lobby:true});
  const room = await host.next('created');
  const first = await connect(url);
  first.send({type:'join', room:room.room, ...compatibility});
  await first.next('joined');
  await lobby(host, state => state.members[1]?.connected);
  first.socket.close();
  await lobby(host, state => state.members[1] === null);
  const second = await connect(url);
  second.send({type:'join', room:room.room, ...compatibility});
  assert.equal((await second.next('joined')).slot, 1);
});

async function create(url) {
  const host = await connect(url);
  host.send({ type: 'create', ...compatibility });
  return { host, room: await host.next('created') };
}

test('incomplete TURN configuration fails before the service starts', () => {
  for (const options of [{turnUrl: 'turn:relay.example:3478'}, {turnSecret: 'test-secret'}])
    assert.throws(() => createRoomService(options), /TURN_URL and TURN_SECRET must be configured together/);
});

test('room rejections distinguish permanent failures from temporary host or service unavailability', async t => {
  const url = await setup(t);
  const guest = await connect(url);
  guest.send({type:'resume', room:'missing-room', resume:'old-token', ...compatibility});
  assert.deepEqual(await guest.next('error'), {type:'error', reason:'Room unavailable', retryable:false});
  const {host,room} = await create(url);
  guest.send({type:'join', room:room.room, ...compatibility, resources:'wrong-resources'});
  assert.deepEqual(await guest.next('error'), {type:'error', reason:'Incompatible game resources or protocol', retryable:false});
  const observer = await connect(url);
  observer.send({type:'join', room:room.room, ...compatibility});
  await observer.next('joined');
  await host.next('peer');
  host.socket.terminate();
  await observer.next('host-wait');
  guest.send({type:'join', room:room.room, ...compatibility});
  assert.deepEqual(await guest.next('error'), {type:'error', reason:'Host reconnecting', retryable:true});
  const full = await connect(await setup(t, {maxRooms:0}));
  full.send({type:'create', ...compatibility});
  assert.deepEqual(await full.next('error'), {type:'error', reason:'Service full', retryable:true});
});

test('TURN endpoints are bounded and share valid temporary credentials', async t => {
  const secret = 'isolated-turn-list-secret';
  const endpoints = ['turn:relay.example:3478?transport=udp',
    'turn:relay.example:3478?transport=tcp', 'turns:relay.example:5349?transport=tcp'];
  const url = await setup(t, {stunUrl:'stun:relay.example:3478',
    turnUrl:` ${endpoints.join(', ')} `, turnSecret:secret});
  const {room} = await create(url);
  assert.equal(room.iceServers[0].urls, 'stun:relay.example:3478');
  const relays = room.iceServers.slice(1);
  assert.deepEqual(relays.map(server => server.urls), endpoints);
  for (const relay of relays) {
    assert.equal(relay.username, relays[0].username);
    assert.equal(relay.credential, createHmac('sha1', secret).update(relay.username).digest('base64'));
  }
  for (const turnUrl of [' ', ',', 'turn:relay.example,', 'https://relay.example',
    'turn:bad host', 'turn:', `turn:${'x'.repeat(2044)}`, endpoints[0] + ',\x00',
    Array(8).fill(endpoints[0]).join(',')])
    assert.throws(() => createRoomService({turnUrl, turnSecret:secret}), /one to seven/);
  const maximum = await setup(t, {stunUrl:'stun:relay.example:3478',
    turnUrl:Array(7).fill(endpoints[0]).join(','), turnSecret:secret});
  assert.equal((await create(maximum)).room.iceServers.length, 8);
});

test('late joins and host recovery receive fresh TURN credentials before negotiation', async t => {
  let now = Date.now();
  t.mock.method(Date, 'now', () => now);
  const secret = 'isolated-turn-test-secret';
  const url = await setup(t, {turnUrl:'turn:relay.example:3478', turnSecret:secret});
  const {host,room} = await create(url);
  const expiry = ice => Number(ice[0].username.split(':')[0]);
  const check = ice => {
    assert.equal(ice.length, 1);
    assert.equal(ice[0].urls, 'turn:relay.example:3478');
    assert.equal(expiry(ice), Math.floor(now/1000)+3600);
    assert.equal(ice[0].credential, createHmac('sha1',secret).update(ice[0].username).digest('base64'));
  };
  check(room.iceServers);
  now += 3601000;
  assert.ok(expiry(room.iceServers) < now/1000);
  const guest = await connect(url);
  guest.send({type:'join', room:room.room, ...compatibility});
  const joined = await guest.next('joined');
  check(joined.iceServers);
  check((await host.next('peer')).iceServers);

  const order = [];
  guest.socket.on('message', bytes => order.push(JSON.parse(bytes.toString()).type));
  host.socket.terminate();
  await guest.next('host-wait');
  now += 3601000;
  assert.ok(expiry(joined.iceServers) < now/1000);
  const returning = await connect(url, host.credential);
  returning.send({type:'resume', room:room.room, resume:room.resume, ...compatibility});
  check((await returning.next('created')).iceServers);
  check((await returning.next('peer')).iceServers);
  returning.send({type:'signal', to:joined.slot, sdp:'new-offer', descriptionType:'offer'});
  check((await guest.next('host-ready')).iceServers);
  assert.equal((await guest.next('signal')).sdp, 'new-offer');
  assert.ok(order.indexOf('host-ready') < order.indexOf('signal'));
});

test('website serves only built application artifacts and never local game resources', async t => {
  const root = await mkdtemp(join(tmpdir(), 'kf-room-web-'));
  t.after(() => rm(root, { recursive: true, force: true }));
  await writeFile(join(root, 'kings-field.html'), '<title>Game</title>');
  await writeFile(join(root, 'kings-field.wasm'), new Uint8Array([0, 97, 115, 109]));
  await writeFile(join(root, 'PSX.EXE'), 'private local resource');
  const url = (await setup(t, { webRoot: root })).replace(/^ws:/, 'http:');
  const page = await fetch(url);
  assert.equal(page.status, 200);
  assert.equal(await page.text(), '<title>Game</title>');
  const wasm = await fetch(url + '/kings-field.wasm', { method: 'HEAD' });
  assert.equal(wasm.status, 200);
  assert.equal(wasm.headers.get('content-type'), 'application/wasm');
  assert.equal(wasm.headers.get('content-length'), '4');
  assert.equal((await wasm.arrayBuffer()).byteLength, 0);
  assert.equal((await fetch(url + '/PSX.EXE')).status, 404);
  assert.equal((await fetch(url + '/rooms', { method: 'POST', body: 'no file uploads' })).status, 404);
});

test('invite rooms enforce compatibility, capacity, and host-only routing', async t => {
  const url = await setup(t);
  const { host, room } = await create(url);
  assert.equal(room.slot, 0);
  const guests = [];
  for (let slot = 1; slot < 4; ++slot) {
    const guest = await connect(url);
    guest.send({ type: 'join', room: room.room, ...compatibility });
    assert.equal((await guest.next('joined')).slot, slot);
    assert.equal((await host.next('peer')).slot, slot);
    guests.push(guest);
  }
  guests[0].send({ type: 'signal', to: 0, from: 3, sdp: 'test-offer', descriptionType: 'offer' });
  assert.deepEqual(await host.next('signal'), { type: 'signal', from: 1,
    sdp: 'test-offer', descriptionType: 'offer' });
  guests[0].send({ type: 'signal', to: 2, candidate: 'candidate:test', mid: '0' });
  assert.equal((await guests[0].next('error')).reason, 'Invalid signaling route');
  const extra = await connect(url);
  extra.send({ type: 'join', room: room.room, ...compatibility, protocol: 5 });
  assert.match((await extra.next('error')).reason, /Incompatible/);
  extra.send({ type: 'join', room: room.room, ...compatibility, resources: 'different' });
  assert.match((await extra.next('error')).reason, /Incompatible/);
  extra.send({ type: 'join', room: room.room, ...compatibility });
  assert.equal((await extra.next('error')).reason, 'Party full');
});

test('an excess client sending a malformed frame cannot crash active rooms', async t => {
  const url = await setup(t, { maxClients: 1 });
  const { host } = await create(url);
  const address = new URL(url);
  const rejected = connectTcp(Number(address.port), address.hostname);
  t.after(() => rejected.destroy());
  await once(rejected, 'connect');
  rejected.write('GET / HTTP/1.1\r\nHost: localhost\r\nUpgrade: websocket\r\n' +
    'Connection: Upgrade\r\nSec-WebSocket-Key: MDEyMzQ1Njc4OWFiY2RlZg==\r\n' +
    'Sec-WebSocket-Version: 13\r\n\r\n');
  const [upgrade] = await once(rejected, 'data');
  assert.match(upgrade.toString(), /^HTTP\/1.1 101 /);
  // The service has sent a capacity close, but the receiver remains active
  // until the handshake finishes. An unmasked client frame raises its error.
  const closed = once(rejected, 'close');
  rejected.end(Buffer.from([0x81, 0x01, 0x78]));
  await closed;
  assert.equal(await (await fetch(url.replace(/^ws:/, 'http:') + '/health')).text(), 'ok\n');
  host.send({ type: 'leave' });
  await host.next('ended');
});

test('reconnect tokens preserve slots, and host loss ends the session without migration', async t => {
  const url = await setup(t, { hostGraceMs: 50 });
  const { host, room } = await create(url);
  const guest = await connect(url);
  guest.send({ type: 'join', room: room.room, ...compatibility });
  const joined = await guest.next('joined');
  await host.next('peer');
  guest.socket.close();
  assert.equal((await host.next('peer-left')).slot, 1);
  const rejoined = await connect(url, guest.credential);
  rejoined.send({ type: 'resume', room: room.room, resume: 'é'.repeat(32), ...compatibility });
  assert.equal((await rejoined.next('error')).reason, 'Invalid reconnect token');
  rejoined.send({ type: 'resume', room: room.room, resume: joined.resume, ...compatibility });
  assert.equal((await rejoined.next('joined')).slot, 1);
  await host.next('peer');
  host.socket.close();
  await rejoined.next('host-wait');
  await rejoined.next('ended');
});

test('profiles reclaim their saved slots across room creation and reverse join order', async t => {
  const url = await setup(t);
  const { host, room } = await create(url);
  const first = await connect(url), second = await connect(url);
  first.send({ type:'join', room:room.room, ...compatibility });
  const a = await first.next('joined');
  const announced = await host.next('peer');
  assert.equal(announced.identity, a.identity);
  assert.equal(a.identity, createHash('sha256').update(Buffer.from(first.credential, 'hex')).digest('hex'));
  second.send({ type:'join', room:room.room, ...compatibility });
  const b = await second.next('joined');
  await host.next('peer');
  first.socket.close();
  await host.next('peer-left');
  // A fresh process has only its persisted profile, not the old room token.
  const reloaded = await connect(url, first.credential);
  reloaded.send({ type:'join', room:room.room, ...compatibility });
  const reload = await reloaded.next('joined');
  assert.equal(reload.slot, a.slot);
  assert.equal(reload.identity, a.identity);
  assert.equal(reload.resumed, true);
  await host.next('peer');
  const impostor = await connect(url);
  impostor.send({ type:'resume', room:room.room, resume:a.resume, ...compatibility });
  assert.match((await impostor.next('error')).reason, /Invalid reconnect token/);
  host.send({ type:'leave' });
  await host.next('ended');
  const nextHost = await connect(url, host.credential);
  const roster = [room.identity, a.identity, b.identity, '0'.repeat(64)];
  nextHost.send({ type:'create', roster, ...compatibility });
  const next = await nextHost.next('created');
  const backSecond = await connect(url, second.credential);
  backSecond.send({ type:'join', room:next.room, ...compatibility });
  assert.equal((await backSecond.next('joined')).slot, b.slot);
  await nextHost.next('peer');
  const stranger = await connect(url, a.identity); // Public IDs are not credentials.
  stranger.send({ type:'join', room:next.room, ...compatibility });
  assert.equal((await stranger.next('joined')).slot, 3, 'A new profile inherited an absent player');
  await nextHost.next('peer');
  const backFirst = await connect(url, first.credential);
  backFirst.send({ type:'join', room:next.room, ...compatibility });
  assert.equal((await backFirst.next('joined')).slot, a.slot);
  const wrongHost = await connect(url);
  wrongHost.send({ type:'create', roster, ...compatibility });
  assert.match((await wrongHost.next('error')).reason, /different player/);
  wrongHost.send({ type:'create', credential:'x'.repeat(64), ...compatibility });
  assert.match((await wrongHost.next('error')).reason, /Invalid player credential/);
});

test('signals cannot cross room boundaries', async t => {
  const url = await setup(t);
  const first = await create(url), second = await create(url);
  const guest = await connect(url);
  guest.send({ type: 'join', room: first.room.room, ...compatibility });
  await guest.next('joined');
  await first.host.next('peer');
  guest.send({ type: 'signal', room: second.room.room, to: 0, candidate: 'candidate:test', mid: '0' });
  assert.equal((await first.host.next('signal')).from, 1);
  // The receiver is derived exclusively from the authenticated socket's room.
  second.host.send({ type: 'leave' });
  await second.host.next('ended');
  guest.send({ type: 'signal', to: 0, candidate: 'candidate:still-connected', mid: '0' });
  assert.equal((await first.host.next('signal')).candidate, 'candidate:still-connected');
});

test('forwarding backlog closes the flooding sender and preserves the host room', async t => {
  const url = await setup(t);
  const {host, room} = await create(url);
  const flood = await connect(url);
  flood.send({type:'join', room:room.room, ...compatibility});
  await flood.next('joined');
  await host.next('peer');
  let hostClosed = false;
  host.socket.on('close', () => { hostClosed = true; });
  host.socket._socket.pause();
  t.after(() => host.socket._socket?.resume());
  const closed = once(flood.socket, 'close');
  const timer = setTimeout(() => flood.socket.terminate(), 3000);
  for (let i = 0; i < 90; ++i)
    flood.send({type:'signal', to:0, descriptionType:'answer', sdp:'x'.repeat(60000)});
  const [code, reason] = await closed;
  clearTimeout(timer);
  host.socket._socket.resume();
  assert.equal(code, 1008);
  assert.equal(reason.toString(), 'Signaling backlog');
  assert.equal((await host.next('peer-left')).slot, 1);
  const healthy = await connect(url);
  healthy.send({type:'join', room:room.room, ...compatibility});
  const joined = await healthy.next('joined');
  assert.equal((await host.next('peer')).slot, joined.slot);
  healthy.send({type:'signal', to:0, candidate:'candidate:healthy', mid:'0'});
  let message;
  do { message = await host.next('signal'); } while (message.from !== joined.slot);
  assert.equal(message.candidate, 'candidate:healthy');
  assert.equal(hostClosed, false);
});

test('host forwarding to a slow guest closes only that guest', async t => {
  const url = await setup(t);
  const {host, room} = await create(url);
  const slow = await connect(url);
  slow.send({type:'join', room:room.room, ...compatibility});
  const joined = await slow.next('joined');
  await host.next('peer');
  let hostClosed = false;
  host.socket.on('close', () => { hostClosed = true; });
  slow.socket._socket.pause();
  t.after(() => slow.socket._socket?.resume());
  const closed = once(slow.socket, 'close');
  const timer = setTimeout(() => slow.socket.terminate(), 3000);
  for (let i = 0; i < 90; ++i)
    host.send({type:'signal', to:joined.slot, descriptionType:'offer', sdp:'x'.repeat(60000)});
  await new Promise(resolve => setTimeout(resolve, 100));
  slow.socket._socket.resume();
  const [code, reason] = await closed;
  clearTimeout(timer);
  assert.equal(code, 1008);
  assert.equal(reason.toString(), 'Signaling backlog');
  assert.equal((await host.next('peer-left')).slot, joined.slot);
  assert.equal(hostClosed, false);
  host.send({type:'leave'});
  await host.next('ended');
});
