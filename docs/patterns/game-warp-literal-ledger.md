# GAME warp and lifecycle retained-literal ledger

Companion to [the warp-mode evidence](game-warp-mode-domain.md). Every retained
numeric/character occurrence in these three files is listed separately.
Comments, strings, identifier digits, enum/macro definitions and address claims
are excluded. Repeated tokens have individual rows; unary minus remains in
the expression. Zero occurrences in the forwarding wrapper does not imply
completeness of other modules or of the larger naming goal.

## `src/game/player_warp.c`

90 retained occurrences.

| Function / datum | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `actor_transform_color_matrix` | 9 | `250` | `{{250, 100, 500}, {250, 100, 500}, {250, 100, 500}}, {0, 0, 0}` | Authored Q12 color-matrix coefficient row 0, column 0; preserve the transform lighting recipe. |
| `actor_transform_color_matrix` | 9 | `100` | `{{250, 100, 500}, {250, 100, 500}, {250, 100, 500}}, {0, 0, 0}` | Authored Q12 color-matrix coefficient row 0, column 1; preserve the transform lighting recipe. |
| `actor_transform_color_matrix` | 9 | `500` | `{{250, 100, 500}, {250, 100, 500}, {250, 100, 500}}, {0, 0, 0}` | Authored Q12 color-matrix coefficient row 0, column 2; preserve the transform lighting recipe. |
| `actor_transform_color_matrix` | 9 | `250` | `{{250, 100, 500}, {250, 100, 500}, {250, 100, 500}}, {0, 0, 0}` | Authored Q12 color-matrix coefficient row 1, column 0; preserve the transform lighting recipe. |
| `actor_transform_color_matrix` | 9 | `100` | `{{250, 100, 500}, {250, 100, 500}, {250, 100, 500}}, {0, 0, 0}` | Authored Q12 color-matrix coefficient row 1, column 1; preserve the transform lighting recipe. |
| `actor_transform_color_matrix` | 9 | `500` | `{{250, 100, 500}, {250, 100, 500}, {250, 100, 500}}, {0, 0, 0}` | Authored Q12 color-matrix coefficient row 1, column 2; preserve the transform lighting recipe. |
| `actor_transform_color_matrix` | 9 | `250` | `{{250, 100, 500}, {250, 100, 500}, {250, 100, 500}}, {0, 0, 0}` | Authored Q12 color-matrix coefficient row 2, column 0; preserve the transform lighting recipe. |
| `actor_transform_color_matrix` | 9 | `100` | `{{250, 100, 500}, {250, 100, 500}, {250, 100, 500}}, {0, 0, 0}` | Authored Q12 color-matrix coefficient row 2, column 1; preserve the transform lighting recipe. |
| `actor_transform_color_matrix` | 9 | `500` | `{{250, 100, 500}, {250, 100, 500}, {250, 100, 500}}, {0, 0, 0}` | Authored Q12 color-matrix coefficient row 2, column 2; preserve the transform lighting recipe. |
| `actor_transform_color_matrix` | 9 | `0` | `{{250, 100, 500}, {250, 100, 500}, {250, 100, 500}}, {0, 0, 0}` | Zero color-matrix translation X. |
| `actor_transform_color_matrix` | 9 | `0` | `{{250, 100, 500}, {250, 100, 500}, {250, 100, 500}}, {0, 0, 0}` | Zero color-matrix translation Y. |
| `actor_transform_color_matrix` | 9 | `0` | `{{250, 100, 500}, {250, 100, 500}, {250, 100, 500}}, {0, 0, 0}` | Zero color-matrix translation Z. |
| `player_warp_shimmer` | 50 | `0` | `scale_y = 0;` | Zero-height start for a growing shimmer. |
| `player_warp_shimmer` | 64 | `1` | `for (i = WARP_SHIMMER_COUNT - 1; i != -1; i--) {` | Last index starts the descending count while the pointer walks forward. |
| `player_warp_shimmer` | 64 | `1` | `for (i = WARP_SHIMMER_COUNT - 1; i != -1; i--) {` | Minus one marks exhaustion after index zero. |
| `player_warp_shimmer` | 78 | `0` | `for (frame = 0; frame < WARP_SHIMMER_FRAMES; frame++) {` | First frame of the fixed-length shimmer loop. |
| `player_warp_shimmer` | 83 | `0` | `for (i = 0; i < WARP_SHIMMER_COUNT; i++) {` | First effect in the forward update traversal. |
| `player_warp_shimmer` | 89 | `1` | `if (current_scale_y < WARP_SHIMMER_TALL_SCALE + 1) {` | One-past bound includes the tall scale endpoint before adding the signed step; preserve the overshoot behavior. |
| `player_warp_shimmer` | 103 | `1` | `for (i = WARP_SHIMMER_COUNT - 1; i != -1; i--) {` | Last index starts the descending count while the pointer walks forward. |
| `player_warp_shimmer` | 103 | `1` | `for (i = WARP_SHIMMER_COUNT - 1; i != -1; i--) {` | Minus one marks exhaustion after index zero. |
| `player_warp_same_floor` | 148 | `1` | `player_state.map_cell.z, -1);` | Subtract one occupancy contribution from the old cell before the within-floor warp. |
| `player_warp_trigger_update` | 182 | `29` | `if (cell == WARP_CELL_KEY(29, 56)) {` | Authored floor-1 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 182 | `56` | `if (cell == WARP_CELL_KEY(29, 56)) {` | Authored floor-1 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 186 | `25` | `} else if (cell == WARP_CELL_KEY(25, 11)) {` | Authored floor-1 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 186 | `11` | `} else if (cell == WARP_CELL_KEY(25, 11)) {` | Authored floor-1 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 189 | `39` | `} else if (cell == WARP_CELL_KEY(39, 35)) {` | Authored floor-1 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 189 | `35` | `} else if (cell == WARP_CELL_KEY(39, 35)) {` | Authored floor-1 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 191 | `15` | `} else if (cell == WARP_CELL_KEY(15, 2)) {` | Authored floor-1 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 191 | `2` | `} else if (cell == WARP_CELL_KEY(15, 2)) {` | Authored floor-1 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 193 | `1` | `return 1;` | True request to enter the ending; the caller keeps the grown shimmer and leaves normal gameplay. |
| `player_warp_trigger_update` | 199 | `29` | `if (cell == WARP_CELL_KEY(29, 56)) {` | Authored floor-2 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 199 | `56` | `if (cell == WARP_CELL_KEY(29, 56)) {` | Authored floor-2 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 202 | `28` | `} else if (cell == WARP_CELL_KEY(28, 18)) {` | Authored floor-2 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 202 | `18` | `} else if (cell == WARP_CELL_KEY(28, 18)) {` | Authored floor-2 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 209 | `25` | `if (cell == WARP_CELL_KEY(25, 11)) {` | Authored floor-3 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 209 | `11` | `if (cell == WARP_CELL_KEY(25, 11)) {` | Authored floor-3 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 212 | `28` | `} else if (cell == WARP_CELL_KEY(28, 18)) {` | Authored floor-3 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 212 | `18` | `} else if (cell == WARP_CELL_KEY(28, 18)) {` | Authored floor-3 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 215 | `7` | `} else if (cell == WARP_CELL_KEY(7, 22) \|\| cell == WARP_CELL_KEY(43, 92)) {` | Authored floor-3 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 215 | `22` | `} else if (cell == WARP_CELL_KEY(7, 22) \|\| cell == WARP_CELL_KEY(43, 92)) {` | Authored floor-3 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 215 | `43` | `} else if (cell == WARP_CELL_KEY(7, 22) \|\| cell == WARP_CELL_KEY(43, 92)) {` | Authored floor-3 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 215 | `92` | `} else if (cell == WARP_CELL_KEY(7, 22) \|\| cell == WARP_CELL_KEY(43, 92)) {` | Authored floor-3 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 223 | `39` | `if (cell == WARP_CELL_KEY(39, 35)) {` | Authored floor-4 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 223 | `35` | `if (cell == WARP_CELL_KEY(39, 35)) {` | Authored floor-4 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 226 | `7` | `} else if (cell == WARP_CELL_KEY(7, 22)) {` | Authored floor-4 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 226 | `22` | `} else if (cell == WARP_CELL_KEY(7, 22)) {` | Authored floor-4 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 229 | `39` | `} else if (cell == WARP_CELL_KEY(39, 69)) {` | Authored floor-4 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 229 | `69` | `} else if (cell == WARP_CELL_KEY(39, 69)) {` | Authored floor-4 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 233 | `43` | `} else if (cell == WARP_CELL_KEY(43, 92)) {` | Authored floor-4 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 233 | `92` | `} else if (cell == WARP_CELL_KEY(43, 92)) {` | Authored floor-4 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 240 | `39` | `if (cell == WARP_CELL_KEY(39, 69)) {` | Authored floor-5 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 240 | `69` | `if (cell == WARP_CELL_KEY(39, 69)) {` | Authored floor-5 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 242 | `70` | `} else if (cell == WARP_CELL_KEY(70, 61)) {` | Authored floor-5 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 242 | `61` | `} else if (cell == WARP_CELL_KEY(70, 61)) {` | Authored floor-5 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 243 | `2` | `player_warp_same_floor(2, 18, 37);` | Encoded floor-five variant 2 selected by this warp; the resource identity is unresolved, so no location-based alias is assigned. |
| `player_warp_trigger_update` | 243 | `18` | `player_warp_same_floor(2, 18, 37);` | Authored destination cell X; the warp places the camera at its tile center. |
| `player_warp_trigger_update` | 243 | `37` | `player_warp_same_floor(2, 18, 37);` | Authored destination cell Z; the warp places the camera at its tile center. |
| `player_warp_trigger_update` | 244 | `18` | `} else if (cell == WARP_CELL_KEY(18, 37)) {` | Authored floor-5 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 244 | `37` | `} else if (cell == WARP_CELL_KEY(18, 37)) {` | Authored floor-5 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 245 | `70` | `player_warp_same_floor(KF_FLOOR5_ENTRY_VARIANT, 70, 61);` | Authored destination cell X; the warp places the camera at its tile center. |
| `player_warp_trigger_update` | 245 | `61` | `player_warp_same_floor(KF_FLOOR5_ENTRY_VARIANT, 70, 61);` | Authored destination cell Z; the warp places the camera at its tile center. |
| `player_warp_trigger_update` | 246 | `5` | `} else if (cell == WARP_CELL_KEY(5, 24)) {` | Authored floor-5 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 246 | `24` | `} else if (cell == WARP_CELL_KEY(5, 24)) {` | Authored floor-5 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 247 | `39` | `player_warp_same_floor(KF_FLOOR5_ALTERNATE_MUSIC_VARIANT, 39, 47);` | Authored destination cell X; the warp places the camera at its tile center. |
| `player_warp_trigger_update` | 247 | `47` | `player_warp_same_floor(KF_FLOOR5_ALTERNATE_MUSIC_VARIANT, 39, 47);` | Authored destination cell Z; the warp places the camera at its tile center. |
| `player_warp_trigger_update` | 248 | `39` | `} else if (cell == WARP_CELL_KEY(39, 47)) {` | Authored floor-5 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 248 | `47` | `} else if (cell == WARP_CELL_KEY(39, 47)) {` | Authored floor-5 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 250 | `2` | `player_warp_same_floor(2, 5, 25);` | Encoded floor-five variant 2 selected by this warp; the resource identity is unresolved, so no location-based alias is assigned. |
| `player_warp_trigger_update` | 250 | `5` | `player_warp_same_floor(2, 5, 25);` | Authored destination cell X; the warp places the camera at its tile center. |
| `player_warp_trigger_update` | 250 | `25` | `player_warp_same_floor(2, 5, 25);` | Authored destination cell Z; the warp places the camera at its tile center. |
| `player_warp_trigger_update` | 252 | `1` | `return 1;` | True request to enter the ending; the caller keeps the grown shimmer and leaves normal gameplay. |
| `player_warp_trigger_update` | 254 | `5` | `} else if (cell == WARP_CELL_KEY(5, 37)) {` | Authored floor-5 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 254 | `37` | `} else if (cell == WARP_CELL_KEY(5, 37)) {` | Authored floor-5 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 255 | `14` | `player_warp_same_floor(KF_FLOOR5_ENTRY_VARIANT, 14, 79);` | Authored destination cell X; the warp places the camera at its tile center. |
| `player_warp_trigger_update` | 255 | `79` | `player_warp_same_floor(KF_FLOOR5_ENTRY_VARIANT, 14, 79);` | Authored destination cell Z; the warp places the camera at its tile center. |
| `player_warp_trigger_update` | 256 | `14` | `} else if (cell == WARP_CELL_KEY(14, 79)) {` | Authored floor-5 trigger cell X; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 256 | `79` | `} else if (cell == WARP_CELL_KEY(14, 79)) {` | Authored floor-5 trigger cell Z; the packed key preserves map coordinates without inventing a location name. |
| `player_warp_trigger_update` | 257 | `2` | `player_warp_same_floor(2, 5, 37);` | Encoded floor-five variant 2 selected by this warp; the resource identity is unresolved, so no location-based alias is assigned. |
| `player_warp_trigger_update` | 257 | `5` | `player_warp_same_floor(2, 5, 37);` | Authored destination cell X; the warp places the camera at its tile center. |
| `player_warp_trigger_update` | 257 | `37` | `player_warp_same_floor(2, 5, 37);` | Authored destination cell Z; the warp places the camera at its tile center. |
| `player_warp_trigger_update` | 261 | `0` | `return 0;` | No ending request; an ordinary floor or within-floor warp may already have occurred. |
| `actor_transform_definition5_to6` | 270 | `1` | `map_event_pool[1].state = KF_MAP_EVENT_DISABLED;` | Authored runtime event slot disabled by the definition-5-to-6 transform; no independently supported event identity. |
| `actor_transform_definition5_to6` | 271 | `2` | `map_event_pool[2].state = KF_MAP_EVENT_DISABLED;` | Second authored runtime event slot disabled by this transform; no independently supported event identity. |
| `actor_transform_definition5_to6` | 276 | `0` | `for (blend = 0; blend < KF_FIXED12_ONE + 1; blend += KF_FIXED12_ONE / ACTOR_TRANSFORM_BLEND_INTERVALS) {` | Zero blend starts the forward transformation. |
| `actor_transform_definition5_to6` | 276 | `1` | `for (blend = 0; blend < KF_FIXED12_ONE + 1; blend += KF_FIXED12_ONE / ACTOR_TRANSFORM_BLEND_INTERVALS) {` | One-past bound includes both Q12 endpoints, producing 65 motion updates. |
| `actor_transform_definition5_to6` | 280 | `0` | `render_frame(0, 0);` | Null position preserves the current view position. |
| `actor_transform_definition5_to6` | 280 | `0` | `render_frame(0, 0);` | Null rotation preserves the current view rotation. |
| `actor_transform_definition5_to6` | 284 | `0` | `for (blend = KF_FIXED12_ONE; blend >= 0; blend -= KF_FIXED12_ONE / ACTOR_TRANSFORM_BLEND_INTERVALS) {` | Zero is included as the reverse interpolation endpoint. |
| `actor_transform_definition5_to6` | 288 | `0` | `render_frame(0, 0);` | Null position preserves the current view position. |
| `actor_transform_definition5_to6` | 288 | `0` | `render_frame(0, 0);` | Null rotation preserves the current view rotation. |

## `src/game/game.c`

12 retained occurrences.

| Function / datum | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `frame_pacer_vsync_count` | 20 | `0` | `u32 frame_pacer_vsync_count = 0;` | Zero initialized VBlank counter or saved counter baseline. |
| `frame_pacer_last_vsync` | 23 | `0` | `u32 frame_pacer_last_vsync = 0;` | Zero initialized VBlank counter or saved counter baseline. |
| `game_main_loop` | 30 | `0` | `memset(&display_state.buffer_index, 0, INITIAL_GRAPHICS_CLEAR_BYTES);` | Zero byte pattern initializes the reviewed state span. |
| `game_main_loop` | 31 | `0` | `memset(actor_state.definitions, 0, INITIAL_ACTOR_CLEAR_BYTES);` | Zero byte pattern initializes the reviewed state span. |
| `game_main_loop` | 32 | `0` | `memset(map_object_state.definitions, 0, INITIAL_MAP_OBJECT_CLEAR_BYTES);` | Zero byte pattern initializes the reviewed state span. |
| `game_main_loop` | 33 | `0` | `memset(&effect_state, 0, sizeof(KfEffectState));` | Zero byte pattern initializes the reviewed state span. |
| `game_main_loop` | 34 | `0` | `memset(map_event_pool, 0, INITIAL_MAP_EVENT_CLEAR_BYTES);` | Zero byte pattern initializes the reviewed state span. |
| `game_main_loop` | 35 | `0` | `memset(&player_state, 0, sizeof(KfPlayerState));` | Zero byte pattern initializes the reviewed state span. |
| `game_main_loop` | 51 | `1` | `SetDispMask(1);` | Boolean SDK display enable. |
| `game_main_loop` | 72 | `0` | `player_state.allow_near_actor_spawn = 0;` | False clears the per-update near-spawn permission. |
| `game_main_loop` | 78 | `0` | `if (player_warp_trigger_update() != 0) {` | Nonzero requests the ending path; it is not a flag that any ordinary warp occurred. |
| `frame_pacer_wait` | 117 | `1` | `if (frame_pacer_last_vsync + (FRAME_PACER_INTERVAL_TICKS - 1)` | Subtract one preserves the strict comparison for the named three-tick interval; the separate wrap test remains unchanged. |

## `src/game/player_warp_shimmer_at_player.c`

0 retained occurrences.
