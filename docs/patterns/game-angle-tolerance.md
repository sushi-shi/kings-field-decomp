# GAME circular-angle tolerance predicate

## Function Match Plan

At `1c91b7e`, GAME `0x80014f6c angle_within_tolerance` owns 60 retail
bytes and is strict objdiff 82.333336%. Its other 15 `game.matrix_rotation`
functions are exact. Before editing, all six semantic queries, the two
adjacent math leaves, all twelve incoming call-site argument/result sequences,
the shared declaration, source history and OPEN counterpart were inspected.

The body is a frameless leaf with no calls, data references, relocations,
strings or candidate outgoing references. It masks the angle subtraction to
12 bits, explicitly sign-extends its third argument to 16 bits and performs
signed comparisons. The result is a full-register 0 or 1 copied from `v1`
to `v0` in the return delay slot. The first branch clears `v1` in its delay
slot; the second branch has a NOP slot. No SDK/GTE wrapper or runtime-library
signature is present, and the function is absent from the vendored census.
The shared-angle row in `overlay_lineage.tsv` independently records all 15
words as identical to OPEN `0x80015c98`.

The callers pass signed rotation fields or computed angle values, and a
signed tolerance or constants 0x155, 0x18e, 0x1c7 and 0x200. All consume the
result as a predicate. The existing `int, int, s16 -> int` C interface agrees
with this O32 evidence; no interface or caller needs to change.

Retail's first branch reaches `li v1,1` when `delta <= range`; otherwise it
tests whether `delta >= 0x1000 - range` and reaches the same true block.
The current C instead uses `range < delta && 0x1000 - range <= delta`.
This is a semantic error, not merely a different result-materialization
sequence: at equal angles with tolerance 0x155 it returns 0, while retail
returns 1. The exact OPEN source uses the two inclusive conditions joined
with `||`. Correct GAME to that directly decoded expression, retaining all
types, constants, function ownership and compiler settings.

Rebuild the complete unit and compare from the first branch divergence.
Require strict objdiff 100%, all 15 raw instruction words (including delay
slots), and preservation of all fifteen exact siblings. Run the full build
and existing checks; bank only this newly exact function. The historical
`source-shapes-gcc257.md` claim of an unexplained result-materialization
residue must be superseded by this CFG finding.

## Final verdict

The corrected two-arm predicate reproduces all 15 retail instruction words,
including both conditional branches, their delay slots, the true block and
the return-delay result copy. Fresh `kf try` and recorded strict objdiff
agree: all **16/16 functions in `game.matrix_rotation` are exact**. No
compiler option, shared type, caller, relocation or OPEN source was changed.

GAME advances **263 -> 264 / 362 exact**, leaving 98 non-exact functions.
Only `GAME.EXE 0x80014f6c` is selected for banking. The complete report
comparison preserves all other 483 function rows across the three images,
including 13 vendored source controls. The full build retains the existing
48 divergent source data-owning units, incomplete known-reference ownership
and six units with conflicting section bases; none is introduced by this
correction. This closes a real predicate error rather than trading source
semantics for a fuzzy-score increase.

Ruff, all 551 existing repository tests (54.201 seconds) and
`git diff --check` pass. No new test infrastructure was needed.
