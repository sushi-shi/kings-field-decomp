# GAME entity-culling compiler traces

## Function Match Plan

At `14f9686`, GAME `8001f218 render_entities`, 1408 retail bytes in
`game.render_scene`, is strict **96.667610%**, 1416 compiled bytes.
Fresh native/traced whole ELF parity holds for source SHA-256
`c438e1d6a070701d40aeea6265d3efad61eade7cc05e29e6f3b78455cbeed93b`.
The pinned compiler fingerprint remains
`222b6cc36272847ffde09ad4b7f2db81632fe7fd282bfc6df1f155939b63cb12`.
Generated evidence is under `build/gcc257/game-entity-cull-traces/`.

All six GAME semantic views, complete retail and candidate bodies, sole
caller window, seven distinct callee contracts/bodies, neighboring event
emitter and weapon renderer, shared layouts, history and previous
[traversal](game-entity-traversal.md) and
[floor-item controls](game-graphics-owner-pilot.md#first-floor-item-owner-result-and-focused-source-controls)
were read. The sole caller at `800202c0` passes no arguments and consumes
no return. Preserve this void interface and all five pool passes. This is
game visibility policy; the 48-byte `SetLightMatrix` provider is separately
attributed to Psy-Q Release 2.5 LIBGTE/MTX+0xc38 and remains external.

Retail has a 48-byte frame, 61 blocks, 40 conditional branches, nine direct
calls, one validated internal jump, 24 validated address pairs and eight
division traps. No strings, candidate outgoing rows or indirect transfers
occur. Preserve every delay slot, signed-halfword countdown, u16 wrapped
window coordinate, byte visibility/lifecycle selector, signed division by
2000, and the 190/128/variable/48/8 pool extents. Keep all existing owners,
types, compiler options and the single-function unit claim.

The first unequal word at +0x5c selects s5 instead of retail s6 for the Z
origin. An earlier comparison layer is still open: all address destinations
exist, but the active tpage address appears after the count/texture reads
instead of before them. The floor-item base derives from blue+58 rather
than tpage+62. CSE2 retains the blue-address pseudo and makes the tpage store
absolute. The complete graphics owner is independently established; do not
change its layout or use cross-field pointer arithmetic.

First bind a real `u16 *active_tpage` after brightness publication and before
the count snapshot, then use it for the existing tpage store. Retail explicitly
forms this member's address at `8001f450`, before those reads. This tests the
member's actual definition/use lifetime without any extra memory operation.
It differs from the previous chained-brightness and count-snapshot controls.
Inspect all ordered references and raw instructions before keeping anything.

The separate actor-square arithmetic question remains at +0x1b0: retail
computes cell+12-view before its final mask, using v1 for the difference;
current code narrows cell+12 in a u16 local first. The earlier combined-u16
relative-coordinate trial moved -12 into a saved register and is rejected.
If needed, test the existing Z value as a word accumulator with the decoded
addition and subtraction, narrowing only for the unsigned comparison. Keep
X unchanged initially. This must preserve all 16-bit input combinations and
must not introduce a new value solely for register selection.

Require strict 100% and every retail word before closure; use native/debug/
traced parity and small controls for any claimed compiler mechanism. Apply
only supported source, rebuild the affected unit, run the full build and
repository checks, then bank only exact results. Rejected trials remain
isolated and do not change production source or the bank.

The early tpage-member binding is byte-identical at 96.667610%, 1416 bytes.
The pointer spelling does not preserve a different address root; reject it.
Proceed with only the planned Z word-accumulator control from production.

The Z word accumulator is also byte-identical; reject it. The current probe
already removes that intermediate narrowing without recovering the retail
subtraction destination.

The exact OPEN `80019240` traversal supplies a stronger source comparison:
its retained [tpage lifetime and shared coordinates](open-entity-traversal-residue.md#exact-closure-shared-visibility-coordinates)
explain both corresponding differences. Its pointer is bound **before** the
color stores, whereas the first GAME trial bound it afterwards. Test that
actual lifetime alone, retaining GAME's separate B/G/R stores, count snapshot
and selector order. Then independently test one function-scope `u16 row/col`
pair reused for the five visibility sweeps. These coordinates have the same
meaning and disjoint lifetimes in each sweep, as in the exact OPEN source;
this is variable reuse, not a declaration-order permutation. Keep the actor
square's existing `dz/dx` form initially. Compose only improvements supported
by raw words and ordered references, and keep OPEN source unchanged.

Binding tpage before the colors reaches **98.465910%**, 1416 bytes. It
recovers the complete material setup, all 24 ordered retail address targets,
and both tpage-relative item cursors. Keep this isolated improvement for
composition after the coordinate trial; it does not yet close the function.
The shared-coordinate trial preserves the order of row calculation and each
current-window load by declaring the existing window pointer before those
assignments. No window load is hoisted in source.

Shared coordinates alone reach **97.690340%**, now exactly 1408 bytes. Both
floor-item and effect row subtractions fill the current-window load delay,
removing the two surplus nops. All nine calls and baseline address targets
remain unchanged; the material-order residue remains because this was an
independent trial. Compose this supported coordinate reuse with the earlier
tpage binding, then inspect the remaining raw differences.

The combined result is **99.488640%**, 1408 bytes, with nine ordered calls
and all 24 ordered retail address pairs. There are 32 unequal words, confined
to GPR operands. The alternate actor-square path still uses separate `dz/dx`
locals rather than the shared row/column coordinates. Extend the same reuse
to that path: assign cell+12, subtract the current view cell with a compound
assignment, then test the narrowed coordinate. This preserves the staged
arithmetic and unsigned wrap, unlike the earlier combined expression that
hoisted -12. Both row and column retain their actual culling meaning; no
additional variable or memory operation is introduced.

The shared square coordinates recover the two Z subtraction/mask words,
reaching **99.517044%**, 1408 bytes. Thirty words remain different only in
GPR operands; calls, addresses, immediates and control destinations agree.
The current-view Z address is held in s6 instead of retail s4, and the two
origin registers and actor/count-result roles differ accordingly.

Test a pointer to the actual signed-halfword `view_cell.z` field, bound before
the actor pass and used for its existing unsigned Z read. Retail explicitly
retains that field address across the actor emitter. Keep the X read through
the complete graphics owner; no pointer is indexed outside the scalar field.
This tests source ownership of the observed address, not a forced lifetime or
hard register. Reject if it changes the anchor, read width or any call/edge.

The scalar view pointer is rejected at **98.661934%**, 1412 bytes. It keeps
the Z anchor but makes the X read an additional independent address pair;
the shared owner's previous relative access is lost. Keep the direct fields.

The actor visibility join combines a byte visibility class and a word boolean
result. Both are consumed only by a zero test. Test that real result channel
as `s32 visible` instead of `u8`, preserving the unsigned byte load, predicate
and both source arms. The complete value range remains 0..255 and needs no
narrowing. This is a bounded width hypothesis for the mixed-result join, not
a changed visibility policy. Keep only a result supported by complete raw
comparison; a no-effect control is discarded.

The word visibility channel is byte-identical at 99.517044%; reject it.
The retained isolated candidate is `shared-square/render_scene.c`, with the
earlier tpage binding and shared wrapped coordinates, including the actor
square. The scalar-view and visibility-width controls are not composed into it.

## Controlled probe evidence

Native, uninstrumented debug and traced whole ELF objects agree for the
baseline and all eight source trials. Repeating the retained trial also
reproduces its complete JSONL trace byte-for-byte. Its source SHA-256 is
`19ec34f756caa0d84bce740ad05996d83e8bad374be6039f8e9aee4322ede321`.

Four small C controls have the same three-compiler parity. A material/array
fixture changes only when its real page pointer is bound. Before the color
stores, CSE1 uses the page at owner+2 as the address root and derives items
with +14. After the stores, CSE1 uses blue at owner+6 and derives items with
+10. CSE2 makes the color stores absolute in the first form while preserving
the page root. This reproduces the main function's tpage-versus-blue address
choice without alias tricks or extra accesses.

A two-pass visibility fixture changes only whether the wrapped row/column
locals are scoped separately or reused. The scoped form is 460 bytes; the
shared form is 456. In the second pass, the scoped output has `lw a1`, a
load-delay nop, `lhu v1`, then `subu v0`. The shared output has `lw a1`,
`subu v1`, `lhu v0`, then the coordinate mask. Both have a 40-byte frame
and the same two calls. The fixture reproduces the removed load-delay nop;
the GAME body removes one in each of its item and effect passes. These are
observations of the pinned probe, not historical compiler attribution.

## Retained result

Apply only the shared-square candidate: one wrapped coordinate pair across
the five passes, including the actor square, and a pointer to the real
texture-page member bound before the brightness stores. Owners, interfaces,
storage widths, constants and all memory operations remain as evidenced.
The result is **99.517044%**, with exactly 352 instructions, nine ordered
calls and 24 ordered address pairs matching retail.

Thirty words still differ in GPR fields. The main differences are the two
saved origins, actor cursor, view-Z address, final actor counter, floor-item
cursor and free-effect selector. No opcode, immediate or control destination
differs. This is an unattributed residue; no register constraint or source
permutation is justified by it, and the function must remain unbanked.

The focused production match performs a real `game.render_scene` compile
and reproduces the retained score. Scoped modern type checking, Ruff, all
734 repository tests and `git diff --check` pass. The ten existing literal
ledger rows retain their expressions and reasons, with source lines refreshed.
An independent production audit verifies all 352 target words against GAME
retail, reproduces the retained trial's resolved body, and checks all thirty
remaining differences are confined to GPR fields.

The required full build retains the existing data/ownership/placement
failures: data comparisons pass for PSX 1/1, GAME 11/41 and OPEN 3/19 units,
with no artifact failures. Target relinking verifies 1/1, 75/77 and 34/38
units respectively. All thirteen vendored source controls remain exact.
No banked function regresses; exact counts remain PSX 1/1, GAME 337/362 and
OPEN 106/108, or 444/471 overall. Twenty-five GAME and two OPEN functions
still require strict closure.

## Window and complete-view pointer controls on master

Function Match Plan at `5491cad3`: refresh the six GAME views, all 352
retail words, the caller at `800202c0`, neighboring event/weapon emitters,
all seven callee implementations/contracts, shared layouts, source history
and the prior lifetime controls above. The current single-claim unit remains
1408 bytes / 99.517044%. Nine calls, 24 address pairs, the internal jump,
all branch destinations and 48-byte frame agree; thirty words differ in GPR
operands. The game pool policy and separate SetLightMatrix SDK attribution
remain unchanged. No source, signature, storage or inventory change is
authorized by the score alone.

The shared row/column correction motivates a separate three-state JSON test
of window-pointer ownership: retain five local `g` pointers, reuse one `g`
across the five passes, or reuse the initial `grid` pointer as well. Preserve
each existing assignment position, every color member, all bounds and loads;
do not hoist a current-window read across a call in source.

| Window pointer | Strict match | Bytes | Unequal retail words |
| --- | ---: | ---: | ---: |
| Existing locals | 99.517044% | 1408 | 30 |
| Shared across passes | 98.750000% | 1408 | 73 |
| Shared initial and pass pointer | 99.133520% | 1408 | 54 |

All three retain the nine ordered calls and 24 ordered address targets.
Neither trial changes the first difference at +5c, the saved Z origin.
Both add a window-register difference at +84/+88 in the map-object pass:
shared-pass uses a0 and shared-initial uses a1, against retail a2. Reject
both ownership forms; source remains canonical. A generated member-renaming
typo was corrected before this complete three-state result; that failed
compile is not a source verdict.

Next test a pointer to the actual four-byte `struct KfVecXZs view_cell`
subobject, retaining both signed-halfword members and their existing unsigned
reads. This differs from the rejected scalar-Z pointer: it never indexes
outside one field, and X/Z stay in their established owner. Bind it either
before the actor pass for its two reads, or before the origin calculations
for all four reads. Keep the window locals and every arithmetic expression
unchanged; do not compose either rejected shared-window trial.

The actor-only pointer emits 1408 bytes / 98.886360%. It anchors on X at
`8009575c`, before the actor-base pair, rather than retail's Z address
`8009575e` after that pair. Z uses +2 and X +0 instead of +0/-2. All nine
calls remain, but the address sequence differs. The initial-and-actor pointer
emits 1404 bytes / 97.119316% and only 22 address pairs, deriving its view
base from the active-window address. Its first difference moves to +28.
Reject both; neither preserves the independently observed address sequence.

Independent recompilation resolves every trial against a target separately
verified against all retail bytes. Both baselines reproduce the production
body exactly. The two valid JSON matrices are
`build/hypotheses/entity-window-lifetime-corrected` and
`build/hypotheses/entity-view-cell-owner`; raw objects and complete linked
word arrays are under `build/entity-window-objects` and
`build/entity-view-cell-objects`. The canonical source, profile and bank
remain unchanged. No compiler mechanism is inferred from these controls.
