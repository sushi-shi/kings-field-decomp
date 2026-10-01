import { createRoomService } from './server.mjs';
import { once } from 'node:events';
import { createServer, get as httpGet } from 'node:http';
import { get as httpsGet } from 'node:https';
import { spawn } from 'node:child_process';
import { mkdtemp, readFile, writeFile, rm } from 'node:fs/promises';
import { join, resolve } from 'node:path';
import { tmpdir } from 'node:os';
import assert from 'node:assert/strict';
import WebSocket from 'ws';
import { createServer as tcpServer, connect as tcpConnect } from 'node:net';
import { createHash, randomBytes, X509Certificate } from 'node:crypto';
import { connect as tlsConnect } from 'node:tls';

const pause = ms => new Promise(resolve => setTimeout(resolve, ms));
const profile = await mkdtemp(join(tmpdir(), 'kf-crossplay-'));
const tlsRelayTest = process.argv.includes('--turn-tls');
const relayTest = tlsRelayTest || process.argv.includes('--turn-tcp');
const stunTest = process.argv.includes('--stun-only');
const httpsTest = tlsRelayTest || process.argv.includes('--https');
const relayProtocol = tlsRelayTest ? 'tls' : 'tcp';
const browserHost = process.argv.includes('--browser-host');
const overflowPackets = process.argv.includes('--overflow-packets');
const overflowSignals = process.argv.includes('--overflow-signals');
const rejectPacket = overflowPackets || process.argv.includes('--reject-packet');
const rejectPeer = overflowSignals || rejectPacket || process.argv.includes('--reject-peer');
let service;
const directory = resolve(process.argv[3]);
const files = new Map(['html', 'js', 'wasm'].map(extension => [
  `/coop-browser-test.${extension}`, [join(directory, `coop-browser-test.${extension}`),
    extension === 'wasm' ? 'application/wasm' : extension === 'js' ? 'text/javascript' : 'text/html'],
]));
const pages = createServer(async (request, response) => {
  const file = files.get(new URL(request.url, 'http://localhost').pathname);
  if (!file) { response.writeHead(404); response.end(); return; }
  try { response.writeHead(200, { 'content-type': file[1] }); response.end(await readFile(file[0])); }
  catch { response.destroy(); }
});
let host, browser, debuggerSocket, turn, proxy, rogue;
let hostExit, hostOutput = '';
function startNative(signaling, room) {
  host = spawn(process.argv[2], [signaling, ...(room ? [room] : [])], {
    stdio:['ignore', 'pipe', 'inherit'],
    env:overflowPackets ? {...process.env, KF_TEST_PEER_OVERFLOW:'1'} : process.env,
  });
  hostExit = once(host, 'exit');
  host.stdout.on('data', data => { hostOutput += data; });
}

async function reservePort() {
  const reservation = tcpServer();
  reservation.listen(0, '127.0.0.1');
  await once(reservation, 'listening');
  const port = reservation.address().port;
  await new Promise(resolve => reservation.close(resolve));
  return port;
}

async function run(program, args) {
  const child = spawn(program, args, {stdio:['ignore', 'pipe', 'pipe']});
  let output = '';
  const capture = bytes => { output = (output + bytes).slice(-8000); };
  child.stdout.on('data', capture); child.stderr.on('data', capture);
  const [code] = await once(child, 'exit');
  assert.equal(code, 0, `${program} failed: ${output}`);
}

function request(url, ca) {
  return new Promise((resolve, reject) => {
    const get = url.startsWith('https:') ? httpsGet : httpGet;
    const req = get(url, {ca}, response => {
      const chunks = [];
      response.on('data', chunk => chunks.push(chunk));
      response.on('end', () => resolve({status:response.statusCode, headers:response.headers,
        body:Buffer.concat(chunks)}));
      response.on('error', reject);
    });
    req.setTimeout(5000, () => req.destroy(new Error('Proxy request timed out')));
    req.on('error', reject);
  });
}

try {
  assert.ok(!(stunTest && relayTest), '--stun-only cannot be combined with a TURN relay mode');
  const browserOptions = [];
  const certificate = join(profile, 'certificate.pem');
  const key = join(profile, 'key.pem');
  let ca;
  if (httpsTest) {
    await run('openssl', ['req', '-x509', '-newkey', 'rsa:2048', '-nodes', '-days', '1',
      '-subj', '/CN=127.0.0.1', '-addext', 'subjectAltName=IP:127.0.0.1',
      '-keyout', key, '-out', certificate]);
    ca = await readFile(certificate);
    // Trust only this disposable certificate in this isolated browser profile.
    const spki = new X509Certificate(ca).publicKey.export({type:'spki', format:'der'});
    browserOptions.push(`--ignore-certificate-errors-spki-list=${createHash('sha256').update(spki).digest('base64')}`);
  }
  let relayOptions = {};
  if (relayTest || stunTest) {
    const port = await reservePort();
    const tlsPort = tlsRelayTest ? await reservePort() : 0;
    if (tlsRelayTest) assert.notEqual(port, tlsPort);
    const secret = randomBytes(32).toString('hex');
    relayOptions = stunTest ? {stunUrl:`stun:127.0.0.1:${port}`} :
      {turnUrl:`turn:127.0.0.1:${port}?transport=udp,turn:127.0.0.1:${port}?transport=tcp` +
      (tlsRelayTest ? `,turns:127.0.0.1:${tlsPort}?transport=tcp` : ''),
      turnSecret:secret};
    let turnConfig = await readFile(new URL('./deploy/turnserver.conf', import.meta.url), 'utf8');
    turnConfig = turnConfig.replace('listening-port=3478', `listening-port=${port}`)
      .replace('tls-listening-port=5349', `tls-listening-port=${tlsPort}`)
      .replace('/etc/coturn/certs/turn.example.com/fullchain.pem', certificate)
      .replace('/etc/coturn/certs/turn.example.com/privkey.pem', key)
      .replace('REPLACE_WITH_THE_SAME_RANDOM_SECRET_AS_THE_ROOM_SERVICE', secret)
      // Local fixtures need loopback/private peers; production keeps its denies.
      .replace(/^denied-peer-ip=.*$/gm, '');
    if (!tlsRelayTest) turnConfig += '\nno-tls\n';
    const turnConfigPath = join(profile, 'turnserver.conf');
    await writeFile(turnConfigPath, turnConfig, {mode:0o600});
    turn = spawn('turnserver', ['-c', turnConfigPath, '--listening-ip=127.0.0.1', '--relay-ip=127.0.0.1',
      '--allow-loopback-peers',
      `--pidfile=${profile}/turn.pid`, `--userdb=${profile}/turn.sqlite`, '--log-file=stdout'],
      {stdio:['ignore', 'pipe', 'pipe']});
    let turnLog = '';
    const log = bytes => { turnLog = (turnLog + bytes).slice(-8000); };
    turn.stdout.on('data', log); turn.stderr.on('data', log);
    let listening = false;
    for (let attempt = 0; attempt < 100 && !listening; ++attempt) {
      listening = await new Promise(resolve => {
        const socket = tcpConnect(port, '127.0.0.1');
        socket.once('connect', () => { socket.destroy(); resolve(true); });
        socket.once('error', () => resolve(false));
      });
      if (!listening) await pause(50);
    }
    assert.ok(listening, `Local TURN listener did not start: ${turnLog}`);
    if (tlsRelayTest) {
      const socket = tlsConnect({host:'127.0.0.1', port:tlsPort, ca});
      socket.setTimeout(5000, () => socket.destroy(new Error('TURN TLS handshake timed out')));
      try {
        await once(socket, 'secureConnect');
        assert.equal(socket.authorized, true);
        assert.ok(['TLSv1.2', 'TLSv1.3'].includes(socket.getProtocol()));
      } finally { socket.destroy(); }
      console.log('coturn template: authenticated TLS listener passed');
    }
  }
  service = createRoomService({...relayOptions, webRoot:directory});
  service.server.listen(0, '127.0.0.1');
  pages.listen(0, '127.0.0.1');
  await Promise.all([once(service.server, 'listening'), once(pages, 'listening')]);
  const signaling = `ws://127.0.0.1:${service.server.address().port}`;
  let browserSignaling = signaling;
  let browserOrigin = `http://127.0.0.1:${pages.address().port}`;
  if (httpsTest) {
    const httpPort = await reservePort();
    const httpsPort = await reservePort();
    assert.notEqual(httpPort, httpsPort);
    browserOrigin = `https://127.0.0.1:${httpsPort}`;
    browserSignaling = `wss://127.0.0.1:${httpsPort}/rooms`;
    let config = await readFile(new URL('./deploy/nginx.conf', import.meta.url), 'utf8');
    config = config.replace('listen 80;', `listen 127.0.0.1:${httpPort};`)
      .replace('listen 443 ssl;', `listen 127.0.0.1:${httpsPort} ssl;`)
      .replace('https://game.example.com$request_uri', `${browserOrigin}$request_uri`)
      .replace('/etc/letsencrypt/live/game.example.com/fullchain.pem', certificate)
      .replace('/etc/letsencrypt/live/game.example.com/privkey.pem', key)
      .replaceAll('game.example.com', '127.0.0.1')
      .replaceAll('127.0.0.1:8787', `127.0.0.1:${service.server.address().port}`);
    // Only the transport fixture gets an extra route; all production routes use
    // the template and the real room service's unchanged static-file allowlist.
    config = config.replace('    location / {', `    location ~ ^/coop-browser-test\\.(html|js|wasm)$ {
        proxy_pass http://127.0.0.1:${pages.address().port};
    }
    location / {`);
    const configPath = join(profile, 'nginx.conf');
    await writeFile(configPath, `daemon off;
master_process off;
pid ${profile}/nginx.pid;
error_log stderr;
events {}
http {
    access_log off;
${config}
}
`);
    const args = ['-p', `${profile}/`, '-e', 'stderr', '-c', configPath];
    await run('nginx', [...args, '-t']);
    proxy = spawn('nginx', args, {stdio:['ignore', 'ignore', 'pipe']});
    let proxyLog = '';
    proxy.stderr.on('data', bytes => { proxyLog = (proxyLog + bytes).slice(-8000); });
    let health;
    for (let attempt = 0; attempt < 100; ++attempt) {
      try { health = await request(`${browserOrigin}/health`, ca); break; }
      catch { if (proxy.exitCode !== null) break; await pause(50); }
    }
    assert.equal(health?.status, 200, `Local HTTPS proxy did not start: ${proxyLog}`);
    assert.equal(health.body.toString(), 'ok\n');
    const redirect = await request(`http://127.0.0.1:${httpPort}/rooms?check=1`);
    assert.equal(redirect.status, 301);
    assert.equal(redirect.headers.location, `${browserOrigin}/rooms?check=1`);
    for (const [path, filename, mime] of [
      ['/', 'kings-field.html', 'text/html'],
      ['/kings-field.js', 'kings-field.js', 'text/javascript'],
      ['/kings-field.wasm', 'kings-field.wasm', 'application/wasm'],
    ]) {
      const response = await request(`${browserOrigin}${path}`, ca);
      assert.equal(response.status, 200, path);
      assert.ok(response.headers['content-type'].startsWith(mime), path);
      assert.deepEqual(response.body, await readFile(join(directory, filename)), path);
    }
    assert.equal((await request(`${browserOrigin}/server.mjs`, ca)).status, 404);
    console.log('nginx template: HTTP redirect, trusted local HTTPS, exact web artifacts and private-file rejection passed');
  }
  let room = '';
  if (!browserHost) {
    startNative(signaling);
    for (let attempt = 0; attempt < 200; ++attempt) {
      room = hostOutput.match(/ROOM ([A-Za-z0-9_-]+)/)?.[1] ?? '';
      if (room || host.exitCode !== null) break;
      await pause(50);
    }
    assert.ok(room, 'Native host did not create a room');
  }
  browser = spawn('chromium', ['--headless', '--no-sandbox', '--disable-gpu',
    ...browserOptions,
    '--remote-debugging-port=0', `--user-data-dir=${profile}`, 'about:blank'], { stdio: 'ignore' });
  let endpoint;
  for (let attempt = 0; attempt < 100; ++attempt) {
    try {
      const [port, path] = (await readFile(join(profile, 'DevToolsActivePort'), 'utf8')).trim().split('\n');
      endpoint = `ws://127.0.0.1:${port}${path}`;
      break;
    } catch { await pause(50); }
  }
  assert.ok(endpoint, 'Chromium did not start');
  debuggerSocket = new WebSocket(endpoint);
  await once(debuggerSocket, 'open');
  let sequence = 0;
  const pending = new Map();
  debuggerSocket.on('message', bytes => {
    const message = JSON.parse(bytes.toString());
    if (message.method === 'Runtime.exceptionThrown')
      console.error('Browser exception:', message.params.exceptionDetails);
    if (message.method === 'Log.entryAdded' && message.params.entry.level === 'error')
      console.error('Browser log:', message.params.entry.text);
    const request = pending.get(message.id);
    if (!request) return;
    pending.delete(message.id);
    if (message.error) request.reject(new Error(JSON.stringify(message.error)));
    else request.resolve(message.result);
  });
  const command = (method, params = {}, sessionId) => new Promise((resolve, reject) => {
    const id = ++sequence;
    pending.set(id, { resolve, reject });
    debuggerSocket.send(JSON.stringify({ id, method, params, sessionId }));
  });
  const page = `${browserOrigin}/coop-browser-test.html?` +
    new URLSearchParams({ signal: browserSignaling, room, ...(browserHost ? {host:'1'} : {}),
      ...(overflowPackets ? {overflow:'packets'} : overflowSignals ? {overflow:'signals'} : {}),
      ...(rejectPeer ? {hold:'1'} : {}) });
  const { targetId } = await command('Target.createTarget', { url: 'about:blank' });
  const { sessionId } = await command('Target.attachToTarget', { targetId, flatten: true });
  await command('Runtime.enable', {}, sessionId);
  await command('Log.enable', {}, sessionId);
  await command('Page.enable', {}, sessionId);
  if (relayTest || stunTest) {
    await command('Page.addScriptToEvaluateOnNewDocument', {source: `
      window.iceEvidence = [];
      window.icePeerCount = 0;
      const OriginalPeer = RTCPeerConnection;
      window.RTCPeerConnection = class extends OriginalPeer {
        constructor(options) {
          const configuration = ${relayTest} ? {...options, iceTransportPolicy:'relay',
            iceServers:options.iceServers.filter(server =>
              server.urls.startsWith('${tlsRelayTest ? 'turns:' : 'turn:'}') &&
              server.urls.endsWith('?transport=tcp'))} : options;
          super(configuration);
          const peer = window.icePeerCount++;
          if (${stunTest} && (!options.iceServers.length ||
              options.iceServers.some(server => !server.urls.startsWith('stun:'))))
            throw new Error('STUN-only fixture received another ICE server type');
          const capture = async () => {
            try {
              const stats = await this.getStats();
              for (const value of stats.values()) {
                if (value.type !== 'transport' || !value.selectedCandidatePairId) continue;
                const pair = stats.get(value.selectedCandidatePairId);
                const local = stats.get(pair.localCandidateId);
                const remote = stats.get(pair.remoteCandidateId);
                window.iceEvidence.push({peer, type:local.candidateType,
                  remoteType:remote.candidateType, protocol:local.relayProtocol});
              }
            } catch {}
          };
          this.addEventListener('connectionstatechange', capture);
          const timer = setInterval(capture, 20);
          this.addEventListener('connectionstatechange', () => {
            if (this.connectionState === 'closed') clearInterval(timer);
          });
        }
      };`}, sessionId);
  }
  await command('Page.navigate', {url:page}, sessionId);
  const browserState = async () => {
    const result = await command('Runtime.evaluate', {
      expression:'({...document.body?.dataset})', returnByValue:true,
    }, sessionId);
    return result.result?.value ?? {};
  };
  if (browserHost) {
    for (let attempt = 0; attempt < 200; ++attempt) {
      room = (await browserState()).room ?? '';
      if (room) break;
      await pause(25);
    }
    assert.ok(room, 'Browser host did not create a room');
    startNative(signaling, room);
  }
  if (rejectPeer) {
    let ready = false;
    // Allow ICE candidate fallback and DTLS retries before injecting a fault.
    for (let attempt = 0; attempt < 2000; ++attempt) {
      ready = (await browserState()).ready === 'yes';
      if (ready) break;
      await pause(25);
    }
    assert.ok(ready, 'Healthy connection did not open before the malformed peer joined');
    const waitPaused = async () => {
      for (let attempt = 0; attempt < 200; ++attempt) {
        if (browserHost ? (await browserState()).paused === 'yes' : hostOutput.includes('PAUSED')) return;
        await pause(10);
      }
      assert.fail('Host did not pause application polling for the queue overflow control');
    };
    if (overflowSignals) {
      assert.ok(browserHost, '--overflow-signals requires --browser-host');
      await command('Runtime.evaluate', {expression:'document.body.dataset.requestPause = "yes"'}, sessionId);
      await waitPaused();
    }
    const header = await readFile(new URL('../../include/kf/net/codec.h', import.meta.url), 'utf8');
    const protocol = Number(header.match(/KF_NET_PROTOCOL_VERSION\s*=\s*(\d+)/)[1]);
    let attempted = false;
    const admission = {type:'join', room, protocol, credential:'3'.repeat(64),
      resources:'crossplay-integration-test', recipe:'test-recipe'};
    if (rejectPacket) {
      const attempt = await command('Runtime.evaluate', {awaitPromise:true, returnByValue:true,
        expression:`(async () => {
          let timer;
          try {
            return await new Promise((resolve, reject) => {
              timer = setTimeout(() => reject(new Error('Rogue data channel did not open')), 10000);
              const socket = new WebSocket(${JSON.stringify(browserSignaling)});
              let pc, chain = Promise.resolve(), candidates = [];
              window.rogueTransport = {socket};
              const send = value => socket.send(JSON.stringify(value));
              socket.onopen = () => send(${JSON.stringify(admission)});
              socket.onerror = () => reject(new Error('Rogue room socket failed'));
              socket.onmessage = event => {
                chain = chain.then(async () => {
                  const message = JSON.parse(event.data);
                  if (message.type === 'joined') {
                    pc = new RTCPeerConnection({iceServers:message.iceServers});
                    window.rogueTransport.pc = pc;
                    pc.onicecandidate = event => {
                      if (event.candidate) send({type:'signal', to:0,
                        candidate:event.candidate.candidate, mid:event.candidate.sdpMid ?? '0'});
                    };
                    const actions = pc.createDataChannel('actions', {ordered:true, negotiated:true, id:0});
                    pc.createDataChannel('state', {ordered:false, maxRetransmits:0, negotiated:true, id:1});
                    window.rogueTransport.actions = actions;
                    actions.onopen = () => {
                      if (!${overflowPackets}) actions.send('invalid text packet');
                      resolve(true);
                    };
                  } else if (message.type === 'signal' && message.sdp) {
                    await pc.setRemoteDescription({type:message.descriptionType, sdp:message.sdp});
                    for (const candidate of candidates) await pc.addIceCandidate(candidate);
                    candidates = [];
                    await pc.setLocalDescription(await pc.createAnswer());
                    send({type:'signal', to:0, descriptionType:'answer', sdp:pc.localDescription.sdp});
                  } else if (message.type === 'signal' && message.candidate) {
                    const candidate = {candidate:message.candidate, sdpMid:message.mid};
                    if (pc.remoteDescription) await pc.addIceCandidate(candidate);
                    else candidates.push(candidate);
                  } else if (message.type === 'error') reject(new Error(message.reason));
                }).catch(reject);
              };
            });
          } finally { clearTimeout(timer); }
        })()`}, sessionId);
      if (attempt.result?.value !== true) {
        const details = await command('Runtime.evaluate', {returnByValue:true, expression:`({
          page:{...document.body.dataset},
          rogue:window.rogueTransport?.pc && {
            connection:window.rogueTransport.pc.connectionState,
            ice:window.rogueTransport.pc.iceConnectionState,
            gathering:window.rogueTransport.pc.iceGatheringState,
            local:window.rogueTransport.pc.localDescription?.sdp.match(/^a=candidate:.*/gm),
            remote:window.rogueTransport.pc.remoteDescription?.sdp.match(/^a=candidate:.*/gm),
            signaling:window.rogueTransport.pc.signalingState,
            channel:window.rogueTransport.actions?.readyState},
          peers:[...(Module.kfTransports?.values() ?? [])].flatMap(session =>
            [...session.peers].map(([slot, peer]) => ({slot, connection:peer.pc.connectionState,
              ice:peer.pc.iceConnectionState, signaling:peer.pc.signalingState,
              gathering:peer.pc.iceGatheringState,
              local:peer.pc.localDescription?.sdp.match(/^a=candidate:.*/gm),
              remote:peer.pc.remoteDescription?.sdp.match(/^a=candidate:.*/gm)})))
        })`}, sessionId);
        assert.fail(`${JSON.stringify(attempt.exceptionDetails)}; ${JSON.stringify(details.result?.value)}`);
      }
      attempted = true;
      if (overflowPackets) {
        await waitPaused();
        await command('Runtime.evaluate', {
          expression:`(() => {
            for (let i = 0; i < 256; ++i) window.rogueTransport.actions.send(new Uint8Array([9,9,9,9]));
          })()`,
        }, sessionId);
      }
    } else {
      rogue = new WebSocket(signaling);
      rogue.on('message', async bytes => {
        const message = JSON.parse(bytes.toString());
        if (overflowSignals && message.type === 'joined' && !attempted) {
          attempted = true;
          for (let i = 0; i < 40; ++i) {
            rogue.send(JSON.stringify({type:'signal', to:0, descriptionType:'answer', sdp:'x'.repeat(60000)}));
            if (i % 4 === 3) await pause(5);
          }
        } else if (!overflowSignals && message.type === 'signal' && message.descriptionType === 'offer' && !attempted) {
          attempted = true;
          rogue.send(JSON.stringify({type:'signal', to:0, descriptionType:'answer', sdp:'not an SDP'}));
        }
      });
      await once(rogue, 'open');
      rogue.send(JSON.stringify(admission));
    }
    let rejected = false;
    for (let attempt = 0; attempt < 200; ++attempt) {
      rejected = browserHost ? Boolean((await browserState()).rejected) : hostOutput.includes('REJECTED ');
      if (rejected) break;
      await pause(25);
    }
    assert.ok(attempted && rejected, 'Host did not isolate the malformed peer');
    if (overflowPackets || overflowSignals) {
      const reason = browserHost ? (await browserState()).rejected : hostOutput;
      assert.ok(reason.includes(`Peer ${overflowPackets ? 'packet' : 'signaling'} queue exceeded its limit`), reason);
    }
    // Keep its room socket open and send late callbacks/signals after rejection.
    // The healthy peer must still finish its exchange in the same room.
    for (const message of [
      {type:'signal', to:0, candidate:'candidate:invalid', mid:'0'},
      {type:'signal', to:0, descriptionType:'answer', sdp:'still invalid'},
    ]) {
      const text = JSON.stringify(message);
      if (rogue) rogue.send(text);
      else await command('Runtime.evaluate', {
        expression:`window.rogueTransport.socket.send(${JSON.stringify(text)})`,
      }, sessionId);
    }
    await pause(100);
    await command('Runtime.evaluate', {expression:'document.body.dataset.release = "yes"'}, sessionId);
    const fault = overflowPackets ? 'packet queue overflow' : overflowSignals ? 'signaling queue overflow' :
      rejectPacket ? 'a text data-channel packet' : 'malformed SDP';
    console.log(`${browserHost ? 'Browser' : 'Native'} host isolated ${fault} and ignored late signals`);
  }
  let outcome;
  for (let attempt = 0; attempt < 650; ++attempt) {
    const result = await command('Runtime.evaluate', {
      expression: '({result: document.body?.dataset.result, detail: document.body?.dataset.detail, status: document.body?.innerText})',
      returnByValue: true,
    }, sessionId);
    outcome = result.result?.value;
    if (outcome?.result) break;
    await pause(100);
  }
  assert.equal(outcome?.result, 'passed', outcome?.detail ?? `Browser test did not finish: ${outcome?.status}`);
  const [code, signal] = await hostExit;
  assert.equal(signal, null);
  assert.equal(code, 0, 'Native host did not receive the browser acknowledgment');
  if (httpsTest) {
    const context = await command('Runtime.evaluate', {
      expression:'location.protocol === "https:" && window.isSecureContext', returnByValue:true,
    }, sessionId);
    assert.equal(context.result?.value, true, 'Transport fixture must run in a secure HTTPS context');
    console.log('Browser admission and packet exchange passed through nginx /rooms WSS upgrade');
  }
  if (relayTest) {
    const stats = await command('Runtime.evaluate', {expression:'window.iceEvidence', returnByValue:true}, sessionId);
    assert.ok(stats.result?.value?.some(value => value.type === 'relay' && value.protocol === relayProtocol),
      `No selected ${relayProtocol.toUpperCase()} relay candidate: ${JSON.stringify(stats.result?.value)}`);
    console.log(`Browser selected ${relayProtocol.toUpperCase()} TURN relay; native host used its UDP-capable transport`);
  }
  if (stunTest) {
    const stats = await command('Runtime.evaluate', {expression:'window.iceEvidence', returnByValue:true}, sessionId);
    const evidence = stats.result?.value ?? [];
    assert.ok(evidence.some(value => value.peer === 0), 'No selected direct candidate pair');
    if (browserHost && rejectPacket)
      assert.ok(evidence.some(value => value.peer === 1), 'No selected browser-to-browser candidate pair');
    assert.ok(evidence.every(value => value.type !== 'relay' && value.remoteType !== 'relay'),
      `STUN-only exchange used a relay: ${JSON.stringify(evidence)}`);
    const types = [...new Set(evidence.map(value => `${value.type}/${value.remoteType}`))];
    console.log(`STUN-only ICE configuration established direct connections without relay candidates (${types.join(', ')})`);
  }
  console.log(outcome.detail);
} finally {
  rogue?.terminate();
  debuggerSocket?.close();
  if (host && host.exitCode === null) host.kill('SIGKILL');
  if (browser && browser.exitCode === null) {
    const exited = once(browser, 'exit');
    browser.kill('SIGKILL');
    await exited;
  }
  if (turn && turn.exitCode === null) {
    const exited = once(turn, 'exit');
    turn.kill('SIGKILL');
    await exited;
  }
  if (proxy && proxy.exitCode === null) {
    const exited = once(proxy, 'exit');
    proxy.kill('SIGTERM');
    await exited;
  }
  await service?.close();
  await new Promise(resolve => pages.close(resolve));
  await rm(profile, { recursive: true, force: true, maxRetries: 5 });
}
