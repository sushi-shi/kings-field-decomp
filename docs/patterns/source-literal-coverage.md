# Source literal coverage map

Working snapshot after the [overlay-mode review](overlay-mode-domains.md):
**111 C files**, **112 source/image variants**,
and **6,428 inline numeric/character occurrences**. The counts include authored
initializers, local extents, component indices, booleans and arithmetic. They
exclude comments, string contents, identifier digits, named enum/macro definitions
and ADDRESS/DATA/RODATA claims. These are inventory counts, **not missing-name
counts**. Vendor sources remain listed for source coverage, not game progress.

This map makes the scope of earlier work visible. A ledger link means that the
file or one of its functions has an explicit ledger reference; it does not
prove that every current occurrence has a valid reason. Several ledgers are
focused or historical. Domain notes explain shared contracts and retained
classes, but require a scope check before claiming complete file coverage.
Zero means no retained literal tokens under this census; it says nothing about
function matching or completeness of semantic identities.

## Current ledger coverage verified

For **86 files / 5,232 occurrences**, the explicit ledger rows were compared
against every current numeric/character token and its complete source-line
expression, including duplicate occurrences and initializers. All matched and
each row has a reason. Line-number drift is excluded from the comparison; source
expressions and multiplicity establish content coverage. These files are marked
below. This validates retained-literal accounting, not proof that every possible
semantic improvement has been exhausted.
This includes the reviewed warp forwarding wrapper, which has zero retained
literal occurrences.
The actor culling review names seven literal uses, reconciles two existing
ledgers and adds complete six-occurrence coverage for the actor pool.
The collision reconciliation verifies 322 already-documented occurrences across
four files and corrects the omitted actor jump consumer of the height table.
The menu confirmation review names 67 raw state occurrences and verifies the
remaining 265 occurrences across four more menu files. The save-result review
names another 20 values in those reviewed modules and the save system, bringing
the current root/list/save ledger to 259 occurrences. `map_scripts.c` remains
excluded until its rows and referent evidence are reconciled. The lighting/fade
review adds complete accounting for 117 occurrences across three files; their
palette data, null arguments and arithmetic origins remain literal with
individual reasons. The palette-domain review promotes the twelve named GAME/OPEN
selectors into distinct enum types and retains the same literal counts. The
overlay-mode review adds complete accounting for 154 OPEN renderer initializer
occurrences while typing the mode flow across GAME, PSX and OPEN.

The [grouped-ledger reconciliation](game-retained-ledger-reconciliation.md)
verified and normalized the death, item-use and notification accounting. Other
focused and historical audits remain outside this subtotal until their current
scope and expressions are checked. No missing-name total can be derived by
subtracting the verified subtotal from the source census.

## Next coverage work

1. Expand remaining partial menu and gameplay audits into complete current ledgers.
2. Continue domain propagation beyond the typed GAME/OPEN model IDs and floor
   identity; audit remaining selector fields and their consumers.
3. Complete per-function accounting for files currently covered only by domain
   notes; investigate unresolved resource identities before naming selectors.
4. Verify every retained occurrence against its reason before marking a file
   complete. File-name mentions, table totals and unchanged object bytes alone
   cannot establish semantic coverage.

## Unresolved source fields

The original 323 matching source lines have been reduced to **10 lines** with
`unknown_` identifiers (**14 identifier tokens**, because some copies name both
sides). Their established uses and missing evidence are:

| Field family | Current evidence | What remains unresolved |
| --- | --- | --- |
| OPEN controls +0x50a/+0x50c/+0x50e | Reset to zero in the opening entity pool. | No validated value-consuming reads establish their roles. |
| Floor-item byte +3 | Copied from placements in both GAME and OPEN; observed values include 8,28,32,64. | No established behavioral consumer supports a semantic label. |
| Map-event definition/runtime bytes | Copied on load; runtime +0x0d is saved/restored. Definition bytes are zero in the reviewed floor resources. | Serialization establishes persistence, not gameplay meaning. |
| Animation clip halfword +2 | All 214 reviewed clip headers contain 20. | No validated consumer establishes units or purpose; calling it a frame rate would be speculative. |

These fields remain unresolved in source rather than receiving invented names.
See [field identities](semantic-field-names.md) and the
[animation layout evidence](game-asset-animation-layout.md). Header-only opaque
fields and address-derived function names are separate from this ten-line count.

## PSX.EXE

1 files; 8 occurrences.

| Source | Occurrences | Existing audit references | Scope to check |
| --- | ---: | --- | --- |
| [main.c](../../src/psx/main.c) | 8 | [complete allocator/startup ledger](allocator-startup-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |

## GAME.EXE

73 files; 5,115 occurrences.

| Source | Occurrences | Existing audit references | Scope to check |
| --- | ---: | --- | --- |
| [actor.c](../../src/game/actor.c) | 171 | [game-actor-core ledger](game-actor-core-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [actor_behavior.c](../../src/game/actor_behavior.c) | 171 | [game-actor-behavior ledger](game-actor-behavior-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [actor_pool.c](../../src/game/actor_pool.c) | 6 | [complete actor-pool ledger](actor-pool-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [asset_registry.c](../../src/game/asset_registry.c) | 1 | [complete cache/camera ledger](game-animation-cache-camera-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [audio.c](../../src/game/audio.c) | 39 | [complete audio/resource ledger](audio-sequence-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [camera_path.c](../../src/game/camera_path.c) | 1 | [complete cache/camera ledger](game-animation-cache-camera-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [cd_file.c](../../src/game/cd_file.c) | 35 | [complete resource/audio ledger](resource-audio-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [collision.c](../../src/game/collision.c) | 265 | [complete ledger](game-world-collision-literal-ledger.md); [reconciliation](collision-ledger-reconciliation.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [collision_grid.c](../../src/game/collision_grid.c) | 5 | [complete ledger](game-world-collision-literal-ledger.md); [reconciliation](collision-ledger-reconciliation.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [color_lerp_rgb555.c](../../src/game/color_lerp_rgb555.c) | 0 | [complete math ledger](math-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [debug_text.c](../../src/game/debug_text.c) | 45 | [complete formatter ledger](formatter-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [display_play_transition.c](../../src/game/display_play_transition.c) | 29 | [domain notes](source-constant-names.md#gpu-setup-system-screens-and-transition-fades) | Expand the domain audit into current occurrence coverage. |
| [effect_dispatch.c](../../src/game/effect_dispatch.c) | 153 | [game-effect-dispatch ledger](game-effect-dispatch-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [effect_map_collision.c](../../src/game/effect_map_collision.c) | 43 | [complete ledger](game-magic-collision-literal-ledger.md); [reconciliation](collision-ledger-reconciliation.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [effect_pool.c](../../src/game/effect_pool.c) | 224 | [game-effect-pool ledger](game-effect-pool-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [effect_update.c](../../src/game/effect_update.c) | 105 | [game-effect-update ledger](game-effect-update-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [entity_model_render.c](../../src/game/entity_model_render.c) | 14 | [game-render-hud ledger](game-render-hud-literal-ledger.md); [game-menu-preview ledger](game-menu-preview-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [entity_render.c](../../src/game/entity_render.c) | 246 | [domain evidence](game-entity-sprite-tables.md) | Expand the domain audit into current occurrence coverage. |
| [equipment.c](../../src/game/equipment.c) | 7 | [complete equipment ledger](game-equipment-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [game.c](../../src/game/game.c) | 12 | [warp/lifecycle ledger](game-warp-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [geometry_render.c](../../src/game/geometry_render.c) | 146 | [game-render-hud ledger](game-render-hud-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [item.c](../../src/game/item.c) | 157 | [confirmation/item ledger](game-menu-confirm-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [lighting.c](../../src/game/lighting.c) | 108 | [complete lighting/fade ledger](game-lighting-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [lighting_presets.c](../../src/game/lighting_presets.c) | 1 | [complete lighting/fade ledger](game-lighting-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [magic.c](../../src/game/magic.c) | 9 | [complete ledger](game-magic-collision-literal-ledger.md); [reconciliation](collision-ledger-reconciliation.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [main.c](../../src/game/main.c) | 4 | [complete allocator/startup ledger](allocator-startup-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [map_event.c](../../src/game/map_event.c) | 28 | [game-map-event ledger](game-map-event-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [map_event_render.c](../../src/game/map_event_render.c) | 6 | [domain evidence](game-map-event-literals.md) | Expand the domain audit into current occurrence coverage. |
| [map_events.c](../../src/game/map_events.c) | 36 | [game-map-event ledger](game-map-event-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [map_load.c](../../src/game/map_load.c) | 53 | [game-map-event ledger](game-map-event-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [map_object.c](../../src/game/map_object.c) | 67 | [domain evidence](game-map-object-motion-constants.md); [current ledger](game-map-object-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [map_object_pool.c](../../src/game/map_object_pool.c) | 78 | [domain evidence](game-map-object-motion-constants.md); [current ledger](game-map-object-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [map_scripts.c](../../src/game/map_scripts.c) | 235 | [game-map-script ledger](game-map-script-literal-ledger.md) | Reconcile current expressions and referent evidence before restoring complete coverage. |
| [matrix.c](../../src/game/matrix.c) | 2 | [complete math ledger](math-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [matrix_rotation.c](../../src/game/matrix_rotation.c) | 82 | [complete math ledger](math-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [memory.c](../../src/game/memory.c) | 11 | [complete allocator/startup ledger](allocator-startup-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu.c](../../src/game/menu.c) | 79 | [complete ledger](menu-root-list-save-literal-ledger.md); [confirmation state](menu-confirm-state.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_config_panel.c](../../src/game/menu_config_panel.c) | 39 | [complete ledger](game-menu-config-literal-ledger.md); [option domain](game-player-option-domain.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_draw_name_list.c](../../src/game/menu_draw_name_list.c) | 0 | [complete equipment ledger](game-equipment-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_draw_stats_header.c](../../src/game/menu_draw_stats_header.c) | 104 | [status ledger](game-menu-status-literal-ledger.md); [layout evidence](game-status-text-layout.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_draw_status_details.c](../../src/game/menu_draw_status_details.c) | 216 | [status ledger](game-menu-status-literal-ledger.md); [layout evidence](game-status-text-layout.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_draw_window.c](../../src/game/menu_draw_window.c) | 3 | [complete ledger](menu-root-list-save-literal-ledger.md); [confirmation state](menu-confirm-state.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_enter_mode.c](../../src/game/menu_enter_mode.c) | 4 | [domain notes](source-constant-names.md#game-lifecycle-warp-modes-and-card-events) | Expand the domain audit into current occurrence coverage. |
| [menu_item_detail.c](../../src/game/menu_item_detail.c) | 87 | [complete detail/dialog ledger](game-menu-detail-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_item_drop.c](../../src/game/menu_item_drop.c) | 121 | [complete ledger](menu-root-list-save-literal-ledger.md); [confirmation state](menu-confirm-state.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_item_model_preview.c](../../src/game/menu_item_model_preview.c) | 36 | [complete inventory-preview ledger](game-menu-inventory-preview-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_list_interact.c](../../src/game/menu_list_interact.c) | 56 | [confirmation/item ledger](game-menu-confirm-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_list_render.c](../../src/game/menu_list_render.c) | 16 | [complete panel ledger](game-menu-panel-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_map_viewer.c](../../src/game/menu_map_viewer.c) | 75 | [complete panel ledger](game-menu-panel-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_panels.c](../../src/game/menu_panels.c) | 56 | [complete ledger](menu-root-list-save-literal-ledger.md); [confirmation state](menu-confirm-state.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_runtime.c](../../src/game/menu_runtime.c) | 100 | [complete runtime ledger](game-menu-runtime-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_select.c](../../src/game/menu_select.c) | 96 | [complete equipment ledger](game-equipment-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_status_panel.c](../../src/game/menu_status_panel.c) | 12 | [complete panel ledger](game-menu-panel-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_two_option_prompt.c](../../src/game/menu_two_option_prompt.c) | 27 | [confirmation/item ledger](game-menu-confirm-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [notify_queue.c](../../src/game/notify_queue.c) | 116 | [complete ledger](game-notification-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [player_core.c](../../src/game/player_core.c) | 74 | [complete ledger](game-player-core-literal-ledger.md); [sound slots](game-player-sound-slots.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [player_death.c](../../src/game/player_death.c) | 190 | [complete ledger](game-player-death-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [player_death_fade.c](../../src/game/player_death_fade.c) | 8 | [complete lighting/fade ledger](game-lighting-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [player_status_effect4.c](../../src/game/player_status_effect4.c) | 0 | [domain evidence](game-player-progression-literal-ledger.md) | No retained literal tokens. |
| [player_update.c](../../src/game/player_update.c) | 168 | [game-player-update ledger](game-player-update-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [player_use_item.c](../../src/game/player_use_item.c) | 42 | [complete ledger](game-item-use-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [player_warp.c](../../src/game/player_warp.c) | 96 | [warp/lifecycle ledger](game-warp-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [player_warp_shimmer_at_player.c](../../src/game/player_warp_shimmer_at_player.c) | 0 | [warp/lifecycle ledger](game-warp-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [pool.c](../../src/game/pool.c) | 35 | [complete cache/camera ledger](game-animation-cache-camera-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render.c](../../src/game/render.c) | 146 | [domain notes](source-constant-names.md#gpu-setup-system-screens-and-transition-fades) | Expand the domain audit into current occurrence coverage. |
| [render_enqueuers.c](../../src/game/render_enqueuers.c) | 95 | [domain notes](source-constant-names.md#tmd-packet-and-scene-depth-constants) | Expand the domain audit into current occurrence coverage. |
| [render_frame.c](../../src/game/render_frame.c) | 41 | [game-render-hud ledger](game-render-hud-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render_map_cells.c](../../src/game/render_map_cells.c) | 191 | [domain notes](source-constant-names.md#map-mesh-banks-and-visibility-classes) | Expand the domain audit into current occurrence coverage. |
| [render_scene.c](../../src/game/render_scene.c) | 14 | [game-render-hud ledger](game-render-hud-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render_sprite.c](../../src/game/render_sprite.c) | 1 | [domain notes](source-constant-names.md#tmd-packet-and-scene-depth-constants) | Expand the domain audit into current occurrence coverage. |
| [resources.c](../../src/game/resources.c) | 14 | [complete resource/audio ledger](resource-audio-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [save_system.c](../../src/game/save_system.c) | 135 | [complete ledger](game-save-system-literal-ledger.md); [status domain](game-save-status-domain.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [sprite_add_ft4.c](../../src/game/sprite_add_ft4.c) | 27 | [domain notes](source-constant-names.md#display-dimensions-and-ordering-table-bounds) | Expand the domain audit into current occurrence coverage. |

## OPEN.EXE

34 files; 1,291 occurrences.

| Source | Occurrences | Existing audit references | Scope to check |
| --- | ---: | --- | --- |
| [angle_mod_delta.c](../../src/open/angle_mod_delta.c) | 1 | [complete math ledger](math-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [audio.c](../../src/open/audio.c) | 23 | [complete audio/resource ledger](audio-sequence-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [audio_play_voice.c](../../src/open/audio_play_voice.c) | 6 | [complete resource/audio ledger](resource-audio-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [audio_spatial.c](../../src/open/audio_spatial.c) | 6 | [complete resource/audio ledger](resource-audio-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [camera_path.c](../../src/open/camera_path.c) | 3 | [complete cache/camera ledger](game-animation-cache-camera-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [debug.c](../../src/open/debug.c) | 0 | [complete formatter ledger](formatter-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [display_adjust.c](../../src/open/display_adjust.c) | 19 | [domain notes](source-constant-names.md#allocator-formatter-and-vram-viewer-plan) | Expand the domain audit into current occurrence coverage. |
| [entity_render.c](../../src/open/entity_render.c) | 78 | [open-render ledger](open-render-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [fixed_vector2_length.c](../../src/open/fixed_vector2_length.c) | 0 | [complete math ledger](math-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [format.c](../../src/open/format.c) | 43 | [complete formatter ledger](formatter-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [item.c](../../src/open/item.c) | 2 | [domain evidence](game-entity-sprite-tables.md) | Expand the domain audit into current occurrence coverage. |
| [main.c](../../src/open/main.c) | 2 | [complete allocator/startup ledger](allocator-startup-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [matrix.c](../../src/open/matrix.c) | 8 | [complete math ledger](math-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [matrix_rotation.c](../../src/open/matrix_rotation.c) | 87 | [complete math ledger](math-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [opening_controller.c](../../src/open/opening_controller.c) | 7 | [complete audio/resource ledger](audio-sequence-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [opening_entity_pool.c](../../src/open/opening_entity_pool.c) | 13 | [pool ledger](open-entity-pool-literal-ledger.md); [domain evidence](open-model-domain.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [opening_fade.c](../../src/open/opening_fade.c) | 24 | [domain notes](source-constant-names.md#gpu-setup-system-screens-and-transition-fades) | Expand the domain audit into current occurrence coverage. |
| [opening_helpers.c](../../src/open/opening_helpers.c) | 4 | [domain notes](source-constant-names.md#controller-mask-and-opening-input-plan) | Expand the domain audit into current occurrence coverage. |
| [opening_render.c](../../src/open/opening_render.c) | 24 | [open-render ledger](open-render-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [opening_render_entities.c](../../src/open/opening_render_entities.c) | 2 | [open-render ledger](open-render-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [opening_scene0_render.c](../../src/open/opening_scene0_render.c) | 0 | [open-render ledger](open-render-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [opening_scenes.c](../../src/open/opening_scenes.c) | 592 | [scene ledger](open-scene-literal-ledger.md); [domain evidence](open-scene-domains.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render.c](../../src/open/render.c) | 20 | [domain notes](source-constant-names.md#gpu-setup-system-screens-and-transition-fades) | Expand the domain audit into current occurrence coverage. |
| [render_init.c](../../src/open/render_init.c) | 154 | [complete initializer ledger](open-render-init-literal-ledger.md); [mode domain](overlay-mode-domains.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render_map.c](../../src/open/render_map.c) | 6 | [domain notes](source-constant-names.md#tmd-packet-and-scene-depth-constants) | Expand the domain audit into current occurrence coverage. |
| [render_map_cells.c](../../src/open/render_map_cells.c) | 8 | [domain notes](source-constant-names.md#map-mesh-banks-and-visibility-classes) | Expand the domain audit into current occurrence coverage. |
| [render_sprite.c](../../src/open/render_sprite.c) | 24 | [domain notes](source-constant-names.md#tmd-packet-and-scene-depth-constants) | Expand the domain audit into current occurrence coverage. |
| [render_tmd.c](../../src/open/render_tmd.c) | 48 | [domain notes](source-constant-names.md#tmd-packet-and-scene-depth-constants) | Expand the domain audit into current occurrence coverage. |
| [render_unlit.c](../../src/open/render_unlit.c) | 5 | [domain notes](source-constant-names.md#tmd-packet-and-scene-depth-constants) | Expand the domain audit into current occurrence coverage. |
| [resources.c](../../src/open/resources.c) | 31 | [complete audio/resource ledger](audio-sequence-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [sound_ref.c](../../src/open/sound_ref.c) | 0 | [complete resource/audio ledger](resource-audio-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [sprite_add_ft4.c](../../src/open/sprite_add_ft4.c) | 42 | [domain notes](source-constant-names.md#display-dimensions-and-ordering-table-bounds) | Expand the domain audit into current occurrence coverage. |
| [vector_math.c](../../src/open/vector_math.c) | 6 | [complete math ledger](math-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [vector_xz_to_angle.c](../../src/open/vector_xz_to_angle.c) | 3 | [complete math ledger](math-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |

## Vendored sources

3 files; 14 occurrences.

| Source | Occurrences | Existing audit references | Scope to check |
| --- | ---: | --- | --- |
| [game_libetc_intr_tail.c](../../src/vendor/game_libetc_intr_tail.c) | 0 | [domain notes](source-constant-names.md#controller-mask-and-opening-input-plan) | No retained literal tokens. |
| [game_libetc_pad.c](../../src/vendor/game_libetc_pad.c) | 7 | [domain notes](source-constant-names.md#controller-mask-and-opening-input-plan) | Expand the domain audit into current occurrence coverage. |
| [open_libetc_pad.c](../../src/vendor/open_libetc_pad.c) | 7 | [domain notes](source-constant-names.md#controller-mask-and-opening-input-plan) | Expand the domain audit into current occurrence coverage. |

## Completion boundary

The naming goal remains open. Completion requires evidence-supported names for
all unknown fields and all constants that warrant names, plus a reason for
every retained literal across
all source files. The unresolved field families above and the scope checks in
this map are outstanding work. A green compiler or unchanged runtime contents
proves neither semantic identity nor documentation completeness.
