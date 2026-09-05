# GAME textured-quad selector arguments

## Function Match Plan

At `78bb67e`, GAME `0x80014314 sprite_add_ft4` owns 448 retail bytes and
is strict objdiff 98.973210%. Its sole caller, `display_play_transition`
(`0x800144d4`, 416 bytes), is exact. Before editing, all six semantic views,
the complete caller and adjacent main body, both SDK callee bodies and
declarations, packet fields, source history and the exact OPEN counterpart
were inspected.

The emitter is a linear function with a 48-byte frame and saved `s0..s6`.
It allocates a 40-byte `POLY_FT4`, reads unsigned position halfwords and
texture bytes at offsets 0/2/4/6, writes the RGB triplet and masks its
ordering-table index with 0x3fff. Color and index are O32 stack arguments.
There are no strings, conditional branches or candidate outgoing references.
The two validated address pairs are `display_state + 0x20` (primitive-buffer
pointer) and `display_state + 0x20024` (ordering-table pointer). The proven
calls are `SetPolyFT4` at `0x800543fc` and `AddPrim` at `0x80054290`; their
delay slots carry the packet pointer and final OT address respectively.
The return delay slot releases the frame.

This emitter is game-owned packet construction, not vendored library code.
Both callees are independently identified by exact Release 2.5
`LIBGPU.LIB/PRIM` section/FID evidence: offsets 0x320 and 0x1b4. The supplied
`LIBGPU.H` declares the authentic `POLY_FT4` fields `u_short clut, tpage`,
and `GetTPage`/`GetClut` return `u_short`. These SDK types remain unchanged.

The caller zero-extends the SDK results into saved full-register values at
`0x800145d4/d8`, then passes them with plain register moves at
`0x80014614/18`. Its C already retains them in `int` locals. The emitter
copies the incoming selectors without further narrowing and only consumes
their low halfwords when storing the packet fields. This supports promoted
word arguments, while not proving a unique signed/unsigned original spelling
or any use of upper bits. Packet-field width alone is not argument-width
evidence.

The first compiled divergence is the texture-pointer destination at +0x28;
21 words differ in saved argument/register roles, but all memory widths,
stores, calls, constants, referents and control flow agree. The independently
exact OPEN counterpart closes the same comparison with `u32 tpage, u32 clut`
([OPEN evidence](open-ft4-descriptor.md)). Test that selector pair consistently
in GAME's declaration and definition. Preserve every body statement, packet
type, OT-index type, descriptor, owner and compiler flag. Rebuild the emitter
and its exact caller, inspect raw words/relocations, then refresh strict
objdiff. Update the curated signature only with the supported result and bank
only a verified strict 100% function after the full build and checks.

## Final verdict

The word selector pair recovers all 21 differing words and the complete
448-byte function. Fresh `kf try` comparisons are exact for both emitter
and caller. The recorded rebuild confirms strict objdiff **100%** for
`sprite_add_ft4` and preserves `display_play_transition` at 100%. The full
raw object disassembly, including encoded instruction words and ordered
relocations, is identical to the retail-derived target. The SDK fields,
both calls, both address pairs and all delay slots remain unchanged.

The curated GAME signature now agrees with the checked definition and
declaration, including the existing unsigned position/texture views and
masked OT index. The promoted selector width is supported by caller/body
evidence and exact compilation; it does not uniquely identify historical
word signedness. Neither OPEN nor any compiler setting was changed.

GAME advances **264 -> 265 / 362 exact**, leaving 97 partial functions.
Only `GAME.EXE 0x80014314` is selected for banking. All other 483 function
report rows, including 13 SDK source controls, retain their previous scores.
The existing inventory check, Ruff, all 551 repository tests (48.341 seconds)
and `git diff --check` pass. Full `kf build` retains the known 48 divergent
source data-owning units, incomplete ownership and six conflicting section
bases; there is no new matching regression.
