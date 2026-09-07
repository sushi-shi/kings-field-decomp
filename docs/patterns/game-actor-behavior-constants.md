# GAME actor behavior units and timing

## Function Match Plan

Continue from `e6c5c5e` with hash-verified GAME retail and unchanged
`probe-gcc257-o2-g0`. Audit the complete eleven-function actor-behavior module.
Every claim has refreshed address/extent, disassembly/CFG, callers, callees,
strings, strict match and source history. Adjacent selection, animation,
effect-construction, drop and damage consumers constrain signatures and
units. Custom actors, definitions and action policies establish game ownership;
SDK math/RNG and library bodies remain external calls. Candidate pointer
edges are not promoted by this naming pass.

Name selection cutoffs and distance factors, movement height thresholds and
velocities, effect aim/speed/propagation parameters, charge divisors, boss
scatter/timing, action animation contacts, pursuit backoff and drop rolls.
Propagate established Fire Ball, Wind Cutter and Light Needle identities in
the effect-code consumers. Replace the local effect-vector `scale` name with
`speed`; the caller multiplies a unit direction by that magnitude. Preserve
every signature, byte/halfword width, arithmetic sign, RNG draw, inequality,
variadic slot, data owner, call, branch and delay slot. Preserve the home
coordinate read before initialization, the exceptional high-half ceiling
comparison, partially initialized effect arguments and action-specific
progress-byte meanings. No new tests or size assertions are proposed.

The existing [actor audit](game-actor-constants.md) and
[state review](game-actor-states.md) supply consumer-by-consumer explanations;
this pass converts supported tuning roles to names and provides a complete
remaining-occurrence ledger. Exact animation alignment and original tuning
rationale remain unknown. Contact phases are cycle positions, not elapsed
frames; probability cutoffs retain their exact accepted integer ranges.

First hypothesis: naming preserves all emitted runtime bytes and ordered
relocations. Freeze all 112 source objects/headers and 484 scores, force the
affected compile, compare frozen before/after builds, independently resolve
all reviewed words/references, and recheck exact bodies against retail and
targets. Run modern checking, inventory, Ruff, existing tests, whitespace
checking and full `kf build` before committing.

| GAME VA / bytes | Function | Initial strict % |
| --- | --- | ---: |
| 8002e2e8 / 960 | actor_select_next_action | 100 |
| 8002e6a8 / 684 | actor_update_awareness | 97.736840 |
| 8002e954 / 940 | actor_move_xz_with_collision | 100 |
| 8002ed00 / 212 | actor_move_along_heading | 100 |
| 8002edd4 / 1108 | actor_spawn_action_effect | 100 |
| 8002f228 / 244 | actor_prepare_charge_toward_player | 100 |
| 8002f31c / 332 | actor_apply_horizontal_movement | 100 |
| 8002f468 / 240 | actor_update_effect_action | 100 |
| 8002f558 / 884 | actor_apply_random_movement | 100 |
| 8002f8cc / 444 | actor_update_boss_death_sequence | 100 |
| 8002fa88 / 3472 | actor_update_current_action | 100 |

## Named units and exact boundaries

The names express observed consumer roles. They do not explain why the
original author chose a particular tuning value. The initial review already
documents the branch/call evidence; this table records the source-level
relationships now made explicit.

| Family | Values, units and retained behavior |
| --- | --- |
| Pursuit distance | Definition value shifted 8 gives 256-world-unit buckets; recent activity multiplies the result by 6. This is not a Q8 fractional weight. |
| Multi-hit selection | Weight 80 goes to the existing distance/facing selector, whose random draw is shifted to 0..2047 before its distance-dependent weighting. |
| Retreat / idle choices | Retreat is considered within twice awareness and accepts raw RNG values at least 5462. Wandering chooses idle below 1092; idle remains idle at or above 8193. Keep exact endpoints, not rounded probabilities. |
| Spawn chance | Stored byte shifted 7 is compared strictly against the full SDK RNG value. Persistent/homebound policy retains its separate bypass. |
| Heading reconsideration | Yaw shift 3 extracts 12 bits. Wandering and return-home each reconsider below 2048; pursuit below 4096. These have separate names despite equal limits. |
| Pursuit backoff | Shift 11 yields 0..15; base 13 gives initial progress 13..28. Reverse movement precedes the `<11` test, so clear uninterrupted backoff lasts 4..19 reverse updates. Selection can interrupt it. |
| Blocked steering | One turn/64 = 64 angle units, or 5.625 degrees, when the already-blocked actor still faces its intended heading. |
| Drops | Drops greater than 600 use long-drop policy; above 3000 use the stricter policy. Accepted RNG values are inclusively 0..16384 or 0..1024, because rejection tests `threshold < rand()`. |
| Steps | Exclusive height limit 1001 admits rises through 1000. Height at least 400 uses Y velocity -300; smaller rises use -120. Distances and signed velocities retain separate units. |
| Charge projection | Length downshift 2 precedes each projected component's division by 16. Approximately 1/64 of separation per update, with two distinct rounding stages; no promised 64-update arrival. |
| Paired effect placement | Local X offsets ±1500 before actor rotation; player cone query range 50000 world units. |
| Effect speeds | Default 600, alternate Lightning Bolt 800, Wind Cutter 800, actor-spawner/scatter 250 world units/update. `speed` multiplies the Q12 direction; it is not visual scale. |
| Untargeted effect fallback | Alternate Lightning Bolt pitch -32 angle units; its count and the spawner/scatter count start at 20. The kinds consume those counts in different phases, so this is not total visible lifetime. |
| Targeted effect endpoints | Alternate Lightning Bolt aims 1000 Y units above the stored player reference. Scatter subtracts 4500 distance, spawner 2000, before division by speed; the shared minimum remains one movement step. |
| Scatter initialization | Three generations and visual scale 3000, distinct from the movement speed and the equal-valued special-attack phase. |
| Melee / special contact | Phases 2700/3000 begin `[phase, phase+animation_step)` windows. Retail melee comparisons are at GAME `8002fedc/8002fef0`; special at `800301c8/800301dc`. No universal frame duration is inferred. |
| Jump contact / floor threshold | Contact phase 3400 uses the existing crossing predicate. Signed table height greater than -5000 selects velocity -220 and phase step 140; otherwise velocity -300 and phase step 110. Retail compares against -4999 to implement the strict source boundary. The table index still subtracts one. |
| Jump collision response | Actor/player contact bounces with Y velocity -120 and resets animation phase. Player contact supplies striking input 15 to the damage formula; it does not guarantee 15 HP loss. Ceiling collision instead starts downward velocity +100. |
| Return-home animation | Reset sample 1200 preserves the existing mixed move/melee-slot accesses and uninitialized home-coordinate read. |
| Multi-hit contacts | Forward phases 2240/2688/3200/3584 use range 8000..11000. Phases 3500/3800 use range 0..11000 and negative/positive eighth-turn yaw offsets respectively. These are authored independent samples, not a regular cadence. |
| Death drops | Phase 2048 is half the 4096-unit animation period. Gold uses the full 15-bit random value; item chance uses an eight-bit draw from shift 7 and inclusive `<=`. Code 99 suppresses both drop sources, while 255 is an additional definition-drop exclusion; no new placement-255 guard is added. |
| Post-death progress | Bound 7 means seven increments followed by the next-update lifecycle transition, unless the floor-4 transformation bypasses the wait. |
| Boss death sounds / effects | Sound slots 1/2/3 trigger at phases 500/1000/1500. Effect and repeated-sound tests use phase modulo death-step times 2/4; these multipliers do not bypass phase clamping. Repeated-sound volume is 100 on the 0..127 API scale. |
| Boss death scatter | X/Z mask 8191 minus bias 4096 gives -4096..4095 world units. Y subtracts mask 4095, giving upward displacement 0..4095. These masks describe spatial sampling, not yaw or animation phase. |

Five effect-code uses now spell the established Fire Ball, Wind Cutter and
Light Needle identities. Raw alternate effect kinds 22/23/24 and boss visual
kind 44 retain resource-dependent distinctions in the remaining ledger.
The type-0x20 class remains unresolved independently of its named low target
bits. No `unknown_` field gains new consumer evidence from this module.

## Remaining literal coverage

The complete module goes from **268 to 173 inline numeric occurrences**,
removing **95 uses**. `actor_select_next_action` now has none outside named
definitions and retail claims. The
[per-occurrence ledger](game-actor-behavior-literal-ledger.md) accounts for
every remaining value, including repeated same-line tokens. It excludes
comments, strings, digits in identifiers, enum definitions and retail claims;
unary signs belong to the expression rather than the numeric token.

## Final verification

All eleven reviewed function scores remain unchanged: ten exact, awareness
97.736840%. Forced compilation and the full comparison/build were followed
by frozen before/after compilation of all 112 units. Every allocated section,
runtime symbol and ordered relocation is identical; only the actor-behavior
debug-line section changes. The reviewed bodies preserve all 2379 emitted
words, 144 direct-call references and 52 data/address references. All 2209
words across the ten exact functions independently match retail and the
delinked targets, including the complete 868-word action dispatcher.

| Function | Final verdict | Words / calls / addresses |
| --- | --- | ---: |
| actor_select_next_action | Exact, unchanged | 240 / 11 / 2 |
| actor_update_awareness | Partial, unchanged | 170 / 11 / 9 |
| actor_move_xz_with_collision | Exact, unchanged | 235 / 6 / 2 |
| actor_move_along_heading | Exact, unchanged | 53 / 5 / 2 |
| actor_spawn_action_effect | Exact, unchanged | 277 / 11 / 3 |
| actor_prepare_charge_toward_player | Exact, unchanged | 61 / 5 / 5 |
| actor_apply_horizontal_movement | Exact, unchanged | 83 / 1 / 2 |
| actor_update_effect_action | Exact, unchanged | 60 / 5 / 4 |
| actor_apply_random_movement | Exact, unchanged | 221 / 7 / 2 |
| actor_update_boss_death_sequence | Exact, unchanged | 111 / 13 / 7 |
| actor_update_current_action | Exact, unchanged | 868 / 69 / 14 |

Awareness still first differs at GAME `8002e6ac`: source saves `$ra` at
stack +0x38, retail at +0x3c. It emits 170 words versus retail's 171. No new
compiler-mechanism attribution or exactness claim follows from naming.

The 484-row live score comparison contains one independent concurrent change:
effect-map collision at GAME `80037850` moves from 97.452630 to 98.221054.
Its separately captured source compiles identically with both frozen naming
contexts, and both objects equal its live runtime object. The other 111 live
objects agree with the naming control. The collision source, its dossier
and the generated README score panel are excluded from this commit.

All 680 existing tests pass (84.364 seconds). Inventory validation, Ruff and
whitespace checking pass. Modern checking preserves the same 300 errors and
65/112 passing variants. Full `kf build` retains existing data ownership and
placement failures: source data PSX 0/1, GAME 9/42, OPEN 2/19; target relink
PSX 1/1, GAME 75/77, OPEN 34/38, six conflicting section bases and no artifact
failures. No tests, size assertions, flake/tooling changes or banking were
added. The wider naming objective remains open.
