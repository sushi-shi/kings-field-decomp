# GAME object-model identifiers

## Function Match Plan

Propagate a byte-sized `KfMapObjectId` through GAME's runtime object field,
actor death-drop fields, spawn parameter and object-ID locals. Keep the
renderer local's halfword representation with `KF_ENUM_STORAGE`. Decode the
shared GAME/OPEN placement byte at GAME's loader: the two programs use
different TMD tables, so the common wire record cannot own GAME's domain.
Decode the actor definition's heterogeneous parameter byte at its drop use.
Encode at saved bytes, definition/TMD indexing and the item-pickup menu API.
The chalice/seal interaction explicitly maps its item ID to the corresponding
map model; it does not make all item IDs and map model IDs one domain.

Before edits, hash-identical retail files, all six semantic views, adjacent
functions, current source and history were inspected for all 23 GAME consumers.
The captured baseline is `c50f18d`. `lbu`/`sb` at object +0 and actor +9,
the placement +6 copy, and the spawn call's byte argument establish the
storage chain. `render_map_object` retains its existing `u16` local and
halfword TMD interface. Calls remain proven; address references retain their
validated/candidate classification. Switch-table indirect edges are not
promoted to proven calls. These game policy/record functions are not vendored
SDK bodies; their Sony library calls remain external.

Keep numeric values, CFG, delay slots, load signedness and ordered referents.
Force affected compiles, compare all 112 source/image variants against an
isolated pre-edit build, preserve all 484 strict function scores and verify
each exact consumer against linked retail words. Run modern compiler misuse
controls, the whole-tree diagnostic comparison, inventory, Ruff, repository
tests, whitespace checks and full `kf build` before commit. No size assertions.

## Resource identity evidence

`KF/COM/COM.DAT` chunk 5 supplies eight-byte object definitions. The first
chunk of each `B1..B5/MIXB.DAT` is the entity TMD with 135 object records.
TMD vertex/primitive offsets are relative to its object-table base; COM and
floor TIMs supply texture pages and palettes. Static geometry views do not
execute the game. The diagnostic renderer does not model lighting, culling
or GPU wrapping of palette uploads extending beyond row 511; geometry and
consumer behavior are independent controls on the proposed identities.

| ID | Identity / distinguishing evidence |
| --- | --- |
| 10, 11, 39 | Dragon Sword, Moonlight Sword, gold coin: existing item identities, cutscene transformation and gold-drop/pickup consumers. |
| 56, 63, 64, 68, 69 | Dragon Chalice and water/earth/fire/wind seal stones: item-use equality and reveal action share these item/model indices. |
| 81 | Beveled wooden lid, 23 vertices/36 primitives; definition behavior 8 hinges the container. |
| 83 | Flat wooden lid, 16 vertices/14 primitives on floors 1/2; accepted by key use, definition behavior 255. Do not assign it behavior 8 merely from its appearance. |
| 85 | Stone container lid, 30 vertices/28 primitives on floors 3/4; behavior 9 is the item-container interaction. |
| 89 | Upright inscribed gravestone, 24 vertices/22 primitives, real mesh only on floor 1; behavior 10 copies a map region. Floor-1 placement 98 has Raito Family Key link 54 and region parameter 0. Key use requires facing and plays an additional sound. The inscription does not establish a personal name. |
| 92 | Broken stone cross, 44 vertices/72 primitives; floor-1 departure script replaces a nearby model with it. |
| 111, 123 | Dry and filled fountain meshes on floor 1; the latter adds water surface/streams. Restore-point action replaces its model with 123 once its link clears. |
| 115 | Boss-encounter projectile emitter: floor-5 geometry and a wind-cutter spawn gated by boss encounter state. |
| 117, 118 | Solid lifting gate and pointed-bar portcullis; both definition behavior 2. |
| 119, 120 | Mirrored wooden hinged-door leaves, height 2500; definition behaviors 0/1. The first leaf selects the wooden-door sound cue. |
| 121, 122 | Mirrored tall hinged-door leaves, height 5000; definition behaviors 0/1. |
| 124, 125 | Fire-ball and wind-cutter emitters, distinguished by their direct effect-constructor arguments. |
| 130, 131 | Wooden signboard and flat inscription panel; screen-image interaction selects image groups 0 and 1 respectively. |
| 135..139 | Effect-backed switch, orbiting projectile, projectile emitter, short swing and long swing: direct loader/action constructor arguments establish the roles. They are beyond the entity TMD's 135 records. |

The rendering cutoff 133 and drop-band ends 43/48/65 remain exclusive bounds,
not identities of whichever model happens to occupy a boundary index. Drop
suppression value 99 is contextual to actor drops, not the free-slot value 255.
Unreferenced model rows have not acquired semantic names by this campaign.

## Consumer baseline

| GAME VA / bytes | Function | Strict baseline |
| --- | --- | ---: |
| `0x80018054 / 1116` | `player_use_item` | 100.000000% |
| `0x8001a5ac / 1284` | `collision_query_world` | 97.943924% |
| `0x8001ebb8 / 384` | `render_map_object` | 100.000000% |
| `0x8001f218 / 1408` | `render_entities` | 97.380684% |
| `0x8002fa88 / 3472` | `actor_update_current_action` | 100.000000% |
| `0x800308c0 / 428` | `actor_pool_load_placements` | 100.000000% |
| `0x80030c7c / 572` | `map_object_mark_collision_edge` | 100.000000% |
| `0x80030eb8 / 196` | `map_object_probe_forward` | 93.755104% |
| `0x80030f7c / 96` | `map_object_pool_clear` | 100.000000% |
| `0x80031008 / 1096` | `map_object_pool_load` | 100.000000% |
| `0x800314f8 / 204` | `map_object_pool_find_near_point` | 100.000000% |
| `0x800315c4 / 448` | `map_object_pool_find_interaction_from` | 100.000000% |
| `0x800317a4 / 144` | `map_object_effect_pool_acquire` | 100.000000% |
| `0x80031834 / 404` | `map_object_spawn_effect` | 94.455444% |
| `0x800319c8 / 396` | `map_object_spawn_actor_debris` | 100.000000% |
| `0x80031b54 / 240` | `map_object_pool_trigger_link` | 100.000000% |
| `0x80031c44 / 132` | `map_object_pool_clear_link` | 100.000000% |
| `0x80031cc8 / 3096` | `map_object_pool_update` | 98.966410% |
| `0x80033f64 / 648` | `map_ambient_script_floor1` | 100.000000% |
| `0x800346a8 / 908` | `map_floor5_transition_cutscene` | 100.000000% |
| `0x80034de4 / 2308` | `map_interaction_dispatch` | 83.436745% |
| `0x80035b5c / 696` | `map_world_state_persist` | 94.821840% |
| `0x80035e44 / 1692` | `map_restore_floor_state` | 99.964540% |

## Final implementation and verification

The retained `KfMapObjectId` domain types the GAME runtime object, both actor
placement/runtime drop fields, spawn parameter and all model-ID locals. The
renderer keeps a halfword storage wrapper. All referenced direct model cases,
comparisons and assignments use named members. The shared placement loader,
saved-floor bytes, heterogeneous actor definition parameters, item/model
mapping and generic definition/TMD/menu interfaces carry explicit conversions.
The three curated field types and spawn signature now agree with the source.

This replaces 38 inline occurrences: pool loading 15, item use 12, runtime
objects 8 and map scripts 3. Five existing ledger documents were refreshed;
26 complete source ledgers still account for every occurrence in their files,
now 2,339 retained values. The whole-source census is 6,792 occurrences across
111 C files, not a count of constants still deserving names. Source unknowns
remain 10 matching lines / 14 identifier occurrences; none acquired a guessed
identity in this campaign.

All 112 source/image variants were compiled from isolated pre/post sources.
Allocated bytes and alignment, runtime symbols and ordered relocations are
identical. Only actor-behavior and scene-render debug line tables change.
Every one of the 23 consumer baselines above has an unchanged final verdict:
15 exact and eight partial. Their controls preserve 5,340 instruction words,
272 ordered calls and 225 address materializations. The exact subset agrees
with 2,571 complete linked retail words, including delay slots.

Across the 484 live scores, the sole movement is the independent OPEN
`render_enqueue_map` improvement in concurrent commit `7665e68`, from
99.978990% to 100%. Its exact committed source was separately compiled with
both versions of this campaign's headers: runtime bytes and relocations agree
with each other and with its live object. All other live objects agree with
the isolated post-change compilation; every GAME score is unchanged. This
campaign claims no additional exact functions and performs no banking.

The eight partial consumers retain these first divergences; these are existing
unattributed instruction differences, not evidence of a particular compiler
mechanism:

| Function / GAME site | Candidate | Retail |
| --- | --- | --- |
| `collision_query_world` / `8001a5b0` | `sw s0,24(sp)` | `sw s2,32(sp)` |
| `render_entities` / `8001f274` | `move s5,v1` | `move s6,v1` |
| `map_object_probe_forward` / `80030ec4` | `lw a3,8(a0)` | `lw t0,8(a0)` |
| `map_object_spawn_effect` / `80031838` | `sw ra,32(sp)` | `sw ra,36(sp)` |
| `map_object_pool_update` / `80031dcc` | `lui a0,0x8005` | `lbu v1,0(s3)` |
| `map_interaction_dispatch` / `80034de4` | `addiu sp,sp,-80` | `addiu sp,sp,-72` |
| `map_world_state_persist` / `80035b6c` | `lui v1,0x800a` | `addiu a1,a0,-543` |
| `map_restore_floor_state` / `80035e8c` | `addiu v0,v0,-1690` | `addiu v1,a0,-1690` |

Clang accepts the actual field-to-field, field-to-parameter and byte-to-halfword
storage chain, including explicit encoding/decoding. Eight independent misuse
controls reject raw/foreign assignments, comparisons, arguments and storage
construction. The whole-tree check retains the same 300 diagnostics and
65/112 passing variants. These are compiler controls, not gameplay fixtures.
All 683 repository tests pass (87.704 seconds), along with inventory, Ruff,
whitespace and the source-ledger audit. The full build still reports source-data
matches PSX 0/1, GAME 9/42 and OPEN 2/19, and target relink PSX 1/1, GAME 75/77
and OPEN 34/38, with zero artifact failures.

OPEN's model domain and remaining object behavior/action domains still need
their own propagation work. Numeric values that are not model IDs remain in
their existing ledgers or open naming campaigns.
