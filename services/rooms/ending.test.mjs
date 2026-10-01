// A bounded terminal snapshot reaches a real game, without navigating a level.
// Run: node services/rooms/ending.test.mjs build/linux /path/to/data /path/to/characters.kfa
import { createRoomService } from './server.mjs';
import { once } from 'node:events';
import { spawn } from 'node:child_process';
import { mkdtemp, rm } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join, resolve } from 'node:path';
import assert from 'node:assert/strict';

const [buildArg, dataArg, packArg] = process.argv.slice(2);
assert.ok(buildArg && dataArg && packArg, 'Expected native build, data directory and character pack');
const build = resolve(buildArg), data = resolve(dataArg), pack = resolve(packArg);
const pause = ms => new Promise(resolve => setTimeout(resolve, ms));
for (const mode of ['close', 'drop', 'ordinary']) {
  const service = createRoomService({ hostGraceMs: 500 });
  service.server.listen(0, '127.0.0.1');
  await once(service.server, 'listening');
  const url = `ws://127.0.0.1:${service.server.address().port}`;
  const saves = await mkdtemp(join(tmpdir(), 'kf-ending-'));
  const children = [];
  function start(executable, args, detached = false) {
    const child = spawn(executable, args, { detached, env: { ...process.env, SDL_AUDIODRIVER: 'dummy' },
      stdio: ['ignore', 'pipe', 'pipe'] });
    child.output = '';
    child.group = detached;
    children.push(child);
    for (const stream of [child.stdout, child.stderr]) stream.on('data', bytes => {
      child.output += bytes; process.stdout.write(bytes);
    });
    return child;
  }
  async function until(child, pattern) {
    const deadline = Date.now() + 30000;
    while (Date.now() < deadline) {
      const match = child.output.match(pattern);
      if (match) return match;
      assert.equal(child.exitCode, null, child.output);
      assert.doesNotMatch(child.output, /Rejected invalid|Cannot encode|AddressSanitizer|runtime error:/);
      await pause(50);
    }
    throw new Error(`Timed out waiting for ${pattern}: ${child.output}`);
  }
  try {
    const host = start(join(build, 'coop-runtime-test'), ['--ending-host', url, pack, mode, data]);
    const [, room] = await until(host, /ROOM (\S+)/);
    const guest = start('xvfb-run', ['-a', join(build, 'kings-field'), '--data', data, '--saves', saves,
      '--signal', url, '--avatars', pack, '--join', room], true);
    await until(host, /SNAPSHOT RECEIVED/);
    if (mode === 'drop') host.kill('SIGKILL');
    if (mode === 'ordinary') {
      const deadline = Date.now() + 10000;
      while (guest.exitCode === null && Date.now() < deadline) await pause(50);
      assert.equal(guest.exitCode, 0, guest.output);
      assert.doesNotMatch(guest.output, /Campaign completed|Playing the campaign ending/);
    } else {
      await until(guest, /Campaign completed; entering the ending/);
      await until(guest, /Playing the campaign ending/);
      await pause(1000);
      assert.equal(guest.exitCode, null, guest.output);
    }
    assert.doesNotMatch(guest.output, /Rejected invalid|Cannot encode|AddressSanitizer|runtime error:|Cannot load/);
    if (mode === 'drop') assert.match(guest.output, /Host connection interrupted/);
    console.log(mode === 'ordinary' ? 'Unfinished room closure returned to lobby' : `Real guest entered the ending after host ${mode}`);
  } finally {
    for (const child of children) {
      try { process.kill(child.group ? -child.pid : child.pid, 'SIGTERM'); } catch {}
    }
    await service.close();
    await rm(saves, { recursive: true, force: true });
  }
}
