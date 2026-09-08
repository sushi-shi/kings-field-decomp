# OPEN inline helpers, local structs, and floating arithmetic

These controls examine source abstractions and arithmetic beyond the earlier
dispatch/address variations. They use the unchanged pinned GCC 2.5.7 profile
and make no historical compiler claim.
The retained OPEN targets are `opening_ending_scroll_run` at 80014e28,
1944 bytes / 99.917694%, and `display_initialize` at 80016adc,
476 bytes / 96.652540% against retail's 472 bytes.

Plans precede edits under `build/open-missed-evidence/`:
`inline-struct-float-plan.md`, `inline-value-reciprocal-plan.md`,
`display-inline-rect-plan.md`, and `display-rect-fields-plan.md`.
Every source is a separate generated full-TU copy. Production sources,
headers, inventories, profiles and banked results are unchanged.

## Inline helpers and aggregate models

| Target/control | Strict score | Bytes | Finding |
| --- | ---: | ---: | --- |
| Ending lighting switch in an ordinary static inline helper, with phase/blend pointer parameters | 94.004110% | 1944 | Inlined; frame shrinks from 264 to 256 bytes and both values become addressable halfword state. |
| Ending local struct pairing short phase and blend | 94.004110% | 1944 | Complete resolved words/calls/references equal the inline-pointer result, including its wrong frame. |
| Ending scalar volume quotient in a value-only inline helper | 99.917694% | 1944 | Complete resolved result equals the retained source; all seven dispatch differences remain. |
| Display paired DTD/ISBG/RGB setup in an inline helper | 95.779660% | 480 | No helper body/call remains, but the DTD address construction becomes an absolute byte store and a separate DRAWENV base is formed later. |
| Display local SDK RECT with aggregate initializer | 55.855930% | 536 | Adds an eight-byte constant table, moves a sibling's string reference, and changes argument/saved-register layout. |
| Same RECT with explicit field initialization | 60.550846% | 520 | Removes that table and restores the sibling; wrong argument/saved-register layout remains. |

The lighting struct has natural signed-short phase/blend fields; there is no
packing, padding or invented carrier. Its first divergence is the frame
allocation itself. The inlined pointer helper preserves all 40 direct calls
and 31 references, but retains halfword loads/stores at sp+168/sp+170 instead
of the retail phase/blend representation. Both retain six raw-exact siblings.
The value-only helper isolates inlining from taking the mutable state addresses;
its neutral result does not imply that all inline helpers must be neutral.

The display pair helper preserves the original field-write order and 18 calls.
Relative to the retained object, its first new difference is at +d8: the DTD
member address/store sequence becomes an absolute store. Later, at the old
+124 boundary, it constructs the first DRAWENV base separately. The net growth
is one instruction and the address-pair count grows to 18 (retail 16, kept 17).
The 48-byte frame/save map and all three siblings remain exact.

The RECT is the authentic SDK type (`short x, y, w, h`), used as local geometry,
not a replacement for DRAWENV or a different global owner. Initial values are
0, 0, 320, 240; the existing coordinates and dimensions use those fields.
Aggregate initialization grows .rodata from 37 to 45 emitted bytes and shifts
`primitive_buffer_allocate`'s format-string reference by eight bytes. That
sibling is therefore not raw exact, despite unchanged call topology.
Explicit field initialization restores its reference and all three siblings.
Both RECT variants use 48-byte frames but a different saved-register map,
so frame size alone is not acceptance evidence. Neither is retained.

## Floating division and reciprocal multiplication

The volume submitted to SsSetMVol ranges from 380 down to zero. Independent
IEEE binary32/binary64 checks over all 381 inputs give the same integer
truncation for division by three and multiplication by a rounded reciprocal.
This is a bounded numerical check, not execution of the target soft-float
routines or a claim about arbitrary integers, rounding modes or special values.

| Arithmetic replacing the integer quotient | Strict score | Bytes | Additional calls |
| --- | ---: | ---: | --- |
| float division | 97.888885% | 1940 | `__floatsisf`, `__divsf3`, `__fixsfsi` |
| float reciprocal multiplication | 97.888885% | 1940 | `__floatsisf`, `__mulsf3`, `__fixsfsi` |
| double division | 95.481480% | 1956 | `__floatsidf`, `__divdf3`, `__fixdfsi` |
| double reciprocal multiplication | 95.481480% | 1956 | `__floatsidf`, `__muldf3`, `__fixdfsi` |

All four have 43 calls instead of retail's 40. They emit no COP1 instructions;
the pinned compiler uses software helpers. They fail the call-set gate before
relocation/score tuning could be relevant. Equal scores for division and
multiplication do not make their helper targets interchangeable. No runtime
symbols or target relocations are invented to accommodate these controls.
All six sibling functions remain raw exact.

## Value-return phase helper and full-width dimension pair

A scalar `static inline` phase-successor helper replaces the two lighting
phase assignments with a promoted increment narrowed through the existing
enum boundary. This separates value inlining from the address-taking helper.
It retains the 264-byte frame, all saved-register homes, 40 calls and ordered
referents, but grows ending to 1952 bytes and scores 96.868310%. Its first raw
difference is at +0x2a0; all six siblings remain exact. The retained source
still performs better. This control is recorded under `lighting-value-inline/`.

A display dimension pair containing two `s32` fields groups width and height
without RECT's halfword narrowing. Explicit field assignments replace the
existing height assignment after ResetGraph; the separate `s16` lower-buffer
origin remains. This scores 63.161020% at 524 bytes, with all 18 calls and three
exact siblings. Its 48-byte frame has different save homes, diverging at +4;
ordered references differ too. Preserving full-width dimensions alone does
not recover the retail code. This control is under `display-dimensions/`.

## Inline double-buffer definition

Moving the four consecutive `SetDefDrawEnv`/`SetDefDispEnv` calls into an
ordinary static inline helper with `s32` height and `s16` lower-origin value
parameters produces the same complete resolved words, calls and references
as the retained display function: 476 bytes, 96.652540%. All three siblings
remain exact, and native/traced ELF files agree. The generated helper groups
a coherent crossed-origin double-buffer operation but supplies no new evidence
for an original helper identity. This neutral result is recorded under
`display-buffer-inline/`, including the explicit retained-object comparison.

## Value-return audio fade step

An ordinary static inline `s32` helper groups the volume decrement and
`SsSetMVol` call and returns the updated volume. The caller keeps its zero
check and sequence replacement transition. This differs from the neutral
quotient-only helper by carrying the returned value across an SDK call.

The resulting ending body is still 1944 bytes with the exact 264-byte frame
and save homes, 40 calls, 31 ordered references, and six raw-exact siblings.
Native/traced ELF parity holds. Strict score becomes 99.876540%, with the
seven retained dispatch differences plus four new instruction differences.
Against the retained body, +0x460 writes the decremented volume into `$v0`
instead of `$t0`; the division at +0x464, comparison at +0x480 and stack store
at +0x490 follow that register change. No other resolved words change.
The helper is not retained. The complete comparison is recorded under
`sequence-step-inline/`; this is an observed source-boundary effect, without
claiming an original helper or attributing a compiler mechanism.

## Value-return paired gradient step

A static inline helper advances and saturates the signed halfword background
blend, calls both existing `color_lerp_cvector` operations with separate SDK
color pointers, and returns the blend. The caller keeps its two color objects,
top-color alias and subsequent drawing calls. No aggregate layout is assumed.

This preserves all 40 calls, 31 ordered references and six raw-exact siblings,
with whole native/traced ELF parity. It changes the ending frame from 264 to
256 bytes, moves every saved-register home down eight bytes and first differs
at the prologue. The body becomes 1924 bytes at 95.734566%, with 308 unequal
aligned words against retail. Returning a scalar by value therefore does not
by itself preserve the caller frame across this larger inline boundary. The
control is rejected and recorded under `gradient-step-inline/`.

## Splitting the gradient helper

Two complementary controls isolate the preceding full gradient helper:

| Inline operation | Strict score | Bytes | Frame and first divergence |
| --- | ---: | ---: | --- |
| Only the two interpolation calls, void return, blend and color pointers as parameters | 96.676956% | 1936 | 256-byte frame; all save homes down eight bytes; first difference at +0. |
| Only halfword blend increment/saturation, returning the updated blend | 96.839510% | 1932 | Retail 264-byte frame and save homes; first difference at +0x294. |

Both retain the 40-call sequence, six raw-exact siblings and native/traced ELF
parity. The interpolation-only control changes ordered references; the
blend-only control retains them. Their unequal aligned retail word counts are
326 and 303 respectively. These results associate the frame shrink with the
interpolation boundary in this controlled split, not merely with a scalar
return. They do not establish an optimizer mechanism. Neither is retained.
Generated evidence is under `gradient-interpolate-inline/` and
`gradient-blend-inline/`.

## Verification and limits

All seventeen native/traced compilations have whole-ELF parity. Each candidate's
calls, referents and raw code were inspected; frame size/save homes were
checked separately. The initialized RECT's sibling assertion correctly failed;
a read-only follow-up audited both completed objects and recorded the shifted
string referent rather than weakening the gate. Plans, generated C, traces,
objects and JSON reports remain under `inline-struct-float/`,
`inline-value-reciprocal/`, `display-inline-rect/`, `display-rect-fields/`,
and the follow-up directories named above, all in the campaign build directory.
They are not committed build products.

No candidate improves the retained source. These results constrain the tested
abstractions and arithmetic sites; they do not exhaust inlining, aggregate
modelling, or other source-level explanations. No function is newly banked.

The final full `kf build` attempt confirms OPEN 106/108, GAME 320/362 and
PSX 1/1 exact functions, but exits 1 on image data/closure checks. It reports
OPEN target relink 34/38, GAME 75/77 and PSX 1/1, with data placement, conflicting
section bases and incomplete ownership among the failures. No production
source, profile or inventory changed in this campaign. Function-level
controls do not establish a clean full build or linked-image closure.

`ruff check scripts tests` and `git diff --check` pass. Repository unittest
discovery passes all 703 tests, with nine skips. No tooling or flake changes
are included, and no exact result is newly banked.
