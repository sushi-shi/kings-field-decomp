# GAME/OPEN memory allocator

GAME and OPEN contain the same seven allocator functions in the same order,
with a constant address delta of `0x4cdc`. All 144 instruction shapes match.
Linked call and data addresses differ, and the mode-1 arena span is deliberately
different: GAME reserves `0xff000` bytes while OPEN reserves `0x113000`.

The curated function identities are:

| Action | GAME | OPEN | Signature |
| --- | ---: | ---: | --- |
| checked Psy-Q allocation | `0x8001aab0` | `0x80015dd4` | `void *memory_malloc_checked(s32 size)` |
| reset LIFO state | `0x8001aae8` | `0x80015e0c` | `void memory_allocation_reset(void)` |
| select allocation mode | `0x8001ab08` | `0x80015e2c` | `void memory_set_allocation_mode(s32 mode)` |
| capture heap start | `0x8001abb0` | `0x80015ed4` | `void memory_capture_system_heap_start(void)` |
| reset Psy-Q heap | `0x8001abd0` | `0x80015ef4` | `void memory_reset_system_heap(void)` |
| allocate and push | `0x8001ac0c` | `0x80015f30` | `void *memory_allocate(s32 size)` |
| release last | `0x8001ac8c` | `0x80015fb0` | `void memory_release_last(void)` |

## State extents

| Identity | GAME | OPEN | Size | Type |
| --- | ---: | ---: | ---: | --- |
| `memory_arena_start` | `0x800a01f0` | `0x80075848` | `0x04` | `u8 *` |
| `memory_arena_end` | `0x800a01f4` | `0x8007584c` | `0x04` | `u8 *` |
| `memory_arena_cursor` | `0x800a01f8` | `0x80075850` | `0x04` | `u8 *` |
| `memory_allocation_depth` | `0x800a01fc` | `0x80075854` | `0x04` | `s32` |
| `memory_allocation_entries` | `0x800a0200` | `0x80075858` | `0x40` | `u32[16]` |
| `memory_system_heap_start` | `0x800a0240` | `0x80075898` | `0x04` | `u8 *` |
| `memory_system_heap_size` | `0x800a0244` | `0x8007589c` | `0x04` | `s32` |

The entry-array extent is exact: both push and pop index it with a four-byte
stride, and the next independently referenced state begins at element 16. Its
element meaning depends on the active mode. Arena allocation stores an aligned
byte count so pop can rewind the cursor; system-heap allocation stores the
returned pointer so pop can call `free`.

These are separate `data_identities.tsv` rows, not fields of a speculative
`KfMemoryAllocator` structure. A structure promotion requires independent
complete-object evidence such as a passed base pointer, whole-object copy or
clear extent, allocation size, or source/header identity. The current code uses
absolute globals and derives the entry-array base from the depth address, which
proves layout relationships without proving original aggregate ownership.

## Vendored boundary

The allocator policy is game-owned. Its `malloc`, `InitHeap`, and `free`
callees are Sony/Psy-Q library code. The GAME `0x800502dc` and OPEN
`0x80030050` `malloc` stubs each match all 16 bytes of Release 2.5
`LIBAPI.LIB/C51`, including the four zero padding bytes after the 12-byte
function extent. They therefore live in `functions_vendored.tsv` and are not
part of the game-function denominator.

The per-function review ledger is
`config/evidence/game_open_semantic_memory_allocator.tsv`; instruction-shape
proof is in `config/evidence/overlay_lineage.tsv`.
