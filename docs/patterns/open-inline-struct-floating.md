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

## Lighting step with mixed pointer/value state

Retail stores the lighting phase as a stack halfword and keeps its blend in
`s4`. A further control passes only the phase by pointer, takes the blend as
an `s16` value, executes the original complete lighting switch and returns the
updated blend. This removes the earlier two-pointer helper's requirement for
an addressable blend without changing the state encodings or SDK calls.

The mixed helper restores the retail 264-byte frame and every save home,
preserves 40 calls and 31 ordered references, and keeps six siblings raw exact.
Native/traced whole-ELF parity holds. However, it emits 1948 bytes at
94.207820%, with 317 unequal aligned words. Its first difference is +0xc0:
the retained/retail blend initialization `move s4,zero` is replaced by a
halfword stack store. Matching the frame therefore does not recover the
retail value allocation. This control is rejected; generated evidence is
under `lighting-mixed-inline/phase-pointer-blend-value/`.

## Value-only panel cadence predicate

A static inline `int` predicate takes the existing short tick and returns
its original `tick == 0 || tick == 2` condition. The caller uses the Boolean
result in the existing panel-update guard; no state test or consumer is added.
This tests an actual predicate return/control boundary instead of changing
which cadence states the source compares.

With the retained top-color pointer, it produces 1960 bytes at 99.074070%.
All seven original dispatch differences remain (first +0x398); the panel
region adds four instructions. There are 81 unequal aligned retail words.
A separately planned composition restores direct top-color arguments, using
the previously established invariant-movement sensitivity. This still emits
1960 bytes but falls to 97.098760%, with 229 unequal aligned words and first
difference +0x2b4 in pointer construction. It does not recover the earlier
hold-state guard's useful result. Both retain all 40 calls, 31 references,
264-byte frame/save homes, six exact siblings and native/traced ELF parity.
Neither is retained. The evidence directories are `tick-predicate-inline/`
and `tick-predicate-direct-top/`.

## Inline panel movement operation

An ordinary static inline void helper takes the existing `KfScreenRect *`
and performs exactly `--panel->y`. The caller retains the original cadence
and stop guards, cursor, clipping and sequence transitions. It introduces no
Boolean result, movement parameter or alternative state representation.

This produces the identical complete resolved ending body, calls and
references as the retained source: 1944 bytes at 99.917694%, with the same
seven dispatch differences. The retail 264-byte frame and save homes, six
raw-exact siblings and native/traced whole-ELF parity all pass. Thus this
pointer-based movement boundary is neutral, unlike the Boolean predicate
boundary. It is not retained as a source reconstruction without additional
evidence. The complete retained-object comparison is under
`panel-step-inline/panel-step-inline/`.

## Inline panel visibility predicate

The rectangle's `y` field is `u16`; the stop-bound check explicitly views it
as signed, while clipping adds the bias and narrows back to `u16` before an
unsigned span comparison. A static inline `int` helper taking a const
rectangle pointer preserves that exact clipping expression and replaces only
the draw visibility predicate. No coordinate type or cadence test changes.

Unlike the two-comparison cadence helper, this produces the identical full
resolved words, calls and references as the retained ending body. It stays
1944 bytes at 99.917694%, with the same seven differences, retail frame/save
homes, six exact siblings and whole native/traced ELF parity. It is neutral
and is not adopted. The explicit retained comparison is under
`panel-visibility-inline/panel-visibility-inline/`.

## Phase-derived destination matrix index

The two lighting phases 0/1 target consecutive matrix presets 3/4. A separate
source control replaces each destination's literal index with midpoint-preset
index plus the current short phase; source-matrix arguments and both call
sites remain unchanged. The case-conditioned expression has the same actual
matrix destinations, without an invented table or merged call.

The compiler folds the index arithmetic completely. The resulting body is
1944 bytes at 99.843620%, with ten unequal retail words. Seven are the retained
dispatch differences. The other three, +0x3c8/+0x3cc/+0x3d4, reverse the address
base of the first matrix pair: the candidate forms midpoint in `a1`, then
black in `a0` by subtracting 64; retail forms black in `a0`, then midpoint in
`a1` by adding 64. Thus the address-construction referent changes even though
the final call arguments do not. The target inventory is not changed.

All 40 calls, the 264-byte frame/save homes, six raw-exact siblings and native/traced
ELF parity pass. The seven dispatch differences remain and this control is
rejected. Evidence is under `lighting-target-index/phase-target-index/`.

## Traversal-owned panel declarations

A main-branch audit through `3ff5011` finds no newly exact OPEN target; its
recent GAME dispatcher lifetime work (`85f127c`) supplies a reason to inspect
ownership but not an OPEN compiler attribution. The ending panel counter and
cursor are reset and used only within each admitted traversal. A control
moves their existing declarations from function scope into that block,
leaving their types, assignments, order and every use unchanged. Searches of
the two main generated corpora found no equivalent scoped declaration form.

The complete resolved body, calls and references remain identical to retained:
1944 bytes, 99.917694%, the same seven dispatch differences. The 264-byte frame,
save homes, six exact siblings and native/traced ELF parity pass. This lexical
scope change is neutral; the GAME role-splitting result cannot be assumed to
transfer. No main-branch source or tooling changes are merged for this test.
Evidence is under `panel-traversal-scope/panel-traversal-scope/`.

## Inline display activation branch

An ordinary static inline void helper takes mode and both existing DRAWENV
pointers and contains the complete final mode-dependent activation branch.
Its DFE stores, intervening PutDrawEnv calls, global DISPENV submission and
SetDispMask calls remain in original order. The caller keeps both pointer
lifetimes and its existing first-equals-second-minus-one assignment.

The complete resolved display body, calls and references equal the retained
object: 476 bytes at 96.652540%, with the same 102 unequal aligned retail words
and retained extra address reference. The 48-byte frame/save map, 18 calls,
three raw-exact siblings and native/traced ELF parity pass. This activation
boundary is neutral and is not adopted. Evidence and retained comparison are
under `display-activation-inline/display-activation-inline/`.

## Draw-pair setup returning the first environment

A static inline DRAWENV-pointer helper takes the retained second-environment
pointer, derives first as second-minus-one, performs the original paired
DTD/ISBG/RGB writes and returns first for the existing activation consumer.
The caller's second-pointer lifetime and all field-write/call ordering remain.
This gives the helper the established in-array relationship instead of two
independent absolute pointer arguments.

It produces 440 bytes at 84.525420%, preserving 18 calls, the 48-byte frame and
all save homes, three raw-exact siblings and native/traced ELF parity. It
reduces address references to 8 (retail 16, retained 17); flag and color stores
reuse signed offsets from the second environment. The first raw divergence
is still +0x1c and 103 aligned retail words differ. This valid pointer relation
produces shorter code, not retail's 472-byte addressing schedule. The control
is rejected without modifying the target inventory. Evidence is under
`display-prepare-return/prepare-return-first/`.

## Narrow starfield angle update followed by wrapping

The shared rotation field is `s16`. Retail loads its halfword, decrements,
masks to twelve bits and stores once at `80015418` through `80015428`.
A control spells the update as `--entity_27->rotation.z` followed by its
compound mask assignment. For wrapped angles 0..4095 the intermediate value
is representable in the signed halfword; the source does not change the
field type or introduce a temporary, call or volatile store.

The pinned compiler combines the statements into the identical full resolved
ending body, including retail's single angle store. Calls and references
match retained, with 1944 bytes at 99.917694%, the same seven differences,
264-byte frame/save homes, six exact siblings and native/traced ELF parity.
This update-boundary control is neutral and is not adopted. The explicit
retained comparison is under `starfield-angle-update/decrement-then-wrap/`.

## Starfield brightness assignment-result chain

Retail reloads red, increments it and stores its low byte to red, blue and
green in order. A control expresses that actual shared value as `g = b = ++r`
using the original complete material-member expressions. The existing guard
keeps red below 255; no byte type, reload-producing guard or call changes.

The complete resolved body, calls and references remain identical to the
retained ending source, including the retail reload and all three stores.
The result is 1944 bytes at 99.917694%, the same seven dispatch differences,
264-byte frame/save homes, six exact siblings and whole native/traced ELF
parity. The assignment-result form is neutral here and is not adopted.
Evidence and retained comparison are under
`starfield-brightness-chain/brightness-assignment-chain/`.

## Verification and limits

All twenty-eight native/traced compilations have whole-ELF parity. Each candidate's
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

After the first seventeen controls, the full `kf build` attempt confirms
OPEN 106/108, GAME 320/362 and PSX 1/1 exact functions, but exits 1 on image
data/closure checks. It reports
OPEN target relink 34/38, GAME 75/77 and PSX 1/1, with data placement, conflicting
section bases and incomplete ownership among the failures. No production
source, profile or inventory changed in this campaign. Function-level
controls do not establish a clean full build or linked-image closure.

`ruff check scripts tests` and `git diff --check` pass. Repository unittest
discovery passes all 703 tests, with nine skips. No tooling or flake changes
are included, and no exact result is newly banked.

After control 26, all 26 native/traced object pairs were compared again and
remain byte-identical. Production source, headers, configuration, tooling and
tests are unchanged from the preceding full verification; its build failure
and test results remain the applicable baseline, not a new successful build.
