# OPEN camera-step lane publication

## Function Match Plan: grouped publication (`7963a51`)

OPEN `80014100` remains 360 bytes and strict 97.777780%. The six semantic
views, full CFG, exact segment/begin siblings, following scene-0 body, all
four caller argument sequences, shared 100-byte state, original source
history and GAME source homolog were rechecked. The one game-owned call,
25 validated state-field pairs, no strings/candidate references, 32-byte
frame, signed word Q4 lanes, signed height argument, halfword rotation
outputs and all sentinel/return delay slots retain the contract below.
There is no vendored body here; SDK sine calls occur only in two callers.

The current two-stage source publishes position X/Y, rotation Y, position Z,
rotation X/Z. Retail's output stores at `800141b0/c0/f4`, `80014204/3c/4c`
instead publish position XYZ followed by rotation XYZ. Its arithmetic and
ordered field references already agree; four working-register roles differ.
Test the natural grouped publication order seen in those six stores while
leaving all six accumulator updates in their current preceding stage. This
is distinct from the rejected immediate-lane-publication trial, which moved
each output ahead of later accumulator updates and lost retail preloads.
No alternate lane orders, new locals, type changes or compiler flags are
part of this trial. Compare from the first accumulator load at +0x50 and
preserve both exact siblings before considering a kept result.

The grouped order emits all 360 retail bytes exactly. The single segment
call, all 25 ordered numeric state referents, every load/branch/return delay
slot, and the 32-byte frame agree. The 784-byte segment-computation and
252-byte initialization siblings remain raw exact. Retain only the move of
rotation-Y publication after rotation X; the accumulator stage, shared types,
signatures and compiler profile are unchanged. This closes the observed
register differences with an evidenced source-order correction, without
claiming a particular compiler allocation mechanism.

Strict objdiff confirms 100% for all three camera functions. Only OPEN
`80014100` is newly banked; OPEN moves 96 -> 97 exact out of 108, with eleven
partials and 99.255% overall fuzzy coverage. The focused build recompiles the
unit and regenerates the OPEN report. Full `kf build` retains only the known
OPEN TMD default-table addend mismatch, thirteen GAME data-addend units and
four GAME historical-best deficits; no banked exact function is lost and all
13 vendor-source controls remain exact. Ruff, `git diff --check` and all 401
existing tests pass (25.247 seconds). One inventory expectation was updated
from the old partial score to the newly verified exact status; no tests were
added or relaxed. GAME source, shared headers and toolchain settings are
unchanged.

## Historical Function Match Plan (`8ac47fe`)

OPEN `opening_camera_path_step`, `0x80014100`, owns 360 bytes and is strict
97.777780% under `probe-gcc257-o2-g0`. The six semantic queries, complete
retail CFG, both preceding camera functions, following scene-0 entry, four
caller argument sequences, shared state layout, source history and GAME
homolog were inspected. The custom global camera interpolator is not present
in the vendored inventory; its single direct callee is the exact game-owned
`opening_camera_path_compute_segment`. No strings or candidate references
occur. All 25 validated address pairs target the single 100-byte camera state.

The argument is signed height displacement: two callers supply zero, and two
pass the arithmetic right shift of the SDK sine result by seven. The frame
is 32 bytes. The initial sentinel branch saves the argument in its delay slot;
the decrement branch stores the remaining count in its delay slot. The final
`jr ra` pops the frame. Signed word fixed-point values and deltas feed six
adds; position outputs are signed words shifted by four, and the three
halfword rotation outputs are masked with `0xfff`. All these facts agree with
the current source; the residue is register operands across independent lanes.

Retail stores each updated accumulator and publishes its shifted value before
all later accumulators have been updated. Test direct lane update/publication
expressions in ordinary position XYZ then rotation XYZ order, retaining each
actual accumulator store through compound assignment. This expresses the
observed per-lane dependency instead of the current two-stage update of all
lanes followed by an interleaved publication order. This is one bounded source
dependency hypothesis, not an independent-lane permutation search. Preserve
the exact 784-byte segment and 252-byte begin siblings, all calls/referents,
widths and sentinel CFG. Do not alter GAME, shared types or compiler flags.

## Historical result

Immediate publication retained 360 bytes, the one call and the same 25 numeric
referents, but changed their order from the first position-Y loads at +0x60.
It emitted 58 unequal aligned words, including different load/store schedules;
retail's early preloads of later rotation deltas were lost. Both neighboring
functions remained raw exact. This does not reproduce the observed dependency
schedule, so the trial was reverted. The strict 97.777780% source remains;
no independent-lane permutation or banking was performed.
