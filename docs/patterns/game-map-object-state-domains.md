# GAME map-object behavior and action domains

## Function Match Plan

Type definition +0 as `KfMapObjectBehavior` and runtime object +40 as
`KfMapObjectAction`, retaining their unsigned-byte representation. Propagate
the latter through `map_object_start_action_if_idle` and the former through
the saved-floor policy local. These fields are different domains even when
their encoded values coincide. The typed behavior-to-action adapter represents
the three direct mappings already performed by retail; it adds no runtime
validation or action selection. Modern compilation checks its parameter and
return domains; legacy C keeps the encoded cast at the original call site.

The setter at GAME `80031784` loads +40 with `lbu`, compares 255, stores `$a1`
with `sb`, clears +42 with `sh`, and returns with its original delay slot.
The interaction calls at `800354e8` and `8003550c` load definition +0 directly
into `$a1`; the linked-trigger call at `80031c08` does the same after `<8`.
The 0/1 door behaviors map to action 0/1. Action 1 has no independent update
body: the action-0 owner rotates its paired object. No new action-1 case or
state reset is justified. Range comparisons should name their exclusive or
inclusive bounds, preserving the existing operators and unsigned loads.

Hash-identical retail, all six semantic views, current source/history and
the established neighboring-function dossiers were checked before edits for
the 15 consumers below. The prior map-object and object-model campaigns
establish game ownership, table/record layouts and external SDK callees.
Direct calls remain proven; address materializations retain their existing
evidence tier. The indirect tables do not gain proven edges through naming.
The first source hypothesis is identical runtime bytes and ordered referents,
with stronger modern domain checking and unchanged unsupported-value behavior.

Require forced affected builds, isolated compilation of all 112 variants,
preservation of 484 strict scores, linked retail-word controls for every exact
consumer, focused compiler rejection controls, whole-tree Clang diagnostics,
inventory, Ruff, repository tests, whitespace checks and full `kf build`.
No size assertions are introduced.

## Retained behavior 3 and resource evidence

The 141 eight-byte definitions in `KF/COM/COM.DAT` chunk 5 use behavior values
0, 1, 2, 8, 9, 10, 11, 12, 13, 14, 64, 65, 83 and 255. None uses 3.
The rendering and collision-marker cases for encoded value 3 remain explicit
unresolved cases. Sharing the lift-door depth bias and two-cell collision
shape does not establish a complete behavior identity or an unused/reserved
meaning. Typing the switch must not invent one; preserve the encoded literal
with an explicit enum decode and retain its ledger reason.

Behavior 12 belongs to model IDs 56, 63, 64, 68 and 69, which the loader starts
in the map-piece reveal action. Name that definition role independently of
the action domain. Behavior 1 belongs to model IDs 120 and 122, the mirrored
partner leaves. The existing 84-word interaction table and 99-word action
table are documented in [the initial map-object audit](game-map-object-constants.md).

## Consumer baseline

| GAME VA / bytes | Function | Strict baseline |
| --- | --- | ---: |
| `0x8001ebb8 / 384` | `render_map_object` | 100.000000% |
| `0x80030c7c / 572` | `map_object_mark_collision_edge` | 100.000000% |
| `0x80030eb8 / 196` | `map_object_probe_forward` | 93.755104% |
| `0x80030f7c / 96` | `map_object_pool_clear` | 100.000000% |
| `0x80031008 / 1096` | `map_object_pool_load` | 100.000000% |
| `0x800315c4 / 448` | `map_object_pool_find_interaction_from` | 100.000000% |
| `0x80031784 / 32` | `map_object_start_action_if_idle` | 100.000000% |
| `0x80031834 / 404` | `map_object_spawn_effect` | 94.455444% |
| `0x800319c8 / 396` | `map_object_spawn_actor_debris` | 100.000000% |
| `0x80031b54 / 240` | `map_object_pool_trigger_link` | 100.000000% |
| `0x80031c44 / 132` | `map_object_pool_clear_link` | 100.000000% |
| `0x80031cc8 / 3096` | `map_object_pool_update` | 98.966410% |
| `0x800346a8 / 908` | `map_floor5_transition_cutscene` | 100.000000% |
| `0x80034de4 / 2308` | `map_interaction_dispatch` | 83.436745% |
| `0x80035b5c / 696` | `map_world_state_persist` | 94.821840% |

## Final implementation and verification

Both selectors now retain their domains through fields, locals and the action
setter. Three behavior-to-action sites use the typed adapter; the actual
retail integer representation is unchanged. The door-family, linked-trigger
and link-clearing predicates name their range bounds without changing their
operators. Behavior 12 and partner action 1 have separate domain members;
neither introduces new control flow. The two raw behavior-3 cases remain
explicit encoded values with documented unresolved identity.

All 112 isolated pre/post object files are identical, including debug sections.
Live objects agree with the isolated compilation and all 484 strict scores are
unchanged from this campaign's captured baseline. Every one of the 15 consumer
rows above has an unchanged final verdict: ten exact and five partial. The
controls preserve 2,749 instruction words, 144 ordered calls and 92 address
materializations. The exact consumers agree with 1,076 complete linked retail
words, including every delay slot. No new exact result or banking is claimed.

| Partial function / GAME site | Unchanged candidate / retail first divergence |
| --- | --- |
| `map_object_probe_forward` / `80030ec4` | `lw a3,8(a0)` / `lw t0,8(a0)` |
| `map_object_spawn_effect` / `80031838` | `sw ra,32(sp)` / `sw ra,36(sp)` |
| `map_object_pool_update` / `80031dcc` | `lui a0,0x8005` / `lbu v1,0(s3)` |
| `map_interaction_dispatch` / `80034de4` | `addiu sp,sp,-80` / `addiu sp,sp,-72` |
| `map_world_state_persist` / `80035b6c` | `lui v1,0x800a` / `addiu a1,a0,-543` |

These remain unattributed existing instruction differences. The source does
not add register carriers, control-flow changes or new runtime adapter calls.

The real-header modern positive control accepts behavior field/local propagation,
the explicit mapping and action field-to-setter propagation. Ten negative
controls reject raw/foreign behavior and action assignments, raw/foreign setter
arguments, raw/foreign adapter inputs, and raw/foreign action comparisons.
Whole-tree diagnostics remain exactly the same 300 errors, with 65/112 variants
passing. Inventory, Ruff, whitespace checks and all 683 tests pass (87.215 s).
The full build retains source-data matches PSX 0/1, GAME 9/42, OPEN 2/19 and
target relink PSX 1/1, GAME 75/77, OPEN 34/38, with zero artifact failures.

The new [map-object ledger](game-map-object-literal-ledger.md) refreshes all
145 retained occurrences in the two modules from their historical reasons,
removing entries superseded by floor/model naming. Together with the refreshed
renderer case and the concurrent floor-item renderer's explicit byte-test
expression, current complete ledgers now cover 28 source files / 2,484
occurrences. The census remains 6,792 occurrences in 111 C files; source
unknowns remain ten matching lines / 14 identifier tokens. This strengthens
field/usage typing and coverage without claiming those remaining raw constants
or unresolved fields are finished.
