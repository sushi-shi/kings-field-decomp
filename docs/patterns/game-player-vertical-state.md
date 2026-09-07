# Player vertical motion and camera height

## Function Match Plan

Starting at `e187d3d`, inspect the GAME.EXE player field at `800a085e`
(`KfPlayerState + de`) across initialization, map synchronization, walking
bob and vertical integration. Use `probe-gcc257-o2-g0`; compiler attribution
remains a probe. The camera offset also crosses restart, death animation and
actor attack-volume construction. The bottomless-pit attribute connects the
Feather Boots exception to the interaction notification.

| GAME VA / retail bytes | Function | Before strict % | Evidence and intended change |
| --- | --- | --- | --- |
| `800151cc / 740` | `game_state_initialize` | 100 | `sb zero` at `80015264` initializes the state, independently of the signed velocity halfword. Name only the state assignment. |
| `800154b0 / 412` | `player_death_restart` | 100 | Calls map synchronization and subtracts 1500 from floor Y at `80015630`; use the shared camera height. |
| `80016ee8 / 344` | `player_sync_position_to_map` | 100 | Signed cell division, unsigned grid byte, negative floor height; `addiu -1500` at `80016fe4`, state reset at `80017024`. Retain two calls and their argument slots. |
| `80017a24 / 92` | `player_update_view_bob` | 100 | `lbu`/nonzero skip; unsigned speed and phase, doubled phase increment, twelve-bit wrap, signed sine shift by six. Name the state and bob conversion. |
| `80017a80 / 632` | `player_update_vertical_motion` | 100 | Byte switch 16/32/0; signed velocity `lh`/`sh`, floor `lw`/`sw`, signed speed comparison at 181. Preserve shared case labels, comparison strictness and one death call. |
| `80018540 / 388` | `player_death_update` | 93.298965 | Shared camera subtraction at `80018628`, followed by vertical motion. Replace the private duplicate; preserve existing non-exact instructions. |
| `8002d6a0 / 344` | `actor_try_attack_player` | 100 | Snapshot camera Y plus 1500 at `8002d708`, height 1700 in the seventh argument. Reuse camera height without conflating it with collision height. |
| `80034de4 / 2308` | `map_interaction_dispatch` | 83.436745 | Attribute 93 branches to notification 24 at `80034f3c..80034f74`. Share its identity with the vertical-motion exception; preserve the rest of the dispatcher. |

The six semantic views, raw CFG/delay slots, ordered references, current match
objects, source history and neighboring functions were captured before edits
under ignored `build/constant-names/player-vertical-state/`. Caller evidence
includes player update, map warp/synchronization and the main-loop transform
snapshot chain. Save read/write copies the full saved player representation;
it does not assign this member numerically. SDK `rsin` remains an authentic
library call. These game-state consumers are not vendored implementations.

The modern view will use a scoped `KfPlayerVerticalState : u8`; the retail C
view retains the existing `u8` storage through `KF_ENUM_BEGIN`. Every explicit
read, write and switch case must retain this domain. Bytewise save copies and
initial zeroing keep their existing behavior, including unrecognized byte
values; a scoped enum is not a save-data validator. No size assertions are added.

## State and units evidence

Zero is the grounded policy: bob advances only then. When the sampled target
floor is above the current foot height (smaller Y), zero changes to `0x20`
and immediately executes the step-up body. Its initial velocity is -300 when
the signed speed is at least 181, otherwise -100; each execution adds velocity
to foot height, then adds 5 to velocity. Reaching or passing the target upward
snaps the foot height to target and returns to zero. This is an automatic
terrain step, not evidence for a user-controlled jump.

When the target is below the current foot height, zero changes to `0x10`,
clears velocity and immediately executes the falling body. Each execution
adds velocity to foot height and then adds 40 to velocity. Only an overshoot
strictly greater than 100 below target snaps to the floor and resets the
state. Neither landing path clears velocity; later transitions set it. The
different acceleration values and overshoot threshold are authored motion
parameters; their original tuning rationale is unknown.

A downward gap greater than 3000 begins death unless Feather Boots are worn
on attribute `0x5d`. That exception skips integration as well as death. The
interaction dispatcher calls notification 24 for this same attribute; its
decoded text is 底なし穴, “Bottomless pit,” as documented in the
[notification review](game-notification-identities.md). Attribute `0x52`
has a separate death test at target Y >= -6999; its environmental identity
still lacks evidence and is not inferred from the other pit type.

Camera Y is `floor_height + view_bob_offset - 1500`, with downward-positive
Y. Restart uses the same expression after clearing bob. Death animation
changes bob, but retains the camera baseline. The main loop copies this camera
position through `player_update_transform_snapshot` and
`actor_set_player_transform`; attack checks add the same 1500 to recover a
foot-height reference that still includes bob. Preserve that bob contribution.
The attack-volume height 1700 is a separate dimension, not a camera offset.

Walking bob advances its angular phase by twice horizontal speed, wraps with
`KF_ANGLE_WRAP_MASK`, and shifts the signed Q12 sine by six. The resulting
nominal amplitude is 64 world units. Keep the arithmetic shift and its negative
rounding; a division rewrite would change it. These constants describe units
and update rules without claiming physical meters, seconds or original design
intent.

## Verification

All eight reviewed bodies retain their strict percentages, raw instructions,
ordered calls and data referents: 1309 candidate words, 77 calls and 194 address
references. Six exact bodies independently reproduce all 641 retail words and
agree with the resolved target objects. The death updater and interaction
dispatcher retain their existing non-exact results; this change does not close
either function.

Their first divergences remain the entry stack adjustment: death update uses
24 candidate bytes versus 32 retail bytes at `80018540`; interaction dispatch
uses 80 versus 72 at `80034de4`. These are unchanged, unattributed codegen
residues, not evidence for changing the recovered constants or state domain.

All 112 units were compiled from frozen before/after source and header trees
using identical source/output paths. Every runtime/data section, symbol and
ordered relocation remains identical. Only the player-core and death-fade
debug line tables differ. Live objects agree with that isolated result except
the independently edited audio module: its spatial-sound function at `80032cf0`
moved from 97.724720% to 100%. That is the only change among all 484 captured
scores and is separate from this naming campaign.

The five affected units were forcibly rebuilt. Inventory validation, Ruff,
all 678 existing tests (81.816 seconds) and whitespace checks pass. Modern
checking retains exactly the previous 320 error diagnostics and 64/112 passing
variants; all five affected modules pass. Temporary compiler controls accept valid player-state assignment,
comparison and local propagation; they reject assigning integer 16 or
`KF_ACTOR_VERTICAL_FALL` to the player field. No permanent tests were added.

Full `kf build` retains the existing data/placement failures: data PSX 0/1,
GAME 9/42 and OPEN 2/19; target relinks 1/1, 75/77 and 34/38 respectively.
There are six conflicting section bases and zero artifact failures. This
batch adds no banks and does not claim a passing full build.

The [motion ledger](game-player-motion-literal-ledger.md) covers all seven
remaining numeric occurrences in the three sync/bob/vertical functions.
The earlier database/startup and death ledgers lose the now-named initial
state and camera height, leaving complete coverage of all 197 numeric
occurrences in `player_death.c` (97 startup, 45 progression, 55 remainder).
The lexer verifies per-expression/token multiplicity and source locations
against the ledgers. Across this batch, 26 inline numeric occurrences become
named constants; the duplicate private death-camera definition is removed.
Other player-core, actor and map-dispatcher literals still need their broader
audits. The separate actor multiplier-unity call site from the preceding
[damage-unit review](game-player-damage-units.md) is also integrated here.
