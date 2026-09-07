# GAME floor identifier domain

## Function Match Plan

One semantic floor domain spans byte player-state fields, word save-summary
fields, byte dialogue parameters and signed-word warp/resource parameters.
Use enum members for floor comparisons, assignments and switch cases. Preserve
all retail widths, source control flow, load signedness and encoded-data bytes.
Explicit numeric conversions belong only at array offsets, text formatting,
serialization and the floor-to-dialogue-stage mapping.

The local sibling implementations were inspected: HoMM2's
`homm2-buka/include/Ints.h` on `decomp-gold-2.1-buka` and Gruntz's `include/Enums.h`
separate domain values, fixed-width field storage and signature representation.
Their storage-proxy pattern supports the same domain at multiple widths.
King's Field retains its active C++20 `kf check-types` command and compiler
errors for cross-domain use. New storage construction/assignment accepts the
owning enum, without implicit raw-integer assignment. Encoded boundaries remain
explicit and do not claim runtime validation.

The player death restart uses 255 as a force-reload sentinel in a signed-word
local. Preserve that extended domain value without treating it as a real floor.
The highest visited floor also selects a dialogue stage: explicitly encode
that mapping rather than retyping dialogue stages as floors. Floor-height
locals are numeric heights and remain separate from floor identity.

Retail sources, all six semantic views, history and proven caller references
were captured before edits. The campaign functions operate on game state,
resource paths and save records; SDK rendering and file helpers remain external.
Calls retain proven classification; address/table references remain validated.
The complete word and relocation controls must preserve each listed baseline,
including delay slots, with no promotion of partial functions to exact.

| Function | GAME VA / bytes | Strict baseline | Calls / branches | Plan |
| --- | --- | ---: | --- | --- |
| `game_state_initialize` | `0x800151cc / 740` | 100.000000% | 3 / 2 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `player_death_restart` | `0x800154b0 / 412` | 100.000000% | 6 / 3 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `player_warp_to_floor_entry` | `0x80017cf8 / 324` | 100.000000% | 6 / 3 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `actor_show_info_image` | `0x80017edc / 200` | 100.000000% | 3 / 4 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `player_use_item` | `0x80018054 / 1116` | 100.000000% | 20 / 20 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `map_resource_path_set_floor` | `0x8001b390 / 20` | 100.000000% | 0 / 0 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `audio_play_current_map_sequence` | `0x8001b49c / 188` | 100.000000% | 1 / 7 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `map_resources_load` | `0x8001b558 / 600` | 100.000000% | 31 / 1 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `effect5_texture_cache_prepare` | `0x8001bae4 / 176` | 100.000000% | 6 / 1 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `menu_map_viewer` | `0x80022d7c / 1024` | 100.000000% | 17 / 9 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `menu_draw_stats_header` | `0x80025f38 / 1440` | 97.991670% | 28 / 9 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `menu_draw_dialog_frame` | `0x80027ee4 / 1180` | 100.000000% | 27 / 7 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `save_system_read_catalog` | `0x8002b078 / 216` | 100.000000% | 2 / 4 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `save_file_write_slot` | `0x8002b73c / 1268` | 100.000000% | 24 / 24 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `save_file_read_slot` | `0x8002beb0 / 972` | 100.000000% | 13 / 22 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `talk_show_dialogue_page` | `0x8002c9d4 / 164` | 98.780490% | 1 / 3 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `actor_apply_damage` | `0x8002d120 / 904` | 100.000000% | 10 / 27 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `actor_play_sound_at_phase` | `0x8002dcfc / 152` | 100.000000% | 3 / 3 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `actor_update_current_action` | `0x8002fa88 / 3472` | 100.000000% | 69 / 86 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `map_object_pool_update` | `0x80031cc8 / 3096` | 98.966410% | 35 / 64 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `audio_play_map_sequence` | `0x80032a4c / 272` | 100.000000% | 6 / 2 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `map_event_refresh_dialogue_stage` | `0x800337ac / 116` | 100.000000% | 0 / 4 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `map_event_interact` | `0x80034a80 / 724` | 100.000000% | 7 / 17 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `map_show_screen_image` | `0x80034d54 / 144` | 88.888885% | 1 / 3 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `map_interaction_dispatch` | `0x80034de4 / 2308` | 83.436745% | 54 / 58 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `map_event_update_animation_loop` | `0x800358e0 / 140` | 100.000000% | 1 / 3 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `map_event_pool_update` | `0x8003596c / 496` | 100.000000% | 8 / 11 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `map_world_state_persist` | `0x80035b5c / 696` | 94.821840% | 0 / 17 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `map_restore_floor_state` | `0x80035e44 / 1692` | 99.964540% | 18 / 27 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `map_load_floor` | `0x80036554 / 164` | 100.000000% | 4 / 0 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `player_warp_change_floor` | `0x80036850 / 348` | 100.000000% | 5 / 7 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `player_warp_same_floor` | `0x800369ac / 324` | 100.000000% | 7 / 3 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |
| `player_warp_trigger_update` | `0x80036af0 / 588` | 100.000000% | 6 / 23 | Propagate the floor domain at existing fields/API boundaries; preserve instructions and referents. |

Verification requires focused forced compiles, all-variant isolated runtime and
ordered-relocation comparison, strict match score preservation, exact retail
instruction controls, compiler rejection controls for mixed domains/raw values,
modern whole-tree diagnostics, inventory, Ruff, repository tests, whitespace
checks and the full build. No size assertions are introduced.

## Final implementation and verification

`KfFloorId` names floors 1..5 and the existing 255 force-reload sentinel.
`KF_ENUM_STORAGE` retains each field's byte or word encoding while exposing the
same enum in C++20. `KF_ENUM_PARAM` retains the byte dialogue parameter; other
floor APIs keep their signed-word representation through `KfFloorId` itself.
The inventory reader records the storage macro's domain and encoding, rejects
undeclared domains, and preserves the three curated field identities. The
source-claim parser support was supplied by concurrent commit `baf44e7`.

All floor comparisons, direct switch cases and destination assignments now
use enum members. Numeric boundaries remain explicit: path digits, menu number
formatting, one-based entry-table offsets, saved-floor byte offsets, and the
intentional highest-floor-to-dialogue-stage mapping. The texture cache's sole
caller supplies a floor; its old generic mode parameter now carries that domain.
The 50 replaced inline occurrences leave a source-wide census of 6,830.
Eight refreshed ledger documents preserve current accounting across all 26
previously verified files, now 2,354 retained occurrences.

The modern compiler accepts valid player/save transfers and explicit encoding
and rejects each of eight misuse controls: raw assignment, foreign-domain
assignment, raw comparison, foreign-domain comparison, raw argument,
foreign-domain argument, implicit integer return, and a storage wrapper whose
owner is not an enum. These controls use the actual project headers and flake
Clang with the MIPS-I/O32 target. They are syntax checks under `build/`, not new
gameplay test programs. No size assertions were added.

All 112 variants were compiled independently before and after. Allocated bytes,
alignment, runtime symbols and ordered relocations are identical. The live
objects agree with that isolated result; all 484 strict scores remain unchanged.
The 33 function controls preserve 6,416 candidate instruction words, 421 ordered
direct calls and 465 address materializations. The 26 exact controls retain
4,034 complete retail instruction words, including all delay slots.

| Function | Final strict score | Verdict |
| --- | ---: | --- |
| `game_state_initialize` | 100.000000% | Exact, unchanged |
| `player_death_restart` | 100.000000% | Exact, unchanged |
| `player_warp_to_floor_entry` | 100.000000% | Exact, unchanged |
| `actor_show_info_image` | 100.000000% | Exact, unchanged |
| `player_use_item` | 100.000000% | Exact, unchanged |
| `map_resource_path_set_floor` | 100.000000% | Exact, unchanged |
| `audio_play_current_map_sequence` | 100.000000% | Exact, unchanged |
| `map_resources_load` | 100.000000% | Exact, unchanged |
| `effect5_texture_cache_prepare` | 100.000000% | Exact, unchanged |
| `menu_map_viewer` | 100.000000% | Exact, unchanged |
| `menu_draw_stats_header` | 97.991670% | Partial, unchanged |
| `menu_draw_dialog_frame` | 100.000000% | Exact, unchanged |
| `save_system_read_catalog` | 100.000000% | Exact, unchanged |
| `save_file_write_slot` | 100.000000% | Exact, unchanged |
| `save_file_read_slot` | 100.000000% | Exact, unchanged |
| `talk_show_dialogue_page` | 98.780490% | Partial, unchanged |
| `actor_apply_damage` | 100.000000% | Exact, unchanged |
| `actor_play_sound_at_phase` | 100.000000% | Exact, unchanged |
| `actor_update_current_action` | 100.000000% | Exact, unchanged |
| `map_object_pool_update` | 98.966410% | Partial, unchanged |
| `audio_play_map_sequence` | 100.000000% | Exact, unchanged |
| `map_event_refresh_dialogue_stage` | 100.000000% | Exact, unchanged |
| `map_event_interact` | 100.000000% | Exact, unchanged |
| `map_show_screen_image` | 88.888885% | Partial, unchanged |
| `map_interaction_dispatch` | 83.436745% | Partial, unchanged |
| `map_event_update_animation_loop` | 100.000000% | Exact, unchanged |
| `map_event_pool_update` | 100.000000% | Exact, unchanged |
| `map_world_state_persist` | 94.821840% | Partial, unchanged |
| `map_restore_floor_state` | 99.964540% | Partial, unchanged |
| `map_load_floor` | 100.000000% | Exact, unchanged |
| `player_warp_change_floor` | 100.000000% | Exact, unchanged |
| `player_warp_same_floor` | 100.000000% | Exact, unchanged |
| `player_warp_trigger_update` | 100.000000% | Exact, unchanged |

The seven partial controls keep their first raw divergence:

| Function | GAME site | Candidate / retail |
| --- | --- | --- |
| menu_draw_stats_header | `80025f38` | Frame allocation 64 / 72 bytes. |
| talk_show_dialogue_page | `8002ca04` | `mfhi v1` / `mfhi v0`. |
| map_object_pool_update | `80031dcc` | `lui a0,0x8005` / `lbu v1,0(s3)`. |
| map_show_screen_image | `80034d84` | `mfhi a2` / `mfhi v1`. |
| map_interaction_dispatch | `80034de4` | Frame allocation 80 / 72 bytes. |
| map_world_state_persist | `80035b6c` | `lui v1,0x800a` / `addiu a1,a0,-543`. |
| map_restore_floor_state | `80035e8c` | `addiu v0,v0,-1690` / `addiu v1,a0,-1690`. |

These remain unattributed existing residues, not new exact results. Modern
whole-tree diagnostics remain the same 300 errors (65/112 variants pass).
Inventory, Ruff, whitespace checks and all 683 local tests pass (87.140 seconds).
`nix flake check -L` passes; its isolated suite skips 143 tests requiring local
retail/runtime prerequisites. The full local `kf build` still reports existing
source-data and target-relink mismatches, with no artifact failures.

Object-model identifiers, including the special handling for model 89, remain
pending their own typed domain and evidence-backed names. This floor campaign
does not classify those remaining literals as complete naming work.
