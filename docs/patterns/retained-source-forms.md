# Retained source forms

This is the complete list of jump labels, steering constructs and
"Unresolved source form" markers left in the KF1 sources after the
October 2026 readability pass (PR #83). Each one was kept because every
natural rewrite tried changed the compiled code. Percentages are the
`kf try` listing similarity of the rewritten function. The current source is
byte-identical; a lower number means the rewrite moved the code away from
retail. "Shapes" refers to the tables in
[source-shapes-gcc257.md](source-shapes-gcc257.md).

The two `do { ... } while (0)` uses in `src/psx/main.c` (`OVERLAY_LAUNCH`) and
`src/game/player_update.c` (`PLAYER_TICK_ARMOR_VITALS`) are ordinary
statement-macro wrappers, not steering. An inline-function version of the
launch macro dropped PSX `main` to 77.4%.

## Jump labels

| Function | Label(s) | Rejected natural form (similarity) |
| --- | --- | --- |
| `actor_pool_find_free` | `done` | `return actor` in loop (88.9%), up-counting `for` (64.7%) |
| `actor_pool_spawn` | `found` | body inside the loop (48.5%), `break` + `count == -1` (73.8%) |
| `actor_distance_to_point`, `player_distance_to_point` | `out_of_range` | repeated `return` (Shapes: shared exit) |
| `actor_try_select_profiled_action` | `rejected` | direct `return` (94.4%) |
| `actor_select_next_action` | `apply_choice` | movement fallback as an inline helper (36.7%) |
| `actor_move_xz_with_collision` | `blocked` | Shapes (first pass) |
| `actor_spawn_action_effect` | `clamp_steps` | duplicated clamp per code (86.9%) |
| `actor_update_current_action` | `vertical`, `apply_gravity`, `land`, `bounce` | Shapes (first pass) |
| `actor_pool_load_placements`, `map_event_pool_load`, `map_object_pool_load`, `opening_entity_pool_load_placements` | `mark_free` / `fill` / `mark_empty` | single `if`/`else` without the shared store (event pool 88.7%); Shapes records the actor-pool form |
| `render_bind_animated_instance` | `retry_allocation`, `reinitialize_record`, `update_vertex_cache` | `while`/`for` retry loop (91.1%) |
| `effect_update_dispatch` | `travel`, `lightning_impact`, `play_phase_sound`, `invalidate_and_advance`, `advance_effect_phase`, `randomize_homing_direction` | Shapes (Wind Cutter tail); first pass |
| `effect_map_collision` | `query_targets`, `rectangle_span` | per-orientation coordinate then one compare (63.6%) |
| `effect_pool_construct` | `initialize_lightning_bolt`, `initialize_lightning_impact` | duplicated initialisation (90.7%) |
| `shop_menu_buy` | `load_selected_model` | duplicated model load (95.5%) |
| `map_event_refresh_dialogue_stage` | `reset_dialogue_page` | duplicated reset (61.3%), clamped target (25.0%); Shapes |
| `map_object_probe_door_closing` | `probe` | offsets in the switch, one query after it (39.3%) |
| `map_interaction_dispatch` | `notify_default`, `notify_linked`, `start_paired_door`, `clear_event_phase` | duplicated default notification (84.4%) |
| `player_update_vertical_motion` | `falling`, `stepping_up`, `apply_camera_height` | grounded transitions first (61.2%) |
| `player_update` | `magic_done`, `cancel`, `store_input` | "weapon magic started" flag (88.0%) |
| `player_update` | `clear_slowed`, `clear_poison`, `clear_curse` | expire-then-clear `if`s (93.4%, 94.9%) |
| `player_use_item` | `done` | notify and `return` at the harp test (98.8%) |
| `player_warp_trigger_update` | `change_floor`, `change_to_floor4` | first pass |
| `format_vsprintf` | `emit_padded`, `copy` | duplicated tails (59.4%), padding helper (34.9%) |
| `opening_entity_render` | `render_alternate` | duplicated tail (90.5%) |
| `opening_run` | `opening_reload` | duplicated reload (73.6%) |
| `opening_cylinder_transition` | `deactivate` | `if (mode != REMOVE)` around the setup (84.9%) |

## Unresolved source forms

| Function | Construct | Rejected natural form (similarity) |
| --- | --- | --- |
| `menu_draw_item_detail` | `do { i = 0; } while (0)` before the name copy | plain `for` (84.6%), index copy (83.3%), `i` initialised at declaration (78.7%), declaration order (84.6%) |
| `menu_item_model_preview`, `menu_draw_pickup_preview`, `menu_draw_item_detail` | `rows = item_name_rows` alias | direct `&item_name_rows[id]` (89.7%, 82.2%) |
| `player_update` | never-used `unused_vector` frame slot | removal (91.8%) |
| `render_map_cell` (GAME) | never-used matrix | first pass |
| `effect_update_orbiting_projectile` | two never-used matrices | first pass |
| `effect_update_dispatch` | spawner phase copied through a local | direct `effect->phase++` (92.4%) |
| `effect_pool_construct` | optional arguments read from `&direction` | [effect-constructor-varargs.md](effect-constructor-varargs.md) |
| `map_object_pool_clear_link` | `<` and `==` tested separately | `<=` (33.3%) |
| `magic_cast` | pitch zeroed then added | first pass |
| `player_calculate_damage_component` | locals seeded from parameters | first pass |
| `cd_file_load_allocated`, `cd_file_load_into` (GAME) | `loaded` also holds the sector count | first pass |
| `talk_show_dialogue_page` | directory digits written through their own pointer | direct template indexing (55.4%) |
| `player_warp_shimmer` | never-read position copy | first pass |
| `opening_run` | redundant "not advance" test | plain skip test (89.5%) |
| `display_initialize` (OPEN) | height held in two locals | first pass |
| `render_enqueue_map` (OPEN) | one variable for both packet views | first pass |
| `render_enqueue_tmd`, `render_enqueue_unlit_triangles` (OPEN) | reserved stack word | [reconstruction-debt-review.md](reconstruction-debt-review.md) |
| `audio_reset_voice_slots` | sentinel kept in its own local | first pass |

"First pass" means the rewrite was tested and rejected during the first
readability pass, and the construct was then left with its marker; that pass
did not record a separate similarity figure.
