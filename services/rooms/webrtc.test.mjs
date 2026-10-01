import { createRoomService } from './server.mjs';
import { once } from 'node:events';
import { spawn } from 'node:child_process';
import assert from 'node:assert/strict';

const service = createRoomService();
service.server.listen(0, '127.0.0.1');
await once(service.server, 'listening');
try {
  const child = spawn(process.argv[2], [`ws://127.0.0.1:${service.server.address().port}`], { stdio: 'inherit' });
  const timer = setTimeout(() => child.kill('SIGKILL'), 30000);
  const [code, signal] = await once(child, 'exit');
  clearTimeout(timer);
  assert.equal(signal, null);
  assert.equal(code, 0, 'Native WebRTC integration failed');
  console.log('Four-party WebRTC: rejection reason, 60 KB reliable packets, state channel, guest and host resume passed');
} finally {
  await service.close();
}
