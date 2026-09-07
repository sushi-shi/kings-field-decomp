# Source literal coverage map

Working snapshot after the [panel-geometry review](game-menu-panel-geometry.md):
**111 C files**, **112 source/image variants**,
and **6,903 inline numeric/character occurrences**. The counts include authored
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

For **23 files / 2,012 occurrences**, the explicit ledger rows were compared
against every current numeric/character token and its complete source-line
expression, including duplicate occurrences and initializers. All matched and
each row has a reason. Line-number drift is excluded from the comparison; source
expressions and multiplicity establish content coverage. These files are marked
below. This validates retained-literal accounting, not proof that every possible
semantic improvement has been exhausted.

The grouped player-death, item-use and notification ledgers were not classified
by that single-occurrence-row check. Their compressed rows require expansion;
unparsed rows do not mean undocumented values. The other existing domain audits
also remain outside this verified subtotal. No missing-name total can be derived
by subtracting this subtotal from the source census.

## Next coverage work

1. Refresh the status-text ledgers after their later reconstruction changes.
2. Audit the remaining warp motion/timing values against retail and complete
   per-function accounting for files currently covered only by domain notes.
3. Verify every retained occurrence against its reason before marking a file
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
| [main.c](../../src/psx/main.c) | 8 | [domain notes](source-constant-names.md#startup-store-destinations-and-initial-heaps) | Expand the domain audit into current occurrence coverage. |

## GAME.EXE

73 files; 5,757 occurrences.

| Source | Occurrences | Existing audit references | Scope to check |
| --- | ---: | --- | --- |
| [actor.c](../../src/game/actor.c) | 174 | [game-actor-core ledger](game-actor-core-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [actor_behavior.c](../../src/game/actor_behavior.c) | 173 | [game-actor-behavior ledger](game-actor-behavior-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [actor_pool.c](../../src/game/actor_pool.c) | 11 | [domain evidence](game-actor-constants.md) | Expand the domain audit into current occurrence coverage. |
| [asset_registry.c](../../src/game/asset_registry.c) | 1 | [domain notes](source-constant-names.md#animation-cache-camera-and-audio-plan) | Expand the domain audit into current occurrence coverage. |
| [audio.c](../../src/game/audio.c) | 45 | [domain notes](source-constant-names.md#animation-cache-camera-and-audio-plan) | Expand the domain audit into current occurrence coverage. |
| [camera_path.c](../../src/game/camera_path.c) | 1 | [domain notes](source-constant-names.md#animation-cache-camera-and-audio-plan) | Expand the domain audit into current occurrence coverage. |
| [cd_file.c](../../src/game/cd_file.c) | 35 | [domain notes](source-constant-names.md#cd-and-resource-registration-plan) | Expand the domain audit into current occurrence coverage. |
| [collision.c](../../src/game/collision.c) | 265 | [domain evidence](game-world-collision-literals.md) | Expand the domain audit into current occurrence coverage. |
| [collision_grid.c](../../src/game/collision_grid.c) | 5 | [domain evidence](game-world-collision-literals.md) | Expand the domain audit into current occurrence coverage. |
| [color_lerp_rgb555.c](../../src/game/color_lerp_rgb555.c) | 0 | [domain notes](source-constant-names.md#math-and-packed-color-batch) | No retained literal tokens. |
| [debug_text.c](../../src/game/debug_text.c) | 64 | [domain notes](source-constant-names.md#allocator-formatter-and-vram-viewer-plan) | Expand the domain audit into current occurrence coverage. |
| [display_play_transition.c](../../src/game/display_play_transition.c) | 29 | [domain notes](source-constant-names.md#gpu-setup-system-screens-and-transition-fades) | Expand the domain audit into current occurrence coverage. |
| [effect_dispatch.c](../../src/game/effect_dispatch.c) | 153 | [game-effect-dispatch ledger](game-effect-dispatch-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [effect_map_collision.c](../../src/game/effect_map_collision.c) | 43 | [game-magic-collision ledger](game-magic-collision-literal-ledger.md) | Check current per-function and initializer coverage; linked ledgers may cover only a subset. |
| [effect_pool.c](../../src/game/effect_pool.c) | 225 | [game-effect-pool ledger](game-effect-pool-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [effect_update.c](../../src/game/effect_update.c) | 105 | [game-effect-update ledger](game-effect-update-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [entity_model_render.c](../../src/game/entity_model_render.c) | 14 | [game-render-hud ledger](game-render-hud-literal-ledger.md); [game-menu-preview ledger](game-menu-preview-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [entity_render.c](../../src/game/entity_render.c) | 246 | [domain evidence](game-entity-sprite-tables.md) | Expand the domain audit into current occurrence coverage. |
| [equipment.c](../../src/game/equipment.c) | 7 | [game-equipment ledger](game-equipment-literal-ledger.md) | Check current per-function and initializer coverage; linked ledgers may cover only a subset. |
| [game.c](../../src/game/game.c) | 12 | [domain notes](source-constant-names.md#game-lifecycle-warp-modes-and-card-events) | Expand the domain audit into current occurrence coverage. |
| [geometry_render.c](../../src/game/geometry_render.c) | 147 | [game-render-hud ledger](game-render-hud-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [item.c](../../src/game/item.c) | 209 | [game-shop ledger](game-shop-literal-ledger.md); [game-item-database ledger](game-item-database-literal-ledger.md) | Check current per-function and initializer coverage; linked ledgers may cover only a subset. |
| [lighting.c](../../src/game/lighting.c) | 108 | [domain notes](source-constant-names.md#lighting-and-death-fade-plan) | Expand the domain audit into current occurrence coverage. |
| [lighting_presets.c](../../src/game/lighting_presets.c) | 1 | [domain notes](source-constant-names.md#lighting-and-death-fade-plan) | Expand the domain audit into current occurrence coverage. |
| [magic.c](../../src/game/magic.c) | 9 | [game-magic-collision ledger](game-magic-collision-literal-ledger.md); [game-spell ledger](game-spell-literal-ledger.md) | Check current per-function and initializer coverage; linked ledgers may cover only a subset. |
| [main.c](../../src/game/main.c) | 4 | [domain notes](source-constant-names.md#startup-store-destinations-and-initial-heaps) | Expand the domain audit into current occurrence coverage. |
| [map_event.c](../../src/game/map_event.c) | 28 | [game-map-event ledger](game-map-event-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [map_event_render.c](../../src/game/map_event_render.c) | 6 | [domain evidence](game-map-event-literals.md) | Expand the domain audit into current occurrence coverage. |
| [map_events.c](../../src/game/map_events.c) | 42 | [game-map-event ledger](game-map-event-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [map_load.c](../../src/game/map_load.c) | 59 | [game-map-event ledger](game-map-event-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [map_object.c](../../src/game/map_object.c) | 77 | [domain evidence](game-map-object-motion-constants.md) | Expand the domain audit into current occurrence coverage. |
| [map_object_pool.c](../../src/game/map_object_pool.c) | 93 | [domain evidence](game-map-object-motion-constants.md) | Expand the domain audit into current occurrence coverage. |
| [map_scripts.c](../../src/game/map_scripts.c) | 246 | [game-map-script ledger](game-map-script-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [matrix.c](../../src/game/matrix.c) | 2 | [domain notes](source-constant-names.md#math-and-packed-color-batch) | Expand the domain audit into current occurrence coverage. |
| [matrix_rotation.c](../../src/game/matrix_rotation.c) | 82 | [domain notes](source-constant-names.md#math-and-packed-color-batch) | Expand the domain audit into current occurrence coverage. |
| [memory.c](../../src/game/memory.c) | 11 | [domain notes](source-constant-names.md#allocator-formatter-and-vram-viewer-plan) | Expand the domain audit into current occurrence coverage. |
| [menu.c](../../src/game/menu.c) | 87 | [game-menu-choice ledger](game-menu-choice-literal-ledger.md); [game-menu-item-list ledger](game-menu-item-list-literal-ledger.md) | Check current per-function and initializer coverage; linked ledgers may cover only a subset. |
| [menu_config_panel.c](../../src/game/menu_config_panel.c) | 52 | [game-menu-choice ledger](game-menu-choice-literal-ledger.md) | Check current per-function and initializer coverage; linked ledgers may cover only a subset. |
| [menu_draw_name_list.c](../../src/game/menu_draw_name_list.c) | 9 | [game-equipment ledger](game-equipment-literal-ledger.md) | Check current per-function and initializer coverage; linked ledgers may cover only a subset. |
| [menu_draw_stats_header.c](../../src/game/menu_draw_stats_header.c) | 130 | [game-menu-status ledger](game-menu-status-literal-ledger.md) | Check current per-function and initializer coverage; linked ledgers may cover only a subset. |
| [menu_draw_status_details.c](../../src/game/menu_draw_status_details.c) | 283 | [game-menu-status ledger](game-menu-status-literal-ledger.md) | Historical ledger predates the signed-rating correction; refresh arithmetic occurrences. |
| [menu_draw_window.c](../../src/game/menu_draw_window.c) | 4 | [game-menu-window ledger](game-menu-window-literal-ledger.md) | Check current per-function and initializer coverage; linked ledgers may cover only a subset. |
| [menu_enter_mode.c](../../src/game/menu_enter_mode.c) | 4 | [domain notes](source-constant-names.md#game-lifecycle-warp-modes-and-card-events) | Expand the domain audit into current occurrence coverage. |
| [menu_item_detail.c](../../src/game/menu_item_detail.c) | 100 | [complete detail/dialog ledger](game-menu-detail-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_item_drop.c](../../src/game/menu_item_drop.c) | 145 | [game-menu-save ledger](game-menu-save-literal-ledger.md); [game-menu-item-list ledger](game-menu-item-list-literal-ledger.md) | Check current per-function and initializer coverage; linked ledgers may cover only a subset. |
| [menu_item_model_preview.c](../../src/game/menu_item_model_preview.c) | 45 | [game-menu-preview ledger](game-menu-preview-literal-ledger.md) | Check current per-function and initializer coverage; linked ledgers may cover only a subset. |
| [menu_list_interact.c](../../src/game/menu_list_interact.c) | 67 | [domain evidence](game-menu-list-control-flow.md) | Expand the domain audit into current occurrence coverage. |
| [menu_list_render.c](../../src/game/menu_list_render.c) | 16 | [complete panel ledger](game-menu-panel-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_map_viewer.c](../../src/game/menu_map_viewer.c) | 75 | [complete panel ledger](game-menu-panel-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_panels.c](../../src/game/menu_panels.c) | 64 | [game-menu-choice ledger](game-menu-choice-literal-ledger.md); [game-spell ledger](game-spell-literal-ledger.md) | Check current per-function and initializer coverage; linked ledgers may cover only a subset. |
| [menu_runtime.c](../../src/game/menu_runtime.c) | 103 | [complete runtime ledger](game-menu-runtime-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_select.c](../../src/game/menu_select.c) | 104 | [game-equipment ledger](game-equipment-literal-ledger.md) | Check current per-function and initializer coverage; linked ledgers may cover only a subset. |
| [menu_status_panel.c](../../src/game/menu_status_panel.c) | 12 | [complete panel ledger](game-menu-panel-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_two_option_prompt.c](../../src/game/menu_two_option_prompt.c) | 41 | [game-menu-save ledger](game-menu-save-literal-ledger.md) | Check current per-function and initializer coverage; linked ledgers may cover only a subset. |
| [notify_queue.c](../../src/game/notify_queue.c) | 116 | [game-notification ledger](game-notification-literal-ledger.md) | Check current per-function and initializer coverage; linked ledgers may cover only a subset. |
| [player_core.c](../../src/game/player_core.c) | 80 | [game-player-core ledger](game-player-core-literal-ledger.md); [game-player-motion ledger](game-player-motion-literal-ledger.md) | Recent movement correction updated its ledger; include that combined core/motion ledger in the next coverage validation. |
| [player_death.c](../../src/game/player_death.c) | 197 | [game-item-database ledger](game-item-database-literal-ledger.md); [game-player-death ledger](game-player-death-literal-ledger.md); [game-player-progression ledger](game-player-progression-literal-ledger.md) | Check current per-function and initializer coverage; linked ledgers may cover only a subset. |
| [player_death_fade.c](../../src/game/player_death_fade.c) | 8 | [domain notes](source-constant-names.md#lighting-and-death-fade-plan) | Expand the domain audit into current occurrence coverage. |
| [player_status_effect4.c](../../src/game/player_status_effect4.c) | 0 | [domain evidence](game-player-progression-literal-ledger.md) | No retained literal tokens. |
| [player_update.c](../../src/game/player_update.c) | 168 | [game-player-update ledger](game-player-update-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [player_use_item.c](../../src/game/player_use_item.c) | 79 | [game-item-use ledger](game-item-use-literal-ledger.md) | Check current per-function and initializer coverage; linked ledgers may cover only a subset. |
| [player_warp.c](../../src/game/player_warp.c) | 113 | [domain notes](source-constant-names.md#game-lifecycle-warp-modes-and-card-events) | Domain note covers states; audit remaining motion/timing and authored coordinate uses per function. |
| [player_warp_shimmer_at_player.c](../../src/game/player_warp_shimmer_at_player.c) | 0 | [domain evidence](semantic-field-names.md) | No retained literal tokens. |
| [pool.c](../../src/game/pool.c) | 35 | [domain notes](source-constant-names.md#animation-cache-camera-and-audio-plan) | Expand the domain audit into current occurrence coverage. |
| [render.c](../../src/game/render.c) | 146 | [domain notes](source-constant-names.md#gpu-setup-system-screens-and-transition-fades) | Expand the domain audit into current occurrence coverage. |
| [render_enqueuers.c](../../src/game/render_enqueuers.c) | 95 | [domain notes](source-constant-names.md#tmd-packet-and-scene-depth-constants) | Expand the domain audit into current occurrence coverage. |
| [render_frame.c](../../src/game/render_frame.c) | 47 | [game-render-hud ledger](game-render-hud-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render_map_cells.c](../../src/game/render_map_cells.c) | 191 | [domain notes](source-constant-names.md#map-mesh-banks-and-visibility-classes) | Expand the domain audit into current occurrence coverage. |
| [render_scene.c](../../src/game/render_scene.c) | 15 | [game-render-hud ledger](game-render-hud-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render_sprite.c](../../src/game/render_sprite.c) | 1 | [domain notes](source-constant-names.md#tmd-packet-and-scene-depth-constants) | Expand the domain audit into current occurrence coverage. |
| [resources.c](../../src/game/resources.c) | 17 | [domain notes](source-constant-names.md#cd-and-resource-registration-plan) | Expand the domain audit into current occurrence coverage. |
| [save_system.c](../../src/game/save_system.c) | 154 | [domain notes](source-constant-names.md#save-statuses-messages-and-io-attempts) | Expand the domain audit into current occurrence coverage. |
| [sprite_add_ft4.c](../../src/game/sprite_add_ft4.c) | 27 | [domain notes](source-constant-names.md#display-dimensions-and-ordering-table-bounds) | Expand the domain audit into current occurrence coverage. |

## OPEN.EXE

34 files; 1,319 occurrences.

| Source | Occurrences | Existing audit references | Scope to check |
| --- | ---: | --- | --- |
| [angle_mod_delta.c](../../src/open/angle_mod_delta.c) | 1 | [domain notes](source-constant-names.md#math-and-packed-color-batch) | Expand the domain audit into current occurrence coverage. |
| [audio.c](../../src/open/audio.c) | 31 | [domain notes](source-constant-names.md#animation-cache-camera-and-audio-plan) | Expand the domain audit into current occurrence coverage. |
| [audio_play_voice.c](../../src/open/audio_play_voice.c) | 6 | [domain notes](source-constant-names.md#animation-cache-camera-and-audio-plan) | Expand the domain audit into current occurrence coverage. |
| [audio_spatial.c](../../src/open/audio_spatial.c) | 6 | [domain notes](source-constant-names.md#animation-cache-camera-and-audio-plan) | Expand the domain audit into current occurrence coverage. |
| [camera_path.c](../../src/open/camera_path.c) | 3 | [domain notes](source-constant-names.md#animation-cache-camera-and-audio-plan) | Expand the domain audit into current occurrence coverage. |
| [debug.c](../../src/open/debug.c) | 0 | [domain notes](source-constant-names.md#allocator-formatter-and-vram-viewer-plan) | No retained literal tokens. |
| [display_adjust.c](../../src/open/display_adjust.c) | 19 | [domain notes](source-constant-names.md#allocator-formatter-and-vram-viewer-plan) | Expand the domain audit into current occurrence coverage. |
| [entity_render.c](../../src/open/entity_render.c) | 84 | [open-render ledger](open-render-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [fixed_vector2_length.c](../../src/open/fixed_vector2_length.c) | 0 | [domain notes](source-constant-names.md#math-and-packed-color-batch) | No retained literal tokens. |
| [format.c](../../src/open/format.c) | 62 | [domain notes](source-constant-names.md#allocator-formatter-and-vram-viewer-plan) | Expand the domain audit into current occurrence coverage. |
| [item.c](../../src/open/item.c) | 2 | [domain evidence](game-entity-sprite-tables.md) | Expand the domain audit into current occurrence coverage. |
| [main.c](../../src/open/main.c) | 2 | [domain notes](source-constant-names.md#startup-store-destinations-and-initial-heaps) | Expand the domain audit into current occurrence coverage. |
| [matrix.c](../../src/open/matrix.c) | 8 | [domain notes](source-constant-names.md#math-and-packed-color-batch) | Expand the domain audit into current occurrence coverage. |
| [matrix_rotation.c](../../src/open/matrix_rotation.c) | 87 | [domain notes](source-constant-names.md#math-and-packed-color-batch) | Expand the domain audit into current occurrence coverage. |
| [opening_controller.c](../../src/open/opening_controller.c) | 9 | [domain notes](source-constant-names.md#controller-mask-and-opening-input-plan) | Expand the domain audit into current occurrence coverage. |
| [opening_entity_pool.c](../../src/open/opening_entity_pool.c) | 13 | [domain evidence](semantic-field-names.md) | Expand the domain audit into current occurrence coverage. |
| [opening_fade.c](../../src/open/opening_fade.c) | 24 | [domain notes](source-constant-names.md#gpu-setup-system-screens-and-transition-fades) | Expand the domain audit into current occurrence coverage. |
| [opening_helpers.c](../../src/open/opening_helpers.c) | 4 | [domain notes](source-constant-names.md#controller-mask-and-opening-input-plan) | Expand the domain audit into current occurrence coverage. |
| [opening_render.c](../../src/open/opening_render.c) | 24 | [open-render ledger](open-render-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [opening_render_entities.c](../../src/open/opening_render_entities.c) | 2 | [open-render ledger](open-render-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [opening_scene0_render.c](../../src/open/opening_scene0_render.c) | 0 | [open-render ledger](open-render-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [opening_scenes.c](../../src/open/opening_scenes.c) | 595 | [domain notes](source-constant-names.md#later-opening-scenes-and-ending-sequence) | Grouped audit covers authored paths/panels and runtime roles; current count is 595 after assertion removal. |
| [render.c](../../src/open/render.c) | 20 | [domain notes](source-constant-names.md#gpu-setup-system-screens-and-transition-fades) | Expand the domain audit into current occurrence coverage. |
| [render_init.c](../../src/open/render_init.c) | 154 | [domain notes](source-constant-names.md#gpu-setup-system-screens-and-transition-fades) | Expand the domain audit into current occurrence coverage. |
| [render_map.c](../../src/open/render_map.c) | 6 | [domain notes](source-constant-names.md#tmd-packet-and-scene-depth-constants) | Expand the domain audit into current occurrence coverage. |
| [render_map_cells.c](../../src/open/render_map_cells.c) | 8 | [domain notes](source-constant-names.md#map-mesh-banks-and-visibility-classes) | Expand the domain audit into current occurrence coverage. |
| [render_sprite.c](../../src/open/render_sprite.c) | 24 | [domain notes](source-constant-names.md#tmd-packet-and-scene-depth-constants) | Expand the domain audit into current occurrence coverage. |
| [render_tmd.c](../../src/open/render_tmd.c) | 36 | [domain notes](source-constant-names.md#tmd-packet-and-scene-depth-constants) | Expand the domain audit into current occurrence coverage. |
| [render_unlit.c](../../src/open/render_unlit.c) | 4 | [domain notes](source-constant-names.md#tmd-packet-and-scene-depth-constants) | Expand the domain audit into current occurrence coverage. |
| [resources.c](../../src/open/resources.c) | 34 | [domain notes](source-constant-names.md#cd-and-resource-registration-plan) | Expand the domain audit into current occurrence coverage. |
| [sound_ref.c](../../src/open/sound_ref.c) | 0 | [domain notes](source-constant-names.md#animation-cache-camera-and-audio-plan) | No retained literal tokens. |
| [sprite_add_ft4.c](../../src/open/sprite_add_ft4.c) | 42 | [domain notes](source-constant-names.md#display-dimensions-and-ordering-table-bounds) | Expand the domain audit into current occurrence coverage. |
| [vector_math.c](../../src/open/vector_math.c) | 6 | [domain notes](source-constant-names.md#math-and-packed-color-batch) | Expand the domain audit into current occurrence coverage. |
| [vector_xz_to_angle.c](../../src/open/vector_xz_to_angle.c) | 3 | [domain notes](source-constant-names.md#math-and-packed-color-batch) | Expand the domain audit into current occurrence coverage. |

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
