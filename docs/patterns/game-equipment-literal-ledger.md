# Retained equipment literals

Complete current ledger for `equipment.c`, `menu_select.c` and
`menu_draw_name_list.c` after the [confirmation/layout review](game-equipment-confirm-layout.md).
All **103 retained occurrences** have an individual row and reason. Duplicate
tokens remain separate; line numbers locate this version, while the complete
expression and multiplicity establish coverage. Claims, definitions and string
contents are excluded.

The [equipment identities](game-item-equipment-identities.md),
[selected-spell types](game-selected-magic-types.md),
[learning-state domain](game-magic-learning-state.md) and
[shared dimensions](game-item-menu-dimensions.md) provide the existing domain
evidence. Encoded glyphs retain the [retail-font interpretation](game-shop-price-domains.md).
Player stock-bank naming and broader item-ID typing remain separate work;
this ledger does not claim that all domain propagation is complete.

## `src/game/equipment.c`

7 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `weapon_records_load_and_mirror_angles` | 15 | `0` | `} while (--remaining != 0);` | Pre-decrement word-copy countdown stops at zero after the complete declared record array. |
| `weapon_records_load_and_mirror_angles` | 17 | `1` | `remaining = KF_WEAPON_RECORD_COUNT - 1;` | Inclusive record countdown starts at count minus one so the post-decrement loop visits all sixteen records. |
| `weapon_records_load_and_mirror_angles` | 22 | `0` | `} while (remaining-- != 0);` | Post-decrement termination includes record countdown zero; the final decrement occurs after its body. |
| `armor_records_load` | 34 | `0` | `} while (--remaining != 0);` | Pre-decrement word-copy countdown stops at zero after the complete declared record array. |
| `fixed6_ratio_step` | 40 | `6` | `return (value << 6) / (span + 1) + 1;` | Multiply the numerator by 64 before signed division. For the reviewed bounded nonnegative stat/rate inputs, denominator plus one avoids zero and final plus one supplies a positive minimum step. Weapon charge uses it directly; magic charge doubles it. The helper name identifies the six-bit scaling; the original choice of 64 is unproven. |
| `fixed6_ratio_step` | 40 | `1` | `return (value << 6) / (span + 1) + 1;` | Multiply the numerator by 64 before signed division. For the reviewed bounded nonnegative stat/rate inputs, denominator plus one avoids zero and final plus one supplies a positive minimum step. Weapon charge uses it directly; magic charge doubles it. The helper name identifies the six-bit scaling; the original choice of 64 is unproven. |
| `fixed6_ratio_step` | 40 | `1` | `return (value << 6) / (span + 1) + 1;` | Multiply the numerator by 64 before signed division. For the reviewed bounded nonnegative stat/rate inputs, denominator plus one avoids zero and final plus one supplies a positive minimum step. Weapon charge uses it directly; magic charge doubles it. The helper name identifies the six-bit scaling; the original choice of 64 is unproven. |

## `src/game/menu_select.c`

96 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `menu_equip_select` | 33 | `20` | `s16 labels[20][MENU_GLYPHS_PER_ROW];` | Twenty-row local workspace with the shared named glyph width. Equipment needs at most fourteen entries including unequip, and ranged magic at most six; original over-allocation rationale is unproven. |
| `menu_equip_select` | 34 | `20` | `u8 codes[20];` | Parallel twenty-entry code workspace, indexed with the label rows; original capacity choice is unproven. |
| `menu_equip_select` | 43 | `0` | `s32 input = 0;` | Initial previous input state has no pressed buttons for edge detection. |
| `menu_equip_select` | 47 | `1` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_equip_select` | 47 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_equip_select` | 50 | `0` | `owned = item_stock[0];` | Player-owned stock bank zero, acquired after the release wait and before the category switch; its item index follows the category bounds. |
| `menu_equip_select` | 82 | `0` | `k = 0;` | Start appending selected rows at the first workspace entry. |
| `menu_equip_select` | 84 | `0` | `if (owned[i] != 0) {` | Any nonzero owned quantity includes that item in the selection list. |
| `menu_equip_select` | 86 | `0` | `for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)` | Copy all ten glyph halfwords, starting at index zero, from the shared fixed-width name row. |
| `menu_equip_select` | 92 | `0` | `labels[k][0] = 0x59;` | First glyph in はずす (unequip), decoded from the retail font in game-shop-price-domains.md; atlas code 89. |
| `menu_equip_select` | 92 | `0x59` | `labels[k][0] = 0x59;` | First glyph in はずす (unequip), decoded from the retail font in game-shop-price-domains.md; atlas code 89. |
| `menu_equip_select` | 93 | `1` | `labels[k][1] = MENU_TEXT_DAKUTEN \| 0x4c;` | Second glyph position: authored atlas code 76 plus the named dakuten bit. |
| `menu_equip_select` | 93 | `0x4c` | `labels[k][1] = MENU_TEXT_DAKUTEN \| 0x4c;` | Second glyph position: authored atlas code 76 plus the named dakuten bit. |
| `menu_equip_select` | 94 | `2` | `labels[k][2] = 0x4c;` | Third glyph position repeats atlas code 76 without dakuten. |
| `menu_equip_select` | 94 | `0x4c` | `labels[k][2] = 0x4c;` | Third glyph position repeats atlas code 76 without dakuten. |
| `menu_equip_select` | 95 | `3` | `labels[k][3] = MENU_TEXT_END;` | Terminator position immediately after the three authored none-label glyphs. |
| `menu_equip_select` | 102 | `0` | `ctx.glyph_rows = &labels[0][0];` | Base address of the first glyph in the first row for the flat list-render API. |
| `menu_equip_select` | 102 | `0` | `ctx.glyph_rows = &labels[0][0];` | Base address of the first glyph in the first row for the flat list-render API. |
| `menu_equip_select` | 103 | `0` | `ctx.quantities = 0;` | Null quantity list: selection panels display names without stock counts. |
| `menu_equip_select` | 105 | `0` | `if (ctx.entry_count != 0) {` | Only preview/render an item when the list has entries. |
| `menu_equip_select` | 106 | `0` | `if (menu_load_item_model(codes[ctx.selected_index]) != 0)` | Any nonzero model-loader result aborts this panel; empty item ID is a successful no-load path. |
| `menu_equip_select` | 113 | `0` | `KF_MENU_PREVIEW_ITEM_MODEL, codes[ctx.selected_index], 0, KF_ITEM_PRICE_BUY)` | The shop index is an unused zero for model/icon previews; the named BUY price mode is also unused on these paths. |
| `menu_equip_select` | 121 | `1` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_equip_select` | 121 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_equip_select` | 127 | `1` | `input = PadRead(1);` | Ignored retail PadRead call-site argument; the linked routine uses its global pad identifier. |
| `menu_equip_select` | 128 | `0` | `if (ctx.entry_count == 0) {` | Empty-list input path, retained even though the current construction appends a none entry. |
| `menu_equip_select` | 129 | `0` | `if (input != 0) {` | Any pressed input acknowledges the empty-list path. |
| `menu_equip_select` | 133 | `0` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 133 | `0` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 135 | `0` | `if (ctx.selected_index != 0) {` | First list-entry boundary for upward cursor movement. |
| `menu_equip_select` | 137 | `0` | `if (ctx.cursor_row == 0)` | Top visible row: scrolling upward changes the window instead of its cursor row. |
| `menu_equip_select` | 142 | `1` | `ctx.selected_index = ctx.entry_count - 1;` | Wrap to the last zero-based entry. |
| `menu_equip_select` | 144 | `0` | `ctx.scroll_offset = 0;` | Reset the scroll window to its first entry. |
| `menu_equip_select` | 145 | `1` | `ctx.cursor_row = ctx.entry_count - 1;` | Bottom row of a list shorter than the visible window. |
| `menu_equip_select` | 148 | `1` | `ctx.cursor_row = ctx.visible_rows - 1;` | Last zero-based visible cursor row. |
| `menu_equip_select` | 151 | `0` | `if (menu_load_item_model(codes[ctx.selected_index]) != 0)` | Any nonzero model-loader result aborts this panel; empty item ID is a successful no-load path. |
| `menu_equip_select` | 153 | `0` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 153 | `0` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 155 | `1` | `if (ctx.selected_index < ctx.entry_count - 1) {` | Downward movement stops at the last zero-based entry before wrapping. |
| `menu_equip_select` | 157 | `1` | `if (ctx.cursor_row == ctx.visible_rows - 1)` | At the bottom visible row, downward movement scrolls the window. |
| `menu_equip_select` | 162 | `0` | `ctx.selected_index = 0;` | Wrap selection back to the first entry. |
| `menu_equip_select` | 163 | `0` | `ctx.scroll_offset = 0;` | Reset the scroll window to its first entry. |
| `menu_equip_select` | 164 | `0` | `ctx.cursor_row = 0;` | Reset the visible cursor to the top row. |
| `menu_equip_select` | 166 | `0` | `if (menu_load_item_model(codes[ctx.selected_index]) != 0)` | Any nonzero model-loader result aborts this panel; empty item ID is a successful no-load path. |
| `menu_equip_select` | 168 | `0` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 168 | `0` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 171 | `0` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 171 | `0` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 177 | `0` | `if (ctx.entry_count != 0)` | Only preview an item when the list has entries. |
| `menu_spell_select` | 233 | `20` | `s16 labels[20][MENU_GLYPHS_PER_ROW];` | Twenty-row local workspace with the shared named glyph width. Equipment needs at most fourteen entries including unequip, and ranged magic at most six; original over-allocation rationale is unproven. |
| `menu_spell_select` | 234 | `20` | `KfSelectedMagicId codes[20];` | Twenty byte-sized selected-spell entries; preserves the evidenced workspace capacity while rejecting unrelated enum values in modern compilation. Original over-allocation rationale unknown. |
| `menu_spell_select` | 239 | `0` | `s32 input = 0;` | Initial previous input state has no pressed buttons for edge detection. |
| `menu_spell_select` | 243 | `1` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_spell_select` | 243 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_spell_select` | 246 | `0` | `k = 0;` | Start appending selected rows at the first workspace entry. |
| `menu_spell_select` | 249 | `0` | `for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)` | Copy all ten glyph halfwords, starting at index zero, from the shared fixed-width name row. |
| `menu_spell_select` | 255 | `0` | `labels[k][0] = 0x59;` | First glyph in はずす (unequip), decoded from the retail font in game-shop-price-domains.md; atlas code 89. |
| `menu_spell_select` | 255 | `0x59` | `labels[k][0] = 0x59;` | First glyph in はずす (unequip), decoded from the retail font in game-shop-price-domains.md; atlas code 89. |
| `menu_spell_select` | 256 | `1` | `labels[k][1] = MENU_TEXT_DAKUTEN \| 0x4c;` | Second glyph position: authored atlas code 76 plus the named dakuten bit. |
| `menu_spell_select` | 256 | `0x4c` | `labels[k][1] = MENU_TEXT_DAKUTEN \| 0x4c;` | Second glyph position: authored atlas code 76 plus the named dakuten bit. |
| `menu_spell_select` | 257 | `2` | `labels[k][2] = 0x4c;` | Third glyph position repeats atlas code 76 without dakuten. |
| `menu_spell_select` | 257 | `0x4c` | `labels[k][2] = 0x4c;` | Third glyph position repeats atlas code 76 without dakuten. |
| `menu_spell_select` | 258 | `3` | `labels[k][3] = MENU_TEXT_END;` | Terminator position immediately after the three authored none-label glyphs. |
| `menu_spell_select` | 265 | `0` | `ctx.glyph_rows = &labels[0][0];` | Base address of the first glyph in the first row for the flat list-render API. |
| `menu_spell_select` | 265 | `0` | `ctx.glyph_rows = &labels[0][0];` | Base address of the first glyph in the first row for the flat list-render API. |
| `menu_spell_select` | 266 | `0` | `ctx.quantities = 0;` | Null quantity list: selection panels display names without stock counts. |
| `menu_spell_select` | 269 | `0` | `if (ctx.entry_count != 0) {` | Only preview/render an item when the list has entries. |
| `menu_spell_select` | 270 | `1` | `if (menu_load_item_texture(KF_ENUM_ENCODE(u8, codes[ctx.selected_index])) == 1)` | Numbered-texture loader returns 1 on load failure; retain the equality check. |
| `menu_spell_select` | 281 | `0` | `KF_MENU_PREVIEW_MAGIC_ICON, KF_ENUM_ENCODE(s32, codes[ctx.selected_index]), 0, KF_ITEM_PRICE_BUY)` | The shop index is an unused zero for model/icon previews; the named BUY price mode is also unused on these paths. |
| `menu_spell_select` | 288 | `1` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_spell_select` | 288 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_spell_select` | 296 | `1` | `input = PadRead(1);` | Ignored retail PadRead call-site argument; the linked routine uses its global pad identifier. |
| `menu_spell_select` | 297 | `0` | `if (ctx.entry_count == 0) {` | Empty-list input path, retained even though the current construction appends a none entry. |
| `menu_spell_select` | 298 | `0` | `if (input != 0) {` | Any pressed input acknowledges the empty-list path. |
| `menu_spell_select` | 302 | `0` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 302 | `0` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 304 | `0` | `if (ctx.selected_index != 0) {` | First list-entry boundary for upward cursor movement. |
| `menu_spell_select` | 306 | `0` | `if (ctx.cursor_row == 0)` | Top visible row: scrolling upward changes the window instead of its cursor row. |
| `menu_spell_select` | 311 | `1` | `ctx.selected_index = ctx.entry_count - 1;` | Wrap to the last zero-based entry. |
| `menu_spell_select` | 313 | `0` | `ctx.scroll_offset = 0;` | Reset the scroll window to its first entry. |
| `menu_spell_select` | 314 | `1` | `ctx.cursor_row = ctx.entry_count - 1;` | Bottom row of a list shorter than the visible window. |
| `menu_spell_select` | 317 | `1` | `ctx.cursor_row = ctx.visible_rows - 1;` | Last zero-based visible cursor row. |
| `menu_spell_select` | 320 | `1` | `if (menu_load_item_texture(KF_ENUM_ENCODE(u8, codes[ctx.selected_index])) == 1)` | Numbered-texture loader returns 1 on load failure; retain the equality check. |
| `menu_spell_select` | 322 | `0` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 322 | `0` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 324 | `1` | `if (ctx.selected_index < ctx.entry_count - 1) {` | Downward movement stops at the last zero-based entry before wrapping. |
| `menu_spell_select` | 326 | `1` | `if (ctx.cursor_row == ctx.visible_rows - 1)` | At the bottom visible row, downward movement scrolls the window. |
| `menu_spell_select` | 331 | `0` | `ctx.selected_index = 0;` | Wrap selection back to the first entry. |
| `menu_spell_select` | 332 | `0` | `ctx.scroll_offset = 0;` | Reset the scroll window to its first entry. |
| `menu_spell_select` | 333 | `0` | `ctx.cursor_row = 0;` | Reset the visible cursor to the top row. |
| `menu_spell_select` | 335 | `1` | `if (menu_load_item_texture(KF_ENUM_ENCODE(u8, codes[ctx.selected_index])) == 1)` | Numbered-texture loader returns 1 on load failure; retain the equality check. |
| `menu_spell_select` | 337 | `0` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 337 | `0` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 340 | `0` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 340 | `0` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 345 | `0` | `if (ctx.entry_count != 0) {` | Only preview/render an item when the list has entries. |

## `src/game/menu_draw_name_list.c`

0 retained occurrences.

All coordinate settings and row advances now use named constants; no retained numeric or character tokens remain.
