# Retained equipment literals

Complete current ledger for `equipment.c`, `menu_select.c` and
`menu_draw_name_list.c` after the [confirmation/layout review](game-equipment-confirm-layout.md).
All **101 retained occurrences** have an individual row and reason. Duplicate
tokens remain separate; line numbers locate this version, while the complete
expression and multiplicity establish coverage. Claims, definitions and string
contents are excluded.

The [equipment identities](game-item-equipment-identities.md),
[selected-spell types](game-selected-magic-types.md),
[learning-state domain](game-magic-learning-state.md) and
[shared dimensions](game-item-menu-dimensions.md) provide the existing domain
evidence. Encoded glyphs retain the [retail-font interpretation](game-shop-price-domains.md).
The [stock-bank](item-stock-banks.md) and [item-ID](item-id-domain.md) reviews
now name bank indices and propagate inventory IDs through equipment storage
and menu boundaries. Broader source-domain review remains open.

## `src/game/equipment.c`

6 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `weapon_records_load_and_mirror_angles` | 15 | `0` | `} while (--remaining != 0);` | Pre-decrement word-copy countdown stops at zero after the complete declared record array. |
| `weapon_records_load_and_mirror_angles` | 17 | `1` | `remaining = KF_WEAPON_RECORD_COUNT - 1;` | Inclusive record countdown starts at count minus one so the post-decrement loop visits all sixteen records. |
| `weapon_records_load_and_mirror_angles` | 22 | `0` | `} while (remaining-- != 0);` | Post-decrement termination includes record countdown zero; the final decrement occurs after its body. |
| `armor_records_load` | 34 | `0` | `} while (--remaining != 0);` | Pre-decrement word-copy countdown stops at zero after the complete declared record array. |
| `fixed6_ratio_step` | 40 | `1` | `return (value << KF_FIXED6_BITS) / (span + 1) + 1;` | Multiply the numerator by 64 before signed division. For the reviewed bounded nonnegative stat/rate inputs, denominator plus one avoids zero and final plus one supplies a positive minimum step. Weapon charge uses it directly; magic charge doubles it. The helper name identifies the six-bit scaling; the original choice of 64 is unproven. |
| `fixed6_ratio_step` | 40 | `1` | `return (value << KF_FIXED6_BITS) / (span + 1) + 1;` | Multiply the numerator by 64 before signed division. For the reviewed bounded nonnegative stat/rate inputs, denominator plus one avoids zero and final plus one supplies a positive minimum step. Weapon charge uses it directly; magic charge doubles it. The helper name identifies the six-bit scaling; the original choice of 64 is unproven. |

## `src/game/menu_select.c`

95 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `menu_equip_select` | 28 | `20` | `s16 labels[20][MENU_GLYPHS_PER_ROW];` | Twenty-row local workspace with the shared named glyph width. Equipment needs at most fourteen entries including unequip, and ranged magic at most six; original over-allocation rationale is unproven. |
| `menu_equip_select` | 29 | `20` | `KfItemId codes[20];` | Parallel twenty-entry code workspace, indexed with the label rows; original capacity choice is unproven. |
| `menu_equip_select` | 38 | `0` | `s32 input = 0;` | Initial previous input state has no pressed buttons for edge detection. |
| `menu_equip_select` | 42 | `1` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_equip_select` | 42 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_equip_select` | 77 | `0` | `k = 0;` | Start appending selected rows at the first workspace entry. |
| `menu_equip_select` | 79 | `0` | `if (owned[i] != 0) {` | Any nonzero owned quantity includes that item in the selection list. |
| `menu_equip_select` | 81 | `0` | `for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)` | Copy all ten glyph halfwords, starting at index zero, from the shared fixed-width name row. |
| `menu_equip_select` | 87 | `0` | `labels[k][0] = 0x59;` | First glyph in はずす (unequip), decoded from the retail font in game-shop-price-domains.md; atlas code 89. |
| `menu_equip_select` | 87 | `0x59` | `labels[k][0] = 0x59;` | First glyph in はずす (unequip), decoded from the retail font in game-shop-price-domains.md; atlas code 89. |
| `menu_equip_select` | 88 | `1` | `labels[k][1] = MENU_TEXT_DAKUTEN \| 0x4c;` | Second glyph position: authored atlas code 76 plus the named dakuten bit. |
| `menu_equip_select` | 88 | `0x4c` | `labels[k][1] = MENU_TEXT_DAKUTEN \| 0x4c;` | Second glyph position: authored atlas code 76 plus the named dakuten bit. |
| `menu_equip_select` | 89 | `2` | `labels[k][2] = 0x4c;` | Third glyph position repeats atlas code 76 without dakuten. |
| `menu_equip_select` | 89 | `0x4c` | `labels[k][2] = 0x4c;` | Third glyph position repeats atlas code 76 without dakuten. |
| `menu_equip_select` | 90 | `3` | `labels[k][3] = MENU_TEXT_END;` | Terminator position immediately after the three authored none-label glyphs. |
| `menu_equip_select` | 97 | `0` | `ctx.glyph_rows = &labels[0][0];` | Base address of the first glyph in the first row for the flat list-render API. |
| `menu_equip_select` | 97 | `0` | `ctx.glyph_rows = &labels[0][0];` | Base address of the first glyph in the first row for the flat list-render API. |
| `menu_equip_select` | 98 | `0` | `ctx.quantities = 0;` | Null quantity list: selection panels display names without stock counts. |
| `menu_equip_select` | 100 | `0` | `if (ctx.entry_count != 0) {` | Only preview/render an item when the list has entries. |
| `menu_equip_select` | 101 | `0` | `if (menu_load_item_model(codes[ctx.selected_index]) != 0)` | Any nonzero model-loader result aborts this panel; empty item ID is a successful no-load path. |
| `menu_equip_select` | 108 | `0` | `KF_MENU_PREVIEW_ITEM_MODEL, KF_ENUM_ENCODE(u8, codes[ctx.selected_index]), 0, KF_ITEM_PRICE_BUY)` | The shop index is an unused zero for model/icon previews; the named BUY price mode is also unused on these paths. |
| `menu_equip_select` | 116 | `1` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_equip_select` | 116 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_equip_select` | 122 | `1` | `input = PadRead(1);` | Ignored retail PadRead call-site argument; the linked routine uses its global pad identifier. |
| `menu_equip_select` | 123 | `0` | `if (ctx.entry_count == 0) {` | Empty-list input path, retained even though the current construction appends a none entry. |
| `menu_equip_select` | 124 | `0` | `if (input != 0) {` | Any pressed input acknowledges the empty-list path. |
| `menu_equip_select` | 128 | `0` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 128 | `0` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 130 | `0` | `if (ctx.selected_index != 0) {` | First list-entry boundary for upward cursor movement. |
| `menu_equip_select` | 132 | `0` | `if (ctx.cursor_row == 0)` | Top visible row: scrolling upward changes the window instead of its cursor row. |
| `menu_equip_select` | 137 | `1` | `ctx.selected_index = ctx.entry_count - 1;` | Wrap to the last zero-based entry. |
| `menu_equip_select` | 139 | `0` | `ctx.scroll_offset = 0;` | Reset the scroll window to its first entry. |
| `menu_equip_select` | 140 | `1` | `ctx.cursor_row = ctx.entry_count - 1;` | Bottom row of a list shorter than the visible window. |
| `menu_equip_select` | 143 | `1` | `ctx.cursor_row = ctx.visible_rows - 1;` | Last zero-based visible cursor row. |
| `menu_equip_select` | 146 | `0` | `if (menu_load_item_model(codes[ctx.selected_index]) != 0)` | Any nonzero model-loader result aborts this panel; empty item ID is a successful no-load path. |
| `menu_equip_select` | 148 | `0` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 148 | `0` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 150 | `1` | `if (ctx.selected_index < ctx.entry_count - 1) {` | Downward movement stops at the last zero-based entry before wrapping. |
| `menu_equip_select` | 152 | `1` | `if (ctx.cursor_row == ctx.visible_rows - 1)` | At the bottom visible row, downward movement scrolls the window. |
| `menu_equip_select` | 157 | `0` | `ctx.selected_index = 0;` | Wrap selection back to the first entry. |
| `menu_equip_select` | 158 | `0` | `ctx.scroll_offset = 0;` | Reset the scroll window to its first entry. |
| `menu_equip_select` | 159 | `0` | `ctx.cursor_row = 0;` | Reset the visible cursor to the top row. |
| `menu_equip_select` | 161 | `0` | `if (menu_load_item_model(codes[ctx.selected_index]) != 0)` | Any nonzero model-loader result aborts this panel; empty item ID is a successful no-load path. |
| `menu_equip_select` | 163 | `0` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 163 | `0` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 166 | `0` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 166 | `0` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 172 | `0` | `if (ctx.entry_count != 0)` | Only preview an item when the list has entries. |
| `menu_spell_select` | 228 | `20` | `s16 labels[20][MENU_GLYPHS_PER_ROW];` | Twenty-row local workspace with the shared named glyph width. Equipment needs at most fourteen entries including unequip, and ranged magic at most six; original over-allocation rationale is unproven. |
| `menu_spell_select` | 229 | `20` | `KfSelectedMagicId codes[20];` | Twenty byte-sized selected-spell entries; preserves the evidenced workspace capacity while rejecting unrelated enum values in modern compilation. Original over-allocation rationale unknown. |
| `menu_spell_select` | 234 | `0` | `s32 input = 0;` | Initial previous input state has no pressed buttons for edge detection. |
| `menu_spell_select` | 238 | `1` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_spell_select` | 238 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_spell_select` | 241 | `0` | `k = 0;` | Start appending selected rows at the first workspace entry. |
| `menu_spell_select` | 244 | `0` | `for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)` | Copy all ten glyph halfwords, starting at index zero, from the shared fixed-width name row. |
| `menu_spell_select` | 250 | `0` | `labels[k][0] = 0x59;` | First glyph in はずす (unequip), decoded from the retail font in game-shop-price-domains.md; atlas code 89. |
| `menu_spell_select` | 250 | `0x59` | `labels[k][0] = 0x59;` | First glyph in はずす (unequip), decoded from the retail font in game-shop-price-domains.md; atlas code 89. |
| `menu_spell_select` | 251 | `1` | `labels[k][1] = MENU_TEXT_DAKUTEN \| 0x4c;` | Second glyph position: authored atlas code 76 plus the named dakuten bit. |
| `menu_spell_select` | 251 | `0x4c` | `labels[k][1] = MENU_TEXT_DAKUTEN \| 0x4c;` | Second glyph position: authored atlas code 76 plus the named dakuten bit. |
| `menu_spell_select` | 252 | `2` | `labels[k][2] = 0x4c;` | Third glyph position repeats atlas code 76 without dakuten. |
| `menu_spell_select` | 252 | `0x4c` | `labels[k][2] = 0x4c;` | Third glyph position repeats atlas code 76 without dakuten. |
| `menu_spell_select` | 253 | `3` | `labels[k][3] = MENU_TEXT_END;` | Terminator position immediately after the three authored none-label glyphs. |
| `menu_spell_select` | 260 | `0` | `ctx.glyph_rows = &labels[0][0];` | Base address of the first glyph in the first row for the flat list-render API. |
| `menu_spell_select` | 260 | `0` | `ctx.glyph_rows = &labels[0][0];` | Base address of the first glyph in the first row for the flat list-render API. |
| `menu_spell_select` | 261 | `0` | `ctx.quantities = 0;` | Null quantity list: selection panels display names without stock counts. |
| `menu_spell_select` | 264 | `0` | `if (ctx.entry_count != 0) {` | Only preview/render an item when the list has entries. |
| `menu_spell_select` | 265 | `1` | `if (menu_load_item_texture(KF_ENUM_ENCODE(u8, codes[ctx.selected_index])) == 1)` | Numbered-texture loader returns 1 on load failure; retain the equality check. |
| `menu_spell_select` | 276 | `0` | `KF_MENU_PREVIEW_MAGIC_ICON, KF_ENUM_ENCODE(s32, codes[ctx.selected_index]), 0, KF_ITEM_PRICE_BUY));` | The shop index is an unused zero for model/icon previews; the named BUY price mode is also unused on these paths. |
| `menu_spell_select` | 283 | `1` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_spell_select` | 283 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_spell_select` | 291 | `1` | `input = PadRead(1);` | Ignored retail PadRead call-site argument; the linked routine uses its global pad identifier. |
| `menu_spell_select` | 292 | `0` | `if (ctx.entry_count == 0) {` | Empty-list input path, retained even though the current construction appends a none entry. |
| `menu_spell_select` | 293 | `0` | `if (input != 0) {` | Any pressed input acknowledges the empty-list path. |
| `menu_spell_select` | 297 | `0` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 297 | `0` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 299 | `0` | `if (ctx.selected_index != 0) {` | First list-entry boundary for upward cursor movement. |
| `menu_spell_select` | 301 | `0` | `if (ctx.cursor_row == 0)` | Top visible row: scrolling upward changes the window instead of its cursor row. |
| `menu_spell_select` | 306 | `1` | `ctx.selected_index = ctx.entry_count - 1;` | Wrap to the last zero-based entry. |
| `menu_spell_select` | 308 | `0` | `ctx.scroll_offset = 0;` | Reset the scroll window to its first entry. |
| `menu_spell_select` | 309 | `1` | `ctx.cursor_row = ctx.entry_count - 1;` | Bottom row of a list shorter than the visible window. |
| `menu_spell_select` | 312 | `1` | `ctx.cursor_row = ctx.visible_rows - 1;` | Last zero-based visible cursor row. |
| `menu_spell_select` | 315 | `1` | `if (menu_load_item_texture(KF_ENUM_ENCODE(u8, codes[ctx.selected_index])) == 1)` | Numbered-texture loader returns 1 on load failure; retain the equality check. |
| `menu_spell_select` | 317 | `0` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 317 | `0` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 319 | `1` | `if (ctx.selected_index < ctx.entry_count - 1) {` | Downward movement stops at the last zero-based entry before wrapping. |
| `menu_spell_select` | 321 | `1` | `if (ctx.cursor_row == ctx.visible_rows - 1)` | At the bottom visible row, downward movement scrolls the window. |
| `menu_spell_select` | 326 | `0` | `ctx.selected_index = 0;` | Wrap selection back to the first entry. |
| `menu_spell_select` | 327 | `0` | `ctx.scroll_offset = 0;` | Reset the scroll window to its first entry. |
| `menu_spell_select` | 328 | `0` | `ctx.cursor_row = 0;` | Reset the visible cursor to the top row. |
| `menu_spell_select` | 330 | `1` | `if (menu_load_item_texture(KF_ENUM_ENCODE(u8, codes[ctx.selected_index])) == 1)` | Numbered-texture loader returns 1 on load failure; retain the equality check. |
| `menu_spell_select` | 332 | `0` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 332 | `0` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 335 | `0` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 335 | `0` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 340 | `0` | `if (ctx.entry_count != 0) {` | Only preview/render an item when the list has entries. |

## `src/game/menu_draw_name_list.c`

0 retained occurrences.

All coordinate settings and row advances now use named constants; no retained numeric or character tokens remain.

The [shared confirmation-state review](menu-confirm-state.md) names the request
flag and preserves its type through both drawing APIs. This ledger reflects
those current expressions; other retained selector domains still need review.
