# GAME menu glyph rendering

## Function Match Plan: number renderer

`GAME.EXE 0x8002a310 menu_draw_number`, 512 bytes, existing
`game.menu_runtime` WIP unit, `probe-gcc257-o2-g0`. Starting strict objdiff
98.359375%. The probe is not historical compiler attribution.

All six required image-qualified semantic views were read. Retail has four
blocks: initial signed-halfword -1 guard, nonempty preheader, glyph loop and
shared return. The frame is 40 bytes; the return restores it in the delay
slot. There are no strings, indirect transfers or candidate references.
Two proven calls delimit each quad: primitive begin at `0x8002a344` (nop
slot) and commit at `0x8002a4dc` (final V byte store in its slot), depth 1000.
Eight validated HI16/LO16 pairs refer to `current_poly_ft4`; together with
the calls they form 18 ordered ELF relocations. No inventory correction is
indicated. The authenticated Psy-Q `POLY_FT4` interface remains unchanged.

The signed code at label +4 selects an eleven-texel atlas row; the horizontal
origin advances seven pixels per code, while the glyph pointer advances two
bytes. `lhu` at label +0/+2 and atlas +0/+2/+8/+10 supports the existing
halfword fields; packet-only texture operations can use low-byte loads without
changing those source field widths. `lh` for termination and multiplication
supports `s16` codes. No input is written and no result is consumed.

All 47 proven call sites were inspected: stats header (10), status details
(24), item-model preview (1), item detail (3), save-dialog frame (8), and list
renderer (1). Every caller passes `DAT_800583e8` in a0 and its stack +16
positioned halfword glyph workspace in a1. Some calls draw explicit slash
glyph 11, others draw formatter output; the list renderer directly writes
quantity codes, glyph-10 blanking and a -1 terminator. These are game menu
policies, not SDK/ASCII formatting bodies. The target is absent from vendored
and FID inventories; the primitive helpers call the actual SDK providers.

Reviewed the preceding string renderer's corresponding initial guard and
preheader, the following backdrop's boundary/frame/API use, both primitive
callees, the shared header and source history (`37c26d2` and current master).
No signature or object-layout change is proposed.

The first and only existing raw instruction difference is the placement of
`move s3,zero`: retail performs it in the nonempty preheader, whereas the
independently maintained C `xoff` is initialized before the initial guard.
First hypothesis: derive the fixed-pitch offset as `i * 7` inside the loop,
instead of maintaining two independently initialized counters. This expresses
the observed index/offset relationship and allows the compiler to introduce
the corresponding induction value only on the nonempty path. Preserve all
packet accesses, calls and constants. If that does not close the function,
inspect the first divergence before considering an explicitly guarded loop.

Closure requires focused rebuild, canonical strict 100%, raw instruction and
ordered relocation agreement, all-image regression comparison, full build,
Ruff, repository tests and diff check. Bank only verified exact rows.

## Function Match Plan: string renderer

`GAME.EXE 0x80029de0 menu_draw_string`, 1328 bytes, same WIP unit/profile.
Starting strict score 99.325300%. All six required semantic views and the
complete retail body were read. A 48-byte frame surrounds a signed -1 glyph
guard, a nonempty preheader, the base glyph path, two independent decoration
paths, and one return. Code mask 0xfff selects a 14-by-12 atlas cell using
the retail signed /16 and %16 expansion. Bits 0x1000/0x2000 independently
draw overlays with origins (196,24)/(210,24), depth 1000. The return delay
slot restores 48 bytes; each commit slot writes the final V byte. The loop
tail advances the horizontal offset by 14 in its branch delay slot.

Six proven begin/commit calls and 24 validated `current_poly_ft4` HI16/LO16
pairs give 54 ELF relocations. No candidate references, strings or indirect
calls. All 63 proven callers were reviewed: config-panel draw (2), stats
header (10), status details (26), name list (8), item preview (2), item detail
(4), save-dialog frame (4), window (2), list render (2), two-option draw (2),
and item-name frame (1). They pass `DAT_800583f4` and either a stack-built
positioned glyph run, a typed menu-window title/row, or an incoming shared
label. The list renderer also uses its compatible title prefix. Signed
halfword termination, unsigned halfword origin fields, read-only inputs and
void result remain supported; the authentic SDK packet type is unchanged.

Neither this target nor the number renderer appears in provider/FID matches.
Atlas/glyph/decoration policy is game-owned; the exact primitive helpers retain
the SDK boundary. Both neighbors (opaque blit and number render), shared source
history and the header were inspected.

First hypothesis: apply the independently observed fourteen-pixel index
relationship directly (`x_offset = i * 14`) inside the glyph loop, as in the
newly exact number renderer. This tests the early offset/font-save preheader
difference without changing the decoration arithmetic. Four other words differ:
retail adds -60/-46 before byte stores where the source adds 196/210. Their
equal low bytes alone do not justify inventing signed texture types or replacing
constants just to bank a score. Keep that residue separate from the loop test.

## Final verdicts

`menu_draw_number`: **100%**, up from 98.359375%. The first focused source
hypothesis closed the function: `xoff = i * 7` in the loop reproduces the
nonempty-path initialization without adding another explicit guard. All 128
instruction words and all 18 ordered relocations match exactly, including
the original loads, packet writes, calls, constants, frame and delay slots.
No changes to SDK types, data identities or relocation inventories were needed.

`menu_draw_string`: **99.987950%**, up from 99.325300%, not exact and not
banked. The independently checked `x_offset = i * 14` relationship reproduces
the entire initial guard/preheader and preserves all 54 ordered relocations.
Of 332 instruction words, only these four still differ:

| Relative offset | Retail word / immediate | Compiled word / immediate |
| --- | --- | --- |
| +0x34c | `2442ffc4`, -60 | `244200c4`, 196 |
| +0x394 | `2442ffc4`, -60 | `244200c4`, 196 |
| +0x494 | `2442ffd2`, -46 | `244200d2`, 210 |
| +0x4dc | `2442ffd2`, -46 | `244200d2`, 210 |

All four are `addiu v0,v0,constant` before a byte store to the decoration's
right-hand U coordinate. Calls, CFG, instruction positions, other raw words,
load widths, data identity and ordered relocation referents agree. The low
bytes agree modulo 256, but this is still an unattributed instruction-selection
residue, not exactness or a proved compiler mechanism. No signed carrier,
packet-type replacement, arithmetic constant change or inline assembly was
introduced to erase it.

The related sprite blitters were inspected as neighboring evidence and remain
unchanged; their negative-halfword immediate selection is a separate campaign.

Final verification: focused rebuild and canonical `kf match` completed;
all 484 function-report rows compared by image/name/size/score, with only the
two changes above and no regressions or losses. GAME 277/362 -> 278/362 exact,
the unit 10/16 -> 11/16, and all images 375/471 -> 376/471. Full `kf build`
remains nonzero for existing non-code gaps: source-data 11/59, config SDK data
2/2, target relink 108/114 and incomplete known-reference ownership. Ruff,
diff check and all 551 repository tests pass (49.124 s). Only the newly
strict-exact `menu_draw_number` row is selected for banking.

## Retail glyph format and asset evidence

The naming pass decodes the retail menu texture without running the game.
`item_load_database` copies the first 0x390 bytes of `KF/COM/STAT.DAT` to
`menu_assets`. Its descriptors at file offsets 0x300 and 0x30c contain the
following six little-endian halfwords:

| Atlas | Texture page | CLUT | U | V | Width | Height |
| --- | --- | --- | --- | --- | --- | --- |
| Number | 28 | 31872 | 240 | 0 | 7 | 11 |
| Text | 28 | 31872 | 0 | 0 | 14 | 12 |

`common_resources_load` uploads the consecutive TIM records in
`KF/COM/MIX.TIM`. The fifth record starts at file offset 231552 (flags 8,
four bits per pixel). Its image payload starts at 232096 and contains
32768 bytes: 64 VRAM words by 256 rows at (768,256), equivalent to a
256-by-256 texel page. Its selected 16-color CLUT is the first 32 bytes at
231572, uploaded to (0,498). These agree with texture page 28 and CLUT 31872
in both descriptors. Decode each little-endian word's four palette indices
from low to high nibble. The selected page and palette are complete payloads;
no behavior is inferred from the unrelated palette rows extending past
VRAM row 511 elsewhere in the file.

The following cells are visible in that decoded page:

| Encoding | Cell / meaning |
| --- | --- |
| Text 0x2e, UV (196,24) | Dakuten, the two-stroke kana voicing mark. Text flag 0x1000 overlays this cell. |
| Text 0x2f, UV (210,24) | Handakuten, the circular kana mark. Text flag 0x2000 overlays this cell. |
| Text 0xff, UV (210,180) | Empty text cell, retained as spacing in status labels. |
| Number 10, UV (240,110) | Empty number cell, selected for leading padding. |
| Number 11, UV (240,121) | Slash used between current and maximum HP/MP. |
| Text 0x88 | The character `毒` (poison), also used beside `player_state.poison_resistance`; see the [status evidence](game-player-poison.md). |
| Text 0xc5, 0xc6 | `正常`, the normal-status label. |

Text layout has sixteen columns of 14-by-12 cells. Number codes select rows
eleven texels high at U=240, advancing seven screen pixels per code. These
are measured asset geometry, not unexplained tuning aliases. Low twelve code
bits select the base text cell; the two independently tested flag bits add
marks over the same screen rectangle. For example, authored label code
0x104c is `MENU_TEXT_DAKUTEN | 0x4c`, producing `ず` from `す` in `はずす`.
Character indices stay asset data; they are not Unicode values.

Reproduction identities (SHA-256):

- `KF/COM/STAT.DAT`, 5708 bytes:
  `3f51069ac6291bffdfeb981b14963a22564b40fa5d9034f226797f84247b97f4`.
- `KF/COM/MIX.TIM`, 331488 bytes:
  `9e1031c32ea9efc2d124ae97dd6f2a0291bc88153b74b8c1d57f99abefd55959`.

The atlas proves the mark origins' low-byte values; it does not resolve the
four raw immediate discrepancies described above. The naming change retains
positive 196/210 arithmetic and keeps `menu_draw_string` partial.
