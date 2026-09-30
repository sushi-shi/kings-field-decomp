# Port status

Linux and WebAssembly build and run the original game/opening through native
platform, rendering, audio and save interfaces. See the
[technical constraints](port-findings.md) before changing behavior.

## Verified scope

- User checks cover Linux controls, general rendering, music/menu/attack sounds
  and brief combat. Native save/load worked through the original menus and a
  fresh process.
- The user confirmed browser audio. Isolated Chromium checks restored the resource
  cache and preserved a seeded save byte-for-byte across a full browser restart.
  Original-menu save/load also passed across a page reload. Loading that save
  through the game menu after a full browser restart remains unexercised.
- The reported plaque edge and flame behavior were reproduced/confirmed in retail;
  neither is an active port defect. The renderer is not claimed pixel-exact.
- Forced GAME/OPEN cycles and floor-entry/save comparisons passed sanitizers.
  They do not establish natural story completion or full-game coverage.

## Remaining work

| Item | Next useful step |
| --- | --- |
| Natural ending and re-entry | Use a suitable save or user-reached ending to verify the original transition and starting another game without stale state. |
| MAGIC bar once stayed empty | Deferred at the user's request until a reproduction or new failing-state evidence arrives. No demonstrated defect; do not reset charge speculatively. |
| Rendering fidelity | Compare equivalent retail inputs/frames; remaining differences include launch-model modulation, GL coverage/interpolation and presentation chronology. |
| Collision result types | Audit producers/consumers before refactoring `map_object_probe_door_closing`; high-word kinds, rejection flags, all-ones and literal-one results are not one uniform bitset. |
| Resource boundaries | Carry end pointers through unchecked traversal; OPEN packet unions, animation metadata and upstream projection lifetimes still need separate work. |

The scheduled GAME TMD enqueuer conversion is complete. C-style casts and other
legacy views remain elsewhere; choose any next family explicitly and preserve
its original behavior. English support and the in-game language switch are in
PR #34, separate from the current `port` branch.

Work on the current user-selected task. Keep verification claims bounded to what
was exercised, use isolated diagnostic state, and do not restart or steer the
user's session without authorization. Update this file when current status changes;
commit history records the implementation sequence.
