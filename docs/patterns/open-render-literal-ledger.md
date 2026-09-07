# OPEN renderer retained-literal ledger

Companion to [the renderer evidence](open-render-constants.md). Every remaining
numeric/character occurrence in these four C files has a row. Comments, strings,
identifier digits, enum/macro definitions and ADDRESS/DATA/RODATA claims are
excluded. Repeated literals have separate rows in source order; unary minus
belongs to the context. The scene-script file only moves existing named model
constants and retains its separate earlier literal audit.

## `src/open/opening_render.c`

26 → 24 retained occurrences.

| Function/data | Line | Literal | Source context | Reason retained |
| --- | ---: | --- | --- | --- |
| `sprite_add_g4` | 27 | `0` | `prim->x0 = position[0];` | Rectangle X halfword index. |
| `sprite_add_g4` | 28 | `1` | `prim->y0 = position[1];` | Rectangle Y halfword index. |
| `sprite_add_g4` | 29 | `0` | `prim->x1 = position[0] + position[2];` | Rectangle X halfword index. |
| `sprite_add_g4` | 29 | `2` | `prim->x1 = position[0] + position[2];` | Rectangle width halfword index. |
| `sprite_add_g4` | 30 | `1` | `prim->y1 = position[1];` | Rectangle Y halfword index. |
| `sprite_add_g4` | 31 | `0` | `prim->x2 = position[0];` | Rectangle X halfword index. |
| `sprite_add_g4` | 32 | `1` | `prim->y2 = position[1] + position[3];` | Rectangle Y halfword index. |
| `sprite_add_g4` | 32 | `3` | `prim->y2 = position[1] + position[3];` | Rectangle height halfword index. |
| `sprite_add_g4` | 33 | `0` | `prim->x3 = position[0] + position[2];` | Rectangle X halfword index. |
| `sprite_add_g4` | 33 | `2` | `prim->x3 = position[0] + position[2];` | Rectangle width halfword index. |
| `sprite_add_g4` | 34 | `1` | `prim->y3 = position[1] + position[3];` | Rectangle Y halfword index. |
| `sprite_add_g4` | 34 | `3` | `prim->y3 = position[1] + position[3];` | Rectangle height halfword index. |
| `sprite_add_g4` | 35 | `0` | `prim->r0 = color0[0];` | Red byte index in the borrowed color array. |
| `sprite_add_g4` | 36 | `1` | `prim->g0 = color0[1];` | Green byte index in the borrowed color array. |
| `sprite_add_g4` | 37 | `2` | `prim->b0 = color0[2];` | Blue byte index in the borrowed color array. |
| `sprite_add_g4` | 38 | `0` | `prim->r1 = color1[0];` | Red byte index in the borrowed color array. |
| `sprite_add_g4` | 39 | `1` | `prim->g1 = color1[1];` | Green byte index in the borrowed color array. |
| `sprite_add_g4` | 40 | `2` | `prim->b1 = color1[2];` | Blue byte index in the borrowed color array. |
| `sprite_add_g4` | 41 | `0` | `prim->r2 = color2[0];` | Red byte index in the borrowed color array. |
| `sprite_add_g4` | 42 | `1` | `prim->g2 = color2[1];` | Green byte index in the borrowed color array. |
| `sprite_add_g4` | 43 | `2` | `prim->b2 = color2[2];` | Blue byte index in the borrowed color array. |
| `sprite_add_g4` | 44 | `0` | `prim->r3 = color3[0];` | Red byte index in the borrowed color array. |
| `sprite_add_g4` | 45 | `1` | `prim->g3 = color3[1];` | Green byte index in the borrowed color array. |
| `sprite_add_g4` | 46 | `2` | `prim->b3 = color3[2];` | Blue byte index in the borrowed color array. |

## `src/open/opening_scene0_render.c`

1 → 0 retained occurrences.

| Function/data | Line | Literal | Source context | Reason retained |
| --- | ---: | --- | --- | --- |

No retained numeric/character literals.

## `src/open/entity_render.c`

102 → 84 retained occurrences.

| Function/data | Line | Literal | Source context | Reason retained |
| --- | ---: | --- | --- | --- |
| `initializers` | 19 | `7` | `KfSpriteQuad floor_item_sprites[7] = {` | Complete authored descriptor count, including the duplicated second frame; not an inferred item-type enum. |
| `initializers` | 20 | `0x90` | `{0x90, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite u coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 20 | `0x00` | `{0x90, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite v coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 20 | `0x20` | `{0x90, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite u_span coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 20 | `0x20` | `{0x90, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite v_span coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 20 | `0xfe00` | `{0x90, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite x coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 20 | `0xfc40` | `{0x90, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite y coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 20 | `0x400` | `{0x90, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite w coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 20 | `0x400` | `{0x90, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite h coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 21 | `0xb0` | `{0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite u coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 21 | `0x00` | `{0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite v coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 21 | `0x20` | `{0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite u_span coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 21 | `0x20` | `{0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite v_span coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 21 | `0xfe00` | `{0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite x coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 21 | `0xfc40` | `{0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite y coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 21 | `0x400` | `{0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite w coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 21 | `0x400` | `{0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite h coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 22 | `0xd0` | `{0xd0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite u coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 22 | `0x00` | `{0xd0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite v coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 22 | `0x20` | `{0xd0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite u_span coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 22 | `0x20` | `{0xd0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite v_span coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 22 | `0xfe00` | `{0xd0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite x coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 22 | `0xfc40` | `{0xd0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite y coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 22 | `0x400` | `{0xd0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite w coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 22 | `0x400` | `{0xd0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite h coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 23 | `0xb0` | `{0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite u coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 23 | `0x00` | `{0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite v coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 23 | `0x20` | `{0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite u_span coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 23 | `0x20` | `{0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite v_span coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 23 | `0xfe00` | `{0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite x coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 23 | `0xfc40` | `{0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite y coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 23 | `0x400` | `{0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite w coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 23 | `0x400` | `{0xb0, 0x00, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored floor-item sprite h coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 24 | `0x90` | `{0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite u coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 24 | `0x20` | `{0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite v coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 24 | `0x20` | `{0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite u_span coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 24 | `0x27` | `{0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite v_span coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 24 | `0xfe00` | `{0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite x coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 24 | `0xfb40` | `{0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite y coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 24 | `0x400` | `{0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite w coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 24 | `0x500` | `{0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite h coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 25 | `0xb0` | `{0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite u coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 25 | `0x20` | `{0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite v coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 25 | `0x20` | `{0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite u_span coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 25 | `0x27` | `{0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite v_span coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 25 | `0xfe00` | `{0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite x coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 25 | `0xfb40` | `{0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite y coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 25 | `0x400` | `{0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite w coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 25 | `0x500` | `{0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite h coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 26 | `0xd0` | `{0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite u coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 26 | `0x20` | `{0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite v coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 26 | `0x20` | `{0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite u_span coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 26 | `0x27` | `{0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite v_span coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 26 | `0xfe00` | `{0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite x coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 26 | `0xfb40` | `{0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite y coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 26 | `0x400` | `{0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite w coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 26 | `0x500` | `{0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored floor-item sprite h coordinate/span; the typed tuple preserves atlas layout and projected billboard geometry. |
| `initializers` | 31 | `0` | `{{0, 0, KF_FIXED12_ONE}, {0, 0, KF_FIXED12_ONE}, {0, 0, 0}}, {0, 0, 0},` | Zero X light-direction coefficient in matrix row 0. |
| `initializers` | 31 | `0` | `{{0, 0, KF_FIXED12_ONE}, {0, 0, KF_FIXED12_ONE}, {0, 0, 0}}, {0, 0, 0},` | Zero Y light-direction coefficient in matrix row 0. |
| `initializers` | 31 | `0` | `{{0, 0, KF_FIXED12_ONE}, {0, 0, KF_FIXED12_ONE}, {0, 0, 0}}, {0, 0, 0},` | Zero X light-direction coefficient in matrix row 1. |
| `initializers` | 31 | `0` | `{{0, 0, KF_FIXED12_ONE}, {0, 0, KF_FIXED12_ONE}, {0, 0, 0}}, {0, 0, 0},` | Zero Y light-direction coefficient in matrix row 1. |
| `initializers` | 31 | `0` | `{{0, 0, KF_FIXED12_ONE}, {0, 0, KF_FIXED12_ONE}, {0, 0, 0}}, {0, 0, 0},` | Zero X coefficient in inactive light row 2. |
| `initializers` | 31 | `0` | `{{0, 0, KF_FIXED12_ONE}, {0, 0, KF_FIXED12_ONE}, {0, 0, 0}}, {0, 0, 0},` | Zero Y coefficient in inactive light row 2. |
| `initializers` | 31 | `0` | `{{0, 0, KF_FIXED12_ONE}, {0, 0, KF_FIXED12_ONE}, {0, 0, 0}}, {0, 0, 0},` | Zero Z coefficient in inactive light row 2. |
| `initializers` | 31 | `0` | `{{0, 0, KF_FIXED12_ONE}, {0, 0, KF_FIXED12_ONE}, {0, 0, 0}}, {0, 0, 0},` | Zero SDK matrix translation X. |
| `initializers` | 31 | `0` | `{{0, 0, KF_FIXED12_ONE}, {0, 0, KF_FIXED12_ONE}, {0, 0, 0}}, {0, 0, 0},` | Zero SDK matrix translation Y. |
| `initializers` | 31 | `0` | `{{0, 0, KF_FIXED12_ONE}, {0, 0, KF_FIXED12_ONE}, {0, 0, 0}}, {0, 0, 0},` | Zero SDK matrix translation Z. |
| `opening_entity_render` | 67 | `0` | `depth = 0;` | Zero additional ordering-table depth bias. |
| `opening_entity_render` | 73 | `20` | `case 20:` | Authored model ordinal: shares the -100 depth bias. No additional scene/story identity is established; preserve the resource index. |
| `opening_entity_render` | 76 | `21` | `case 21:` | Authored model ordinal: increments yaw. No additional scene/story identity is established; preserve the resource index. |
| `opening_entity_render` | 77 | `22` | `case 22:` | Authored model ordinal: increments yaw. No additional scene/story identity is established; preserve the resource index. |
| `opening_entity_render` | 80 | `23` | `case 23:` | Authored model ordinal: decrements yaw. No additional scene/story identity is established; preserve the resource index. |
| `opening_entity_render` | 81 | `24` | `case 24:` | Authored model ordinal: decrements yaw. No additional scene/story identity is established; preserve the resource index. |
| `opening_entity_render` | 84 | `25` | `case 25:` | Authored model ordinal: uses the alternate perspective-right projection helper. No additional scene/story identity is established; preserve the resource index. |
| `opening_entity_render` | 88 | `0` | `render_enqueue_tmd(object_id, 0);` | Zero additional ordering-table depth bias. |
| `opening_entity_render` | 101 | `0` | `depth = 0;` | Zero additional ordering-table depth bias. |
| `render_floor_item` | 128 | `0` | `if (facing != 0) {` | Zero packed facing selects billboard rotation; nonzero values select fixed yaw. |
| `render_floor_item` | 141 | `1` | `&floor_item_sprites[item->item_id + item->animation_frame], depth_bias, 1);` | True sprite perspective-adjustment flag; the callee boosts its perspective/fog input by one half. |
| `render_floor_item` | 142 | `1` | `next_frame = item->animation_frame + 1;` | Advance by one animation frame after rendering the current descriptor. |
| `render_floor_item` | 145 | `0xff` | `if ((next_frame & 0xff) >= (frame_count & KF_FLOOR_ITEM_FRAME_COUNT_MASK)) {` | Explicit byte narrowing before comparing the packed frame count; preserve the current increment/wrap ordering. |
| `render_floor_item` | 146 | `0` | `item->animation_frame = 0;` | Restart at the first zero-based frame after reaching the frame count. |
| `opening_render_entities_and_items` | 162 | `1` | `for (remaining = KF_OPENING_ENTITY_CAPACITY - 1; remaining != -1; remaining--) {` | Convert entity capacity to the last zero-based countdown value. |
| `opening_render_entities_and_items` | 162 | `1` | `for (remaining = KF_OPENING_ENTITY_CAPACITY - 1; remaining != -1; remaining--) {` | Negative-one countdown exhaustion; unary minus remains in the source context. |
| `opening_render_entities_and_items` | 185 | `1` | `while (--remaining != -1) {` | Negative-one countdown exhaustion; unary minus remains in the source context. |

## `src/open/opening_render_entities.c`

2 → 2 retained occurrences.

| Function/data | Line | Literal | Source context | Reason retained |
| --- | ---: | --- | --- | --- |
| `opening_render_entities` | 14 | `1` | `remaining = KF_OPENING_ENTITY_CAPACITY - 1;` | Convert entity capacity to the last zero-based countdown value. |
| `opening_render_entities` | 20 | `1` | `} while (--remaining != -1);` | Negative-one countdown exhaustion; unary minus remains in the source context. |
