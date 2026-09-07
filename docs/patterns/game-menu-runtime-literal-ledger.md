# Retained GAME menu-runtime literals

Complete account of **100 numeric/character occurrences** in
`src/game/menu_runtime.c` after the [runtime review](game-menu-runtime-constants.md).
Comments, string contents, identifier digits, named definitions and retail claims
are excluded. Every repeated token has its own row; signs are expression operators.
The [confirmation-choice review](game-menu-confirm-choice.md) names the two
first-choice comparisons in the widget renderer.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 33 | `0` | `(u32)&((MenuWindowLayout *)0)->rows[0].codes == 0x1c ? 1 : -1];` | Existing offset check: first row code prefix is byte 28; null-base constant member-address expression and positive/negative bound encode the check. This is not a size assertion. |
| `initializers` | 33 | `0` | `(u32)&((MenuWindowLayout *)0)->rows[0].codes == 0x1c ? 1 : -1];` | Existing offset check: first row code prefix is byte 28; null-base constant member-address expression and positive/negative bound encode the check. This is not a size assertion. |
| `initializers` | 33 | `0x1c` | `(u32)&((MenuWindowLayout *)0)->rows[0].codes == 0x1c ? 1 : -1];` | Existing offset check: first row code prefix is byte 28; null-base constant member-address expression and positive/negative bound encode the check. This is not a size assertion. |
| `initializers` | 33 | `1` | `(u32)&((MenuWindowLayout *)0)->rows[0].codes == 0x1c ? 1 : -1];` | Existing offset check: first row code prefix is byte 28; null-base constant member-address expression and positive/negative bound encode the check. This is not a size assertion. |
| `initializers` | 33 | `1` | `(u32)&((MenuWindowLayout *)0)->rows[0].codes == 0x1c ? 1 : -1];` | Existing offset check: first row code prefix is byte 28; null-base constant member-address expression and positive/negative bound encode the check. This is not a size assertion. |
| `initializers` | 44 | `0` | `POLY_FT4 *current_poly_ft4 = 0;` | Initialized null current primitive pointer; each renderer binds it to the active primitive cursor before use. |
| `menu_draw_item_name_frame` | 98 | `0` | `rotation.t[0] = 0xdc;` | GTE translation x component 220, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_draw_item_name_frame` | 98 | `0xdc` | `rotation.t[0] = 0xdc;` | GTE translation x component 220, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_draw_item_name_frame` | 99 | `1` | `rotation.t[1] = 0x8c;` | GTE translation y component 140, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_draw_item_name_frame` | 99 | `0x8c` | `rotation.t[1] = 0x8c;` | GTE translation y component 140, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_draw_item_name_frame` | 100 | `2` | `rotation.t[2] = 0x5dc;` | GTE translation z component 1500, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_draw_item_name_frame` | 100 | `0x5dc` | `rotation.t[2] = 0x5dc;` | GTE translation z component 1500, in the transform coordinate units; authored preview placement, not an angle or Q12 coefficient. |
| `menu_draw_item_name_frame` | 106 | `0` | `light_source.m[0][0] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 106 | `0` | `light_source.m[0][0] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 107 | `0` | `light_source.m[0][1] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 107 | `1` | `light_source.m[0][1] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 108 | `0` | `light_source.m[0][2] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 108 | `2` | `light_source.m[0][2] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 109 | `1` | `light_source.m[1][0] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 109 | `0` | `light_source.m[1][0] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 110 | `1` | `light_source.m[1][1] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 110 | `1` | `light_source.m[1][1] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 111 | `1` | `light_source.m[1][2] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 111 | `2` | `light_source.m[1][2] = -KF_FIXED12_ONE;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 112 | `2` | `light_source.m[2][0] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 112 | `0` | `light_source.m[2][0] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 112 | `0` | `light_source.m[2][0] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 113 | `2` | `light_source.m[2][1] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 113 | `1` | `light_source.m[2][1] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 113 | `0` | `light_source.m[2][1] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 114 | `2` | `light_source.m[2][2] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 114 | `2` | `light_source.m[2][2] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 114 | `0` | `light_source.m[2][2] = 0;` | Matrix row/column indices select the 3×3 directional coefficients: first two rows use named negative Q12 unity; third row is zero and contributes no direction. |
| `menu_draw_item_name_frame` | 123 | `0x80` | `string.x = 0x80;` | Authored text-origin x coordinate 128 screen pixels; retained as local layout data, with original placement rationale unknown. |
| `menu_draw_item_name_frame` | 124 | `0x24` | `string.y = 0x24;` | Authored text-origin y coordinate 36 screen pixels; retained as local layout data, with original placement rationale unknown. |
| `menu_draw_item_name_frame` | 125 | `0` | `for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {` | Copy the ten signed glyph halfwords of the shared name-row representation, starting at its first entry. |
| `menu_draw_item_name_frame` | 131 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency on each of the four mirrored pickup-window quads. |
| `menu_draw_item_name_frame` | 153 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency on each of the four mirrored pickup-window quads. |
| `menu_draw_item_name_frame` | 175 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency on each of the four mirrored pickup-window quads. |
| `menu_draw_item_name_frame` | 197 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency on each of the four mirrored pickup-window quads. |
| `menu_draw_item_name_frame` | 220 | `3` | `&menu_assets.background_quads[display_state.buffer_index][3]);` | Persistent background-quad indices 3,2,1,0 inserted in reverse order; equal-depth AddPrim prepending reverses their submission order. |
| `menu_draw_item_name_frame` | 223 | `2` | `&menu_assets.background_quads[display_state.buffer_index][2]);` | Persistent background-quad indices 3,2,1,0 inserted in reverse order; equal-depth AddPrim prepending reverses their submission order. |
| `menu_draw_item_name_frame` | 226 | `1` | `&menu_assets.background_quads[display_state.buffer_index][1]);` | Persistent background-quad indices 3,2,1,0 inserted in reverse order; equal-depth AddPrim prepending reverses their submission order. |
| `menu_draw_item_name_frame` | 229 | `0` | `&menu_assets.background_quads[display_state.buffer_index][0]);` | Persistent background-quad indices 3,2,1,0 inserted in reverse order; equal-depth AddPrim prepending reverses their submission order. |
| `menu_blit_sprite_translucent` | 253 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency for this textured quad. |
| `menu_draw_string` | 293 | `0` | `for (i = 0; string->codes[i] != MENU_TEXT_END; i++) {` | Begin at the first zero-based glyph position; the named terminator bounds the traversal. |
| `menu_draw_number` | 381 | `0` | `for (i = 0; label->codes[i] != MENU_TEXT_END; i++) {` | Begin at the first zero-based glyph position; the named terminator bounds the traversal. |
| `menu_draw_window_backdrop` | 418 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency for this textured quad. |
| `menu_draw_window_backdrop` | 440 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency for this textured quad. |
| `menu_draw_window_backdrop` | 462 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency for this textured quad. |
| `menu_draw_window_backdrop` | 484 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean enables semi-transparency for this textured quad. |
| `menu_draw_window_backdrop` | 507 | `3` | `&menu_assets.background_quads[display_state.buffer_index][3]);` | Explicit persistent-quad index in reverse insertion order 3,2,1,0; AddPrim inserts at the bucket head, so preserve this sequence. |
| `menu_draw_window_backdrop` | 510 | `2` | `&menu_assets.background_quads[display_state.buffer_index][2]);` | Explicit persistent-quad index in reverse insertion order 3,2,1,0; AddPrim inserts at the bucket head, so preserve this sequence. |
| `menu_draw_window_backdrop` | 513 | `1` | `&menu_assets.background_quads[display_state.buffer_index][1]);` | Explicit persistent-quad index in reverse insertion order 3,2,1,0; AddPrim inserts at the bucket head, so preserve this sequence. |
| `menu_draw_window_backdrop` | 516 | `0` | `&menu_assets.background_quads[display_state.buffer_index][0]);` | Explicit persistent-quad index in reverse insertion order 3,2,1,0; AddPrim inserts at the bucket head, so preserve this sequence. |
| `menu_frame_begin` | 527 | `0` | `display_state.buffer_index = display_state.buffer_index == 0;` | Toggle the double-buffer selector by logical negation: zero becomes one and every nonzero value becomes zero. |
| `menu_present_frame` | 541 | `0` | `DrawSync(0);` | SDK blocking synchronization mode waits for queued GPU drawing to finish. |
| `menu_present_frame` | 542 | `0` | `VSync(0);` | SDK synchronization mode waits for the next vertical blank. |
| `menu_present_frame` | 545 | `1` | `DrawOTag(display_state.ordering_table + (KF_ORDERING_TABLE_LENGTH - 1));` | Subtract one from the entry count to address the final entry of the reverse ordering table. |
| `primitive_buffer_commit_poly_ft4` | 560 | `2` | `depth <<= 2;` | Convert a depth index to a byte offset by multiplying by four, the width of each ordering-table entry; this is address scaling, not projected-depth scaling. |
| `menu_list_init` | 574 | `12` | `list->title_x = 12;` | Authored list-title X origin, in screen pixels, also used by the loaded titled window records; no recovered rationale chooses twelve. |
| `menu_list_init` | 575 | `19` | `list->title_y = 19;` | Authored list-title Y origin, in screen pixels, matching the loaded title baseline; no recovered rationale chooses nineteen. |
| `menu_list_init` | 576 | `0` | `for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {` | Copy positions zero through nine of the ten-halfword label representation into the equally sized title buffer, including any terminator and trailing words; do not turn this into a terminator-controlled copy. |
| `menu_list_init` | 579 | `0x16` | `list->list_x = 0x16;` | Authored list-panel X origin, 22 screen pixels. The renderer adds its three-pixel text inset independently. |
| `menu_list_init` | 580 | `0x26` | `list->list_y = 0x26;` | Authored list-panel Y origin, 38 screen pixels. The renderer adds its three-pixel inset and twelve-pixel row offsets. |
| `menu_list_init` | 581 | `0` | `list->entry_count = 0;` | The list initially has no entries; each caller fills the count after constructing its rows. |
| `menu_list_init` | 583 | `0` | `list->scroll_offset = 0;` | Start the visible window at entry zero. |
| `menu_list_init` | 584 | `0` | `list->selected_index = 0;` | Initial selection is the first entry, with empty lists handled by the caller. |
| `menu_list_init` | 585 | `0` | `list->cursor_row = 0;` | Initial highlight is the top visible row. |
| `menu_format_number` | 593 | `0` | `s32 i = 0;` | Begin filling the output at its first zero-based glyph position. |
| `menu_format_number` | 596 | `0` | `blank = (pad_zero == 0) ? MENU_NUMBER_BLANK : 0;` | Zero pad flag requests the named blank glyph; otherwise the numeric atlas code zero supplies a leading zero digit. |
| `menu_format_number` | 596 | `0` | `blank = (pad_zero == 0) ? MENU_NUMBER_BLANK : 0;` | Zero pad flag requests the named blank glyph; otherwise the numeric atlas code zero supplies a leading zero digit. |
| `menu_format_number` | 601 | `1` | `for (i = count - 1; i >= 0; i--) {` | Start at the last of count digit positions and walk through position zero, inclusive. |
| `menu_format_number` | 601 | `0` | `for (i = count - 1; i >= 0; i--) {` | Start at the last of count digit positions and walk through position zero, inclusive. |
| `menu_format_number` | 602 | `10` | `out[i] = value % 10;` | Decimal radix: remainder extracts the next base-ten digit, already equal to its numeric atlas code. |
| `menu_format_number` | 603 | `10` | `value /= 10;` | Decimal radix: signed integer division removes the digit just emitted. |
| `menu_format_number` | 604 | `0` | `if (value == 0) {` | A zero quotient means no more significant digits remain. |
| `menu_format_number` | 605 | `1` | `i = -1;` | Set a negative loop index so the next loop test exits after its decrement; preserve this control-flow form. |
| `menu_load_item_model` | 618 | `0` | `if (cd_file_load_table_entry(&asset, KF_ENUM_ENCODE(s32, id)) != 0) {` | Nonzero file-loader status is failure and exits before yaw reset or model-registration publication. |
| `menu_load_item_model` | 619 | `1` | `return 1;` | Model-loader failure result; existing callers test nonzero or equality to one. Full loader-result enum propagation remains separate from rotation recovery. |
| `menu_load_item_model` | 622 | `1` | `menu_item_model_allocation_pending = 1;` | Publish that the menu TMD slot now owns an allocation to release. |
| `menu_load_item_model` | 624 | `0` | `menu_item_preview_rotation.vy = 0;` | Reset the successfully loaded or empty model to zero yaw; pitch/roll and SDK alignment halfword remain intact. |
| `menu_load_item_model` | 625 | `0` | `return 0;` | Model-loader success result, including the empty-slot no-load path. |
| `menu_release_item_model` | 631 | `1` | `if (menu_item_model_allocation_pending == 1) {` | Release only the exact published allocation-pending value one; preserve equality instead of widening to any nonzero value. |
| `menu_release_item_model` | 633 | `0` | `menu_item_model_allocation_pending = 0;` | Clear the allocation-pending flag after releasing the TMD slot. |
| `menu_load_item_texture` | 641 | `16` | `char name[16] = "TIM\\M000.";` | Local sixteen-byte filename workspace initialized from the retail template, including zero fill beyond its terminator; its extra capacity has no recovered semantic unit. |
| `menu_load_item_texture` | 647 | `1` | `number = id + 1;` | Convert the zero-based menu texture ID to the one-based decimal filename number. |
| `menu_load_item_texture` | 648 | `5` | `name[5] = number / 100 + '0';` | Template byte5 is the hundreds digit; division by decimal hundred and ASCII zero encode that position directly. |
| `menu_load_item_texture` | 648 | `100` | `name[5] = number / 100 + '0';` | Template byte5 is the hundreds digit; division by decimal hundred and ASCII zero encode that position directly. |
| `menu_load_item_texture` | 648 | `'0'` | `name[5] = number / 100 + '0';` | Template byte5 is the hundreds digit; division by decimal hundred and ASCII zero encode that position directly. |
| `menu_load_item_texture` | 649 | `100` | `remainder = number % 100;` | Keep the final two decimal digits after extracting the hundreds digit. |
| `menu_load_item_texture` | 650 | `6` | `name[6] = remainder / 10 + '0';` | Template byte6 is the tens digit; division by decimal ten and ASCII zero encode that position directly. |
| `menu_load_item_texture` | 650 | `10` | `name[6] = remainder / 10 + '0';` | Template byte6 is the tens digit; division by decimal ten and ASCII zero encode that position directly. |
| `menu_load_item_texture` | 650 | `'0'` | `name[6] = remainder / 10 + '0';` | Template byte6 is the tens digit; division by decimal ten and ASCII zero encode that position directly. |
| `menu_load_item_texture` | 651 | `7` | `name[7] = remainder % 10 + '0';` | Template byte7 is the units digit; remainder modulo decimal ten and ASCII zero encode that position directly. |
| `menu_load_item_texture` | 651 | `10` | `name[7] = remainder % 10 + '0';` | Template byte7 is the units digit; remainder modulo decimal ten and ASCII zero encode that position directly. |
| `menu_load_item_texture` | 651 | `'0'` | `name[7] = remainder % 10 + '0';` | Template byte7 is the units digit; remainder modulo decimal ten and ASCII zero encode that position directly. |
| `menu_load_item_texture` | 653 | `0` | `if (cd_file_load_into(destination, name) != 0) {` | Propagate any nonzero loader failure through the wrapper failure result. |
| `menu_load_item_texture` | 654 | `1` | `return 1;` | Wrapper failure result; the loader failed before texture upload. |
| `menu_load_item_texture` | 658 | `0` | `return 0;` | Wrapper success result, including the no-texture sentinel path. |

The [shared confirmation-state review](menu-confirm-state.md) names the request
flag and preserves its type through both drawing APIs. This ledger reflects
those current expressions; other retained selector domains still need review.
