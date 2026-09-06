# GAME actor animation-table slots

## Function Match Plan

This naming campaign changes the indices into `KfActorDefinition`'s three
parallel animation tables. It preserves the selected entries, resource IDs,
action codes, signatures, object layouts, control flow and ordered referents.
The source baseline is `7065b39`; all three units select the pinned
`probe-gcc257-o2-g0` profile in `config/units.toml`.
`kf init` verified the retail inputs before refreshing all six semantic views
and source history for these eight GAME functions and their adjacent helpers.

| GAME VA / bytes | Function | Initial strict match | Specific evidence and first change |
| --- | --- | ---: | --- |
| `8002cf84 / f4` | `actor_pool_begin_death_by_definition` | 100% | Eight direct calls from boss death and floor restoration. Tests the definition byte at +`0x12` for `0xff` before selecting action 6. Name death slot 4 and its unavailable marker. |
| `8002d120 / 388` | `actor_apply_damage` | 89.986725% | Six direct combat/projectile callers; ten direct outgoing calls. Surviving and fatal damage test slots 3 and 4 before actions 5 and 6. Name the hit/death slots without altering the nine-argument halfword ABI or damage arithmetic. |
| `8002e2e8 / 3c0` | `actor_select_next_action` | 100% | Four direct callers and eleven outgoing calls. Availability checks use definition bytes +`0x0e`..+`0x19`; the helper arguments are a separate action-code domain. Replace literal arithmetic with the actual slots and use a distinct unavailable marker. |
| `8002e6a8 / 2ac` | `actor_update_awareness` | 97.736840% | Called by the pool updater; eleven outgoing calls. Slot 1 availability chooses action 32 after initialization. Name movement slot 1 without changing distance, spawning or lifecycle policy. |
| `8002f468 / f0` | `actor_update_effect_action` | 100% | Dispatcher calls at `8003039c/ac/bc` pass 0/1/2. Adds 8 to select the animation table entry, loads its byte ID and halfword step/trigger, then calls the effect spawner. Name slot 8 as the first configured effect slot; retain the signed word parameter and five outgoing calls. |
| `8002f8cc / 1bc` | `actor_update_boss_death_sequence` | 100% | One direct caller in action 6 and thirteen outgoing calls. Debris and loop sound periods use death-slot step multiplied by 2 and 4. Name slot 4 without changing phase tests or effect/sound targets. |
| `8002fa88 / d90` | `actor_update_current_action` | 98.938940% | Pool updater calls it; 71 direct outgoing calls. Dispatches by action, loads named table slots, then dispatches vertical state. Keep both indirect jumps (`8002fae8`, `80030690`) and all existing table ownership; candidate incoming table edges are not promoted. |
| `800342ec / f4` | `map_ambient_script_floor5` | 100% | Called by the map-event updater; five outgoing calls and two `TALK` image strings. Five validated actor-state addresses write definition 7's slots 2/8/9/10/11 to resource IDs 2/3/3/3/1. Name the destination slots only. The curated function identity remains candidate. |

The byte animation IDs at definition +`0x0e` and halfword step/trigger arrays
at +`0x3a` and +`0x5a` have sixteen entries each. These are existing shared-layout
facts; replacing the three array bounds with a shared count adds no new
ownership or padding claim. The functions' actor pool, floor, combat, map and
effect dependencies establish game behavior, supported by the existing
vendor-negative dossiers in `game_semantic_actor_core.tsv`,
`game_semantic_actor_ai.tsv` and `game_semantic_actor_actions.tsv`. SDK/libc
callees remain library calls. No library body is reconstructed here.

Each function retains its decoded return and following delay slot. Raw
instruction and ordered relocation comparison must show that the constant
substitutions change no machine behavior, including pre-existing differences
in the three non-exact functions. Rebuild the three affected units, compare
all 112 objects and the complete strict report, then run full build, repository
tests, lint and whitespace checks. No new banking is intended.

## Three distinct numeric domains

`KF_ACTOR_ACTION_INDEX(action) ((action) - 2)` was not a general mapping:
action 2 pursues the player with slot 1, but the macro yields slot 0. Several
call sites passed an artificial action number solely to select an index.
The dispatcher instead establishes this mapping:

| Slot | `KF_ACTOR_ANIM_SLOT_` suffix | Observed use |
| ---: | --- | --- |
| 0 | `IDLE` | Action 0 advances its animation without horizontal movement. |
| 1 | `MOVE` | Actions 1/2/3 wander, approach and retreat; 32 leaves a blocked placement; 33 returns toward the placement position. |
| 2 | `MELEE` | Action 4 attacks within awareness distance; action 33 also explicitly reads this slot near home. |
| 3 | `HIT_REACTION` | Action 5 plays after surviving damage when available. |
| 4 | `DEATH` | Action 6 plays after fatal damage or the definition-wide death request. |
| 5 | `JUMP_ATTACK` | Action 16 applies upward velocity, horizontal charge and a later attack. |
| 6 | `SPECIAL_ATTACK` | Action 17 uses `special_attack_range` and attacks at phase 3000. No specific attack identity is inferred. |
| 7 | `DRIFT` | Action 18 applies random movement on three axes and random yaw drift. No flying-creature identity is inferred. |
| 8/9/10 | `EFFECT0` / `EFFECT1` / `EFFECT2` | Actions 19/20/21 use configured effect indices 0/1/2. Those indices select definition parameters; they do not identify particular spells. |
| 11 | `MULTI_HIT_ATTACK` | Action 22 tests six distinct hit phases, including two yaw-offset attacks. |

Slots 12..15 are within the physical table extent, but this dispatcher does
not select them. The old claim that eight effect actions were supported was
not justified by the three decoded callers. Leave those four slots unnamed.

The byte stored in a slot is a resource animation ID. `ff` means that the
slot is unavailable, now spelled `KF_ACTOR_ANIMATION_NONE`. It does not mean
an action has finished or that an actor pool slot is free. Those other fields
have independent contracts even though they use the same byte value.

The return-home case's reads of slot 2 and advancement with slot 1's step
are preserved exactly, as is its existing uninitialized-home-position issue.
The names describe the usual slot role without imposing a new rule that
would rewrite those retail operations.

## Inline values retained in this change

| Sites | Values | Reason |
| --- | --- | --- |
| Floor-5 animation writes | 2/3/3/3/1 | Authored resource animation IDs, distinct from destination slots. The same resource ID can serve several action slots. |
| Effect helper calls | 0/1/2 | Ordinal configured-effect indices shared with the parameter/spawner interface. Specific spell identities are unresolved. |
| Initial animation assignment | 0 | Resource animation zero, not an index into the actor definition tables. |
| Definition selector | 7 | Authored actor-definition ID; no supported proper-name alias. |
| Boss emission periods | 2/4 | Multiples of the death animation's phase step, making debris/sound cadence explicit. |
| Action, progress, lifecycle and vertical-state codes | Existing values | Separate state-machine domains; this table-slot review does not claim to finish their naming or the actor tuning audit. |
| Retail claims | Addresses and extents | Literal ownership evidence consumed by the build. |

## Final verdicts

All eight edited functions retain their initial strict scores. Five remain
exact; damage (89.986725%), awareness (97.736840%) and the dispatcher
(98.938940%) retain identical compiled instructions and relocations, including
their existing non-exact differences. No score improvement or new exact claim
is attributed to naming.

Resolving the source and target objects' real relocation targets back to the
retail image verifies every instruction, including delay slots, in the five
edited exact functions:

| Function | Retail words | Direct calls | Ordered data addresses |
| --- | ---: | ---: | ---: |
| `actor_pool_begin_death_by_definition` | 61 | 1 | 1 |
| `actor_select_next_action` | 240 | 11 | 2 |
| `actor_update_effect_action` | 60 | 5 | 4 |
| `actor_update_boss_death_sequence` | 111 | 13 | 7 |
| `map_ambient_script_floor5` | 61 | 5 | 11 |

That is 533 retail words. The adjacent horizontal-movement helper supplies
another unchanged exact control: 83 words, one call and two data addresses.
All 112 objects preserve every section except `game.actor`'s debug line
table, which reflects the wrapped availability condition. All 484 function
scores and the complete strict report are unchanged. Normalizing only the
declared constant substitutions also proves identical statements, values,
types and order in the four source/header files. Curated inventories,
relocations and bank rows are untouched.

`ruff check scripts tests`, `git diff --check` and all 651 repository tests
pass (86.679 seconds, nine skips). The full `kf build` still fails its existing
data/relink gates: source data 7/60; independent SDK/config data 4/4;
target relink PSX 1/1, GAME 75/77 and OPEN 34/38, with two GAME and four OPEN
conflicting section bases and no artifact failures. The header dependency
rebuild compiled all affected GAME units; this was not a Ninja no-op check.
