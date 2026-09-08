# Source literal coverage map

Working snapshot after the [sprite-table and staff-phase review](game-entity-sprite-tables.md#table-count-and-staff-phase-naming-follow-up):
**111 C files**, **112 source/image variants**, and
**5,800 inline numeric/character occurrences**. Counts include authored
initializers, local extents, component indices, booleans and arithmetic. They
exclude comments, string contents, identifier digits, named enum/macro definitions
and ADDRESS/DATA/RODATA claims. These are inventory counts, **not missing-name
counts**. Vendor sources are included for source coverage, not game progress.

## Current ledger coverage verified

For **all 111 files / 5,800 occurrences**, explicit ledger rows were compared
against every current numeric/character token and its complete source-line
expression, including duplicate occurrences and initializers. All matched and
each row has a reason. Line-number drift is excluded from the comparison;
expressions and multiplicity establish content coverage. The tables below link
the current ledgers. Zero means no retained literal tokens under this census;
it does not establish complete semantic identities or binary matching.

The final file reviews add the 146-occurrence
[GAME renderer setup ledger](game-render-setup-literal-ledger.md) and reconcile
all 211 current [map-script occurrences](game-map-script-literal-ledger.md).
The latter also names the two image-group selectors and the shop sequence index.
The subsequent floor-item review names both zero-facing billboard predicates.
The animation/turn review types two map-event fields through all their uses and
names eight raw clip/control values. The character review adds a byte ID domain
through map records and dialogue, names three exchange selectors and preserves
the integer representation at filename and shop boundaries. Shipped placement
evidence then resolves character 12 and link 55 as the floor-3 dialogue door
unlock, removing two more raw selectors. The progress-link review names six
weapon-door and boss-emitter link uses from shipped placements and consumers.
The map-copy review names eight region selections and propagates their byte
enum through the helper and its dynamic object-parameter boundary.
The effect-kind review carries the existing identities through byte storage,
all 33 current constructor calls, dispatch, weapon effects and masked actor
profile selection; its explicit numeric boundaries retain the same literal count.
The stock-bank review names 62 direct player/shop selectors and four price-column
origins, removing 66 more raw occurrences while retaining quantity values.
The floor-sprite review corrects the misleading `item_id` field to
`base_sprite_index` in both placement and runtime records; its halfword stream
terminator is separate from inventory item IDs. No literal counts change.
The inventory review carries seventy existing item names through seven equipment
fields, five byte lists and nine item-specific interfaces. Integer loop/control
values and shared menu payloads use explicit boundaries; the six word-argument
helpers retain their legacy C parameter widths. Literal counts remain unchanged.
The save-overlay review names five raw selectors and carries the signed domain
through the renderer, confirmation prompt and every save/load cursor boundary.
The shop-ID review types the optional shop byte and four shop-specific word
interfaces, with explicit price-index and shared-widget boundaries. Its existing
table-order identities and constant values leave the literal count unchanged.
The sound-cue review carries three feedback identities through the helper and
90 callers, and names the common channel volume, removing two raw occurrences.
The save-slot review types directory tags and read/write IDs while retaining
byte storage and signed-halfword parameters. Naming the one-based origin in
two menu calls and the catalogue reader removes three more raw occurrences.
The ground-effect review names both child-spawn helpers and carries the branch
role through its halfword field, word helper argument and variadic boundaries.
Its existing role values and positional reads leave literal counts unchanged.
The homing review types the byte selector through both constructor cases and
steering comparisons, retaining the callers' encoded mixed-word payloads and
naming the shared direction-randomization label. Literal counts are unchanged.
The actor effect-slot review names three dispatcher arguments and carries the
word-sized slot enum through selection and spawning. Ledger reconciliation also
accounts for an intervening shared shop-model-load branch (one fewer literal)
and word-preserving weapon/effect expressions.
The alternate effect-kind review names five constructor variants across fifteen
uses, based on their explicit normalization to existing behavior kinds.
The effect resource review names the final two used kind IDs, all constructor
billboard/model selectors and first-clip choices, plus three dispatch selector
comparisons. Together these replace 93 raw occurrences.
The packed-collision review names 28 kind-shift, detail-mask and ceiling-code
occurrences while preserving the original high-half ceiling comparison.
The transformation review names its 64 blend intervals and 40-unit Y step,
removing six raw occurrences while retaining both endpoint updates.
The status-rating review names eight attack/defense formula weights, retaining
the original unsigned subtotal, signed divisions and truncation points.
The player combat-tuning review names starting gold/defense, seven equipment
effects and two damage weights, removing sixteen raw occurrences.
The combat-component review names weapon/actor array indices and the actor
damage denominator multiplier, removing fourteen raw occurrences.
The representation review names charge scaling, spawn-sequence wraparound,
warp-model scale and packed audio notes, removing eight raw occurrences.
The renderer follow-up names three sprite-table bounds and the two staff
remapping phase values, removing five raw occurrences.
Historical ledgers and domain notes may describe earlier source, so their old
counts are not current totals. This comparison establishes literal accounting,
not proof that every possible semantic improvement has been exhausted.
Builds, compiler checks, tests and post-edit matches remain deferred until the
naming pass finishes, as requested.

## Remaining semantic work

1. Investigate the unresolved field families below using actual consumers and
   resource evidence; serialization or a reset alone does not establish a name.
2. Continue enum propagation through selector fields, locals and APIs, preserving
   the encoded widths and explicit boundaries used by the retail code.
3. Review the semantic adequacy of retained-literal reasons and unresolved
   resource selectors. A reason being present does not prove that a value should
   remain unnamed; promote it when evidence establishes a useful domain or role.

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

73 files; 4,579 occurrences.

| Source | Occurrences | Existing audit references | Scope to check |
| --- | ---: | --- | --- |
| [actor.c](../../src/game/actor.c) | 162 | [game-actor-core ledger](game-actor-core-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [actor_behavior.c](../../src/game/actor_behavior.c) | 145 | [game-actor-behavior ledger](game-actor-behavior-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [actor_pool.c](../../src/game/actor_pool.c) | 6 | [complete actor-pool ledger](actor-pool-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [asset_registry.c](../../src/game/asset_registry.c) | 1 | [complete cache/camera ledger](game-animation-cache-camera-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [audio.c](../../src/game/audio.c) | 38 | [complete audio/resource ledger](audio-sequence-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [camera_path.c](../../src/game/camera_path.c) | 1 | [complete cache/camera ledger](game-animation-cache-camera-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [cd_file.c](../../src/game/cd_file.c) | 35 | [complete resource/audio ledger](resource-audio-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [collision.c](../../src/game/collision.c) | 265 | [complete ledger](game-world-collision-literal-ledger.md); [reconciliation](collision-ledger-reconciliation.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [collision_grid.c](../../src/game/collision_grid.c) | 5 | [complete ledger](game-world-collision-literal-ledger.md); [reconciliation](collision-ledger-reconciliation.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [color_lerp_rgb555.c](../../src/game/color_lerp_rgb555.c) | 0 | [complete math ledger](math-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [debug_text.c](../../src/game/debug_text.c) | 45 | [complete formatter ledger](formatter-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [display_play_transition.c](../../src/game/display_play_transition.c) | 20 | [complete fade/quad ledger](fade-quad-literal-ledger.md); [color types](quad-color-types.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [effect_dispatch.c](../../src/game/effect_dispatch.c) | 134 | [game-effect-dispatch ledger](game-effect-dispatch-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [effect_map_collision.c](../../src/game/effect_map_collision.c) | 43 | [complete ledger](game-magic-collision-literal-ledger.md); [reconciliation](collision-ledger-reconciliation.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [effect_pool.c](../../src/game/effect_pool.c) | 138 | [game-effect-pool ledger](game-effect-pool-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [effect_update.c](../../src/game/effect_update.c) | 95 | [game-effect-update ledger](game-effect-update-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [entity_model_render.c](../../src/game/entity_model_render.c) | 14 | [game-render-hud ledger](game-render-hud-literal-ledger.md); [game-menu-preview ledger](game-menu-preview-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [entity_render.c](../../src/game/entity_render.c) | 241 | [complete renderer ledger](entity-render-literal-ledger.md); [review](renderer-literal-review.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [equipment.c](../../src/game/equipment.c) | 6 | [complete equipment ledger](game-equipment-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [game.c](../../src/game/game.c) | 12 | [warp/lifecycle ledger](game-warp-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [geometry_render.c](../../src/game/geometry_render.c) | 146 | [game-render-hud ledger](game-render-hud-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [item.c](../../src/game/item.c) | 146 | [confirmation/item ledger](game-menu-confirm-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [lighting.c](../../src/game/lighting.c) | 108 | [complete lighting/fade ledger](game-lighting-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [lighting_presets.c](../../src/game/lighting_presets.c) | 1 | [complete lighting/fade ledger](game-lighting-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [magic.c](../../src/game/magic.c) | 9 | [complete ledger](game-magic-collision-literal-ledger.md); [reconciliation](collision-ledger-reconciliation.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [main.c](../../src/game/main.c) | 4 | [complete allocator/startup ledger](allocator-startup-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [map_event.c](../../src/game/map_event.c) | 26 | [game-map-event ledger](game-map-event-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [map_event_render.c](../../src/game/map_event_render.c) | 6 | [complete renderer ledger](entity-render-literal-ledger.md); [review](renderer-literal-review.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [map_events.c](../../src/game/map_events.c) | 33 | [game-map-event ledger](game-map-event-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [map_load.c](../../src/game/map_load.c) | 42 | [game-map-event ledger](game-map-event-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [map_object.c](../../src/game/map_object.c) | 63 | [domain evidence](game-map-object-motion-constants.md); [current ledger](game-map-object-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [map_object_pool.c](../../src/game/map_object_pool.c) | 78 | [domain evidence](game-map-object-motion-constants.md); [current ledger](game-map-object-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [map_scripts.c](../../src/game/map_scripts.c) | 211 | [game-map-script ledger](game-map-script-literal-ledger.md); [image groups](map-image-groups.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [matrix.c](../../src/game/matrix.c) | 2 | [complete math ledger](math-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [matrix_rotation.c](../../src/game/matrix_rotation.c) | 82 | [complete math ledger](math-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [memory.c](../../src/game/memory.c) | 11 | [complete allocator/startup ledger](allocator-startup-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu.c](../../src/game/menu.c) | 77 | [complete ledger](menu-root-list-save-literal-ledger.md); [confirmation state](menu-confirm-state.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_config_panel.c](../../src/game/menu_config_panel.c) | 39 | [complete ledger](game-menu-config-literal-ledger.md); [option domain](game-player-option-domain.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_draw_name_list.c](../../src/game/menu_draw_name_list.c) | 0 | [complete equipment ledger](game-equipment-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_draw_stats_header.c](../../src/game/menu_draw_stats_header.c) | 104 | [status ledger](game-menu-status-literal-ledger.md); [layout evidence](game-status-text-layout.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_draw_status_details.c](../../src/game/menu_draw_status_details.c) | 208 | [status ledger](game-menu-status-literal-ledger.md); [layout evidence](game-status-text-layout.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_draw_window.c](../../src/game/menu_draw_window.c) | 3 | [complete ledger](menu-root-list-save-literal-ledger.md); [confirmation state](menu-confirm-state.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_enter_mode.c](../../src/game/menu_enter_mode.c) | 2 | [complete small-module ledger](small-module-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_item_detail.c](../../src/game/menu_item_detail.c) | 82 | [complete detail/dialog ledger](game-menu-detail-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_item_drop.c](../../src/game/menu_item_drop.c) | 118 | [complete ledger](menu-root-list-save-literal-ledger.md); [confirmation state](menu-confirm-state.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_item_model_preview.c](../../src/game/menu_item_model_preview.c) | 35 | [complete inventory-preview ledger](game-menu-inventory-preview-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_list_interact.c](../../src/game/menu_list_interact.c) | 56 | [confirmation/item ledger](game-menu-confirm-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_list_render.c](../../src/game/menu_list_render.c) | 16 | [complete panel ledger](game-menu-panel-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_map_viewer.c](../../src/game/menu_map_viewer.c) | 75 | [complete panel ledger](game-menu-panel-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_panels.c](../../src/game/menu_panels.c) | 56 | [complete ledger](menu-root-list-save-literal-ledger.md); [confirmation state](menu-confirm-state.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_runtime.c](../../src/game/menu_runtime.c) | 100 | [complete runtime ledger](game-menu-runtime-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_select.c](../../src/game/menu_select.c) | 95 | [complete equipment ledger](game-equipment-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_status_panel.c](../../src/game/menu_status_panel.c) | 12 | [complete panel ledger](game-menu-panel-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [menu_two_option_prompt.c](../../src/game/menu_two_option_prompt.c) | 26 | [confirmation/item ledger](game-menu-confirm-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [notify_queue.c](../../src/game/notify_queue.c) | 92 | [complete ledger](game-notification-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [player_core.c](../../src/game/player_core.c) | 74 | [complete ledger](game-player-core-literal-ledger.md); [sound slots](game-player-sound-slots.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [player_death.c](../../src/game/player_death.c) | 134 | [complete ledger](game-player-death-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [player_death_fade.c](../../src/game/player_death_fade.c) | 8 | [complete lighting/fade ledger](game-lighting-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [player_status_effect4.c](../../src/game/player_status_effect4.c) | 0 | [complete small-module ledger](small-module-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [player_update.c](../../src/game/player_update.c) | 168 | [game-player-update ledger](game-player-update-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [player_use_item.c](../../src/game/player_use_item.c) | 39 | [complete ledger](game-item-use-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [player_warp.c](../../src/game/player_warp.c) | 90 | [warp/lifecycle ledger](game-warp-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [player_warp_shimmer_at_player.c](../../src/game/player_warp_shimmer_at_player.c) | 0 | [warp/lifecycle ledger](game-warp-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [pool.c](../../src/game/pool.c) | 35 | [complete cache/camera ledger](game-animation-cache-camera-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render.c](../../src/game/render.c) | 146 | [complete renderer setup ledger](game-render-setup-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render_enqueuers.c](../../src/game/render_enqueuers.c) | 94 | [complete renderer ledger](renderer-packet-literal-ledger.md); [review](renderer-literal-review.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render_frame.c](../../src/game/render_frame.c) | 35 | [game-render-hud ledger](game-render-hud-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render_map_cells.c](../../src/game/render_map_cells.c) | 20 | [complete map-cell ledger](map-cell-literal-ledger.md); [visibility domain](visibility-screen-domains.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render_scene.c](../../src/game/render_scene.c) | 10 | [game-render-hud ledger](game-render-hud-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render_sprite.c](../../src/game/render_sprite.c) | 0 | [complete renderer ledger](entity-render-literal-ledger.md); [review](renderer-literal-review.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [resources.c](../../src/game/resources.c) | 14 | [complete resource/audio ledger](resource-audio-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [save_system.c](../../src/game/save_system.c) | 130 | [complete ledger](game-save-system-literal-ledger.md); [status domain](game-save-status-domain.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [sprite_add_ft4.c](../../src/game/sprite_add_ft4.c) | 12 | [complete fade/quad ledger](fade-quad-literal-ledger.md); [color types](quad-color-types.md) | Current token/expression coverage verified; every occurrence has a reason. |

## OPEN.EXE

34 files; 1,199 occurrences.

| Source | Occurrences | Existing audit references | Scope to check |
| --- | ---: | --- | --- |
| [angle_mod_delta.c](../../src/open/angle_mod_delta.c) | 1 | [complete math ledger](math-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [audio.c](../../src/open/audio.c) | 23 | [complete audio/resource ledger](audio-sequence-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [audio_play_voice.c](../../src/open/audio_play_voice.c) | 6 | [complete resource/audio ledger](resource-audio-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [audio_spatial.c](../../src/open/audio_spatial.c) | 5 | [complete resource/audio ledger](resource-audio-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [camera_path.c](../../src/open/camera_path.c) | 3 | [complete cache/camera ledger](game-animation-cache-camera-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [debug.c](../../src/open/debug.c) | 0 | [complete formatter ledger](formatter-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [display_adjust.c](../../src/open/display_adjust.c) | 19 | [complete small-module ledger](small-module-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [entity_render.c](../../src/open/entity_render.c) | 75 | [open-render ledger](open-render-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [fixed_vector2_length.c](../../src/open/fixed_vector2_length.c) | 0 | [complete math ledger](math-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [format.c](../../src/open/format.c) | 43 | [complete formatter ledger](formatter-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [item.c](../../src/open/item.c) | 1 | [complete small-module ledger](small-module-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [main.c](../../src/open/main.c) | 2 | [complete allocator/startup ledger](allocator-startup-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [matrix.c](../../src/open/matrix.c) | 8 | [complete math ledger](math-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [matrix_rotation.c](../../src/open/matrix_rotation.c) | 87 | [complete math ledger](math-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [opening_controller.c](../../src/open/opening_controller.c) | 7 | [complete audio/resource ledger](audio-sequence-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [opening_entity_pool.c](../../src/open/opening_entity_pool.c) | 13 | [pool ledger](open-entity-pool-literal-ledger.md); [domain evidence](open-model-domain.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [opening_fade.c](../../src/open/opening_fade.c) | 15 | [complete fade/quad ledger](fade-quad-literal-ledger.md); [color types](quad-color-types.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [opening_helpers.c](../../src/open/opening_helpers.c) | 4 | [complete small-module ledger](small-module-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [opening_render.c](../../src/open/opening_render.c) | 0 | [open-render ledger](open-render-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [opening_render_entities.c](../../src/open/opening_render_entities.c) | 2 | [open-render ledger](open-render-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [opening_scene0_render.c](../../src/open/opening_scene0_render.c) | 0 | [open-render ledger](open-render-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [opening_scenes.c](../../src/open/opening_scenes.c) | 570 | [scene ledger](open-scene-literal-ledger.md); [domain evidence](open-scene-domains.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render.c](../../src/open/render.c) | 19 | [complete small-module ledger](small-module-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render_init.c](../../src/open/render_init.c) | 154 | [complete initializer ledger](open-render-init-literal-ledger.md); [mode domain](overlay-mode-domains.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render_map.c](../../src/open/render_map.c) | 6 | [complete renderer ledger](renderer-packet-literal-ledger.md); [review](renderer-literal-review.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render_map_cells.c](../../src/open/render_map_cells.c) | 8 | [complete map-cell ledger](map-cell-literal-ledger.md); [visibility domain](visibility-screen-domains.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render_sprite.c](../../src/open/render_sprite.c) | 23 | [complete sprite ledger](open-sprite-literal-ledger.md); [cue domain](sprite-cue-tmd-slots.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render_tmd.c](../../src/open/render_tmd.c) | 48 | [complete renderer ledger](renderer-packet-literal-ledger.md); [review](renderer-literal-review.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [render_unlit.c](../../src/open/render_unlit.c) | 5 | [complete renderer ledger](renderer-packet-literal-ledger.md); [review](renderer-literal-review.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [resources.c](../../src/open/resources.c) | 31 | [complete audio/resource ledger](audio-sequence-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [sound_ref.c](../../src/open/sound_ref.c) | 0 | [complete resource/audio ledger](resource-audio-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [sprite_add_ft4.c](../../src/open/sprite_add_ft4.c) | 12 | [complete fade/quad ledger](fade-quad-literal-ledger.md); [color types](quad-color-types.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [vector_math.c](../../src/open/vector_math.c) | 6 | [complete math ledger](math-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [vector_xz_to_angle.c](../../src/open/vector_xz_to_angle.c) | 3 | [complete math ledger](math-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |

## Vendored sources

3 files; 14 occurrences.

| Source | Occurrences | Existing audit references | Scope to check |
| --- | ---: | --- | --- |
| [game_libetc_intr_tail.c](../../src/vendor/game_libetc_intr_tail.c) | 0 | [complete small-module ledger](small-module-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [game_libetc_pad.c](../../src/vendor/game_libetc_pad.c) | 7 | [complete small-module ledger](small-module-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |
| [open_libetc_pad.c](../../src/vendor/open_libetc_pad.c) | 7 | [complete small-module ledger](small-module-literal-ledger.md) | Current token/expression coverage verified; every occurrence has a reason. |

## Completion boundary

The naming goal remains open. Completion requires evidence-supported names for
all unknown fields and all constants that warrant names, plus a reason for
every retained literal across all source files. Current occurrence accounting
is complete; the unresolved field families, further type propagation and semantic
adequacy review above remain outstanding work. A green compiler or unchanged runtime contents
proves neither semantic identity nor documentation completeness.
