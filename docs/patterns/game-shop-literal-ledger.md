# Retained shop-menu literals

Complete root/buy/sell ledger after the [price-domain audit](game-shop-price-domains.md) and [consumable/accessory identities](game-item-consumables-accessories.md), refreshed by the [shop-panel flow audit](game-shop-panel-flow.md).
All **139 remaining occurrences**, down from 188, have specific reasons. Claims
and named enum definitions are separate; signs do not count as numeric tokens.

The [shared-dimension review](game-item-menu-dimensions.md) names the database
extents and glyph-row width while preserving local workspace capacities.

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
| `item_menu_buy` | 220 | `0` | `s32 confirm = 0;` | Clear the pending confirmation/highlight flag before processing input. |
| `item_menu_buy` | 221 | `0` | `s32 input = 0;` | No prior pressed button bits at entry; used for input edge detection. |
| `item_menu_buy` | 223 | `99` | `s32 selection = -99;` | Pending item selection, outside the 0..79 item IDs and cancellation -1. This is the outer list protocol, not the confirmation-widget result; original -99 choice unproven. |
| `item_menu_buy` | 225, 275 | `1 × 2, 0 × 2` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_buy` | 230 | `0` | `found = 0;` | Start appending displayed entries at the first workspace row. |
| `item_menu_buy` | 232, 241 | `0 × 4` | `if (inv[slot] != 0 && item_stock[0][slot] < KF_ITEM_STACK_CAPACITY) {` | Any nonzero shop availability includes an item if the player-owned bank zero is below its named stack cap. |
| `item_menu_buy` | 233, 242 | `0 × 2` | `for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)` | Copy all ten glyph halfwords, starting at zero, from the shared item-name row. |
| `item_menu_buy` | 240 | `0` | `for (slot = 0; slot < KF_ITEM_VERDITE; slot++) {` | Append the remaining database IDs 0..41 after the 42..79 band; preserve retail display order. |
| `item_menu_buy` | 250 | `9` | `ctx.visible_rows = 9;` | Authored shop viewport shows nine rows; this overrides the generic initializer’s eleven-row capacity. Original layout choice is unproven. |
| `item_menu_buy` | 252 | `0 × 2` | `ctx.glyph_rows = &entries[0][0];` | Flat renderer input begins at the first glyph of the first row. |
| `item_menu_buy` | 253 | `0` | `ctx.quantities = 0;` | Null quantity source disables count rendering in this shop list; availability still controls inclusion. |
| `item_menu_buy` | 256 | `0` | `if (ctx.entry_count != 0) {` | Only load/render a preview when the constructed list has an entry. |
| `item_menu_buy` | 257, 305, 320 | `0 × 3` | `if (menu_load_item_model(index[ctx.selected_index]) != 0)` | Any nonzero model-load result aborts the panel; retain the complete failure predicate. |
| `item_menu_buy` | 265 | `1` | `if (confirm == 1) {` | Enter the confirmation widget only for the exact set value one; this also proves the former sell-price argument was one. |
| `item_menu_buy` | 269 | `99` | `selection = -99;` | A cancelled confirmation leaves the outer selection pending; assign after the widget call, as its return-branch delay slot requires. |
| `item_menu_buy` | 273 | `0` | `confirm = 0;` | Unconditionally clear the request before the exit guard, including when the panel remains open after a declined confirmation. |
| `item_menu_buy` | 274 | `99` | `if (selection != -99) {` | A chosen item or cancellation completes the outer modal selection; pending -99 continues input. |
| `item_menu_buy` | 281 | `1` | `input = PadRead(1);` | Ignored retail call-site argument; this is not a controller-port selector. |
| `item_menu_buy` | 282 | `0` | `if (ctx.entry_count == 0) {` | Handle an empty list without reading a selected item. |
| `item_menu_buy` | 283 | `0` | `if (input != 0) {` | Any button press acknowledges an empty list and exits its panel. |
| `item_menu_buy` | 285, 332 | `1 × 2` | `selection = -1;` | Outer panel cancellation outside the valid item-ID range. |
| `item_menu_buy` | 287 | `0 × 2` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_buy` | 289 | `0` | `if (ctx.selected_index != 0) {` | Upward movement has not reached the first zero-based entry. |
| `item_menu_buy` | 291 | `0` | `if (ctx.cursor_row == 0)` | At the first visible row, scroll upward; otherwise move the cursor. Retail lays out the scroll arm first. |
| `item_menu_buy` | 296 | `1` | `ctx.selected_index = ctx.entry_count - 1;` | Wrap to the last zero-based entry before choosing the short-list or full-viewport arm; required for long lists too. |
| `item_menu_buy` | 298, 317 | `0 × 2` | `ctx.scroll_offset = 0;` | Reset the viewport to start at the first entry. |
| `item_menu_buy` | 299 | `1` | `ctx.cursor_row = ctx.entry_count - 1;` | Bottom row of a list shorter than the viewport. |
| `item_menu_buy` | 302 | `1` | `ctx.cursor_row = ctx.visible_rows - 1;` | Last zero-based row of the full viewport. |
| `item_menu_buy` | 307 | `0 × 2` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_buy` | 309 | `1` | `if (ctx.selected_index < ctx.entry_count - 1) {` | Test the last-entry boundary before moving downward or wrapping. |
| `item_menu_buy` | 311 | `1` | `if (ctx.cursor_row == ctx.visible_rows - 1)` | At the bottom visible row, scroll downward; otherwise move the cursor. Retail lays out the scroll arm first. |
| `item_menu_buy` | 316 | `0` | `ctx.selected_index = 0;` | Wrap to the first item in the list. |
| `item_menu_buy` | 318 | `0` | `ctx.cursor_row = 0;` | Place the visible cursor on the first row after wrapping. |
| `item_menu_buy` | 322 | `0 × 2` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_buy` | 324 | `1` | `< item_buy_prices[index[ctx.selected_index]][shop_id - 1]) {` | Convert one-based shop ID into its zero-based price column for the affordability check. |
| `item_menu_buy` | 328 | `1` | `confirm = 1;` | Set the Boolean confirmation/highlight request on a fresh confirm-button edge. |
| `item_menu_buy` | 330 | `0 × 2` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_buy` | 336 | `0` | `if (ctx.entry_count != 0)` | Only render a preview when the constructed list has an entry. |
| `item_menu_buy` | 342 | `1` | `if (selection != -1) {` | Apply inventory/gold changes only for a completed item selection, not cancellation. |
| `item_menu_buy` | 345 | `1` | `player_state.gold -= item_buy_prices[selection][shop_id - 1];` | Subtract the selected item’s unsigned halfword price in the shop’s zero-based column. |
| `item_menu_buy` | 346 | `0` | `item_stock[0][selection]++;` | Increment the player-owned stock bank, distinct from shop availability. |
| `item_menu_sell` | 366 | `0` | `s32 confirm = 0;` | Clear the pending confirmation/highlight flag before processing input. |
| `item_menu_sell` | 367 | `0` | `s32 input = 0;` | No prior pressed button bits at entry; used for input edge detection. |
| `item_menu_sell` | 369 | `99` | `s32 selection = -99;` | Pending item selection, outside the 0..79 item IDs and cancellation -1. This is the outer list protocol, not the confirmation-widget result; original -99 choice unproven. |
| `item_menu_sell` | 371, 422 | `1 × 2, 0 × 2` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_sell` | 375 | `0` | `inv = item_stock[0];` | Sell from the player-owned quantity bank zero. |
| `item_menu_sell` | 376 | `0` | `found = 0;` | Start appending displayed entries at the first workspace row. |
| `item_menu_sell` | 377 | `0` | `for (slot = 0; slot < KF_ITEM_GOLD_CROSS; slot++) {` | Start with item zero and stop before Gold Cross ID 52; the Gold Cross and later key/quest-item band is excluded from selling. |
| `item_menu_sell` | 378 | `0` | `if (inv[slot] != 0) {` | Only possessed player items can become sale candidates. |
| `item_menu_sell` | 388 | `0` | `if (available[found] != 0) {` | After subtracting an equipped copy, retain the row only if another copy remains available. |
| `item_menu_sell` | 389 | `0` | `for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)` | Copy all ten glyph halfwords, starting at zero, from the shared item-name row. |
| `item_menu_sell` | 397 | `9` | `ctx.visible_rows = 9;` | Authored shop viewport shows nine rows; this overrides the generic initializer’s eleven-row capacity. Original layout choice is unproven. |
| `item_menu_sell` | 399 | `0 × 2` | `ctx.glyph_rows = &entries[0][0];` | Flat renderer input begins at the first glyph of the first row. |
| `item_menu_sell` | 400 | `0` | `ctx.quantities = 0;` | Null quantity source disables count rendering in this shop list; availability still controls inclusion. |
| `item_menu_sell` | 403 | `0` | `if (ctx.entry_count != 0) {` | Only load/render a preview when the constructed list has an entry. |
| `item_menu_sell` | 404, 452, 467 | `0 × 3` | `if (menu_load_item_model(index[ctx.selected_index]) != 0)` | Any nonzero model-load result aborts the panel; retain the complete failure predicate. |
| `item_menu_sell` | 412 | `1` | `if (confirm == 1) {` | Enter the confirmation widget only for the exact set value one; this also proves the former sell-price argument was one. |
| `item_menu_sell` | 416 | `99` | `selection = -99;` | A cancelled confirmation leaves the outer selection pending; assign after the widget call, as its return-branch delay slot requires. |
| `item_menu_sell` | 420 | `0` | `confirm = 0;` | Unconditionally clear the request before the exit guard, including when the panel remains open after a declined confirmation. |
| `item_menu_sell` | 421 | `99` | `if (selection != -99) {` | A chosen item or cancellation completes the outer modal selection; pending -99 continues input. |
| `item_menu_sell` | 428 | `1` | `input = PadRead(1);` | Ignored retail call-site argument; this is not a controller-port selector. |
| `item_menu_sell` | 429 | `0` | `if (ctx.entry_count == 0) {` | Handle an empty list without reading a selected item. |
| `item_menu_sell` | 430 | `0` | `if (input != 0) {` | Any button press acknowledges an empty list and exits its panel. |
| `item_menu_sell` | 432, 474 | `1 × 2` | `selection = -1;` | Outer panel cancellation outside the valid item-ID range. |
| `item_menu_sell` | 434 | `0 × 2` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_sell` | 436 | `0` | `if (ctx.selected_index != 0) {` | Upward movement has not reached the first zero-based entry. |
| `item_menu_sell` | 438 | `0` | `if (ctx.cursor_row == 0)` | At the first visible row, scroll upward; otherwise move the cursor. Retail lays out the scroll arm first. |
| `item_menu_sell` | 443 | `1` | `ctx.selected_index = ctx.entry_count - 1;` | Wrap to the last zero-based entry before choosing the short-list or full-viewport arm; required for long lists too. |
| `item_menu_sell` | 445, 464 | `0 × 2` | `ctx.scroll_offset = 0;` | Reset the viewport to start at the first entry. |
| `item_menu_sell` | 446 | `1` | `ctx.cursor_row = ctx.entry_count - 1;` | Bottom row of a list shorter than the viewport. |
| `item_menu_sell` | 449 | `1` | `ctx.cursor_row = ctx.visible_rows - 1;` | Last zero-based row of the full viewport. |
| `item_menu_sell` | 454 | `0 × 2` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_sell` | 456 | `1` | `if (ctx.selected_index < ctx.entry_count - 1) {` | Test the last-entry boundary before moving downward or wrapping. |
| `item_menu_sell` | 458 | `1` | `if (ctx.cursor_row == ctx.visible_rows - 1)` | At the bottom visible row, scroll downward; otherwise move the cursor. Retail lays out the scroll arm first. |
| `item_menu_sell` | 463 | `0` | `ctx.selected_index = 0;` | Wrap to the first item in the list. |
| `item_menu_sell` | 465 | `0` | `ctx.cursor_row = 0;` | Place the visible cursor on the first row after wrapping. |
| `item_menu_sell` | 469 | `0 × 2` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_sell` | 471 | `1` | `confirm = 1;` | Set the Boolean confirmation/highlight request on a fresh confirm-button edge. |
| `item_menu_sell` | 472 | `0 × 2` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_sell` | 478 | `0` | `if (ctx.entry_count != 0)` | Only render a preview when the constructed list has an entry. |
| `item_menu_sell` | 484 | `1` | `if (selection != -1) {` | Apply inventory/gold changes only for a completed item selection, not cancellation. |
| `item_menu_sell` | 486 | `1` | `player_state.gold += item_sell_prices[selection][shop_id - 1];` | Credit the selected item’s sell price in the shop’s zero-based price column. |
