# GAME menu confirmation protocol

## Function Match Plan

Starting at master `51dfd8d`, use the hash-verified GAME retail and the six-view
snapshots under `build/constant-names/menu-confirmation-protocol/`. Inspect the
seven callers, preview/render callees, neighbors and source histories before
editing. These are game UI policy, absent from the vendored inventory; PadRead
and GPU/SDK operations remain provider boundaries. All listed units use the
pinned `probe-gcc257-o2-g0` profile.

Type footer kind, preview mode and confirmation result through the declaration,
definition, local result and all seven callers. The outer panels retain their
separate row/item selections. Correct the misleading chosen-entry description.
Use the shared MenuGlyphString for the two labels: retail puts their origins
at sp+16 and sp+40 and saves registers starting at sp+64, consistent with the
existing two complete 24-byte positioned labels. Preserve label contents,
terminators, input-edge tests, final redraw and button-release loop.

Compare the type-only edit first, then the label representation independently.
Require other objects and all prior exact functions to retain their code and
ordered referents. Inspect the first actual divergence if the widget changes;
a better fuzzy score alone is not evidence. Run modern compiler checks, lint,
repository tests, whitespace checks and the full build before commit.

## Evidence snapshots

| GAME address / bytes | Function | Starting strict % | Constraint |
| --- | --- | --- | --- |
| `0x80028380 / 852` | `menu_list_interact` | 87.07981 | Full-width a1/a2 label/preview enums; -99 pending, 0 accepted, -1 cancelled; 104-byte frame, two redraw sites, ordered PadRead/render/sound calls. |
| `0x80021538 / 1476` | `item_menu_buy` | 97.20054 | Direct widget caller; named label/preview arguments, compares result against -1; outer selection remains a separate integer domain. |
| `0x80021afc / 1280` | `item_menu_sell` | 96.77187 | Direct widget caller; named label/preview arguments, compares result against -1; outer selection remains a separate integer domain. |
| `0x80022608 / 1908` | `menu_use_item_panel` | 92.8218 | Direct widget caller; named label/preview arguments, compares result against -1; outer selection remains a separate integer domain. |
| `0x8002317c / 1328` | `menu_magic_panel` | 95.89759 | Direct widget caller; named label/preview arguments, compares result against -1; outer selection remains a separate integer domain. |
| `0x800238d8 / 1476` | `menu_equip_select` | 97.89973 | Direct widget caller; named label/preview arguments, compares result against -1; outer selection remains a separate integer domain. |
| `0x80023e9c / 1136` | `menu_spell_select` | 98.06338 | Direct widget caller; named label/preview arguments, compares result against -1; outer selection remains a separate integer domain. |
| `0x800249a8 / 1212` | `menu_drop_item` | 98.85478 | Direct widget caller; named label/preview arguments, compares result against -1; outer selection remains a separate integer domain. |
| `0x800291ec / 268` | `menu_draw_two_option` | 100.0 | Unedited callee/adjacency control; signed glyph streams, detail price table, quad enqueue or model-preview behavior constrains the widget interface. |
| `0x80029de0 / 1328` | `menu_draw_string` | 99.98795 | Unedited callee/adjacency control; signed glyph streams, detail price table, quad enqueue or model-preview behavior constrains the widget interface. |
| `0x80027b7c / 732` | `menu_draw_item_detail` | 91.28416 | Unedited callee/adjacency control; signed glyph streams, detail price table, quad enqueue or model-preview behavior constrains the widget interface. |
| `0x80027e58 / 72` | `menu_add_marker_quad` | 100.0 | Unedited callee/adjacency control; signed glyph streams, detail price table, quad enqueue or model-preview behavior constrains the widget interface. |
| `0x80027ea0 / 68` | `menu_add_frame_quad` | 100.0 | Unedited callee/adjacency control; signed glyph streams, detail price table, quad enqueue or model-preview behavior constrains the widget interface. |
| `0x80027ee4 / 1180` | `menu_draw_dialog_frame` | 100.0 | Unedited callee/adjacency control; signed glyph streams, detail price table, quad enqueue or model-preview behavior constrains the widget interface. |
| `0x800279c4 / 440` | `menu_item_model_preview` | 98.181816 | Unedited callee/adjacency control; signed glyph streams, detail price table, quad enqueue or model-preview behavior constrains the widget interface. |

## Protocol and labels

Up/down toggles the footer position 0/1. The confirm edge computes its negative:
0 accepts and -1 declines. The cancel button also produces -1. The internal -99
sentinel continues the loop and never returns. Its original numeric choice is
unknown. Decoding the negated positional index to the result enum retains the
retail `negu` and documents the narrow arithmetic boundary.

Preview modes 0/1/2 select item model, item detail and the already loaded magic
icon respectively. Detail arguments are the one-based shop price column and a
nonzero sell-price flag. Both are unused zeros in model/icon callers. The kind
fallback stays equip for values outside 0..4, as in retail; current equip callers
pass 5. Kind 2 has no current direct caller but its glyphs establish yes/no.

The atlas was decoded statically from hash-verified `KF/COM/MIX.TIM`, 331488
bytes, SHA256 `9e1031c32ea9efc2d124ae97dd6f2a0291bc88153b74b8c1d57f99abefd55959`.
The fifth TIM starts at 231552; its sixteen RGB555 palette entries start at
231572 and 32768 image bytes at 232096. The 4bpp page is 256 by 256; glyphs are
14 by 12 texels, sixteen columns. For code c, use c & 0xfff and origin
`(c % 16 * 14, c / 16 * 12)` with integer division. Decode low nibble first;
bit 0x1000 overlays atlas cell 0x2e for dakuten. Visual inspection yields:

| Kind / use | Glyph codes before terminator | Retail text | Meaning |
| --- | --- | --- | --- |
| 0 | 72 42 | 使う | Use |
| 1 | 75 52 6a | 捨てる | Discard |
| 2 | 59 41 | はい | Yes |
| 3 | 74 42 | 買う | Buy |
| 4 | 73 6a | 売る | Sell |
| fallback (callers: 5) | 70 71 | 装備 | Equip |
| kind 2 decline | 41 41 43 | いいえ | No |
| other decline | 63 61 6a | やめる | Cancel |
| equipment empty choice | 59 104c 4c | はずす | Unequip |
| pickup accept | 53 6a | とる | Take |

All codes in this table are hexadecimal (source decimal 99 is glyph 0x63).
These authored glyph sequences remain inline asset data, not numeric state
constants. The two additional rows resolve the earlier pickup/equipment label
translation uncertainty without assigning names to item assets.

## Retained literal ledger

All **65 ordinary numeric occurrences** in `menu_list_interact.c` are covered
below. Claims and enum definitions are excluded. Glyph-array positions and
text codes count separately. The equipment ledger now has 147 occurrences
after this protocol audit and the [price follow-up](game-shop-price-domains.md).

| Lines | Tokens | Expression | Reason |
| --- | --- | --- | --- |
| 22 | `0` | `selected = 0;` | Initial footer position is the first option; a renderer index, distinct from its result enum. |
| 23, 100 | `0 × 2` | `highlight = 0;` | Clear the Boolean confirmation highlight before input and on each active frame. |
| 24 | `0` | `prev_pad = 0;` | No previous pressed buttons before the first edge comparison. |
| 26, 95 | `1 × 2, 0 × 2` | `while (PadRead(1) != 0) {` | Preserve the ignored retail PadRead argument 1 and wait for zero button bits; the linked SDK uses global PadIdentifier. |
| 29 | `0x60` | `opt0.x = 0x60;` | Authored first-label X coordinate 96 pixels; original placement rationale unknown. |
| 30 | `0xb9` | `opt0.y = 0xb9;` | Authored first-label Y coordinate 185 pixels. |
| 31 | `0x60` | `opt1.x = 0x60;` | Authored second-label X coordinate 96 pixels, aligned with the first label. |
| 32 | `0xcd` | `opt1.y = 0xcd;` | Authored second-label Y coordinate 205 pixels, twenty pixels below the first; original spacing choice unknown. |
| 34 | `0, 0x72` | `opt0.codes[0] = 0x72;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 35, 47 | `1 × 2, 0x42 × 2` | `opt0.codes[1] = 0x42;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 37 | `0, 0x75` | `opt0.codes[0] = 0x75;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 38 | `1, 0x52` | `opt0.codes[1] = 0x52;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 39 | `2, 0x6a` | `opt0.codes[2] = 0x6a;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 40 | `3` | `opt0.codes[3] = MENU_TEXT_END;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 43 | `0, 0x59` | `opt0.codes[0] = 0x59;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 44 | `1, 0x41` | `opt0.codes[1] = 0x41;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 46 | `0, 0x74` | `opt0.codes[0] = 0x74;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 49 | `0, 0x73` | `opt0.codes[0] = 0x73;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 50 | `1, 0x6a` | `opt0.codes[1] = 0x6a;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 52 | `0, 0x70` | `opt0.codes[0] = 0x70;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 53 | `1, 0x71` | `opt0.codes[1] = 0x71;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 55 | `2` | `opt0.codes[2] = MENU_TEXT_END;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 58 | `0, 0x41` | `opt1.codes[0] = 0x41;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 59 | `1, 0x41` | `opt1.codes[1] = 0x41;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 60 | `2, 0x43` | `opt1.codes[2] = 0x43;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 62 | `0, 99` | `opt1.codes[0] = 99;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 63 | `1, 0x61` | `opt1.codes[1] = 0x61;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 64 | `2, 0x6a` | `opt1.codes[2] = 0x6a;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 66 | `3` | `opt1.codes[3] = MENU_TEXT_END;` | Consecutive glyph/terminator index in an authored positioned label; any numeric right-hand side is an atlas code decoded in the label table above. Kept inline as text data. |
| 102 | `1` | `pad = PadRead(1);` | Preserve the ignored retail PadRead call-site argument; it does not select a controller port. |
| 103 | `0 × 2` | `if (((pad & PADLup) == 0 \|\| (prev_pad & PADLup) != 0) &&` | Zero tests the absence of a named current/previous button bit; together they detect a rising edge. |
| 104 | `0 × 2` | `((pad & PADLdown) == 0 \|\| (prev_pad & PADLdown) != 0)) {` | Zero tests the absence of a named current/previous button bit; together they detect a rising edge. |
| 105 | `0 × 2` | `if ((pad & PADRright) == 0 \|\| (prev_pad & PADRright) != 0) {` | Zero tests the absence of a named current/previous button bit; together they detect a rising edge. |
| 106 | `0 × 2` | `if ((pad & PADRdown) != 0 && (prev_pad & PADRdown) == 0) {` | Zero tests the absence of a named current/previous button bit; together they detect a rising edge. |
| 112 | `1` | `highlight = 1;` | Show confirmation highlight on the final two rendered frames. |
| 117 | `0` | `selected = (selected == 0);` | Toggle the two positional renderer indices 0 and 1; negation later maps them to accepted/cancelled. |
| 120 | `1` | `} while (1);` | Unconditional modal loop; returns only after completing the final redraw and button-release wait. |

## Verification and final verdicts

The type-only edit preserves every non-debug section of all 112 objects and
all 484 strict scores. Replacing the short local label view changes exactly
35 instruction immediates: frame allocation/release, saved-register slots,
stack arguments, and second-label addresses/stores. Opcodes, registers,
branches, instruction order, relocations, initialized data and symbols retain
their prior bytes. The procedure descriptor changes only frame size 88 to 104.
Retail directly supports the new 104-byte frame and label origins 16/40.

The widget improves 87.07981 → 87.24413%. Its first remaining divergence is
at +0x38: source retains the item argument in s3, retail in s2; +0x44 initializes
previous buttons in the opposite register. The compiled body remains 824 bytes
against retail's 852. This is partial reconstruction with unattributed residue;
no compiler mechanism or new exact result is claimed. All other 483 scores and
all banked results are preserved; no banking rows change. The full report differs
only in this widget's fuzzy values and the aggregate fuzzy percentage.

Four unedited exact controls reproduce 397 complete retail words, including
ordered calls/referents. The modern compiler remains 64 passing / 48 failing
source-image variants, with the same multiset of 320 diagnostics. Ruff and
whitespace checks pass. The 656-test run finishes in 80.589 seconds with one
existing failure in the user's untracked save/load-hub test; it is preserved.
Full build retains existing source-data 7/60, SDK/config-data 4/4 and target
relink PSX 1/1, GAME 75/77, OPEN 34/38 findings, with two GAME and four OPEN
section-base conflicts. No tooling/flake implementation changed.

| GAME address | Function | Final strict % | Verdict |
| --- | --- | --- | --- |
| `0x80028380` | `menu_list_interact` | 87.24413 | Typed protocol and corrected label storage; partial |
| `0x80021538` | `item_menu_buy` | 97.20054 | Code and score unchanged; partial |
| `0x80021afc` | `item_menu_sell` | 96.77187 | Code and score unchanged; partial |
| `0x80022608` | `menu_use_item_panel` | 92.8218 | Code and score unchanged; partial |
| `0x8002317c` | `menu_magic_panel` | 95.89759 | Code and score unchanged; partial |
| `0x800238d8` | `menu_equip_select` | 97.89973 | Code and score unchanged; partial |
| `0x80023e9c` | `menu_spell_select` | 98.06338 | Code and score unchanged; partial |
| `0x800249a8` | `menu_drop_item` | 98.85478 | Code and score unchanged; partial |
| `0x800291ec` | `menu_draw_two_option` | 100.0 | Exact control; 67 raw retail words |
| `0x80029de0` | `menu_draw_string` | 99.98795 | Code and score unchanged; partial |
| `0x80027b7c` | `menu_draw_item_detail` | 91.28416 | Code and score unchanged; partial |
| `0x80027e58` | `menu_add_marker_quad` | 100.0 | Exact control; 18 raw retail words |
| `0x80027ea0` | `menu_add_frame_quad` | 100.0 | Exact control; 17 raw retail words |
| `0x80027ee4` | `menu_draw_dialog_frame` | 100.0 | Exact control; 295 raw retail words |
| `0x800279c4` | `menu_item_model_preview` | 98.181816 | Code and score unchanged; partial |
