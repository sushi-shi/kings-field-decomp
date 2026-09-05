# GAME actor cone search

## Function Match Plan and pre-edit evidence

Target: `GAME.EXE:0x8002d7f8 actor_pool_find_target_in_cone`, 388 bytes,
`src/game/actor.c` / `game.actor`; initially 96.804120% strict objdiff.
GAME begins at 270/362 exact, the actor unit at 22/29. Retail hashes were
validated with `kf init`, and all six matcher semantic views were read with
`--image game`, including unfiltered incoming and outgoing references.

All six proven call sites were inspected in retail, including their argument
producers, stack output pointer and result use:

| Caller / call site | Facing input | Range / tolerance | Distance slot |
| --- | --- | --- | --- |
| `player_use_item` / `80018408` | `lh`, camera yaw | 6000 / `0x155` | frame+24 |
| `player_update` / `800196f0` | camera-yaw `lhu`, then explicit `sll`/`sra` | 20000 / `0x155` | frame+112 |
| `player_update` / `8001975c` | `lh`, camera yaw | 20000 / `0x555` | frame+112 |
| `effect_update_dispatch` / `8003978c` | `lh`, effect rotation at +`0x1e` | 20000 / `0x555` | frame+120 |
| `magic_cast` / `8003a3a0` | `lh`, camera yaw | 20000 / `0x155` | frame+104 |
| `magic_cast` / `8003a5d4` | `lh`, camera yaw | 20000 / `0x155` | frame+120 |

- Callers provide a position pointer through `$a0` and consume `$v0` as an
  actor pointer: null-test, publish a target, derive an actor index, or pass
  the actor to a display/effect operation. The fifth argument points to a
  word output, passed at caller frame+16. Every facing value is already a
  promoted signed halfword. No wider caller input has been found.
- Retail has an 88-byte frame and a 128-record scan, starting its `u16`
  countdown at 127. It filters lifecycle 1, excludes action `0x7f` and the
  current actor, rejects distance -1, and retains the strictly smallest
  angular error within tolerance. Best error starts at 30000; pointer and
  distance start at zero. The two derived actor cursors advance by 72 bytes.
- Two validated HI16/LO16 pairs target `actor_state+0x720` (the shared
  `KfActor[128]` member) and `actor_state+0x2b3c` (current actor). Both
  direct calls are proven: `actor_distance_to_point` at `0x8002d8a0` and
  `vector_xz_to_angle` at `0x8002d8cc`. No strings, candidate outgoing
  references or indirect control transfers occur in the target.
- The distance call supplies point-Y sentinel `0xffff` in its delay slot,
  forwards the word range on the stack and passes both heights as zero.
  Its callee explicitly skips vertical testing for that sentinel. The
  angle call's delay slot computes the Z difference.
- The angle result minus facing is masked by `0xfff`. Retail copies the
  wrapped value into the folded result in the branch delay slot, subtracts
  it from `0x1000` only above 2048, and forms a signed-halfword value for
  comparisons. The current C instead has word-sized wrapped/folded locals
  plus a separate short comparison copy, reproducing the event selector's
  previously closed extra-copy symptom.
- The loop tests the old narrowed counter and decrements in the delay
  slot. Its output pointer is loaded from frame+104; the selected distance
  is written and the selected actor returned. The `jr` delay slot releases
  all 88 frame bytes. These boundaries and constants remain unchanged.
- Both neighbors were inspected: exact `actor_try_attack_player`
  (`0x8002d6a0`, 344 bytes) and `actor_distance_to_point`
  (`0x8002d97c`, 240 bytes). The latter, the angle helper, the exact event
  selector, source history and shared actor/effect layouts were reviewed.
- Vendor negative control: this is game-owned actor-selection policy,
  absent from vendored/FID inventories. Its direct callees are game helpers;
  their `SquareRoot0` and `catan` dependencies have separate Psy-Q archive/FID
  attribution and authentic `LIBGTE.H` declarations. No SDK body is counted.

First hypothesis: make the wrapped and folded angle locals `s16`, removing
the redundant comparison copy, while keeping the current interface. If the
argument-save difference remains, separately test `s16 facing`, supported by
all six caller value chains. Lack of input extension alone does not prove an
`s32` parameter when the angle operation discards high bits. Keep the range,
tolerance, data ownership, call set, loop and profile unchanged. Require
strict 100%, all 97 encoded words and ordered relocations before closure.

## Focused results

The two short locals reproduce the complete loop and angle calculation.
The only remaining instruction difference is the incoming-facing save:
retail places `move s8,a1` after the two word-argument stack stores, while
the current `s32` parameter emits it before them. Test the separately planned
`s16 facing` signature now, consistently in the source, shared declaration
and curated identity. Every inspected caller already supplies that exact
promoted signed-halfword value; no caller cast or stored field is changed.

The locals-only trial left the argument-save difference (99.0% textual
similarity). The separately tested short-facing interface closes it. The kept
source uses `s16` facing, wrapped delta and folded error, retaining word range
and tolerance. This is a caller-supported source reconstruction and an exact
probe result, not proof of a uniquely recoverable historical typedef or a
particular optimizer mechanism.

## Final verdict and verification

- Strict canonical objdiff: **100%**, up from 96.804120%; actor unit **23/29**
  exact, up from 22/29; GAME **271/362**, up from 270/362.
- All **97 raw instruction words** agree, including every immediate, branch
  displacement and delay slot. All six ordered relocation entries agree:
  HI16/LO16 `actor_state+0x720` at relative `0x48/0x4c`, HI16/LO16
  `actor_state+0x2b3c` at `0x78/0x7c`, `R_MIPS_26 actor_distance_to_point`
  at `0xa8`, and `R_MIPS_26 vector_xz_to_angle` at `0xd4`.
- The full report comparison changes only this function. All other **483**
  function rows, including SDK verification controls and all caller units,
  retain their scores and extents. The item-use caller unit stays 3/3 exact.
  OPEN remains 97/108; PSX remains 1/1. No vendored body or denominator changed.
- Focused object rebuilding, canonical matching and the full `kf build` ran.
  The full build still exits nonzero on existing closure work: source data
  **11/59**, target relink **108/114** with six conflicting section bases,
  and incomplete known-reference ownership. The two config-backed SDK data
  contributions still pass. No gate was weakened.
- `ruff check scripts tests`, all **551** repository tests (47.198 seconds),
  and `git diff --check` pass. Only the exact GAME selector is selected for
  banking; generated evidence remains under ignored `build/` paths.

The related event and actor selectors both needed short angle temporaries.
Their interfaces are not blindly unified: the actor's word tolerance and
range remain as supported by its call sites and retail comparisons.
