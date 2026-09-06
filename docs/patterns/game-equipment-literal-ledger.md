# Retained equipment literals

Complete ledger for `equipment.c`, `menu_select.c` and `menu_draw_name_list.c`
after the [equipment domain audit](game-equipment-domains.md). Claim addresses
and extents are separate. Negative signs are operators, so -99 contributes
one numeric token. Line numbers locate this source version.

All **155 retained occurrences** have specific reasons.

| Function | Lines | Tokens | Expression | Reason |
| --- | --- | --- | --- | --- |
| `weapon_records_load_and_mirror_angles` | 15 | `0` | `} while (--remaining != 0);` | Pre-decrement word-copy countdown stops at zero after the complete declared record array. |
| `weapon_records_load_and_mirror_angles` | 17 | `1` | `remaining = KF_WEAPON_RECORD_COUNT - 1;` | Inclusive record countdown starts at count minus one so the post-decrement loop visits all sixteen records. |
| `weapon_records_load_and_mirror_angles` | 22 | `0` | `} while (remaining-- != 0);` | Post-decrement termination includes record countdown zero; the final decrement occurs after its body. |
| `armor_records_load` | 34 | `0` | `} while (--remaining != 0);` | Pre-decrement word-copy countdown stops at zero after the complete declared record array. |
| `fixed6_ratio_step` | 40 | `6, 1 × 2` | `return (value << 6) / (span + 1) + 1;` | Multiply the stat numerator by 64 before integer division; denominator plus one and final plus one guarantee a positive step for nonnegative inputs. Weapon charge uses the result directly, magic charge doubles it. This is an authored charge-rate scale, not a later-decoded Q6 quantity; original choice of 64 is unproven. |
| `menu_equip_select` | 28 | `20, 10` | `s16 labels[20][10];` | Twenty-row local workspace with ten signed glyphs per row, matching the shared label format; current equipment ranges need at most fourteen rows including none, and spell selection at most six. Original over-allocation rationale is unknown. |
| `menu_equip_select` | 29 | `20` | `u8 codes[20];` | Parallel twenty-entry code workspace, indexed with the label rows; original capacity choice is unproven. |
| `menu_equip_select` | 37 | `0` | `s32 confirm = 0;` | Initially no pending confirmation; this is a UI highlight/request flag, not an equipment category. |
| `menu_equip_select` | 38 | `0` | `s32 input = 0;` | Initial previous input state has no pressed buttons for edge detection. |
| `menu_equip_select` | 40 | `99` | `s32 selection = -99;` | Pending-selection sentinel outside item/spell codes and cancellation -1; retained pending a shared list-result protocol audit. The choice of -99 is unproven. |
| `menu_equip_select` | 42, 114 | `1 × 2, 0 × 2` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_equip_select` | 76 | `0` | `k = 0;` | Start appending selected rows at the first workspace entry. |
| `menu_equip_select` | 77 | `0` | `owned = &item_stock[0][start];` | Player-owned stock bank zero, starting at the chosen category range. |
| `menu_equip_select` | 79 | `0` | `if (*owned != 0) {` | Any nonzero owned quantity includes that item in the selection list. |
| `menu_equip_select` | 81 | `0, 10` | `for (j = 0; j < 10; j++)` | Copy all ten glyph halfwords, starting at index zero, from the shared fixed-width name row. |
| `menu_equip_select` | 87 | `0, 0x59` | `labels[k][0] = 0x59;` | First glyph position in the authored none label; atlas code 89. Its localized identity is not independently decoded here. |
| `menu_equip_select` | 88 | `1, 0x4c` | `labels[k][1] = MENU_TEXT_DAKUTEN \| 0x4c;` | Second glyph position: authored atlas code 76 plus the named dakuten bit. |
| `menu_equip_select` | 89 | `2, 0x4c` | `labels[k][2] = 0x4c;` | Third glyph position repeats atlas code 76 without dakuten. |
| `menu_equip_select` | 90 | `3` | `labels[k][3] = MENU_TEXT_END;` | Terminator position immediately after the three authored none-label glyphs. |
| `menu_equip_select` | 94 | `1` | `menu_list_init(&ctx, 1, KF_ENUM_ENCODE(s32, category));` | Menu-window layout 1 is the equipment title bank; its row index follows the menu category, not the player slot. |
| `menu_equip_select` | 96 | `10` | `ctx.glyphs_per_entry = 10;` | The shared name-row representation has ten glyph halfwords per entry. |
| `menu_equip_select` | 97 | `0 × 2` | `ctx.glyph_rows = &labels[0][0];` | Base address of the first glyph in the first row for the flat list-render API. |
| `menu_equip_select` | 98 | `0` | `ctx.quantities = 0;` | Null quantity list: selection panels display names without stock counts. |
| `menu_equip_select` | 100 | `0` | `if (ctx.entry_count != 0) {` | Only preview/render an item when the list has entries. |
| `menu_equip_select` | 101, 144, 159 | `0 × 3` | `if (menu_load_item_model(codes[ctx.selected_index]) != 0)` | Any nonzero model-loader result aborts this panel; empty item ID is a successful no-load path. |
| `menu_equip_select` | 106 | `1` | `if (confirm == 1) {` | A set confirmation flag enters the second-stage confirmation widget. |
| `menu_equip_select` | 107 | `5, 0 × 3, 1` | `if (menu_list_interact(&ctx, 5, 0, codes[ctx.selected_index], 0, 0) == -1)` | Dialog kind 5 chooses the equip label, preview mode 0 uses the item model, and both price/detail arguments are unused zeros; -1 cancels the widget. These mixed mode/kind/result domains require the shared widget audit. |
| `menu_equip_select` | 108 | `99` | `selection = -99;` | Return to pending selection after the confirmation widget is cancelled; distinct from exiting the outer panel. |
| `menu_equip_select` | 112 | `0` | `confirm = 0;` | Clear the pending confirmation/highlight request for the next input frame. |
| `menu_equip_select` | 113 | `99` | `if (selection != -99) {` | Any completed selection or cancellation ends the modal loop; pending -99 continues it. |
| `menu_equip_select` | 120 | `1` | `input = PadRead(1);` | Ignored retail PadRead call-site argument; the linked routine uses its global pad identifier. |
| `menu_equip_select` | 121 | `0` | `if (ctx.entry_count == 0) {` | Empty-list input path, retained even though the current construction appends a none entry. |
| `menu_equip_select` | 122 | `0` | `if (input != 0) {` | Any pressed input acknowledges the empty-list path. |
| `menu_equip_select` | 124, 166 | `1 × 2` | `selection = -1;` | Outer panel cancellation, outside the nonnegative item IDs or row indices. |
| `menu_equip_select` | 126 | `0 × 2` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 128 | `0` | `if (ctx.selected_index != 0) {` | First list-entry boundary for upward cursor movement. |
| `menu_equip_select` | 130 | `0` | `if (ctx.cursor_row == 0)` | Top visible row: scrolling upward changes the window instead of its cursor row. |
| `menu_equip_select` | 135 | `1` | `ctx.selected_index = ctx.entry_count - 1;` | Wrap to the last zero-based entry. |
| `menu_equip_select` | 137, 156 | `0 × 2` | `ctx.scroll_offset = 0;` | Reset the scroll window to its first entry. |
| `menu_equip_select` | 138 | `1` | `ctx.cursor_row = ctx.entry_count - 1;` | Bottom row of a list shorter than the visible window. |
| `menu_equip_select` | 141 | `1` | `ctx.cursor_row = ctx.visible_rows - 1;` | Last zero-based visible cursor row. |
| `menu_equip_select` | 146 | `0 × 2` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 148 | `1` | `if (ctx.selected_index < ctx.entry_count - 1) {` | Downward movement stops at the last zero-based entry before wrapping. |
| `menu_equip_select` | 150 | `1` | `if (ctx.cursor_row == ctx.visible_rows - 1)` | At the bottom visible row, downward movement scrolls the window. |
| `menu_equip_select` | 155 | `0` | `ctx.selected_index = 0;` | Wrap selection back to the first entry. |
| `menu_equip_select` | 157 | `0` | `ctx.cursor_row = 0;` | Reset the visible cursor to the top row. |
| `menu_equip_select` | 161 | `0 × 2` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 163 | `1` | `confirm = 1;` | Set the UI confirmation/highlight request on the confirm edge. |
| `menu_equip_select` | 164 | `0 × 2` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_equip_select` | 170 | `0` | `if (ctx.entry_count != 0)` | Only preview an item when the list has entries. |
| `menu_equip_select` | 177 | `1` | `if (selection != -1) {` | Apply only a completed selection; outer cancellation leaves equipment or magic unchanged. |
| `menu_equip_select` | 194 | `0x15` | `if (selection == 0x15) {` | Authored head-armor item ID 21 clears arm/leg equipment and blocks their menu rows while equipped. Its proper resource name is not established here. |
| `menu_spell_select` | 226 | `20, 10` | `s16 labels[20][10];` | Twenty-row local workspace with ten signed glyphs per row, matching the shared label format; current equipment ranges need at most fourteen rows including none, and spell selection at most six. Original over-allocation rationale is unknown. |
| `menu_spell_select` | 227 | `20` | `u8 codes[20];` | Parallel twenty-entry code workspace, indexed with the label rows; original capacity choice is unproven. |
| `menu_spell_select` | 232 | `0` | `s32 confirm = 0;` | Initially no pending confirmation; this is a UI highlight/request flag, not an equipment category. |
| `menu_spell_select` | 233 | `0` | `s32 input = 0;` | Initial previous input state has no pressed buttons for edge detection. |
| `menu_spell_select` | 235 | `99` | `s32 selection = -99;` | Pending-selection sentinel outside item/spell codes and cancellation -1; retained pending a shared list-result protocol audit. The choice of -99 is unproven. |
| `menu_spell_select` | 237, 281 | `1 × 2, 0 × 2` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_spell_select` | 240 | `0` | `k = 0;` | Start appending selected rows at the first workspace entry. |
| `menu_spell_select` | 241 | `4` | `name = magic_name_rows[4].codes;` | Start at the first selectable attack-spell name, magic ID 4. |
| `menu_spell_select` | 242 | `4, 9, 10` | `for (code = 4; code < 9; code++, name += 10) {` | Selectable attack-spell IDs 4..8; advance one ten-halfword name row each iteration. IDs 0..3 belong to the separate instant-use magic panel. |
| `menu_spell_select` | 243 | `1` | `if (magic_records[code].learned == 1) {` | The selector accepts exactly learned byte 1; do not widen the retail predicate to any nonzero value. |
| `menu_spell_select` | 244 | `0, 10` | `for (j = 0; j < 10; j++)` | Copy all ten glyph halfwords, starting at index zero, from the shared fixed-width name row. |
| `menu_spell_select` | 250 | `0, 0x59` | `labels[k][0] = 0x59;` | First glyph position in the authored none label; atlas code 89. Its localized identity is not independently decoded here. |
| `menu_spell_select` | 251 | `1, 0x4c` | `labels[k][1] = MENU_TEXT_DAKUTEN \| 0x4c;` | Second glyph position: authored atlas code 76 plus the named dakuten bit. |
| `menu_spell_select` | 252 | `2, 0x4c` | `labels[k][2] = 0x4c;` | Third glyph position repeats atlas code 76 without dakuten. |
| `menu_spell_select` | 253 | `3` | `labels[k][3] = MENU_TEXT_END;` | Terminator position immediately after the three authored none-label glyphs. |
| `menu_spell_select` | 257 | `1 × 2` | `menu_list_init(&ctx, 1, 1);` | Equipment title bank 1, row 1 for magic; positional layout indices at the shared widget boundary. |
| `menu_spell_select` | 259 | `10` | `ctx.glyphs_per_entry = 10;` | The shared name-row representation has ten glyph halfwords per entry. |
| `menu_spell_select` | 260 | `0 × 2` | `ctx.glyph_rows = &labels[0][0];` | Base address of the first glyph in the first row for the flat list-render API. |
| `menu_spell_select` | 261 | `0` | `ctx.quantities = 0;` | Null quantity list: selection panels display names without stock counts. |
| `menu_spell_select` | 264, 338 | `0 × 2` | `if (ctx.entry_count != 0) {` | Only preview/render an item when the list has entries. |
| `menu_spell_select` | 265, 313, 328 | `1 × 3` | `if (menu_load_item_texture(codes[ctx.selected_index]) == 1)` | Numbered-texture loader returns 1 on load failure; retain the equality check. |
| `menu_spell_select` | 274 | `1` | `if (confirm == 1) {` | A set confirmation flag enters the second-stage confirmation widget. |
| `menu_spell_select` | 275 | `5, 2, 0 × 2, 1` | `if (menu_list_interact(&ctx, 5, 2, codes[ctx.selected_index], 0, 0) == -1)` | Dialog kind 5 chooses the equip label, preview mode 2 draws the already loaded spell marker, and both detail arguments are unused zeros; -1 cancels the widget. Shared protocol typing remains a follow-up. |
| `menu_spell_select` | 276 | `99` | `selection = -99;` | Return to pending selection after the confirmation widget is cancelled; distinct from exiting the outer panel. |
| `menu_spell_select` | 280 | `99` | `if (selection != -99) {` | Any completed selection or cancellation ends the modal loop; pending -99 continues it. |
| `menu_spell_select` | 287 | `0` | `confirm = 0;` | Clear the pending confirmation/highlight request for the next input frame. |
| `menu_spell_select` | 289 | `1` | `input = PadRead(1);` | Ignored retail PadRead call-site argument; the linked routine uses its global pad identifier. |
| `menu_spell_select` | 290 | `0` | `if (ctx.entry_count == 0) {` | Empty-list input path, retained even though the current construction appends a none entry. |
| `menu_spell_select` | 291 | `0` | `if (input != 0) {` | Any pressed input acknowledges the empty-list path. |
| `menu_spell_select` | 293, 335 | `1 × 2` | `selection = -1;` | Outer panel cancellation, outside the nonnegative item IDs or row indices. |
| `menu_spell_select` | 295 | `0 × 2` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 297 | `0` | `if (ctx.selected_index != 0) {` | First list-entry boundary for upward cursor movement. |
| `menu_spell_select` | 299 | `0` | `if (ctx.cursor_row == 0)` | Top visible row: scrolling upward changes the window instead of its cursor row. |
| `menu_spell_select` | 304 | `1` | `ctx.selected_index = ctx.entry_count - 1;` | Wrap to the last zero-based entry. |
| `menu_spell_select` | 306, 325 | `0 × 2` | `ctx.scroll_offset = 0;` | Reset the scroll window to its first entry. |
| `menu_spell_select` | 307 | `1` | `ctx.cursor_row = ctx.entry_count - 1;` | Bottom row of a list shorter than the visible window. |
| `menu_spell_select` | 310 | `1` | `ctx.cursor_row = ctx.visible_rows - 1;` | Last zero-based visible cursor row. |
| `menu_spell_select` | 315 | `0 × 2` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 317 | `1` | `if (ctx.selected_index < ctx.entry_count - 1) {` | Downward movement stops at the last zero-based entry before wrapping. |
| `menu_spell_select` | 319 | `1` | `if (ctx.cursor_row == ctx.visible_rows - 1)` | At the bottom visible row, downward movement scrolls the window. |
| `menu_spell_select` | 324 | `0` | `ctx.selected_index = 0;` | Wrap selection back to the first entry. |
| `menu_spell_select` | 326 | `0` | `ctx.cursor_row = 0;` | Reset the visible cursor to the top row. |
| `menu_spell_select` | 330 | `0 × 2` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 332 | `1` | `confirm = 1;` | Set the UI confirmation/highlight request on the confirm edge. |
| `menu_spell_select` | 333 | `0 × 2` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_spell_select` | 345 | `1` | `if (selection != -1) {` | Apply only a completed selection; outer cancellation leaves equipment or magic unchanged. |
| `menu_draw_name_list` | 19 | `0xae` | `gs.x = 0xae;` | Authored equipment-name column X coordinate 174 pixels; original placement rationale unknown. |
| `menu_draw_name_list` | 20 | `0x28` | `gs.y = 0x28;` | Authored first row Y coordinate 40 pixels. |
| `menu_draw_name_list` | 26, 32, 38, 44, 50, 56, 62 | `0x14 × 7` | `gs.y += 0x14;` | Twenty-pixel fixed row spacing, also consumed for empty slots: eight rows span Y=40..180, preserving category positions rather than compacting names. |
