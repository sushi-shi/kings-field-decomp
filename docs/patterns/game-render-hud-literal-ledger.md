# GAME HUD and rendering retained-literal ledger

Companion to [the evidence and verification note](game-render-hud-constants.md).
Every remaining numeric/character occurrence in the four reviewed C files has
a row. Comments, strings, identifier digits, named enum/macro definitions and
ADDRESS/DATA/RODATA claims are excluded. Repeated tokens are listed separately
in source order; unary minus belongs to the context. This ledger documents
retained values and explicitly identifies unresolved type/behavior debt.

The [sprite-state review](game-hud-sprite-state.md) types both state fields
and names the effect table state values, compass slot and bound.

## `src/game/entity_model_render.c`

14 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `render_actor` | 50 | `0` | `object = tmd_get_object(0);` | First object within the selected TMD resource; local array origin, not a global asset ID. |
| `render_actor` | 53 | `0` | `actor->animation_phase, object->vertex_count) == 0) {` | Animation-binding boolean success/failure predicate; preserve exact zero comparison. |
| `render_actor` | 54 | `0` | `tmd_select_object_vertices(0);` | First object within the selected TMD resource; local array origin, not a global asset ID. |
| `render_actor` | 55 | `0` | `tmd_project_vertices(tmd_get_object(0)->vertex_count);` | First object within the selected TMD resource; local array origin, not a global asset ID. |
| `render_actor` | 61 | `0` | `if (descriptor-- == 0) {` | High-nibble zero selects the native TMD material; preserve the postdecrement and byte narrowing of the one-based cache selector. |
| `render_actor` | 62 | `0` | `render_enqueue_tmd(0, 0);` | First object within the selected TMD resource. |
| `render_actor` | 62 | `0` | `render_enqueue_tmd(0, 0);` | Zero extra ordering-table depth bias. |
| `render_actor` | 66 | `0` | `render_enqueue_model(0, 0);` | First object within the selected TMD resource. |
| `render_actor` | 66 | `0` | `render_enqueue_model(0, 0);` | Zero extra ordering-table depth bias. |
| `render_map_object` | 99 | `3` | `case KF_ENUM_DECODE(KfMapObjectBehavior, 3):` | Unresolved authored map-object behavior ID sharing the lift-door depth bias; no evidence justifies calling the behavior a lift door. |
| `render_map_object` | 107 | `0` | `depth = 0;` | No additional ordering-table depth bias in the default behavior. |
| `menu_render_item_model` | 121 | `0` | `tmd_select_object_vertices(0);` | First object within the selected TMD resource; local array origin, not a global asset ID. |
| `menu_render_item_model` | 122 | `0` | `tmd_project_vertices(tmd_get_object(0)->vertex_count);` | First object within the selected TMD resource; local array origin, not a global asset ID. |
| `menu_render_item_model` | 123 | `0` | `render_enqueue_tmd(0, MENU_ITEM_DEPTH_BIAS);` | First object within the selected TMD resource. |

## `src/game/render_scene.c`

10 retained occurrences. The [visibility-domain review](visibility-screen-domains.md)
names the four hidden-cell comparisons. The [actor culling review](actor-culling-domain.md)
replaces the raw mode comparison with a typed visibility-grid member.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `render_entities` | 32 | `1` | `for (i = KF_MAP_OBJECT_CAPACITY - 1; i != -1; i--) {` | Convert pool capacity into the last zero-based countdown value. |
| `render_entities` | 32 | `1` | `for (i = KF_MAP_OBJECT_CAPACITY - 1; i != -1; i--) {` | Negative-one exhausted countdown sentinel; token 1 is preceded by unary minus. |
| `render_entities` | 48 | `1` | `for (i = KF_ACTOR_CAPACITY - 1; i != -1; i--) {` | Convert pool capacity into the last zero-based countdown value. |
| `render_entities` | 48 | `1` | `for (i = KF_ACTOR_CAPACITY - 1; i != -1; i--) {` | Negative-one exhausted countdown sentinel; token 1 is preceded by unary minus. |
| `render_entities` | 75 | `0` | `if (visible != 0) {` | Boolean/nonzero visibility result gates the actor emitter. |
| `render_entities` | 92 | `1` | `for (i--; i != -1; i--) {` | Negative-one exhausted countdown sentinel; token 1 is preceded by unary minus. |
| `render_entities` | 108 | `1` | `for (i = KF_EFFECT_CAPACITY - 1; i != -1; i--) {` | Convert pool capacity into the last zero-based countdown value. |
| `render_entities` | 108 | `1` | `for (i = KF_EFFECT_CAPACITY - 1; i != -1; i--) {` | Negative-one exhausted countdown sentinel; token 1 is preceded by unary minus. |
| `render_entities` | 129 | `1` | `for (i = KF_MAP_EVENT_CAPACITY - 1; i != -1; i--) {` | Convert pool capacity into the last zero-based countdown value. |
| `render_entities` | 129 | `1` | `for (i = KF_MAP_EVENT_CAPACITY - 1; i != -1; i--) {` | Negative-one exhausted countdown sentinel; token 1 is preceded by unary minus. |

## `src/game/geometry_render.c`

146 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 16 | `0` | `{KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x15, 0x32, 5}},` | Opaque header byte +1 is authored zero; no consumer supports a semantic field identity. |
| `initializers` | 16 | `0x50` | `{KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x15, 0x32, 5}},` | Authored HUD u coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 16 | `0x03` | `{KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x15, 0x32, 5}},` | Authored HUD v coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 16 | `1` | `{KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x15, 0x32, 5}},` | Authored HUD u_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 16 | `5` | `{KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x15, 0x32, 5}},` | Authored HUD v_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 16 | `0x1f` | `{KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x15, 0x32, 5}},` | Authored HUD x coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 16 | `0x15` | `{KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x15, 0x32, 5}},` | Authored HUD y coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 16 | `0x32` | `{KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x15, 0x32, 5}},` | Authored HUD w coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 16 | `5` | `{KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x15, 0x32, 5}},` | Authored HUD h coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 17 | `0` | `{KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x23, 0x32, 5}},` | Opaque header byte +1 is authored zero; no consumer supports a semantic field identity. |
| `initializers` | 17 | `0x50` | `{KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x23, 0x32, 5}},` | Authored HUD u coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 17 | `0x03` | `{KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x23, 0x32, 5}},` | Authored HUD v coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 17 | `1` | `{KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x23, 0x32, 5}},` | Authored HUD u_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 17 | `5` | `{KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x23, 0x32, 5}},` | Authored HUD v_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 17 | `0x1f` | `{KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x23, 0x32, 5}},` | Authored HUD x coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 17 | `0x23` | `{KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x23, 0x32, 5}},` | Authored HUD y coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 17 | `0x32` | `{KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x23, 0x32, 5}},` | Authored HUD w coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 17 | `5` | `{KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x23, 0x32, 5}},` | Authored HUD h coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 18 | `0` | `{KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x16, 0x32, 2}},` | Opaque header byte +1 is authored zero; no consumer supports a semantic field identity. |
| `initializers` | 18 | `0x50` | `{KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x16, 0x32, 2}},` | Authored HUD u coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 18 | `0x15` | `{KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x16, 0x32, 2}},` | Authored HUD v coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 18 | `1` | `{KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x16, 0x32, 2}},` | Authored HUD u_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 18 | `2` | `{KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x16, 0x32, 2}},` | Authored HUD v_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 18 | `0x81` | `{KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x16, 0x32, 2}},` | Authored HUD x coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 18 | `0x16` | `{KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x16, 0x32, 2}},` | Authored HUD y coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 18 | `0x32` | `{KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x16, 0x32, 2}},` | Authored HUD w coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 18 | `2` | `{KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x16, 0x32, 2}},` | Authored HUD h coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 19 | `0` | `{KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x24, 0x32, 2}},` | Opaque header byte +1 is authored zero; no consumer supports a semantic field identity. |
| `initializers` | 19 | `0x50` | `{KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x24, 0x32, 2}},` | Authored HUD u coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 19 | `0x15` | `{KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x24, 0x32, 2}},` | Authored HUD v coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 19 | `1` | `{KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x24, 0x32, 2}},` | Authored HUD u_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 19 | `2` | `{KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x24, 0x32, 2}},` | Authored HUD v_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 19 | `0x81` | `{KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x24, 0x32, 2}},` | Authored HUD x coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 19 | `0x24` | `{KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x24, 0x32, 2}},` | Authored HUD y coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 19 | `0x32` | `{KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x24, 0x32, 2}},` | Authored HUD w coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 19 | `2` | `{KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x24, 0x32, 2}},` | Authored HUD h coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 20 | `0` | `{KF_HUD_HIDDEN, 0, {0, 0x50, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Opaque header byte +1 is authored zero; no consumer supports a semantic field identity. |
| `initializers` | 20 | `0` | `{KF_HUD_HIDDEN, 0, {0, 0x50, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD u coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 20 | `0x50` | `{KF_HUD_HIDDEN, 0, {0, 0x50, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD v coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 20 | `0x38` | `{KF_HUD_HIDDEN, 0, {0, 0x50, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD u_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 20 | `0x10` | `{KF_HUD_HIDDEN, 0, {0, 0x50, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD v_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 20 | `0xc7` | `{KF_HUD_HIDDEN, 0, {0, 0x50, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD x coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 20 | `0x16` | `{KF_HUD_HIDDEN, 0, {0, 0x50, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD y coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 20 | `0x38` | `{KF_HUD_HIDDEN, 0, {0, 0x50, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD w coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 20 | `0x10` | `{KF_HUD_HIDDEN, 0, {0, 0x50, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD h coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 21 | `0` | `{KF_HUD_HIDDEN, 0, {0, 0x60, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Opaque header byte +1 is authored zero; no consumer supports a semantic field identity. |
| `initializers` | 21 | `0` | `{KF_HUD_HIDDEN, 0, {0, 0x60, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD u coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 21 | `0x60` | `{KF_HUD_HIDDEN, 0, {0, 0x60, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD v coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 21 | `0x38` | `{KF_HUD_HIDDEN, 0, {0, 0x60, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD u_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 21 | `0x10` | `{KF_HUD_HIDDEN, 0, {0, 0x60, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD v_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 21 | `0xc7` | `{KF_HUD_HIDDEN, 0, {0, 0x60, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD x coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 21 | `0x16` | `{KF_HUD_HIDDEN, 0, {0, 0x60, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD y coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 21 | `0x38` | `{KF_HUD_HIDDEN, 0, {0, 0x60, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD w coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 21 | `0x10` | `{KF_HUD_HIDDEN, 0, {0, 0x60, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD h coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 22 | `0` | `{KF_HUD_HIDDEN, 0, {0, 0x40, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Opaque header byte +1 is authored zero; no consumer supports a semantic field identity. |
| `initializers` | 22 | `0` | `{KF_HUD_HIDDEN, 0, {0, 0x40, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD u coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 22 | `0x40` | `{KF_HUD_HIDDEN, 0, {0, 0x40, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD v coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 22 | `0x38` | `{KF_HUD_HIDDEN, 0, {0, 0x40, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD u_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 22 | `0x10` | `{KF_HUD_HIDDEN, 0, {0, 0x40, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD v_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 22 | `0xc7` | `{KF_HUD_HIDDEN, 0, {0, 0x40, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD x coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 22 | `0x16` | `{KF_HUD_HIDDEN, 0, {0, 0x40, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD y coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 22 | `0x38` | `{KF_HUD_HIDDEN, 0, {0, 0x40, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD w coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 22 | `0x10` | `{KF_HUD_HIDDEN, 0, {0, 0x40, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD h coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 23 | `0` | `{KF_HUD_HIDDEN, 0, {0, 0x70, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Opaque header byte +1 is authored zero; no consumer supports a semantic field identity. |
| `initializers` | 23 | `0` | `{KF_HUD_HIDDEN, 0, {0, 0x70, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD u coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 23 | `0x70` | `{KF_HUD_HIDDEN, 0, {0, 0x70, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD v coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 23 | `0x38` | `{KF_HUD_HIDDEN, 0, {0, 0x70, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD u_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 23 | `0x10` | `{KF_HUD_HIDDEN, 0, {0, 0x70, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD v_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 23 | `0xc7` | `{KF_HUD_HIDDEN, 0, {0, 0x70, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD x coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 23 | `0x16` | `{KF_HUD_HIDDEN, 0, {0, 0x70, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD y coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 23 | `0x38` | `{KF_HUD_HIDDEN, 0, {0, 0x70, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD w coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 23 | `0x10` | `{KF_HUD_HIDDEN, 0, {0, 0x70, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},` | Authored HUD h coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 24 | `0` | `{KF_HUD_VISIBLE, 0, {0, 0, 0x4a, 0x0b, 0x0a, 0x12, 0x4a, 0x0b}},` | Opaque header byte +1 is authored zero; no consumer supports a semantic field identity. |
| `initializers` | 24 | `0` | `{KF_HUD_VISIBLE, 0, {0, 0, 0x4a, 0x0b, 0x0a, 0x12, 0x4a, 0x0b}},` | Authored HUD u coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 24 | `0` | `{KF_HUD_VISIBLE, 0, {0, 0, 0x4a, 0x0b, 0x0a, 0x12, 0x4a, 0x0b}},` | Authored HUD v coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 24 | `0x4a` | `{KF_HUD_VISIBLE, 0, {0, 0, 0x4a, 0x0b, 0x0a, 0x12, 0x4a, 0x0b}},` | Authored HUD u_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 24 | `0x0b` | `{KF_HUD_VISIBLE, 0, {0, 0, 0x4a, 0x0b, 0x0a, 0x12, 0x4a, 0x0b}},` | Authored HUD v_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 24 | `0x0a` | `{KF_HUD_VISIBLE, 0, {0, 0, 0x4a, 0x0b, 0x0a, 0x12, 0x4a, 0x0b}},` | Authored HUD x coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 24 | `0x12` | `{KF_HUD_VISIBLE, 0, {0, 0, 0x4a, 0x0b, 0x0a, 0x12, 0x4a, 0x0b}},` | Authored HUD y coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 24 | `0x4a` | `{KF_HUD_VISIBLE, 0, {0, 0, 0x4a, 0x0b, 0x0a, 0x12, 0x4a, 0x0b}},` | Authored HUD w coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 24 | `0x0b` | `{KF_HUD_VISIBLE, 0, {0, 0, 0x4a, 0x0b, 0x0a, 0x12, 0x4a, 0x0b}},` | Authored HUD h coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 25 | `0` | `{KF_HUD_VISIBLE, 0, {0, 0x10, 0x4a, 0x0b, 0x0a, 0x20, 0x4a, 0x0b}},` | Opaque header byte +1 is authored zero; no consumer supports a semantic field identity. |
| `initializers` | 25 | `0` | `{KF_HUD_VISIBLE, 0, {0, 0x10, 0x4a, 0x0b, 0x0a, 0x20, 0x4a, 0x0b}},` | Authored HUD u coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 25 | `0x10` | `{KF_HUD_VISIBLE, 0, {0, 0x10, 0x4a, 0x0b, 0x0a, 0x20, 0x4a, 0x0b}},` | Authored HUD v coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 25 | `0x4a` | `{KF_HUD_VISIBLE, 0, {0, 0x10, 0x4a, 0x0b, 0x0a, 0x20, 0x4a, 0x0b}},` | Authored HUD u_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 25 | `0x0b` | `{KF_HUD_VISIBLE, 0, {0, 0x10, 0x4a, 0x0b, 0x0a, 0x20, 0x4a, 0x0b}},` | Authored HUD v_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 25 | `0x0a` | `{KF_HUD_VISIBLE, 0, {0, 0x10, 0x4a, 0x0b, 0x0a, 0x20, 0x4a, 0x0b}},` | Authored HUD x coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 25 | `0x20` | `{KF_HUD_VISIBLE, 0, {0, 0x10, 0x4a, 0x0b, 0x0a, 0x20, 0x4a, 0x0b}},` | Authored HUD y coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 25 | `0x4a` | `{KF_HUD_VISIBLE, 0, {0, 0x10, 0x4a, 0x0b, 0x0a, 0x20, 0x4a, 0x0b}},` | Authored HUD w coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 25 | `0x0b` | `{KF_HUD_VISIBLE, 0, {0, 0x10, 0x4a, 0x0b, 0x0a, 0x20, 0x4a, 0x0b}},` | Authored HUD h coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 26 | `0` | `{KF_HUD_VISIBLE, 0, {0, 0x20, 0x5c, 8, 0x5e, 0x13, 0x5c, 8}},` | Opaque header byte +1 is authored zero; no consumer supports a semantic field identity. |
| `initializers` | 26 | `0` | `{KF_HUD_VISIBLE, 0, {0, 0x20, 0x5c, 8, 0x5e, 0x13, 0x5c, 8}},` | Authored HUD u coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 26 | `0x20` | `{KF_HUD_VISIBLE, 0, {0, 0x20, 0x5c, 8, 0x5e, 0x13, 0x5c, 8}},` | Authored HUD v coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 26 | `0x5c` | `{KF_HUD_VISIBLE, 0, {0, 0x20, 0x5c, 8, 0x5e, 0x13, 0x5c, 8}},` | Authored HUD u_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 26 | `8` | `{KF_HUD_VISIBLE, 0, {0, 0x20, 0x5c, 8, 0x5e, 0x13, 0x5c, 8}},` | Authored HUD v_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 26 | `0x5e` | `{KF_HUD_VISIBLE, 0, {0, 0x20, 0x5c, 8, 0x5e, 0x13, 0x5c, 8}},` | Authored HUD x coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 26 | `0x13` | `{KF_HUD_VISIBLE, 0, {0, 0x20, 0x5c, 8, 0x5e, 0x13, 0x5c, 8}},` | Authored HUD y coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 26 | `0x5c` | `{KF_HUD_VISIBLE, 0, {0, 0x20, 0x5c, 8, 0x5e, 0x13, 0x5c, 8}},` | Authored HUD w coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 26 | `8` | `{KF_HUD_VISIBLE, 0, {0, 0x20, 0x5c, 8, 0x5e, 0x13, 0x5c, 8}},` | Authored HUD h coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 27 | `0` | `{KF_HUD_VISIBLE, 0, {0, 0x30, 0x5c, 8, 0x5e, 0x21, 0x5c, 8}},` | Opaque header byte +1 is authored zero; no consumer supports a semantic field identity. |
| `initializers` | 27 | `0` | `{KF_HUD_VISIBLE, 0, {0, 0x30, 0x5c, 8, 0x5e, 0x21, 0x5c, 8}},` | Authored HUD u coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 27 | `0x30` | `{KF_HUD_VISIBLE, 0, {0, 0x30, 0x5c, 8, 0x5e, 0x21, 0x5c, 8}},` | Authored HUD v coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 27 | `0x5c` | `{KF_HUD_VISIBLE, 0, {0, 0x30, 0x5c, 8, 0x5e, 0x21, 0x5c, 8}},` | Authored HUD u_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 27 | `8` | `{KF_HUD_VISIBLE, 0, {0, 0x30, 0x5c, 8, 0x5e, 0x21, 0x5c, 8}},` | Authored HUD v_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 27 | `0x5e` | `{KF_HUD_VISIBLE, 0, {0, 0x30, 0x5c, 8, 0x5e, 0x21, 0x5c, 8}},` | Authored HUD x coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 27 | `0x21` | `{KF_HUD_VISIBLE, 0, {0, 0x30, 0x5c, 8, 0x5e, 0x21, 0x5c, 8}},` | Authored HUD y coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 27 | `0x5c` | `{KF_HUD_VISIBLE, 0, {0, 0x30, 0x5c, 8, 0x5e, 0x21, 0x5c, 8}},` | Authored HUD w coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 27 | `8` | `{KF_HUD_VISIBLE, 0, {0, 0x30, 0x5c, 8, 0x5e, 0x21, 0x5c, 8}},` | Authored HUD h coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 28 | `0` | `{KF_HUD_VISIBLE, 0, {0, 0x80, 0x21, 0x20, 0x10d, 0x12, 0x21, 0x20}},` | Opaque header byte +1 is authored zero; no consumer supports a semantic field identity. |
| `initializers` | 28 | `0` | `{KF_HUD_VISIBLE, 0, {0, 0x80, 0x21, 0x20, 0x10d, 0x12, 0x21, 0x20}},` | Authored HUD u coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 28 | `0x80` | `{KF_HUD_VISIBLE, 0, {0, 0x80, 0x21, 0x20, 0x10d, 0x12, 0x21, 0x20}},` | Authored HUD v coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 28 | `0x21` | `{KF_HUD_VISIBLE, 0, {0, 0x80, 0x21, 0x20, 0x10d, 0x12, 0x21, 0x20}},` | Authored HUD u_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 28 | `0x20` | `{KF_HUD_VISIBLE, 0, {0, 0x80, 0x21, 0x20, 0x10d, 0x12, 0x21, 0x20}},` | Authored HUD v_span coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 28 | `0x10d` | `{KF_HUD_VISIBLE, 0, {0, 0x80, 0x21, 0x20, 0x10d, 0x12, 0x21, 0x20}},` | Authored HUD x coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 28 | `0x12` | `{KF_HUD_VISIBLE, 0, {0, 0x80, 0x21, 0x20, 0x10d, 0x12, 0x21, 0x20}},` | Authored HUD y coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 28 | `0x21` | `{KF_HUD_VISIBLE, 0, {0, 0x80, 0x21, 0x20, 0x10d, 0x12, 0x21, 0x20}},` | Authored HUD w coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 28 | `0x20` | `{KF_HUD_VISIBLE, 0, {0, 0x80, 0x21, 0x20, 0x10d, 0x12, 0x21, 0x20}},` | Authored HUD h coordinate/span in the typed sprite tuple; atlas and screen layout supply its context, so retain as data. |
| `initializers` | 29 | `0` | `{KF_HUD_END, 0, {0xff, 0xff, 0xff, 0xff, 0xffff, 0xffff, 0xffff, 0xffff}},` | Opaque header byte +1 is authored zero; no consumer supports a semantic field identity. |
| `initializers` | 29 | `0xff` | `{KF_HUD_END, 0, {0xff, 0xff, 0xff, 0xff, 0xffff, 0xffff, 0xffff, 0xffff}},` | Terminating row u fill, never consumed as a sprite; preserve the authored all-ones payload. |
| `initializers` | 29 | `0xff` | `{KF_HUD_END, 0, {0xff, 0xff, 0xff, 0xff, 0xffff, 0xffff, 0xffff, 0xffff}},` | Terminating row v fill, never consumed as a sprite; preserve the authored all-ones payload. |
| `initializers` | 29 | `0xff` | `{KF_HUD_END, 0, {0xff, 0xff, 0xff, 0xff, 0xffff, 0xffff, 0xffff, 0xffff}},` | Terminating row u_span fill, never consumed as a sprite; preserve the authored all-ones payload. |
| `initializers` | 29 | `0xff` | `{KF_HUD_END, 0, {0xff, 0xff, 0xff, 0xff, 0xffff, 0xffff, 0xffff, 0xffff}},` | Terminating row v_span fill, never consumed as a sprite; preserve the authored all-ones payload. |
| `initializers` | 29 | `0xffff` | `{KF_HUD_END, 0, {0xff, 0xff, 0xff, 0xff, 0xffff, 0xffff, 0xffff, 0xffff}},` | Terminating row x fill, never consumed as a sprite; preserve the authored all-ones payload. |
| `initializers` | 29 | `0xffff` | `{KF_HUD_END, 0, {0xff, 0xff, 0xff, 0xff, 0xffff, 0xffff, 0xffff, 0xffff}},` | Terminating row y fill, never consumed as a sprite; preserve the authored all-ones payload. |
| `initializers` | 29 | `0xffff` | `{KF_HUD_END, 0, {0xff, 0xff, 0xff, 0xff, 0xffff, 0xffff, 0xffff, 0xffff}},` | Terminating row w fill, never consumed as a sprite; preserve the authored all-ones payload. |
| `initializers` | 29 | `0xffff` | `{KF_HUD_END, 0, {0xff, 0xff, 0xff, 0xff, 0xffff, 0xffff, 0xffff, 0xffff}},` | Terminating row h fill, never consumed as a sprite; preserve the authored all-ones payload. |
| `render_weapon` | 63 | `16` | `SetGeomScreen(*(const u16 *)((const u8 *)player_state.equipped_weapon_record + 16));` | Proven unsigned weapon projection field offset; the full weapon render-block type remains unresolved. |
| `render_weapon` | 66 | `0` | `model.t[0] = *(const s16 *)(fields + 28);` | SDK MATRIX translation component index: 0=X, 1=Y, 2=Z. |
| `render_weapon` | 66 | `28` | `model.t[0] = *(const s16 *)(fields + 28);` | Proven signed weapon translation field offset; full KfWeaponRecord render-block modeling remains debt, not an offset-macro identity. |
| `render_weapon` | 67 | `1` | `model.t[1] = *(const s16 *)(fields + 30);` | SDK MATRIX translation component index: 0=X, 1=Y, 2=Z. |
| `render_weapon` | 67 | `30` | `model.t[1] = *(const s16 *)(fields + 30);` | Proven signed weapon translation field offset; full KfWeaponRecord render-block modeling remains debt, not an offset-macro identity. |
| `render_weapon` | 68 | `2` | `model.t[2] = *(const s16 *)(fields + 32);` | SDK MATRIX translation component index: 0=X, 1=Y, 2=Z. |
| `render_weapon` | 68 | `32` | `model.t[2] = *(const s16 *)(fields + 32);` | Proven signed weapon translation field offset; full KfWeaponRecord render-block modeling remains debt, not an offset-macro identity. |
| `render_weapon` | 69 | `36` | `RotMatrix((SVECTOR *)(fields + 36), &model);` | Proven rotation-vector start in the weapon record; preserve SDK SVECTOR interpretation while the complete render block is recovered. |
| `render_weapon` | 73 | `0` | `object = tmd_get_object(0);` | First object within the selected TMD resource; local array origin, not a global asset ID. |
| `render_weapon` | 75 | `0` | `&player_state.weapon_animation_cache, KF_ASSET_WEAPON, 0,` | First animation/visibility tag for the weapon resource; attack phase supplies the changing frame. |
| `render_weapon` | 77 | `0` | `object->vertex_count) != 0) {` | Animation-binding boolean success/failure predicate; preserve exact zero comparison. |
| `render_weapon` | 80 | `32` | `(s16)*(const u16 *)((const u8 *)player_state.equipped_weapon_record + 32) >> WEAPON_DEPTH_BIAS_SHIFT;` | Proven weapon Z-translation offset, explicitly sign-extended before shifting; shared object-type recovery remains open. |
| `render_weapon` | 81 | `0` | `render_enqueue_tmd(0, -depth_bias + WEAPON_BASE_DEPTH_BIAS);` | First object within the selected TMD resource. |
| `render_effect_sprites` | 107 | `0` | `model.t[0] = entry->translation_x;` | SDK MATRIX translation component index: 0=X, 1=Y, 2=Z. |
| `render_effect_sprites` | 108 | `1` | `model.t[1] = entry->translation_y;` | SDK MATRIX translation component index: 0=X, 1=Y, 2=Z. |
| `render_effect_sprites` | 109 | `2` | `model.t[2] = entry->translation_z;` | SDK MATRIX translation component index: 0=X, 1=Y, 2=Z. |
| `render_effect_sprites` | 118 | `0` | `object = tmd_get_object(0);` | First object within the selected TMD resource; local array origin, not a global asset ID. |
| `render_effect_sprites` | 122 | `0` | `object->vertex_count) != 0) {` | Animation-binding boolean success/failure predicate; preserve exact zero comparison. |
| `render_effect_sprites` | 124 | `0` | `render_enqueue_tmd(0, 0);` | First object within the selected TMD resource. |
| `render_effect_sprites` | 124 | `0` | `render_enqueue_tmd(0, 0);` | Zero extra ordering-table depth bias. |

## `src/game/render_frame.c`

35 retained occurrences. The [notification sprite-state review](game-notification-identities.md#sprite-visibility-state)
names the three visibility predicates. The [sprite cue review](sprite-cue-tmd-slots.md) names
the three notification depth-cue arguments.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 30 | `0` | `{KF_EFFECT_SPRITE_ACTIVE, 0, 0, 0x33, 0x11e, 0x22, 0xc8, {0, 0}, {0, 0, 0, 0}, {0, 0}, 0},` | Initial animation visibility tag zero, passed to the animation binder. |
| `initializers` | 30 | `0` | `{KF_EFFECT_SPRITE_ACTIVE, 0, 0, 0x33, 0x11e, 0x22, 0xc8, {0, 0}, {0, 0, 0, 0}, {0, 0}, 0},` | Initial asset variant zero in the authored animation descriptor. |
| `initializers` | 30 | `0x33` | `{KF_EFFECT_SPRITE_ACTIVE, 0, 0, 0x33, 0x11e, 0x22, 0xc8, {0, 0}, {0, 0, 0, 0}, {0, 0}, 0},` | Authored compass X/Y scale numerator 51 in Q12 units; preserve the model-specific scale. |
| `initializers` | 30 | `0x11e` | `{KF_EFFECT_SPRITE_ACTIVE, 0, 0, 0x33, 0x11e, 0x22, 0xc8, {0, 0}, {0, 0, 0, 0}, {0, 0}, 0},` | Authored compass model X translation 286; model geometry and view coordinates determine placement. |
| `initializers` | 30 | `0x22` | `{KF_EFFECT_SPRITE_ACTIVE, 0, 0, 0x33, 0x11e, 0x22, 0xc8, {0, 0}, {0, 0, 0, 0}, {0, 0}, 0},` | Authored compass model Y translation 34; preserve its placement tuple. |
| `initializers` | 30 | `0xc8` | `{KF_EFFECT_SPRITE_ACTIVE, 0, 0, 0x33, 0x11e, 0x22, 0xc8, {0, 0}, {0, 0, 0, 0}, {0, 0}, 0},` | Authored compass model Z translation 200; a position, not an alias for projection distance. |
| `initializers` | 30 | `0` | `{KF_EFFECT_SPRITE_ACTIVE, 0, 0, 0x33, 0x11e, 0x22, 0xc8, {0, 0}, {0, 0, 0, 0}, {0, 0}, 0},` | Opaque byte +12 is authored zero; no semantic name is established. |
| `initializers` | 30 | `0` | `{KF_EFFECT_SPRITE_ACTIVE, 0, 0, 0x33, 0x11e, 0x22, 0xc8, {0, 0}, {0, 0, 0, 0}, {0, 0}, 0},` | Opaque byte +13 is authored zero; no semantic name is established. |
| `initializers` | 30 | `0` | `{KF_EFFECT_SPRITE_ACTIVE, 0, 0, 0x33, 0x11e, 0x22, 0xc8, {0, 0}, {0, 0, 0, 0}, {0, 0}, 0},` | Initial rotation X is zero angle. |
| `initializers` | 30 | `0` | `{KF_EFFECT_SPRITE_ACTIVE, 0, 0, 0x33, 0x11e, 0x22, 0xc8, {0, 0}, {0, 0, 0, 0}, {0, 0}, 0},` | Initial rotation Y is zero angle. |
| `initializers` | 30 | `0` | `{KF_EFFECT_SPRITE_ACTIVE, 0, 0, 0x33, 0x11e, 0x22, 0xc8, {0, 0}, {0, 0, 0, 0}, {0, 0}, 0},` | Initial rotation Z is zero angle; the frame replaces it with wrapped negative view yaw. |
| `initializers` | 30 | `0` | `{KF_EFFECT_SPRITE_ACTIVE, 0, 0, 0x33, 0x11e, 0x22, 0xc8, {0, 0}, {0, 0, 0, 0}, {0, 0}, 0},` | SDK SVECTOR pad is zero in the authored rotation tuple. |
| `initializers` | 30 | `0` | `{KF_EFFECT_SPRITE_ACTIVE, 0, 0, 0x33, 0x11e, 0x22, 0xc8, {0, 0}, {0, 0, 0, 0}, {0, 0}, 0},` | Opaque byte +22 is authored zero; no semantic name is established. |
| `initializers` | 30 | `0` | `{KF_EFFECT_SPRITE_ACTIVE, 0, 0, 0x33, 0x11e, 0x22, 0xc8, {0, 0}, {0, 0, 0, 0}, {0, 0}, 0},` | Opaque byte +23 is authored zero; no semantic name is established. |
| `initializers` | 30 | `0` | `{KF_EFFECT_SPRITE_ACTIVE, 0, 0, 0x33, 0x11e, 0x22, 0xc8, {0, 0}, {0, 0, 0, 0}, {0, 0}, 0},` | Null initial animation-cache pointer; the binder owns allocation. |
| `initializers` | 36 | `0` | `RECT hud_palette_rect = {0, 500, 16, 1};` | Palette rectangle X origin in VRAM; authored material data. |
| `initializers` | 36 | `500` | `RECT hud_palette_rect = {0, 500, 16, 1};` | Palette rectangle Y selects the authored HUD/message CLUT row in VRAM. |
| `initializers` | 36 | `16` | `RECT hud_palette_rect = {0, 500, 16, 1};` | Palette rectangle width is 16 colors for this four-bit texture material. |
| `initializers` | 36 | `1` | `RECT hud_palette_rect = {0, 500, 16, 1};` | Palette rectangle height is one CLUT row. |
| `initializers` | 39 | `0` | `RECT notification_palette_rect = {0, 499, 16, 1};` | Palette rectangle X origin in VRAM; authored material data. |
| `initializers` | 39 | `499` | `RECT notification_palette_rect = {0, 499, 16, 1};` | Palette rectangle Y selects the authored HUD/message CLUT row in VRAM. |
| `initializers` | 39 | `16` | `RECT notification_palette_rect = {0, 499, 16, 1};` | Palette rectangle width is 16 colors for this four-bit texture material. |
| `initializers` | 39 | `1` | `RECT notification_palette_rect = {0, 499, 16, 1};` | Palette rectangle height is one CLUT row. |
| `render_frame` | 73 | `1` | `+ (player_state.vitals.maximum_hp - 1) / HUD_GAUGE_WIDTH)` | Arithmetic endpoint in the retail gauge correction term (maximum minus one)/width; preserve its division placement. |
| `render_frame` | 76 | `1` | `+ (player_state.vitals.maximum_mp - 1) / HUD_GAUGE_WIDTH)` | Arithmetic endpoint in the retail gauge correction term (maximum minus one)/width; preserve its division placement. |
| `render_frame` | 122 | `0` | `model.t[0] = 0;` | SDK MATRIX translation component index: 0=X, 1=Y, 2=Z. |
| `render_frame` | 122 | `0` | `model.t[0] = 0;` | Zero X translation centers the notification model transform. |
| `render_frame` | 123 | `1` | `model.t[1] = NOTIFICATION_MODEL_Y;` | SDK MATRIX translation component index: 0=X, 1=Y, 2=Z. |
| `render_frame` | 124 | `2` | `model.t[2] = NOTIFICATION_MODEL_Z;` | SDK MATRIX translation component index: 0=X, 1=Y, 2=Z. |
| `render_frame` | 125 | `0` | `spin.vz = 0;` | Zero rotation around this axis; notification effect supplies only X rotation. |
| `render_frame` | 126 | `0` | `spin.vy = 0;` | Zero rotation around this axis; notification effect supplies only X rotation. |
| `render_frame` | 136 | `0` | `render_enqueue_sprite(&record[KF_NOTIFICATION_TEXT_SPRITE].sprite, 0, KF_SPRITE_DEPTH_CUE_NORMAL);` | Zero additional ordering-table depth bias. |
| `render_frame` | 139 | `0` | `render_enqueue_sprite(&record[KF_NOTIFICATION_GOLD_SPRITE].sprite, 0, KF_SPRITE_DEPTH_CUE_NORMAL);` | Zero additional ordering-table depth bias. |
| `render_frame` | 144 | `1` | `for (i = KF_NOTIFICATION_THOUSANDS_SPRITE - KF_NOTIFICATION_ONES_SPRITE; i != -1; i--) {` | Negative-one exhausted countdown sentinel; token 1 is preceded by unary minus. |
| `render_frame` | 146 | `0` | `render_enqueue_sprite(&record->sprite, 0, KF_SPRITE_DEPTH_CUE_NORMAL);` | Zero additional ordering-table depth bias. |
