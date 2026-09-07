# GAME actor selection and combat constants

## Function Match Plan

Baseline `dc8d112`, hash-verified GAME retail, unchanged `probe-gcc257-o2-g0`.
The complete actor core and behavior modules have fresh per-function address,
extent, disassembly/CFG, caller/callee, string, history and strict-match
snapshots. Existing action-selection, damage, cone and animation dossiers
constrain semantics. Custom actor definitions, weighted profiles and effect
population checks establish game ownership; SDK/libc providers remain calls.
No inventory evidence tier, function identity or data owner changes.

Name the base-power weighting divisor, late-death damage cutoff, status RNG
extraction, initial cone ranking error, boss sound distances, selection range
and odds adjustments, facing-bypass limits and spawner population bound.
Share random-yaw extraction, multi-hit range bounds and the boss sound-table
extent through the actor header and existing behavior consumers. Keep all
integer widths, signedness, operation order, narrowings, predicates, calls,
delay slots and data bytes. The multi-hit selector remains generic in its
signature; its sole source caller supplies the multi-hit action.

Affected-function hypotheses: initialization maps 15 RNG bits to 12 yaw bits;
the component formula adds base power divided by five; damage excludes dying
actors at phase 1548; melee status draws eight random bits; cone search seeds
its best angular error with 30000; boss audio uses ranges 20000/60000. The four
selectors retain their precise distance bands and repeated random draws.
In particular, odds stored as `u16` still narrow after each compound shift,
while the profiled selector retains signed word-sized weighted odds.

Freeze all 112 objects and 484 scores, force both actor compiles, independently
compile the frozen before/after source/header sets, and compare every runtime
section, symbol and ordered relocation. Resolve all reviewed function words
and references, recheck exact bodies against retail and targets, and validate
both initialized tables against retail. Run modern checking, inventory, Ruff,
existing tests, whitespace checks and full `kf build`. No new tests or size
assertions are proposed. The initial hypothesis is unchanged emitted runtime
and match state; partial functions remain separate matching work.

The [original actor audit](game-actor-constants.md) records the earlier
consumer analysis. This pass supplies a complete core literal ledger and
refreshes the existing behavior ledger after moving shared definitions.

| GAME VA / bytes | Function | Initial strict % |
| --- | --- | ---: |
| 0x8002ca78 / 60 | actor_pool_find_free | 100.000000 |
| 0x8002cab4 / 32 | func_8002cab4 | 100.000000 |
| 0x8002cad4 / 112 | actor_set_player_transform | 100.000000 |
| 0x8002cb44 / 116 | actor_update_cell_from_position | 100.000000 |
| 0x8002cbb8 / 156 | actor_set_position | 100.000000 |
| 0x8002cc54 / 16 | actor_set_rotation | 100.000000 |
| 0x8002cc64 / 196 | actor_initialize | 100.000000 |
| 0x8002cd28 / 164 | actor_initialize_current | 100.000000 |
| 0x8002cdcc / 188 | actor_initialize_slot | 100.000000 |
| 0x8002ce88 / 64 | actor_pool_clear | 100.000000 |
| 0x8002cec8 / 12 | actor_set_action | 100.000000 |
| 0x8002ced4 / 176 | actor_pool_spawn | 100.000000 |
| 0x8002cf84 / 244 | actor_pool_begin_death_by_definition | 100.000000 |
| 0x8002d078 / 168 | combat_calculate_damage_component | 100.000000 |
| 0x8002d120 / 904 | actor_apply_damage | 100.000000 |
| 0x8002d4a8 / 504 | actor_pool_apply_radial_damage | 90.087300 |
| 0x8002d6a0 / 344 | actor_try_attack_player | 100.000000 |
| 0x8002d7f8 / 388 | actor_pool_find_target_in_cone | 100.000000 |
| 0x8002d97c / 240 | actor_distance_to_point | 100.000000 |
| 0x8002da6c / 324 | actor_pool_find_overlap | 100.000000 |
| 0x8002dbb0 / 140 | actor_bind_current | 100.000000 |
| 0x8002dc3c / 52 | actor_advance_animation_wrapped | 100.000000 |
| 0x8002dc70 / 92 | actor_advance_animation_clamped | 100.000000 |
| 0x8002dccc / 48 | actor_animation_crossed_phase | 100.000000 |
| 0x8002dcfc / 152 | actor_play_sound_at_phase | 100.000000 |
| 0x8002dd94 / 288 | actor_try_select_action_distance_facing | 100.000000 |
| 0x8002deb4 / 356 | actor_try_select_ground_action | 100.000000 |
| 0x8002e018 / 216 | actor_try_select_facing_action | 100.000000 |
| 0x8002e0f0 / 504 | actor_try_select_profiled_action | 96.333336 |
| 0x8002e2e8 / 960 | actor_select_next_action | 100.000000 |
| 0x8002e6a8 / 684 | actor_update_awareness | 97.736840 |
| 0x8002e954 / 940 | actor_move_xz_with_collision | 100.000000 |
| 0x8002ed00 / 212 | actor_move_along_heading | 100.000000 |
| 0x8002edd4 / 1108 | actor_spawn_action_effect | 100.000000 |
| 0x8002f228 / 244 | actor_prepare_charge_toward_player | 100.000000 |
| 0x8002f31c / 332 | actor_apply_horizontal_movement | 100.000000 |
| 0x8002f468 / 240 | actor_update_effect_action | 100.000000 |
| 0x8002f558 / 884 | actor_apply_random_movement | 100.000000 |
| 0x8002f8cc / 444 | actor_update_boss_death_sequence | 100.000000 |
| 0x8002fa88 / 3472 | actor_update_current_action | 100.000000 |

## Named roles and boundaries

These names describe observed behavior. The original tuning rationale for
values such as 1548, 4001 and 7000 remains unknown.

| Family | Meaning and preserved behavior |
| --- | --- |
| Damage formula | Base power contributes one fifth to each nonzero attack channel. This is a weight; callers already supply damage subunits. The separate division by twice defense stays explicit arithmetic. |
| Dying damage cutoff | Phase 1548 prevents later damage during the dying action. This is an animation coordinate, not an elapsed frame count. |
| Status chance | Shifting the 15-bit RNG value by 7 yields eight bits, compared strictly below the stored chance. |
| Cone ranking | Initial signed-halfword error 30000 exceeds every folded angular error, whose range is 0..2048. It is an initial ranking sentinel, not a world distance. |
| Boss audio | Maximum range 20000 and attenuation distance 60000 feed the spatial audio API. |
| Selection draw | RNG shift 4 yields 0..2047. Separate facing-bypass draws retain strict limits 1638 or 819. |
| Distance selector | Beyond four times the supplied distance scale, homebound actors reject and others divide odds by 16. Then near, middle and outer bands multiply by four, multiply by two or divide by four. The middle endpoint remains scale plus half scale. Each `u16` compound shift still narrows. |
| Ground selector | After the existing floor-height and player-target checks, distances above 7000 halve odds; distances below 4001 reject; the remaining band multiplies odds by eight. |
| Multi-hit selector | The sole source caller requests multi-hit. Below 8000 rejects, 8000..11000 multiplies odds by four, and above 11000 divides odds by 16. The upper contact bound is therefore not a hard selection cutoff. Shared bounds also serve the behavior module's attack contacts. |
| Profile selector | Authored signed-halfword weights are Q8. Weighted odds remain signed word-sized arithmetic. Spawner selection counts all occupied active actors plus live actor-spawner effects, including the selecting actor, and accepts fewer than two. This does not change pool capacity. |
| Shared yaw | RNG shift 3 supplies twelve angle bits in initialization and wandering. |
| Boss sound count | Four records determine both the definition and shared declaration. The behavior reads indices 1..3, including the final all-88 record. |

The [core ledger](game-actor-core-literal-ledger.md) accounts for all **174**
remaining numeric occurrences: 89 initializer tokens and 85 function tokens.
This pass removes **29** inline numeric uses from `actor.c` (203 to 174) and
one additional header array-bound use. Initializer tokens comprise 12 whole-row
zero initializers, 65 explicit profile fields and 12 sound bytes. Retained
function literals express formula arithmetic, boundaries, nulls, initialization,
component indices and authored floor/definition bindings; each has a reason.

The [behavior ledger](game-actor-behavior-literal-ledger.md) still contains
173 occurrences and has been refreshed for the shared names and line numbers.
Both ledgers were checked against a token census excluding comments, strings,
identifier digits, enum definitions and retail claims. No `unknown_` field
acquired new consumer evidence during this review.

## Verification

All 40 functions in the table retain their listed strict scores: **37 exact
and three partial**. Both actor units were forcibly compiled. Independent
frozen before/after compilation of all 112 units preserves allocated bytes,
alignment, runtime symbols and ordered relocations. Live objects agree with
those controls; only the two actor objects' debug-line sections differ.
All 484 function scores are unchanged.

The 25 action-profile rows at GAME `0x80056080` occupy 250 bytes; the four
boss-sound rows at GAME `0x8005617c` occupy 12 bytes. Independently expanded
source initializers, compiled symbol ranges, target symbol ranges and retail
agree on all 262 bytes. The two bytes between the claims are not attributed
to either table.

The three existing partials retain these first raw divergences:

| Function | GAME instruction VA | Source | Retail |
| --- | --- | --- | --- |
| actor_pool_apply_radial_damage | 0x8002d4d4 | `move s4,a0` | `andi s6,a2,0xffff` |
| actor_try_select_profiled_action | 0x8002e108 | `andi s3,a2,0x1f` | `andi s2,a2,0x1f` |
| actor_update_awareness | 0x8002e6ac | `sw ra,56(sp)` | `sw ra,60(sp)` |

These are unchanged comparison observations; this naming pass makes no claim
about their compiler cause and banks no additional exact function.

Inventory, Ruff and whitespace checks pass. All **680 existing tests pass**
in 85.188 seconds. Modern checking retains the same **300 error diagnostics**
and **65/112** passing source/image variants. Full `kf build` ran and still
fails existing data/relink checks: source-data matches are PSX 0/1, GAME 9/42
and OPEN 2/19; target relinks are PSX 1/1, GAME 75/77 and OPEN 34/38, with six
conflicting section bases and zero artifact failures. No tooling or flake
files changed.
