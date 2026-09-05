# OPEN floor-item rendering: facing-byte mask

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
