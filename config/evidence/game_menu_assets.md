# GAME runtime-loaded menu assets

## Function Match Plan

Baseline: `e58b856`, `probe-gcc257-o2-g0`, GAME.EXE only. Recover the complete
912-byte first destination of `item_load_database` as one shared typed BSS
owner in `item.c`. Do not initialize BSS from disc bytes, create overlapping
interior globals, change signatures, or weaken data comparison. Preserve all
360 exact game functions, including the six exact consumers below.

The six semantic views (address, block disassembly, incoming/outgoing xrefs,
strings and strict match) were inspected for each function. All direct caller
windows and immediate neighbors were also inspected. Reproduction uses
`kf sema --image game` with each name below; local generated dossiers are
`build/menu-assets-evidence.log` and `build/menu-assets-callers.log`.

| GAME VA / body bytes | Function | Before % | Evidence and source hypothesis |
| --- | --- | ---: | --- |
| 80020cfc / 5dc | item_load_database | 99.746666 | No arguments; first aligned/unaligned copy loops transfer 912 bytes to 580e8; source then advances 912. Use the whole object and its sizeof. |
| 80022d7c / 400 | menu_map_viewer | 100 | Item code in a0; shared screen quads use 160-byte buffer stride, indices 3,2,1,0 at OT 3000. |
| 8002430c / 69c | menu_status_panel | 73.281330 | No arguments; four mirrored tiles read backdrop tpage/clut/width/height as halfwords and u/v as bytes. |
| 80025da0 / 198 | menu_config_panel_draw | 100 | Existing by-value ConfigPanelParams, including caller-stack state pointer; option descriptors, glyph atlas, four background quads. |
| 80025f38 / 5a0 | menu_draw_stats_header | 97.686110 | No arguments; glyph and number atlas addresses passed in a0, positioned text in a1. |
| 800264d8 / cb4 | menu_draw_status_details | 95.110700 | No arguments; same atlases, player halfword statistics and word experience/gold. |
| 8002718c / 838 | menu_draw_name_list | 100 | No arguments; eight optional item/spell names selected by byte IDs; glyph atlas. |
| 800279c4 / 1b8 | menu_item_model_preview | 98.181816 | Item ID in a0, ff empty sentinel; glyph and number atlases after model rendering. |
| 80027b7c / 2dc | menu_draw_item_detail | 91.284160 | ID/price column/buy-sell mode in a0-a2; atlases and row background, not an item-icon descriptor. |
| 80027e58 / 48 | menu_add_marker_quad | 100 | No arguments; AddPrim OT 500, 40-byte buffer stride from 58228. |
| 80027ea0 / 44 | menu_add_frame_quad | 100 | No arguments; AddPrim OT 0, 40-byte buffer stride from 58278. |
| 80027ee4 / 49c | menu_draw_dialog_frame | 98.623726 | Summary pointer/kind in a0-a1; 144-byte dialog-buffer stride, 24-byte quads, indices 2..4; three 24-byte summary rows. |
| 80028914 / 15c | menu_draw_window | 88.172420 | Four integer arguments; normal row descriptor 58424, confirmed row 58430 when flag=1, cursor 5846c, glyph atlas. Six literal address pairs lack relocation rows. |
| 80028a70 / 77c | menu_list_render | 88.252610 | KfMenuList pointer in a0; four 12-byte tile descriptors, byte u/v, halfword geometry, optional quantity source. |
| 800291ec / 10c | menu_draw_two_option | 100 | Two positioned text pointers and two integer selectors in a0-a3; normal/highlight backgrounds and selection cursor. |
| 800292f8 / 7b8 | menu_draw_item_name_frame | 68.518220 | Item ID in a0; backdrop address retained across four tiled packets; retail additionally enqueues four background quads missing in current source. |
| 8002a510 / 6a4 | menu_draw_window_backdrop | 58.192940 | No arguments; backdrop-relative background quad addresses; retail mirrors horizontal/vertical texture corners in three of four tiles, unlike current source. |

All are absent from the vendored function inventory. Their menu/resource
composition is game code; calls to SDK AddPrim, SetSemiTrans, matrix helpers,
PadRead and CdSearchFile remain vendor boundaries. Keep authentic LIBGPU.H
POLY_FT4 (40 bytes) and POLY_F4 (24 bytes), not replacement packet structs.
Only the loader and map viewer reference strings (`COM\\STAT.DAT`, item-model
and map paths); the other 15 functions use glyph codes, not ASCII literals.
History includes the prior menu bank/layout/tile campaigns `94d7e54`,
`001d8fd`, `237f691` and type ownership split `37c26d2`.

## Independent owner and layout evidence

`80020da4/80020da8` materialize 800580e8. Both copy paths end at source+912
and advance to the separately addressed menu-window bank at 80058478.
The old declaration covered only the first 320 bytes of this copy.

Independently, `menu_draw_item_name_frame` retains 80058400 in s2 at
80029438/8002943c, then derives 80058160, 80058138, 80058110 and 800580e8 by
subtracting 672, 712, 752 and 792 at 80029a04..80029a70. The shared backdrop
does the same at 8002ab08..8002ab74. Thus the quad array and later descriptor
have a common addressable owner; mere adjacency or a bulk copy is not the
sole justification. Every AddPrim delay slot finishes the indexed address.
The original aggregate spelling and original TU boundary remain WIP.

| Offset | Field | Type |
| ---: | --- | --- |
| 000 | background_quads | POLY_FT4[2][4] |
| 140 | mid_depth_quads | POLY_FT4[2] |
| 190 | foreground_quads | POLY_FT4[2] |
| 1e0 | dialog_quads | POLY_F4[2][6] |
| 300 | number_atlas | MenuSpriteDef |
| 30c | glyph_atlas | MenuSpriteDef |
| 318 | window_backdrop | MenuTileSprite |
| 324 | option_background | MenuSpriteDef |
| 330 | option_highlight | MenuSpriteDef |
| 33c | row_background | MenuSpriteDef |
| 348 | row_confirmed_background | MenuSpriteDef |
| 354 | list_tiles | MenuTileSprite[4] |
| 384 | selection_cursor | MenuSpriteDef |

The resource `KF/COM/STAT.DAT` is 5708 bytes, SHA-256
`3f51069ac6291bffdfeb981b14963a22564b40fa5d9034f226797f84247b97f4`.
Its first 912 bytes hash to
`a2e38066975460311d8cda7bb905573dfd3645f6aa2820f2fc824ee3c3295d42`.
This is secondary serialized-layout evidence, not executable initialized data.
The first twelve packets have nine-word DMA lengths and GPU command 2c;
the first eight cover the 320x240 screen in four 160x120 quads per buffer.
The next twelve slots are 24-byte flat quads (command 2a, five-word length),
except the all-zero slot at +270. Do not fabricate a duplicate for that slot.
The final twelve 12-byte descriptors fit exactly; number/glyph dimensions are
7x11 / 14x12, backdrop 71x104, option boxes 50x19, row boxes 132x19, and
cursor 12x16. Existing byte u/v views retain the opaque high bytes at +5/+7.

## Verification sequence

1. Introduce the owner/type and replace references without semantic changes.
   Keep every existing reviewed/candidate status; root rebinding preserves S+A.
2. Compare rebuilt functions and raw relocation targets first. Curate the six
   missing menu-window address pairs from their raw instructions and consumers.
3. Restore the independently evidenced item-preview enqueue calls and backdrop
   mirroring, then compare ordered calls, packet writes and retail delay slots.
4. Add owner/layout, relocation round-trip and bounded behavior controls; run
   focused matches, full build, repository tests/lint and diff checks. Record
   per-function final verdicts below; no fuzzy score is closure.

## Final verdicts

All 17 ordered direct-call sequences now agree with retail. The ownership-only
build preserved the six exact controls; restoring the four preview AddPrim
calls brought its sequence from 19 to 23. All 380 reviewed menu-asset address
pairs (374 existing and six newly curated) pass safe delinking and exact raw
HI16/LO16 restoration with the original target, including non-adjacent pairs.
No candidate evidence was promoted as a side effect of aggregate rebinding.

| Function | Final strict % | Verdict / first remaining evidence |
| --- | ---: | --- |
| item_load_database | 99.746666 | Unchanged non-exact. All six copies agree; later filename division uses different registers. |
| menu_map_viewer | 100 | Exact preserved. |
| menu_status_panel | 78.468090 | Improved, not exact. Frame 112 versus 48 bytes and subsequent instruction stream differ; call sequence agrees. |
| menu_config_panel_draw | 100 | Exact preserved. |
| menu_draw_stats_header | 97.686110 | Unchanged non-exact. Frame 72 versus 64 bytes; later player-stat branch/source form remains unresolved. |
| menu_draw_status_details | 95.110700 | Unchanged non-exact. First difference is constant placement across a load delay; later stat branches also remain unresolved. |
| menu_draw_name_list | 100 | Exact preserved. |
| menu_item_model_preview | 98.181816 | Unchanged non-exact. Final index shift moves across the name-bank address pair; referent and call sequence agree. |
| menu_draw_item_detail | 91.284160 | Unchanged non-exact. Saved column/mode registers differ first, followed by name-copy instruction order. |
| menu_add_marker_quad | 100 | Exact preserved. |
| menu_add_frame_quad | 100 | Exact preserved. |
| menu_draw_dialog_frame | 98.623726 | Unchanged non-exact. Summary pointer allocation and later row traversal differ. |
| menu_draw_window | 86.908040 | Honest drop from aggregate references; no banked regression. Six missing address pairs now represented. Additional saved register and signed lh versus source lhu title check remain unresolved. |
| menu_list_render | 88.252610 | Unchanged non-exact. Frame 96 versus 104 bytes, row-pointer allocation and later loop forms differ. |
| menu_draw_two_option | 100 | Exact preserved. |
| menu_draw_item_name_frame | 94.522270 | Improved, not exact. Missing enqueue calls restored; frame 224 versus 160 bytes and rotation-vector owner/reference form still differ. |
| menu_draw_window_backdrop | 99.971760 | Improved, not exact. Correct mirror orientation; only frame/save offsets differ (104 versus 40 bytes). No fake locals or padding added. |

These are observed residues, not attributed compiler mechanisms. Existing menu
coordinate signedness needs a separate caller-wide type review: the shared
header currently uses u16 coordinates, but menu_draw_window uses lh for its
zero test. Adding the existing menu structures to checked layout inventories
does not establish historical typedefs or settle that signedness. The two
opaque byte lanes in MenuTileSprite remain explicitly opaque.

`tests/test_game_menu_assets.py` checks the complete single owner and all 13
contiguous fields, both ELF NOBITS allocations and strict section comparison,
all 380 relocation pairs, both retail/source loader bodies at all four source
alignments, every byte in all six destination banks, and the untouched four-byte
gap before prices. Bounded retail/source render controls exercise both display
buffers, byte-wrapping UV coordinates, all four mirrored packets and ordered
background enqueue arguments. The item-preview control also checks copied name
codes and angle wrap. Explicit CD/GPU/matrix hooks isolate these boundaries;
they do not prove the hooked bodies or whole-menu execution.

The item owner's complete .rodata (40 bytes) and .bss (912 bytes) match. The
912-byte NOBITS extent is allocation/ownership matching, not a fabricated
executable payload; runtime-copy controls separately cover the loaded bytes.
All 360/471 exact game functions and all 13 vendor controls remain exact against
the pre-campaign report. No new function is banked. Source/header literal `DAT_`
occurrences fall 564 to 179 (grep hit lines: 509 to 172). Reached GAME source owners rise 73 to 74 and
config-only ranges fall 380 to 363; the full block absorbs 17 fragmented ranges.
Unpaired reached config ranges fall 646 to 629 (GAME 362, OPEN 267).

All 556 repository tests pass without skips; Ruff and diff checks pass. Flake
checks pass with 81 explicitly optional local-retail/tool skips. Focused builds
and the full comparison graph rebuilt the affected objects. The required full
`kf build` remains red: strict source data stays 11/59, SDK contributions 2/2,
and target relinking 108/114 with the same six placement conflicts. Reachability
still reports candidate paths, census/code overlaps and indirect-control gaps.
This campaign does not establish exhaustive reachable-byte coverage or linked
executable equality, and no gate was relaxed.
