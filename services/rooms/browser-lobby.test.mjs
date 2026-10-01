// Real browser import -> host -> share code -> join. Optional --menus adds one
// starting-area menu action; use only when scripted gameplay tests are authorized.
// Run: node services/rooms/browser-lobby.test.mjs build/wasm /path/to/Japanese.bin
//   /tmp/lobby.png --menus /path/to/KFIII.bin [reference.kfa]
// --crossplay uses the last two arguments for the native game executable and
// matching Japanese resource directory, after the character pack/disc.
// --preview checks local selection/rendering and startup cleanup, with no gameplay input.
// --preview-only skips gameplay, TURN and KF1 import; use '-' for the Japanese disc argument.
// KF_TEST_AVATAR selects the guest body for the short --pose scene (default 41).
// --combat uses the native executable/data arguments and checks lethal friendly
// fire with one short starting-area step and bounded weapon input.
// --save-flow uses those executable/data arguments to prepare a direct save-point
// fixture, then exercises saving, revival, the return-staff menu/load barrier
// and rehosting from the manual checkpoint.
// --save-failure first injects a synchronous IndexedDB quota failure, checks
// that the checkpoint/dead guest stay unchanged, then retries the save flow.
// --travel-flow places the party at the real floor-five gate for a short step
// out/back, a floor-four load barrier and reconnect, without level navigation.
// --wipe-flow retains enemies there and checks death notices/checkpoint restores
// without gameplay input; it shares the native executable/resource arguments.
// --travel-crossplay adds two native guests to the gate check, filling all four
// slots, and reconnects a browser and native guest after the floor transition.
// --spell-flow seeds a starting-area Fire Ball fixture, then checks one real
// guest cast, lethal friendly fire, no PvP rewards and reconnect.
import { createRoomService } from './server.mjs';
import { once } from 'node:events';
import { spawn, execFile } from 'node:child_process';
import { promisify } from 'node:util';
import { access, mkdtemp, readFile, rm, writeFile } from 'node:fs/promises';
import { dirname, join, resolve } from 'node:path';
import { tmpdir } from 'node:os';
import { createServer as tcpServer, connect as tcpConnect } from 'node:net';
import { randomBytes, createHash } from 'node:crypto';
import assert from 'node:assert/strict';
import WebSocket from 'ws';

const [build, disc, screenshot, scenario, avatars, reference, nativeResources] = process.argv.slice(2);
const previewOnly = scenario === '--preview-only'; // Character resources only; never starts gameplay.
const crossplay = ['--crossplay','--combat'].includes(scenario);
const nativeFixture = crossplay || ['--save-flow','--save-failure','--travel-flow','--travel-crossplay','--wipe-flow','--spell-flow'].includes(scenario);
const standing = ['--standing-avatar', '--pose', '--prediction'].includes(scenario);
const standingAvatar = Number(process.env.KF_TEST_AVATAR ?? 41);
assert.ok(Number.isInteger(standingAvatar) && standingAvatar >= 0 && standingAvatar < 44,
  'KF_TEST_AVATAR must be a character slot');
assert.ok(build && disc && avatars, 'Expected WASM build, Japanese disc, screenshot, scenario, and local KFIII disc or character pack');
if (!previewOnly) await access(resolve(disc));
if (nativeFixture) assert.ok(reference && nativeResources, 'This scenario requires a native executable and matching resources');
const referenceHash = reference && !nativeFixture ? createHash('sha256').update(await readFile(reference)).digest('hex') : null;
const profile = await mkdtemp(join(tmpdir(), 'kf-browser-party-'));
const reservation = tcpServer();
reservation.listen(0, '127.0.0.1');
await once(reservation, 'listening');
const turnPort = reservation.address().port;
await new Promise(resolve => reservation.close(resolve));
const secret = randomBytes(24).toString('hex');
const service = createRoomService({ webRoot: resolve(build),
  stunUrl: `stun:127.0.0.1:${turnPort}`, turnUrl: `turn:127.0.0.1:${turnPort}?transport=tcp`, turnSecret: secret });
const connections = [];
service.server.on('upgrade', (_request, socket) => connections.push(socket));
service.server.listen(0, '127.0.0.1');
await once(service.server, 'listening');
const url = `http://127.0.0.1:${service.server.address().port}/`;
const pause = ms => new Promise(resolve => setTimeout(resolve, ms));
let browser, debuggerSocket, turn;
const nativeClients = [];
let turnLog = '';
const logs = new Map(), failures = [];
async function nativePlayer(args) {
  const saves = await mkdtemp(join(profile, 'native-'));
  const child = spawn('xvfb-run', ['-a', resolve(reference), '--data', resolve(nativeResources),
    '--saves', saves, '--signal', url.replace(/^http/, 'ws'),
    /\.kfa$/i.test(avatars) ? '--avatars' : '--avatar-disc', resolve(avatars), ...args], {
    detached: true, env: { ...process.env, SDL_AUDIODRIVER: 'dummy' }, stdio: ['ignore', 'pipe', 'pipe']
  });
  child.output = '';
  nativeClients.push(child);
  for (const stream of [child.stdout, child.stderr]) stream.on('data', bytes => {
    child.output += bytes; process.stdout.write(bytes);
  });
  return child;
}
async function nativeUntil(child, pattern, after = 0) {
  for (let i = 0; i < 450; ++i) {
    assert.equal(child.exitCode, null, child.output);
    assert.doesNotMatch(child.output, /Rejected invalid|Cannot encode|AddressSanitizer|runtime error:/);
    const found = child.output.slice(after).match(pattern);
    if (found) return found;
    await pause(100);
  }
  throw new Error(`Native game timed out waiting for ${pattern}: ${child.output}`);
}
try {
  if (!previewOnly) {
  turn = spawn('turnserver', ['-n', '--listening-ip=127.0.0.1', '--relay-ip=127.0.0.1',
    `--listening-port=${turnPort}`, '--min-port=49160', '--max-port=49260',
    '--no-cli', '--no-tls', '--no-dtls', '--use-auth-secret', `--static-auth-secret=${secret}`,
    '--realm=kf-local-test', '--allow-loopback-peers', '--no-multicast-peers',
    `--pidfile=${join(profile, 'turn.pid')}`, `--userdb=${join(profile, 'turn.sqlite')}`, '--log-file=stdout'],
    { stdio: ['ignore', 'pipe', 'pipe'] });
  for (const stream of [turn.stdout, turn.stderr]) stream.on('data', bytes => { turnLog = (turnLog + bytes).slice(-12000); });
  let listening = false;
  for (let i = 0; i < 100; ++i) {
    listening = await new Promise(resolve => {
      const probe = tcpConnect(turnPort, '127.0.0.1');
      probe.on('connect', () => { probe.destroy(); resolve(true); });
      probe.on('error', () => resolve(false));
    });
    if (listening) break;
    await pause(50);
  }
  assert.ok(listening, `Local TURN server failed: ${turnLog}`);
  }
  browser = spawn('chromium', ['--headless', '--no-sandbox', '--disable-dev-shm-usage',
    '--enable-unsafe-swiftshader', '--use-gl=angle', '--use-angle=swiftshader',
    '--remote-debugging-port=0', `--user-data-dir=${profile}`, 'about:blank'], { stdio: 'ignore' });
  let endpoint;
  for (let i = 0; i < 100; ++i) {
    try {
      const [port, path] = (await readFile(join(profile, 'DevToolsActivePort'), 'utf8')).trim().split('\n');
      endpoint = `ws://127.0.0.1:${port}${path}`;
      break;
    } catch { await pause(100); }
  }
  assert.ok(endpoint, 'Chromium did not start');
  debuggerSocket = new WebSocket(endpoint);
  await once(debuggerSocket, 'open');
  let sequence = 0;
  const pending = new Map();
  debuggerSocket.on('message', bytes => {
    const message = JSON.parse(bytes.toString());
    if (message.method === 'Runtime.consoleAPICalled') {
      const line = message.params.args.map(arg => arg.value ?? arg.description ?? '').join(' ');
      logs.get(message.sessionId)?.push(line);
      if (/Online |World synchronized|Player \d|interrupted|Host connection/.test(line)) console.log(line);
    }
    if (message.method === 'Runtime.exceptionThrown') failures.push(message.params.exceptionDetails);
    const callback = pending.get(message.id);
    if (!callback) return;
    pending.delete(message.id);
    clearTimeout(callback.timer);
    if (message.error) callback.reject(new Error(JSON.stringify(message.error)));
    else callback.resolve(message.result);
  });
  function cdp(method, params = {}, sessionId) {
    return new Promise((resolve, reject) => {
      const id = ++sequence;
      const timer = setTimeout(() => { pending.delete(id); reject(new Error(`CDP timeout: ${method}`)); }, 60000);
      pending.set(id, { resolve, reject, timer });
      debuggerSocket.send(JSON.stringify({ id, method, params, sessionId }));
    });
  }
  async function evaluate(session, expression) {
    const result = await cdp('Runtime.evaluate', { expression, returnByValue: true, awaitPromise: true, userGesture: true }, session);
    assert.ok(!result.exceptionDetails, JSON.stringify(result.exceptionDetails));
    return result.result.value;
  }
  async function until(session, expression, allowNavigation = false) {
    for (let i = 0; i < 450; ++i) {
      try {
        if (await evaluate(session, expression)) return;
      } catch (error) {
        // Page.reload acknowledges before the old execution context disappears.
        if (!allowNavigation || !/Inspected target navigated or closed|Execution context was destroyed|Cannot find context with specified id/.test(error.message)) throw error;
      }
      assert.deepEqual(failures, []);
      await pause(100);
    }
    for (const page of logs.keys()) {
      console.error('Browser network state:', JSON.stringify(await evaluate(page, `Array.from(Module.kfTransports?.values() ?? [], s =>
        ({socket: s.socket.readyState, peers: Array.from(s.peers.entries(), ([slot, p]) =>
          ({slot, connection: p.pc.connectionState, ice: p.pc.iceConnectionState, signaling: p.pc.signalingState,
            channels: p.channels.map(c => ({state: c.readyState, buffered: c.bufferedAmount}))}))}))`)));
      console.error('Browser log:', logs.get(page));
    }
    throw new Error(`Timed out: ${expression}\n${await evaluate(session, 'document.body.innerText')}`);
  }
  async function player(avatar = 41) {
    const { browserContextId } = await cdp('Target.createBrowserContext');
    const { targetId } = await cdp('Target.createTarget', { url: 'about:blank', browserContextId });
    const { sessionId } = await cdp('Target.attachToTarget', { targetId, flatten: true });
    logs.set(sessionId, []);
    await cdp('Runtime.enable', {}, sessionId);
    await cdp('Page.enable', {}, sessionId);
    // Force the NAT fallback so a successful localhost direct path cannot hide
    // broken credential issuance, relay allocation, or relayed game traffic.
    await cdp('Page.addScriptToEvaluateOnNewDocument', { source: `const OriginalPeer = RTCPeerConnection;
      window.avatarRequests = []; window.actionReplies = [];
      window.roomAdmissions = [];
      window.fullWorld = null; window.worldTransfer = null;
      window.clientHeader = null;
      window.waitingEpochs = []; window.loadedWorlds = []; window.heldLoaded = null;
      window.latestSnapshotEpoch = 0; window.firstTravelTick = null;
      window.delayedSnapshots = 0; window.droppedSnapshots = 0;
      window.latestSnapshot = 0;
      const OriginalSocket = WebSocket;
      window.WebSocket = class extends OriginalSocket {
        constructor(...args) {
          super(...args);
          this.addEventListener('message', event => {
            const m = JSON.parse(event.data);
            if (m.type === 'created' || m.type === 'joined')
              roomAdmissions.push({slot:m.slot, identity:m.identity, resumed:m.resumed});
          });
        }
        send(text) {
          const m = JSON.parse(text);
          if (m.type === 'create' || m.type === 'join')
            window.roomCompatibility = {resources:m.resources, recipe:m.recipe};
          return super.send(text);
        }
      };
      window.RTCPeerConnection = class extends OriginalPeer {
        constructor(options) { super({...options, iceTransportPolicy: 'relay'}); }
        createDataChannel(label, options) {
          const channel = super.createDataChannel(label, options);
          const send = channel.send.bind(channel);
          channel.send = bytes => {
            const b = bytes instanceof ArrayBuffer ? new DataView(bytes) : new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
            if (b.byteLength === 19 && b.getUint8(6) === 5) {
              const loaded = {epoch:b.getUint32(7,true),tick:b.getUint32(11,true)};
              if (window.holdLoaded) {
                const copy = bytes.slice();
                window.heldLoaded = loaded;
                window.releaseLoaded = () => {
                  window.holdLoaded=false; loadedWorlds.push(loaded); send(copy); window.heldLoaded=null;
                };
                return;
              }
              loadedWorlds.push(loaded);
            }
            if (b.byteLength >= 19 && b.getUint8(6) === 1)
              clientHeader = new Uint8Array(b.buffer,b.byteOffset,19).slice();
            if (b.byteLength >= 19 && b.getUint8(6) === 2) {
              if (window.dropSnapshots) { ++window.droppedSnapshots; return; }
              if (window.snapshotDelay) {
                ++window.delayedSnapshots;
                const copy = bytes.slice();
                setTimeout(() => { if (channel.readyState === 'open') send(copy); }, window.snapshotDelay);
                return;
              }
            }
            if (b.byteLength === 28 && b.getUint8(6) === 4 && b.getUint8(19) === 13)
              avatarRequests.push({sequence:b.getUint32(11, true), slot:b.getUint16(20, true)});
            return send(bytes);
          };
          channel.addEventListener('message', event => {
            const b = new DataView(event.data);
            if (b.byteLength >= 19 && b.getUint8(6) === 2) {
              latestSnapshot = b.getUint32(11, true);
              latestSnapshotEpoch = b.getUint32(7,true);
              if (latestSnapshotEpoch === window.travelEpoch && firstTravelTick === null) firstTravelTick=latestSnapshot;
            }
            if (b.byteLength === 19 && b.getUint8(6) === 7) waitingEpochs.push(b.getUint32(7,true));
            if (b.byteLength === 19 && [9,10].includes(b.getUint8(6)))
              actionReplies.push({sequence:b.getUint32(11, true), kind:b.getUint8(6)});
            if (b.byteLength > 27 && b.getUint8(6) === 3) {
              const total = b.getUint32(19,true), offset = b.getUint32(23,true);
              if (total > 132*1024 || offset + b.byteLength - 27 > total) throw Error('Invalid fixture transfer');
              if (offset === 0) worldTransfer = {bytes:new Uint8Array(total), received:0};
              if (worldTransfer?.bytes.length === total && worldTransfer.received === offset) {
                worldTransfer.bytes.set(new Uint8Array(event.data,27),offset);
                worldTransfer.received += b.byteLength - 27;
                if (worldTransfer.received === total) fullWorld = worldTransfer.bytes;
              }
            }
          });
          return channel;
        }
      };` }, sessionId);
    await cdp('Page.navigate', { url }, sessionId);
    await until(sessionId, "document.getElementById('disc') && !document.getElementById('disc').disabled");
    assert.equal(await evaluate(sessionId, 'hostButton.disabled && joinButton.disabled'), true,
      'Online play enabled before resource import');
    const document = await cdp('DOM.getDocument', {}, sessionId);
    if (!previewOnly) {
    const input = await cdp('DOM.querySelector', { nodeId: document.root.nodeId, selector: '#disc' }, sessionId);
    await cdp('DOM.setFileInputFiles', { nodeId: input.nodeId, files: [resolve(disc)] }, sessionId);
    await until(sessionId, '!playButton.disabled');
    assert.equal(await evaluate(sessionId, 'hostButton.disabled && joinButton.disabled'), true,
      'Online play enabled before character import');
    assert.equal(await evaluate(sessionId, 'verifyFiles(collectedFiles(dataRoot))'), true);
    }
    if (avatars) {
      const input = await cdp('DOM.querySelector', { nodeId: document.root.nodeId, selector: '#avatars' }, sessionId);
      await cdp('DOM.setFileInputFiles', { nodeId: input.nodeId, files: [resolve(avatars)] }, sessionId);
      await until(sessionId, 'avatarPath !== null && !avatarSelect.disabled');
      assert.equal(await evaluate(sessionId, 'avatarSelect.options.length'), 39);
      if (referenceHash) assert.equal(await evaluate(sessionId, `(async () => {
        const hash = await crypto.subtle.digest('SHA-256', Module.FS.readFile(avatarPath));
        return Array.from(new Uint8Array(hash), b => b.toString(16).padStart(2, '0')).join('');
      })()`), referenceHash, 'Browser extraction differs from the independent reference pack');
      await evaluate(sessionId, `avatarSelect.value = '${avatar}'; avatarSelect.dispatchEvent(new Event('change'));`);
      if (!previewOnly) await until(sessionId, '!hostButton.disabled && !joinButton.disabled');
    }
    return sessionId;
  }
  if (scenario === '--combat') {
    const native = await nativePlayer(['--host']);
    const [, code] = await nativeUntil(native, /Online room: ([^;]+);/);
    const guest = await player(41);
    await evaluate(guest, `joinCode.value=${JSON.stringify(code)}; joinButton.click();`);
    await until(guest, 'statusLabel.textContent.startsWith("Connected.") && clientHeader !== null');
    await nativeUntil(native, /Player 2 admitted/);
    const admitted = await evaluate(guest, 'roomAdmissions.at(-1)');
    async function summary() {
      await evaluate(guest, `(() => { fullWorld=null;
        const peer=Array.from(Array.from(Module.kfTransports.values())[0].peers.values())[0];
        const request=clientHeader.slice(); request[6]=6; peer.channels[0].send(request); })()`);
      await until(guest, 'fullWorld !== null');
      const tick = await evaluate(guest, 'new DataView(fullWorld.buffer).getUint32(11,true)');
      await until(guest, `latestSnapshot > ${tick}`);
      const path = join(profile,'combat-world.kfs');
      await writeFile(path, Buffer.from(await evaluate(guest, 'Array.from(fullWorld)')));
      const {stdout} = await promisify(execFile)(join(dirname(resolve(reference)),'coop-runtime-test'),
        ['--snapshot-summary',path], {timeout:10000});
      return JSON.parse(stdout);
    }
    async function key(key, code, windowsVirtualKeyCode, duration) {
      await cdp('Input.dispatchKeyEvent', {type:'keyDown',key,code,windowsVirtualKeyCode}, guest);
      await pause(duration);
      await cdp('Input.dispatchKeyEvent', {type:'keyUp',key,code,windowsVirtualKeyCode}, guest);
    }
    const before = await summary();
    assert.ok(before.hp > 0, 'Native host did not start alive');
    await evaluate(guest, 'Module.canvas.focus()');
    await key('s','KeyS',83,250);
    await pause(500);
    let after = before;
    for (let swing=0; swing<36 && after.hp; ++swing) {
      await key(' ','Space',32,120);
      await pause(2500);
      after = await summary();
      console.log(`Friendly-fire swing ${swing+1}: host HP ${after.hp}/${after.maximum_hp}`);
      assert.ok(after.hp <= before.hp, 'Combat unexpectedly healed the host');
      if (swing === 2) assert.ok(after.hp < before.hp, 'Starting-area melee did not reach the host');
    }
    assert.equal(after.hp,0,'Guest melee could not kill the native host');
    assert.match(native.output,/Player 1 died; spectating/,'Dead host did not enter spectator presentation');
    assert.equal(after.experience,before.experience,'Friendly-fire victim gained experience');
    const tick = await evaluate(guest,'latestSnapshot');
    await pause(750);
    assert.ok(await evaluate(guest, `latestSnapshot > ${tick} && !stopped`), 'Host death stopped the shared world');
    connections.at(-1).destroy();
    await until(guest, `roomAdmissions.length>1 && roomAdmissions.at(-1).resumed && statusLabel.textContent.startsWith('Connected.')`);
    const resumed = await evaluate(guest, 'roomAdmissions.at(-1)');
    assert.equal(resumed.slot,admitted.slot,'Combat reconnect changed party membership');
    assert.equal(resumed.identity,admitted.identity,'Combat reconnect changed player identity');
    assert.equal((await summary()).hp,0,'Reconnect revived the dead host');
    assert.doesNotMatch(native.output,/Rejected invalid|Cannot encode|AddressSanitizer|runtime error:/);
    assert.doesNotMatch(logs.get(guest).join('\n'),/Rejected invalid|Cannot encode|Application stopped|deadline exceeded/);
    assert.deepEqual(failures,[]);
    if (screenshot) {
      const shot=await cdp('Page.captureScreenshot',{format:'png'},guest);
      await writeFile(screenshot,Buffer.from(shot.data,'base64'));
    }
    console.log('Browser guest dealt lethal melee friendly fire to the native host; world and party survived reconnect');
  } else if (['--save-flow','--save-failure','--travel-flow','--travel-crossplay','--wipe-flow','--spell-flow'].includes(scenario)) {
    const spell = scenario === '--spell-flow';
    const entrance = !spell && !['--save-flow','--save-failure'].includes(scenario);
    const wipe = scenario === '--wipe-flow';
    const host = await player(41), guest = await player(spell ? 24 : 41);
    async function hostRoom(saved = false) {
      await evaluate(host, `campaignSelect.value='${saved ? 1 : 0}'; hostButton.click()`);
      await until(host, '!document.getElementById("room-panel").hidden');
      return evaluate(host, 'document.getElementById("room-code").textContent');
    }
    async function joinRoom(code, status = 'Connected.') {
      await evaluate(guest, `joinCode.value=${JSON.stringify(code)}; joinButton.click()`);
      await until(guest, `statusLabel.textContent.startsWith(${JSON.stringify(status)}) && fullWorld !== null`);
      return evaluate(guest, 'roomAdmissions.at(-1)');
    }
    async function closeRoom() {
      await evaluate(host, "Array.from(Module.kfTransports.values())[0].socket.send(JSON.stringify({type:'leave'}))");
      for (const page of [host, guest]) await until(page, 'stopped');
      for (const page of [host, guest]) {
        await cdp('Page.reload', {}, page);
        await until(page, 'typeof hostButton !== "undefined" && !hostButton.disabled', true);
      }
    }
    async function captureWorld() {
      await until(guest, 'clientHeader !== null');
      await evaluate(guest, `(() => { fullWorld=null;
        const peer=Array.from(Array.from(Module.kfTransports.values())[0].peers.values())[0];
        const request=clientHeader.slice(); request[6]=6; peer.channels[0].send(request); })()`);
      await until(guest, 'fullWorld !== null');
      const tick = await evaluate(guest, 'new DataView(fullWorld.buffer).getUint32(11,true)');
      await until(guest, `latestSnapshot > ${tick}`);
      return Buffer.from(await evaluate(guest, 'Array.from(fullWorld)'));
    }
    async function fixture(mode, bytes) {
      const input = join(profile, 'save-input.kfs'), output = join(profile, 'save-output.kfs');
      await writeFile(input, bytes);
      const args = ['-a', join(dirname(resolve(reference)), 'coop-runtime-test'), mode, resolve(nativeResources), input];
      if (mode !== '--inspect-world') args.push(output);
      const {stdout, stderr} = await promisify(execFile)('xvfb-run', args,
        {timeout:30000, env:{...process.env, SDL_AUDIODRIVER:'dummy'}});
      assert.doesNotMatch(stdout + stderr, /AddressSanitizer|runtime error:/);
      const summary = JSON.parse(stdout.match(/^FIXTURE (.+)$/m)?.[1] ?? 'null');
      assert.ok(summary, stdout + stderr);
      return {summary, bytes:mode !== '--inspect-world' ? await readFile(output) : bytes};
    }
    const originalCode = await hostRoom();
    const originalAdmission = await joinRoom(originalCode);
    const snapshot = await captureWorld();
    if (screenshot) await writeFile(screenshot + '.input.kfs', snapshot);
    const compatibility = await evaluate(host, 'roomCompatibility');
    await closeRoom();
    const prepared = await fixture(spell ? '--spell-fixture' : wipe ? '--wipe-fixture' : entrance ? '--travel-fixture' : '--save-fixture', snapshot);
    if (screenshot) await writeFile(screenshot + '.prepared.kfs', prepared.bytes);
    assert.equal(prepared.summary.players[1].presence, entrance || spell ? 2 : 3);
    if (!entrance && !spell) assert.equal(prepared.summary.players[1].hp, 0);
    const header = Buffer.alloc(134);
    header.write('KFC1'); header.writeUInt16LE(1,4);
    header.write(createHash('sha256').update(compatibility.resources+'\0'+compatibility.recipe+'\0').digest('hex'),6);
    header.write(createHash('sha256').update(prepared.bytes).digest('hex'),70);
    const seed = Buffer.concat([header, prepared.bytes]);
    await evaluate(host, `(async () => {
      const db = await openCache('kings-field-saves');
      await new Promise((resolve,reject) => {
        const tx=db.transaction('files','readwrite',{durability:'strict'});
        tx.oncomplete=resolve; tx.onabort=()=>reject(tx.error);
        tx.objectStore('files').put(new Uint8Array(${JSON.stringify(Array.from(seed))}),'party1.kfc');
      }); db.close();
    })()`);
    await cdp('Page.reload', {}, host);
    await until(host, 'typeof hostButton !== "undefined" && !hostButton.disabled', true);
    const savedCode = await hostRoom(true);
    if (spell) {
      const admission=await joinRoom(savedCode);
      assert.equal(admission.identity,originalAdmission.identity);
      assert.equal(admission.slot,originalAdmission.slot);
      const before=(await fixture('--inspect-world',await captureWorld())).summary;
      assert.equal(before.players[0].hp,1);
      assert.equal(before.players[1].presence,2);
      await evaluate(guest,'Module.canvas.focus()');
      await cdp('Input.dispatchKeyEvent',{type:'keyDown',key:'q',code:'KeyQ',windowsVirtualKeyCode:81},guest);
      await pause(150);
      await cdp('Input.dispatchKeyEvent',{type:'keyUp',key:'q',code:'KeyQ',windowsVirtualKeyCode:81},guest);
      for (let attempt=0; attempt<100 && !logs.get(host).includes('Player 1 died; spectating'); ++attempt) await pause(100);
      const after=(await fixture('--inspect-world',await captureWorld())).summary;
      console.log(`Guest Fire Ball: host HP ${before.players[0].hp} -> ${after.players[0].hp}; guest MP ${before.players[1].mp} -> ${after.players[1].mp}`);
      assert.equal(after.players[0].hp,0,'Guest Fire Ball did not kill the host');
      assert.equal(after.players[0].presence,3,'Spell victim did not become a spectator');
      assert.equal(after.players[1].presence,2,'Guest cast killed the entire party');
      assert.ok(after.players[1].mp<before.players[1].mp,'Guest input did not consume spell MP');
      assert.equal(after.floor,before.floor);
      assert.equal(after.variant,before.variant);
      for (let slot=0; slot<2; ++slot)
        for (const field of ['identity','gold','experience','magic_training','physical_training'])
          assert.equal(after.players[slot][field],before.players[slot][field],`Spell PvP changed ${field} for player ${slot+1}`);
      const tick=await evaluate(guest,'latestSnapshot');
      await pause(600);
      assert.ok(await evaluate(guest,`latestSnapshot>${tick} && !stopped`),'Host death stopped the shared world');
      connections.at(-1).destroy();
      await until(guest,"roomAdmissions.length>1 && roomAdmissions.at(-1).resumed && statusLabel.textContent.startsWith('Connected.')");
      const resumed=(await fixture('--inspect-world',await captureWorld())).summary;
      // A host scheduling pause may start a new epoch without restoring a save.
      assert.equal(resumed.floor,after.floor);
      assert.equal(resumed.variant,after.variant);
      assert.deepEqual(resumed.players,after.players,'Spell reconnect lost personal state or revived the victim');
      if (screenshot) {
        const shot=await cdp('Page.captureScreenshot',{format:'png'},guest);
        await writeFile(screenshot,Buffer.from(shot.data,'base64'));
      }
      await closeRoom();
      console.log('Guest Fire Ball killed the host without PvP rewards; simulation, party and death state survived reconnect');
    } else if (wipe) {
      await joinRoom(savedCode,'Waiting for the host to reach a safe entry point.');
      const epoch=await evaluate(guest,'new DataView(fullWorld.buffer).getUint32(7,true)');
      await until(guest, `latestSnapshotEpoch >= ${epoch+2}`);
      assert.ok(logs.get(host).some(line => line === 'Player 1 died; spectating'),
        'Combat fixture did not exercise the host death notice');
      const restored=(await fixture('--inspect-world',Buffer.from(await evaluate(guest,'Array.from(fullWorld)')))).summary;
      assert.equal(restored.floor,5);
      assert.ok(restored.epoch>epoch);
      assert.deepEqual(restored.players[0],prepared.summary.players[0],
        'Wipe did not restore the host HP, position and personal state');
      for (let slot=0; slot<2; ++slot) {
        assert.equal(restored.players[slot].identity,prepared.summary.players[slot].identity);
        assert.equal(restored.players[slot].gold,prepared.summary.players[slot].gold);
      }
      await closeRoom();
      console.log('Host death notices and repeated checkpoint restoration survived the floor-five combat fixture');
    } else if (entrance) {
      const admission = await joinRoom(savedCode);
      assert.equal(admission.identity, originalAdmission.identity);
      assert.equal(admission.slot, originalAdmission.slot);
      const browserConnection=connections.at(-1);
      const nativeGuests=[];
      if (scenario === '--travel-crossplay') {
        for (let i=0; i<2; ++i) {
          const child=await nativePlayer(['--join',savedCode]);
          const [,room,slotText]=await nativeUntil(child,/Online room: ([^;]+); player (\d+)/);
          const slot=Number(slotText);
          assert.equal(room,savedCode);
          assert.equal(slot,i+3);
          nativeGuests.push({child,slot,socket:connections.at(-1)});
          await nativeUntil(child,/World synchronized: epoch \d+, tick \d+, floor 5/);
          for (let attempt=0; attempt<300 && !logs.get(host).includes(`Player ${slot} admitted`); ++attempt) await pause(100);
          assert.ok(logs.get(host).includes(`Player ${slot} admitted`),`Native player ${slot} was not admitted`);
        }
      }
      const before = (await fixture('--inspect-world', await captureWorld())).summary;
      assert.equal(before.floor, 5);
      assert.equal(before.variant, 1);
      assert.equal(before.players.length,nativeGuests.length+2);
      assert.equal(new Set(before.players.map(member=>member.identity)).size,before.players.length);
      for (const member of before.players) {
        assert.equal(member.presence, 2);
        assert.deepEqual([member.cell_x,member.cell_z],[39,69]);
      }
      async function step(key, code, windowsVirtualKeyCode) {
        await evaluate(host, 'Module.canvas.focus()');
        await cdp('Input.dispatchKeyEvent',{type:'keyDown',key,code,windowsVirtualKeyCode},host);
        await pause(600);
        await cdp('Input.dispatchKeyEvent',{type:'keyUp',key,code,windowsVirtualKeyCode},host);
        await pause(300);
      }
      await step('w','KeyW',87);
      const outside = (await fixture('--inspect-world', await captureWorld())).summary;
      assert.equal(outside.epoch,before.epoch,'Guest standing at the entrance moved the split party');
      assert.equal(outside.floor,5);
      assert.notDeepEqual([outside.players[0].cell_x,outside.players[0].cell_z],[39,69],
        'Short host step did not leave the entrance');
      assert.deepEqual([outside.players[1].cell_x,outside.players[1].cell_z],[39,69]);
      const epoch=before.epoch+1;
      await evaluate(guest, `window.holdLoaded=true; window.travelEpoch=${epoch}`);
      await step('s','KeyS',83);
      await until(guest, `heldLoaded?.epoch === ${epoch}`);
      assert.equal(await evaluate(guest, `waitingEpochs.includes(${epoch})`),true);
      const heldTick=await evaluate(guest,'heldLoaded.tick');
      await pause(600);
      assert.equal(await evaluate(guest,'firstTravelTick'),null,'Host advanced before the new floor loaded');
      await evaluate(guest,'releaseLoaded()');
      await until(guest,'firstTravelTick !== null && statusLabel.textContent.startsWith("Connected.")');
      const firstTick=await evaluate(guest,'firstTravelTick');
      assert.ok(firstTick>heldTick && firstTick<=heldTick+2,'Simulation advanced during the floor load barrier');
      const arrived=(await fixture('--inspect-world',await captureWorld())).summary;
      assert.equal(arrived.epoch,epoch);
      assert.equal(arrived.floor,4);
      assert.equal(arrived.variant,0);
      assert.deepEqual(arrived.players,before.players,'Floor change lost a member or changed personal state');
      for (const {child} of nativeGuests)
        await nativeUntil(child,new RegExp(`World synchronized: epoch ${epoch}, tick \\d+, floor 4`));
      browserConnection.destroy();
      await until(guest,'roomAdmissions.length>1 && roomAdmissions.at(-1).resumed && statusLabel.textContent.startsWith("Connected.")');
      const reconnected=(await fixture('--inspect-world',await captureWorld())).summary;
      assert.deepEqual(reconnected,arrived,'Reconnect changed the arrived world or immediately retriggered travel');
      if (nativeGuests.length) {
        const {child,socket,slot}=nativeGuests[0];
        const after=child.output.length;
        socket.destroy();
        await nativeUntil(child,/Online connection interrupted; reconnecting/,after);
        await nativeUntil(child,new RegExp(`Online room: ${savedCode}; player ${slot}`),after);
        await nativeUntil(child,new RegExp(`World synchronized: epoch ${epoch}, tick \\d+, floor 4`),after);
        assert.deepEqual((await fixture('--inspect-world',await captureWorld())).summary,arrived,
          'Native reconnect changed the arrived party');
        for (const {child} of nativeGuests)
          assert.doesNotMatch(child.output,/Rejected invalid|Cannot encode|AddressSanitizer|runtime error:/);
      }
      if (screenshot) {
        const shot=await cdp('Page.captureScreenshot',{format:'png'},guest);
        await writeFile(screenshot,Buffer.from(shot.data,'base64'));
      }
      await closeRoom();
      console.log(`${before.players.length}-player floor-five/four gate travel waited for the whole party and loaded acknowledgment; personal state and arrival survived reconnect`);
    } else {
      const deadAdmission = await joinRoom(savedCode, 'You died.');
      assert.equal(deadAdmission.identity, originalAdmission.identity);
      assert.equal(deadAdmission.slot, originalAdmission.slot);
      assert.equal(deadAdmission.resumed, true);
      await evaluate(host, 'Module.canvas.focus()');
      async function key(key, code, windowsVirtualKeyCode, page = host) {
        await cdp('Input.dispatchKeyEvent', {type:'keyDown',key,code,windowsVirtualKeyCode}, page);
        await pause(180);
        await cdp('Input.dispatchKeyEvent', {type:'keyUp',key,code,windowsVirtualKeyCode}, page);
        await pause(700);
      }
      async function storedCheckpoint() {
        return Buffer.from(await evaluate(host, `(async () => {
          const tx=Module.kfSaveDb.transaction('files','readonly');
          return new Promise((resolve,reject) => {
            const request=tx.objectStore('files').get('party1.kfc');
            request.onsuccess=()=>resolve(Array.from(request.result)); request.onerror=()=>reject(request.error);
          });
        })()`));
      }
      if (scenario === '--save-failure') {
        await evaluate(host, `(() => {
          const put=IDBObjectStore.prototype.put;
          window.failedSaveAttempts=0;
          IDBObjectStore.prototype.put=function(value,key) {
            if (key==='party1.kfc') { ++failedSaveAttempts; throw new DOMException('Injected storage failure','QuotaExceededError'); }
            return put.call(this,value,key);
          };
        })()`);
        await key('e','KeyE',69);
        await key('Enter','Enter',13);
        await key('Enter','Enter',13);
        await until(host,'failedSaveAttempts===1 && Module.kfCampaignWrite!==6');
        assert.equal(await evaluate(host,'Module.kfCampaignWrite'),4,'Transaction completion overwrote the quota failure');
        assert.deepEqual(await storedCheckpoint(),seed,'Failed save replaced the durable checkpoint');
        const failed=(await fixture('--inspect-world',await captureWorld())).summary;
        assert.equal(failed.players[1].presence,3,'Failed save revived the guest');
        assert.equal(failed.players[1].hp,0,'Failed save healed the guest');
        await closeRoom(); // Recreate IndexedDB bindings and retry from the preserved checkpoint.
        await joinRoom(await hostRoom(true),'You died.');
        await evaluate(host,'Module.canvas.focus()');
        console.log('Quota failure preserved the checkpoint and did not revive the guest');
      }
      await key('e','KeyE',69); // Actual save-point interaction.
      await key('Enter','Enter',13); // Slot 1.
      await key('Enter','Enter',13); // Confirm save.
      await until(guest, 'statusLabel.textContent.startsWith("Connected.")');
      const saved = await storedCheckpoint();
      assert.notDeepEqual(saved, seed, 'Save menu did not persist a new checkpoint');
      assert.equal(saved.toString('ascii',0,4), 'KFC1');
      assert.equal(createHash('sha256').update(saved.subarray(134)).digest('hex'), saved.toString('ascii',70,134));
      const checkpoint = (await fixture('--inspect-world', saved.subarray(134))).summary;
      for (let slot=0; slot<2; ++slot) {
        assert.equal(checkpoint.players[slot].presence, 2, 'Checkpoint did not revive the party');
        assert.ok(checkpoint.players[slot].hp > 0);
        assert.equal(checkpoint.players[slot].gold, prepared.summary.players[slot].gold);
        assert.equal(checkpoint.players[slot].identity, prepared.summary.players[slot].identity);
      }
      const beforeTravel = (await fixture('--inspect-world', await captureWorld())).summary;
      assert.equal(beforeTravel.players[1].return_staff, 1);
      assert.notDeepEqual([beforeTravel.players[0].cell_x,beforeTravel.players[0].cell_z], [15,2]);
      const travelEpoch = beforeTravel.epoch + 1;
      await evaluate(guest, `window.holdLoaded=true; window.travelEpoch=${travelEpoch}; Module.canvas.focus()`);
      await key('Tab','Tab',9,guest);
      await key('Enter','Enter',13,guest); // Use item.
      await key('ArrowDown','ArrowDown',40,guest); // Staff follows the starting herb.
      await key('Enter','Enter',13,guest);
      await key('Enter','Enter',13,guest); // Confirm the guest's return request.
      await until(guest, `heldLoaded?.epoch === ${travelEpoch}`);
      assert.equal(await evaluate(guest, `waitingEpochs.includes(${travelEpoch})`), true,
        'Host did not announce the travel load barrier');
      const heldTick = await evaluate(guest, 'heldLoaded.tick');
      await pause(600);
      assert.equal(await evaluate(guest, 'firstTravelTick'), null, 'Host sent live state before the guest acknowledged loading');
      await evaluate(guest, 'releaseLoaded()');
      await until(guest, 'firstTravelTick !== null && statusLabel.textContent.startsWith("Connected.")');
      const firstTick = await evaluate(guest, 'firstTravelTick');
      assert.ok(firstTick > heldTick && firstTick <= heldTick+2, 'Host simulation advanced during the load barrier');
      const afterTravel = (await fixture('--inspect-world', await captureWorld())).summary;
      assert.equal(afterTravel.epoch, travelEpoch);
      assert.equal(afterTravel.floor, beforeTravel.floor);
      for (let slot=0; slot<2; ++slot)
        assert.deepEqual(afterTravel.players[slot], {...beforeTravel.players[slot],cell_x:15,cell_z:2},
          'Party return changed personal state or left a player behind');
      console.log('Guest used the return staff through its menu; both players travelled and the host waited for the loaded acknowledgment');
      if (screenshot) {
        const shot = await cdp('Page.captureScreenshot', {format:'png'}, guest);
        await writeFile(screenshot, Buffer.from(shot.data,'base64'));
      }
      await closeRoom();
      // Loaded living members follow the existing safe-entry admission rule.
      // Inspect their preserved state while waiting, without navigating the host.
      const returned = await joinRoom(await hostRoom(true), 'Waiting for the host to reach a safe entry point.');
      assert.equal(returned.identity, originalAdmission.identity);
      assert.equal(returned.slot, originalAdmission.slot);
      assert.equal(returned.resumed, true);
      const reloaded = (await fixture('--inspect-world', Buffer.from(await evaluate(guest, 'Array.from(fullWorld)')))).summary;
      assert.equal(reloaded.players[1].presence, 4, 'Returning guest bypassed safe-entry admission');
      assert.deepEqual(reloaded.players, [checkpoint.players[0], {...checkpoint.players[1], presence:4}],
        'Rehosting changed saved personal state');
      await closeRoom();
      console.log('Real save-point menu persisted the campaign, revived the guest and restored both profiles/personal gold after rehosting');
    }
    for (const lines of logs.values()) assert.doesNotMatch(lines.join('\n'), /Rejected invalid|Cannot encode|Application stopped|deadline exceeded/);
    assert.deepEqual(failures, []);
  } else if (scenario === '--preview' || previewOnly) {
    const page = await player(41);
    await evaluate(page, `window.previewPixels = () => {
      drawAvatarPreview();
      const {gl} = avatarPreview;
      const pixels = new Uint8Array(240*280*4);
      gl.readPixels(0,0,240,280,gl.RGBA,gl.UNSIGNED_BYTE,pixels);
      if (gl.getError() !== gl.NO_ERROR) throw Error('Character preview GL error');
      let hash=0, foreground=0;
      for (let i=0; i<pixels.length; i+=4) {
        hash=Math.imul(hash,31)^((pixels[i]<<16)|(pixels[i+1]<<8)|pixels[i+2]);
        if (pixels[i]!==pixels[0] || pixels[i+1]!==pixels[1] || pixels[i+2]!==pixels[2]) ++foreground;
      }
      return {hash,foreground};
    };
    window.selectPreviewPose = motion => {
      avatarPose.value = String(motion);
      avatarPose.dispatchEvent(new Event('change'));
      return previewPixels();
    };`);
    assert.equal(await evaluate(page, '!!avatarPreview && !avatarPreviewCanvas.hidden'), true,
      'Validated character did not produce a preview: ' + logs.get(page).join('\n'));
    const slots = await evaluate(page, 'Array.from(avatarSelect.options, o => Number(o.value))');
    assert.equal(await evaluate(page, "Module.ccall('kf_avatar_preview_count','number',['number'],[1])"),
      1106*3, 'Slot 1 forearm correction removed body geometry');
    assert.equal(await evaluate(page, "Module.ccall('kf_avatar_preview_count','number',['number'],[5])"),
      (809-80)*3, 'Slot 5 curation removed hand faces or retained the pipe');
    assert.equal(await evaluate(page, "Module.ccall('kf_avatar_preview_count','number',['number'],[19])"),
      791*3, 'Slot 19 hand correction removed body geometry');
    assert.equal(await evaluate(page, "Module.ccall('kf_avatar_preview_count','number',['number'],[26])"),
      620*3, 'Slot 26 hand correction removed body geometry');
    assert.equal(await evaluate(page, "Module.ccall('kf_avatar_preview_count','number',['number'],[27])"),
      (996-74)*3, 'Slot 27 curation removed hand faces or retained its carried sword');
    assert.equal(await evaluate(page, "Module.ccall('kf_avatar_preview_count','number',['number'],[14])"),
      (672-54)*3, 'Slot 14 curation did not remove exactly the carried cane');
    assert.equal(await evaluate(page, "Module.ccall('kf_avatar_preview_count','number',['number'],[22])"),
      (719-28)*3, 'Slot 22 curation removed part of the body or left its carried tool');
    assert.equal(await evaluate(page, "Module.ccall('kf_avatar_preview_count','number',['number'],[24])"),
      (728-16)*3, 'Slot 24 curation did not remove exactly the carried book');
    for (const slot of slots) {
      const rendered = await evaluate(page, `avatarSelect.value='${slot}'; loadAvatarPreview(); previewPixels()`);
      assert.ok(rendered.foreground > 100, `Character ${slot} rendered an empty preview`);
      assert.equal(await evaluate(page, "avatarPreviewHelp.textContent.includes('Walking and combat')"),
        [1,5,14,19,22,24,26,27,39,41].includes(slot), `Character ${slot} has incorrect pose availability`);
      assert.equal(await evaluate(page, '!avatarPoseControls.hidden'),
        [1,5,14,19,22,24,26,27,39,41].includes(slot), `Character ${slot} has incorrect pose controls`);
      if (slot === standingAvatar || process.env.KF_CAPTURE_ALL_AVATARS === '1') {
        await capture(`${slot}-selected`);
        await evaluate(page, 'avatarPreviewYaw = 1.2; drawAvatarPreview()');
        await capture(`${slot}-selected-side`);
        await evaluate(page, 'avatarPreviewYaw = 0.35; drawAvatarPreview()');
      }
    }
    await evaluate(page, "avatarSelect.value='41'; loadAvatarPreview(); avatarPreviewCanvas.focus();");
    const idle = await evaluate(page, 'previewPixels()');
    async function capture(name) {
      if (!screenshot) return;
      const png = await evaluate(page, "drawAvatarPreview(); avatarPreviewCanvas.toDataURL('image/png')");
      await writeFile(screenshot + '.' + name + '.png', Buffer.from(png.split(',')[1], 'base64'));
    }
    for (const slot of [1,5,14,19,22,24,26,27,39,41]) {
      await evaluate(page, `avatarSelect.value='${slot}'; loadAvatarPreview()`);
      const base = await evaluate(page, 'previewPixels()');
      await capture(`${slot}-idle`);
      await evaluate(page, 'selectPreviewPose(5)');
      assert.notEqual((await evaluate(page, 'previewPixels()')).hash, base.hash,
        `Character ${slot} stayed static while casting`);
      await capture(`${slot}-cast`);
      await evaluate(page, 'avatarPreviewYaw = 1.2; drawAvatarPreview()');
      await capture(`${slot}-cast-side`);
      await evaluate(page, 'avatarPreviewYaw = 0.35; selectPreviewPose(0)');
      assert.equal((await evaluate(page, 'previewPixels()')).hash, base.hash,
        `Character ${slot} changed its base mesh while casting`);
      for (const motion of [1,2,3,4]) {
        const posed = await evaluate(page, `selectPreviewPose(${motion})`);
        assert.notEqual(posed.hash, base.hash, `Character ${slot} pose ${motion} stayed at rest`);
        assert.equal(await evaluate(page, 'avatarPhase.disabled'), false, 'Pose slider is disabled');
        if (slot === standingAvatar) await capture(`${slot}-pose-${motion}`);
        const start = await evaluate(page, `avatarPhase.value='0'; avatarPhase.dispatchEvent(new Event('input')); previewPixels()`);
        assert.notEqual(start.hash, posed.hash, `Character ${slot} pose ${motion} ignored the slider`);
      }
      await evaluate(page, 'selectPreviewPose(0)');
      assert.equal(await evaluate(page, 'avatarPhase.disabled'), true, 'Rest pose exposes an ineffective slider');
      assert.equal((await evaluate(page, 'previewPixels()')).hash, base.hash,
        `Character ${slot} motion changed its loaded base mesh`);
    }
    await evaluate(page, "avatarSelect.value='41'; loadAvatarPreview(); avatarPreviewCanvas.focus()");
    await capture('idle');
    await evaluate(page, 'selectPreviewPose(5)');
    assert.notEqual((await evaluate(page, 'previewPixels()')).hash, idle.hash, 'Casting left the body in its idle pose');
    await capture('cast');
    await evaluate(page, 'avatarPreviewYaw = 1.2; drawAvatarPreview()');
    await capture('cast-side');
    await evaluate(page, 'avatarPreviewYaw = 0.35');
    await evaluate(page, 'selectPreviewPose(0)');
    assert.equal((await evaluate(page, 'previewPixels()')).hash, idle.hash, 'Casting changed the loaded base mesh');
    await cdp('Input.dispatchKeyEvent', {type:'keyDown',code:'ArrowRight',key:'ArrowRight',windowsVirtualKeyCode:39}, page);
    await cdp('Input.dispatchKeyEvent', {type:'keyUp',code:'ArrowRight',key:'ArrowRight',windowsVirtualKeyCode:39}, page);
    const rotated = await evaluate(page, 'previewPixels()');
    assert.notEqual(rotated.hash, idle.hash, 'Keyboard rotation did not turn the preview');
    const drag = await evaluate(page, `(() => { const r=avatarPreviewCanvas.getBoundingClientRect();
      return {x:r.x+r.width/2,y:r.y+r.height/2}; })()`);
    await cdp('Input.dispatchMouseEvent', {type:'mousePressed',...drag,button:'left',clickCount:1}, page);
    await cdp('Input.dispatchMouseEvent', {type:'mouseMoved',x:drag.x+40,y:drag.y,button:'left',buttons:1}, page);
    await cdp('Input.dispatchMouseEvent', {type:'mouseReleased',x:drag.x+40,y:drag.y,button:'left',clickCount:1}, page);
    const dragged = await evaluate(page, 'previewPixels()');
    assert.notEqual(dragged.hash, rotated.hash, 'Dragging did not turn the preview');
    assert.equal(await evaluate(page, `Module.ccall('kf_avatar_preview_vertices','number',['number','number','number'],[44,0,0])`), 0,
      'Preview bridge accepted an unavailable model');
    assert.equal(await evaluate(page, `Module.ccall('kf_avatar_preview_vertices','number',['number','number','number'],[41,6,0])`), 0,
      'Preview bridge accepted an unavailable pose');
    const beforeLoss = await evaluate(page, `selectPreviewPose(1); avatarPhase.value='1337';
      avatarPhase.dispatchEvent(new Event('input')); previewPixels()`);
    await evaluate(page, `window.previewContextLoss = avatarPreview.gl.getExtension('WEBGL_lose_context'); previewContextLoss.loseContext();`);
    await until(page, 'avatarPreview === null');
    await evaluate(page, 'previewContextLoss.restoreContext()');
    await until(page, 'avatarPreview !== null');
    assert.equal((await evaluate(page, 'previewPixels()')).hash, beforeLoss.hash, 'Context restoration changed the selected pose');
    assert.equal(await evaluate(page, 'avatarPose.value'), '1');
    assert.equal(await evaluate(page, 'avatarPhase.value'), '1337');
    if (!previewOnly) {
      await evaluate(page, 'playButton.click()');
      await until(page, 'running');
      assert.equal(await evaluate(page, 'avatarPreview === null && avatarPreviewPanel.hidden'), true,
        'Starting gameplay retained the lobby preview resources');
    } else {
      assert.equal(await evaluate(page, 'running || dataRoot !== null'), false,
        'Character-only preview started gameplay or imported game resources');
    }
    assert.deepEqual(failures, []);
    console.log('All 39 local character previews, ten rigs with pose/phase controls, rotation and context restoration passed; ' +
      (previewOnly ? 'gameplay was not started' : 'gameplay preview cleanup passed'));
  } else {
  const host = await player(standing ? 0 : 41);
  // A wrong disc must leave the already validated characters/cache usable.
  const hostDocument = await cdp('DOM.getDocument', {}, host);
  const characterInput = await cdp('DOM.querySelector', {nodeId:hostDocument.root.nodeId, selector:'#avatars'}, host);
  await cdp('DOM.setFileInputFiles', {nodeId:characterInput.nodeId, files:[resolve(disc)]}, host);
  await until(host, '!avatarInput.disabled && avatarStatus.textContent.includes("SLUS-00255")');
  assert.equal(await evaluate(host, 'avatarPath !== null && avatarSelect.options.length === 39 && !hostButton.disabled'), true,
    'A rejected disc replaced the validated characters');
  await evaluate(host, 'hostButton.click()');
  await until(host, "!document.getElementById('room-panel').hidden");
  const code = await evaluate(host, "document.getElementById('room-code').textContent");
  assert.match(code, /^[A-Za-z0-9_-]+$/);
  const guest = await player(avatars && !standing ? 0 : standingAvatar);
  await evaluate(guest, `joinCode.value = ${JSON.stringify(code)}; joinButton.click();`);
  await until(guest, "!document.getElementById('room-panel').hidden");
  assert.equal(await evaluate(guest, "document.getElementById('room-code').textContent"), code);
  for (const client of [host, guest])
    assert.equal(await evaluate(client, 'roomCompatibility.resources'),
      await evaluate(client, "Module.ccall('kf_resource_files_hash', 'string', ['string'], [languageInput.value])"),
      'Startup resource hashing disagrees with the independently verified browser manifest');
  await until(guest, 'statusLabel.textContent.startsWith("Connected.")');
  for (let i = 0; i < 300 && !logs.get(host).some(line => line === 'Player 2 admitted'); ++i) await pause(100);
  assert.ok(logs.get(host).some(line => line === 'Player 2 admitted'), 'Browser guest was not admitted');
  const originalAdmission = await evaluate(guest, 'roomAdmissions.at(-1)');
  if (avatars) {
    if (!standing) {
      await until(guest, 'avatarRequests.some(request => request.slot === 0 && actionReplies.some(reply => reply.kind === 9 && reply.sequence === request.sequence))');
      console.log('Guest character selection accepted by the host');
    }
    if (['--avatars', '--standing-avatar', '--pose'].includes(scenario)) {
      async function capturePose(name) {
        if (!screenshot) return;
        const clip = await evaluate(host, `(() => { const r=Module.canvas.getBoundingClientRect();
          return {x:r.x+scrollX,y:r.y+scrollY,width:r.width,height:r.height,scale:1}; })()`);
        const shot = await cdp('Page.captureScreenshot', {format:'png',clip,captureBeyondViewport:true}, host);
        await writeFile(screenshot + '.' + name + '.png', Buffer.from(shot.data, 'base64'));
      }
      // One short step from the shared spawn; no route finding or playthrough.
      await cdp('Input.dispatchKeyEvent', {type:'keyDown', code:'KeyW', key:'w', windowsVirtualKeyCode:87}, guest);
      if (scenario === '--pose') {
        await pause(600);
        await capturePose('walk1');
        await pause(150);
        await capturePose('walk2');
      } else await pause(1500);
      await cdp('Input.dispatchKeyEvent', {type:'keyUp', code:'KeyW', key:'w', windowsVirtualKeyCode:87}, guest);
      await pause(500);
      if (scenario === '--pose') {
        await capturePose('idle');
        await cdp('Input.dispatchKeyEvent', {type:'keyDown', code:'Space', key:' ', windowsVirtualKeyCode:32}, guest);
        await pause(50);
        await cdp('Input.dispatchKeyEvent', {type:'keyUp', code:'Space', key:' ', windowsVirtualKeyCode:32}, guest);
        await pause(300);
        await capturePose('windup');
        await pause(150);
        await capturePose('strike');
        await pause(400);
        await capturePose('recovered');
      }
      if (screenshot) {
        const shot = await cdp('Page.captureScreenshot', {format:'png'}, host);
        await writeFile(screenshot + '.host.png', Buffer.from(shot.data, 'base64'));
      }
    }
  }
  if (['--prediction', '--remote-smoothing'].includes(scenario)) {
    const mover = scenario === '--remote-smoothing' ? host : guest;
    await evaluate(host, 'window.snapshotDelay = 150');
    await evaluate(mover, 'Module.canvas.focus()');
    await cdp('Input.dispatchKeyEvent', {type:'keyDown', code:'KeyW', key:'w', windowsVirtualKeyCode:87}, mover);
    await pause(800);
    await evaluate(host, 'window.dropSnapshots = true');
    await pause(500);
    await cdp('Input.dispatchKeyEvent', {type:'keyUp', code:'KeyW', key:'w', windowsVirtualKeyCode:87}, mover);
    const beforeRecovery = await evaluate(guest, 'latestSnapshot');
    await evaluate(host, 'window.dropSnapshots = false; window.snapshotDelay = 0');
    await until(guest, `latestSnapshot > ${beforeRecovery}`);
    await pause(500);
    assert.ok(await evaluate(host, 'delayedSnapshots >= 5 && droppedSnapshots >= 5'),
      'The movement check did not exercise delayed and missing world snapshots');
    assert.equal(await evaluate(guest, 'statusLabel.textContent.startsWith("Connected.")'), true);
    if (screenshot) {
      const shot = await cdp('Page.captureScreenshot', {format:'png'}, guest);
      await writeFile(screenshot + (scenario === '--remote-smoothing' ? '.remote.png' : '.prediction.png'), Buffer.from(shot.data, 'base64'));
    }
    console.log(`${scenario === '--remote-smoothing' ? 'Remote' : 'Local'} starting-area movement recovered after 150 ms snapshot delay and a 500 ms snapshot gap`);
  }
  async function menuAction(page) {
    await evaluate(page, `window.menuResults = []; window.menuSnapshots = 0;
      const peer = Array.from(Array.from(Module.kfTransports.values())[0].peers.values())[0];
      peer.channels[0].addEventListener('message', event => {
        const bytes = new DataView(event.data);
        // Avatar selection shares this channel and can finish after menu entry.
        if (bytes.byteLength === 19 && [9, 10].includes(bytes.getUint8(6)) &&
            !avatarRequests.some(request => request.sequence === bytes.getUint32(11, true)))
          menuResults.push({kind: bytes.getUint8(6), sequence: bytes.getUint32(11, true)});
      });
      peer.channels[1].addEventListener('message', () => ++menuSnapshots);
      Module.canvas.focus();`);
    async function key(key, code, keyCode) {
      const params = { key, code, windowsVirtualKeyCode: keyCode, nativeVirtualKeyCode: keyCode };
      await cdp('Input.dispatchKeyEvent', { ...params, type: 'keyDown' }, page);
      await pause(180);
      await cdp('Input.dispatchKeyEvent', { ...params, type: 'keyUp' }, page);
      await pause(500);
    }
    await key('Tab', 'Tab', 9);
    await pause(1000);
    await key('Enter', 'Enter', 13); // Use item.
    await key('Enter', 'Enter', 13); // Starting herb.
    await key('Enter', 'Enter', 13); // Confirm use.
    await until(page, 'menuResults.length > 0');
    const replies = await evaluate(page, 'menuResults');
    assert.equal(replies.length, 1, `Unexpected guest menu replies: ${JSON.stringify(replies)}`);
    assert.equal(replies[0].kind, 9, 'Host rejected the guest menu action');
    assert.ok(await evaluate(page, 'menuSnapshots > 10'), 'Guest menu paused world synchronization');
    console.log('Guest starting-area menu action accepted while world snapshots continued');
  }
  if (['--menus','--campaign','--crossplay'].includes(scenario)) await menuAction(guest);
  async function relayTypes(page, slot) {
    return evaluate(page, `(async () => {
      const peers = Array.from(Module.kfTransports.values())[0].peers;
      const peer = ${slot === undefined ? 'Array.from(peers.values())[0]' : `peers.get(${slot})`};
      if (peer.pc.getConfiguration().iceTransportPolicy !== 'relay')
        throw new Error('Test did not force relay policy');
      const stats = await peer.pc.getStats();
      const transport = Array.from(stats.values()).find(s => s.type === 'transport' && s.selectedCandidatePairId);
      const pair = stats.get(transport.selectedCandidatePairId);
      return [stats.get(pair.localCandidateId).candidateType, stats.get(pair.remoteCandidateId).candidateType];
    })()`);
  }
  if (crossplay) {
    const native = await nativePlayer(['--join', code]);
    await nativeUntil(native, /World synchronized: epoch [1-9]\d*, tick \d+, floor 1/);
    for (let i = 0; i < 300 && !logs.get(host).some(line => line === 'Player 3 admitted'); ++i) await pause(100);
    assert.ok(logs.get(host).some(line => line === 'Player 3 admitted'), 'Browser host did not admit the native guest');
    assert.equal((await relayTypes(host, 2))[0], 'relay', 'Native guest bypassed the browser host TURN relay');
    const offset = native.output.length;
    connections.at(-1).destroy();
    await nativeUntil(native, /Online connection interrupted; reconnecting/, offset);
    await nativeUntil(native, /World synchronized: epoch [1-9]\d*, tick \d+, floor 1/, offset);
    assert.match(native.output.slice(offset), new RegExp(`Online room: ${code}; player 3`));
    const exited = once(native, 'exit');
    process.kill(-native.pid, 'SIGTERM');
    await exited;
    console.log('Native guest joined and reconnected to the actual browser host');
  }
  for (const page of [host, guest]) {
    const types = await relayTypes(page);
    assert.deepEqual(types, ['relay', 'relay'], 'World traffic bypassed TURN');
  }
  await pause(1500);
  const before = logs.get(guest).length;
  const hostBefore = logs.get(host).length;
  connections[1].destroy();
  for (let i = 0; i < 300 && !logs.get(guest).slice(before).some(line => line.startsWith('World synchronized:')); ++i)
    await pause(100);
  assert.ok(logs.get(guest).slice(before).some(line => line.startsWith('World synchronized:')), 'Browser reconnect did not synchronize');
  assert.equal(await evaluate(guest, "document.getElementById('room-code').textContent"), code);
  assert.doesNotMatch(logs.get(host).slice(hostBefore).join('\n'), /Online connection interrupted|Online room connection restored/,
    'A guest disconnect interrupted the host room connection');
  assert.equal(await evaluate(host, 'stopped'), false);
  assert.equal(await evaluate(guest, 'stopped'), false);
  assert.equal(await evaluate(host, 'document.getElementById("restart").checkVisibility()'), false);
  // Recreate WASM and transport state. Only the persistent local profile survives.
  await cdp('Page.reload', {}, guest);
  await until(guest, 'typeof hostButton !== "undefined" && !hostButton.disabled', true);
  await evaluate(guest, `joinCode.value = ${JSON.stringify(code)}; joinButton.click();`);
  await until(guest, 'statusLabel.textContent.startsWith("Connected.")');
  const reopened = await evaluate(guest, 'roomAdmissions.at(-1)');
  assert.equal(reopened.slot, originalAdmission.slot, 'Reload took another character slot');
  assert.equal(reopened.identity, originalAdmission.identity, 'Reload lost the persistent player profile');
  assert.equal(reopened.resumed, true, 'Reload created a fresh room participant');
  assert.deepEqual(failures, []);
  for (const lines of logs.values()) assert.doesNotMatch(lines.join('\n'), /Rejected invalid|Cannot encode|Application stopped|deadline exceeded/);
  if (screenshot) {
    await evaluate(host, 'window.scrollTo(0, 0)');
    const layout = await cdp('Page.getLayoutMetrics', {}, host);
    const capture = await cdp('Page.captureScreenshot', { format: 'png', captureBeyondViewport: true,
      clip: { ...layout.cssContentSize, scale: 1 } }, host);
    await writeFile(screenshot, Buffer.from(capture.data, 'base64'));
  }
  if (scenario === '--campaign') {
    // Checkpoint fixture from the real synchronized starting-area world. This
    // exercises loading/rehosting without routing players to a distant save point.
    await until(guest, 'fullWorld !== null');
    const snapshot = Buffer.from(await evaluate(guest, 'Array.from(fullWorld)'));
    const compatibility = await evaluate(host, 'roomCompatibility');
    const wrap = bytes => {
      const header = Buffer.alloc(134);
      header.write('KFC1'); header.writeUInt16LE(1,4);
      header.write(createHash('sha256').update(compatibility.resources + '\0' + compatibility.recipe + '\0').digest('hex'),6);
      header.write(createHash('sha256').update(bytes).digest('hex'),70);
      return Buffer.concat([header,bytes]);
    };
    const wrongOwner = Buffer.from(snapshot);
    wrongOwner[25] ^= 1; // First member's public identity, after header/quest/presence.
    const fixtures = [wrap(snapshot), Buffer.from([1,2,3]), wrap(wrongOwner)];
    await evaluate(host, `(async () => {
      const db = Module.kfSaveDb;
      await new Promise((resolve,reject) => {
        const tx = db.transaction('files','readwrite',{durability:'strict'});
        tx.oncomplete = resolve; tx.onabort = () => reject(tx.error);
        ${fixtures.map((bytes,i) => `tx.objectStore('files').put(new Uint8Array(${JSON.stringify(Array.from(bytes))}),'party${i+1}.kfc');`).join('\n')}
      });
    })()`);
  }
  const leaveLogs = new Map([host, guest].map(page => [page, logs.get(page).length]));
  await evaluate(host, "Array.from(Module.kfTransports.values())[0].socket.send(JSON.stringify({type: 'leave'}))");
  await until(host, 'stopped && !document.getElementById("restart").hidden');
  await until(guest, 'stopped && !document.getElementById("restart").hidden');
  for (const page of [host, guest]) {
    const closing = logs.get(page).slice(leaveLogs.get(page)).join('\n');
    assert.match(closing, /The host closed the room/);
    assert.doesNotMatch(closing, /reconnecting|deadline exceeded|Room unavailable/);
  }
  assert.equal(await evaluate(guest, 'document.getElementById("room-panel").hidden'), true);
  await cdp('Page.reload', {}, guest);
  await until(guest, 'typeof hostButton !== "undefined" && !hostButton.disabled');
  assert.equal(await evaluate(guest, 'dataRoot !== null && !running && !stopped'), true,
    'Returning to the lobby lost locally cached game resources');
  assert.equal(await evaluate(guest, 'avatarSelect.options.length'), 39, 'Returning to the lobby lost character resources');
  assert.equal(await evaluate(guest, 'Number(avatarSelect.value)'), standing ? standingAvatar : 0,
    'Returning to the lobby lost the selected character');
  if (crossplay) {
    const native = await nativePlayer(['--host']);
    const [, nativeCode] = await nativeUntil(native, /Online room: ([^;]+);/);
    await evaluate(guest, `joinCode.value = ${JSON.stringify(nativeCode)}; joinButton.click();`);
    await until(guest, 'statusLabel.textContent.startsWith("Connected.")');
    await nativeUntil(native, /Player 2 admitted/);
    assert.equal((await relayTypes(guest, 0))[0], 'relay', 'Browser guest bypassed its TURN relay');
    await until(guest, 'avatarRequests.some(request => request.slot === 0 && actionReplies.some(reply => reply.kind === 9 && reply.sequence === request.sequence))');
    await menuAction(guest);
    const guestOffset = logs.get(guest).length, nativeOffset = native.output.length;
    connections.at(-1).destroy();
    await nativeUntil(native, /Player 2 reconnected/, nativeOffset);
    for (let i = 0; i < 300 && !logs.get(guest).slice(guestOffset).some(line => line.startsWith('World synchronized:')); ++i)
      await pause(100);
    assert.ok(logs.get(guest).slice(guestOffset).some(line => line.startsWith('World synchronized:')),
      'Browser guest did not resynchronize with the native host');
    if (screenshot) {
      const shot = await cdp('Page.captureScreenshot', {format:'png'}, guest);
      await writeFile(screenshot + '.native-host.png', Buffer.from(shot.data, 'base64'));
    }
    assert.doesNotMatch(native.output, /Rejected invalid|Cannot encode|AddressSanitizer|runtime error:/);
    for (const lines of logs.values()) assert.doesNotMatch(lines.join('\n'), /Rejected invalid|Cannot encode|Application stopped|deadline exceeded/);
    await cdp('Page.reload', {}, guest);
    await until(guest, 'typeof hostButton !== "undefined" && !hostButton.disabled');
    const exited = once(native, 'exit');
    process.kill(-native.pid, 'SIGTERM');
    await exited;
    console.log('Browser guest joined the actual native host, completed a menu action and reconnected');
  }
  if (scenario === '--campaign') {
    await cdp('Page.reload', {}, host);
    await until(host, 'typeof hostButton !== "undefined" && !hostButton.disabled');
    const options = await evaluate(host, 'Array.from(campaignSelect.options,o => ({value:o.value,disabled:o.disabled,text:o.textContent}))');
    assert.equal(options.length,4);
    assert.match(options[1].text,/Floor 1 · Level 1/);
    assert.equal(options[1].disabled,false);
    assert.equal(options[2].disabled,true,'Corrupt campaign was offered for hosting');
    assert.equal(options[3].disabled,true,'Another profile’s campaign was offered for hosting');
    assert.match(options[3].text,/different player profile/);
    const rehostLog = logs.get(host).length;
    await evaluate(host, "campaignSelect.value = '1'; hostButton.click()");
    await until(host, '!document.getElementById("room-panel").hidden');
    const newCode = await evaluate(host, 'document.getElementById("room-code").textContent');
    assert.notEqual(newCode,code);
    await evaluate(guest, `joinCode.value = ${JSON.stringify(newCode)}; joinButton.click();`);
    await until(guest, 'statusLabel.textContent.startsWith("Connected.")');
    const returned = await evaluate(guest, 'roomAdmissions.at(-1)');
    assert.equal(returned.identity,originalAdmission.identity);
    assert.equal(returned.slot,originalAdmission.slot);
    assert.equal(returned.resumed,true,'Rehosting discarded the saved character roster');
    assert.ok(logs.get(host).slice(rehostLog).some(line => line === 'Player 2 returned'),
      'The restored character never passed safe admission');
    await evaluate(host, "Array.from(Module.kfTransports.values())[0].socket.send(JSON.stringify({type:'leave'}))");
    await until(host,'stopped'); await until(guest,'stopped');
    await cdp('Page.reload', {}, guest);
    await until(guest, 'typeof hostButton !== "undefined" && !hostButton.disabled');
    console.log('Saved campaign catalogue, ownership checks and rehosting with the returning character passed');
  }
  await evaluate(guest, `(async () => {
    const db = await openCache('kings-field-characters');
    await new Promise((resolve,reject) => {
      const tx = db.transaction('files','readwrite');
      tx.oncomplete = resolve; tx.onabort = () => reject(tx.error);
      tx.objectStore('files').put(new Uint8Array([1,2,3]),'pack');
    });
    db.close();
  })()`);
  await cdp('Page.reload', {}, guest);
  await until(guest, 'typeof playButton !== "undefined" && !playButton.disabled');
  assert.equal(await evaluate(guest, 'hostButton.disabled && joinButton.disabled && avatarPath === null'), true,
    'A damaged character cache enabled multiplayer');
  assert.match(await evaluate(guest, 'avatarStatus.textContent'), /cache unavailable or invalid/);
  assert.equal(await evaluate(guest, 'verifyFiles(collectedFiles(dataRoot))'), true,
    'A damaged character cache affected original game resources');
  assert.deepEqual(failures, []);
  console.log('Browser import, host/join codes, TURN world sync, reconnect, profile reload and return to lobby passed');
  }
} finally {
  for (const child of nativeClients) {
    if (child.exitCode !== null || child.signalCode !== null) continue;
    const exited = once(child, 'exit');
    try { process.kill(-child.pid, 'SIGTERM'); } catch {}
    await exited;
  }
  debuggerSocket?.close();
  if (browser && browser.exitCode === null) {
    const exited = once(browser, 'exit');
    browser.kill('SIGKILL');
    await exited;
  }
  await service.close();
  if (turn && turn.exitCode === null) {
    const exited = once(turn, 'exit');
    turn.kill('SIGTERM');
    await exited;
  }
  await rm(profile, { recursive: true, force: true, maxRetries: 5 });
}
