# GAME STAT.DAT bank ownership and placement

## Function Match Plan

Baseline `b02ff37`, GAME.EXE, `probe-gcc257-o2-g0`. Define the five remaining
STAT.DAT destinations in `item.c`, retaining their independently established
types, extents and BSS storage. Keep their shared declarations in game/menu.h.
Replace menu_list_init's obsolete interior-global view with the real window
row's glyph field. Do not change the loader, other item function bodies,
signatures, copy lengths, or banked functions to accommodate a data score.

The loader and initializer have full six-view semantic dossiers in the local
`build/menu-banks-evidence.log`: image-qualified address, block disassembly,
incoming/outgoing references, strings and strict match. The item unit's five
other functions are immutable controls, not new reconstruction claims. Prior
type/consumer evidence is in game_semantic_item_menu_data.tsv and
game_semantic_menu_window_layouts.tsv. Source history includes `110ec5b`,
`3a0f6fc`, and the preceding complete menu-asset campaign `b02ff37`.

| GAME VA / body bytes | Function | Before % | Evidence / kept hypothesis |
| --- | --- | ---: | --- |
| 80020b4c / 1b0 | item_load_floor_placements | 98.888885 | Unchanged placement-pointer input, ffff sentinel, 12-byte source and 24-byte destination records; rand remains an external provider. |
| 80020cfc / 5dc | item_load_database | 99.746666 | No arguments; one game_main_loop caller; six exact copy extents and two resource strings prove the separately addressed banks. |
| 800212d8 / 260 | item_menu_root | 100 | Unchanged integer context; window 7 / three-row selector and buy/sell dispatch. Exact unit control. |
| 80021538 / 5c4 | item_menu_buy | 97.200540 | Unchanged shop context; item*20 names and unsigned halfword prices at item*4+column*2. |
| 80021afc / 500 | item_menu_sell | 96.771870 | Unchanged shop context; inventory filtering and the separate sell-price bank. |
| 80021ffc / 2b8 | item_use_confirm | 100 | Unchanged item input and integer outcome; yes/no menu. Exact unit control. |
| 8002ad6c / 8c | menu_list_init | 93.000000 | Leaf, no strings/calls; a0=list, a1=window, a2=row. Twenty glyph bytes begin at window*264+row*24+28. |

These functions are absent from functions_vendored.tsv. Database partitioning,
menu selection and list initialization are game policy; libc, CD, controller
and GPU implementations remain vendor boundaries. No SDK body is reconstructed
or included as new game progress.

## Independent byte and ownership evidence

The resource remains the 5708-byte COM/STAT.DAT documented in
[game_menu_assets.md](game_menu_assets.md). It supplies runtime bytes, not
initialized bytes of GAME.EXE. The loader's six destination address pairs and
source advances independently establish these ranges:

| Owner | GAME VA | Bytes | Resource offset | ELF BSS offset |
| --- | --- | ---: | ---: | ---: |
| menu_assets | 800580e8 | 912 | 0 | 0 |
| menu_window_layouts | 80058478 | 2376 | 912 | 912 |
| item_name_rows | 80058dc0 | 1600 | 3288 | 3288 |
| magic_name_rows | 80059400 | 180 | 4888 | 4888 |
| item_buy_prices | 800594b8 | 320 | 5068 | 5072 |
| item_sell_prices | 800595f8 | 320 | 5388 | 5392 |

The loader copies 176+4 bytes into magic_name_rows, then materializes 800594b8
for prices (`80021050..8002106c`). Thus 800594b4..800594b8 is a four-byte
destination gap, not another glyph, source-resource bytes, or a new global.
The RAM span is 5712 bytes and the six object sizes sum to 5708. Each bank is
separately addressed; there is no evidence here for an invented enclosing C
structure with a padding member. The owning loader module defines all six
globals in ascending order, with consumers using the existing typed externs.

The seven menu_list_init call windows constrain (window, row) to (7,0), (7,1),
(0,0), (0,1), (1,equipment choice), (1,1), and (0,4), respectively. The call
sites are 80021580, 80021b44, 80022648, 800231bc, 80023a80, 80023fac, 800249e8.
Immediate neighbors are primitive_buffer_commit_poly_ft4 and menu_format_number.
The initializer copies ten `lhu`/`sh` halfwords, not four halfwords from an
eight-byte object. Its final `sb` at 8002adf4 is the return delay slot and writes
glyph capacity 8. Fields at +32/+36 are untouched. The old DAT_80058494 extern
and MenuLabelCell view are removed; the reviewed pair at 8002ad98/8002ad9c now
spells menu_window_layouts+28 without changing its raw retail target or tier.

## Delinker packing versus actual placement

The ownership-only source build already emitted the offsets above. The old
target packing rule limited exported BSS claims to four-byte alignment, putting
both price arrays four bytes early and admitting no single correct bank base.
The same address-bounded eight-byte packing constraint already used for private
BSS now also applies to explicitly global BSS. Unknown linkage and initialized
data retain their previous constraints; logical object sizes are not rounded
and arbitrary gaps are not filled.

This working packing constraint is supported by both the retail price-bank gap
and the pinned compiler smoke controls. GCC 2.5.7 emits eight-byte tentative
allocations for four-byte local and exported words, unlike the four-byte
allocations emitted by GCC 2.6.0. It does not prove the historical compiler,
section alignment, or original small-BSS/external-BSS allocation class.

The cross-image target audit changes only section alignment for GAME cd_file,
pool and map_events and OPEN camera_path, render_init and render_map_cells.
Both vendor PAD targets move the distant exported PadIdentifier offset from
12 to 16; their existing incompatible bases remain failures. Other changes
are the six-bank owner and the initializer's corrected referent. No unrelated
initialized payload or instruction is altered. Independent target relinking
stays 108/114 with the same six conflicting-section-base failures.

Crucially, identical bank extents and named offsets are not sufficient for
source closure. The target's eight-byte section alignment permits 800580e8;
the compiled source's sixteen-byte BSS alignment does not. The default data
gate now reuses the relink verifier's placement rules on both sides, retaining
every DATA/RODATA claim and ignoring only code-size failures in this data check.
It rejects the menu BSS with invalid-section-placement. No ELF flags, source
padding or claim address is modified to suppress that finding.

The same check removes five previous data-only matches: GAME item,
save_system (.rodata), lighting (.data), OPEN render_map_cells and
opening_entity_pool. Strict source data is now 6/59 (PSX 0/1, GAME 4/39,
OPEN 2/19), with SDK provider contributions still 2/2. Native objdiff section
scores alone do not establish placement at the curated retail addresses.

## Final function verdicts and controls

All six item function instruction streams and ordered relocations are unchanged
from the baseline. The two exact unit controls remain exact; the other four
retain their preceding scores. Their first residues remain, respectively,
the placement counter register/copy, filename-division registers, and buy/sell
local allocation followed by later CFG differences. These are not attributed
compiler mechanisms or new reconstruction results.

menu_list_init is 80.714290%, not exact. Its first difference is a copy of the
list argument to a3; the typed row access materializes owner+24 then adds the
four-byte glyph-field offset, whereas retail directly materializes owner+28.
The full effective address agrees, but the extra address arithmetic and register
choices do not byte-match. Retain the evidenced field access, not the obsolete
interior global, and do not bank this result. All other 483 function-score rows
are unchanged, preserving all 360 exact game functions and 13 vendor controls.

Tests verify every byte of all six runtime copies at all four source alignments
and preserve the four-byte gap. All 90 window/row selections are exercised in
both retail and compiled initializer bodies, with high-bit glyph codes,
exact prefix writes, preserved pointers, and no extra calls. All 110 reviewed
bank references (7 window, 83 item-name, 13 magic-name, 4 buy, 3 sell) pass safe
delinking and raw HI16/LO16 restoration. The preceding 380 menu-asset pairs and
renderer controls remain covered. These bounded functions do not execute the
game or prove their hooked CD/GPU service implementations.

Synthetic placement controls reject either object's incompatible alignment,
conflicting initialized/BSS bases, and equal but unclaimed initialized sections.
They also retain a positive case with unequal alignments that both permit the
actual base; numeric alignment equality is not a substitute for placement.

GAME reached source owners rise 74 to 79; unpaired config ranges fall 629 to
624 (GAME 357, OPEN 267). Source/header literal DAT_ occurrences fall 179 to
177. This is ownership progress and stricter verification, not whole-section
source closure or exhaustive reachable-byte coverage. The default full build
still fails the explicit data, reachability and target-placement gates.

All 564 local repository tests pass without skips; Ruff and diff checks pass.
`nix flake check -L` passes, including native compiler/assembler and objdiff
controls; its 83 skips require optional local retail/oracle artifacts that the
local run did exercise. Focused `kf try` builds retain 2/6 exact item functions
and 8/16 exact menu-runtime functions. The full graph recompiles all 112 source
units and regenerates all three comparisons; its nonzero result is the required
failure on unresolved data/placement/reachability, not a waived verification.
