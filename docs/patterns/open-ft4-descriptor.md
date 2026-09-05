# OPEN textured-quad descriptor width

## Function Match Plan (`8ac47fe`)

OPEN `sprite_add_ft4` at `0x8001399c` is 448 bytes and strict 98.973210%
under `probe-gcc257-o2-g0`. Its 48-byte frame, two SDK calls, two validated
address pairs, complete linear CFG and return delay slot agree. Twenty-one
register-operand words differ, first at +0x28 (saved texture pointer). The
position descriptor supplies halfwords; texture fields are read as unsigned
bytes at offsets 0/2/4/6 and stored into SDK byte UV lanes. Color and OT index
are O32 stack arguments; page/CLUT stores are halfwords, OT index is masked
with `0x3fff`, and the allocated SDK packet is 40 bytes.

The six semantic queries, full body, all three call argument sequences,
adjacent G4/F4 bodies, SDK `RECT`/`POLY_FT4` declarations, SetPolyFT4/AddPrim
bodies and PRIM attribution, source history and GAME homolog were inspected.
This is game-owned packet policy around separately attributed SDK helpers.
There are no strings or candidate outgoing references. The unrelated GP
initialization at `0x8001aa7c` happens to point at the first UV descriptor;
it is not evidence of a second texture consumer or of field widths.

All three descriptors occupy eight initialized bytes with pairs encoding
origin/span: scene 3 `{0,0,255,254}`, ending `{0,1,255,254}`, fade
`{0,0,255,240}`. Current `u8[8]` identities spell the odd bytes explicitly.
Test the authentic eight-byte SDK `RECT` as a four-halfword descriptor, with
`x/y/w/h` fields narrowed only by their byte GPU destinations. This is a
candidate source-type recovery, not proof that ignored upper bytes were
original halfwords: inspect the emitted load widths before retaining it.
Change shared OPEN declarations and all three data definitions/call sites
together; keep position/color types, packet order, allocation and scalar ABI
unchanged. Preserve the exact F4 sibling and fade caller, and do not regress
the scene-3 or ending callers. No GAME source or compiler flags are changed.

## Result

The `RECT` trial emitted the same 448-byte body and the same 21 register-word
differences. All texture accesses remained `lbu`, including the low-byte
loads for the halfword sums. Both SDK calls, both numeric address pairs and
the exact 276-byte F4 sibling were preserved. Thus the retail byte loads do
not distinguish the two original descriptor types under this probe.

Reverted the candidate type and all caller/data edits: it supplies no new
matching evidence and the untouched high-byte ownership remains uncertain.
The current strict result is unchanged, 98.973210%; no exact result is banked.
