# SDK vector publication and source order

## Function Match Plans at `806f1ed`

Two existing partials construct SDK vectors but spell their assignments in
retail's observed store order. The supplied `LIBGPU.H`, lines 79–80, defines
`setVector(v, x, y, z)` as comma-separated assignments to `vx`, `vy`, `vz`;
it does not initialize the padding member. This gives a specific source/API
hypothesis, not an arbitrary lane-order search. Keep the existing
`probe-gcc257-o2-g0` profile, types, globals and control flow unchanged.

For both functions, refresh the six image-qualified semantic views, full
retail disassembly/CFG, constraining caller, callees, adjacent boundaries,
shared layouts, source history and vendor evidence before editing. Compile
each real source change, compare from the first divergence, then resolve all
instructions and compare ordered relocation sites, kinds, symbols and encoded
addends against the delinked target. Preserve every exact sibling.

### OPEN `render_map_cell`, `80018bbc`/464 bytes

Baseline is strict 87.922420%, 472 compiled bytes. The body has nine calls,
nine validated HI16/LO16 address pairs, one validated internal jump, six
conditional branches and one shared return. No strings or candidate outgoing
references occur. Its 80-byte frame holds MATRIX at 16, SVECTOR at 48 and
the flag at 56; saved `s0/s1/s2/ra` occupy 64/68/72/76. Both early object
rejections reach the common restore tail, whose return delay slot frees the
frame.

The sole caller, exact `opening_render_map_cells` at `80018d8c`/320 bytes,
bounds full-width row/column to 0..99 and supplies an unsigned visibility
byte. Attribute selection decrements/wraps an unsigned byte before checking
99. Orientation is word-sized; height-grid loads are unsigned bytes. Position
is the SDK SVECTOR and the three camera inputs are narrowed to unsigned low
halfwords. Constants remain 2000, -100 and the orientation offsets of 2000.

The custom grid/object policy is not a vendored body. Its SDK matrix services
are separately identified in `functions_vendored.tsv`: SetRotMatrix,
SetTransMatrix and SetLightMatrix have Release 2.5 FID evidence; RotTrans and
MulMatrix0 have cross-overlay object-lineage evidence, with exact older SDK
revision unresolved. Their raw bodies and real prototypes support the current
MATRIX/SVECTOR/VECTOR interfaces. RotTrans writes only three words to MATRIX.t;
MulMatrix0 fills the rotation without overwriting those translation words.

Ignoring shifted branch destinations, the first baseline mismatch is +0x60:
retail puts `li t1,1` in the bounds-guard delay slot, while the old source has
a nop and a later `li a2,1`. Retail later stores position X/Z/Y. Test the
authentic `setVector` X/Y/Z assignment form, retaining all three expressions,
casts, orientation if/else arms, data owners, SDK calls and exact traversal.

### GAME `camera_path_step`, `80033680`/284 bytes

Baseline is strict 90.915490%, with the correct body extent. The single
proven call is `camera_path_compute_segment` at `800332e4`; there are no
address pairs, strings or candidate references. Three conditional branches
implement the initial -1 sentinel, countdown expiry and post-segment sentinel.
Both early exits share the normal restore tail. The 32-byte frame saves
`s0/s1/s2/ra` at 16/20/24/28; the decrement is stored in a branch delay slot,
and the return slot releases the frame.

The sole caller, `map_floor5_transition_cutscene`, passes its 100-byte stack
state at sp+56 and zero height offset at `80034718/1c`. It tests state+96
after the call and sends position/rotation to the renderer. Segment/begin
siblings prove the shared VECTOR position, SVECTOR rotation, word-sized Q4
accumulators/deltas and signed count. The camera policy is game-owned, not a
vendored routine; only the segment helper's SquareRoot0 is separately
attributed to SDK LIBGTE/MSC.

All six accumulator updates already agree. The first differing word is
+0xb4: retail preloads rotation-Y from state+48 into `a0` before publishing
position X/Y. Retail then stores rotation Y, position Z, rotation X/Z; the
old C follows that store order but loads rotation Y too late. Test two SDK
`setVector` calls, publishing position XYZ then rotation XYZ. Keep the
accumulator stage, Q4 shifts, signed height addition and `0xfff` rotation masks
unchanged. The 732-byte segment and 192-byte begin siblings must remain exact.

## Results and discriminating control

Both SDK-source trials are strict objdiff **100%**. OPEN map cell becomes
464 bytes with all 116 raw words and 28 ordered relocation rows exact;
its 320-byte traversal retains all 80 words and 21 relocation rows. GAME
camera step retains 284 bytes with all 71 words and its one relocation exact;
the 732/192-byte siblings retain all words and five relocation rows each.

In a temporary source control, replace only the macro invocations with separate
semicolon-terminated assignments in the same natural XYZ order. All five
functions still reproduce every linked instruction and ordered relocation.
Thus the source order is sufficient under this probe; comma-expression
boundaries or unique historical use of `setVector` are not established.
Retain the concise authentic SDK spelling, without claiming compiler/version
attribution from it.

The compiler emits OPEN's X/Z/Y stores and GAME's interleaved position/rotation
stores from these natural grouped assignments. Raw store order therefore did
not prove C statement order. In particular, the prior camera follow-up in
[game-fog-interpolation.md](game-fog-interpolation.md) rejected a useful source
hypothesis prematurely by treating those orders as identical. This new
controlled compile supersedes that conclusion. Earlier map trials and ownership
work remain documented in [open-map-render-residue.md](open-map-render-residue.md).

The existing OPEN runtime-owner control now checks the entire map-cell body,
not only its matrix/epilogue suffix. No shared layout, inventory, relocation,
compiler setting or unrelated function body changes are required.

## Campaign verification

The full 484-row report changes only these two functions: GAME advances
290 -> 291/362 exact and OPEN 97 -> 98/108, with 71 and 10 partials remaining.
All three previously exact siblings and all thirteen vendored-source controls
remain exact. The two additions account for 748 retail code bytes.

Ruff, all 642 repository tests (79.496 seconds) and `git diff --check` pass.
Full `kf build` still fails its pre-existing data/closure/placement gates:
5/60 source data-owning units match, 4/4 independent provider contributions
match, and target relinking verifies 110/116 units with six conflicting
section bases. There are no artifact failures or new exact-function losses.
Only GAME `80033680` and OPEN `80018bbc` are selected for banking.
