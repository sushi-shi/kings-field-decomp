# OPEN camera-step lane publication

## Function Match Plan (`8ac47fe`)

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

## Result

Immediate publication retained 360 bytes, the one call and the same 25 numeric
referents, but changed their order from the first position-Y loads at +0x60.
It emitted 58 unequal aligned words, including different load/store schedules;
retail's early preloads of later rotation deltas were lost. Both neighboring
functions remained raw exact. This does not reproduce the observed dependency
schedule, so the trial was reverted. The strict 97.777780% source remains;
no independent-lane permutation or banking was performed.
