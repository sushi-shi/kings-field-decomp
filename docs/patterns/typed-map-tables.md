# Typed map grids and definition tables

## Function Match Plan

Baseline `ac2a72f`. GAME's map-resource loader and OPEN scene-zero loader copy
five 0x9c4-word grids in the same order. Cell consumers use 100-byte row
strides, while collision queries already compute a flat cell index. The
known 10000-byte extent and complete word-copy loops support a `KfMapGrid`
union with 100x100 cells, a flat byte array and 2500 words. Preserve each
curated global identity and existing BSS ownership; no placement or table
capacity is inferred from address spacing. Flat-index consumers use the
full byte view instead of indexing beyond row zero of a 2D declaration.

The GAME actor-definition loader copies exactly 456 words (twelve 152-byte
records); the map-object definition loader copies 320 words (160 eight-byte
records). Their sole resource callers, complete copy extents and all
record consumers support table unions with entry and whole-word views.
Propagate complete-table input signatures and use the entry member throughout
both existing state owners. Typed pointers remain typed after the generic
serialized resource boundary.

The 8500-byte saved-world union already has word and floor-record views.
Its bytewise initialization needs a complete byte view too. The separate
item-stock cast still marks a whole-object byte-clear boundary. Preserve the full clear
counts, existing loop shapes, store widths, constants and delay slots.

All selected game code is excluded from library attribution by its map,
resource, gameplay or render state; existing vendor callees retain their SDK
contracts. Six semantic views for every function, source histories, adjacent
functions and existing map/collision/resource dossiers are reviewed before
editing. Complete pre-edit objects and snapshots are saved under
`build/cast-model/typed-map-tables/`. No candidate relocation is promoted.

| Image / function | VA / bytes | Strict before | Blocks/JAL/returns | Final verdict |
| --- | --- | ---: | --- | --- |
| GAME.EXE `game_main_loop` | `800146b8 / 2e4` | 100 | 11/43/1 | 100%; raw unchanged |
| GAME.EXE `game_state_initialize` | `800151cc / 2e4` | 100 | 5/3/1 | 100%; raw unchanged |
| GAME.EXE `player_sync_position_to_map` | `80016ee8 / 158` | 100 | 11/2/1 | 100%; raw unchanged |
| GAME.EXE `player_move_horizontal` | `800171fc / 828` | 96.569 | 100/7/1 | 96.569%; raw unchanged |
| GAME.EXE `player_update_vertical_motion` | `80017a80 / 278` | 100 | 26/1/1 | 100%; raw unchanged |
| GAME.EXE `player_update` | `80018880 / 1a1c` | 96.9096 | 325/66/1 | 96.9096%; raw unchanged |
| GAME.EXE `map_floor_height_for_cell_position` | `8001a29c / 1b0` | 100 | 37/0/1 | 100%; raw unchanged |
| GAME.EXE `collision_adjust_cell_occupancy` | `8001a4e8 / c4` | 100 | 8/0/1 | 100%; raw unchanged |
| GAME.EXE `collision_query_world` | `8001a5ac / 504` | 97.9439 | 49/5/1 | 97.9439%; raw unchanged |
| GAME.EXE `common_resources_load` | `8001b180 / 210` | 100 | 7/10/1 | 100%; raw unchanged |
| GAME.EXE `map_resources_load` | `8001b558 / 258` | 100 | 4/31/1 | 100%; raw unchanged |
| GAME.EXE `render_map_cell` | `8001e5ec / 250` | 99.8784 | 24/9/1 | 99.8784%; raw unchanged |
| GAME.EXE `render_actor` | `8001e9a4 / 214` | 100 | 7/20/1 | 100%; raw unchanged |
| GAME.EXE `render_map_object` | `8001ebb8 / 180` | 100 | 8/15/1 | 100%; raw unchanged |
| GAME.EXE `item_load_floor_placements` | `80020b4c / 1b0` | 100 | 7/1/1 | 100%; raw unchanged |
| GAME.EXE `actor_initialize` | `8002cc64 / c4` | 100 | 5/2/1 | 100%; raw unchanged |
| GAME.EXE `actor_pool_begin_death_by_definition` | `8002cf84 / f4` | 100 | 11/1/1 | 100%; raw unchanged |
| GAME.EXE `actor_apply_damage` | `8002d120 / 388` | 100 | 39/10/1 | 100%; raw unchanged |
| GAME.EXE `actor_pool_apply_radial_damage` | `8002d4a8 / 1f8` | 100 | 14/2/1 | 100%; raw unchanged |
| GAME.EXE `actor_pool_find_overlap` | `8002da6c / 144` | 100 | 11/1/1 | 100%; raw unchanged |
| GAME.EXE `actor_bind_current` | `8002dbb0 / 8c` | 100 | 6/0/1 | 100%; raw unchanged |
| GAME.EXE `actor_try_select_ground_action` | `8002deb4 / 164` | 100 | 14/4/1 | 100%; raw unchanged |
| GAME.EXE `actor_update_current_action` | `8002fa88 / d90` | 100 | 166/69/1 | 100%; raw unchanged |
| GAME.EXE `actor_definitions_load` | `80030a6c / 2c` | 100 | 3/0/1 | 100%; raw unchanged |
| GAME.EXE `map_apply_copy_region` | `80030a98 / 1e4` | 100 | 8/0/1 | 100%; raw unchanged |
| GAME.EXE `map_object_mark_collision_edge` | `80030c7c / 23c` | 100 | 29/0/1 | 100%; raw unchanged |
| GAME.EXE `map_object_probe_forward` | `80030eb8 / c4` | 93.7551 | 13/1/1 | 93.7551%; raw unchanged |
| GAME.EXE `map_object_definitions_load` | `80030fdc / 2c` | 100 | 3/0/1 | 100%; raw unchanged |
| GAME.EXE `map_object_pool_load` | `80031008 / 448` | 100 | 43/11/1 | 100%; raw unchanged |
| GAME.EXE `map_object_pool_find_near_point` | `800314f8 / cc` | 100 | 9/1/1 | 100%; raw unchanged |
| GAME.EXE `map_object_pool_find_interaction_from` | `800315c4 / 1c0` | 100 | 14/5/1 | 100%; raw unchanged |
| GAME.EXE `map_object_pool_trigger_link` | `80031b54 / f0` | 100 | 13/1/1 | 100%; raw unchanged |
| GAME.EXE `map_object_pool_clear_link` | `80031c44 / 84` | 100 | 7/0/1 | 100%; raw unchanged |
| GAME.EXE `map_object_pool_update` | `80031cc8 / c18` | 98.9664 | 118/35/1 | 98.9664%; raw unchanged |
| GAME.EXE `map_event_pool_load` | `800338b8 / 22c` | 100 | 8/1/1 | 100%; raw unchanged |
| GAME.EXE `map_ambient_script_floor5` | `800342ec / f4` | 100 | 6/5/1 | 100%; raw unchanged |
| GAME.EXE `map_floor5_transition_cutscene` | `800346a8 / 38c` | 100 | 32/12/1 | 100%; raw unchanged |
| GAME.EXE `map_interaction_dispatch` | `80034de4 / 904` | 96.4905 | 119/54/1 | 96.4905%; raw unchanged |
| GAME.EXE `map_world_state_persist` | `80035b5c / 2b8` | 94.8218 | 28/0/1 | 94.8218%; raw unchanged |
| GAME.EXE `map_restore_floor_state` | `80035e44 / 69c` | 100 | 50/18/1 | 100%; raw unchanged |
| GAME.EXE `effect_map_collision` | `80037850 / 76c` | 98.2211 | 126/1/1 | 98.2211%; raw unchanged |
| GAME.EXE `effect_floor_deform_line` | `800384f8 / 1cc` | 100 | 13/1/1 | 100%; raw unchanged |
| GAME.EXE `effect_spawn_ground_branch` | `800388b4 / 184` | 100 | 11/3/1 | 100%; raw unchanged |
| GAME.EXE `effect_update_dispatch` | `80038a38 / 180c` | 96.9396 | 257/69/1 | 96.9396%; raw unchanged |
| GAME.EXE `magic_cast` | `8003a2a0 / 4c0` | 98.8915 | 37/13/1 | 98.8915%; raw unchanged |
| OPEN.EXE `opening_resources_load_scene0` | `80016348 / 1c8` | 100 | 1/18/1 | 100%; raw unchanged |
| OPEN.EXE `render_map_cell` | `80018bbc / 1d0` | 100 | 12/9/1 | 100%; raw unchanged |
| OPEN.EXE `item_load_floor_placements` | `800197e4 / 1b0` | 100 | 7/1/1 | 100%; raw unchanged |
| OPEN.EXE `opening_entity_pool_load_placements` | `80019a24 / 180` | 100 | 11/0/1 | 100%; raw unchanged |

Rebuild all affected units, check complete layouts and coherent curated
signatures, and compare each linked word, call and ordered referent against
baseline and retail. Keep all exact functions and existing partial outputs.
Then run the complete build, target-C census, tests, lint and diff checks;
run flake checks for the new checked-header entry. Bank only verified exact
functions directly changed by this stage.


## Result

All 177 compared functions in 27 units retain their linked instructions,
ordered calls and physical referents: 161 remain exact and sixteen existing
partials remain unchanged. The 49 directly edited functions have 38 exact
and eleven unchanged partial verdicts in the table above. Startup clears now
pass addresses of their actual actor/map-object owners; their original start
addresses and explicit clear lengths remain unchanged.

Five grids have coherent cell, linear-byte and full-word views in both
images. The two definition loaders accept complete tables, and every record
consumer uses their entry members. The saved-world initializer uses the
whole 8500-byte view. Fourteen C pointer casts and one header pointer cast
are removed. The census has 714 written casts, 42 in headers, with 452 C
pointer casts: 354 fewer than the original 806. All 112 target-C variants
parse without errors; the startup file was reparsed after its address fix.

All 713 repository tests pass (nine skips), Ruff and diff checks pass, and
`nix flake check -L` passes. The full build retains 439/471 exact functions
and the existing closure gates with no artifact failures. Only the 38 exact
functions directly edited by this stage are eligible for banking.
