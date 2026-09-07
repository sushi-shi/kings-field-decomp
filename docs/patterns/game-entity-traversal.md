# GAME entity-pool traversal

## Function Match Plan: wrapped origins and shared continuations

GAME `8001f218`, `render_entities(void)`, owns 1408 retail bytes in the
single-function `game.render_scene` unit. The initial strict score is
91.250000%; the fresh candidate is 1416 bytes under the unchanged
`probe-gcc257-o2-g0` profile. All six image-qualified semantic views, the
complete retail body/CFG, sole caller, emitter callees, adjacent boundaries,
shared layouts, source history and OPEN traversal evidence were inspected.

Retail has a 48-byte frame, saves ra and s0..s6, and contains 61 blocks,
40 conditional branches, nine direct calls, one internal jump, eight division
traps and one return with a frame-restoring delay slot. There are 24 validated
address pairs and one validated internal jump; no strings, candidate outgoing
references or unresolved indirect transfers. The sole caller is render_frame
at `800202c0`: no arguments, nop delay slot, no consumed result.

The five passes walk 190 map objects (44-byte stride), 128 actors (72), the
halfword floor-item count (24), 48 effects (60) and eight map events (68).
Each counter uses signed-halfword countdown to -1. All visibility rows and
columns wrap to u16 before unsigned window comparisons and row-major byte
lookup. Item/effect coordinates are signed words divided by 2000. The actor
variant path instead compares wrapped cell offsets plus 12 against 24.
The call order is tmd_select(1), map-object emitter, actor emitter,
SetLightMatrix, floor-item emitter, SetLightMatrix, effect emitter,
SetLightMatrix and map-event emitter. Lifecycle/type guards and all skipped
paths remain intact.

This pool/visibility policy is game code, absent from the vendored roster.
SetLightMatrix is independently identified as Release 2.5 LIBGTE/MTX+0xc38
by a unique 384-fixed-bit FID; its retail body and SDK MATRIX-pointer
declaration are independent negative controls against reconstructing SDK code.

First test only the two saved window-origin locals as u16. Both inputs are
unsigned halfword loads and every observer sees a subsequent u16 difference;
no full-word observer exists. OPEN's independently retained wrapped origins
restore its entry and entity loop without new masks. This is a supported
wrapped-coordinate model, not proof of the original local spelling.

Then, as a separate control, place actor/effect pointer advancement in their
common body continuation before the counter update. Retail advances the
coordinate pointer at `8001f404`/`8001f698` before decrementing the counter;
the current comma update emits counter work in guards and an extra effect
block. Explicit shared continuation labels can preserve every skip while
advancing each record exactly once; no dummy state or forced instructions.

An earlier-layer ownership issue is already known: retail retains the
active tpage address `8009505a` and derives floor_items with +62 and its
coordinate pointer with +66. The current separate globals materialize the
item base independently. Do not fake that relationship with out-of-object
pointer arithmetic; leave this owner campaign unresolved here. Compare each
focused compile from the first divergence, check all raw words and ordered
numeric references, and preserve all banked functions. Partial improvement
is not an exact match and must not be banked.

## Focused controls

Wrapped origins emit 1424 bytes and strict 93.056816%, recovering the entry
load/arithmetic order and map-object pointer roles without adding masks. The
first raw difference moves from +0x28 to the saved-origin register at +0x5c.
The actor continuation restores its common pointer-before-counter tail and
all actor guard delay slots, reducing the candidate to 1420 bytes. Adding
the effect continuation removes its split tail: all 61 block successor lists
now agree with retail. The nine call targets and all 25 candidate address
pairs retain their original numeric sequence. The extra floor_items pair
still reflects the independently recorded owner gap.

### Actor variant-coordinate boundary

Retail `8001f3c4..8001f3cc` adds 12, subtracts the view Z, then narrows once
before comparing against 24. Current source first narrows cell+12 into dz
and then narrows the subtraction separately. Test assigning the complete
wrapped relative coordinate to each existing u16 dz/dx local, then comparing
the locals directly. This removes an unobserved intermediate narrowing and
makes their delta meaning explicit; all arithmetic remains defined under
integer promotion. No local, operation, guard or state is added. Reject if
the emitted boundary does not improve or the numeric references/CFG change.

The combined-coordinate control keeps all references and known successors,
but materializes 0xfff4 in an additional saved register, moves the addition to
the view operand, and expands the frame to 56 bytes. It emits 1432 bytes and
does not recover the observed two-instruction Z subtraction/mask boundary.
Restore the staged cell-plus-12 expressions. Retain only wrapped origins and
the two shared pointer-first continuations.

## Final verdict and verification

The retained change reaches strict **95.164770%**, up from **91.250000%**.
It emits 1420 bytes versus retail's 1408. The 48-byte frame, all 40 branches,
nine calls, division traps, return/delay slot and complete known successor
lists are preserved; both loop continuations now follow retail's shared
pointer-before-counter form. The original 324 unequal aligned words fall to
218, although alignment-sensitive counts are diagnostic rather than a score.
The first difference is the saved-origin register at +0x5c; actor Z's
subtraction/mask destination still differs at +0x1b0. No register-mechanism
attribution follows from those symptoms.

The floor-item material/base ownership issue remains at +0x238: candidate
uses an independent floor_items pair instead of deriving the pool from the
material tpage member. Both signed world-Z division/window sequences also
retain an extra load-delay nop and different subtraction destinations.
These are recorded residues, not reasons to fabricate an owner or mask a
reference. This function is partial and is not banked.

Fresh compilation checks every resolved word and ordered numeric reference.
The delinked target itself reconstructs the hash-verified retail body,
SHA-256 `07cf39c53268d3d9afeeeb683861395797cd1b1c4fba23b010b8b65b9352bb68`.
All nine calls match retail; the candidate's ordered 25 address pairs equal
the pre-campaign source, retaining only the known extra floor_items pair.
The scored object equals the fresh compilation. Recompilation using the
committed `c6ad920` player/collision headers gives identical entire text and
non-debug relocations, isolating the result from concurrent naming changes.

Across all 484 scored rows only render_entities changes from the preceding
committed audio campaign. GAME stays 311/362 exact, OPEN 98/108 and PSX 1/1:
410/471 total. Fresh raw controls also preserve all 27 exact GAME actor,
14 GAME audio and five OPEN spatial-audio functions. Ruff, all 678 repository
tests (79.690 seconds), and `git diff --check` pass. Focused matching actually
recompiles game.render_scene; full `kf build` retains the existing failures:
source data PSX 0/1, GAME 9/42, OPEN 2/19; target relinks 1/1, 75/77, 34/38;
six conflicting-section cases, incomplete known-reference ownership and zero
artifact failures. No header, toolchain profile, tooling, test, inventory or
OPEN source change belongs to this campaign.
