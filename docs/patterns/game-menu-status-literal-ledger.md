# Status-text literal ledger

Complete ledger for `src/game/menu_draw_stats_header.c` and
`src/game/menu_draw_status_details.c` after the [class-title review](game-menu-class-titles.md).
All 413 ordinary numeric occurrences are covered; retail address/size
claims and named constant definitions are excluded. Multiple tokens on a
line count separately, including glyph positions and glyph values.

## menu_draw_stats_header

This section records the class-title review snapshot. The later
[index-lifetime recovery](game-stats-header-index.md) replaces both
`class_glyph_offset` and `i` with `glyph_index`, removing one declaration;
all listed values and expressions otherwise remain, one source line earlier.

130 occurrences.

| Lines | Tokens | Expression | Reason |
| --- | --- | --- | --- |
| 19 | `0xb5` | `gs.x = 0xb5;` | Explicit screen X anchor 181 pixels for the following text run; preserve the authored layout, whose original placement rationale is unknown. |
| 20, 72 | `0x24 × 2` | `gs.y = 0x24;` | Explicit screen Y anchor 36 pixels for the following text run; preserve the authored layout, whose original placement rationale is unknown. |
| 21 | `0, 0x82` | `gs.codes[0] = 0x82;` | At zero-based glyph position 0, encode authored text-atlas cell 0x82. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 22 | `1, 0x83` | `gs.codes[1] = 0x83;` | At zero-based glyph position 1, encode authored text-atlas cell 0x83. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 23 | `2, 0x84` | `gs.codes[2] = 0x84;` | At zero-based glyph position 2, encode authored text-atlas cell 0x84. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 24, 30, 37 | `3 × 3` | `gs.codes[3] = MENU_TEXT_END;` | Place the named terminator at zero-based glyph position 3; the index is the actual preceding run length. |
| 27 | `0, 0x2b` | `gs.codes[0] = 0x2b;` | At zero-based glyph position 0, encode authored text-atlas cell 0x2b. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 28 | `1, 0x1c` | `gs.codes[1] = MENU_TEXT_DAKUTEN \| 0x1c;` | At zero-based glyph position 1, encode authored text-atlas cell 0x1c with the named dakuten overlay. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 29, 65 | `2 × 2, 0x2a × 2` | `gs.codes[2] = 0x2a;` | At zero-based glyph position 2, encode authored text-atlas cell 0x2a. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 31, 38, 44, 50, 54, 60, 68, 76, 81, 106, 111, 123, 141, 165 | `0x17 × 14` | `gs.y += 0x17;` | Main-menu statistics use eight rows at a 23-pixel pitch, Y=36..197. Keep the explicit authored spacing; it is not a universal font-cell height. |
| 34 | `0, 7` | `gs.codes[0] = 7;` | At zero-based glyph position 0, encode authored text-atlas cell 0x7. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 35 | `1, 0x28` | `gs.codes[1] = 0x28;` | At zero-based glyph position 1, encode authored text-atlas cell 0x28. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 36 | `2, 0xc` | `gs.codes[2] = 0xc;` | At zero-based glyph position 2, encode authored text-atlas cell 0xc. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 41 | `0, 0xcc` | `gs.codes[0] = 0xcc;` | At zero-based glyph position 0, encode authored text-atlas cell 0xcc. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 42 | `1, 0xcd` | `gs.codes[1] = 0xcd;` | At zero-based glyph position 1, encode authored text-atlas cell 0xcd. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 43, 49, 59 | `2 × 3` | `gs.codes[2] = MENU_TEXT_END;` | Place the named terminator at zero-based glyph position 2; the index is the actual preceding run length. |
| 47 | `0, 0xf0` | `gs.codes[0] = 0xf0;` | At zero-based glyph position 0, encode authored text-atlas cell 0xf0. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 48 | `1, 0xf2` | `gs.codes[1] = 0xf2;` | At zero-based glyph position 1, encode authored text-atlas cell 0xf2. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 53 | `0, 0xf1` | `gs.codes[0] = 0xf1;` | At zero-based glyph position 0, encode authored text-atlas cell 0xf1. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 57 | `0, 0x85` | `gs.codes[0] = 0x85;` | At zero-based glyph position 0, encode authored text-atlas cell 0x85. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 58 | `1, 0x86` | `gs.codes[1] = 0x86;` | At zero-based glyph position 1, encode authored text-atlas cell 0x86. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 63 | `0, 0x9` | `gs.codes[0] = MENU_TEXT_DAKUTEN \| 0x9;` | At zero-based glyph position 0, encode authored text-atlas cell 0x9 with the named dakuten overlay. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 64 | `1, 0x2d` | `gs.codes[1] = 0x2d;` | At zero-based glyph position 1, encode authored text-atlas cell 0x2d. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 66 | `3, 0x13` | `gs.codes[3] = MENU_TEXT_DAKUTEN \| 0x13;` | At zero-based glyph position 3, encode authored text-atlas cell 0x13 with the named dakuten overlay. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 67, 102 | `4 × 2` | `gs.codes[4] = MENU_TEXT_END;` | Place the named terminator at zero-based glyph position 4; the index is the actual preceding run length. |
| 71, 105, 164 | `0xfb × 3` | `gs.x = 0xfb;` | Explicit screen X anchor 251 pixels for the following text run; preserve the authored layout, whose original placement rationale is unknown. |
| 73 | `6, 0` | `menu_format_number(player_state.experience, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 77 | `6, 0` | `menu_format_number(player_state.progress_state.level, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 80 | `0xed` | `gs.x = 0xed;` | Explicit screen X anchor 237 pixels for the following text run; preserve the authored layout, whose original placement rationale is unknown. |
| 83 | `0` | `class_glyph_offset = 0;` | Zero-based low/middle/high magic-tier column in the three-column class-title grid; ordinary index arithmetic. |
| 85 | `2` | `class_glyph_offset = 2;` | Zero-based low/middle/high magic-tier column in the three-column class-title grid; ordinary index arithmetic. |
| 87 | `1` | `class_glyph_offset = 1;` | Zero-based low/middle/high magic-tier column in the three-column class-title grid; ordinary index arithmetic. |
| 90 | `1` | `if (player_state.base_physical_power > (MENU_CLASS_MIDDLE_STAT_MIN - 1)) {` | Subtract one to express the inclusive minimum with the original strict greater-than comparison. |
| 94 | `2` | `class_glyph_offset += 2 * MENU_CLASS_MAGIC_TIER_COUNT;` | The high physical tier is row index two; multiply it by the named three-column stride. |
| 98 | `0` | `gs.codes[0] = class_glyph_offset + MENU_CLASS_FIRST_GLYPH;` | Write zero-based glyph position 0 of the four-cell class title; any trailing 1/2/3 is the corresponding cell offset from its named first glyph. |
| 99 | `1 × 2` | `gs.codes[1] = class_glyph_offset + MENU_CLASS_FIRST_GLYPH + 1;` | Write zero-based glyph position 1 of the four-cell class title; any trailing 1/2/3 is the corresponding cell offset from its named first glyph. |
| 100 | `2 × 2` | `gs.codes[2] = class_glyph_offset + MENU_CLASS_FIRST_GLYPH + 2;` | Write zero-based glyph position 2 of the four-cell class title; any trailing 1/2/3 is the corresponding cell offset from its named first glyph. |
| 101 | `3 × 2` | `gs.codes[3] = class_glyph_offset + MENU_CLASS_FIRST_GLYPH + 3;` | Write zero-based glyph position 3 of the four-cell class title; any trailing 1/2/3 is the corresponding cell offset from its named first glyph. |
| 107 | `6, 0` | `menu_format_number(player_state.progress_state.current_floor, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 110, 122 | `0xe6 × 2` | `gs.x = 0xe6;` | Explicit screen X anchor 230 pixels for the following text run; preserve the authored layout, whose original placement rationale is unknown. |
| 112 | `4, 0` | `menu_format_number(player_state.vitals.current_hp, 4, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 114, 126 | `0 × 2` | `gs.codes[0] = MENU_NUMBER_SLASH;` | Place the named slash at zero-based glyph position 0; this is a one-glyph separator before the maximum HP/MP value. |
| 115, 127 | `1 × 2` | `gs.codes[1] = MENU_TEXT_END;` | Place the named terminator at zero-based glyph position 1; the index is the actual preceding run length. |
| 116, 128 | `0x1c × 2` | `gs.x += 0x1c;` | Advance 28 pixels across the four-digit current HP/MP field at seven pixels per number glyph. |
| 118, 130 | `7 × 2` | `gs.x += 7;` | Advance one seven-pixel number glyph past the slash to the maximum HP/MP field. |
| 119 | `4, 0` | `menu_format_number(player_state.vitals.maximum_hp, 4, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 124 | `4, 0` | `menu_format_number(player_state.vitals.current_mp, 4, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 131 | `4, 0` | `menu_format_number(player_state.vitals.maximum_mp, 4, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 134 | `0xdf` | `gs.x = 0xdf;` | Explicit screen X anchor 223 pixels for the following text run; preserve the authored layout, whose original placement rationale is unknown. |
| 135 | `0` | `gs.codes[0] = MENU_TEXT_BLANK;` | Initialize zero-based glyph position 0 to the named blank; preserve the five-cell status workspace and its right alignment. |
| 136 | `1` | `gs.codes[1] = MENU_TEXT_BLANK;` | Initialize zero-based glyph position 1 to the named blank; preserve the five-cell status workspace and its right alignment. |
| 137 | `2` | `gs.codes[2] = MENU_TEXT_BLANK;` | Initialize zero-based glyph position 2 to the named blank; preserve the five-cell status workspace and its right alignment. |
| 138 | `3` | `gs.codes[3] = MENU_TEXT_BLANK;` | Initialize zero-based glyph position 3 to the named blank; preserve the five-cell status workspace and its right alignment. |
| 139 | `4` | `gs.codes[4] = MENU_TEXT_BLANK;` | Initialize zero-based glyph position 4 to the named blank; preserve the five-cell status workspace and its right alignment. |
| 140 | `5` | `gs.codes[5] = MENU_TEXT_END;` | Place the named terminator at zero-based glyph position 5; the index is the actual preceding run length. |
| 142 | `0` | `if (player_state.status_effect_flags == 0) {` | Zero means no active status bits, or no bits selected by the named status mask; ordinary bitset emptiness test. |
| 143 | `3, 0xc5` | `gs.codes[3] = 0xc5;` | At zero-based glyph position 3, encode the first cell of 正常 (normal). These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 144 | `4, 0xc6` | `gs.codes[4] = 0xc6;` | At zero-based glyph position 4, encode the second cell of 正常 (normal). These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 146 | `4` | `i = 4;` | Start filling status icons at the rightmost position of the five-cell workspace. |
| 147 | `0` | `if ((player_state.status_effect_flags & KF_PLAYER_STATUS_SLOWED) != 0) {` | Zero means no active status bits, or no bits selected by the named status mask; ordinary bitset emptiness test. |
| 148 | `4, 0xc9` | `gs.codes[4] = 0xc9;` | At zero-based glyph position 4, encode the slowed-status glyph. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 149 | `3` | `i = 3;` | The slowed icon occupies position four; start other icons one position to its left. |
| 151 | `0` | `if ((player_state.status_effect_flags & KF_PLAYER_STATUS_POISON) != 0) {` | Zero means no active status bits, or no bits selected by the named status mask; ordinary bitset emptiness test. |
| 152 | `0x88` | `gs.codes[i] = 0x88;` | At the next free position while filling right to left, encode 毒 (poison). These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 155 | `0` | `if ((player_state.status_effect_flags & KF_PLAYER_STATUS_DARKNESS) != 0) {` | Zero means no active status bits, or no bits selected by the named status mask; ordinary bitset emptiness test. |
| 156 | `199` | `gs.codes[i] = 199;` | At the next free position while filling right to left, encode the darkness-status glyph. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 159 | `0` | `if ((player_state.status_effect_flags & KF_PLAYER_STATUS_CURSE) != 0) {` | Zero means no active status bits, or no bits selected by the named status mask; ordinary bitset emptiness test. |
| 160 | `200` | `gs.codes[i] = 200;` | At the next free position while filling right to left, encode the curse-status glyph. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 166 | `6, 0` | `menu_format_number(player_state.gold, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |

## menu_draw_status_details

This ledger records the class-title review snapshot. The later
[rating-arithmetic correction](game-status-rating-arithmetic.md) replaces
the outer attack shift with signed division by eight and removes the
unsigned defense cast; its arithmetic discussion supersedes those rows below.
The subsequent [shared-index correction](game-stats-header-index.md#detailed-status-follow-up)
replaces `class_glyph_offset` and `i` with `glyph_index`, removing one
declaration without changing the listed values or arithmetic expressions.

283 occurrences.

| Lines | Tokens | Expression | Reason |
| --- | --- | --- | --- |
| 20 | `0x15` | `gs.x = 0x15;` | Explicit screen X anchor 21 pixels for the following text run; preserve the authored layout, whose original placement rationale is unknown. |
| 21, 93 | `0x23 × 2` | `gs.y = 0x23;` | Explicit screen Y anchor 35 pixels for the following text run; preserve the authored layout, whose original placement rationale is unknown. |
| 22 | `0, 0x82` | `gs.codes[0] = 0x82;` | At zero-based glyph position 0, encode authored text-atlas cell 0x82. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 23 | `1, 0x83` | `gs.codes[1] = 0x83;` | At zero-based glyph position 1, encode authored text-atlas cell 0x83. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 24 | `2, 0x84` | `gs.codes[2] = 0x84;` | At zero-based glyph position 2, encode authored text-atlas cell 0x84. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 25, 31, 38, 210, 216, 245, 252 | `3 × 7` | `gs.codes[3] = MENU_TEXT_END;` | Place the named terminator at zero-based glyph position 3; the index is the actual preceding run length. |
| 28 | `0, 0x2b` | `gs.codes[0] = 0x2b;` | At zero-based glyph position 0, encode authored text-atlas cell 0x2b. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 29 | `1, 0x1c` | `gs.codes[1] = MENU_TEXT_DAKUTEN \| 0x1c;` | At zero-based glyph position 1, encode authored text-atlas cell 0x1c with the named dakuten overlay. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 30, 60 | `2 × 2, 0x2a × 2` | `gs.codes[2] = 0x2a;` | At zero-based glyph position 2, encode authored text-atlas cell 0x2a. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 32, 39, 45, 49, 55, 63, 69, 74, 83, 89, 97, 102, 127, 139, 157, 181, 184, 187, 190, 197, 246 | `0x10 × 21` | `gs.y += 0x10;` | Status-detail left rows use a 16-pixel pitch, Y=35..195; the same increment separates the attack/defense headings on the right. Keep the explicit authored spacing; it is not a universal font-cell height. |
| 35 | `0, 7` | `gs.codes[0] = 7;` | At zero-based glyph position 0, encode authored text-atlas cell 0x7. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 36 | `1, 0x28` | `gs.codes[1] = 0x28;` | At zero-based glyph position 1, encode authored text-atlas cell 0x28. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 37 | `2, 0xc` | `gs.codes[2] = 0xc;` | At zero-based glyph position 2, encode authored text-atlas cell 0xc. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 42 | `0, 0xf0` | `gs.codes[0] = 0xf0;` | At zero-based glyph position 0, encode authored text-atlas cell 0xf0. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 43 | `1, 0xf2` | `gs.codes[1] = 0xf2;` | At zero-based glyph position 1, encode authored text-atlas cell 0xf2. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 44, 54, 68, 73, 267 | `2 × 5` | `gs.codes[2] = MENU_TEXT_END;` | Place the named terminator at zero-based glyph position 2; the index is the actual preceding run length. |
| 48 | `0, 0xf1` | `gs.codes[0] = 0xf1;` | At zero-based glyph position 0, encode authored text-atlas cell 0xf1. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 52 | `0, 0x85` | `gs.codes[0] = 0x85;` | At zero-based glyph position 0, encode authored text-atlas cell 0x85. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 53 | `1, 0x86` | `gs.codes[1] = 0x86;` | At zero-based glyph position 1, encode authored text-atlas cell 0x86. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 58 | `0, 0x9` | `gs.codes[0] = MENU_TEXT_DAKUTEN \| 0x9;` | At zero-based glyph position 0, encode authored text-atlas cell 0x9 with the named dakuten overlay. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 59 | `1, 0x2d` | `gs.codes[1] = 0x2d;` | At zero-based glyph position 1, encode authored text-atlas cell 0x2d. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 61 | `3, 0x13` | `gs.codes[3] = MENU_TEXT_DAKUTEN \| 0x13;` | At zero-based glyph position 3, encode authored text-atlas cell 0x13 with the named dakuten overlay. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 62, 123 | `4 × 2` | `gs.codes[4] = MENU_TEXT_END;` | Place the named terminator at zero-based glyph position 4; the index is the actual preceding run length. |
| 66 | `0, 0x8c` | `gs.codes[0] = 0x8c;` | At zero-based glyph position 0, encode authored text-atlas cell 0x8c. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 67 | `1, 0x8b` | `gs.codes[1] = 0x8b;` | At zero-based glyph position 1, encode authored text-atlas cell 0x8b. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 72 | `0, 0x78` | `gs.codes[0] = 0x78;` | At zero-based glyph position 0, encode authored text-atlas cell 0x78. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 77 | `0, 0xce` | `gs.codes[0] = 0xce;` | At zero-based glyph position 0, encode authored text-atlas cell 0xce. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 78 | `1, 0xcf` | `gs.codes[1] = 0xcf;` | At zero-based glyph position 1, encode authored text-atlas cell 0xcf. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 79 | `2, 0x89` | `gs.codes[2] = 0x89;` | At zero-based glyph position 2, encode authored text-atlas cell 0x89. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 80 | `3, 0x8a` | `gs.codes[3] = 0x8a;` | At zero-based glyph position 3, encode authored text-atlas cell 0x8a. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 81 | `4, 0x8b` | `gs.codes[4] = 0x8b;` | At zero-based glyph position 4, encode authored text-atlas cell 0x8b. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 82, 88, 156, 234, 275 | `5 × 5` | `gs.codes[5] = MENU_TEXT_END;` | Place the named terminator at zero-based glyph position 5; the index is the actual preceding run length. |
| 86 | `2, 0x7a` | `gs.codes[2] = 0x7a;` | At zero-based glyph position 2, encode authored text-atlas cell 0x7a. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 87 | `3, 0xd0` | `gs.codes[3] = 0xd0;` | At zero-based glyph position 3, encode authored text-atlas cell 0xd0. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 92, 180 | `0x5b × 2` | `gs.x = 0x5b;` | Explicit screen X anchor 91 pixels for the following text run; preserve the authored layout, whose original placement rationale is unknown. |
| 94 | `6, 0` | `menu_format_number(player_state.experience, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 98 | `6, 0` | `menu_format_number(player_state.progress_state.level, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 101 | `0x4d` | `gs.x = 0x4d;` | Explicit screen X anchor 77 pixels for the following text run; preserve the authored layout, whose original placement rationale is unknown. |
| 104 | `0` | `class_glyph_offset = 0;` | Zero-based low/middle/high magic-tier column in the three-column class-title grid; ordinary index arithmetic. |
| 106 | `2` | `class_glyph_offset = 2;` | Zero-based low/middle/high magic-tier column in the three-column class-title grid; ordinary index arithmetic. |
| 108 | `1` | `class_glyph_offset = 1;` | Zero-based low/middle/high magic-tier column in the three-column class-title grid; ordinary index arithmetic. |
| 111 | `1` | `if (player_state.base_physical_power > (MENU_CLASS_MIDDLE_STAT_MIN - 1)) {` | Subtract one to express the inclusive minimum with the original strict greater-than comparison. |
| 115 | `2` | `class_glyph_offset += 2 * MENU_CLASS_MAGIC_TIER_COUNT;` | The high physical tier is row index two; multiply it by the named three-column stride. |
| 119 | `0` | `gs.codes[0] = class_glyph_offset + MENU_CLASS_FIRST_GLYPH;` | Write zero-based glyph position 0 of the four-cell class title; any trailing 1/2/3 is the corresponding cell offset from its named first glyph. |
| 120 | `1 × 2` | `gs.codes[1] = class_glyph_offset + MENU_CLASS_FIRST_GLYPH + 1;` | Write zero-based glyph position 1 of the four-cell class title; any trailing 1/2/3 is the corresponding cell offset from its named first glyph. |
| 121 | `2 × 2` | `gs.codes[2] = class_glyph_offset + MENU_CLASS_FIRST_GLYPH + 2;` | Write zero-based glyph position 2 of the four-cell class title; any trailing 1/2/3 is the corresponding cell offset from its named first glyph. |
| 122 | `3 × 2` | `gs.codes[3] = class_glyph_offset + MENU_CLASS_FIRST_GLYPH + 3;` | Write zero-based glyph position 3 of the four-cell class title; any trailing 1/2/3 is the corresponding cell offset from its named first glyph. |
| 126, 138 | `0x46 × 2` | `gs.x = 0x46;` | Explicit screen X anchor 70 pixels for the following text run; preserve the authored layout, whose original placement rationale is unknown. |
| 128 | `4, 0` | `menu_format_number(player_state.vitals.current_hp, 4, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 130, 142 | `0 × 2` | `gs.codes[0] = MENU_NUMBER_SLASH;` | Place the named slash at zero-based glyph position 0; this is a one-glyph separator before the maximum HP/MP value. |
| 131, 143 | `1 × 2` | `gs.codes[1] = MENU_TEXT_END;` | Place the named terminator at zero-based glyph position 1; the index is the actual preceding run length. |
| 132, 144 | `0x1c × 2` | `gs.x += 0x1c;` | Advance 28 pixels across the four-digit current HP/MP field at seven pixels per number glyph. |
| 134, 146 | `7 × 2` | `gs.x += 7;` | Advance one seven-pixel number glyph past the slash to the maximum HP/MP field. |
| 135 | `4, 0` | `menu_format_number(player_state.vitals.maximum_hp, 4, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 140 | `4, 0` | `menu_format_number(player_state.vitals.current_mp, 4, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 147 | `4, 0` | `menu_format_number(player_state.vitals.maximum_mp, 4, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 150 | `0x3f` | `gs.x = 0x3f;` | Explicit screen X anchor 63 pixels for the following text run; preserve the authored layout, whose original placement rationale is unknown. |
| 151, 213, 249 | `0 × 3` | `gs.codes[0] = MENU_TEXT_BLANK;` | Initialize zero-based glyph position 0 to the named blank; preserve the five-cell status workspace and its right alignment. |
| 152 | `1` | `gs.codes[1] = MENU_TEXT_BLANK;` | Initialize zero-based glyph position 1 to the named blank; preserve the five-cell status workspace and its right alignment. |
| 153 | `2` | `gs.codes[2] = MENU_TEXT_BLANK;` | Initialize zero-based glyph position 2 to the named blank; preserve the five-cell status workspace and its right alignment. |
| 154 | `3` | `gs.codes[3] = MENU_TEXT_BLANK;` | Initialize zero-based glyph position 3 to the named blank; preserve the five-cell status workspace and its right alignment. |
| 155 | `4` | `gs.codes[4] = MENU_TEXT_BLANK;` | Initialize zero-based glyph position 4 to the named blank; preserve the five-cell status workspace and its right alignment. |
| 158 | `0` | `if (player_state.status_effect_flags == 0) {` | Zero means no active status bits, or no bits selected by the named status mask; ordinary bitset emptiness test. |
| 159 | `3, 0xc5` | `gs.codes[3] = 0xc5;` | At zero-based glyph position 3, encode the first cell of 正常 (normal). These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 160 | `4, 0xc6` | `gs.codes[4] = 0xc6;` | At zero-based glyph position 4, encode the second cell of 正常 (normal). These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 162 | `4` | `i = 4;` | Start filling status icons at the rightmost position of the five-cell workspace. |
| 163 | `0` | `if ((player_state.status_effect_flags & KF_PLAYER_STATUS_SLOWED) != 0) {` | Zero means no active status bits, or no bits selected by the named status mask; ordinary bitset emptiness test. |
| 164 | `4, 0xc9` | `gs.codes[4] = 0xc9;` | At zero-based glyph position 4, encode the slowed-status glyph. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 165 | `3` | `i = 3;` | The slowed icon occupies position four; start other icons one position to its left. |
| 167 | `0` | `if ((player_state.status_effect_flags & KF_PLAYER_STATUS_POISON) != 0) {` | Zero means no active status bits, or no bits selected by the named status mask; ordinary bitset emptiness test. |
| 168 | `0x88` | `gs.codes[i] = 0x88;` | At the next free position while filling right to left, encode 毒 (poison). These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 171 | `0` | `if ((player_state.status_effect_flags & KF_PLAYER_STATUS_DARKNESS) != 0) {` | Zero means no active status bits, or no bits selected by the named status mask; ordinary bitset emptiness test. |
| 172 | `199` | `gs.codes[i] = 199;` | At the next free position while filling right to left, encode the darkness-status glyph. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 175 | `0` | `if ((player_state.status_effect_flags & KF_PLAYER_STATUS_CURSE) != 0) {` | Zero means no active status bits, or no bits selected by the named status mask; ordinary bitset emptiness test. |
| 176 | `200` | `gs.codes[i] = 200;` | At the next free position while filling right to left, encode the curse-status glyph. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 182 | `6, 0` | `menu_format_number(player_state.gold, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 185 | `6, 0` | `menu_format_number(player_state.physical_power, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 188 | `6, 0` | `menu_format_number(player_state.magic, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 193 | `3, 1` | `player_state.piercing_attack) * 3 >> 1) +` | The displayed attack subtotal triples the three physical components and shifts right once, preserving integer truncation of the authored 3/2 weighting. |
| 194 | `2, 10, 3` | `((u32)player_state.holy_attack + player_state.fire_attack) * 2) * 10) >> 3,` | Double the holy/fire subtotal, combine it with the physical subtotal, multiply by ten, then shift right three bits (divide by eight). This is the authored display-score formula, not direct HP damage; retain its rounding order. |
| 195, 202 | `6 × 2, 0 × 2` | `6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 200 | `5` | `player_state.piercing_defense + player_state.poison_resistance / 5 +` | Divide poison resistance by five before adding it to the defense subtotal; preserve this authored display weighting and its integer truncation. |
| 201 | `10, 7` | `player_state.magic_defense + player_state.fire_defense) * 10) / 7,` | Scale the combined defense subtotal by ten and divide by seven. Preserve the authored display-score formula; the divisor is not inferred to be a count of equal components. |
| 205 | `0xb5` | `gs.x = 0xb5;` | Explicit screen X anchor 181 pixels for the following text run; preserve the authored layout, whose original placement rationale is unknown. |
| 206 | `0x1e` | `gs.y = 0x1e;` | Explicit screen Y anchor 30 pixels for the following text run; preserve the authored layout, whose original placement rationale is unknown. |
| 207 | `0, 0x89` | `gs.codes[0] = 0x89;` | At zero-based glyph position 0, encode authored text-atlas cell 0x89. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 208 | `1, 0x8a` | `gs.codes[1] = 0x8a;` | At zero-based glyph position 1, encode authored text-atlas cell 0x8a. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 209, 244 | `2 × 2, 0x8b × 2` | `gs.codes[2] = 0x8b;` | At zero-based glyph position 2, encode authored text-atlas cell 0x8b. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 214, 250 | `1 × 2, 0xd1 × 2` | `gs.codes[1] = 0xd1;` | At zero-based glyph position 1, encode authored text-atlas cell 0xd1. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 215, 251 | `2 × 2, 0x6a × 2` | `gs.codes[2] = 0x6a;` | At zero-based glyph position 2, encode authored text-atlas cell 0x6a. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 217, 222, 227, 235, 239, 253, 258, 263, 268, 276, 280, 287, 290, 293, 296, 302, 305, 308, 311, 314 | `0xe × 20` | `gs.y += 0xe;` | Individual attack/defense labels and numeric components use a 14-pixel row pitch. Keep the explicit authored spacing; it is not a universal font-cell height. |
| 220, 256 | `1 × 2, 0xd2 × 2` | `gs.codes[1] = 0xd2;` | At zero-based glyph position 1, encode authored text-atlas cell 0xd2. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 221, 257 | `2 × 2, 0x51 × 2` | `gs.codes[2] = 0x51;` | At zero-based glyph position 2, encode authored text-atlas cell 0x51. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 225, 261 | `1 × 2, 0xd3 × 2` | `gs.codes[1] = 0xd3;` | At zero-based glyph position 1, encode authored text-atlas cell 0xd3. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 226, 262 | `2 × 2, 0x4c × 2` | `gs.codes[2] = 0x4c;` | At zero-based glyph position 2, encode authored text-atlas cell 0x4c. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 230 | `1, 0xbf` | `gs.codes[1] = 0xbf;` | At zero-based glyph position 1, encode authored text-atlas cell 0xbf. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 231, 272 | `2 × 2, 0x58 × 2` | `gs.codes[2] = 0x58;` | At zero-based glyph position 2, encode authored text-atlas cell 0x58. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 232, 273 | `3 × 2, 0x78 × 2` | `gs.codes[3] = 0x78;` | At zero-based glyph position 3, encode authored text-atlas cell 0x78. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 233, 274 | `4 × 2, 0x79 × 2` | `gs.codes[4] = 0x79;` | At zero-based glyph position 4, encode authored text-atlas cell 0x79. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 238, 279 | `1 × 2, 0xd4 × 2` | `gs.codes[1] = 0xd4;` | At zero-based glyph position 1, encode authored text-atlas cell 0xd4. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 242 | `0, 0x7a` | `gs.codes[0] = 0x7a;` | At zero-based glyph position 0, encode authored text-atlas cell 0x7a. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 243 | `1, 0xd0` | `gs.codes[1] = 0xd0;` | At zero-based glyph position 1, encode authored text-atlas cell 0xd0. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 266 | `1, 0x88` | `gs.codes[1] = 0x88;` | At zero-based glyph position 1, encode 毒 (poison). These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 271 | `1, 0x78` | `gs.codes[1] = 0x78;` | At zero-based glyph position 1, encode authored text-atlas cell 0x78. These are glyph-stream data, not Unicode or gameplay IDs; retain the literal cell rather than inventing a per-character constant. |
| 283 | `0xfb` | `gs.x = 0xfb;` | Explicit screen X anchor 251 pixels for the following text run; preserve the authored layout, whose original placement rationale is unknown. |
| 284 | `0x2c` | `gs.y = 0x2c;` | Explicit screen Y anchor 44 pixels for the following text run; preserve the authored layout, whose original placement rationale is unknown. |
| 285 | `6, 0` | `menu_format_number(player_state.cutting_attack, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 288 | `6, 0` | `menu_format_number(player_state.striking_attack, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 291 | `6, 0` | `menu_format_number(player_state.piercing_attack, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 294 | `6, 0` | `menu_format_number(player_state.holy_attack, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 297 | `6, 0` | `menu_format_number(player_state.fire_attack, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 299 | `0x1e` | `gs.y += 0x1e;` | Move from the final attack value at Y=100 to the first defense value at Y=130, crossing the intervening heading. Keep the explicit authored spacing; it is not a universal font-cell height. |
| 300 | `6, 0` | `menu_format_number(player_state.cutting_defense, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 303 | `6, 0` | `menu_format_number(player_state.striking_defense, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 306 | `6, 0` | `menu_format_number(player_state.piercing_defense, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 309 | `6, 0` | `menu_format_number(player_state.poison_resistance, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 312 | `6, 0` | `menu_format_number(player_state.magic_defense, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
| 315 | `6, 0` | `menu_format_number(player_state.fire_defense, 6, 0, gs.codes);` | The count is a decimal field width (four HP/MP digits or six other digits); padding mode zero selects leading blank glyphs. These are local presentation parameters. |
