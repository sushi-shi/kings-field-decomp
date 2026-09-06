# Retained shop-menu literals

Complete root/buy/sell ledger after the [price-domain audit](game-shop-price-domains.md) and [consumable/accessory identities](game-item-consumables-accessories.md).
All **153 remaining occurrences**, down from 188, have specific reasons. Claims
and named enum definitions are separate; signs do not count as numeric tokens.

| Function | Lines | Tokens | Expression | Reason |
| --- | --- | --- | --- | --- |
| `item_menu_root` | 131 | `0` | `s32 confirm = 0;` | Clear the pending confirmation/highlight flag before processing input. |
| `item_menu_root` | 132 | `0` | `s32 input = 0;` | No prior pressed button bits at entry; used for input edge detection. |
| `item_menu_root` | 134 | `99` | `s32 done = -99;` | Root is still running. The -99 control sentinel is outside its row choices; the original choice of this negative value is unproven. |
| `item_menu_root` | 135 | `1` | `s32 selection = -1;` | Root has no selected sub-panel yet; -1 is outside buy/sell row indices. |
| `item_menu_root` | 138, 141, 144 | `0 × 3` | `menu_draw_window(KF_MENU_WINDOW_SHOP, KF_SHOP_CHOICE_COUNT, KF_SHOP_ROW_BUY, 0);` | Draw the initial shop window with confirmation highlight disabled; its resource, choice count and initial row are named. |
| `item_menu_root` | 146, 155, 168 | `1 × 3, 0 × 3` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_root` | 151 | `1` | `if (selection != -1 \|\| done == selection) {` | Redraw a chosen sub-panel highlight, or the final closed frame when done and the cleared selection both equal -1. Preserve this retail guard without conflating row and loop-control domains. |
| `item_menu_root` | 166 | `1` | `selection = -1;` | Clear the pending sub-panel choice after dispatch. |
| `item_menu_root` | 167 | `99` | `if (done != -99) {` | Any completed root loop-control state exits; -99 is the running sentinel. |
| `item_menu_root` | 174 | `0` | `confirm = 0;` | Clear the pending confirmation/highlight request for subsequent input. |
| `item_menu_root` | 176 | `1` | `input = PadRead(1);` | Ignored retail call-site argument; this is not a controller-port selector. |
| `item_menu_root` | 177 | `0 × 2` | `if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_root` | 183 | `0 × 2` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_root` | 189 | `0 × 2` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_root` | 191 | `1` | `confirm = 1;` | Set the Boolean confirmation/highlight request on a fresh confirm-button edge. |
| `item_menu_root` | 195, 198 | `1 × 2` | `done = -1;` | Mark the root closed by return-row confirmation or the cancel button. |
| `item_menu_root` | 196 | `0 × 2` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_buy` | 213 | `80, 10` | `s16 entries[80][10];` | Workspace has one ten-halfword glyph row for each of the 80 item IDs in the loaded database. |
| `item_menu_buy` | 214 | `80` | `u8 available[80];` | One byte per possible item row: shop availability bytes on buy, unequipped player quantities on sell. The buy copy remains unused by display; preserve the retail stores. |
| `item_menu_buy` | 215 | `80` | `u8 index[80];` | Parallel mapping from each possible displayed row back to its byte item ID. |
| `item_menu_buy` | 220 | `0` | `s32 input = 0;` | No prior pressed button bits at entry; used for input edge detection. |
| `item_menu_buy` | 222 | `0` | `s32 confirm = 0;` | Clear the pending confirmation/highlight flag before processing input. |
| `item_menu_buy` | 223 | `99` | `s32 selection = -99;` | Pending item selection, outside the 0..79 item IDs and cancellation -1. This is the outer list protocol, not the confirmation-widget result; original -99 choice unproven. |
| `item_menu_buy` | 225, 274 | `1 × 2, 0 × 2` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_buy` | 230 | `0` | `found = 0;` | Start appending displayed entries at the first workspace row. |
| `item_menu_buy` | 231 | `80` | `for (slot = KF_ITEM_VERDITE; slot < 80; slot++) {` | Authored buy ordering puts IDs 42..79 first, then 0..41; 80 is the database endpoint. Row 42 is Verdite. Do not invent a uniform item category for this mixed band. |
| `item_menu_buy` | 232, 241 | `0 × 4` | `if (inv[slot] != 0 && item_stock[0][slot] < KF_ITEM_STACK_CAPACITY) {` | Any nonzero shop availability includes an item if the player-owned bank zero is below its named stack cap. |
| `item_menu_buy` | 233, 242 | `0 × 2, 10 × 2` | `for (j = 0; j < 10; j++)` | Copy all ten glyph halfwords, starting at zero, from the shared item-name row. |
| `item_menu_buy` | 240 | `0` | `for (slot = 0; slot < KF_ITEM_VERDITE; slot++) {` | Append the remaining database IDs 0..41 after the 42..79 band; preserve retail display order. |
| `item_menu_buy` | 250 | `9` | `ctx.visible_rows = 9;` | Authored shop viewport shows nine rows; this overrides the generic initializer’s eleven-row capacity. Original layout choice is unproven. |
| `item_menu_buy` | 251 | `10` | `ctx.glyphs_per_entry = 10;` | Each item name is the full ten-halfword row copied from the database. |
| `item_menu_buy` | 252 | `0 × 2` | `ctx.glyph_rows = &entries[0][0];` | Flat renderer input begins at the first glyph of the first row. |
| `item_menu_buy` | 253 | `0` | `ctx.quantities = 0;` | Null quantity source disables count rendering in this shop list; availability still controls inclusion. |
| `item_menu_buy` | 256 | `0` | `if (ctx.entry_count != 0) {` | Only load/render a preview when the constructed list has an entry. |
| `item_menu_buy` | 257, 302, 317 | `0 × 3` | `if (menu_load_item_model(index[ctx.selected_index]) != 0)` | Any nonzero model-load result aborts the panel; retain the complete failure predicate. |
| `item_menu_buy` | 265 | `1` | `if (confirm == 1) {` | Enter the confirmation widget only for the exact set value one; this also proves the former sell-price argument was one. |
| `item_menu_buy` | 266 | `99` | `selection = -99;` | Reset the outer item selection to pending before confirmation; a declined widget leaves it pending. |
| `item_menu_buy` | 272 | `99` | `if (selection != -99) {` | A chosen item or cancellation completes the outer modal selection; pending -99 continues input. |
| `item_menu_buy` | 273 | `0` | `confirm = 0;` | Clear the pending confirmation/highlight request for subsequent input. |
| `item_menu_buy` | 280 | `1` | `input = PadRead(1);` | Ignored retail call-site argument; this is not a controller-port selector. |
| `item_menu_buy` | 281 | `0` | `if (ctx.entry_count == 0) {` | Handle an empty list without reading a selected item. |
| `item_menu_buy` | 282 | `0` | `if (input != 0) {` | Any button press acknowledges an empty list and exits its panel. |
| `item_menu_buy` | 284, 329 | `1 × 2` | `selection = -1;` | Outer panel cancellation outside the valid item-ID range. |
| `item_menu_buy` | 286 | `0 × 2` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_buy` | 288 | `0` | `if (ctx.selected_index != 0) {` | Upward movement has not reached the first zero-based entry. |
| `item_menu_buy` | 290 | `0` | `if (ctx.cursor_row != 0)` | Above the first visible row, move the cursor upward; at zero, scroll instead. |
| `item_menu_buy` | 295 | `1` | `ctx.selected_index = ctx.entry_count - 1;` | Wrap to the last zero-based list entry. |
| `item_menu_buy` | 296, 314 | `0 × 2` | `ctx.scroll_offset = 0;` | Reset the viewport to start at the first entry. |
| `item_menu_buy` | 297 | `1` | `ctx.cursor_row = ctx.entry_count - 1;` | Bottom row of a list shorter than the viewport. |
| `item_menu_buy` | 300 | `1` | `ctx.cursor_row = ctx.visible_rows - 1;` | Last zero-based row of the full viewport. |
| `item_menu_buy` | 304 | `0 × 2` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_buy` | 306 | `1` | `if (ctx.selected_index < ctx.entry_count - 1) {` | Test the last-entry boundary before moving downward or wrapping. |
| `item_menu_buy` | 308 | `1` | `if (ctx.cursor_row != ctx.visible_rows - 1)` | Move within the viewport until its bottom row, then scroll. |
| `item_menu_buy` | 313 | `0` | `ctx.selected_index = 0;` | Wrap to the first item in the list. |
| `item_menu_buy` | 315 | `0` | `ctx.cursor_row = 0;` | Place the visible cursor on the first row after wrapping. |
| `item_menu_buy` | 319 | `0 × 2` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_buy` | 321 | `1` | `< item_buy_prices[index[ctx.selected_index]][shop_id - 1]) {` | Convert one-based shop ID into its zero-based price column for the affordability check. |
| `item_menu_buy` | 325 | `1` | `confirm = 1;` | Set the Boolean confirmation/highlight request on a fresh confirm-button edge. |
| `item_menu_buy` | 327 | `0 × 2` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_buy` | 333 | `0` | `if (ctx.entry_count != 0)` | Only render a preview when the constructed list has an entry. |
| `item_menu_buy` | 339 | `1` | `if (selection != -1) {` | Apply inventory/gold changes only for a completed item selection, not cancellation. |
| `item_menu_buy` | 342 | `1` | `player_state.gold -= item_buy_prices[selection][shop_id - 1];` | Subtract the selected item’s unsigned halfword price in the shop’s zero-based column. |
| `item_menu_buy` | 343 | `0` | `item_stock[0][selection]++;` | Increment the player-owned stock bank, distinct from shop availability. |
| `item_menu_sell` | 356 | `80, 10` | `s16 entries[80][10];` | Workspace has one ten-halfword glyph row for each of the 80 item IDs in the loaded database. |
| `item_menu_sell` | 357 | `80` | `u8 available[80];` | One byte per possible item row: shop availability bytes on buy, unequipped player quantities on sell. The buy copy remains unused by display; preserve the retail stores. |
| `item_menu_sell` | 358 | `80` | `u8 index[80];` | Parallel mapping from each possible displayed row back to its byte item ID. |
| `item_menu_sell` | 363 | `0` | `s32 input = 0;` | No prior pressed button bits at entry; used for input edge detection. |
| `item_menu_sell` | 365 | `0` | `s32 confirm = 0;` | Clear the pending confirmation/highlight flag before processing input. |
| `item_menu_sell` | 366 | `99` | `s32 selection = -99;` | Pending item selection, outside the 0..79 item IDs and cancellation -1. This is the outer list protocol, not the confirmation-widget result; original -99 choice unproven. |
| `item_menu_sell` | 368, 418 | `1 × 2, 0 × 2` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_sell` | 372 | `0` | `inv = item_stock[0];` | Sell from the player-owned quantity bank zero. |
| `item_menu_sell` | 373 | `0` | `found = 0;` | Start appending displayed entries at the first workspace row. |
| `item_menu_sell` | 374 | `0` | `for (slot = 0; slot < KF_ITEM_GOLD_CROSS; slot++) {` | Start with item zero and stop before Gold Cross ID 52; the Gold Cross and later key/quest-item band is excluded from selling. |
| `item_menu_sell` | 375 | `0` | `if (inv[slot] != 0) {` | Only possessed player items can become sale candidates. |
| `item_menu_sell` | 385 | `0` | `if (available[found] != 0) {` | After subtracting an equipped copy, retain the row only if another copy remains available. |
| `item_menu_sell` | 386 | `0, 10` | `for (j = 0; j < 10; j++)` | Copy all ten glyph halfwords, starting at zero, from the shared item-name row. |
| `item_menu_sell` | 394 | `9` | `ctx.visible_rows = 9;` | Authored shop viewport shows nine rows; this overrides the generic initializer’s eleven-row capacity. Original layout choice is unproven. |
| `item_menu_sell` | 395 | `10` | `ctx.glyphs_per_entry = 10;` | Each item name is the full ten-halfword row copied from the database. |
| `item_menu_sell` | 396 | `0 × 2` | `ctx.glyph_rows = &entries[0][0];` | Flat renderer input begins at the first glyph of the first row. |
| `item_menu_sell` | 397 | `0` | `ctx.quantities = 0;` | Null quantity source disables count rendering in this shop list; availability still controls inclusion. |
| `item_menu_sell` | 400 | `0` | `if (ctx.entry_count != 0) {` | Only load/render a preview when the constructed list has an entry. |
| `item_menu_sell` | 401, 446, 461 | `0 × 3` | `if (menu_load_item_model(index[ctx.selected_index]) != 0)` | Any nonzero model-load result aborts the panel; retain the complete failure predicate. |
| `item_menu_sell` | 409 | `1` | `if (confirm == 1) {` | Enter the confirmation widget only for the exact set value one; this also proves the former sell-price argument was one. |
| `item_menu_sell` | 410 | `99` | `selection = -99;` | Reset the outer item selection to pending before confirmation; a declined widget leaves it pending. |
| `item_menu_sell` | 416 | `99` | `if (selection != -99) {` | A chosen item or cancellation completes the outer modal selection; pending -99 continues input. |
| `item_menu_sell` | 417 | `0` | `confirm = 0;` | Clear the pending confirmation/highlight request for subsequent input. |
| `item_menu_sell` | 424 | `1` | `input = PadRead(1);` | Ignored retail call-site argument; this is not a controller-port selector. |
| `item_menu_sell` | 425 | `0` | `if (ctx.entry_count == 0) {` | Handle an empty list without reading a selected item. |
| `item_menu_sell` | 426 | `0` | `if (input != 0) {` | Any button press acknowledges an empty list and exits its panel. |
| `item_menu_sell` | 428, 468 | `1 × 2` | `selection = -1;` | Outer panel cancellation outside the valid item-ID range. |
| `item_menu_sell` | 430 | `0 × 2` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_sell` | 432 | `0` | `if (ctx.selected_index != 0) {` | Upward movement has not reached the first zero-based entry. |
| `item_menu_sell` | 434 | `0` | `if (ctx.cursor_row != 0)` | Above the first visible row, move the cursor upward; at zero, scroll instead. |
| `item_menu_sell` | 439 | `1` | `ctx.selected_index = ctx.entry_count - 1;` | Wrap to the last zero-based list entry. |
| `item_menu_sell` | 440, 458 | `0 × 2` | `ctx.scroll_offset = 0;` | Reset the viewport to start at the first entry. |
| `item_menu_sell` | 441 | `1` | `ctx.cursor_row = ctx.entry_count - 1;` | Bottom row of a list shorter than the viewport. |
| `item_menu_sell` | 444 | `1` | `ctx.cursor_row = ctx.visible_rows - 1;` | Last zero-based row of the full viewport. |
| `item_menu_sell` | 448 | `0 × 2` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_sell` | 450 | `1` | `if (ctx.selected_index < ctx.entry_count - 1) {` | Test the last-entry boundary before moving downward or wrapping. |
| `item_menu_sell` | 452 | `1` | `if (ctx.cursor_row != ctx.visible_rows - 1)` | Move within the viewport until its bottom row, then scroll. |
| `item_menu_sell` | 457 | `0` | `ctx.selected_index = 0;` | Wrap to the first item in the list. |
| `item_menu_sell` | 459 | `0` | `ctx.cursor_row = 0;` | Place the visible cursor on the first row after wrapping. |
| `item_menu_sell` | 463 | `0 × 2` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_sell` | 465 | `1` | `confirm = 1;` | Set the Boolean confirmation/highlight request on a fresh confirm-button edge. |
| `item_menu_sell` | 466 | `0 × 2` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_sell` | 472 | `0` | `if (ctx.entry_count != 0)` | Only render a preview when the constructed list has an entry. |
| `item_menu_sell` | 478 | `1` | `if (selection != -1) {` | Apply inventory/gold changes only for a completed item selection, not cancellation. |
| `item_menu_sell` | 480 | `1` | `player_state.gold += item_sell_prices[selection][shop_id - 1];` | Credit the selected item’s sell price in the shop’s zero-based price column. |
