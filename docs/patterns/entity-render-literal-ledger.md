# Entity and HUD renderer retained literals

Companion to [the renderer review](renderer-literal-review.md). All **250**
remaining numeric occurrences in the listed files have individual reasons.
Claims, comments, strings and named constant definitions are excluded.
Repeated tokens have separate rows in source order.

## `src/game/entity_render.c`

244 retained occurrences.

| Function / data owner | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `floor_item_sprites` | 18 | `7` | `KfSpriteQuad floor_item_sprites[7] = {` | Seven complete floor-sprite descriptors, supported by placement selectors and the independent OPEN table; this is a table extent, not an item ID. |
| `floor_item_sprites` | 19 | `0x90` | `{0x90, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 19 | `0x0` | `{0x90, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 19 | `0x20` | `{0x90, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 19 | `0x20` | `{0x90, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 19 | `0xfe00` | `{0x90, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `floor_item_sprites` | 19 | `0xfc40` | `{0x90, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `floor_item_sprites` | 19 | `0x400` | `{0x90, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 19 | `0x400` | `{0x90, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 20 | `0xb0` | `{0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 20 | `0x0` | `{0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 20 | `0x20` | `{0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 20 | `0x20` | `{0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 20 | `0xfe00` | `{0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `floor_item_sprites` | 20 | `0xfc40` | `{0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `floor_item_sprites` | 20 | `0x400` | `{0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 20 | `0x400` | `{0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 21 | `0xd0` | `{0xd0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 21 | `0x0` | `{0xd0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 21 | `0x20` | `{0xd0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 21 | `0x20` | `{0xd0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 21 | `0xfe00` | `{0xd0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `floor_item_sprites` | 21 | `0xfc40` | `{0xd0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `floor_item_sprites` | 21 | `0x400` | `{0xd0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 21 | `0x400` | `{0xd0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 22 | `0xb0` | `{0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 22 | `0x0` | `{0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 22 | `0x20` | `{0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 22 | `0x20` | `{0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 22 | `0xfe00` | `{0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `floor_item_sprites` | 22 | `0xfc40` | `{0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `floor_item_sprites` | 22 | `0x400` | `{0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 22 | `0x400` | `{0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 23 | `0x90` | `{0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 23 | `0x20` | `{0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 23 | `0x20` | `{0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 23 | `0x27` | `{0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 23 | `0xfe00` | `{0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `floor_item_sprites` | 23 | `0xfb40` | `{0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `floor_item_sprites` | 23 | `0x400` | `{0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 23 | `0x500` | `{0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 24 | `0xb0` | `{0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 24 | `0x20` | `{0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 24 | `0x20` | `{0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 24 | `0x27` | `{0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 24 | `0xfe00` | `{0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `floor_item_sprites` | 24 | `0xfb40` | `{0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `floor_item_sprites` | 24 | `0x400` | `{0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 24 | `0x500` | `{0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 25 | `0xd0` | `{0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 25 | `0x20` | `{0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 25 | `0x20` | `{0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 25 | `0x27` | `{0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 25 | `0xfe00` | `{0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `floor_item_sprites` | 25 | `0xfb40` | `{0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `floor_item_sprites` | 25 | `0x400` | `{0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `floor_item_sprites` | 25 | `0x500` | `{0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 29 | `22` | `KfSpriteQuad effect_billboard_sprites[22] = {` | Twenty-two complete effect-sprite descriptors through the next distinct HUD layout, supported by constructor/frame selectors. |
| `effect_billboard_sprites` | 30 | `0x0` | `{0x0, 0x0, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 30 | `0x0` | `{0x0, 0x0, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 30 | `0x2f` | `{0x0, 0x0, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 30 | `0x2f` | `{0x0, 0x0, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 30 | `0xfe80` | `{0x0, 0x0, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 30 | `0xfe80` | `{0x0, 0x0, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 30 | `0x300` | `{0x0, 0x0, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 30 | `0x300` | `{0x0, 0x0, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 31 | `0x30` | `{0x30, 0x0, 0x2f, 0x2f, 0xfde7, 0xfde7, 0x433, 0x433},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 31 | `0x0` | `{0x30, 0x0, 0x2f, 0x2f, 0xfde7, 0xfde7, 0x433, 0x433},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 31 | `0x2f` | `{0x30, 0x0, 0x2f, 0x2f, 0xfde7, 0xfde7, 0x433, 0x433},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 31 | `0x2f` | `{0x30, 0x0, 0x2f, 0x2f, 0xfde7, 0xfde7, 0x433, 0x433},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 31 | `0xfde7` | `{0x30, 0x0, 0x2f, 0x2f, 0xfde7, 0xfde7, 0x433, 0x433},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 31 | `0xfde7` | `{0x30, 0x0, 0x2f, 0x2f, 0xfde7, 0xfde7, 0x433, 0x433},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 31 | `0x433` | `{0x30, 0x0, 0x2f, 0x2f, 0xfde7, 0xfde7, 0x433, 0x433},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 31 | `0x433` | `{0x30, 0x0, 0x2f, 0x2f, 0xfde7, 0xfde7, 0x433, 0x433},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 32 | `0x30` | `{0x30, 0x0, 0x2f, 0x2f, 0xfd74, 0xfd74, 0x519, 0x519},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 32 | `0x0` | `{0x30, 0x0, 0x2f, 0x2f, 0xfd74, 0xfd74, 0x519, 0x519},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 32 | `0x2f` | `{0x30, 0x0, 0x2f, 0x2f, 0xfd74, 0xfd74, 0x519, 0x519},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 32 | `0x2f` | `{0x30, 0x0, 0x2f, 0x2f, 0xfd74, 0xfd74, 0x519, 0x519},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 32 | `0xfd74` | `{0x30, 0x0, 0x2f, 0x2f, 0xfd74, 0xfd74, 0x519, 0x519},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 32 | `0xfd74` | `{0x30, 0x0, 0x2f, 0x2f, 0xfd74, 0xfd74, 0x519, 0x519},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 32 | `0x519` | `{0x30, 0x0, 0x2f, 0x2f, 0xfd74, 0xfd74, 0x519, 0x519},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 32 | `0x519` | `{0x30, 0x0, 0x2f, 0x2f, 0xfd74, 0xfd74, 0x519, 0x519},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 33 | `0x60` | `{0x60, 0x0, 0x2f, 0x2f, 0xfd4d, 0xfd4d, 0x566, 0x566},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 33 | `0x0` | `{0x60, 0x0, 0x2f, 0x2f, 0xfd4d, 0xfd4d, 0x566, 0x566},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 33 | `0x2f` | `{0x60, 0x0, 0x2f, 0x2f, 0xfd4d, 0xfd4d, 0x566, 0x566},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 33 | `0x2f` | `{0x60, 0x0, 0x2f, 0x2f, 0xfd4d, 0xfd4d, 0x566, 0x566},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 33 | `0xfd4d` | `{0x60, 0x0, 0x2f, 0x2f, 0xfd4d, 0xfd4d, 0x566, 0x566},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 33 | `0xfd4d` | `{0x60, 0x0, 0x2f, 0x2f, 0xfd4d, 0xfd4d, 0x566, 0x566},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 33 | `0x566` | `{0x60, 0x0, 0x2f, 0x2f, 0xfd4d, 0xfd4d, 0x566, 0x566},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 33 | `0x566` | `{0x60, 0x0, 0x2f, 0x2f, 0xfd4d, 0xfd4d, 0x566, 0x566},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 34 | `0x60` | `{0x60, 0x0, 0x2f, 0x2f, 0xfd3a, 0xfd3a, 0x58c, 0x58c},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 34 | `0x0` | `{0x60, 0x0, 0x2f, 0x2f, 0xfd3a, 0xfd3a, 0x58c, 0x58c},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 34 | `0x2f` | `{0x60, 0x0, 0x2f, 0x2f, 0xfd3a, 0xfd3a, 0x58c, 0x58c},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 34 | `0x2f` | `{0x60, 0x0, 0x2f, 0x2f, 0xfd3a, 0xfd3a, 0x58c, 0x58c},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 34 | `0xfd3a` | `{0x60, 0x0, 0x2f, 0x2f, 0xfd3a, 0xfd3a, 0x58c, 0x58c},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 34 | `0xfd3a` | `{0x60, 0x0, 0x2f, 0x2f, 0xfd3a, 0xfd3a, 0x58c, 0x58c},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 34 | `0x58c` | `{0x60, 0x0, 0x2f, 0x2f, 0xfd3a, 0xfd3a, 0x58c, 0x58c},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 34 | `0x58c` | `{0x60, 0x0, 0x2f, 0x2f, 0xfd3a, 0xfd3a, 0x58c, 0x58c},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 35 | `0x0` | `{0x0, 0x30, 0x2f, 0x2f, 0xfe34, 0xfe34, 0x399, 0x399},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 35 | `0x30` | `{0x0, 0x30, 0x2f, 0x2f, 0xfe34, 0xfe34, 0x399, 0x399},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 35 | `0x2f` | `{0x0, 0x30, 0x2f, 0x2f, 0xfe34, 0xfe34, 0x399, 0x399},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 35 | `0x2f` | `{0x0, 0x30, 0x2f, 0x2f, 0xfe34, 0xfe34, 0x399, 0x399},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 35 | `0xfe34` | `{0x0, 0x30, 0x2f, 0x2f, 0xfe34, 0xfe34, 0x399, 0x399},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 35 | `0xfe34` | `{0x0, 0x30, 0x2f, 0x2f, 0xfe34, 0xfe34, 0x399, 0x399},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 35 | `0x399` | `{0x0, 0x30, 0x2f, 0x2f, 0xfe34, 0xfe34, 0x399, 0x399},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 35 | `0x399` | `{0x0, 0x30, 0x2f, 0x2f, 0xfe34, 0xfe34, 0x399, 0x399},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 36 | `0x0` | `{0x0, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 36 | `0x60` | `{0x0, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 36 | `0x2f` | `{0x0, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 36 | `0x2f` | `{0x0, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 36 | `0xfe80` | `{0x0, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 36 | `0xfe80` | `{0x0, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 36 | `0x300` | `{0x0, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 36 | `0x300` | `{0x0, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 37 | `0x30` | `{0x30, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 37 | `0x60` | `{0x30, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 37 | `0x2f` | `{0x30, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 37 | `0x2f` | `{0x30, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 37 | `0xfe80` | `{0x30, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 37 | `0xfe80` | `{0x30, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 37 | `0x300` | `{0x30, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 37 | `0x300` | `{0x30, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 38 | `0x60` | `{0x60, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 38 | `0x30` | `{0x60, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 38 | `0x2f` | `{0x60, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 38 | `0x2f` | `{0x60, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 38 | `0xfe80` | `{0x60, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 38 | `0xfe80` | `{0x60, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 38 | `0x300` | `{0x60, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 38 | `0x300` | `{0x60, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 39 | `0x30` | `{0x30, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 39 | `0x30` | `{0x30, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 39 | `0x2f` | `{0x30, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 39 | `0x2f` | `{0x30, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 39 | `0xfe80` | `{0x30, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 39 | `0xfe80` | `{0x30, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 39 | `0x300` | `{0x30, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 39 | `0x300` | `{0x30, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 40 | `0x60` | `{0x60, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 40 | `0x60` | `{0x60, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 40 | `0x2f` | `{0x60, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 40 | `0x2f` | `{0x60, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 40 | `0xfe80` | `{0x60, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 40 | `0xfe80` | `{0x60, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 40 | `0x300` | `{0x60, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 40 | `0x300` | `{0x60, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 41 | `0xe0` | `{0xe0, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 41 | `0x48` | `{0xe0, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 41 | `0x17` | `{0xe0, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 41 | `0x5c` | `{0xe0, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 41 | `0xff38` | `{0xe0, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 41 | `0xec78` | `{0xe0, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 41 | `0x190` | `{0xe0, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 41 | `0x1388` | `{0xe0, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 42 | `0xe0` | `{0xe0, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 42 | `0x76` | `{0xe0, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 42 | `0x17` | `{0xe0, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 42 | `0x5c` | `{0xe0, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 42 | `0xff38` | `{0xe0, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 42 | `0xec78` | `{0xe0, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 42 | `0x190` | `{0xe0, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 42 | `0x1388` | `{0xe0, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 43 | `0xe0` | `{0xe0, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 43 | `0xa2` | `{0xe0, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 43 | `0x17` | `{0xe0, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 43 | `0x5c` | `{0xe0, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 43 | `0xff38` | `{0xe0, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 43 | `0xec78` | `{0xe0, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 43 | `0x190` | `{0xe0, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 43 | `0x1388` | `{0xe0, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 44 | `0xe0` | `{0xe0, 0x48, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 44 | `0x48` | `{0xe0, 0x48, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 44 | `0x17` | `{0xe0, 0x48, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 44 | `0x5c` | `{0xe0, 0x48, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 44 | `0xfed4` | `{0xe0, 0x48, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 44 | `0xec78` | `{0xe0, 0x48, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 44 | `0x258` | `{0xe0, 0x48, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 44 | `0x1388` | `{0xe0, 0x48, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 45 | `0xe0` | `{0xe0, 0x76, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 45 | `0x76` | `{0xe0, 0x76, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 45 | `0x17` | `{0xe0, 0x76, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 45 | `0x5c` | `{0xe0, 0x76, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 45 | `0xfed4` | `{0xe0, 0x76, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 45 | `0xec78` | `{0xe0, 0x76, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 45 | `0x258` | `{0xe0, 0x76, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 45 | `0x1388` | `{0xe0, 0x76, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 46 | `0xe0` | `{0xe0, 0xa2, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 46 | `0xa2` | `{0xe0, 0xa2, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 46 | `0x17` | `{0xe0, 0xa2, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 46 | `0x5c` | `{0xe0, 0xa2, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 46 | `0xfed4` | `{0xe0, 0xa2, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 46 | `0xec78` | `{0xe0, 0xa2, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 46 | `0x258` | `{0xe0, 0xa2, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 46 | `0x1388` | `{0xe0, 0xa2, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 47 | `0x0` | `{0x0, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 47 | `0x90` | `{0x0, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 47 | `0x2f` | `{0x0, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 47 | `0x2f` | `{0x0, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 47 | `0xfe80` | `{0x0, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 47 | `0xfe80` | `{0x0, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 47 | `0x300` | `{0x0, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 47 | `0x300` | `{0x0, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 48 | `0x30` | `{0x30, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 48 | `0x90` | `{0x30, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 48 | `0x2f` | `{0x30, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 48 | `0x2f` | `{0x30, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 48 | `0xfe80` | `{0x30, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 48 | `0xfe80` | `{0x30, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 48 | `0x300` | `{0x30, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 48 | `0x300` | `{0x30, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 49 | `0xc8` | `{0xc8, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 49 | `0x48` | `{0xc8, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 49 | `0x17` | `{0xc8, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 49 | `0x5c` | `{0xc8, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 49 | `0xff38` | `{0xc8, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 49 | `0xec78` | `{0xc8, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 49 | `0x190` | `{0xc8, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 49 | `0x1388` | `{0xc8, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 50 | `0xc8` | `{0xc8, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 50 | `0x76` | `{0xc8, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 50 | `0x17` | `{0xc8, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 50 | `0x5c` | `{0xc8, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 50 | `0xff38` | `{0xc8, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 50 | `0xec78` | `{0xc8, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 50 | `0x190` | `{0xc8, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 50 | `0x1388` | `{0xc8, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 51 | `0xc8` | `{0xc8, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture U origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 51 | `0xa2` | `{0xc8, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture V origin; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 51 | `0x17` | `{0xc8, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture U span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 51 | `0x5c` | `{0xc8, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored texture V span; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 51 | `0xff38` | `{0xc8, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local X origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 51 | `0xec78` | `{0xc8, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local Y origin; preserve the unsigned halfword bit pattern and its later signed-corner conversion. |
| `effect_billboard_sprites` | 51 | `0x190` | `{0xc8, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local width; retain the exact atlas placement or rectangle span for this descriptor. |
| `effect_billboard_sprites` | 51 | `0x1388` | `{0xc8, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},` | Authored local height; retain the exact atlas placement or rectangle span for this descriptor. |
| `render_floor_item` | 79 | `0` | `if ((u8)facing != 0) {` | Zero packed facing selects billboard orientation; nonzero facing uses the named fixed-yaw conversion. |
| `render_floor_item` | 94 | `1` | `next_frame = item->animation_frame + 1;` | Advance the animation by one frame before its original byte-wrap comparison. |
| `render_floor_item` | 97 | `0xff` | `if ((next_frame & 0xff) >= (frame_count & KF_FLOOR_ITEM_FRAME_COUNT_MASK)) {` | The full byte mask preserves modulo-256 narrowing before comparing with the packed low-nibble frame count. |
| `render_floor_item` | 98 | `0` | `item->animation_frame = 0;` | Reset an exhausted animation to its first frame; frame zero is an ordinal, not an independent mode. |
| `render_actor_sprite` | 137 | `0` | `render_enqueue_sprite(&effect_billboard_sprites[sprite->sprite_id], 0, KF_SPRITE_DEPTH_CUE_NORMAL);` | Add no caller depth bias to the projected effect billboard. |
| `render_actor_sprite` | 144 | `0` | `object = tmd_get_object(0);` | Select the first object in the already selected TMD asset; asset identity is handled separately. |
| `render_actor_sprite` | 147 | `0` | `object->vertex_count) == 0) {` | Zero reports that no animated binding was supplied, selecting the original static-vertex fallback. |
| `render_actor_sprite` | 148 | `0` | `tmd_select_object_vertices(0);` | Bind the first TMD object vertex array in the static fallback. |
| `render_actor_sprite` | 149 | `0` | `tmd_project_vertices(tmd_get_object(0)->vertex_count);` | Read the first TMD object count for the static projection fallback. |
| `render_actor_sprite` | 153 | `0` | `render_enqueue_tmd(0, EFFECT_MODEL_DEPTH_BIAS);` | Enqueue the first object of the selected effect asset with the separately named depth bias. |

## `src/game/map_event_render.c`

6 retained occurrences.

| Function / data owner | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `render_map_event` | 41 | `0` | `object = tmd_get_object(0);` | Select the first object in the already selected TMD asset; asset identity is handled separately. |
| `render_map_event` | 44 | `0` | `object->vertex_count) == 0) {` | Zero reports that no animated binding was supplied, selecting the original static-vertex fallback. |
| `render_map_event` | 45 | `0` | `tmd_select_object_vertices(0);` | Bind the first TMD object vertex array in the static fallback. |
| `render_map_event` | 46 | `0` | `tmd_project_vertices(tmd_get_object(0)->vertex_count);` | Read the first TMD object count for the static projection fallback. |
| `render_map_event` | 50 | `0` | `render_enqueue_tmd(0, 0);` | Select the first object of the bound map-event asset. |
| `render_map_event` | 50 | `0` | `render_enqueue_tmd(0, 0);` | Add no caller depth bias to the map-event model. |

## `src/game/render_sprite.c`

0 retained occurrences.
