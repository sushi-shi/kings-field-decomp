# GAME fog interpolation expression

## Function Match Plan

Starting at `60f66c7`, GAME `0x800203bc fog_interpolate_near` has a
68-byte retail body and strict objdiff 92.941180%. Its four neighbors in
`game.matrix` are exact. All six image-qualified semantic queries, both
adjacent wrappers, the three incoming call sites, source history, the SDK
header and the exact OPEN counterpart were inspected before editing.

The signature is `void fog_interpolate_near(s32 start, s32 end, s32 ratio)`.
Death-fade callers pass the saved signed distance, zero and a signed Q12
blend; player update passes 5000, 11000 and its fade scaled by 128. There
are no strings, conditional branches or unresolved outgoing candidates.
Retail computes the signed product `(end - start) * ratio`, arithmetic-shifts
by 12, adds the preserved start and stores to `render_state + 0xa0` at
`0x80095740`. This is one validated HI16/LO16 pair. The sole proven call is
`SetFogNear` at `0x8004f4b4`, with projection 200 in its delay slot. The
24-byte frame is released in the return delay slot after the delayed RA load.

The target is absent from the vendored census. Its interpolation and render
state write are game-owned policy; the callee is independently attributed
to Release 2.5 `LIBGTE.LIB/GEO`, offset `0xb68`, by the exact provider section
and FID evidence. `LIBGTE.H` declares `void SetFogNear(long a, long h)`.

Retail begins by preserving start in `v0`, then puts the scaled difference
in `a0`. The old accumulator expression omits that first move and exchanges
these arithmetic destinations. The independently exact OPEN counterpart
(`0x80019658`, see [its evidence](open-fog-interpolation.md)) supports writing
the expression as `(((end - start) * ratio) >> 12) + start`. Test only that
source change, preserving all types, ownership, SDK interfaces and flags.

Rebuild `game.matrix`, inspect the first divergence and all raw instructions
and ordered relocations, and preserve the four exact neighboring functions.
Then run the full build and repository checks. Bank only a verified strict
100% result; do not alter unrelated baseline rows or OPEN source.

## Final verdict

The expression change closes the function at strict objdiff **100%** and
68 bytes. A fresh `kf try` reports all five matrix functions exact; the
recorded `kf match` rebuild also confirms all five at strict 100%. Raw
disassembly agrees on all 17 instructions, including the initial preserved
start, signed multiply/shift, state write, projection argument and return
delay slot. Ordered relocations are HI16/LO16 to `render_state + 0xa0`, then
R_MIPS_26 to `SetFogNear`, at the same instruction offsets as the target.
No profile, type, identity, relocation or OPEN source was changed.

GAME advances **262 -> 263 / 362 exact**, with 99 non-exact functions left.
Only `GAME.EXE 0x800203bc` is selected for new banking. Full-image function
comparison preserves all other 483 report rows, including the 13 vendored
source controls. Ruff, all 551 existing repository tests and `git diff
--check` pass. Full `kf build` retains the existing strict failures: 48 of
59 source data-owning units diverge, known-reference ownership is incomplete,
and six units have conflicting section bases. No new data divergence or
previously exact function loss was introduced.

## Follow-up comparison: camera step

This historical conclusion is superseded by the
[SDK vector publication control](sdk-vector-publication.md): natural grouped
XYZ assignments do emit GAME's interleaved store order and close the function
at strict 100%. The earlier inspection below did not test that source form.

GAME `camera_path_step` (`0x80033680`, 284 bytes, strict 90.915490%) was
inspected without edits through the six semantic views and a fresh focused
comparison. The exact OPEN publication-order correction is not transferable:
GAME retail stores position X/Y, rotation Y, position Z, then rotation X/Z,
which is already the current GAME C order. Its first divergence is the late
load of `rotation_fixed.vy` (state +48); the remaining differences affect
registers and ordering in the publication stage. Both preceding camera
functions remain exact. There is no new supported source correction here,
so no lane permutation, compiler change or banking was attempted.
