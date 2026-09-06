# GAME actor state values

## Function Match Plan

Name the actor's action, lifecycle, vertical, collision and slot-policy values
across all seven source consumers, including rendering and saved floor state.
Rename only `KfActor.action_timer` to `action_progress`: this byte holds
initial/running/locked/completed markers, jump phases and two kinds of counter.
The unrelated map-object timer retains its name. Preserve byte storage,
signatures, all constants, CFG, calls, ordered relocation targets and claims.
No new function body, enum-sized ABI field or table ownership is introduced.

Baseline: master `1db3e5b`. All seven units use `probe-gcc257-o2-g0`, which
remains a compiler probe. Hash-identical retail files were verified with
`kf init`; each function has refreshed address, disassembly/CFG, caller,
callee, string and strict-match views, plus source history. Earlier adjacent
function dossiers and the actor animation-slot review constrain the call
contracts. Every changed state field is loaded/stored as a byte (`lbu/sb`);
existing halfword motion and phase fields keep their widths.

| GAME VA / size | Function | Initial strict % | Evidence and proposed change |
| --- | --- | ---: | --- |
| `8001f218 / 580` | `render_entities` | 91.25 | Lifecycle 1 alone passes the actor render filter. |
| `8002cc64 / c4` | `actor_initialize` | 100 | Stores active lifecycle, clear collision, no vertical motion, action `ff` and progress `ff`; placed policies use authored heading. |
| `8002cdcc / bc` | `actor_initialize_slot` | 100 | Explicit active store precedes position restoration and initialization; preserve both active stores. |
| `8002ce88 / 40` | `actor_pool_clear` | 100 | Free slot and dormant lifecycle stores accompany the null animation cache. |
| `8002cec8 / c` | `actor_set_action` | 100 | `sb` at +8 installs the action; the return delay slot clears +`38`. |
| `8002ced4 / b0` | `actor_pool_spawn` | 100 | Writes dynamic slot policy 0 and initializes pursuit action 2. |
| `8002cf84 / f4` | `actor_pool_begin_death_by_definition` | 100 | Active actors with a death animation enter action 6; others become disabled. |
| `8002d120 / 388` | `actor_apply_damage` | 89.986725 | Dying/post-death guards, drift-to-fall transition, hit reaction and death selection. |
| `8002d4a8 / 1f8` | `actor_pool_apply_radial_damage` | 86.103170 | Selects active actors except post-death and the current actor. |
| `8002d7f8 / 184` | `actor_pool_find_target_in_cone` | 100 | Same active/post-death exclusion; leave its angle math intact. |
| `8002da6c / 144` | `actor_pool_find_overlap` | 100 | Active actors block overlap except dying/post-death and current actor. |
| `8002dd94 / 120` | `actor_try_select_action_distance_facing` | 100 | Preserves an unfinished matching action; homebound policy rejects far targets; `ff` returns no action. |
| `8002deb4 / 164` | `actor_try_select_ground_action` | 100 | Shared unfinished-action gate and no-action rejection; existing ground and facing tests unchanged. |
| `8002e018 / d8` | `actor_try_select_facing_action` | 100 | Same gate/rejection protocol around its distinct distance and facing policy. |
| `8002e0f0 / 1f8` | `actor_try_select_profiled_action` | 96.333336 | Same gate/rejection; profile 9 counts active actors and kind-9 effects. |
| `8002e2e8 / 3c0` | `actor_select_next_action` | 100 | Action choices and old-action tests map to decoded dispatcher arms; progress `f0` prevents automatic selection. |
| `8002e6a8 / 2ac` | `actor_update_awareness` | 97.736840 | Three lifecycle arms and spawn policies; state 2 waits for range exit, without movement. |
| `8002e954 / 3ac` | `actor_move_xz_with_collision` | 100 | Axis sliding, blocked turning, ordinary fall, long drop and step-up transitions. |
| `8002ed00 / d4` | `actor_move_along_heading` | 100 | Sliding state selects the existing 3/2 turn-rate expression. |
| `8002f468 / f0` | `actor_update_effect_action` | 100 | Progress starts at 0, locks at `f0`, completes at `ff`; effect indices remain separate. |
| `8002f558 / 374` | `actor_apply_random_movement` | 100 | Renames the progress field; preserves its unexplained nonzero collision marker 2. |
| `8002f8cc / 1bc` | `actor_update_boss_death_sequence` | 100 | Ends in disabled lifecycle with cleared progress; sound/effect cadence unchanged. |
| `8002fa88 / d90` | `actor_update_current_action` | 98.938940 | Complete action/vertical switches and nested progress phases; preserve both indirect jumps and all delay slots. |
| `80030818 / a8` | `actor_pool_update` | 100 | Occupied slots run awareness; active lifecycle alone runs the action dispatcher. |
| `800308c0 / 1ac` | `actor_pool_load_placements` | 100 | Both initialized and free-tail slots receive dormant lifecycle; preserve the stream's slot policy. |
| `80033f64 / 288` | `map_ambient_script_floor1` | 100 | Script reactivates a disabled actor by restoring dormant state and initializing its slot. |
| `80035b5c / 2b8` | `map_world_state_persist` | 94.821840 | Persistent/homebound slots save disabled as 3 and every other lifecycle as dormant 0. |
| `80035e44 / 69c` | `map_restore_floor_state` | 99.964540 | Restores serialized lifecycle bytes and disables the floor-1 scripted actor until its trigger. |

The existing actor-core/AI/action vendor-negative dossiers and the concrete
map/save/render callers establish game-owned behavior. SDK and libc bodies
remain calls, not matching progress. Candidate table edges or function-identity
confidence are not promoted by naming. The shared `KfActor` byte offsets
remain +0 slot policy, +6 lifecycle, +8 action, +`0b` vertical, +`38` progress
and +`39` collision. Record these contracts in the existing field inventory.

Rebuild all seven units, compare all 112 objects and the full strict report
against the master baseline, independently resolve exact functions to retail
words and ordered addresses, and retain a per-function verdict. Run full build,
lint, existing tests and whitespace checks. No banking is intended.

## Action IDs

These are the values of `actor->action`, not animation-table indices or the
resource animation IDs stored in those slots.

| Value | `KF_ACTOR_ACTION_` suffix | Decoded behavior |
| ---: | --- | --- |
| 0 | `IDLE` | Advances idle animation without horizontal movement. |
| 1 | `WANDER` | Chooses random headings and moves forward. |
| 2 | `PURSUE` | Faces/approaches the player, with backward retries when blocked. |
| 3 | `RETREAT` | Faces the player and moves backward. |
| 4 | `MELEE_ATTACK` | Attacks within the definition's awareness distance. |
| 5 | `HIT_REACTION` | Selected after surviving damage when its animation exists. |
| 6 | `DYING` | Death animation, sound and drops; then post-death handling. |
| 16 | `JUMP_ATTACK` | Upward velocity, charge, attack phase and landing wait. |
| 17 | `SPECIAL_ATTACK` | Uses the special attack range and phase 3000. No specific attack identity is inferred. |
| 18 | `DRIFT` | Random three-axis movement and yaw drift. |
| 19/20/21 | `EFFECT0` / `EFFECT1` / `EFFECT2` | Runs configured effect index 0/1/2; these do not identify specific spells. |
| 22 | `MULTI_HIT_ATTACK` | Six attack-phase tests, including two yaw-offset hits. |
| 32 | `EXIT_BLOCKED_PLACEMENT` | Moves forward while the placement collides with the world; reselects when clear. |
| 33 | `RETURN_HOME` | Moves toward authored placement coordinates and aligns its heading. |
| 127 | `POST_DEATH` | Delayed lifecycle transition, or the definition-5-to-6 transformation on floor 4. |
| 255 | `NONE` | No selected action / selection rejected. |

## Lifecycle and allocation policy

Lifecycle 0 (`DORMANT`) is eligible for activation within the 28000-unit
distance limit. Lifecycle 1 (`ACTIVE`) participates in updates, rendering and
target selection. Lifecycle 2 (`WAIT_FOR_RANGE_EXIT`) suppresses the actor
until the player leaves the 32000-unit range, then returns to dormant. It
does not retreat or move while suppressed. Lifecycle 3 (`DISABLED`) is skipped
by awareness, actions and rendering. It is also used for a scripted actor
before its reveal, so calling it permanently dead would be incorrect.

Slot policy and lifecycle are independent bytes. Slot 0 (`DYNAMIC`) is used
by the runtime spawner. Slot 1 (`PERSISTENT`) retains disabled state in the
floor save data; slot 3 (`HOMEBOUND`) adds return-home action selection and a
far-target restriction. Slot 2 (`RESPAWNING`) repeatedly tries its spawn roll
while dormant and returns to dormant after death. `ff` remains `FREE` and
terminates the placement stream. These names describe the implemented policy,
not an actor's species or an original source spelling.

## Vertical and collision states

| Vertical value | `KF_ACTOR_VERTICAL_` suffix | Behavior |
| ---: | --- | --- |
| 0 | `NONE` | No update in the vertical switch; drift may still move Y separately. |
| 1 | `STEP_UP` | Upward velocity toward a higher floor, then clamp to that floor. |
| 2 | `FALL` | Gravity until reaching floor height; also used after damaging a drifting actor. |
| 3 | `LONG_DROP` | A drop greater than 600 units; same gravity arm, but the horizontal mover blocks the floor-result code in this state. |
| 4 | `JUMP_ATTACK` | Vertical world queries, landing, ceiling response and actor/player bounce behavior started by the jump attack. |

Collision states are the horizontal mover's steering history: 0 (`CLEAR`),
1 (`SLIDING`, one axis succeeded) and 2 (`BLOCKED`, both axis retries failed
and heading was reversed). They are separate from collision-query return
codes. A call with `stop_on_collision` can report blockage without writing
this steering byte. Sliding also selects a turn rate of `(rate * 3) >> 1`;
the existing repeated-add expression remains intact.

## Progress is not solely elapsed time

`KF_ACTOR_PROGRESS_INIT` (0) starts an action; `RUNNING` (1) marks the ordinary
running phase; `LOCKED` (`f0`) prevents automatic action selection; `COMPLETE`
(`ff`) permits re-selection even if the chosen action is unchanged. Locking
does not prevent damage from choosing hit/death actions. An action code of
`ff`, an unavailable animation of `ff` and progress `ff` have distinct names.

The jump action uses private phase names: rising (1), attack pending (2) and
waiting for landing (3). These are discrete phases, not frame durations.
After reaching nonnegative vertical velocity, it recomputes the charge and
waits for phase crossing 3400 before attacking; it then waits to leave the
jump vertical state. The return-home arm's existing uninitialized coordinate
read and mixed animation-slot accesses remain unchanged.

| Remaining progress literal | Meaning and reason to retain it |
| --- | --- |
| Drift collision marker 2 | The collision path writes a nonzero progress value. No consumer distinguishes 2 from another nonzero value in this action; its exact original rationale is unresolved. |
| Pursuit shift 11, bias 13, cutoff 11 | `rand() >> 11` gives 0..15, producing progress values 13..28. If pursuit stays selected and reverse moves remain clear, the move-before-test order gives 4..19 backward updates, then restores running state. The authored backoff tuning is unresolved. |
| Post-death bound 7 | Seven updates increment progress 0 through 7; the next post-death update changes lifecycle. The special transformation bypasses this wait. The duration's design rationale is unresolved. |
| Increment/decrement 1 | Advances or consumes one progress update; ordinary counting arithmetic. |

Distances, velocities, random gates, attack phases and authored definition/
effect IDs outside these state domains retain their existing values. This
state-machine review does not claim that the remaining actor tuning and
effect-parameter literal audit is complete.

## Final function verdicts

The seven affected units were forcibly rebuilt. All 112 source objects retain
identical sections except `.debug_line` in the actor and behavior objects.
The complete 484-function strict report is unchanged. Expanding the declared
state constants and normalizing the single field rename also reproduces every
original statement, value, type and ordering in the eight source/header files.
All offsets, widths and field-confidence levels remain unchanged; no function
or data identities, relocations, claims or bank entries changed.

Each function below retains its initial strict score. The 20 exact functions
also independently reproduce 1844 retail instruction words, including delay
slots, with matching ordered call and data addresses. The eight non-exact
functions retain their existing residues without attribution to a compiler
mechanism. No new exact result is claimed or banked.

| GAME VA | Final verdict | Retail words / calls / data addresses independently checked |
| --- | --- | ---: |
| `8001f218` | Unchanged, 91.25% | — |
| `8002cc64` | Remains exact | 49 / 2 / 1 |
| `8002cdcc` | Remains exact | 47 / 4 / 1 |
| `8002ce88` | Remains exact | 16 / 0 / 1 |
| `8002cec8` | Remains exact | 3 / 0 / 0 |
| `8002ced4` | Remains exact | 44 / 4 / 1 |
| `8002cf84` | Remains exact | 61 / 1 / 1 |
| `8002d120` | Unchanged, 89.986725% | — |
| `8002d4a8` | Unchanged, 86.103170% | — |
| `8002d7f8` | Remains exact | 97 / 2 / 2 |
| `8002da6c` | Remains exact | 81 / 1 / 1 |
| `8002dd94` | Remains exact | 72 / 4 / 3 |
| `8002deb4` | Remains exact | 89 / 4 / 5 |
| `8002e018` | Remains exact | 54 / 3 / 3 |
| `8002e0f0` | Unchanged, 96.333336% | — |
| `8002e2e8` | Remains exact | 240 / 11 / 2 |
| `8002e6a8` | Unchanged, 97.736840% | — |
| `8002e954` | Remains exact | 235 / 6 / 2 |
| `8002ed00` | Remains exact | 53 / 5 / 2 |
| `8002f468` | Remains exact | 60 / 5 / 4 |
| `8002f558` | Remains exact | 221 / 7 / 2 |
| `8002f8cc` | Remains exact | 111 / 13 / 7 |
| `8002fa88` | Unchanged, 98.938940% | — |
| `80030818` | Remains exact | 42 / 4 / 1 |
| `800308c0` | Remains exact | 107 / 1 / 1 |
| `80033f64` | Remains exact | 162 / 5 / 19 |
| `80035b5c` | Unchanged, 94.821840% | — |
| `80035e44` | Unchanged, 99.964540% | — |

`ruff check scripts tests` and `git diff --check` pass. Full test discovery
ran 654 tests in 88.472 seconds: 653 passed, and the pre-existing untracked
save/load-hub control failed its instruction comparison. Its source and object
are unchanged from `1db3e5b`; that unrelated local work is preserved separately.

Full `kf build` retains the baseline failures: source data matches 7/60
contributions (PSX 0/1, GAME 5/40, OPEN 2/19), config-provider data matches 4/4,
and target relink verifies PSX 1/1, GAME 75/77 and OPEN 34/38. The two GAME and
four OPEN conflicting section bases remain; there are zero artifact failures.
The full build is therefore not passing, and this naming change does not claim
to repair those ownership/placement discrepancies.
