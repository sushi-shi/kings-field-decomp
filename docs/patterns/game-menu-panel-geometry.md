# GAME list, status-panel and map geometry

## Function Match Plan

Captured worktree at `03659b90ff266be9bcd163e70ae88d4e9f88e587`; image **GAME.EXE**, pinned `probe-gcc257-o2-g0` profile. All three units retain their existing source ownership and signatures.

The six semantic views, current source, history and neighboring menu callers/renderers were reviewed before edits. Calls remain proven direct transfers; data references retain their validated classification. Preserve branch/return delay slots, exact arithmetic grouping, signed list guards, the unsigned quantity division and draw order. Game menu/resource consumers distinguish these bodies from vendored GPU, pad and memory helpers, whose SDK calls remain external.

| Function | GAME VA / bytes | Strict baseline | Retail calls / conditional branches / returns | Source hypothesis |
| --- | --- | ---: | --- | --- |
| `menu_map_viewer` | `80022d7c / 1024` | 100.000000% | 17 / 9 / 1 | Name map-set selectors, RGB, packed resource locations, crop span and origins; preserve all packet assignments and input waits. |
| `menu_status_panel` | `8002430c / 1692` | 81.900710% | 20 / 5 / 1 | Share the verified 71×104 backdrop grid spacing with runtime; retain authored left origin 6 and all mirrored UV reads. |
| `menu_list_render` | `80028a70 / 1916` | 100.000000% | 14 / 13 / 1 | Name text inset 3, row pitch 12 and quantity offsets 110/1; preserve exact loop and coordinate-expression shape. |

The map and list renderers are strict exact controls. The status-panel loop
remains partial and must preserve its baseline. This is naming and literal
accounting, not a new exact reconstruction. Require forced affected builds,
all-variant isolated runtime/relocation comparison, exact retail word checks,
modern diagnostics, inventory, Ruff, repository tests and full build.

## Geometry evidence

The [runtime review](game-menu-runtime-constants.md) establishes the shipped
STAT.DAT backdrop descriptor at offset 0x318 as 71×104 pixels. Status origins
X 6/77 and Y 16/120 use the same grid as the runtime's other panels. Promote its
column/row step and top/bottom constants to the shared menu header; keep each
panel's horizontal origin local. Fixed grid positions remain separate from
runtime descriptor width/height reads for quad edges.

The four list-tile records at STAT.DAT offset 0x354 decode as follows. All use
tpage 27 and CLUT 31808, U 0 and width 132; the mutable descriptor still supplies
packet dimensions at runtime.

| Tile role/index | V | Height |
| --- | ---: | ---: |
| backdrop/0 | 208 | 4 |
| row/1 | 214 | 12 |
| end/2 | 228 | 4 |
| selected/3 | 194 | 12 |

Retail list text and row tiles advance 12 pixels; the end tile begins after
`visible_rows` pitches. The three-pixel inset aligns the text and row tiles
inside the list origin. Quantity text moves 110 pixels right and one down;
the numeric atlas height 11 fits the text atlas/list height 12 with that vertical
adjustment. These facts name the geometry; they do not establish an aesthetic
reason for the 110-pixel quantity column.

Quantity division remains unsigned decimal 10 (`DIVU` at `80028bac`), with both
quotient and remainder captured before the workspace stores. The
[exact list reconstruction](game-menu-list-control-flow.md) established the
shared glyph object, source-pointer advance, signed row guards and parenthesized
bottom-edge additions. None of those source facts changes.

All ten MAP files M11..M15 and M21..M25 were rechecked: each is 33,312 bytes,
with palette rectangle (0,501,16,16) and image rectangle (960,256,64,256) in VRAM
words. Thus the 4-bit image is 256×256 texels. M11 SHA-256:
`62f7caf0d4396df5896fa6f5bc0f6c2abcc4cd900f40dbc8b084fdd64a3cd335`.
The packet's coordinate span 219 covers its authored crop, from UV 0 to 219,
and screen (50,10) to(269,229); it is not the full resource dimension.

COM/MIX.TIM image 3 starts at byte 198240, with palette rectangle (0,497,16,16)
and image rectangle (704,256,64,256). Its whole-file SHA-256 is
`9e1031c32ea9efc2d124ae97dd6f2a0291bc88153b74b8c1d57f99abefd55959`.
The map's packed CLUT 0x7d40/tpage 0x1f and marker's 0x7c40/0x1b select those
resource locations. RGB 64 is shared texture modulation, independently of the
map's semi-transparency call. The marker starts at (58,216), spans 4 coordinate
units and moves(+2×cell_x,−2×cell_z). Derive its opposite edges from that span;
retain the authored origin rather than claiming a new gameplay coordinate.

Map set 1 is selected only for `KF_ITEM_WATCHMAN_MAP`; every other input uses
set 2. Name the second branch as the default set to preserve this wider input
behavior. Path character positions, ASCII digit conversion, buffer indices,
SDK Booleans and empty-input tests remain literal with explicit ledger reasons.

## Final verification

Eighteen new private constants and four existing backdrop constants moved to
the shared header name 82 inline uses: map 32, status-panel 32, list 18. The
[refreshed panel ledger](game-menu-panel-literal-ledger.md) accounts for all
103 retained occurrences (75/12/16), replacing the old list workspace and
control-flow assumptions. The whole-source census is 6,903 occurrences;
current token/expression accounting is verified for 23 files / 2,012 occurrences.
These totals do not measure the number of names still missing.

All four affected units were forcibly rebuilt. `kf build compare` passed.
Before/after isolated compilation of 112 source/image variants preserves every
allocated section, its alignment, runtime symbols and ordered relocations.
All live objects equal the isolated controls; only the four edited C files'
debug line sections change. All 484 strict function scores are unchanged.
The captured concurrent status-text work is identical in both controls;
no unrelated object or score was excluded from comparison.

The three reviewed functions preserve 1,090 resolved instruction words,
51 ordered calls and 115 address pairs. Both exact functions additionally
match all 735 retail words and their delinked target objects. Final verdicts:
map 100%, list 100%, status panel 81.900710%, unchanged from their baselines.
The status-panel first difference remains its entry stack allocation:
candidate 48 bytes, retail 112 bytes at GAME `8002430c`. This is an
unattributed source/codegen residue; no compiler mechanism is asserted.
No new exact result or banking is claimed.

Inventory, Ruff and `git diff --check` pass. All 680 existing tests pass
(94.064 seconds), with no tests added. Modern checking retains exactly the
same 300 diagnostics and 65/112 passing variants. Full `kf build` retains the
existing data ownership/section-placement and relink failures: source-data
PSX 0/1, GAME 9/42, OPEN 2/19; target relinks PSX 1/1, GAME 75/77, OPEN 34/38,
with six conflicting-section-base reports and no artifact failures.
