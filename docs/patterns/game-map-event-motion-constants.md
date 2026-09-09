# Map-event motion and sound constants

## Function Match Plan

Continue from `409ae33`, using hash-verified GAME retail and unchanged
`probe-gcc257-o2-g0`. Capture all seventeen claims in the map-event query,
runtime and floor-load modules: address/extent, CFG and disassembly,
callers/callees, strings, strict match and source history. Review the angle
approach, forward-vector, Q11 scaling, spatial audio and collision consumers.
The [earlier event audit](game-map-event-literals.md) establishes the units
and the inverse saved-record format. These are game event and floor policies;
SDK RNG, trigonometry and square root remain external library bodies.

Name the cone's initial best angular error, wandering turn cap and vector
scale, heading-reconsideration threshold and yaw extraction, animation-loop
sound distances, and the RNG fraction used for restored drop positions.
Preserve signed halfword/word types, all predicates, RNG draws, operation
order, byte-stream reads, sound arguments, referents and delay slots. The
collision-cell bit's authored meaning and serialized unknown fields remain
unresolved; do not invent identities for them.

Hypothesis: naming preserves all emitted runtime. Freeze all 112 source
objects and 484 scores, force the three affected compiles, independently
compile frozen before/after source sets, and compare allocated sections,
symbols and ordered relocations. Resolve every reviewed body and recheck
all exact functions against retail and target objects. Refresh the complete
three-module literal ledger. Run modern checking, inventory, Ruff, existing
tests, whitespace checks and full `kf build`. No new tests or size assertions
are proposed. The two existing partial parsers remain separate matching work.

## Names and units

| Name | Value | Consumer evidence |
| --- | ---: | --- |
| `MAP_EVENT_CONE_INITIAL_BEST_ERROR` | 30000 | GAME `80033bcc` initializes the best signed-halfword angular error. Wrapped/folded candidate angles are 0..2048, so every valid candidate can improve this initial sentinel. Its particular magnitude is unexplained. |
| `MAP_EVENT_WANDER_TURN_STEP` | 70 | `80035738`, the delay slot of `angle_approach`, supplies the shortest-arc turn cap: 70/4096 turn, about 6.15234 degrees per update. |
| `MAP_EVENT_WANDER_VECTOR_SCALE` | 20 | `80035750` supplies the scalar to `vector2s_scale_shift11`. Forward components are Q12 negative sine/cosine; multiplying by 20 and shifting by 11 gives nominal 40-world-unit motion, with component rounding. The scalar itself is not a speed of 20 world units. |
| `MAP_EVENT_WANDER_TURN_RANDOM_LIMIT` | 1584 | `80035854` compares the raw SDK RNG result strictly below 1584, only after a clear move has reached its heading. The accepted set contains 1584 of 32768 outputs; no independence or elapsed-time claim follows. |
| `MAP_EVENT_RANDOM_YAW_SHIFT` | 3 | `80035868` and `800358a0` extract twelve angle bits from the SDK's fifteen-bit random output. One path follows the optional clear-movement reconsideration; the other responds to collision. |
| `MAP_EVENT_LOOP_SOUND_MAX_DISTANCE` | 18000 | `80035958`, the sound-call delay slot, supplies the exclusive audible cutoff in world units. The audio consumer rejects distance at or above this bound. |
| `MAP_EVENT_LOOP_SOUND_ATTENUATION_DISTANCE` | 50000 | `8003593c` prepares the fifth sound argument. It is the linear attenuation denominator in Q7, distinct from the audible cutoff. Floor 5's first event plays the sound after animation wrap. |
| `MAP_RESTORE_POSITION_RANDOM_BITS` | 15 | Arithmetic shifts at `80036048`, `80036084`, `80036180` and `800361bc` scale `rand() * 2000` by 32768. The resulting 0..1999 offset stays inside the saved cell. Both X/Z draws in both restored drop groups use the same contract. |

These are consumer roles, not explanations for the original tuning choices.
The shortest-arc clamp, comparison endpoints, three conditional wander RNG
calls, four restore call sites, collision query arguments and sound draw order
are preserved. The cone's Mirror of Truth caller still supplies its existing
range and tolerance. No animation resource or person identity is inferred.

The source census falls from **141 to 129** numeric occurrences: query module
29 to 28, runtime module 49 to 42, and load module 63 to 59. Eight definitions
replace **twelve inline uses**. The complete
[remaining-literal ledger](game-map-event-literal-ledger.md) now records one
row per token, retaining consumer-specific reasons for all 129 occurrences.
It also incorporates the already-committed sword identities and drop-band
names that made the older 145-occurrence count stale. Ordinary arithmetic,
zero initialization, boolean markers, byte packing and authored floor/link
indices retain their explicit literals. Collision-cell bit `0x80` and the
serialized unknown fields still lack evidence for fuller semantic identities.

## Verification and function verdicts

All 112 independently compiled before/after objects retain their runtime
sections, alignment, symbols and ordered relocations. The live objects agree
with those controls. Only the three changed objects have different debug-line
sections. All 484 strict function scores remain unchanged.

The seventeen reviewed functions preserve 1350 instruction words,
53 direct call sites and 65 ordered data-address pairs.
The fifteen exact functions reproduce all 752 words against both retail
and target objects. No new function is banked. Each row below retains its
initial strict score, signature and widths under `probe-gcc257-o2-g0`.

| GAME VA / bytes | Function | Initial = final strict % | Words / calls / addresses | Verdict |
| --- | --- | ---: | --- | --- |
| 0x8003379c / 16 | `map_event_set_current` | 100.000000 | 4 / 0 / 1 | Exact, unchanged |
| 0x800337ac / 116 | `map_event_refresh_dialogue_stage` | 100.000000 | 29 / 0 / 1 | Exact, unchanged |
| 0x80033820 / 152 | `map_event_advance_animation_blocking` | 100.000000 | 38 / 3 / 0 | Exact, unchanged |
| 0x800338b8 / 556 | `map_event_pool_load` | 100.000000 | 139 / 1 / 2 | Exact, unchanged |
| 0x80033ae4 / 168 | `map_event_distance_to_point` | 100.000000 | 42 / 1 / 0 | Exact, unchanged |
| 0x80033b8c / 324 | `map_event_pool_find_target_in_cone` | 100.000000 | 81 / 2 / 1 | Exact, unchanged |
| 0x80033cd0 / 176 | `map_event_pool_find_overlap` | 100.000000 | 44 / 1 / 1 | Exact, unchanged |
| 0x800356e8 / 32 | `map_event_timers_reset` | 100.000000 | 8 / 0 / 2 | Exact, unchanged |
| 0x80035708 / 472 | `map_event_update_wander` | 100.000000 | 118 / 9 / 1 | Exact, unchanged |
| 0x800358e0 / 140 | `map_event_update_animation_loop` | 100.000000 | 35 / 1 / 4 | Exact, unchanged |
| 0x8003596c / 496 | `map_event_pool_update` | 100.000000 | 124 / 8 / 8 | Exact, unchanged |
| 0x80035b5c / 696 | `map_world_state_persist` | 94.821840 | 175 / 0 / 7 | Partial, unchanged |
| 0x80035e14 / 48 | `map_unload_floor` | 100.000000 | 12 / 3 / 0 | Exact, unchanged |
| 0x80035e44 / 1692 | `map_restore_floor_state` | 99.964540 | 423 / 18 / 32 | Partial, unchanged |
| 0x800364e0 / 116 | `map_refresh_dialogue_stages` | 100.000000 | 29 / 1 / 1 | Exact, unchanged |
| 0x80036554 / 164 | `map_load_floor` | 100.000000 | 41 / 4 / 4 | Exact, unchanged |
| 0x800365f8 / 32 | `map_load_floor_wrapper` | 100.000000 | 8 / 1 / 0 | Exact, unchanged |

The partial save parser first differs at GAME `80035b6c`: source
`lui v1,0x800a`, retail `addiu a1,a0,-0x21f`. The restore parser first
differs at `80035e8c`: source `addiu v0,v0,-0x69a`, retail
`addiu v1,a0,-0x69a`. These unchanged observations do not establish a
compiler mechanism.

Inventory, Ruff and whitespace checks pass. All **680 existing tests pass**
in 85.475 seconds. Modern checking retains the same **300 error diagnostics**
and **65/112** passing source/image variants; all three changed variants pass.
Full `kf build` ran and retains existing data/relink failures: source-data
matches PSX 0/1, GAME 9/42, OPEN 2/19; target relinks PSX 1/1, GAME 75/77,
OPEN 34/38, with six conflicting section bases and zero artifact failures.
No tooling or flake files changed. The ten unresolved `unknown_` source
occurrences and wider constant audit remain open.
