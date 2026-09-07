# GAME actor drift and home-position limits

## Function Match Plan

Name four motion parameters in `actor_update_current_action`, GAME
`0x8002fa88/0xd90`, in `game.actor_behavior`. Baseline `3b9aed4` has this
function banked at strict 100%. The unit has ten exact functions and one
partial, `actor_update_awareness`. This is a naming change to the dispatcher;
no helper body, field storage, source dependency, or relocation changes.

Hash-identical retail files and all six semantic views were refreshed for
the dispatcher, its random-motion helper at `0x8002f558/0x374`, and its sole
direct caller, `actor_pool_update` at `0x80030818/0xa8`. The caller invokes
the dispatcher at `0x8003087c` with no arguments and ignores the result.
The neighboring boss-death routine ends at `0x8002fa88`; the pool updater
starts immediately after the dispatcher. Source history includes the
home-distance dependency correction `735e0b0`, actor type propagation, and
the earlier constant/state reviews. The complete control/referent snapshot
is recorded in [the home-distance review](game-actor-home-distances.md):
868 words, 69 direct calls, 86 conditional branches, 43 direct jumps,
two indirect jumps, one return and 14 ordered HI16/LO16 pairs. All 201
transfers retain their delay slots, including the five early exits.

This is actor-specific game policy; neither the dispatcher nor its motion
helper has vendored attribution. The called `rand` body is separately
attributed to Psy-Q `LIBAPI.LIB` member `C47`. Both inspected game bodies
have no string references. The two dispatch targets remain unresolved in
the navigator; naming does not promote candidate switch edges.

The source hypothesis is limited to four local integer constants. Drift
movement components are signed halfwords added to signed word positions;
yaw drift is a signed halfword added to the 4096-unit wrapped angle.
Home differences remain signed words calculated before the bounds tests.
Force a unit compile, compare all object sections and all strict scores,
independently resolve the dispatcher to complete retail instruction words
and ordered referents, then run the full build, lint and existing tests.

## Names and measured meaning

| Constant | Value | Evidence and units |
| --- | ---: | --- |
| `ACTOR_DRIFT_AXIS_ACCELERATION` | 1 | The call at `0x80030308` supplies step 1. The helper adds or subtracts this from each movement component before applying movement: world units per update squared, with independently selected signs. |
| `ACTOR_DRIFT_AXIS_SPEED_LIMIT` | 100 | The same call supplies 100 in its delay slot. The helper clamps each X/Y/Z component separately to ±100 world units per update. This is not a bound of 100 on the vector's magnitude. |
| `ACTOR_DRIFT_YAW_SPEED_LIMIT` | 32 | `0x8003033c..0x80030374` compares and clamps the signed yaw increment to ±32. Adding it to rotation at `0x80030384` gives a maximum angular speed of 2.8125 degrees per update. The four bounds/assignments share this parameter. |
| `ACTOR_HOME_AXIS_TOLERANCE` | 200 | `0x80030460..0x80030478` adds 199 and unsigned-compares against 399 on each signed coordinate difference. Each accepted integer difference is -199..199. This defines an axis-aligned square, with an exclusive tolerance of one tenth of a 2000-unit map tile. |

The names identify tunable physical quantities and make repeated uses agree.
Retail establishes their values and effects, but not why the designers chose
them. No conversion to meters, seconds, or a presumed update frequency is
supported by this evidence. Collision handling can reject a displacement
and reflect movement components; the bounds do not promise uninterrupted
movement at their maximum rates.

The unchanged yaw `++`/`--` applies one angular unit per update to the yaw
increment. Zero resets initialize motion and animation. The RNG expression
`(RAND_MAX + 1) / 2` splits its possible result values in half; it is not a
measured probability over time. Return-home's raw cutoff 2048 and authored
animation sample `0x4b0` retain their explanations in the
[actor literal ledger](game-actor-constants.md). Preserve the original
read-before-assignment on the home initialization path and its differing
move/melee animation accesses.

## Progress storage review

The search of all source/header uses still finds three roles in the same
`action_progress` byte: named state markers, jump phases, and arithmetic
counters. Pursuit assigns `(rand() >> 11) + 13`, then decrements during
backoff; post-death increments from zero to seven. Drift writes 2 on a
collision, but its consumer only distinguishes zero from nonzero. These
are documented in [the state review](game-actor-states.md).

A state enum alone does not describe that arithmetic. Keep `u8` for this
pass; a future stronger model must represent both counters and the
action-dependent state domains without casting every counter operation or
inventing names for counter values. This is outstanding type-modeling work,
not a claim that actor enum propagation is complete.

## Verification

Four constants replace ten inline numeric occurrences. The forced compile
preserves every section of all 112 source objects except `.debug_line` in
the actor behavior object. All 484 strict function scores remain unchanged.
The ten exact functions in the rebuilt unit independently reproduce 2209
retail instruction words, including delay slots, with matching ordered calls
and HI16/LO16 referents.

| GAME VA | Function | Final verdict |
| --- | --- | --- |
| `8002e2e8` | `actor_select_next_action` | Unchanged, 100% |
| `8002e6a8` | `actor_update_awareness` | Unchanged, 97.736840% |
| `8002e954` | `actor_move_xz_with_collision` | Unchanged, 100% |
| `8002ed00` | `actor_move_along_heading` | Unchanged, 100% |
| `8002edd4` | `actor_spawn_action_effect` | Unchanged, 100% |
| `8002f228` | `actor_prepare_charge_toward_player` | Unchanged, 100% |
| `8002f31c` | `actor_apply_horizontal_movement` | Unchanged, 100% |
| `8002f468` | `actor_update_effect_action` | Unchanged, 100% |
| `8002f558` | `actor_apply_random_movement` | Unchanged, 100% |
| `8002f8cc` | `actor_update_boss_death_sequence` | Unchanged, 100% |
| `8002fa88` | `actor_update_current_action` | Unchanged, 100% |

The dispatcher retains all 868 words, 69 calls and 14 ordered address pairs;
its linked-byte SHA-256 remains
`ac036226c3167ecb900baf7ad50a46a602ec2b6c58fa4f82e984167bc94cdd12`.
No new match is claimed, and the existing bank entry is unchanged.

Modern Clang checking passes for `game.actor_behavior`. All 665 repository
tests pass (82.318 seconds), as do inventory validation, Ruff and
`git diff --check`. Full `kf build` remains non-green on existing data
ownership/placement checks: PSX data 0/1, GAME 9/42, OPEN 2/19; target relink
1/1, 75/77, 34/38 respectively, with six conflicting section bases and zero
artifact failures. The byte-identical code/data comparison establishes that
this change introduces no new residue.
