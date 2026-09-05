# GAME projection parameter countdown

## Function Match Plan

Starting at `7eb6c4b`, inspect the two non-exact projection loops in
`game.render`. Both use the same selected vertex cursor, eight-byte output
entries and two SDK services. All six required semantic views for each
function, all ten incoming call sites, adjacent release/transform functions,
SDK bodies and headers, source history and the OPEN counterparts were read
before editing. Twelve of the eighteen functions in the unit are exact.

`GAME.EXE 0x8001c60c tmd_project_vertices(s32 count)` occupies 156 bytes,
strict 98.846150%. Nine call sites pass the object's word-sized vertex count.
The 64-byte frame holds four SDK output words; the zero-count branch owns
the initial count-minus-one instruction. The loop uses a signed word
countdown ending at -1, forward eight-byte strides, doubled low-halfword
perspective and low-halfword depth stores. Its first differing word is the
countdown destination (`s0` in retail, `s1` in the probe); the output-depth
pointer takes the opposite register. Other instructions and referents agree.

`GAME.EXE 0x8001c6a8 tmd_project_vertices_shift(s32 count, u8 shift)` is
172 bytes, strict 98.953490%. Its sole caller `render_weapon` passes a word
vertex count and shift 3. The same 64-byte frame, zero guard, countdown and
pointer strides apply. It masks the shift to eight bits, loads full signed
depth and arithmetic-shifts before the halfword store. The first discrepancy
is the same countdown/output-depth pointer register exchange.

Each function has no strings or candidate outgoing references. Two validated
HI16/LO16 pairs name `tmd_projected_vertices` and `current_tmd_vertices`.
The two proven calls are `RotTransPers` and `ReadSZ2`; their delay slots
carry the flags pointer and perspective store. The return delay slot pops
64 bytes. No call, output width, SDK-local extent or delay-slot dependency
will be changed.

The loops are game-owned traversal, not SDK progress. `ReadSZ2` belongs to
the exact Release 2.5 `LIBGTE/REG` section at member offset 0x1ac; its retail
body writes only the first of the two pointers passed by GAME. The genuine
second argument storage and compatibility declaration remain. `RotTransPers`
has independently supported earlier `LIBGTE/SMP` lineage, not an exact
Release 2.5 attribution. Its authentic header signature uses `SVECTOR *`
and three `long *` outputs.

Retail converts the incoming count into its sole countdown at entry; it
does not retain the original count. The exact OPEN shift sibling uses
`for (count--; count != -1; count--)` with that same source lifetime. Test
this direct parameter countdown in GAME, removing the extra `remaining`
local while preserving all body operations, signed count type, zero-input
behavior, widths, ordering, declarations and compiler flags. This is a
source-lifetime hypothesis, not a claim about an allocator mechanism. Do
not permute local declarations or force registers. Compare each focused
build from the first real divergence, preserve the exact transform sibling
and all other banked functions, and bank only strict 100% results after the
full build and existing checks.

## Final verdicts

Changing only the first loop to the parameter countdown makes
`tmd_project_vertices` exact in the focused comparison. Applying the same
source-lifetime change to the second loop makes `tmd_project_vertices_shift`
exact as well. The recorded rebuild confirms strict objdiff **100%** for
both: 156 bytes / 39 raw words and 172 bytes / 43 raw words respectively.

All raw words agree, including the initial decrement, output-depth pointer,
loop branch and delay slots. Each function's six ordered relocations agree
in relative offset, kind and target: two HI16/LO16 pairs followed by
`RotTransPers` and `ReadSZ2` calls. Both SDK output contracts and every body
operation remain unchanged. No type, ownership, function signature, SDK
implementation, compiler flag or OPEN source was modified.

`game.render` advances from 12/18 to **14/18 exact**. Its four remaining
partial functions retain their scores, and the non-perspective transform
retains 100%. All other 482 function report rows across the three images,
including 13 SDK source controls, are preserved. GAME advances
**265 -> 267 / 362 exact**, leaving 95 partial functions. Only
`GAME.EXE 0x8001c60c` and `GAME.EXE 0x8001c6a8` are selected for banking.

Ruff, all 551 existing repository tests (55.642 seconds) and
`git diff --check` pass. Full `kf build` retains the known 48 divergent
source data-owning units, incomplete ownership and six conflicting section
bases. There are no new match or data-comparison regressions. The separate
TMD preparation countdown/frame difference was inspected but not edited;
these two exact results do not close that function or establish a compiler
mechanism for its remaining differences.
