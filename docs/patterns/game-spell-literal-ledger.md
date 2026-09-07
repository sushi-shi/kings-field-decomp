# Retained spell and equipment-root menu literals

Complete per-occurrence ledger for `src/game/magic.c` and `src/game/menu_panels.c`
after the [spell identity audit](game-spell-identities.md) and [selected-spell types](game-selected-magic-types.md) and [learning-state typing](game-magic-learning-state.md). Claim addresses and
extents are excluded; named constants are counted separately. Negative signs
are operators. The function/expression identifies each use; lines locate this
source version. Positional resource indices, representation arithmetic and
authored tuning retain their numeric form with the specific reasons below.

All **94 retained occurrences** in **71 expression groups** have explicit reasons.
The [magic-panel flow audit](game-magic-panel-flow.md) refreshes its source
locations and exit/cursor predicates; direct name indexing removes one literal.

The [list-result review](game-menu-list-results.md) names pending/no-selection
controls and recovery amounts, and refreshes earlier window/choice substitutions.

The [shared-dimension review](game-item-menu-dimensions.md) names the database
extents and glyph-row width while preserving local workspace capacities.

| Function | Lines | Tokens | Expression | Reason |
| --- | --- | --- | --- | --- |
| `menu_magic_panel` | 31 | `10` | `s16 labels[10][MENU_GLYPHS_PER_ROW];` | Ten-row local workspace, distinct from the named number of glyphs in each row. At most four learned instant spells are appended; original over-allocation rationale is unknown. |
| `menu_magic_panel` | 32 | `16` | `u8 codes[16];` | Sixteen-byte parallel spell-code workspace; only four learned instant spells can be appended here. Original capacity choice unknown. |
| `menu_magic_panel` | 36 | `0` | `s32 confirm = 0;` | Initially no pending confirmation; this is a UI highlight/request flag, not an equipment category. |
| `menu_magic_panel` | 37 | `0` | `s32 input = 0;` | Initial previous input state has no pressed buttons for edge detection. |
| `menu_magic_panel` | 41, 78 | `1 × 2, 0 × 2` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_magic_panel` | 45 | `0` | `found = 0;` | Start appending learned spell rows at the first workspace entry. |
| `menu_magic_panel` | 48 | `0` | `for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)` | Copy all ten glyph halfwords, starting at index zero, from the shared fixed-width name row. |
| `menu_magic_panel` | 56 | `0 × 2` | `ctx.glyph_rows = &labels[0][0];` | Base address of the first glyph in the first row for the flat list-render API. |
| `menu_magic_panel` | 57 | `0` | `ctx.quantities = 0;` | Null quantity list: selection panels display names without stock counts. |
| `menu_magic_panel` | 60 | `0` | `if (ctx.entry_count != 0) {` | Only preview/render an item when the list has entries. |
| `menu_magic_panel` | 61, 110, 125 | `1 × 3` | `if (menu_load_item_texture(codes[ctx.selected_index]) == 1)` | Numbered-texture loader returns exactly1 on failure; preserve the existing equality check. |
| `menu_magic_panel` | 69 | `1` | `if (confirm == 1) {` | A set confirmation flag enters the second-stage confirmation widget. |
| `menu_magic_panel` | 71 | `0` | `KF_MENU_PREVIEW_MAGIC_ICON, codes[ctx.selected_index], 0, KF_ITEM_PRICE_BUY)` | The shop/detail index is an unused zero for the magic-icon confirmation path. |
| `menu_magic_panel` | 84 | `0` | `confirm = 0;` | Clear the pending confirmation/highlight request for the next input frame. |
| `menu_magic_panel` | 86 | `1` | `input = PadRead(1);` | Ignored retail PadRead call-site argument; the linked routine uses its global pad identifier. |
| `menu_magic_panel` | 87 | `0` | `if (ctx.entry_count == 0) {` | No learned instant spells produces an empty list; it has no appended none entry. |
| `menu_magic_panel` | 88 | `0` | `if (input != 0) {` | Any pressed input acknowledges the empty-list path. |
| `menu_magic_panel` | 92 | `0 × 2` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | Named button present now and absent previously gives a rising edge; zero means no corresponding bit. |
| `menu_magic_panel` | 94 | `0` | `if (ctx.selected_index != 0) {` | First list-entry boundary for upward cursor movement. |
| `menu_magic_panel` | 96 | `0` | `if (ctx.cursor_row == 0)` | Scroll upward at the first visible row; otherwise move the cursor. Retail places the scroll arm first. |
| `menu_magic_panel` | 101 | `1` | `ctx.selected_index = ctx.entry_count - 1;` | Set the last entry before either upward-wrap viewport arm, including the retained long-list path. |
| `menu_magic_panel` | 103, 122 | `0 × 2` | `ctx.scroll_offset = 0;` | Reset the scroll window to its first entry. |
| `menu_magic_panel` | 104 | `1` | `ctx.cursor_row = ctx.entry_count - 1;` | Bottom row of a list shorter than the visible window. |
| `menu_magic_panel` | 107 | `1` | `ctx.cursor_row = ctx.visible_rows - 1;` | Last zero-based visible cursor row. |
| `menu_magic_panel` | 112 | `0 × 2` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | Named button present now and absent previously gives a rising edge; zero means no corresponding bit. |
| `menu_magic_panel` | 114 | `1` | `if (ctx.selected_index < ctx.entry_count - 1) {` | Downward movement stops at the last zero-based entry before wrapping. |
| `menu_magic_panel` | 116 | `1` | `if (ctx.cursor_row == ctx.visible_rows - 1)` | Scroll downward at the bottom visible row; otherwise move the cursor. Retail places the scroll arm first. |
| `menu_magic_panel` | 121 | `0` | `ctx.selected_index = 0;` | Wrap selection back to the first entry. |
| `menu_magic_panel` | 123 | `0` | `ctx.cursor_row = 0;` | Reset the visible cursor to the top row. |
| `menu_magic_panel` | 127 | `0 × 2` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Named button present now and absent previously gives a rising edge; zero means no corresponding bit. |
| `menu_magic_panel` | 129 | `1` | `confirm = 1;` | Set the UI confirmation/highlight request on the confirm edge. |
| `menu_magic_panel` | 130 | `0 × 2` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Named button present now and absent previously gives a rising edge; zero means no corresponding bit. |
| `menu_magic_panel` | 135 | `0` | `if (ctx.entry_count != 0)` | Only preview an item when the list has entries. |
| `menu_option_root` | 170 | `0` | `s32 cursor = 0;` | Start the equipment menu at its first row (weapon); this integer tracks UI position. |
| `menu_option_root` | 171 | `0` | `s32 confirm = 0;` | Initially no pending confirmation; this is a UI highlight/request flag, not an equipment category. |
| `menu_option_root` | 172 | `0` | `s32 input = 0;` | Initial previous input state has no pressed buttons for edge detection. |
| `menu_option_root` | 179 | `0 × 2` | `menu_draw_window(KF_MENU_WINDOW_EQUIPMENT, KF_MENU_EQUIPMENT_ROW_COUNT, 0, 0);` | Retail equipment layout1 contains nine rows: eight equipment/magic categories and return. Start at cursor zero without a confirmation highlight. |
| `menu_option_root` | 188 | `1, 0` | `while (PadRead(1) != 0)` | Preserve the ignored PadRead call-site argument 1 and wait until the returned button bits are zero; the linked SDK uses global PadIdentifier, not this argument as a port. |
| `menu_option_root` | 214 | `0` | `confirm = 0;` | Clear the pending confirmation/highlight request for the next input frame. |
| `menu_option_root` | 216 | `1` | `input = PadRead(1);` | Ignored retail PadRead call-site argument; the linked routine uses its global pad identifier. |
| `menu_option_root` | 217 | `0 × 2` | `if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | Named button present now and absent previously gives a rising edge; zero means no corresponding bit. |
| `menu_option_root` | 219 | `0` | `if (cursor != 0)` | First-row boundary for upward movement. |
| `menu_option_root` | 223 | `0 × 2` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | Named button present now and absent previously gives a rising edge; zero means no corresponding bit. |
| `menu_option_root` | 228 | `0` | `cursor = 0;` | Wrap downward to the first equipment row. |
| `menu_option_root` | 229 | `0 × 2` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Named button present now and absent previously gives a rising edge; zero means no corresponding bit. |
| `menu_option_root` | 231 | `1` | `confirm = 1;` | Set the UI confirmation/highlight request on the confirm edge. |
| `menu_option_root` | 236 | `0 × 2` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Named button present now and absent previously gives a rising edge; zero means no corresponding bit. |
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
