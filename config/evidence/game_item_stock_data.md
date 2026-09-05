# GAME item-stock ownership campaign

## Function Match Plan (before source changes)

Baseline: `21f786e`, `probe-gcc260-o2-g0`, hash-validated GAME.EXE.
Six views (`addr`, `disasm --blocks`, `xref`, `xref --callees`, `strings`,
`match`) were inspected for each row below, along with source history and
the callers/adjacent claims. Local pre-edit artifacts are under
`build/item-stock-baseline-a54t2Z`; the reproducible six-view dossier is
`build/item-stock-evidence.log` (neither is a committed input).

Hypothesis: GAME `800652a8..80065397` is one BSS object, `u8 item_stock[3][80]`.
Bank 0 holds player quantities; the buy panel selects a bank with `arg * 80`
and independently checks bank 0 against 99. The two price columns use
`arg - 1`. Banks 1 and 2 hold shop availability/stock, not equipment categories.
The buy panel decrements shop stock only for item 52; do not describe every
shop entry as a finite quantity. Item IDs remain numeric, without guessed lore.

The initialization function clears all 240 bytes and seeds all three banks.
It supports reconstruction ownership in `game.player_death`, not proof of an
original TU boundary. Move the shared declaration from `game_state.h` to
`item.h`, and give the complete save field the same two-dimensional byte type.
No overlapping interior globals, explicit BSS initializer, alignment override,
new size assertion, or compiler-shaping statement is justified.

All bodies remain C game policy, absent from the vendored inventory. Existing
SDK/GTE/I/O callees are library boundaries, not reconstructed library bodies.
The prior vendor-negative dossiers are `game_semantic_player_death.tsv`,
`game_semantic_item_menu_data.tsv`, and `game_semantic_save_system.tsv`.
No candidate indirect branch or relocation is promoted by this campaign.

Each row retains the existing signature, call set, branch topology and return
delay slot. Byte loads/stores remain unsigned. Stable cursor bases and byte
post-decrements are preserved (see `docs/patterns/source-shapes-gcc257.md`).
The only first hypothesis is a complete, shared data owner and bank indexing;
the eight non-exact bodies' existing code-generation residues are not targets.

| GAME VA / extent | Function | Before % | Individual evidence and intended source change | Final verdict |
| --- | --- | ---: | --- | --- |
| 151cc / 2e4 | game_state_initialize | 100 | No arguments/result; startup and restart callers; three equipment/magic helper calls. Clear loop count `ef`, 33 constant `sb` seeds, world clear `2134`. Whole-array byte cursor and bank/slot seed indices; retain helper arguments 0, (0, ff), 8 and delay-slot loop increment. | Exact retained (100%) |
| 154b0 / 19c | player_death_restart | 100 | No arguments/result; death-update caller. Bank-0 item 47 enables the persist branch, otherwise initialization; retain byte post-decrement and call-delay store, position/pool/audio calls. | Exact retained (100%) |
| 18054 / 45c | player_use_item | 100 | u8 item argument masked at entry from player update; switch and GTE/trigonometric callees unchanged. Bank-0 object-ID store and item 57 post-decrement/light timer 1000. | Exact retained (100%) |
| 21538 / 5c4 | item_menu_buy | 97.20054 | s32 shop argument from item-menu root, 80-byte multiplication, scans 42..79 then 0..41; unsigned stock bytes, bank-0 cap 99. Retain menu calls/CFG, price halfwords, item-52 shop decrement and player increment. | Non-exact, score and bytes unchanged |
| 21afc / 500 | item_menu_sell | 96.77187 | s32 price-column argument from root; scans player bank, excludes equipped copies. Stable bank-0 pointer, selected byte post-decrement and price/gold update; no shop-page indexing. | Non-exact, score and bytes unchanged |
| 21ffc / 2b8 | item_use_confirm | 100 | s32 item, s32 result through menu-enter mode 1. Byte quantity capped at 99, confirmation increments bank 0; retains sentinel -99, results 0/1/2, pad/UI calls and edge tests. Existing consumption comment is contradicted by `lbu/addiu 1/sb`. | Exact retained (100%) |
| 22608 / 774 | menu_use_item_panel | 92.8218 | No args, selection/negative result through menu root; item 55 pretest, stable bank-0 base; special slots 55/73 and consumable ranges. Retain pad/UI/HP/status calls and selection CFG. | Non-exact, score and bytes unchanged |
| 238d8 / 5c4 | menu_equip_select | 97.89973 | Existing s32 source equipment category (curated u32 signature remains WIP), category-dependent ranges within 80; bank-0 pointer plus range start. Preserve preheader pointer, switch, item/equipment/UI calls. | Non-exact, score and bytes unchanged |
| 249a8 / 4bc | menu_drop_item | 98.85478 | No args; scans 80 quantities, excludes equipped copies, selected byte decrement. Keep stable base and all menu/model/pad calls. | Non-exact, score and bytes unchanged |
| 279c4 / 1b8 | menu_item_model_preview | 98.181816 | s32 item, ff guard, GTE model and ten-halfword item name; unsigned quantity read by item ID for width-2 number formatter. Only bank-0 subscript changes. | Non-exact, score and bytes unchanged |
| 27b7c / 2dc | menu_draw_item_detail | 91.28416 | s32 item/shop/mode, unsigned halfword prices, byte quantity and width-2 formatter; bank 0. Menu render calls and price branches unchanged. | Non-exact, score and bytes unchanged |
| 2b73c / 4f4 | save_file_write_slot | 100 | Signed-16-bit 1-based slot, s32 result, save-system caller. SDK file calls/path strings unchanged. Stock source 2b974/978; payload +2440; full 240-byte alignment-checked copy, end-pointer +240 in branch delay slot. | Exact retained (100%) |
| 2beb0 / 3cc | save_file_read_slot | 100 | Signed-16-bit slot, s32 result, save-system caller. SDK read/path branches unchanged. Destination 2c16c/170; source payload +2440, end +2530 in branch delay slot. Same complete byte-aligned save field. | Exact retained (100%) |
| 343e0 / 58 | map_action_script_floor1 | 100 | No args, floor dispatch caller; bank-0 item 56 read gates world flag, copy-region and sound calls. | Exact retained (100%) |
| 34610 / 90 | map_action_script_floor3 | 100 | No args, floor dispatch caller; bank-0 item 50 gates magic record 7 and notification; other world/magic branch untouched. | Exact retained (100%) |
| 346a8 / 38c | map_floor5_transition_cutscene | 100 | No args, floor-5 script caller; unequip weapon 10 if selected, clear bank-0 item 10, then existing camera/effect sequence. | Exact retained (100%) |
| 34a80 / 2d4 | map_event_interact | 100 | Existing event-pointer argument from dispatch; item 52 -> 53, item 59 consumption, item 47 -> 62. Retain three event branches, post-decrements, notification and delay-slot stores. | Exact retained (100%) |
| 35e44 / 69c | map_restore_floor_state | 99.96454 | No args, floor-load caller; restore loops and unresolved floor dispatch unchanged. Bank-0 items 10/11 gate clearing link 52. | Non-exact, score and bytes unchanged |

No narrow string identity is part of this BSS object. The save functions' two
memory-card path literals stay in their existing RODATA owner. The other
consumers use existing glyph/data tables rather than a literal at this address.

## Planned verification

Check one 240-byte BSS owner, independent compiler `sizeof` and real ELF extent,
all reviewed HI16/LO16 pairs including interior addends, and a bounded retail
initializer control with nonzero fill/neighbor sentinels. Check both shop-bank
selections independently. Compare source text bytes and function scores against
the saved baseline, then run focused matches, all tests/lint, full build and
flake checks. Any real 16-byte source alignment conflict at retail address
`800652a8` remains a strict failure; identical BSS size alone is not closure.

## Final verdict

The shared owner and three-bank type are retained. All 18 individual verdicts
are recorded above: ten exact consumers remain exactly 100%; eight pre-existing
non-exact consumers have identical code bytes and scores. Across all 112 source
objects, `.text`, `.rodata`, `.data`, `.sdata` bytes and ordered relocations are
unchanged after normalizing only the renamed identity. All 484 function-score
rows are unchanged: 360/471 game functions plus 13 vendor controls are exact.
All 101 source DATA extents agree with the independent pinned-compiler size
probe (formerly 100 claims). No new function banking or SDK reclassification.

Both source and target contain one global 240-byte `SHT_NOBITS` symbol at BSS
offset zero. Source BSS alignment is genuinely 16; target alignment is 8.
`800652a8 % 16 == 8`, so the strict comparator correctly rejects source
placement. The owner moves from config-only to source-owned, but is **not a
strict data match**. No load-image bytes or zero initializer were invented for
this runtime object; the initializer control is behavioral evidence, not
initialized-data equality.

All 64 reviewed paired references round-trip to their original retail words
with the same interior offsets, low opcodes and evidence levels. A bounded
retail initializer control checks both nonzero-fill patterns, all 240 output
bytes, the 33 seed positions, untouched neighbors, the 8500-byte world clear,
and the three ordered helper arguments. Sixteen bounded retail buy-list cases
check both banks, player counts 0/98/99, all selected positions and cross-bank
contamination. These controls run individual retail bodies with explicit
hooks; they do not execute the game or waive candidate placement constraints.

Source/header literal `DAT_` occurrences: **167 -> 100**. Unresolved data
identities: **2750 -> 2749**. GAME reached ownership: **82 -> 83** source ranges,
**357 -> 356** config-only ranges. Unmatched reached config ranges across the
images: **623 -> 622** (GAME 354, OPEN 268; four separately matched SDK ranges
remain outside that unmatched count).

Strict source-data units: **8/60 -> 7/60** (GAME 5/40, OPEN 2/19, PSX 0/1).
Independent SDK data controls remain **4/4**; target round-trip remains
**110/116** with the same six unrelated conflicting-section-base units.
The final full build is red for strict data/reachability/placement, not a
compiler failure or a banked-function regression. Full reachable-byte closure
and linked-executable comparison are still incomplete.

Verification: forced focused compiles for all 12 consuming units; recorded
GAME comparison; full `kf build`; `ruff check scripts tests`; **602 tests pass
locally without skips**; `nix flake check -L` passes (**602 tests, 106 optional
local-artifact/tool skips**, plus native compiler-size controls); and
`git diff --check`. Logs use `build/item-stock-*` and are not committed.
