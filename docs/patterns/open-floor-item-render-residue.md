# OPEN floor-item rendering: facing-byte mask

## Exact OPEN closure: 16-bit value, byte consumption

OPEN `800190f4 render_floor_item`, 332 bytes, began at strict 98.795180%.
Its nine calls, six validated address pairs, one internal jump, 80-byte frame,
signed depth biases, sprite selection and animation tail already agreed. The
only missing retail instruction was the unsigned-byte narrowing between the
high-nibble mask and its zero test.

Retail's two operations describe distinct domains: the decoded facing value is
held as `u16`, while the branch consumes it as `u8`. Declaring the real local
accordingly and spelling that consumption as `(u8)facing` emits the observed
`andi 0xf0; andi 0xff` sequence. A focused rebuild is strict **100.000000%**:
all 332 bytes, calls and relocations match, and the exact preceding entity
renderer remains unchanged. The function is banked.

## GAME/OPEN Function Match Plan: one-based facing index at `29d1ddb`

Both image-qualified six-view snapshots were refreshed against hash-verified
retail, together with their caller argument windows, matrix-Y/SDK boundaries,
placement initializers, shared record, neighbors and source history. GAME
`8001ed90` and OPEN `800190f4 render_floor_item` retain a 332-byte retail extent,
328-byte candidate, strict 98.795180%, nine calls, six address pairs, six
blocks, two branches, one internal jump and 80-byte frame/return slot. There
are no strings, candidates or indirect transfers. Custom facing/animation
policy is game-owned; matrix and GTE services retain their SDK attribution.

The packed high nibble encodes zero for billboarding and nonzero values for
biased quarter turns: `((packed & 0xf0) - 16) << 6` equals
`((packed >> 4) - 1) << 10` on every nonzero high nibble. Test decoding that
one-based index into the existing byte local, testing its old value while
post-decrementing, then using its zero-based value for the angle. The zero
case still takes the billboard path and never consumes the wrapped index.
This differs from updating the masked byte by sixteen or reusing a frame
temporary; it asks whether the actor descriptor's consumed byte-index form
also explains this format. Keep all calls, reads, writes, owners and the
animation tail unchanged. No artificial mask, carrier or padding is added.

Compare the entire raw body, ordered targets and both units' siblings after
real compilation. In particular, extra shifts or a moved narrowing reject
the hypothesis even if the total size equals 332. Only strict 100% and full
verification can justify banking; semantic equivalence alone is insufficient.

Both image-specific trials emit 328 bytes. The first difference moves to
+0x88: the matrix-address setup replaces the load-delay nop, followed by
`srl 4`, a zero branch with `addiu -1` in its slot, then `andi ff` and
`sll 10`. Retail instead retains `andi f0; andi ff` before its guard and
`addiu -16; sll 6` after it. All nine calls and six ordered numeric data
targets agree; both exact neighboring renderers and OPEN's traversal retain
their complete baseline words/references. Reject this source form in both
images. The actor's index result does not transfer to this facing decoder;
no floor-item C, header, profile or banking change is kept.

Verification of these floor and OPEN traversal controls: fresh complete-unit
compiles preserve the canonical source streams, and all 484 strict report
scores equal the pre-trial snapshot. GAME remains 312/362 exact and OPEN
98/108; neither floor renderer nor the traversal is newly banked. Ruff and
`git diff --check` pass, and all 680 tests pass in 83.582 seconds. Full
`kf build` retains the existing data/ownership/placement failures: source
data PSX 0/1, GAME 9/42, OPEN 2/19; target relink 1/1, 75/77, 34/38;
zero artifact failures. Concurrent effect naming work is separate from
these documentation-only findings, and subsequent unrelated source edits
are not part of this campaign's verification claim.

## GAME/OPEN Function Match Plan: byte temporary lifetime at `11552f7`

The exact actor-index correction at `d7e0260` motivates a new lifetime
question, not another spelling of the same mask. Refresh both images' six
semantic views, complete bodies and CFGs, sole 24-byte-stride callers
(`GAME:8001f560`, `OPEN:80019488`), exact neighboring renderers, placement
loaders, shared item layout, matrix-Y signed-halfword boundary, SDK headers
and vendored provider rows. The game-owned placement/facing/frame policy is
not a vendored body; its `LIBGTE` MTX/SMP services remain separately attributed.
There are no strings, candidates or indirect transfers in either target.

| Image | Function | Retail / current bytes | Strict baseline |
| --- | --- | --- | --- |
| GAME | `8001ed90 render_floor_item` | 332 / 328 | 98.795180% |
| OPEN | `800190f4 render_floor_item` | 332 / 328 | 98.795180% |

Each body has nine direct calls, six address pairs, six blocks, two
conditional branches, one internal jump and one common return with the
80-byte frame restoration in its delay slot. The complete linked bodies
currently differ only by the missing `andi ff` at +90 and shifted internal
destinations. Preserve every load/store, selected image's owner, sprite data,
150/200 depth biases, high/low nibble masks, signed angle API and draw-before-
animation-update order. The initializer's full-packed-byte random seed is
unchanged; this trial must not silently repair its initial-index limitation.

Retail's facing test and post-draw frame increment both consume byte values;
the increment is stored and narrowed before its unsigned frame-count compare.
Test using one consumed byte temporary for those successive, non-overlapping
phases rather than the current `u8 facing` plus separate `u32 next_frame` and
explicit `& ff`. Independently test a byte-sized but separate `next_frame`
to distinguish byte width from source-variable lifetime. Both candidates
retain the post-call packed-byte read and defined byte wrap, remove no real
side effect, and add no local, volatile access, synthetic mask or padding.
Keep only humane source supported by full linked words and ordered referents;
preserve the exact GAME actor-sprite / OPEN entity siblings and the current
OPEN traversal bytes. Rebuild both affected units, require strict 100% for
banking, and run the full project checks before handoff.

The separate-byte frame control leaves the OPEN instruction stream unchanged.
Reusing that byte across the draw boundary does not restore the facing mask
and additionally changes the animation-tail load order/registers. Reject
cross-phase reuse. A second, narrower question stays within the facing phase:
retail subtracts the biased zero-yaw value after its nonzero guard. Test
updating the existing facing byte by that subtraction, then consuming it in
the angle shift. For every permitted nonzero high nibble the result is
0..224, so no overflow or angle behavior changes. Preserve the original
animation temporaries; reject the trial if it introduces additional narrowing
or otherwise fails the complete-body comparison.

The same complete-unit controls were compiled separately for GAME and OPEN.
Both images give the same verdict: separate byte frame is byte-identical to
baseline (328 bytes); shared cross-phase byte is 332 bytes but retains the
original omission and changes the animation tail; byte facing subtraction is
332 bytes but emits `andi a0,a0,ff` after subtraction instead of retail's
pre-guard narrowing. Its four unequal aligned words are at +90..+9c. Neither
same size nor improved listing similarity makes that an exact match.
All nine ordered calls and six numeric address pairs remain correct in every
trial. Both exact neighboring bodies and the OPEN traversal's baseline
instruction/reference stream are unchanged. No floor-renderer source edit
is retained and neither function is banked.

## Function Match Plan: packed-control value (`5784c14`)

OPEN `800190f4` remains 332 retail/328 probe bytes, strict 98.795180%.
Hashes, six semantic views, full CFG, sole traversal caller, neighboring
entity/sprite bodies, placement loader, shared record, matrix-Y boundary,
SDK declarations/provider evidence and history were refreshed. Nine calls,
six validated address pairs, one internal jump, no strings/candidates, the
80-byte frame and the established ABI retain the contract below.

The unsigned byte at item+2 supplies facing through mask `f0`, and is read
again after sprite emission for the frame-count mask `0f`. The loader copies
the entire serialized byte; its random-frame initialization must continue
reading that entire value. Test one complete byte-sized union view with raw
storage and unsigned four-bit facing/frame-count fields. Preserve both reads:
the high field shifted by four feeds the existing u8 facing; the low field
replaces the later raw temporary and mask. Keep the record layout and all
other expressions unchanged, with a one-byte pinned-compiler size assertion.
No forced mask, helper, volatile, extra frame object or alternate record view
is justified by the missing instruction.

The packed-control value emits identical resolved words and ordered targets
for all three unit functions. The floor renderer remains 328 bytes, the
entity sibling remains 552-byte raw exact, and traversal remains 668 bytes.
The trial is not adopted. A separate compile retaining the pre-assembler
output confirms `andi $2,$2,0x00f0` directly precedes the zero branch;
`andi ff` is already absent there. The assembler is not removing an emitted
narrowing instruction. This locates the observed omission in the probe
pipeline without identifying the historical compiler or an optimizer cause.

## Function Match Plan: branch assignment value (`819211b`)

OPEN `800190f4` remains 332 retail/328 probe bytes, strict 98.795180%.
Retail hashes, all six semantic views, complete CFG, the sole 24-byte-stride
caller, both neighboring functions, placement loader/shared record, matrix-Y
and sprite consumers, SDK declarations/provider evidence and source history
were refreshed. Nine calls, six address pairs, one internal jump, no strings
or candidate references, the 80-byte frame and complete delay slots retain
the supported contract below. The SDK services are separate vendored bodies;
the placement/facing/animation policy is game code.

The first difference is still the missing byte narrowing at `80019184`.
Retail reads, masks and narrows the facing value directly into its zero test,
then uses that same value in the angle expression. Test the ordinary C
assignment-expression form of that branch: assign the masked byte to the
existing `u8 facing` within the condition itself. Unlike the rejected
byte-read/compound-mask trial, the branch consumes the assignment's narrowed
value rather than a subsequent local read. Keep the mask, record, angle,
signed depth biases, SDK calls and animation tail unchanged. This is a source
hypothesis, not proof of the original spelling or a reason to add a synthetic
mask. Compare the entire unit and keep its exact entity renderer unchanged.

The assignment-expression branch emits the identical 328-byte candidate and
ordered relocations, with the same omitted mask. All 552 bytes of the exact
entity renderer and the traversal's existing 668-byte candidate are unchanged.
Restore the separate assignment; no source or match change is kept here.

## Function Match Plan: byte read followed by nibble masking (`8ac47fe`)

OPEN `0x800190f4`, 332 retail bytes, starts at strict 98.795180% with a
328-byte candidate under `probe-gcc257-o2-g0`. The six semantic views,
complete retail CFG, sole traversal caller, neighboring entity/traversal
source, placement loader and shared item fields, sprite helper, matrix-Y
callee, SDK declarations/provider ledger and source history retain the
contract below. There are nine direct calls, six validated address pairs,
one validated internal jump and no strings or candidate outgoing references.
The typed item/sprite and SDK matrix interfaces are unchanged. The frame is
80 bytes and the return delay slot restores it; branch displacements account
for the absent `andi v0,v0,0xff` at `0x80019184`.

The packed-control byte is read with `lbu`; its low nibble later limits the
byte animation counter and its high nibble selects the facing branch/angle.
Test reading the full packed byte into the existing `u8 facing` and then
applying `facing &= 0xf0`, rather than assigning a masked promoted expression
directly. This distinguishes a byte read/update from the previous bit-clear
constant trial without adding a carrier, changing signedness, altering an
owner or editing GAME. Compare raw instructions and ordered referents after
a real focused build, and preserve the exact neighboring entity function.

The separate byte read/mask produces the same missing-`andi` diff: 328 bytes,
the same nine numeric calls and six numeric address targets. The preceding
552-byte entity renderer remains raw-exact, and the traversal's size/raw
differences are unchanged. Remove the trial; the byte-update spelling does
not explain the narrowing instruction under this probe. No source edit is
retained for this function.

## Retail and source contract

`OPEN.EXE:0x800190f4` is the 0x14c-byte `render_floor_item` body. Its sole
caller walks the 24-byte `floor_items` records, and the placement loader
establishes the same `KfFloorItem` fields used by GAME. The renderer transforms
the position, chooses a facing rotation or the pitch matrix, draws the sprite,
then increments and wraps its byte animation frame.

The projection helper at `OPEN.EXE:0x800189a0` consumes all eight fields of the
12-byte `KfSpriteQuad`. Its second argument is a signed ordering-table depth
bias, not a geometric scale: `sll`/`sra` at 0x80018b6c/0x80018b70 sign-extend
it immediately before addition to the projected origin depth. The third
argument selects a 1.5-times fog interpolation term when equal to one.

The exact neighboring `opening_entity_render` and this body share a caller,
view-state ownership and transform setup. `open.entity_render` therefore owns
their contiguous run; this is a WIP grouping, not proof of an original TU.

## First divergence

With `probe-gcc257-o2-g0`, strict objdiff is **98.795180%**. Retail contains:

```text
OPEN 0x80019178  lbu   v0,2(s1)
OPEN 0x8001917c  nop
OPEN 0x80019180  andi  v0,v0,0xf0
OPEN 0x80019184  andi  v0,v0,0xff
OPEN 0x80019188  beqz  v0,0x800191bc
```

The reconstructed source uses `u8 facing = item->facing_and_frame_count &
0xf0` (assignment follows the transform setup). The probe omits the second
mask and emits 0x148 bytes. All remaining instructions and ordered referents
agree, apart from branch/jump offsets shifted by that four-byte omission.
The 80-byte stack frame, SDK calls, signed depth-bias values 150/200, sprite
index stride and byte animation wrap all agree. The table bytes also match.

GAME's homolog at 0x8001ed90 contains the same extra mask at 0x8001ee20.
The complete retail pair has 59/83 identical words and 83/83 identical
opcode/register shapes. The differing immediates are image-specific link
targets and matrix offsets: GAME's view-position field follows two additional
matrices. `kf lineage` verifies the recorded comparison.

This is an **unattributed code-generation residue**. The precise historical
compiler/profile and the mechanism producing the extra mask are unresolved.
No fake local, volatile qualifier, forced assembly or alternate object layout
is justified. This function is not banked as exact.

## Sprite table and resource control

`floor_item_sprites` owns the seven consistent 12-byte records at
OPEN 0x800358e0..0x80035934. The first four share a 1024-by-1024 local rectangle
and the U sequence 0x90, 0xb0, 0xd0, 0xb0. The next three share a
1024-by-1280 rectangle and U sequence 0x90, 0xb0, 0xd0 at V 0x20. Following
bytes do not follow the same descriptor shape and remain unowned. Seven is a
supported working extent, not a recovered original array declaration.

`B0/MIXA0.` (564052 bytes, SHA-256
`4221c17d405e2b85d63f925977cc1d4418a5e35ecd0d2c355bfbf8caa7718526`)
contains its floor-item chunk at file offset 0x8996c, after the two VAB chunks
and map grids. It has 14 placements followed by the sentinel. Every placement
uses item ID zero and packed facing/frame count 0x04, confirming active use
of the first four descriptors. The last three are retained from their complete
record shape; this resource does not prove their runtime use.

## Related cleanup control

`TMD_PACKET_BODY` in the OPEN TMD decoder is invoked only with a local `u8 *`.
Removing its same-type cast leaves the compiled `open.render` object
byte-identical in a controlled old/new rebuild (SHA-256
`6d27d812e3cb0092db5de752109708ac8d07ecc60f1a4337954c1cbbc1b48fd3`).
The decoder stays at 98.333336%; its existing frame/order residue is unchanged.

## Master follow-up: clearing the frame-count nibble

Pre-edit OPEN `800190f4`/`0x14c` at `2560194`: 98.795180% strict, 328 probe
bytes versus 332 retail. The refreshed six image-qualified queries, complete
disassembly/CFG, caller at `80019488`, adjacent entity/traversal code, shared
item fields and loader, source history, GAME source homolog and pinned SDK
MTX/SMP headers and provider symbols retain the existing contract. Nine
direct calls, six address pairs, one internal jump, all frame/delay slots,
sprite table ownership and numeric referents agree; there are no strings or
candidate references. The item policy is game-owned, not an SDK body.

The packed byte's low nibble is the frame count, independently consumed by
`andi ...,0xf` at `80019214`; the high nibble controls facing. Test clearing
the low nibble (`value & ~0x0f`) before assignment to the existing `u8 facing`,
rather than selecting it with the already byte-sized `0xf0` constant. This
is the same operation over every possible byte and preserves the subsequent
angle, branch and frame-wrap behavior. The hypothesis is that the promoted
bit-clear expression followed by byte assignment may explain the two retail
masks; it is not a proved original spelling. No width, field, owner, compiler
profile, extra statement or GAME source is changed. Compare the complete
instruction/relocation stream and both neighboring functions after rebuilding.

The bit-clear expression produced the same instructions and relocations as
the original high-nibble mask. The extra retail `andi 0xff` is still omitted;
strict scores remain 98.795180% for this function, 100% for the preceding
entity renderer and 92.867470% for the following traversal. The experiment
was reverted. It rules out this source spelling under the pinned probe, not
other independently supported source facts or a historical compiler variant.
