# Map-event movement, queries and saved records

## Function Match Plan

Starting at `80e4ee7`, audit every numeric/character expression in
`src/game/map_event.c`, `map_events.c` and `map_load.c`. The six semantic views,
source histories and surrounding claims were captured before edits under
`build/constant-names/map-event-literals/`. All queries select GAME.EXE and
hash-verified retail. These are game record, movement and floor-policy
functions; SDK `rand`, trigonometry and square root remain library callees.
No library body is reconstructed or counted as progress.

Reuse the existing angle, distance-reduction, collision and volume domains.
Name the shared floor-slot extent, record-stream offset and lossy saved-yaw
shift across save and restore. Use the actual link member's extent for its
complete byte copy. Preserve the stream-pointer arithmetic order, byte copy
loop, signed locals, branch predicates, delay slots and relocation addends.
Replace five raw actor-state byte offsets with definition 7's existing
animation-slot fields, checked against the inverse floor-5 ambient script.

The before/final function table and literal ledger below cover all three
modules, including functions whose bodies require no edits. Every exact
function must retain 100%; the two partial parsers must retain their code and
scores. Force affected compilation and compare all 112 objects and 484 strict
scores with the immutable pre-campaign baseline, independently resolve exact
functions against retail, then run lint, repository tests and full `kf build`.
No new tests or size assertions are intended.

## Evidence and retained tuning

`map_event_distance_to_point` at `80033ae4` checks an X/Z square before its
distance calculation. Signed coordinate deltas are shifted right by three
before squaring; the square root is shifted left by three afterward. This is
the established `KF_LENGTH_SQUARE_DOWNSHIFT` contract, including arithmetic
rounding of negative coordinates. A negative result is the ordinary query
miss sentinel. The cone helper wraps to twelve angle bits and folds values
strictly above a half turn; retain `half turn + 1` rather than changing the
boundary. Its initial best angle 30000 exceeds every folded angle (0..2048)
while fitting s16. The reason for that particular sentinel is unknown.

Wandering passes 70 to `angle_approach`: about 6.15234 degrees per update, with
clamping along the shorter arc. It passes 20 to the Q11 vector scaler after
Q12 `{-sin, -cos}` generation, yielding a nominal 40-world-unit displacement
(one fiftieth of a map tile), with integer rounding by component. Keep these
rates explicit rather than claiming a rationale for their chosen magnitudes.
The SDK's `RAND_MAX` is 32767; shifting by three maps its 15-bit output to a
12-bit yaw. Threshold 1584 admits 1584 of 32768 possible outputs (4.83398%)
only after an unblocked move has reached the current heading. That is an
accepted-output fraction, not proof of independent random trials or elapsed
time. No original tuning rationale is established.

The wander query passes height-ignore 65535, zero height, and flags 0x8040.
`collision_query_world` decodes the low 0x40 as skipping the map-event pool;
bits above eight select collision-cell flags. Bit 0x80 rejects the cell before
occupant traversal. Keep that cell bit explicit: the operation is proven but
its authored map meaning is not. The query still considers terrain, player,
actors and map objects. Occupancy is decremented before movement and incremented
afterward, preventing the moving event from contributing its own cell count.

The looping-animation sound uses full volume, an exclusive cutoff of 18000
world units and attenuation-distance parameter 50000. The audio callee uses
`(50000 - distance) / 50000` in Q7 before panning, then refuses playback at
distance >= 18000. Thus the cutoff is nine tile lengths while the attenuation
scale is twenty-five; 50000 is not an audible radius. At the cutoff the ideal
unquantized attenuation is still 64%, but the sound is suppressed. These
unusual authored distances remain explicit, without inventing a sound identity.
The ambient countdown reloads 10 after its zero update, so scripts run once
per eleven pool updates.

Saved world state comprises five 1700-byte floor slots. The variable record
stream begins ten bytes into each slot, after the persistent script prefix.
Both parsers use `base - (1700 - 10) + 1700 * floor`; retain this operation
order while naming the shared format constants. The complete link payload is
eight bytes. Two adjacent ten-object drop groups save yaw as the high eight
bits of a twelve-bit turn; restoration shifts left by four and deliberately
does not recover the discarded low bits.

The floor-5 reset bytes 0x438, 0x43e, 0x43f, 0x440 and 0x441 are
`7 * 0x98 + 0x0e + {2,8,9,10,11}`: definition 7's melee, effect0/1/2 and
multi-hit animation slots. The floor-5 ambient script at `800342ec` fills
those same slots with clip IDs 2, 3, 3, 3 and 1. The reset writes the existing
`KF_ACTOR_ANIMATION_NONE` encoding. This supports field/slot names, not a
proper creature name or a type change for resource animation IDs.

## Literal coverage and verification

The [per-occurrence ledger](game-map-event-literal-ledger.md) accounts for all
157 ordinary numeric/character tokens retained in these three sources, down
from 179. The 40 address/extent tokens in `ADDRESS`, `DATA` and `RODATA` remain
literal ownership claims: their exact values are the build's retail evidence.
The one source enum value, ambient countdown 10, is a named definition and is
explained above. The three saved-format values live in the shared header and
are likewise explained above. Comments, string contents and digits embedded
in identifier names are not C literal expressions. No uncategorized token was
accepted by the review ledger.

All three affected objects were forcibly rebuilt. All 112 reconstruction
objects retain their complete non-debug sections, including symbols and
relocations; only these three objects' `.debug_line` sections differ. All 484
strict function scores and the complete objdiff report are identical to the
post-`80e4ee7` baseline. The 15 exact campaign functions reproduce all 752
retail instruction words and their ordered calls/data referents. Both partial
parsers retain their complete emitted code. No functions were newly banked.

Modern Clang passes all three changed source variants. Ruff and whitespace
checks pass. The existing suite ran 656 tests: 655 pass, with only the known
untracked save/load-hub instruction mismatch failing. No repository tests or
size assertions were added. Full `kf build` retains the existing data and
ownership failures: source data 7/60; SDK data 4/4; target relink PSX 1/1,
GAME 75/77 and OPEN 34/38. Those broader closure gaps remain open.

The ten unresolved `unknown_` source lines are unchanged. This audit covers
the three named modules only; the repository-wide naming and literal work
is still in progress.

## Function snapshots

All rows use `probe-gcc257-o2-g0`; the signature and load/store widths are
unchanged. Calls below count direct call instructions, including repeated
calls to the same callee. Full control flow and referents are in the captured
six-view dossiers.

| GAME VA / bytes | Function | Initial = final strict % | Direct calls | Final verdict / linked words |
| --- | --- | ---: | ---: | --- |
| `0x8003379c / 16` | `map_event_set_current` | 100 | 0 | Exact, unchanged / 4 |
| `0x800337ac / 116` | `map_event_refresh_dialogue_stage` | 100 | 0 | Exact, unchanged / 29 |
| `0x80033820 / 152` | `map_event_advance_animation_blocking` | 100 | 3 | Exact, unchanged / 38 |
| `0x800338b8 / 556` | `map_event_pool_load` | 100 | 1 | Exact, unchanged / 139 |
| `0x80033ae4 / 168` | `map_event_distance_to_point` | 100 | 1 | Exact, unchanged / 42 |
| `0x80033b8c / 324` | `map_event_pool_find_target_in_cone` | 100 | 2 | Exact, unchanged / 81 |
| `0x80033cd0 / 176` | `map_event_pool_find_overlap` | 100 | 1 | Exact, unchanged / 44 |
| `0x800356e8 / 32` | `map_event_timers_reset` | 100 | 0 | Exact, unchanged / 8 |
| `0x80035708 / 472` | `map_event_update_wander` | 100 | 9 | Exact, unchanged / 118 |
| `0x800358e0 / 140` | `map_event_update_animation_loop` | 100 | 1 | Exact, unchanged / 35 |
| `0x8003596c / 496` | `map_event_pool_update` | 100 | 8 | Exact, unchanged / 124 |
| `0x80035b5c / 696` | `map_world_state_persist` | 94.8218 | 0 | Partial, unchanged |
| `0x80035e14 / 48` | `map_unload_floor` | 100 | 3 | Exact, unchanged / 12 |
| `0x80035e44 / 1692` | `map_restore_floor_state` | 99.9645 | 18 | Partial, unchanged |
| `0x800364e0 / 116` | `map_refresh_dialogue_stages` | 100 | 1 | Exact, unchanged / 29 |
| `0x80036554 / 164` | `map_load_floor` | 100 | 4 | Exact, unchanged / 41 |
| `0x800365f8 / 32` | `func_800365f8` | 100 | 1 | Exact, unchanged / 8 |
