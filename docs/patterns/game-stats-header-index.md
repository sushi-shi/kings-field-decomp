# GAME statistics header: shared glyph-index lifetime

## Function Match Plan

GAME `80025f38 menu_draw_stats_header` owns 1,440 retail bytes and starts at
strict 97.686110% under the unchanged `probe-gcc257-o2-g0`. Before editing,
refresh the six image-qualified semantic views against hash-verified retail,
the complete CFG/disassembly, all three caller sites, adjacent functions,
glyph/number interfaces, shared layouts and source history. The caller
`menu_root` passes no arguments at `8002238c`, `800223ec`, and `800225b4`
and consumes no result. The preceding configuration draw helper ends at
this entry; the following detailed status renderer begins at `800264d8`.

The body has 28 proven direct calls, 36 validated HI16/LO16 address pairs,
two validated internal jumps, nineteen blocks, nine conditional branches
and one shared return. There are no strings, candidate outgoing references,
indirect transfers or unresolved graph edges. Glyph codes are signed
halfwords in the shared `MenuGlyphString`; X/Y are unsigned halfwords.
The frame is 72 retail bytes versus 64 in the 1,428-byte candidate, and its
release is the return delay slot. Complete ordered numeric calls and data
targets already agree. First raw divergence is the frame instruction.

The custom player-statistics, title-atlas and status-icon policy is game
code, not a vendored SDK body. Its three callees are the game glyph renderer,
number renderer and decimal formatter, not copied library implementations.
The [class-title review](game-menu-class-titles.md) supplies the atlas and
threshold evidence; history includes its naming change `c05548a`, the
shared menu owner and the original header reconstruction `52e9c9b`.

First source hypothesis: one word-sized glyph index is reused for the
class-title calculation and the later right-to-left status-icon positions.
Retail computes the class offset in `$a0`, using `$v1` for the two loaded
stats. Later, the status index is also in `$a0`; the class value is dead
before the intervening calls, and the status phase initializes its value
before use. This register reuse motivates a source-lifetime control; it
does not prove a recovered original variable name or declaration.

Test the single consumed index without changing the arithmetic, predicates,
glyph stores, reads, call order, object ownership or compiler options. Do
not introduce a row-spacing carrier, padding or forced register. Compile
the entire candidate, inspect changed raw words and all ordered targets,
then record strict objdiff and run the full project checks. Partial
improvement is not eligible for banking.

## Result and retained source

The single `s32 glyph_index` replaces `class_glyph_offset` and `i` in this
function only. It first selects the class row/column and four-glyph offset,
then is reset to four for status placement, or three when the slowed icon
already occupies the rightmost slot. Both phases retain all previous values
and stores. The analogous detailed status renderer was unchanged in this
header pass; its subsequent control is recorded below.

Exactly eighteen candidate words change, at these relative byte offsets:

```text
248 24c 258 260 268 270 274 278 27c
284 28c 29c 2a0 2a4 2a8 2b0 2b8 2c0
```

All are the class calculation's `$a0`/`$v1` operand choices, now matching
retail's corresponding instructions. Every other candidate word is unchanged.
The complete ordered sequences of 28 numeric calls and 36 materialized data
addresses equal both the original candidate and retail. Re-linking the
delinked target independently reproduces every retail word, SHA-256
`7d1673885f8227cac1a0c764545afa312b56d1260f3ca8bc5f7e2c1664bf8a0e`.
A separate fresh canonical compile equals the isolated trial and recorded
live object's complete text.

Strict objdiff improves **97.686110% -> 97.991670%**. The candidate remains
1,428 bytes, with nineteen blocks, nine branches, two internal jumps and
one return. Known successor lists and ordered control operands agree;
this is not a semantic-equivalence proof.

## Remaining difference

Retail places `li s4,23` in the first class-guard delay slot at `8002619c`
and reuses it in the subsequent Y advances. The candidate emits immediate
additions instead and schedules the next `sltiu` in that slot. Retail also
uses `$v1` for the preceding X=237 store, where the candidate uses `$v0`.
The retained X=251 value uses `$s5` in retail versus `$s4` in the candidate;
the differing saves and frame remain. No compiler mechanism or historical
compiler identity is established by these observations. This function is
still partial and is not banked.

## Verification

The focused `kf match` really delinks and recompiles the unit. Full
`kf build` and a 484-row score comparison show only this function's score
movement; no exact function regresses. Non-vendored counts remain GAME
313/362, OPEN 98/108 and PSX 1/1, with all thirteen vendored controls exact.
Concurrent menu-runtime naming work is excluded from this campaign.

All 680 existing repository tests pass in 84.891 seconds; Ruff and
`git diff --check` pass. No tests, toolchain options, shared headers,
inventory claims or banking entries change.

The full build remains non-green on existing data/ownership/placement gates:
source data PSX 0/1, GAME 9/42, OPEN 2/19; target relink PSX 1/1, GAME 75/77,
OPEN 34/38, six conflicting section bases and zero artifact failures. These
failures remain part of the wider reconstruction work, not waived checks.

## Detailed status follow-up

### Function Match Plan

GAME `800264d8 menu_draw_status_details` starts at strict 98.161130%, after
the [signed-rating correction](game-status-rating-arithmetic.md). Retail and
candidate both contain 3,252 bytes, an 80-byte frame, 31 blocks, 16 branches,
two internal jumps and one shared return. The six image-qualified views,
complete disassembly/CFG, caller, three callee implementations, adjacent
functions, shared types and source history were inspected before editing.
`menu_status_panel` calls at `80024350` without arguments or a consumed
result; the following name-list renderer is exact. The same custom
player-statistics/atlas policy excludes vendored ownership. There are 72
proven calls, 89 validated HI16/LO16 pairs, no strings and no unresolved
indirect transfers. The target relinks to all 813 retail words, SHA-256
`f210314ce339a8c9c3fd1f3e81596d2e4e3b8fb0755a971ce57b9582d3e9c2aa`.

Test the same shared glyph-index lifetime as the header, keeping the real
signed attack subtotal separate. Retail's class index is in `$a0`, with
the two base-stat loads in `$v1`; the later status-slot index also uses
`$a0`. The class value is dead before the status phase reinitializes it.
This supports a single consumed index as a source hypothesis, not proof of
the original declarations. Preserve all widths, predicates, stores, calls,
referents, frame and compiler options. The first existing raw difference
is `+3c`, where retail materializes Y=35 and the candidate emits a nop.

### Result and negative control

One `s32 glyph_index` replaces `class_glyph_offset` and `i`; `attack_rating`
is unchanged. Exactly eighteen words recover the class calculation's retail
register roles, at these candidate-relative offsets:

```text
2fc 300 30c 314 31c 324 328 32c 330
338 340 350 354 358 35c 364 36c 374
```

The other 795 candidate words are unchanged. Complete ordered sequences
of 72 numeric calls and 89 data addresses equal both the original candidate
and raw retail. Fresh canonical compilation equals the trial's complete
text and relocations, except for one `.debug_line` relocation offset: its
24-byte displacement is exactly the difference in source-path length.
The canonical and recorded live objects agree in full text and all
relocations. This debug exception is checked explicitly, not generalized
into ignored relocation differences.

A separate arithmetic control spells the existing multiplication and
division as `attack_rating *= 10; attack_rating /= 8;`. Its text and every
relocation equal the retained single-expression trial; it is not adopted.

Strict objdiff improves **98.161130% -> 98.296430%**. Size, frame and known
successor lists remain unchanged. The first ordered branch-operand residue
is still the signed attack guard, using `$a0` in retail versus `$v1` in
the candidate. Initial Y materialization, retained 16-pixel spacing,
blank/glyph constant placement and other rating-register differences remain.
No missing early return or compiler mechanism is inferred. The function
remains partial and is not banked.

The full 484-row comparison shows only this function's score movement and
no exact regression: GAME 313/362, OPEN 98/108, PSX 1/1, with all thirteen
vendored controls exact. Full `kf build` retains the same data/ownership/
placement failures documented above, with zero artifact failures. Concurrent
menu naming/layout changes are outside this source-and-evidence campaign.
All 680 existing repository tests pass in 83.512 seconds; Ruff and
`git diff --check` pass. No shared header, tooling, inventory or banking
change belongs to this follow-up.
