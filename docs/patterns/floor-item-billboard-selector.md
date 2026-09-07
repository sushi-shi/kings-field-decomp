# Floor-item billboard selector

## Function Match Plan

Name the zero facing selector `KF_FLOOR_ITEM_FACING_BILLBOARD` in the shared
item constants and both `render_floor_item` consumers. Keep the packed byte,
the unsigned-halfword local, its byte-narrowed comparison and the existing yaw
arithmetic. The byte combines a facing nibble with a frame count; giving that
whole field a facing enum would conflate two independent values.

| Image / function | Retail evidence |
| --- | --- |
| GAME `0x8001ed90`, size `0x14c` | At `0x8001ee14..24`, load byte +2, mask with `0xf0`, narrow with `0xff`, then branch on zero. The zero branch uses the pitch matrix and depth bias 200; the other subtracts 16, shifts six, and composes the yaw with the view matrix. |
| OPEN `0x800190f4`, size `0x14c` | The same operations at `0x80019178..88`, with the zero branch at `0x800191bc`, using OPEN's own graphics owner. |

Each complete disassembly/CFG has an 80-byte frame, nine proven calls, six
validated data pairs and one validated internal jump, with no string references.
The proven callers are GAME `render_entities` at `0x8001f560` and OPEN
`opening_render_entities_and_items` at `0x80019488`. The existing
[descriptor/placement evidence](game-entity-sprite-tables.md), source history,
adjacent entity renderers and loader dossiers establish the packed-byte flow.
This is game rendering policy around the authentic SDK matrix operations.

The previous ledger called this a mathematical zero predicate. Its actual
branch role supports a useful semantic name: billboard orientation differs
from fixed yaw zero (`KF_FLOOR_ITEM_FACING_ZERO_YAW`, encoded as `0x10`).
The zero selector warrants a name even though the arithmetic frame reset
elsewhere remains literal. No new angle names or resource restrictions are
inferred; all other high-nibble encodings retain the existing conversion.

## Verification boundary

Review by reversing the two name substitutions and checking the shared
constant's value. Reconcile their two removed literal rows. Builds, compiler
checks, tests, post-edit matching and banking remain deferred until the naming
pass finishes. Stored match results are not verification of this edit.

Source review passed: reversing the names exactly recovers both prior source
files, and the header only adds the zero-valued constant. All 111 current C
files retain complete token/expression accounting for 6,100 occurrences. Neither
renderer has post-edit binary verification. The naming goal remains open.
