# OPEN artificial vertex-address arithmetic review

Both cleanups remain open. The baseline at `941f2a26` is strict 100% for both
functions; replacing `base + v1 + (v2 - v1)` with the direct prepared-vertex
lookup loses those matches. No source, profile, inventory, or bank change is
retained, and the checklist still contains two cases.

| OPEN.EXE function | Direct lookup, strict objdiff | Frame: retail → direct | Remaining differences |
| --- | ---: | --- | --- |
| `render_enqueue_tmd`, `0x8001764c` | 99.965060% | 96 → 88 bytes | Saved-register homes and header/count spill offsets |
| `render_enqueue_unlit_triangles`, `0x80018344` | 99.455620% | 64 → 56 bytes | Saved-register homes and exchanged vertex-one/two registers |

The direct TMD form follows the other eleven arms' `offset + vertices` order.
Body sizes remain 3320 and 676 bytes. Raw comparison starts at the frame
instruction; calls, branch destinations, constants outside stack operands,
ordered text relocations, and TMD switch rows remain unchanged.

The native, debug, and traced GCC 2.5.7 builds emitted identical function bytes
and ordered text relocations for each baseline/direct control. Traces identify
the subtraction result as pseudo 77 in TMD and 103 in unlit. Both disappear
during `combine`, but global allocation still records four references and no
hard register for each. Reload then adds one unused eight-byte home, at
`sp+32` and `sp+16`, respectively. Direct controls have no such home.
The pinned source's `reload1.c` visits pseudos in ascending order and
`alter_reg` allocates a home for an unassigned, referenced pseudo without an
equivalent value. This explains the current probe result, not the original C.

This mechanism supplies no independently supported replacement. Moving the
redundant expression, hiding it in a helper, or manufacturing another dead
temporary would preserve the debt. Prior declaration, pointer-array, field
pointer, qualifier, and scheduling controls remain negative evidence in the
two function dossiers. Both banked functions stay unchanged.

Fresh focused builds reproduce both 100% baselines. The full 99-unit build
still fails its existing data/placement checks; no new build failure is
introduced by this documentation-only review.

## Follow-up

The [reconstruction debt review](reconstruction-debt-review.md) attributes
the extra slot to a pseudo whose reference count `combine.c` leaves stale
after a three-insn combination re-emits its first insn, shows that an unused
aggregate local leaves the same slot, and removes the owner recovery around
the arithmetic; a never-read local now carries the slot and both arms use
the direct lookup.
