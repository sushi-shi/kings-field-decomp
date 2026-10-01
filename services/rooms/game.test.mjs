// Starts actual clients without synthesizing game input. This verifies room
// admission and world synchronization, not hands-on movement or combat.
import { createRoomService } from './server.mjs';
import { once } from 'node:events';
import { spawn } from 'node:child_process';
import { cp, mkdtemp, readFile, rm, writeFile } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join, resolve } from 'node:path';
import assert from 'node:assert/strict';

const executable = resolve(process.argv[2]);
const data = resolve(process.argv[3]);
assert.ok(process.argv[4], 'Expected a local KFIII disc or character pack after the game data directory');
const characters = resolve(process.argv[4]);
const service = createRoomService();
// Dropping these signaling TCP connections exercises the application's real
// recovery path without injecting gameplay input or adding a production API.
const connections = [];
service.server.on('upgrade', (_request, socket) => connections.push(socket));
service.server.listen(0, '127.0.0.1');
await once(service.server, 'listening');
const roots = [], clients = [];
const url = `ws://127.0.0.1:${service.server.address().port}`;
async function start(args, existingSaves, resources = data) {
  const saves = existingSaves ?? await mkdtemp(join(tmpdir(), 'kf-coop-game-'));
  if (!existingSaves) roots.push(saves);
  const child = spawn('xvfb-run', ['-a', executable, '--data', resources, '--saves', saves, '--signal', url,
    /\.kfa$/i.test(characters) ? '--avatars' : '--avatar-disc', characters, ...args], {
    detached: true, env: { ...process.env, SDL_AUDIODRIVER: 'dummy' }, stdio: ['ignore', 'pipe', 'pipe']
  });
  clients.push(child);
  child.saves = saves;
  child.output = '';
  child.stdout.on('data', bytes => { child.output += bytes; process.stdout.write(bytes); });
  child.stderr.on('data', bytes => { child.output += bytes; process.stderr.write(bytes); });
  return child;
}
async function until(child, pattern, after = 0) {
  const end = Date.now() + 30000;
  while (Date.now() < end) {
    const match = child.output.slice(after).match(pattern);
    if (match) return match;
    assert.equal(child.exitCode, null, child.output);
    await new Promise(resolve => setTimeout(resolve, 100));
  }
  throw new Error(`Timed out waiting for ${pattern}: ${child.output}`);
}
try {
  const host = await start(['--host']);
  const [, room] = await until(host, /Online room: ([^;]+);/);
  const guest = await start(['--join', room]);
  await until(guest, /World synchronized: epoch [1-9]\d*, tick \d+, floor 1/);
  await until(host, /Player 2 admitted/);
  await new Promise(resolve => setTimeout(resolve, 3000));
  let guestOffset = guest.output.length, hostOffset = host.output.length;
  connections[1].destroy();
  await until(guest, /Online connection interrupted; reconnecting/, guestOffset);
  await until(guest, new RegExp(`Online room: ${room}; player 2`), guestOffset);
  await until(guest, /World synchronized: epoch [1-9]\d*, tick \d+, floor 1/, guestOffset);
  await until(host, /Player 2 reconnected/, hostOffset);
  guestOffset = guest.output.length; hostOffset = host.output.length;
  const beforeEpoch = Number([...guest.output.matchAll(/World synchronized: epoch (\d+)/g)].at(-1)[1]);
  connections[0].destroy();
  await until(guest, /Host connection interrupted; waiting/, guestOffset);
  await until(host, /Online room connection restored; synchronizing party/, hostOffset);
  const restored = await until(guest, /World synchronized: epoch (\d+), tick \d+, floor 1/, guestOffset);
  assert.ok(Number(restored[1]) > beforeEpoch, 'Host resume failed to invalidate stale world traffic');
  await until(host, /Player 2 reconnected/, hostOffset);
  assert.equal(host.exitCode, null, host.output);
  assert.equal(guest.exitCode, null, guest.output);
  assert.doesNotMatch(guest.output, /Rejected invalid|Cannot encode|AddressSanitizer|runtime error:/);
  assert.doesNotMatch(host.output, /Cannot encode|AddressSanitizer|runtime error:/);
  assert.equal([...host.output.matchAll(/Player 2 admitted/g)].length, 1, 'Reconnect created a fresh character');
  const exited = once(guest, 'exit');
  hostOffset = host.output.length;
  process.kill(-guest.pid, 'SIGTERM');
  await exited;
  const reopened = await start(['--join', room], guest.saves);
  await until(reopened, new RegExp(`Online room: ${room}; player 2`));
  await until(reopened, /World synchronized: epoch [1-9]\d*, tick \d+, floor 1/);
  await until(host, /Player 2 (returned|reconnected)/, hostOffset);
  assert.equal(host.exitCode, null, host.output);
  assert.doesNotMatch(host.output, /different player|different character/);
  const different = await mkdtemp(join(tmpdir(), 'kf-coop-resources-'));
  roots.push(different);
  await cp(data, different, { recursive: true });
  const changed = join(different, 'COPY.TXT');
  const bytes = await readFile(changed);
  assert.ok(bytes.length);
  bytes[0] ^= 1; // Same paths and sizes, different content in an unused disc file.
  await writeFile(changed, bytes);
  const incompatible = await start(['--join', room], undefined, different);
  await until(incompatible, /Incompatible game resources or protocol/);
  assert.doesNotMatch(incompatible.output, /World synchronized/);
  assert.doesNotMatch(host.output, /Player 3 admitted/);
  console.log('Actual native game admission, reconnect, host resynchronization, profile restart and resource mismatch rejection passed');
} finally {
  for (const child of clients) {
    try { process.kill(-child.pid, 'SIGTERM'); } catch {}
  }
  await service.close();
  await Promise.all(roots.map(path => rm(path, { recursive: true, force: true })));
}
