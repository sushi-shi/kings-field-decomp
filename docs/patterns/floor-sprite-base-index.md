# Floor-decoration base sprite index

## Function Match Plan

Rename the first halfword of `KfFloorItemPlacement` and `KfFloorItem` from
`item_id` to `base_sprite_index`, following the renderer's actual consumer.
Propagate the name through both placement loaders, both floor renderers, shared
field metadata and the existing layout fixture. Keep unsigned-halfword storage,
the placement terminator, all values and expressions. No inventory-item enum
belongs on this sprite-table index.

Current source/history and the cached image-specific disassembly/CFG, callers,
callees, strings and historical match records were reviewed. The existing
[descriptor/resource evidence](game-entity-sprite-tables.md) and
[OPEN loader review](open-item-cursor.md) establish this game-owned family and
its adjacent render/resource functions. SDK transforms and the random-number
service remain external. Historical match records are not verification of edits.

| Image / function / extent | Evidence snapshot |
| --- | --- |
| GAME `item_load_floor_placements`, `0x80020b4c / 0x1b0` | A 40-byte frame, one proven call and three validated address pairs; no strings. Reads the first unsigned halfword for the 0xffff terminator at `0x80020b78`. Copies placement +0 to runtime +0 with `lhu`/`sh` at `0x80020bdc`/`0x80020be4`. Rename all three sentinel reads and the copy. |
| OPEN `item_load_floor_placements`, `0x800197e4 / 0x1b0` | The homologous 40-byte-frame loader has one proven call, three validated pairs and no strings. Its first terminator read is at `0x80019810`; `0x80019874`/`0x8001987c` copy the same halfword. Preserve its direct count-member accesses and cursor/delay-slot structure. |
| GAME `render_floor_item`, `0x8001ed90 / 0x14c` | An 80-byte frame, nine proven calls, six validated pairs and no strings. `lhu` at `0x8001ee70` reads +0 and `lbu` at `0x8001ee74` reads frame +20. Their sum is scaled by twelve, added to sprite-table base `0x80055b00`, and passed to the sprite enqueuer at `0x8001ee94`. Rename only the base index. |
| OPEN `render_floor_item`, `0x800190f4 / 0x14c` | The same frame/call/reference shape. Reads base/frame at `0x800191d4`/`0x800191d8`, scales their sum by twelve and uses OPEN's table at `0x800358e0` for the enqueue call at `0x800191f8`. Rename only the base index. |

## Domain correction

The first halfword selects the beginning of a sprite-animation sequence. The
placement review found base zero with four frames, and base four with three
frames in the seven-record logical descriptor table. Both images carry the same
descriptor bytes. The sprite sequence arithmetic establishes this role directly;
it does not identify inventory entries such as Short Sword or Morning Star just
because their database IDs happen to be zero and four.

The field stays an integer because it is an arithmetic table index, with the
frame byte added before addressing. The loader's 0xffff value terminates this
placement stream; inventory's separate `KF_ITEM_NONE` remains 0xff. Consequently,
these floor fields do not require a halfword inventory-ID domain. This corrects
the provisional item-ID reasoning recorded in the stock-bank review.

The current random initial frame can exceed the seven logical descriptors for
some packed facing/count bytes, as documented in the descriptor review. This
rename preserves that calculation, draw-before-wrap order and table extent.
It does not infer a visual asset name or assign meaning to unconsumed byte +3.

Builds, compiler checks, tests, post-edit matches and banking remain deferred
until naming is finished. The existing layout fixture needs only the renamed
field reference; no new check is introduced.

## Source result

Both shared fields, all twelve source uses in four functions, the two metadata
rows and the existing fixture reference now say `base_sprite_index`. Reversing
only that spelling and the header explanation reproduces the four C files,
header and fixture exactly. Metadata retains its supported tier, byte offsets
and unsigned-halfword widths while citing the consuming renderers.

All 111 C files and 6,007 retained literal occurrences remain accounted for;
this identifier correction removes no numeric values. Each of the four functions
above remains unverified after editing. No build, compiler check, test, post-edit
match or banking ran, and source review is not a binary-match claim. Inventory
ID typing and the remaining unknown fields are still unfinished.
