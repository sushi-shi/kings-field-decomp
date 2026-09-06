# GAME equipment slot and selection domains

## Function Match Plan

Start at `cae2910` with freshly verified GAME retail hashes. Review the
equipment loaders, player slot setter and its initialization/menu callers,
weapon/magic selection, stat/update consumers, empty preview behavior and
equipment menu category dispatch. Six-view dossiers, source history and
initial scores for 26 functions are captured under
`build/constant-names/equipment-domains/`. The complete baseline is
`build/constant-names/post-master-cae2910/`.

Name item/magic empty values separately from the slot setter's refresh-only
selector. Type the byte-sized player slot parameter and the full-width menu
category parameter across their declarations, definitions and callers.
The orders differ: menu weapon/magic/body/shield/head/arm/leg/accessory versus
player shield/head/arm/leg/body/accessory. Generic menu cursor/selection
positions become categories at dispatch; retain the integer widget boundary.
Name menu item ranges and the shared armor-table item origin. Express the
record-copy word counts using the actual array/element sizes and name the
16-record weapon and 42-record armor extents without adding size assertions.

Preserve all predicates, arithmetic, calls, delay slots and ordered referents.
In particular, initialization passes item zero with slot 255 to refresh
record pointers without changing any slot. Empty weapon selection does not
clear its stale record pointer, while empty magic selection does clear its
pointer. Do not homogenize those behaviors. The texture loader's empty value
belongs to a broader numbered-texture API, not exclusively the spell domain.

These are game-owned selection and data-loading rules outside the vendored
inventory. SDK rendering/input calls remain library boundaries. No library
body or candidate indirect edge is promoted. Capture final per-function
verdicts, force compilation and compare the full object/report baseline,
independently verify exact bodies against retail, run modern Clang,
repository tests/lint and full `kf build`, then commit the verified campaign.
The literal audit covers the complete equipment loaders, selection panels
and status-name column; other consumer modules remain in the wider audit.

## Evidence and domain boundaries

The player slot setter masks a1 to eight bits at GAME 8001684c, checks the
range 0..5, and writes the corresponding byte at player offsets 90,91,93,94,
92,95. Initialization supplies 255 and bypasses all writes, then follows the
common pointer-refresh/stat-recalculation tail. Menu row 2 instead supplies
player slot 4; row 3 supplies slot 0. Both callers prove why these are
different types even though most values overlap.

Player item IDs are bytes. Value 255 suppresses equipment stat reads, model
loads and status names; menu lists append that value as their empty choice.
Magic ID 255 independently selects no spell and clears the selected-record
pointer. The generic TIM loader also skips 255, but other callers request
texture numbers 103,104,105,114 and 998; its parameter is not a magic ID.

The armor lookup scales `(item_id - 13)` by the 28-byte record stride. The
loader copies 294 words, agreeing with 42 records. Weapon loading copies 176
words, agreeing with sixteen 44-byte records, and mirrors all sixteen angle
fields with the countdown initialized to fifteen. The equipment menu offers
only thirteen weapon item IDs; record capacity and selectable range are not
the same count.

## Literal coverage and verification

The [three-module ledger](game-equipment-literal-ledger.md) accounts for all
155 ordinary literal occurrences in `equipment.c`, `menu_select.c` and
`menu_draw_name_list.c`, down from 209. Header definitions carry the recovered
domains and capacities; 14 claim literals in these modules retain their
addresses/extents. The local label buffers remain twenty rows of ten glyphs;
their unused capacity has no proven original rationale. Status names retain
fixed twenty-pixel rows even for empty slots. Magic selection lists IDs 4..8,
whereas the separate instant-use panel handles IDs 0..3.

The charge helper's factor 64 and two increments remain explicit with their
measured formula in the ledger. Its output is consumed as a charge step,
not decoded as a Q6 value. Likewise, the modal widget's kind/mode/pending
values are documented pending the shared widget protocol audit. Neither
their numerical choice nor localized glyph identities are invented.

All twelve affected units compile after forced rebuild. All sections in all
112 objects are byte-identical to the `cae2910` baseline, including debug
sections. All 484 strict scores and the entire objdiff report are unchanged.
The 20 exact reviewed functions independently reproduce 2859 complete retail
instruction words after relocation application, with identical ordered calls
and data referents. The six partial functions retain their code and scores.
No functions are newly banked.

Modern Clang retains 64 passing and 48 failing source/image variants; all
320 error diagnostics are unchanged after line/column normalization. The
full repository run exercised 656 tests in 84.858 seconds and found two
stale evidence-table signatures alongside the known untracked save/load-hub
failure. After correcting those existing final-signature rows, all 110
inventory tests pass on the focused rerun, leaving only the unrelated hub
failure. No new tests or size assertions are added. Ruff and whitespace checks
pass. Full `kf build` retains the existing source-data result 7/60, SDK data
4/4 and target relink PSX 1/1, GAME 75/77, OPEN 34/38; data ownership and
placement closure still fail independently of this naming work.

## Function snapshots and final verdicts

All rows select GAME.EXE and `probe-gcc257-o2-g0`. Each captured dossier
contains initial signatures, widths, CFG, delay slots, calls and referents.

| GAME VA / bytes | Function | Initial = final strict % | Verdict / retail words |
| --- | --- | ---: | --- |
| `0x800150a8 / 84` | `weapon_records_load_and_mirror_angles` | 100 | Exact / 21 |
| `0x800150fc / 44` | `armor_records_load` | 100 | Exact / 11 |
| `0x80015128 / 60` | `fixed6_ratio_step` | 100 | Exact / 15 |
| `0x80016848 / 488` | `player_set_equipment_slot` | 100 | Exact / 122 |
| `0x80016a30 / 244` | `player_equip_weapon` | 100 | Exact / 61 |
| `0x80016b24 / 156` | `player_begin_weapon_attack` | 100 | Exact / 39 |
| `0x80016bc0 / 612` | `player_update_weapon_attack` | 100 | Exact / 153 |
| `0x80017a80 / 632` | `player_update_vertical_motion` | 100 | Exact / 158 |
| `0x800151cc / 740` | `game_state_initialize` | 100 | Exact / 185 |
| `0x80015714 / 2068` | `player_recalculate_combat_stats` | 100 | Exact / 517 |
| `0x80016324 / 912` | `player_apply_damage` | 100 | Exact / 228 |
| `0x800167e4 / 100` | `player_select_magic` | 100 | Exact / 25 |
| `0x80018880 / 6684` | `player_update` | 96.9455 | Partial, unchanged |
| `0x800238d8 / 1476` | `menu_equip_select` | 97.8997 | Partial, unchanged |
| `0x80023e9c / 1136` | `menu_spell_select` | 98.0634 | Partial, unchanged |
| `0x800236ac / 556` | `menu_option_root` | 100 | Exact / 139 |
| `0x8002718c / 2104` | `menu_draw_name_list` | 100 | Exact / 526 |
| `0x8002aea4 / 104` | `menu_load_item_model` | 100 | Exact / 26 |
| `0x8002af48 / 304` | `menu_load_item_texture` | 100 | Exact / 76 |
| `0x80027b7c / 732` | `menu_draw_item_detail` | 91.2842 | Partial, unchanged |
| `0x80027e58 / 72` | `menu_add_marker_quad` | 100 | Exact / 18 |
| `0x80027ea0 / 68` | `menu_add_frame_quad` | 100 | Exact / 17 |
| `0x80027ee4 / 1180` | `menu_draw_dialog_frame` | 100 | Exact / 295 |
| `0x800279c4 / 440` | `menu_item_model_preview` | 98.1818 | Partial, unchanged |
| `0x80028380 / 852` | `menu_list_interact` | 87.0798 | Partial, unchanged |
| `0x800346a8 / 908` | `map_floor5_transition_cutscene` | 100 | Exact / 227 |
