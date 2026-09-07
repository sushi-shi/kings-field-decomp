# Retained GAME menu-runtime literals

Complete account of **103 numeric/character occurrences** in
`src/game/menu_runtime.c` after the [runtime review](game-menu-runtime-constants.md).
Comments, string contents, identifier digits, named definitions and retail claims
are excluded. Every repeated token has its own row; signs are expression operators.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 39 | `0` | `(u32)&((MenuWindowLayout *)0)->rows[0].codes == 0x1c ? 1 : -1];` | Existing offset check: first row code prefix is byte 28; null-base constant member-address expression and positive/negative bound encode the check. This is not a size assertion. |
| `initializers` | 39 | `0` | `(u32)&((MenuWindowLayout *)0)->rows[0].codes == 0x1c ? 1 : -1];` | Existing offset check: first row code prefix is byte 28; null-base constant member-address expression and positive/negative bound encode the check. This is not a size assertion. |
| `initializers` | 39 | `0x1c` | `(u32)&((MenuWindowLayout *)0)->rows[0].codes == 0x1c ? 1 : -1];` | Existing offset check: first row code prefix is byte 28; null-base constant member-address expression and positive/negative bound encode the check. This is not a size assertion. |
| `initializers` | 39 | `1` | `(u32)&((MenuWindowLayout *)0)->rows[0].codes == 0x1c ? 1 : -1];` | Existing offset check: first row code prefix is byte 28; null-base constant member-address expression and positive/negative bound encode the check. This is not a size assertion. |
| `initializers` | 39 | `1` | `(u32)&((MenuWindowLayout *)0)->rows[0].codes == 0x1c ? 1 : -1];` | Existing offset check: first row code prefix is byte 28; null-base constant member-address expression and positive/negative bound encode the check. This is not a size assertion. |
| `initializers` | 50 | `0` | `POLY_FT4 *current_poly_ft4 = 0;` | Initialized null current primitive pointer; each renderer binds it to the active primitive cursor before use. |
| `menu_draw_two_option` | 63 | `0` | `if (selected == 0) {` | Zero selects the first supplied option; the else path handles every other value. This is a positional selector, not an independently identified gameplay state. |
| `menu_draw_two_option` | 68 | `1` | `if (highlight == 1) {` | Highlight is enabled only for the exact Boolean value one; preserve equality rather than treating every nonzero value as true. |
| `menu_draw_two_option` | 69 | `0` | `if (selected == 0) {` | Zero selects the first supplied option; the else path handles every other value. This is a positional selector, not an independently identified gameplay state. |
| `menu_draw_item_name_frame` | 104 | `0` | `rotation.t[0] = 0xdc;` | GTE translation x component 220, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_draw_item_name_frame` | 104 | `0xdc` | `rotation.t[0] = 0xdc;` | GTE translation x component 220, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_draw_item_name_frame` | 105 | `1` | `rotation.t[1] = 0x8c;` | GTE translation y component 140, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_draw_item_name_frame` | 105 | `0x8c` | `rotation.t[1] = 0x8c;` | GTE translation y component 140, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_draw_item_name_frame` | 106 | `2` | `rotation.t[2] = 0x5dc;` | GTE translation z component 1500, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_draw_item_name_frame` | 106 | `0x5dc` | `rotation.t[2] = 0x5dc;` | GTE translation z component 1500, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_draw_item_name_frame` | 112 | `0` | `light_source.m[0][0] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 112 | `0` | `light_source.m[0][0] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 113 | `0` | `light_source.m[0][1] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 113 | `1` | `light_source.m[0][1] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 114 | `0` | `light_source.m[0][2] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 114 | `2` | `light_source.m[0][2] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 115 | `1` | `light_source.m[1][0] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 115 | `0` | `light_source.m[1][0] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 116 | `1` | `light_source.m[1][1] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 116 | `1` | `light_source.m[1][1] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 117 | `1` | `light_source.m[1][2] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 117 | `2` | `light_source.m[1][2] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 118 | `2` | `light_source.m[2][0] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 118 | `0` | `light_source.m[2][0] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 118 | `0` | `light_source.m[2][0] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 119 | `2` | `light_source.m[2][1] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 119 | `1` | `light_source.m[2][1] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 119 | `0` | `light_source.m[2][1] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 120 | `2` | `light_source.m[2][2] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 120 | `2` | `light_source.m[2][2] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 120 | `0` | `light_source.m[2][2] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 129 | `0x80` | `string.x = 0x80;` | Authored text-origin x coordinate 128 screen pixels; retained as local layout data, with original placement rationale unknown. |
| `menu_draw_item_name_frame` | 130 | `0x24` | `string.y = 0x24;` | Authored text-origin y coordinate 36 screen pixels; retained as local layout data, with original placement rationale unknown. |
| `menu_draw_item_name_frame` | 131 | `0` | `for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {` | Copy the ten signed glyph halfwords of the shared name-row representation, starting at its first entry. |
| `menu_draw_item_name_frame` | 137 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency on each of the four mirrored pickup-window quads. |
| `menu_draw_item_name_frame` | 159 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency on each of the four mirrored pickup-window quads. |
| `menu_draw_item_name_frame` | 181 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency on each of the four mirrored pickup-window quads. |
| `menu_draw_item_name_frame` | 203 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency on each of the four mirrored pickup-window quads. |
| `menu_draw_item_name_frame` | 226 | `3` | `&menu_assets.background_quads[display_state.buffer_index][3]);` | Persistent background-quad indices 3,2,1,0 inserted in reverse order; equal-depth AddPrim prepending reverses their submission order. |
| `menu_draw_item_name_frame` | 229 | `2` | `&menu_assets.background_quads[display_state.buffer_index][2]);` | Persistent background-quad indices 3,2,1,0 inserted in reverse order; equal-depth AddPrim prepending reverses their submission order. |
| `menu_draw_item_name_frame` | 232 | `1` | `&menu_assets.background_quads[display_state.buffer_index][1]);` | Persistent background-quad indices 3,2,1,0 inserted in reverse order; equal-depth AddPrim prepending reverses their submission order. |
| `menu_draw_item_name_frame` | 235 | `0` | `&menu_assets.background_quads[display_state.buffer_index][0]);` | Persistent background-quad indices 3,2,1,0 inserted in reverse order; equal-depth AddPrim prepending reverses their submission order. |
| `menu_blit_sprite_translucent` | 259 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency for this textured quad. |
| `menu_draw_string` | 299 | `0` | `for (i = 0; string->codes[i] != MENU_TEXT_END; i++) {` | Begin at the first zero-based glyph position; the named terminator bounds the traversal. |
| `menu_draw_number` | 387 | `0` | `for (i = 0; label->codes[i] != MENU_TEXT_END; i++) {` | Begin at the first zero-based glyph position; the named terminator bounds the traversal. |
| `menu_draw_window_backdrop` | 424 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency for this textured quad. |
| `menu_draw_window_backdrop` | 446 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency for this textured quad. |
| `menu_draw_window_backdrop` | 468 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency for this textured quad. |
| `menu_draw_window_backdrop` | 490 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency for this textured quad. |
| `menu_draw_window_backdrop` | 513 | `3` | `&menu_assets.background_quads[display_state.buffer_index][3]);` | Explicit persistent-quad index in reverse insertion order 3,2,1,0; AddPrim inserts at the bucket head, so preserve this sequence. |
| `menu_draw_window_backdrop` | 516 | `2` | `&menu_assets.background_quads[display_state.buffer_index][2]);` | Explicit persistent-quad index in reverse insertion order 3,2,1,0; AddPrim inserts at the bucket head, so preserve this sequence. |
| `menu_draw_window_backdrop` | 519 | `1` | `&menu_assets.background_quads[display_state.buffer_index][1]);` | Explicit persistent-quad index in reverse insertion order 3,2,1,0; AddPrim inserts at the bucket head, so preserve this sequence. |
| `menu_draw_window_backdrop` | 522 | `0` | `&menu_assets.background_quads[display_state.buffer_index][0]);` | Explicit persistent-quad index in reverse insertion order 3,2,1,0; AddPrim inserts at the bucket head, so preserve this sequence. |
| `menu_frame_begin` | 533 | `0` | `display_state.buffer_index = display_state.buffer_index == 0;` | Toggle the double-buffer selector by logical negation: zero becomes one and every nonzero value becomes zero. |
| `menu_present_frame` | 547 | `0` | `DrawSync(0);` | SDK blocking synchronization mode waits for queued GPU drawing to finish. |
| `menu_present_frame` | 548 | `0` | `VSync(0);` | SDK synchronization mode waits for the next vertical blank. |
| `menu_present_frame` | 551 | `1` | `DrawOTag(display_state.ordering_table + (KF_ORDERING_TABLE_LENGTH - 1));` | Subtract one from the entry count to address the final entry of the reverse ordering table. |
| `primitive_buffer_commit_poly_ft4` | 566 | `2` | `depth <<= 2;` | Convert a depth index to a byte offset by multiplying by four, the width of each ordering-table entry; this is address scaling, not projected-depth scaling. |
| `menu_list_init` | 580 | `12` | `list->title_x = 12;` | Authored list-title X origin, in screen pixels, also used by the loaded titled window records; no recovered rationale chooses twelve. |
| `menu_list_init` | 581 | `19` | `list->title_y = 19;` | Authored list-title Y origin, in screen pixels, matching the loaded title baseline; no recovered rationale chooses nineteen. |
| `menu_list_init` | 582 | `0` | `for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {` | Copy positions zero through nine of the ten-halfword label representation into the equally sized title buffer, including any terminator and trailing words; do not turn this into a terminator-controlled copy. |
| `menu_list_init` | 585 | `0x16` | `list->list_x = 0x16;` | Authored list-panel X origin, 22 screen pixels. The renderer adds its three-pixel text inset independently. |
| `menu_list_init` | 586 | `0x26` | `list->list_y = 0x26;` | Authored list-panel Y origin, 38 screen pixels. The renderer adds its three-pixel inset and twelve-pixel row offsets. |
| `menu_list_init` | 587 | `0` | `list->entry_count = 0;` | The list initially has no entries; each caller fills the count after constructing its rows. |
| `menu_list_init` | 589 | `0` | `list->scroll_offset = 0;` | Start the visible window at entry zero. |
| `menu_list_init` | 590 | `0` | `list->selected_index = 0;` | Initial selection is the first entry, with empty lists handled by the caller. |
| `menu_list_init` | 591 | `0` | `list->cursor_row = 0;` | Initial highlight is the top visible row. |
| `menu_format_number` | 599 | `0` | `s32 i = 0;` | Begin filling the output at its first zero-based glyph position. |
| `menu_format_number` | 602 | `0` | `blank = (pad_zero == 0) ? MENU_NUMBER_BLANK : 0;` | Zero pad flag requests the named blank glyph; otherwise the numeric atlas code zero supplies a leading zero digit. |
| `menu_format_number` | 602 | `0` | `blank = (pad_zero == 0) ? MENU_NUMBER_BLANK : 0;` | Zero pad flag requests the named blank glyph; otherwise the numeric atlas code zero supplies a leading zero digit. |
| `menu_format_number` | 607 | `1` | `for (i = count - 1; i >= 0; i--) {` | Start at the last of count digit positions and walk through position zero, inclusive. |
| `menu_format_number` | 607 | `0` | `for (i = count - 1; i >= 0; i--) {` | Start at the last of count digit positions and walk through position zero, inclusive. |
| `menu_format_number` | 608 | `10` | `out[i] = value % 10;` | Decimal radix: remainder extracts the next base-ten digit, already equal to its numeric atlas code. |
| `menu_format_number` | 609 | `10` | `value /= 10;` | Decimal radix: signed integer division removes the digit just emitted. |
| `menu_format_number` | 610 | `0` | `if (value == 0) {` | A zero quotient means no more significant digits remain. |
| `menu_format_number` | 611 | `1` | `i = -1;` | Set a negative loop index so the next loop test exits after its decrement; preserve this control-flow form. |
| `menu_load_item_model` | 624 | `0` | `if (cd_file_load_table_entry(&asset, id) != 0) {` | Nonzero file-loader status is failure and exits before yaw reset or model-registration publication. |
| `menu_load_item_model` | 625 | `1` | `return 1;` | Model-loader failure result; existing callers test nonzero or equality to one. Full loader-result enum propagation remains separate from rotation recovery. |
| `menu_load_item_model` | 628 | `1` | `menu_item_model_allocation_pending = 1;` | Publish that the menu TMD slot now owns an allocation to release. |
| `menu_load_item_model` | 630 | `0` | `menu_item_preview_rotation.vy = 0;` | Reset the successfully loaded or empty model to zero yaw; pitch/roll and SDK alignment halfword remain intact. |
| `menu_load_item_model` | 631 | `0` | `return 0;` | Model-loader success result, including the empty-slot no-load path. |
| `menu_release_item_model` | 637 | `1` | `if (menu_item_model_allocation_pending == 1) {` | Release only the exact published allocation-pending value one; preserve equality instead of widening to any nonzero value. |
| `menu_release_item_model` | 639 | `0` | `menu_item_model_allocation_pending = 0;` | Clear the allocation-pending flag after releasing the TMD slot. |
| `menu_load_item_texture` | 647 | `16` | `char name[16] = "TIM\\M000.";` | Local sixteen-byte filename workspace initialized from the retail template, including zero fill beyond its terminator; its extra capacity has no recovered semantic unit. |
| `menu_load_item_texture` | 653 | `1` | `number = id + 1;` | Convert the zero-based menu texture ID to the one-based decimal filename number. |
| `menu_load_item_texture` | 654 | `5` | `name[5] = number / 100 + '0';` | Template byte5 is the hundreds digit; division by decimal hundred and ASCII zero encode that position directly. |
| `menu_load_item_texture` | 654 | `100` | `name[5] = number / 100 + '0';` | Template byte5 is the hundreds digit; division by decimal hundred and ASCII zero encode that position directly. |
| `menu_load_item_texture` | 654 | `'0'` | `name[5] = number / 100 + '0';` | Template byte5 is the hundreds digit; division by decimal hundred and ASCII zero encode that position directly. |
| `menu_load_item_texture` | 655 | `100` | `remainder = number % 100;` | Keep the final two decimal digits after extracting the hundreds digit. |
| `menu_load_item_texture` | 656 | `6` | `name[6] = remainder / 10 + '0';` | Template byte6 is the tens digit; division by decimal ten and ASCII zero encode that position directly. |
| `menu_load_item_texture` | 656 | `10` | `name[6] = remainder / 10 + '0';` | Template byte6 is the tens digit; division by decimal ten and ASCII zero encode that position directly. |
| `menu_load_item_texture` | 656 | `'0'` | `name[6] = remainder / 10 + '0';` | Template byte6 is the tens digit; division by decimal ten and ASCII zero encode that position directly. |
| `menu_load_item_texture` | 657 | `7` | `name[7] = remainder % 10 + '0';` | Template byte7 is the units digit; remainder modulo decimal ten and ASCII zero encode that position directly. |
| `menu_load_item_texture` | 657 | `10` | `name[7] = remainder % 10 + '0';` | Template byte7 is the units digit; remainder modulo decimal ten and ASCII zero encode that position directly. |
| `menu_load_item_texture` | 657 | `'0'` | `name[7] = remainder % 10 + '0';` | Template byte7 is the units digit; remainder modulo decimal ten and ASCII zero encode that position directly. |
| `menu_load_item_texture` | 659 | `0` | `if (cd_file_load_into(destination, name) != 0) {` | Propagate any nonzero loader failure through the wrapper failure result. |
| `menu_load_item_texture` | 660 | `1` | `return 1;` | Wrapper failure result; the loader failed before texture upload. |
| `menu_load_item_texture` | 664 | `0` | `return 0;` | Wrapper success result, including the no-texture sentinel path. |
