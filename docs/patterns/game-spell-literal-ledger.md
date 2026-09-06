# Retained spell and equipment-root menu literals

Complete per-occurrence ledger for `src/game/magic.c` and `src/game/menu_panels.c`
after the [spell identity audit](game-spell-identities.md) and [selected-spell types](game-selected-magic-types.md) and [learning-state typing](game-magic-learning-state.md). Claim addresses and
extents are excluded; named constants are counted separately. Negative signs
are operators. The function/expression identifies each use; lines locate this
source version. Positional resource indices, representation arithmetic and
authored tuning retain their numeric form with the specific reasons below.

All **125 retained occurrences** in **90 expression groups** have explicit reasons.
The [magic-panel flow audit](game-magic-panel-flow.md) refreshes its source
locations and exit/cursor predicates; direct name indexing removes one literal.

| Function | Lines | Tokens | Expression | Reason |
| --- | --- | --- | --- | --- |
| `effect_pool_reset` | 14 | `0` | `for (i = 0; i < KF_EFFECT_CAPACITY; i++) {` | Start the complete effect-pool reset at zero-based entry zero. |
| `magic_load_records` | 26 | `0` | `for (count = sizeof effect_state.magic / sizeof *source; count != 0; count--) {` | Zero terminates the full runtime-table word-copy countdown; the count comes from the actual 24-record array. |
| `magic_cast` | 53 | `200` | `offset.vx = -200;` | Authored launch offset X=-200 world units before camera rotation; original placement rationale unknown. |
| `magic_cast` | 54 | `200` | `offset.vy = 200;` | Authored launch offset Y=200 world units before camera rotation; original placement rationale unknown. |
| `magic_cast` | 55 | `400` | `offset.vz = 400;` | Authored launch offset Z=400 world units before camera rotation; original placement rationale unknown. |
| `magic_cast` | 66, 124 | `0x4e20 × 2` | `player_state.camera_rotation.vy, 0x4e20, KF_ACTOR_AIM_TOLERANCE, &distance);` | Target-cone maximum range is 20000 world units, ten map tiles; retain the authored range independently of launch speed. |
| `magic_cast` | 68 | `0` | `if (target == 0) {` | Null target selects the untargeted launch path. |
| `magic_cast` | 69, 78 | `600 × 2` | `scale = 600;` | Base projectile displacement scale is 600 world units per active update for Fire Ball/Light Needle; Q12 direction rounding is preserved. Original speed rationale unknown. |
| `magic_cast` | 71, 89, 96 | `800 × 3` | `scale = 800;` | Lightning Bolt and Wind Cutter use 800 world units per active update; targeted Lightning Bolt divides range by this scale for its countdown. Original speed rationale unknown. |
| `magic_cast` | 72 | `128` | `angles.x = -128;` | Untargeted Lightning Bolt pitch is -128/4096 turn, or -11.25 degrees; authored aim, not a distance. |
| `magic_cast` | 73 | `20` | `distance = 20;` | The scratch distance becomes the untargeted Lightning Bolt countdown: twenty successful movement/decrement updates before its zero-triggered impact, unless collision triggers earlier. Original duration rationale unknown. |
| `magic_cast` | 81 | `1` | `map_cell_attribute_grid[target->cell_z][target->cell_x] - 1]` | Convert the one-based cell attribute into its zero-based height-table index. |
| `magic_cast` | 82 | `4999` | `>= -4999) {` | Signed attribute-height threshold is strictly above -5000; it chooses the 3000 versus 5000 world-unit aim adjustment. Its level-specific rationale remains unresolved. |
| `magic_cast` | 84 | `3000` | `world_pos.vy + 3000 - target->position.vy, -distance);` | Lightning Bolt aim uses a 3000-world-unit Y adjustment on the selected attribute-height path; preserve signed vertical difference and negative horizontal distance. |
| `magic_cast` | 87 | `5000` | `world_pos.vy + 5000 - target->position.vy, -distance);` | Other attribute-height path uses a 5000-world-unit Y adjustment; original authored aim rationale unknown. |
| `magic_cast` | 109, 113 | `0xa × 2` | `0xa, KF_EFFECT_USE_PLAYER_MAGIC \| KF_EFFECT_COLLISION_TARGET_ACTORS,` | Forwarded effect ID10 also supplies unity when consumed as the player-damage tenths multiplier. Keep this multipurpose byte separate from spell kinds and actor indices. |
| `magic_cast` | 110 | `1` | `KF_ENUM_ENCODE(u8, player_state.selected_magic_id), &world_pos, &direction, &rotation, 1);` | True optional sound request after the Light Needle rotation-pointer argument. |
| `magic_cast` | 114 | `1` | `KF_ENUM_ENCODE(u8, player_state.selected_magic_id), &world_pos, &direction, distance, 1);` | True optional sound request; the preceding slot is the Lightning Bolt countdown and is ignored by Fire Ball/Wind Cutter. |
| `magic_cast` | 125 | `0` | `if (target != 0) {` | A nonnull target supplies the Fire Wall spawn position. |
| `magic_cast` | 127, 143 | `0xa × 2` | `0xa, KF_EFFECT_USE_PLAYER_MAGIC \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,` | Effect ID10 supplies unity to the downstream player-damage tenths multiplier on Fire Wall paths; it is separate from the collision-target flags. |
| `magic_cast` | 136 | `6000` | `- (rsin(player_state.camera_rotation.vy) * 6000 >> KF_FIXED12_BITS);` | Untargeted Fire Wall X offset projects 6000 world units, three tiles, through the Q12 direction; integer rounding remains. Original range rationale unknown. |
| `magic_cast` | 138 | `6000` | `+ (rcos(player_state.camera_rotation.vy) * 6000 >> KF_FIXED12_BITS);` | Matching Z projection of the same 6000-world-unit Fire Wall placement distance. |
| `effect_pool_sweep` | 156 | `1` | `u16 i = KF_EFFECT_CAPACITY - 1;` | Inclusive post-decrement traversal begins at count minus one and visits all 48 effect records. |
| `effect_pool_sweep` | 164 | `0` | `} while (i-- != 0);` | Post-decrement test processes the last record when the old countdown is zero. |
| `menu_magic_panel` | 27 | `10 × 2` | `s16 labels[10][10];` | Ten-row stack workspace with ten glyph halfwords per row; the instant panel currently offers at most four spells. Preserve the evidenced capacity; original over-allocation rationale unknown. |
| `menu_magic_panel` | 28 | `16` | `u8 codes[16];` | Sixteen-byte parallel spell-code workspace; only four learned instant spells can be appended here. Original capacity choice unknown. |
| `menu_magic_panel` | 32 | `0` | `s32 confirm = 0;` | Initially no pending confirmation; this is a UI highlight/request flag, not an equipment category. |
| `menu_magic_panel` | 33 | `0` | `s32 input = 0;` | Initial previous input state has no pressed buttons for edge detection. |
| `menu_magic_panel` | 35 | `99` | `s32 selection = -99;` | Pending-selection sentinel outside item/spell codes and cancellation -1; retained pending a shared list-result protocol audit. The choice of -99 is unproven. |
| `menu_magic_panel` | 37, 74 | `1 × 2, 0 × 2` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_magic_panel` | 39 | `0, 1` | `menu_list_init(&ctx, 0, 1);` | Authored window layout0, title row1 selects the magic panel heading; positional resource indices, distinct from spell IDs. |
| `menu_magic_panel` | 41 | `0` | `found = 0;` | Start appending learned spell rows at the first workspace entry. |
| `menu_magic_panel` | 44 | `0, 10` | `for (j = 0; j < 10; j++)` | Copy all ten glyph halfwords, starting at index zero, from the shared fixed-width name row. |
| `menu_magic_panel` | 51 | `10` | `ctx.glyphs_per_entry = 10;` | The shared name-row representation has ten glyph halfwords per entry. |
| `menu_magic_panel` | 52 | `0 × 2` | `ctx.glyph_rows = &labels[0][0];` | Base address of the first glyph in the first row for the flat list-render API. |
| `menu_magic_panel` | 53 | `0` | `ctx.quantities = 0;` | Null quantity list: selection panels display names without stock counts. |
| `menu_magic_panel` | 56 | `0` | `if (ctx.entry_count != 0) {` | Only preview/render an item when the list has entries. |
| `menu_magic_panel` | 57, 106, 121 | `1 × 3` | `if (menu_load_item_texture(codes[ctx.selected_index]) == 1)` | Numbered-texture loader returns exactly1 on failure; preserve the existing equality check. |
| `menu_magic_panel` | 58, 107, 122 | `1 × 3` | `return -1;` | Texture-load failure returns -1 directly, bypassing the shared normal-return move and all spell effects. |
| `menu_magic_panel` | 65 | `1` | `if (confirm == 1) {` | A set confirmation flag enters the second-stage confirmation widget. |
| `menu_magic_panel` | 67 | `0` | `KF_MENU_PREVIEW_MAGIC_ICON, codes[ctx.selected_index], 0, KF_ITEM_PRICE_BUY)` | The shop/detail index is an unused zero for the magic-icon confirmation path. |
| `menu_magic_panel` | 69 | `99` | `selection = -99;` | A cancelled confirmation resets the outer selection after the widget call; its branch delay slot performs this assignment. |
| `menu_magic_panel` | 73 | `99` | `if (selection != -99) {` | Any completed selection or cancellation ends the modal loop; pending -99 continues it. |
| `menu_magic_panel` | 80 | `0` | `confirm = 0;` | Clear the pending confirmation/highlight request for the next input frame. |
| `menu_magic_panel` | 82 | `1` | `input = PadRead(1);` | Ignored retail PadRead call-site argument; the linked routine uses its global pad identifier. |
| `menu_magic_panel` | 83 | `0` | `if (ctx.entry_count == 0) {` | No learned instant spells produces an empty list; it has no appended none entry. |
| `menu_magic_panel` | 84 | `0` | `if (input != 0) {` | Any pressed input acknowledges the empty-list path. |
| `menu_magic_panel` | 86, 128 | `1 × 2` | `selection = -1;` | Outer panel cancellation, outside the nonnegative item IDs or row indices. |
| `menu_magic_panel` | 88 | `0 × 2` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | Named button present now and absent previously gives a rising edge; zero means no corresponding bit. |
| `menu_magic_panel` | 90 | `0` | `if (ctx.selected_index != 0) {` | First list-entry boundary for upward cursor movement. |
| `menu_magic_panel` | 92 | `0` | `if (ctx.cursor_row == 0)` | Scroll upward at the first visible row; otherwise move the cursor. Retail places the scroll arm first. |
| `menu_magic_panel` | 97 | `1` | `ctx.selected_index = ctx.entry_count - 1;` | Set the last entry before either upward-wrap viewport arm, including the retained long-list path. |
| `menu_magic_panel` | 99, 118 | `0 × 2` | `ctx.scroll_offset = 0;` | Reset the scroll window to its first entry. |
| `menu_magic_panel` | 100 | `1` | `ctx.cursor_row = ctx.entry_count - 1;` | Bottom row of a list shorter than the visible window. |
| `menu_magic_panel` | 103 | `1` | `ctx.cursor_row = ctx.visible_rows - 1;` | Last zero-based visible cursor row. |
| `menu_magic_panel` | 108 | `0 × 2` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | Named button present now and absent previously gives a rising edge; zero means no corresponding bit. |
| `menu_magic_panel` | 110 | `1` | `if (ctx.selected_index < ctx.entry_count - 1) {` | Downward movement stops at the last zero-based entry before wrapping. |
| `menu_magic_panel` | 112 | `1` | `if (ctx.cursor_row == ctx.visible_rows - 1)` | Scroll downward at the bottom visible row; otherwise move the cursor. Retail places the scroll arm first. |
| `menu_magic_panel` | 117 | `0` | `ctx.selected_index = 0;` | Wrap selection back to the first entry. |
| `menu_magic_panel` | 119 | `0` | `ctx.cursor_row = 0;` | Reset the visible cursor to the top row. |
| `menu_magic_panel` | 123 | `0 × 2` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Named button present now and absent previously gives a rising edge; zero means no corresponding bit. |
| `menu_magic_panel` | 125 | `1` | `confirm = 1;` | Set the UI confirmation/highlight request on the confirm edge. |
| `menu_magic_panel` | 126 | `0 × 2` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Named button present now and absent previously gives a rising edge; zero means no corresponding bit. |
| `menu_magic_panel` | 131 | `0` | `if (ctx.entry_count != 0)` | Only preview an item when the list has entries. |
| `menu_magic_panel` | 136 | `1` | `if (selection != -1) {` | Only a completed spell selection enters the casting path; cancellation reaches the shared normal return without indexing a record. |
| `menu_magic_panel` | 149 | `3` | `player_state.vitals.current_hp += player_state.magic * 3;` | Bless restores three times the current magic stat as HP, with the existing halfword store before the maximum-HP cap. Original healing multiplier rationale unknown. |
| `menu_option_root` | 166 | `0` | `s32 cursor = 0;` | Start the equipment menu at its first row (weapon); this integer tracks UI position. |
| `menu_option_root` | 167 | `0` | `s32 confirm = 0;` | Initially no pending confirmation; this is a UI highlight/request flag, not an equipment category. |
| `menu_option_root` | 168 | `0` | `s32 input = 0;` | Initial previous input state has no pressed buttons for edge detection. |
| `menu_option_root` | 170 | `99` | `s32 result = -99;` | Pending outer-menu result outside completed cancellation -1; original choice of -99 unknown. |
| `menu_option_root` | 171 | `1` | `s32 selection = -1;` | Initially no equipment subpanel selection is pending. |
| `menu_option_root` | 175 | `1, 9, 0 × 2` | `menu_draw_window(1, 9, 0, 0);` | Retail equipment layout1 contains nine rows: eight equipment/magic categories and return. Start at cursor zero without a confirmation highlight. |
| `menu_option_root` | 179 | `1` | `if (selection != -1 \|\| result == selection) {` | Redraw after a completed subpanel selection or the matching -1 outer result; retain the two-result protocol. |
| `menu_option_root` | 182, 237 | `1 × 2, 9 × 2` | `menu_draw_window(1, 9, cursor, confirm);` | Same authored equipment layout1 and complete nine-row extent; this UI row count is independent of the coincidentally nine spell labels. |
| `menu_option_root` | 184 | `1, 0` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_option_root` | 206 | `1` | `selection = -1;` | Clear the completed subpanel selection before reading the next equipment-menu input. |
| `menu_option_root` | 207 | `99` | `if (result != -99)` | A completed outer-menu result ends the modal loop; -99 remains pending. |
| `menu_option_root` | 210 | `0` | `confirm = 0;` | Clear the pending confirmation/highlight request for the next input frame. |
| `menu_option_root` | 212 | `1` | `input = PadRead(1);` | Ignored retail PadRead call-site argument; the linked routine uses its global pad identifier. |
| `menu_option_root` | 213 | `0 × 2` | `if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | Named button present now and absent previously gives a rising edge; zero means no corresponding bit. |
| `menu_option_root` | 215 | `0` | `if (cursor != 0)` | First-row boundary for upward movement. |
| `menu_option_root` | 218 | `8` | `cursor = 8;` | Wrap upward to zero-based return row8, the final row in the nine-row equipment layout. |
| `menu_option_root` | 219 | `0 × 2` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | Named button present now and absent previously gives a rising edge; zero means no corresponding bit. |
| `menu_option_root` | 221 | `8` | `if (cursor != 8)` | Return row8 is the final-row boundary for downward movement. |
| `menu_option_root` | 224 | `0` | `cursor = 0;` | Wrap downward to the first equipment row. |
| `menu_option_root` | 225 | `0 × 2` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Named button present now and absent previously gives a rising edge; zero means no corresponding bit. |
| `menu_option_root` | 227 | `1` | `confirm = 1;` | Set the UI confirmation/highlight request on the confirm edge. |
| `menu_option_root` | 228 | `8` | `if (cursor < 8)` | Rows0..7 dispatch categories; row8 completes the outer menu instead. |
| `menu_option_root` | 231, 234 | `1 × 2` | `result = -1;` | Complete the outer menu on return-row confirmation or the cancel edge. |
| `menu_option_root` | 232 | `0 × 2` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Named button present now and absent previously gives a rising edge; zero means no corresponding bit. |
