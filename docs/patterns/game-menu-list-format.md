# GAME menu list and decimal-glyph helpers

## Function Match Plan

Image: `GAME.EXE`; existing WIP unit `game.menu_runtime`. The helpers are
related by the menu glyph/list API, not merely by adjacent addresses. The
`probe-gcc257-o2-g0` probe is not evidence of the original compiler. Strict objdiff 100% plus raw
instruction and ordered relocation agreement is the closure condition.

### `menu_list_init` — `0x8002ad6c`, 140 bytes

Starting strict score: 93%. Three retail blocks, one ten-halfword copy loop,
no frame, calls, strings, or indirect transfers. One validated HI16/LO16 pair
at `0x8002ad98/0x8002ad9c` refers to `menu_window_layouts + 0x1c`; no candidate
references. `lhu/sh` copies ten codes; the loop branch advances the destination
by two in its delay slot. The return delay slot stores glyph width 8 at +31.
Other constants are title origin 12/19, list origin 22/38, eleven visible
rows, and zero count/scroll/selection/cursor bytes. Pointer fields are untouched.

All seven proven callers pass their stack +24 list object and do not use a
result: buy at `0x80021580` selects (7,0), sell at `0x80021b44` selects (7,1),
use-item at `0x80022648` selects (0,0), magic at `0x800231bc` selects (0,1),
equipment at `0x80023a80` selects (1,category), spell at `0x80023fac` selects
(1,1), and drop at `0x800249e8` selects (0,4). The signature remains
`void (KfMenuList *, s32, s32)`; full-width index arithmetic and the shared
40-byte context are supported by caller stores and renderer reads.

The `item_load_database` copy is exactly 2376 bytes, nine 264-byte layouts;
`menu_draw_window` strides 264 bytes per window and 24 per positioned label.
See `config/evidence/game_semantic_menu_window_layouts.tsv`. Thus the old
`DAT_80058494[row * 33 + column * 3]` eight-byte-cell alias is precisely
`menu_window_layouts[row].rows[column].codes`. Its four-halfword subarray is
not the true ten-code object. Remove that sole-consumer alias, retaining the
existing aggregate identity and the relocation's absolute target/addend.

First hypothesis: read the typed codes directly in the indexed copy loop.
Retail initializes its real loop counter before computing column/row offsets;
the current cached source pointer computes those offsets before initialization.
The first divergence follows the four title stores/setup instructions. Do not
invent register carriers or separate overlapping data objects.

### `menu_format_number` — `0x8002adf8`, 172 bytes

Starting strict score: 83.488370%. Fourteen decoded blocks including checked
signed-division traps, an eight-byte empty frame, no calls, strings, external
references, relocations, or indirect transfers. Inputs are signed value/count,
a word-sized padding flag, and mutable halfword glyph output. Pad zero selects
blank glyph 10; other flags select glyph 0. Fill skips for count <= 0, but the
`out[count] = -1` store still executes in the digit-loop entry branch slot.
Decimal div/rem is signed, with divisor 10 and the toolchain's checked `div`
expansion. The return restores the eight-byte frame in its delay slot.

The 41 proven calls come from notification update (1), stats header (8),
status details (22), item-model preview (1), item detail (3), and save-dialog
frame (6). All 41 raw call sites and their source uses were reviewed: every
padding argument is zero, counts are 1/2/4/6, and destinations are halfword
stack arrays (usually `MenuGlyphString.codes`). Values arrive from byte,
halfword, and word fields, or signed arithmetic results; no caller consumes a
return value. The body, not the always-zero callers, proves nonzero padding.

First hypothesis: replace the early `break` with `i = -1` on zero quotient.
Retail explicitly sets the index to -1 and then executes the ordinary
decrement/test tail; its corresponding strength-reduced output pointer is
reset relative to the original output base. No additional output store occurs.
After separately checking that change, inspect the remaining fill preheader:
retail initializes the loop counter before selecting the invariant blank.

## Shared evidence and controls

Read image-qualified addr, block disassembly, incoming/outgoing xrefs, strings,
and match for both helpers. Reviewed both neighbors (quad commit and item-model
load), shared loader/renderer, and original utility source/history before edits.
Neither helper is in vendored/FID inventories. Game-specific list geometry,
glyph 10 padding and halfword -1 terminators distinguish these bodies from SDK
copy/ASCII formatting routines; neither contains a library implementation.

Rebuild after each source hypothesis, then canonical focused match, full build,
raw word/relocation comparison, all-image function regression comparison, Ruff,
repository tests, and diff check before banking only exact rows and committing.
The existing source-data/relink gaps remain separate from function closure.

## Focused results and final verdicts

`menu_list_init`: **100%**, up from 93%. The typed indexed copy reproduces
all 35 instruction words, including the original column-before-row preheader
and `a3` loop counter. Both ordered relocations agree at relative +0x2c/+0x30,
HI16 then LO16 for `menu_window_layouts`, with low addend +0x1c. Corrected the
stale curated relocation symbol without moving its absolute target. Removed
the obsolete `MenuLabelCell` type and extern; no new data identity or source
DATA claim was added. Compile-time checks guard the existing 24-byte label,
264-byte window, and first row's codes at +0x1c.

`menu_format_number`: **100%**, up from 83.488370%. All 43 words match,
including the checked signed divide, conditional index/pointer reset, loop-tail
delay slots and eight-byte frame. There are no relocations. The signature and
halfword output remain unchanged. The kept source has a real index initialized
to zero, a signed word-sized padding temporary, and `i = -1` on exhaustion.
Word arithmetic in the retail padding calculation supports computing in `s32`
and narrowing only on output; halfword stores alone did not establish a short
temporary. Exact output supports this reconstruction, not historical proof of
the original local declaration or compiler backend mechanism.

Controls, each focused-built and compared from its first divergence:

- Replacing only `break` with `i = -1` reproduced the entire digit loop and
  reached strict 90.465120%; the remaining difference was the fill preheader.
- Moving the padding conditional into each fill store emitted branches in the
  loop and an internal jump relocation, strict 76.279070%; rejected.
- Initializing the index before the existing short padding temporary emitted
  exactly the same preheader residue; not a closure by itself.
- Explicit if/else assignment to the incoming padding argument emitted an
  extra branch/jump and changed frame/entry control flow; rejected.
- A word-sized padding temporary on the initialized-index source reproduced
  the five retail preheader instructions and closed the function.

The `kf try` textual similarity percentages are not strict objdiff scores.
Its first typed-list run compared against an old alias-named target; canonical
`kf match` regenerated the owner-aware target and verified the preserved
address addend. All exact claims above use the canonical report plus raw words.

Final verification after adding the layout guards:

- Canonical objdiff: both helpers 100%; unit 8/16 -> 10/16 exact, GAME
  275/362 -> 277/362 exact, all images 373/471 -> 375/471 exact.
- Raw comparison: 35/35 and 43/43 words equal, with the same two ordered
  initializer relocations and no formatter relocations. The layout guards
  leave both raw disassemblies unchanged.
- All 484 reported function rows compared by image/name/size/score: only
  these two scores changed; the other 482 rows, including OPEN/PSX and
  vendored verification controls, are unchanged.
- Ruff and `git diff --check` pass. All 551 repository tests pass (49.275 s).
- Full `kf build` remains nonzero for pre-existing non-code closure gaps:
  source-data comparison 11/59, config SDK data 2/2, target relink 108/114,
  and incomplete known-reference data ownership. No gate was weakened and
  this campaign does not claim data or full-image closure.

Only the two newly strict-exact helper rows are selected for banking. No
unattributed residue remains in these bodies; six other menu-runtime functions
remain partial and the existing unit's historical boundary is still WIP.
