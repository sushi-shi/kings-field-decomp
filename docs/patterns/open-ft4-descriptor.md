# OPEN textured-quad descriptor width

## Function Match Plan: scalar argument widths (`31d2676`)

OPEN `8001399c`/448 bytes starts at strict 98.973210% under
`probe-gcc257-o2-g0`. All six semantic queries, complete body, all three
caller argument sequences, adjacent G4/F4 bodies, SetPolyFT4/AddPrim bodies,
SDK declarations/provider evidence, shared graphics owner, source history
and GAME source homolog were inspected. The 48-byte frame, seven saved
registers, two direct calls, two validated address pairs and return delay
slot agree; there are no strings, branches or candidate outgoing references.
The first of 21 unequal words is the saved texture pointer at +0x28. The
helper remains game-owned packet policy around separately attributed PRIM
providers; the existing descriptor-width ambiguity is not reopened.

Scene 3 and ending scroll retain GetTPage/GetClut results in word arrays and
pass them through `lw a2/a3`; fade retains promoted word values. The helper
copies those incoming registers and consumes each only in a halfword packet
store. Thus packet-field width alone does not prove the function's scalar
parameter width. Test `u32 tpage, u32 clut` together as the matching selector
pair, changing the shared OPEN declaration and definition consistently while
retaining authentic SDK return and packet-field types. This is a candidate
boundary model, not a claim that unobserved upper argument bits were used.
Keep OT-index width, source statements, owners, call order and compiler flags
unchanged. Check both raw operand differences and all three callers, preserving
the exact F4 sibling/fade caller. No GAME source or identity is changed.

The word-sized selector pair recovers all 21 differing instruction words:
the focused comparison is exact across all 448 bytes, both numeric call
targets and both address pairs. The 276-byte F4 sibling and 252-byte fade
caller remain raw exact; scene 3 retains its frame-only differences and
ending scroll retains the same 167 unequal words. Keep the word selector
boundary and update the curated OPEN signature, subject to strict objdiff
verification. Packet fields and SDK return types remain unsigned halfwords;
the helper parameter types now reflect the promoted values its callers pass.
No OT-index experiment is needed to close this body.

The real focused rebuild confirms **100.000000% strict objdiff**. The shared
header change rebuilds all 24 OPEN consumers without regressing an existing
exact function: OPEN advances from 95/108 to **96/108 exact**. Scene 3 remains
99.931370%, ending scroll 97.129630%, and the fade/F4 controls remain 100%.
This closes the reconstructed helper; it does not identify a unique original
signed/unsigned word spelling or prove historical compiler attribution.

Full `kf build` preserves all prior exact functions and all 13 SDK source
controls. It still reports the existing OPEN TMD switch-addend mismatch and
unchanged GAME switch/four historical-best failures. Ruff and
`git diff --check` pass. The existing 401 tests take 17.798 seconds: 400 pass
and the previously recorded 883-versus-882 pointer-cast floor fails. No casts,
tests, toolchain options, SDK bodies or GAME source are changed here.

`kf bank --function open:0x8001399c` banks this exact function only; every
other baseline row, including the F4 sibling and GAME rows, is unchanged.

## Historical descriptor plan (`8ac47fe`)

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
