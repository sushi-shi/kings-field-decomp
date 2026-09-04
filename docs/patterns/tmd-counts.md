# TMD format counts versus consumer widths

`KfTmdHeader.object_count` and `KfTmdObject.primitive_count` are unsigned
32-bit disk fields, at offsets `0x08` and `0x14`. The previous model split
each into a low halfword and an invented `*_high` member. Commit `20c4d84`
introduced that split while reconstructing the preparation routine; its
halfword load does not establish a halfword field in the file format.

Sony's November 1998 *File Formats*, p. 2-25, describes a three-word header
and a seven-word object record with an unsigned word primitive count. It
also distinguishes relative object-block offsets from fixed-up pointers.
This corroborates the disk layout, not the compiler or SDK revision used
for the 1994 game. The pinned SDK header search did not supply a matching
object-record typedef; the project keeps its explicit fixed-width disk
view instead of inventing a vendor declaration.
[Sony File Formats](https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/Filefrmt.pdf#page=69).

## Retail consumers

The five-function evidence snapshots and verdicts are in
`config/evidence/game_semantic_tmd_counts.tsv`.

| Consumer | Retail count access | Source contract |
| --- | --- | --- |
| GAME preparation, `0x8001c2b0` | `lhu` at `0x8001c2bc` and `0x8001c2e4` | explicitly narrow both disk counts to `u16` |
| OPEN preparation, `0x80017030` | `lhu` at `0x8001703c` and `0x80017064` | explicitly narrow both disk counts to `u16` |
| GAME native emitter, `0x8001c7f8` | `lw t0,20(v0)` at `0x8001c830` | full `u32` primitive count |
| GAME model emitter, `0x8001d730` | `lw t0,20(v0)` at `0x8001d768` | full `u32` primitive count |
| GAME map emitter, `0x8001de18` | `lw s7,20(s0)` at `0x8001de74` | full `u32` primitive count |

The preparation loops mask their countdown tests to sixteen bits. The
emitters do not: their word countdowns test zero and decrement without a
halfword mask. Their local counters now use `u32`, matching the unsigned
format field and giving defined wrapping arithmetic. No call, packet
constant, source branch, address claim, or relocation target was changed.
The two record sizes remain `0x0c` and `0x1c`; other fields retain their
offsets. No source initializer or use of either invented high member exists.

This distinction matters even though an ordinary small asset has zero high
bits: the old emitter source silently truncated a count that retail loads
in full. An on-disk field must not encode one consumer's narrowing policy.

## Header ownership

The eight common TMD function declarations now live beside their shared
types in `tmd.h`, instead of being duplicated in `game_render.h` and
`open_render.h`. The signatures agree with both image-qualified identity
inventories. Declaration parameters use the curated `slot` and
`object_index` names. This is a shared interface, not merged executable
state: the different GAME/OPEN state layouts remain in their respective
headers, as do GAME-only shift-projection and non-perspective operations.

## Match verdict

Both preparation routines retain their halfword count loads and previous
scores: GAME remains 94.135414%, OPEN 98.333336%. Their missing eight-byte
frames and other existing differences are not fixed by correcting the
disk layout.

The three emitters now use word loads. Their strict objdiff fuzzy scores
are 52.039013%, 60.490950%, and 17.954199%, respectively, versus
52.032856%, 60.764706%, and 18.305344% before. The lower intermediate
scores do not invalidate the directly evidenced width correction. The
first remaining differences are frame size or current-asset/base setup;
packet-dispatch and shared-object/address-formation differences remain
unresolved. No compiler mechanism or exact closure is claimed.

All 340 historically exact non-vendored functions remain exact (GAME 260,
OPEN 79, PSX 1). All 55 data-owning units pass strict data comparison.
Vendor exclusions are unchanged; none of these five routines is newly
counted as game progress.
