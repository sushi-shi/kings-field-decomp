# Retained menu-preview literals

Complete ledger for the three preview functions, menu model renderer, model
load/release pair, and initializers in their three edited modules. Named enum
definitions and ADDRESS/DATA/RODATA claims are excluded. Other functions in
the same source files are outside this focused ledger.

This historical snapshot accounts for **158 occurrences**. The [drawing-layer review](game-menu-drawing-layers.md)
names the eight former pickup-window/background depth literals.

The [shared-dimension review](game-item-menu-dimensions.md) names the database
extents and glyph-row width while preserving local workspace capacities.

The [complete runtime ledger](game-menu-runtime-literal-ledger.md) now covers
the full runtime module, including these focused slices.

The [complete detail/dialog ledger](game-menu-detail-literal-ledger.md) covers
the full item-detail module after naming its repeated line spacing.

| Function | Lines | Tokens | Expression | Reason |
| --- | --- | --- | --- | --- |
| `initializers` | 6 | `0 × 4` | `SVECTOR menu_item_preview_rotation = {0, 0, 0, 0};` | Retail initializer: zero pitch, yaw, roll and SDK alignment halfword. The loader subsequently resets only yaw. |
| `menu_item_model_preview` | 26 | `0, 0x230` | `rot.t[0] = 0x230;` | GTE translation x component 560, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_item_model_preview` | 27 | `1, 0x8c` | `rot.t[1] = 0x8c;` | GTE translation y component 140, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_item_model_preview` | 28 | `2, 0x5dc` | `rot.t[2] = 0x5dc;` | GTE translation z component 1500, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_item_model_preview` | 34 | `0 × 2` | `lsrc.m[0][0] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_item_model_preview` | 35 | `0, 1` | `lsrc.m[0][1] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_item_model_preview` | 36 | `0, 2` | `lsrc.m[0][2] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_item_model_preview` | 37 | `1, 0` | `lsrc.m[1][0] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_item_model_preview` | 38 | `1 × 2` | `lsrc.m[1][1] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_item_model_preview` | 39 | `1, 2` | `lsrc.m[1][2] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_item_model_preview` | 40 | `2, 0 × 2` | `lsrc.m[2][0] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_item_model_preview` | 41 | `2, 1, 0` | `lsrc.m[2][1] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_item_model_preview` | 42 | `2 × 2, 0` | `lsrc.m[2][2] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_item_model_preview` | 51 | `0xae` | `gs.x = 0xae;` | Authored text-origin x coordinate 174 screen pixels; retained as local layout data, with original placement rationale unknown. |
| `menu_item_model_preview` | 52 | `0x24` | `gs.y = 0x24;` | Authored text-origin y coordinate 36 screen pixels; retained as local layout data, with original placement rationale unknown. |
| `menu_item_model_preview` | 54 | `0` | `for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {` | Copy the ten signed glyph halfwords of the shared name-row representation, starting at its first entry. |
| `menu_item_model_preview` | 59 | `0xe6` | `gs.x = 0xe6;` | Authored text-origin x coordinate 230 screen pixels; retained as local layout data, with original placement rationale unknown. |
| `menu_item_model_preview` | 60 | `0, 0xca` | `gs.codes[0] = 0xca;` | Consecutive glyph or terminator position in the authored gold/held-quantity labels. Numeric right-hand sides are font-atlas codes; named diacritic/terminator bits retain their distinct encoding meanings. |
| `menu_item_model_preview` | 61 | `1, 0xcb` | `gs.codes[1] = 0xcb;` | Consecutive glyph or terminator position in the authored gold/held-quantity labels. Numeric right-hand sides are font-atlas codes; named diacritic/terminator bits retain their distinct encoding meanings. |
| `menu_item_model_preview` | 62 | `2` | `gs.codes[2] = MENU_TEXT_END;` | Consecutive glyph or terminator position in the authored gold/held-quantity labels. Numeric right-hand sides are font-atlas codes; named diacritic/terminator bits retain their distinct encoding meanings. |
| `menu_item_model_preview` | 63 | `18` | `gs.y += 18;` | Advance the price/quantity baseline by 18 screen pixels; authored line spacing. |
| `menu_item_model_preview` | 66 | `0x117` | `gs.x = 0x117;` | Authored text-origin x coordinate 279 screen pixels; retained as local layout data, with original placement rationale unknown. |
| `menu_item_model_preview` | 67 | `0 × 2, 2` | `menu_format_number(item_stock[0][item_id], 2, 0, gs.codes);` | Player-owned stock bank zero; two quantity digits fit the known 99 stack capacity, with blank leading padding. |
| `menu_draw_item_detail` | 34 | `0, 0x230` | `rot.t[0] = 0x230;` | GTE translation x component 560, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_draw_item_detail` | 35 | `1, 0x8c` | `rot.t[1] = 0x8c;` | GTE translation y component 140, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_draw_item_detail` | 36 | `2, 0x5dc` | `rot.t[2] = 0x5dc;` | GTE translation z component 1500, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_draw_item_detail` | 42 | `0 × 2` | `lsrc.m[0][0] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_detail` | 43 | `0, 1` | `lsrc.m[0][1] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_detail` | 44 | `0, 2` | `lsrc.m[0][2] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_detail` | 45 | `1, 0` | `lsrc.m[1][0] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_detail` | 46 | `1 × 2` | `lsrc.m[1][1] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_detail` | 47 | `1, 2` | `lsrc.m[1][2] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_detail` | 48 | `2, 0 × 2` | `lsrc.m[2][0] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_detail` | 49 | `2, 1, 0` | `lsrc.m[2][1] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_detail` | 50 | `2 × 2, 0` | `lsrc.m[2][2] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_detail` | 59 | `0xae` | `gs.x = 0xae;` | Authored text-origin x coordinate 174 screen pixels; retained as local layout data, with original placement rationale unknown. |
| `menu_draw_item_detail` | 60 | `0x24` | `gs.y = 0x24;` | Authored text-origin y coordinate 36 screen pixels; retained as local layout data, with original placement rationale unknown. |
| `menu_draw_item_detail` | 62 | `0` | `for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {` | Copy the ten signed glyph halfwords of the shared name-row representation, starting at its first entry. |
| `menu_draw_item_detail` | 67 | `0xc8` | `gs.x = 0xc8;` | Authored text-origin x coordinate 200 screen pixels; retained as local layout data, with original placement rationale unknown. |
| `menu_draw_item_detail` | 70 | `1, 6, 0` | `menu_format_number(prices[item_id][shop_id - 1], 6, 0, gs.codes);` | Convert one-based shop column to a zero-based array index; render six price digits with blank rather than zero padding. |
| `menu_draw_item_detail` | 73, 81 | `0xf2 × 2` | `gs.x = 0xf2;` | Authored text-origin x coordinate 242 screen pixels; retained as local layout data, with original placement rationale unknown. |
| `menu_draw_item_detail` | 74 | `0, 0x9` | `gs.codes[0] = MENU_TEXT_DAKUTEN \| 0x9;` | Consecutive glyph or terminator position in the authored gold/held-quantity labels. Numeric right-hand sides are font-atlas codes; named diacritic/terminator bits retain their distinct encoding meanings. |
| `menu_draw_item_detail` | 75 | `1, 0x2d` | `gs.codes[1] = 0x2d;` | Consecutive glyph or terminator position in the authored gold/held-quantity labels. Numeric right-hand sides are font-atlas codes; named diacritic/terminator bits retain their distinct encoding meanings. |
| `menu_draw_item_detail` | 76 | `2, 0x2a` | `gs.codes[2] = 0x2a;` | Consecutive glyph or terminator position in the authored gold/held-quantity labels. Numeric right-hand sides are font-atlas codes; named diacritic/terminator bits retain their distinct encoding meanings. |
| `menu_draw_item_detail` | 77 | `3, 0x13` | `gs.codes[3] = MENU_TEXT_DAKUTEN \| 0x13;` | Consecutive glyph or terminator position in the authored gold/held-quantity labels. Numeric right-hand sides are font-atlas codes; named diacritic/terminator bits retain their distinct encoding meanings. |
| `menu_draw_item_detail` | 78 | `4` | `gs.codes[4] = MENU_TEXT_END;` | Consecutive glyph or terminator position in the authored gold/held-quantity labels. Numeric right-hand sides are font-atlas codes; named diacritic/terminator bits retain their distinct encoding meanings. |
| `menu_draw_item_detail` | 82 | `0, 0xca` | `gs.codes[0] = 0xca;` | Consecutive glyph or terminator position in the authored gold/held-quantity labels. Numeric right-hand sides are font-atlas codes; named diacritic/terminator bits retain their distinct encoding meanings. |
| `menu_draw_item_detail` | 83 | `1, 0xcb` | `gs.codes[1] = 0xcb;` | Consecutive glyph or terminator position in the authored gold/held-quantity labels. Numeric right-hand sides are font-atlas codes; named diacritic/terminator bits retain their distinct encoding meanings. |
| `menu_draw_item_detail` | 84 | `2` | `gs.codes[2] = MENU_TEXT_END;` | Consecutive glyph or terminator position in the authored gold/held-quantity labels. Numeric right-hand sides are font-atlas codes; named diacritic/terminator bits retain their distinct encoding meanings. |
| `menu_draw_item_detail` | 88 | `0x11c` | `gs.x = 0x11c;` | Authored text-origin x coordinate 284 screen pixels; retained as local layout data, with original placement rationale unknown. |
| `menu_draw_item_detail` | 89 | `0 × 2, 2` | `menu_format_number(item_stock[0][item_id], 2, 0, gs.codes);` | Player-owned stock bank zero; two quantity digits fit the known 99 stack capacity, with blank leading padding. |
| `menu_draw_item_detail` | 97 | `28` | `gs.x = menu_window_layouts[KF_ENUM_ENCODE(s32, KF_MENU_WINDOW_SHOP)].rows[KF_SHOP_ROW_GOLD].x + 28;` | Place the gold value 28 screen pixels right of menu window 7 row 3, with the original spacing rationale unproven. |
| `menu_draw_item_detail` | 99 | `6, 0` | `menu_format_number(player_state.gold, 6, 0, gs.codes);` | Render six gold digits without leading zeros; authored field width, shared numeric-render API Boolean. |
| `initializers` | 20 | `0 × 2, 0x1c, 1 × 2` | `(u32)&((MenuWindowLayout *)0)->rows[0].codes == 0x1c ? 1 : -1];` | Existing offset check: first row code prefix is byte 28; null-base constant member-address expression and positive/negative bound encode the check. This is not a size assertion. |
| `initializers` | 31 | `0` | `POLY_FT4 *current_poly_ft4 = 0;` | Initialized null current primitive pointer; each renderer binds it to the active primitive cursor before use. |
| `menu_draw_item_name_frame` | 85 | `0, 0xdc` | `rotation.t[0] = 0xdc;` | GTE translation x component 220, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_draw_item_name_frame` | 86 | `1, 0x8c` | `rotation.t[1] = 0x8c;` | GTE translation y component 140, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_draw_item_name_frame` | 87 | `2, 0x5dc` | `rotation.t[2] = 0x5dc;` | GTE translation z component 1500, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_draw_item_name_frame` | 93 | `0 × 2` | `light_source.m[0][0] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 94 | `0, 1` | `light_source.m[0][1] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 95 | `0, 2` | `light_source.m[0][2] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 96 | `1, 0` | `light_source.m[1][0] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 97 | `1 × 2` | `light_source.m[1][1] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 98 | `1, 2` | `light_source.m[1][2] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 99 | `2, 0 × 2` | `light_source.m[2][0] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 100 | `2, 1, 0` | `light_source.m[2][1] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 101 | `2 × 2, 0` | `light_source.m[2][2] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 110 | `0x80` | `string.x = 0x80;` | Authored text-origin x coordinate 128 screen pixels; retained as local layout data, with original placement rationale unknown. |
| `menu_draw_item_name_frame` | 111 | `0x24` | `string.y = 0x24;` | Authored text-origin y coordinate 36 screen pixels; retained as local layout data, with original placement rationale unknown. |
| `menu_draw_item_name_frame` | 112 | `0` | `for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {` | Copy the ten signed glyph halfwords of the shared name-row representation, starting at its first entry. |
| `menu_draw_item_name_frame` | 118, 140, 162, 184 | `1 × 4` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency on each of the four mirrored pickup-window quads. |
| `menu_draw_item_name_frame` | 207 | `3` | `&menu_assets.background_quads[display_state.buffer_index][3]);` | Persistent background-quad indices 3,2,1,0 inserted in reverse order; equal-depth AddPrim prepending reverses their submission order. |
| `menu_draw_item_name_frame` | 210 | `2` | `&menu_assets.background_quads[display_state.buffer_index][2]);` | Persistent background-quad indices 3,2,1,0 inserted in reverse order; equal-depth AddPrim prepending reverses their submission order. |
| `menu_draw_item_name_frame` | 213 | `1` | `&menu_assets.background_quads[display_state.buffer_index][1]);` | Persistent background-quad indices 3,2,1,0 inserted in reverse order; equal-depth AddPrim prepending reverses their submission order. |
| `menu_draw_item_name_frame` | 216 | `0` | `&menu_assets.background_quads[display_state.buffer_index][0]);` | Persistent background-quad indices 3,2,1,0 inserted in reverse order; equal-depth AddPrim prepending reverses their submission order. |
| `menu_load_item_model` | 617 | `0` | `if (cd_file_load_table_entry(&asset, id) != 0) {` | Nonzero file-loader status is failure and exits before yaw reset or model-registration publication. |
| `menu_load_item_model` | 618 | `1` | `return 1;` | Model-loader failure result; existing callers test nonzero or equality to one. Full loader-result enum propagation remains separate from rotation recovery. |
| `menu_load_item_model` | 621 | `1` | `menu_item_model_allocation_pending = 1;` | Publish that the menu TMD slot now owns an allocation to release. |
| `menu_load_item_model` | 623 | `0` | `menu_item_preview_rotation.vy = 0;` | Reset the successfully loaded or empty model to zero yaw; pitch/roll and SDK alignment halfword remain intact. |
| `menu_load_item_model` | 624 | `0` | `return 0;` | Model-loader success result, including the empty-slot no-load path. |
| `menu_release_item_model` | 630 | `1` | `if (menu_item_model_allocation_pending == 1) {` | Release only the exact published allocation-pending value one; preserve equality instead of widening to any nonzero value. |
| `menu_release_item_model` | 632 | `0` | `menu_item_model_allocation_pending = 0;` | Clear the allocation-pending flag after releasing the TMD slot. |
| `menu_render_item_model` | 112 | `0xc8` | `SetGeomScreen(0xc8);` | Projection-plane distance 200 in the SDK GTE projection scale; controls perspective magnification. Original authored choice unknown. |
| `menu_render_item_model` | 114 | `0` | `tmd_select_object_vertices(0);` | The menu model uses object zero within the selected TMD asset. |
| `menu_render_item_model` | 115 | `0` | `tmd_project_vertices(tmd_get_object(0)->vertex_count);` | Project the vertex extent of object zero from the selected menu TMD. |
| `menu_render_item_model` | 116 | `0, 0x3e8` | `render_enqueue_tmd(0, 0x3e8);` | Enqueue object zero at authored menu depth parameter 1000; preserve the established renderer layering independently of projection distance. |

The [preview-layout review](game-menu-preview-layout.md) names shared transform,
text and formatting settings. Use the [complete inventory-preview ledger](game-menu-inventory-preview-literal-ledger.md)
and [complete detail ledger](game-menu-detail-literal-ledger.md) for current expressions and counts.
