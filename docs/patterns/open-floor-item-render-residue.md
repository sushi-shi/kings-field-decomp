# OPEN floor-item rendering: facing-byte mask

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
