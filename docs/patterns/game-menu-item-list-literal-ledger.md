# Item-use and discard-panel literal ledger

Complete ledger for `menu_use_item_panel` and `menu_drop_item`: **101 retained
occurrences**, down from 135 (55 item-use and 46 discard). Definitions of named
constants and retail claims are excluded. Signs are operators. See the
[list-result and recovery review](game-menu-list-results.md). The separate
`player_use_item` special-item handler has its own existing ledger.

The [shared-dimension review](game-item-menu-dimensions.md) names the database
extents and glyph-row width while preserving local workspace capacities.

| Function | Lines | Tokens | Expression | Reason |
| --- | --- | --- | --- | --- |
| `menu_use_item_panel` | 162 | `50` | `s16 labels[50][MENU_GLYPHS_PER_ROW];` | Fifty-row local workspace, now sharing the named glyph width. The admitted item sets need at most 34 distinct rows; why the original reserves fifty is unproven. |
| `menu_use_item_panel` | 163 | `56` | `u8 counts[56];` | Parallel 56-byte quantity/item-code workspace in the retail frame. Its capacity differs from the fifty label rows and exceeds the current admitted set; preserve the observed extents without inventing a common capacity. |
| `menu_use_item_panel` | 164 | `56` | `u8 codes[56];` | Parallel 56-byte quantity/item-code workspace in the retail frame. Its capacity differs from the fifty label rows and exceeds the current admitted set; preserve the observed extents without inventing a common capacity. |
| `menu_use_item_panel` | 169 | `0` | `s32 confirm = 0;` | Initially no pending confirmation; this is a UI highlight/request flag, not an equipment category. |
| `menu_use_item_panel` | 170 | `0` | `s32 input = 0;` | Initial previous input state has no pressed buttons for edge detection. |
| `menu_use_item_panel` | 174, 237 | `1 × 2, 0 × 2` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_use_item_panel` | 178 | `0` | `inv = item_stock[0];` | Player-owned stock bank zero; other banks belong to the wider stock/save/shop domain, so no merchant identity is inferred from this index. |
| `menu_use_item_panel` | 179 | `0` | `found = 0;` | Begin appending compacted rows at the first workspace entry. |
| `menu_use_item_panel` | 180 | `0` | `if (inv[KF_ITEM_WATCHMAN_MAP] != 0) {` | Any nonzero owned map quantity includes its row before the other usable-item ranges. |
| `menu_use_item_panel` | 181, 188, 196, 205 | `0 × 4` | `for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)` | Copy all ten glyph halfwords, starting at index zero, from the shared fixed-width name row. |
| `menu_use_item_panel` | 187 | `0` | `if (inv[KF_ITEM_SORCERER_MAP] != 0) {` | Any nonzero owned map quantity includes its row before the other usable-item ranges. |
| `menu_use_item_panel` | 195, 204 | `0 × 2` | `if (code != KF_ITEM_WATCHMAN_MAP && code != KF_ITEM_SORCERER_MAP && inv[code] != 0) {` | Append only owned quantities greater than zero after excluding the maps already listed. The quantity is an unsigned byte; zero means no stock. |
| `menu_use_item_panel` | 214 | `0 × 2` | `ctx.glyph_rows = &labels[0][0];` | Base address of the first glyph in the first row for the flat list-render API. |
| `menu_use_item_panel` | 218 | `0` | `if (ctx.entry_count != 0) {` | Only preview/render an item when the list has entries. |
| `menu_use_item_panel` | 219, 267, 282, 290 | `0 × 4` | `if (menu_load_item_model(codes[ctx.selected_index]) != 0)` | Any nonzero model-loader result aborts the panel. Item use returns named no-selection; discard returns void. These early exits preserve their existing cleanup order and differ from the normal release path. |
| `menu_use_item_panel` | 227 | `1` | `if (confirm == 1) {` | A set confirmation flag enters the second-stage confirmation widget. |
| `menu_use_item_panel` | 229 | `0` | `KF_MENU_PREVIEW_ITEM_MODEL, codes[ctx.selected_index], 0, KF_ITEM_PRICE_BUY)` | Unused shop index zero in the shared confirmation API; this model-preview path does not request a shop-price detail view. |
| `menu_use_item_panel` | 235 | `0` | `confirm = 0;` | Clear the pending confirmation/highlight request for the next input frame. |
| `menu_use_item_panel` | 243 | `1` | `input = PadRead(1);` | Ignored retail PadRead call-site argument; the linked routine uses its global pad identifier. |
| `menu_use_item_panel` | 244 | `0` | `if (ctx.entry_count == 0) {` | No eligible owned rows produces the empty-list input path; any subsequent pressed input exits with no selection. |
| `menu_use_item_panel` | 245 | `0` | `if (input != 0) {` | Any pressed input acknowledges the empty-list path. |
| `menu_use_item_panel` | 249 | `0 × 2` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_use_item_panel` | 251 | `0` | `if (ctx.selected_index != 0) {` | First list-entry boundary for upward cursor movement. |
| `menu_use_item_panel` | 253 | `0` | `if (ctx.cursor_row == 0)` | At the first visible row, upward movement scrolls the window; otherwise it decrements the visible cursor row. |
| `menu_use_item_panel` | 258 | `1` | `ctx.selected_index = ctx.entry_count - 1;` | Wrap to the last zero-based entry. |
| `menu_use_item_panel` | 260, 279 | `0 × 2` | `ctx.scroll_offset = 0;` | Reset the scroll window to its first entry. |
| `menu_use_item_panel` | 261 | `1` | `ctx.cursor_row = ctx.entry_count - 1;` | Bottom row of a list shorter than the visible window. |
| `menu_use_item_panel` | 264 | `1` | `ctx.cursor_row = ctx.visible_rows - 1;` | Last zero-based visible cursor row. |
| `menu_use_item_panel` | 269 | `0 × 2` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_use_item_panel` | 271 | `1` | `if (ctx.selected_index < ctx.entry_count - 1) {` | Downward movement stops at the last zero-based entry before wrapping. |
| `menu_use_item_panel` | 273 | `1` | `if (ctx.cursor_row == ctx.visible_rows - 1)` | At the last zero-based visible row, downward movement scrolls the window; otherwise it increments the visible cursor row. |
| `menu_use_item_panel` | 278 | `0` | `ctx.selected_index = 0;` | Wrap selection back to the first entry. |
| `menu_use_item_panel` | 280 | `0` | `ctx.cursor_row = 0;` | Reset the visible cursor to the top row. |
| `menu_use_item_panel` | 284 | `0 × 2` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_use_item_panel` | 293 | `1` | `confirm = 1;` | Set the UI confirmation/highlight request on the confirm edge. |
| `menu_use_item_panel` | 295 | `0 × 2` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_use_item_panel` | 301 | `0` | `if (ctx.entry_count != 0)` | Only preview an item when the list has entries. |
| `menu_use_item_panel` | 322, 324 | `0 × 2` | `player_state.status_effect_flags = 0;` | Grass leaf and fruit clear the entire status bitset. Keep an empty mask distinct from any one named status bit or a claim about additional effects. |
| `menu_drop_item` | 41 | `0` | `s32 confirm = 0;` | Initially no pending confirmation; this is a UI highlight/request flag, not an equipment category. |
| `menu_drop_item` | 42 | `0` | `s32 input = 0;` | Initial previous input state has no pressed buttons for edge detection. |
| `menu_drop_item` | 46, 99 | `1 × 2, 0 × 2` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_drop_item` | 50 | `0` | `found = 0;` | Begin appending compacted rows at the first workspace entry. |
| `menu_drop_item` | 51 | `0` | `code = 0;` | Start scanning the complete stock array at its first zero-based item ID. |
| `menu_drop_item` | 53 | `0` | `inv = item_stock[0];` | Player-owned stock bank zero; other banks belong to the wider stock/save/shop domain, so no merchant identity is inferred from this index. |
| `menu_drop_item` | 55 | `0` | `if (inv[code] != 0) {` | Only owned nonzero quantities enter the equipped-copy adjustment and row-construction path. |
| `menu_drop_item` | 65 | `0` | `if (counts[found] != 0) {` | Append a discard row only if a quantity remains after subtracting one equipped copy. Preserve the single decrement when any of the seven equipment IDs matches. |
| `menu_drop_item` | 66 | `0` | `for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)` | Copy all ten glyph halfwords, starting at index zero, from the shared fixed-width name row. |
| `menu_drop_item` | 76 | `0 × 2` | `ctx.glyph_rows = &labels[0][0];` | Base address of the first glyph in the first row for the flat list-render API. |
| `menu_drop_item` | 77 | `0` | `ctx.quantities = 0;` | Null quantity list: selection panels display names without stock counts. |
| `menu_drop_item` | 80 | `0` | `if (ctx.entry_count != 0) {` | Only preview/render an item when the list has entries. |
| `menu_drop_item` | 81, 129, 144 | `0 × 3` | `if (menu_load_item_model(codes[ctx.selected_index]) != 0)` | Any nonzero model-loader result aborts the panel. Item use returns named no-selection; discard returns void. These early exits preserve their existing cleanup order and differ from the normal release path. |
| `menu_drop_item` | 89 | `1` | `if (confirm == 1) {` | A set confirmation flag enters the second-stage confirmation widget. |
| `menu_drop_item` | 91 | `0` | `KF_MENU_PREVIEW_ITEM_MODEL, codes[ctx.selected_index], 0, KF_ITEM_PRICE_BUY)` | Unused shop index zero in the shared confirmation API; this model-preview path does not request a shop-price detail view. |
| `menu_drop_item` | 97 | `0` | `confirm = 0;` | Clear the pending confirmation/highlight request for the next input frame. |
| `menu_drop_item` | 105 | `1` | `input = PadRead(1);` | Ignored retail PadRead call-site argument; the linked routine uses its global pad identifier. |
| `menu_drop_item` | 106 | `0` | `if (ctx.entry_count == 0) {` | No eligible owned rows produces the empty-list input path; any subsequent pressed input exits with no selection. |
| `menu_drop_item` | 107 | `0` | `if (input != 0) {` | Any pressed input acknowledges the empty-list path. |
| `menu_drop_item` | 111 | `0 × 2` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_drop_item` | 113 | `0` | `if (ctx.selected_index != 0) {` | First list-entry boundary for upward cursor movement. |
| `menu_drop_item` | 115 | `0` | `if (ctx.cursor_row == 0)` | At the first visible row, upward movement scrolls the window; otherwise it decrements the visible cursor row. |
| `menu_drop_item` | 120 | `1` | `ctx.selected_index = ctx.entry_count - 1;` | Wrap to the last zero-based entry. |
| `menu_drop_item` | 122, 141 | `0 × 2` | `ctx.scroll_offset = 0;` | Reset the scroll window to its first entry. |
| `menu_drop_item` | 123 | `1` | `ctx.cursor_row = ctx.entry_count - 1;` | Bottom row of a list shorter than the visible window. |
| `menu_drop_item` | 126 | `1` | `ctx.cursor_row = ctx.visible_rows - 1;` | Last zero-based visible cursor row. |
| `menu_drop_item` | 131 | `0 × 2` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_drop_item` | 133 | `1` | `if (ctx.selected_index < ctx.entry_count - 1) {` | Downward movement stops at the last zero-based entry before wrapping. |
| `menu_drop_item` | 135 | `1` | `if (ctx.cursor_row == ctx.visible_rows - 1)` | At the last zero-based visible row, downward movement scrolls the window; otherwise it increments the visible cursor row. |
| `menu_drop_item` | 140 | `0` | `ctx.selected_index = 0;` | Wrap selection back to the first entry. |
| `menu_drop_item` | 142 | `0` | `ctx.cursor_row = 0;` | Reset the visible cursor to the top row. |
| `menu_drop_item` | 146 | `0 × 2` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_drop_item` | 148 | `1` | `confirm = 1;` | Set the UI confirmation/highlight request on the confirm edge. |
| `menu_drop_item` | 149 | `0 × 2` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Current named button bit set and previous bit clear form a rising edge; zero tests Boolean absence. |
| `menu_drop_item` | 155 | `0` | `if (ctx.entry_count != 0)` | Only preview an item when the list has entries. |
