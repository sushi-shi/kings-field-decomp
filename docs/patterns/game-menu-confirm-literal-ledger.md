# GAME confirmation and item retained-literal ledger

Complete per-occurrence accounting for the three sources below. See the
[choice-domain evidence](game-menu-confirm-choice.md) and
[decoded labels](game-menu-confirmation-protocol.md#protocol-and-labels).
Named definitions, claims, comments, string contents and identifier digits are
excluded. Unary signs remain in expressions. Repeated literals have separate
rows. The [shop-control review](game-shop-menu-controls.md) subsequently names
the root action/phase controls, mixed list-result sentinels and shop viewport.
Other documented retained selectors may still need a future semantic domain.

## `src/game/menu_list_interact.c`

56 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `menu_list_interact` | 29 | `0` | `pad = 0;` | Initial button word has no pressed bits for subsequent edge detection. |
| `menu_list_interact` | 31 | `1` | `while (PadRead(1) != 0) {` | Ignored retail PadRead call-site argument one; the linked SDK uses its global pad identifier, so this is not named as a controller port. |
| `menu_list_interact` | 31 | `0` | `while (PadRead(1) != 0) {` | Preserve the ignored retail PadRead argument one; the linked SDK reads its global pad identifier. A zero returned button mask means release; the loop waits while any button remains pressed. |
| `menu_list_interact` | 39 | `0` | `opt0.codes[0] = 0x72;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 39 | `0x72` | `opt0.codes[0] = 0x72;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 40 | `1` | `opt0.codes[1] = 0x42;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 40 | `0x42` | `opt0.codes[1] = 0x42;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 42 | `0` | `opt0.codes[0] = 0x75;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 42 | `0x75` | `opt0.codes[0] = 0x75;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 43 | `1` | `opt0.codes[1] = 0x52;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 43 | `0x52` | `opt0.codes[1] = 0x52;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 44 | `2` | `opt0.codes[2] = 0x6a;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 44 | `0x6a` | `opt0.codes[2] = 0x6a;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 45 | `3` | `opt0.codes[3] = MENU_TEXT_END;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 48 | `0` | `opt0.codes[0] = 0x59;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 48 | `0x59` | `opt0.codes[0] = 0x59;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 49 | `1` | `opt0.codes[1] = 0x41;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 49 | `0x41` | `opt0.codes[1] = 0x41;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 51 | `0` | `opt0.codes[0] = 0x74;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 51 | `0x74` | `opt0.codes[0] = 0x74;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 52 | `1` | `opt0.codes[1] = 0x42;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 52 | `0x42` | `opt0.codes[1] = 0x42;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 54 | `0` | `opt0.codes[0] = 0x73;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 54 | `0x73` | `opt0.codes[0] = 0x73;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 55 | `1` | `opt0.codes[1] = 0x6a;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 55 | `0x6a` | `opt0.codes[1] = 0x6a;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 57 | `0` | `opt0.codes[0] = 0x70;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 57 | `0x70` | `opt0.codes[0] = 0x70;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 58 | `1` | `opt0.codes[1] = 0x71;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 58 | `0x71` | `opt0.codes[1] = 0x71;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 60 | `2` | `opt0.codes[2] = MENU_TEXT_END;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 63 | `0` | `opt1.codes[0] = 0x41;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 63 | `0x41` | `opt1.codes[0] = 0x41;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 64 | `1` | `opt1.codes[1] = 0x41;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 64 | `0x41` | `opt1.codes[1] = 0x41;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 65 | `2` | `opt1.codes[2] = 0x43;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 65 | `0x43` | `opt1.codes[2] = 0x43;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 67 | `0` | `opt1.codes[0] = 99;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 67 | `99` | `opt1.codes[0] = 99;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 68 | `1` | `opt1.codes[1] = 0x61;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 68 | `0x61` | `opt1.codes[1] = 0x61;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 69 | `2` | `opt1.codes[2] = 0x6a;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 69 | `0x6a` | `opt1.codes[2] = 0x6a;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 71 | `3` | `opt1.codes[3] = MENU_TEXT_END;` | Authored glyph sequence and consecutive element index for the selected use/drop/yes/buy/sell/equip or no/cancel label; the protocol dossier decodes each sequence. Numeric codes are text asset data; the named terminator ends the string. |
| `menu_list_interact` | 99 | `1` | `while (PadRead(1) != 0) {` | Ignored retail PadRead call-site argument one; the linked SDK uses its global pad identifier, so this is not named as a controller port. |
| `menu_list_interact` | 99 | `0` | `while (PadRead(1) != 0) {` | Preserve the ignored retail PadRead argument one; the linked SDK reads its global pad identifier. A zero returned button mask means release; the loop waits while any button remains pressed. |
| `menu_list_interact` | 107 | `1` | `pad = PadRead(1);` | Ignored retail PadRead call-site argument one; the linked SDK uses its global pad identifier, so this is not named as a controller port. |
| `menu_list_interact` | 108 | `0` | `if (((pad & PADLup) != 0 && (prev_pad & PADLup) == 0) \|\|` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `menu_list_interact` | 108 | `0` | `if (((pad & PADLup) != 0 && (prev_pad & PADLup) == 0) \|\|` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `menu_list_interact` | 109 | `0` | `((pad & PADLdown) != 0 && (prev_pad & PADLdown) == 0)) {` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `menu_list_interact` | 109 | `0` | `((pad & PADLdown) != 0 && (prev_pad & PADLdown) == 0)) {` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `menu_list_interact` | 116 | `0` | `} else if ((pad & PADRright) != 0 && (prev_pad & PADRright) == 0) {` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `menu_list_interact` | 116 | `0` | `} else if ((pad & PADRright) != 0 && (prev_pad & PADRright) == 0) {` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `menu_list_interact` | 120 | `0` | `} else if ((pad & PADRdown) != 0 && (prev_pad & PADRdown) == 0) {` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `menu_list_interact` | 120 | `0` | `} else if ((pad & PADRdown) != 0 && (prev_pad & PADRdown) == 0) {` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `menu_list_interact` | 135 | `1` | `} while (1);` | Modal loop continues until the explicit completed-result return after its final redraw and button-release wait. |

## `src/game/menu_two_option_prompt.c`

26 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `menu_two_option_prompt` | 27 | `0` | `s32 input = 0;` | Initial button word has no pressed bits for subsequent edge detection. |
| `menu_two_option_prompt` | 31 | `1` | `while (PadRead(1) != 0)` | Ignored retail PadRead call-site argument one; the linked SDK uses its global pad identifier, so this is not named as a controller port. |
| `menu_two_option_prompt` | 31 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument one; the linked SDK reads its global pad identifier. A zero returned button mask means release; the loop waits while any button remains pressed. |
| `menu_two_option_prompt` | 39 | `0` | `label_a.codes[0] = 0x59;` | Consecutive glyph/terminator position in the authored yes (はい) or no (いいえ) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `menu_two_option_prompt` | 39 | `0x59` | `label_a.codes[0] = 0x59;` | Consecutive glyph/terminator position in the authored yes (はい) or no (いいえ) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `menu_two_option_prompt` | 40 | `1` | `label_a.codes[1] = 0x41;` | Consecutive glyph/terminator position in the authored yes (はい) or no (いいえ) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `menu_two_option_prompt` | 40 | `0x41` | `label_a.codes[1] = 0x41;` | Consecutive glyph/terminator position in the authored yes (はい) or no (いいえ) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `menu_two_option_prompt` | 41 | `2` | `label_a.codes[2] = MENU_TEXT_END;` | Consecutive glyph/terminator position in the authored yes (はい) or no (いいえ) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `menu_two_option_prompt` | 44 | `0` | `label_b.codes[0] = 0x41;` | Consecutive glyph/terminator position in the authored yes (はい) or no (いいえ) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `menu_two_option_prompt` | 44 | `0x41` | `label_b.codes[0] = 0x41;` | Consecutive glyph/terminator position in the authored yes (はい) or no (いいえ) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `menu_two_option_prompt` | 45 | `1` | `label_b.codes[1] = 0x41;` | Consecutive glyph/terminator position in the authored yes (はい) or no (いいえ) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `menu_two_option_prompt` | 45 | `0x41` | `label_b.codes[1] = 0x41;` | Consecutive glyph/terminator position in the authored yes (はい) or no (いいえ) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `menu_two_option_prompt` | 46 | `2` | `label_b.codes[2] = 0x43;` | Consecutive glyph/terminator position in the authored yes (はい) or no (いいえ) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `menu_two_option_prompt` | 46 | `0x43` | `label_b.codes[2] = 0x43;` | Consecutive glyph/terminator position in the authored yes (はい) or no (いいえ) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `menu_two_option_prompt` | 47 | `3` | `label_b.codes[3] = MENU_TEXT_END;` | Consecutive glyph/terminator position in the authored yes (はい) or no (いいえ) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `menu_two_option_prompt` | 56 | `1` | `while (PadRead(1) != 0)` | Ignored retail PadRead call-site argument one; the linked SDK uses its global pad identifier, so this is not named as a controller port. |
| `menu_two_option_prompt` | 56 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument one; the linked SDK reads its global pad identifier. A zero returned button mask means release; the loop waits while any button remains pressed. |
| `menu_two_option_prompt` | 63 | `1` | `input = PadRead(1);` | Ignored retail PadRead call-site argument one; the linked SDK uses its global pad identifier, so this is not named as a controller port. |
| `menu_two_option_prompt` | 64 | `0` | `if (((input & PADLup) != 0 && (prev & PADLup) == 0) \|\|` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `menu_two_option_prompt` | 64 | `0` | `if (((input & PADLup) != 0 && (prev & PADLup) == 0) \|\|` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `menu_two_option_prompt` | 65 | `0` | `((input & PADLdown) != 0 && (prev & PADLdown) == 0)) {` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `menu_two_option_prompt` | 65 | `0` | `((input & PADLdown) != 0 && (prev & PADLdown) == 0)) {` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `menu_two_option_prompt` | 71 | `0` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `menu_two_option_prompt` | 71 | `0` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `menu_two_option_prompt` | 75 | `0` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `menu_two_option_prompt` | 75 | `0` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |

## `src/game/item.c`

146 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `item_load_floor_placements` | 52 | `0` | `game_graphics_runtime.floor_item_count = 0;` | Begin the placement count at zero before scanning the end marker. |
| `item_load_database` | 89 | `40` | `char name[40] = "\\KF\\ITEM0\\I000.TMD;1";` | Forty-byte local path workspace holds the 20-character ISO path plus NUL. Only directory/digit positions are rewritten; the original extra-capacity rationale is unproven. |
| `item_load_database` | 94 | `0` | `if (cd_file_load_allocated(&stat_data, "COM\\STAT.DAT") != 0)` | Any nonzero allocation/load result takes the fatal startup exit; zero alone permits parsing the loaded STAT.DAT banks. |
| `item_load_database` | 95 | `1` | `exit(1);` | Nonzero process-exit status for the fatal database-load path; this is not a file or menu-selection ID. |
| `item_load_database` | 112 | `0` | `for (i = 0; i < KF_ITEM_COUNT; i++) {` | Begin the item-ID/file-table scan at zero; the named item count supplies its exclusive endpoint. |
| `item_load_database` | 113 | `1` | `s32 n = i + 1;` | Convert zero-based item ID to the one-based file number I001 through I080. |
| `item_load_database` | 116 | `8` | `name[8] = i / 30 + '1';` | Path byte eight selects ITEM1/ITEM2/ITEM3; integer division groups IDs 0..29, 30..59 and 60..79 into thirty-file directory bands, and ASCII one encodes the directory digit. Preserve the shipped file organization. |
| `item_load_database` | 116 | `30` | `name[8] = i / 30 + '1';` | Path byte eight selects ITEM1/ITEM2/ITEM3; integer division groups IDs 0..29, 30..59 and 60..79 into thirty-file directory bands, and ASCII one encodes the directory digit. Preserve the shipped file organization. |
| `item_load_database` | 116 | `'1'` | `name[8] = i / 30 + '1';` | Path byte eight selects ITEM1/ITEM2/ITEM3; integer division groups IDs 0..29, 30..59 and 60..79 into thirty-file directory bands, and ASCII one encodes the directory digit. Preserve the shipped file organization. |
| `item_load_database` | 117 | `11` | `name[11] = n / 100 + '0';` | Path byte eleven is the decimal hundreds digit of the one-based file number; divide by one hundred and add ASCII zero. |
| `item_load_database` | 117 | `100` | `name[11] = n / 100 + '0';` | Path byte eleven is the decimal hundreds digit of the one-based file number; divide by one hundred and add ASCII zero. |
| `item_load_database` | 117 | `'0'` | `name[11] = n / 100 + '0';` | Path byte eleven is the decimal hundreds digit of the one-based file number; divide by one hundred and add ASCII zero. |
| `item_load_database` | 118 | `100` | `rem = n % 100;` | Keep the final two decimal digits after writing the hundreds position. |
| `item_load_database` | 119 | `12` | `name[12] = rem / 10 + '0';` | Path byte twelve is the decimal tens digit; base ten and ASCII zero encode it. |
| `item_load_database` | 119 | `10` | `name[12] = rem / 10 + '0';` | Path byte twelve is the decimal tens digit; base ten and ASCII zero encode it. |
| `item_load_database` | 119 | `'0'` | `name[12] = rem / 10 + '0';` | Path byte twelve is the decimal tens digit; base ten and ASCII zero encode it. |
| `item_load_database` | 120 | `13` | `name[13] = rem % 10 + '0';` | Path byte thirteen is the decimal units digit; modulo ten and ASCII zero encode it. |
| `item_load_database` | 120 | `10` | `name[13] = rem % 10 + '0';` | Path byte thirteen is the decimal units digit; modulo ten and ASCII zero encode it. |
| `item_load_database` | 120 | `'0'` | `name[13] = rem % 10 + '0';` | Path byte thirteen is the decimal units digit; modulo ten and ASCII zero encode it. |
| `item_load_database` | 121 | `0` | `if (CdSearchFile((CdlFILE *)&cd_file_table[i], name) != 0) {` | Any nonzero SDK search result identifies a found file whose recorded size may be rounded; failed searches leave that table entry alone. |
| `item_load_database` | 122 | `1` | `if ((cd_file_table[i].size & (KF_CD_SECTOR_BYTES - 1)) != 0)` | Subtract one from the power-of-two sector size to form its low-byte-count mask. Zero masked bits mean already aligned; only partial sectors enter the round-up path. |
| `item_load_database` | 122 | `0` | `if ((cd_file_table[i].size & (KF_CD_SECTOR_BYTES - 1)) != 0)` | Subtract one from the power-of-two sector size to form its low-byte-count mask. Zero masked bits mean already aligned; only partial sectors enter the round-up path. |
| `item_load_database` | 124 | `1` | `((cd_file_table[i].size >> KF_CD_SECTOR_SHIFT) + 1) << KF_CD_SECTOR_SHIFT;` | Add one to the truncated complete-sector count before shifting back to bytes; the preceding partial-sector guard prevents rounding an aligned size up unnecessarily. |
| `item_menu_root` | 139 | `0` | `s32 input = 0;` | No prior pressed button bits at entry; used for input edge detection. |
| `item_menu_root` | 153 | `1` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_root` | 153 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_root` | 163 | `1` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_root` | 163 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_root` | 176 | `1` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_root` | 176 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_root` | 184 | `1` | `input = PadRead(1);` | Ignored retail call-site argument; this is not a controller-port selector. |
| `item_menu_root` | 185 | `0` | `if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_root` | 185 | `0` | `if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_root` | 191 | `0` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_root` | 191 | `0` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_root` | 197 | `0` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_root` | 197 | `0` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_root` | 204 | `0` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_root` | 204 | `0` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_buy` | 229 | `0` | `s32 input = 0;` | No prior pressed button bits at entry; used for input edge detection. |
| `item_menu_buy` | 233 | `1` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_buy` | 233 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_buy` | 238 | `0` | `found = 0;` | Start appending displayed entries at the first workspace row. |
| `item_menu_buy` | 240 | `0` | `if (inv[slot] != 0 && item_stock[KF_ITEM_STOCK_PLAYER][slot] < KF_ITEM_STACK_CAPACITY) {` | Any nonzero shop availability includes an item if the player-owned bank zero is below its named stack cap. |
| `item_menu_buy` | 241 | `0` | `for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)` | Copy all ten glyph halfwords, starting at zero, from the shared item-name row. |
| `item_menu_buy` | 248 | `0` | `for (slot = 0; slot < KF_ENUM_ENCODE(s32, KF_ITEM_VERDITE); slot++) {` | Append the remaining database IDs 0..41 after the 42..79 band; preserve retail display order. |
| `item_menu_buy` | 249 | `0` | `if (inv[slot] != 0 && item_stock[KF_ITEM_STOCK_PLAYER][slot] < KF_ITEM_STACK_CAPACITY) {` | Any nonzero shop availability includes an item if the player-owned bank zero is below its named stack cap. |
| `item_menu_buy` | 250 | `0` | `for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)` | Copy all ten glyph halfwords, starting at zero, from the shared item-name row. |
| `item_menu_buy` | 260 | `0` | `ctx.glyph_rows = &entries[0][0];` | Flat renderer input begins at the first glyph of the first row. |
| `item_menu_buy` | 260 | `0` | `ctx.glyph_rows = &entries[0][0];` | Flat renderer input begins at the first glyph of the first row. |
| `item_menu_buy` | 261 | `0` | `ctx.quantities = 0;` | Null quantity source disables count rendering in this shop list; availability still controls inclusion. |
| `item_menu_buy` | 264 | `0` | `if (ctx.entry_count != 0) {` | Only load/render a preview when the constructed list has an entry. |
| `item_menu_buy` | 265 | `0` | `if (menu_load_item_model(index[ctx.selected_index]) != 0)` | Any nonzero model-load result aborts the panel; retain the complete failure predicate. |
| `item_menu_buy` | 283 | `1` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_buy` | 283 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_buy` | 289 | `1` | `input = PadRead(1);` | Ignored retail call-site argument; this is not a controller-port selector. |
| `item_menu_buy` | 290 | `0` | `if (ctx.entry_count == 0) {` | Handle an empty list without reading a selected item. |
| `item_menu_buy` | 291 | `0` | `if (input != 0) {` | Any button press acknowledges an empty list and exits its panel. |
| `item_menu_buy` | 295 | `0` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_buy` | 295 | `0` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_buy` | 297 | `0` | `if (ctx.selected_index != 0) {` | Upward movement has not reached the first zero-based entry. |
| `item_menu_buy` | 299 | `0` | `if (ctx.cursor_row == 0)` | At the first visible row, scroll upward; otherwise move the cursor. Retail lays out the scroll arm first. |
| `item_menu_buy` | 304 | `1` | `ctx.selected_index = ctx.entry_count - 1;` | Wrap to the last zero-based entry before choosing the short-list or full-viewport arm; required for long lists too. |
| `item_menu_buy` | 306 | `0` | `ctx.scroll_offset = 0;` | Reset the viewport to start at the first entry. |
| `item_menu_buy` | 307 | `1` | `ctx.cursor_row = ctx.entry_count - 1;` | Bottom row of a list shorter than the viewport. |
| `item_menu_buy` | 310 | `1` | `ctx.cursor_row = ctx.visible_rows - 1;` | Last zero-based row of the full viewport. |
| `item_menu_buy` | 315 | `0` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_buy` | 315 | `0` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_buy` | 317 | `1` | `if (ctx.selected_index < ctx.entry_count - 1) {` | Test the last-entry boundary before moving downward or wrapping. |
| `item_menu_buy` | 319 | `1` | `if (ctx.cursor_row == ctx.visible_rows - 1)` | At the bottom visible row, scroll downward; otherwise move the cursor. Retail lays out the scroll arm first. |
| `item_menu_buy` | 324 | `0` | `ctx.selected_index = 0;` | Wrap to the first item in the list. |
| `item_menu_buy` | 325 | `0` | `ctx.scroll_offset = 0;` | Reset the viewport to start at the first entry. |
| `item_menu_buy` | 326 | `0` | `ctx.cursor_row = 0;` | Place the visible cursor on the first row after wrapping. |
| `item_menu_buy` | 328 | `0` | `if (menu_load_item_model(index[ctx.selected_index]) != 0)` | Any nonzero model-load result aborts the panel; retain the complete failure predicate. |
| `item_menu_buy` | 330 | `0` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_buy` | 330 | `0` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_buy` | 338 | `0` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_buy` | 338 | `0` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_buy` | 344 | `0` | `if (ctx.entry_count != 0)` | Only render a preview when the constructed list has an entry. |
| `item_menu_sell` | 375 | `0` | `s32 input = 0;` | No prior pressed button bits at entry; used for input edge detection. |
| `item_menu_sell` | 379 | `1` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_sell` | 379 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_sell` | 384 | `0` | `found = 0;` | Start appending displayed entries at the first workspace row. |
| `item_menu_sell` | 385 | `0` | `for (slot = 0; slot < KF_ENUM_ENCODE(s32, KF_ITEM_GOLD_CROSS); slot++) {` | Start with item zero and stop before Gold Cross ID 52; the Gold Cross and later key/quest-item band is excluded from selling. |
| `item_menu_sell` | 386 | `0` | `if (inv[slot] != 0) {` | Only possessed player items can become sale candidates. |
| `item_menu_sell` | 396 | `0` | `if (available[found] != 0) {` | After subtracting an equipped copy, retain the row only if another copy remains available. |
| `item_menu_sell` | 397 | `0` | `for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)` | Copy all ten glyph halfwords, starting at zero, from the shared item-name row. |
| `item_menu_sell` | 407 | `0` | `ctx.glyph_rows = &entries[0][0];` | Flat renderer input begins at the first glyph of the first row. |
| `item_menu_sell` | 407 | `0` | `ctx.glyph_rows = &entries[0][0];` | Flat renderer input begins at the first glyph of the first row. |
| `item_menu_sell` | 408 | `0` | `ctx.quantities = 0;` | Null quantity source disables count rendering in this shop list; availability still controls inclusion. |
| `item_menu_sell` | 411 | `0` | `if (ctx.entry_count != 0) {` | Only load/render a preview when the constructed list has an entry. |
| `item_menu_sell` | 412 | `0` | `if (menu_load_item_model(index[ctx.selected_index]) != 0)` | Any nonzero model-load result aborts the panel; retain the complete failure predicate. |
| `item_menu_sell` | 430 | `1` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_sell` | 430 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument 1 and wait until no buttons remain pressed; the linked SDK uses global PadIdentifier. |
| `item_menu_sell` | 436 | `1` | `input = PadRead(1);` | Ignored retail call-site argument; this is not a controller-port selector. |
| `item_menu_sell` | 437 | `0` | `if (ctx.entry_count == 0) {` | Handle an empty list without reading a selected item. |
| `item_menu_sell` | 438 | `0` | `if (input != 0) {` | Any button press acknowledges an empty list and exits its panel. |
| `item_menu_sell` | 442 | `0` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_sell` | 442 | `0` | `} else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_sell` | 444 | `0` | `if (ctx.selected_index != 0) {` | Upward movement has not reached the first zero-based entry. |
| `item_menu_sell` | 446 | `0` | `if (ctx.cursor_row == 0)` | At the first visible row, scroll upward; otherwise move the cursor. Retail lays out the scroll arm first. |
| `item_menu_sell` | 451 | `1` | `ctx.selected_index = ctx.entry_count - 1;` | Wrap to the last zero-based entry before choosing the short-list or full-viewport arm; required for long lists too. |
| `item_menu_sell` | 453 | `0` | `ctx.scroll_offset = 0;` | Reset the viewport to start at the first entry. |
| `item_menu_sell` | 454 | `1` | `ctx.cursor_row = ctx.entry_count - 1;` | Bottom row of a list shorter than the viewport. |
| `item_menu_sell` | 457 | `1` | `ctx.cursor_row = ctx.visible_rows - 1;` | Last zero-based row of the full viewport. |
| `item_menu_sell` | 460 | `0` | `if (menu_load_item_model(index[ctx.selected_index]) != 0)` | Any nonzero model-load result aborts the panel; retain the complete failure predicate. |
| `item_menu_sell` | 462 | `0` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_sell` | 462 | `0` | `} else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_sell` | 464 | `1` | `if (ctx.selected_index < ctx.entry_count - 1) {` | Test the last-entry boundary before moving downward or wrapping. |
| `item_menu_sell` | 466 | `1` | `if (ctx.cursor_row == ctx.visible_rows - 1)` | At the bottom visible row, scroll downward; otherwise move the cursor. Retail lays out the scroll arm first. |
| `item_menu_sell` | 471 | `0` | `ctx.selected_index = 0;` | Wrap to the first item in the list. |
| `item_menu_sell` | 472 | `0` | `ctx.scroll_offset = 0;` | Reset the viewport to start at the first entry. |
| `item_menu_sell` | 473 | `0` | `ctx.cursor_row = 0;` | Place the visible cursor on the first row after wrapping. |
| `item_menu_sell` | 475 | `0` | `if (menu_load_item_model(index[ctx.selected_index]) != 0)` | Any nonzero model-load result aborts the panel; retain the complete failure predicate. |
| `item_menu_sell` | 477 | `0` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_sell` | 477 | `0` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_sell` | 480 | `0` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_sell` | 480 | `0` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | A named current button bit is set and its previous bit is clear: zero/nonzero tests detect one rising edge. |
| `item_menu_sell` | 486 | `0` | `if (ctx.entry_count != 0)` | Only render a preview when the constructed list has an entry. |
| `item_pickup_confirm` | 509 | `0` | `s32 input = 0;` | Initial button word has no pressed bits for subsequent edge detection. |
| `item_pickup_confirm` | 515 | `0` | `if (menu_load_item_model(item_id) != 0)` | Any nonzero model-loader status aborts pickup with the existing not-acquired result. |
| `item_pickup_confirm` | 520 | `0` | `accept_label.codes[0] = 0x53;` | Consecutive glyph/terminator position in the take (とる) or cancel (やめる) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `item_pickup_confirm` | 520 | `0x53` | `accept_label.codes[0] = 0x53;` | Consecutive glyph/terminator position in the take (とる) or cancel (やめる) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `item_pickup_confirm` | 521 | `1` | `accept_label.codes[1] = 0x6a;` | Consecutive glyph/terminator position in the take (とる) or cancel (やめる) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `item_pickup_confirm` | 521 | `0x6a` | `accept_label.codes[1] = 0x6a;` | Consecutive glyph/terminator position in the take (とる) or cancel (やめる) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `item_pickup_confirm` | 522 | `2` | `accept_label.codes[2] = MENU_TEXT_END;` | Consecutive glyph/terminator position in the take (とる) or cancel (やめる) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `item_pickup_confirm` | 525 | `0` | `decline_label.codes[0] = 0x63;` | Consecutive glyph/terminator position in the take (とる) or cancel (やめる) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `item_pickup_confirm` | 525 | `0x63` | `decline_label.codes[0] = 0x63;` | Consecutive glyph/terminator position in the take (とる) or cancel (やめる) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `item_pickup_confirm` | 526 | `1` | `decline_label.codes[1] = 0x61;` | Consecutive glyph/terminator position in the take (とる) or cancel (やめる) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `item_pickup_confirm` | 526 | `0x61` | `decline_label.codes[1] = 0x61;` | Consecutive glyph/terminator position in the take (とる) or cancel (やめる) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `item_pickup_confirm` | 527 | `2` | `decline_label.codes[2] = 0x6a;` | Consecutive glyph/terminator position in the take (とる) or cancel (やめる) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `item_pickup_confirm` | 527 | `0x6a` | `decline_label.codes[2] = 0x6a;` | Consecutive glyph/terminator position in the take (とる) or cancel (やめる) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `item_pickup_confirm` | 528 | `3` | `decline_label.codes[3] = MENU_TEXT_END;` | Consecutive glyph/terminator position in the take (とる) or cancel (やめる) label, decoded in the retail font audit; numeric codes remain text asset data. |
| `item_pickup_confirm` | 548 | `1` | `while (PadRead(1) != 0)` | Ignored retail PadRead call-site argument one; the linked SDK uses its global pad identifier, so this is not named as a controller port. |
| `item_pickup_confirm` | 548 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument one; the linked SDK reads its global pad identifier. A zero returned button mask means release; the loop waits while any button remains pressed. |
| `item_pickup_confirm` | 560 | `1` | `while (PadRead(1) != 0)` | Ignored retail PadRead call-site argument one; the linked SDK uses its global pad identifier, so this is not named as a controller port. |
| `item_pickup_confirm` | 560 | `0` | `while (PadRead(1) != 0)` | Preserve the ignored retail PadRead argument one; the linked SDK reads its global pad identifier. A zero returned button mask means release; the loop waits while any button remains pressed. |
| `item_pickup_confirm` | 567 | `1` | `input = PadRead(1);` | Ignored retail PadRead call-site argument one; the linked SDK uses its global pad identifier, so this is not named as a controller port. |
| `item_pickup_confirm` | 568 | `0` | `if (((input & PADLup) != 0 && (prev & PADLup) == 0)` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `item_pickup_confirm` | 568 | `0` | `if (((input & PADLup) != 0 && (prev & PADLup) == 0)` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `item_pickup_confirm` | 569 | `0` | `\|\| ((input & PADLdown) != 0 && (prev & PADLdown) == 0)) {` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `item_pickup_confirm` | 569 | `0` | `\|\| ((input & PADLdown) != 0 && (prev & PADLdown) == 0)) {` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `item_pickup_confirm` | 575 | `0` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `item_pickup_confirm` | 575 | `0` | `} else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `item_pickup_confirm` | 587 | `0` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |
| `item_pickup_confirm` | 587 | `0` | `} else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {` | Zero tests the absence of the named button bit. Current set and previous clear detect a rising edge, preserving movement/confirm/cancel priority. |

The [shared confirmation-state review](menu-confirm-state.md) names the request
flag and preserves its type through both drawing APIs. This ledger reflects
those current expressions; other retained selector domains still need review.
