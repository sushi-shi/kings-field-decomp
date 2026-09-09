# Allocator and startup retained-literal ledger

Complete accounting for four source files and **25 retained occurrences**.
The shared allocator compiles for GAME and OPEN, so these four files cover
five source/image variants. See the [allocation-mode evidence](allocation-mode-domain.md).
Each duplicate token has its own row. Claims and named definitions are excluded.

The mode values were already named; the current change separates them into a
strict enum domain and types the API. It does not reduce the inline-token count.
The callers in GAME `game.c` and `resources.c`, and OPEN `opening_controller.c`
and `resources.c`, remain covered by their existing complete ledgers.

## `src/game/memory.c`

11 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `memory_malloc_checked` | 32 | `1` | `if ((u32)block + MEMORY_CACHED_RAM_BASE > MEMORY_MAIN_RAM_BYTES - 1) {` | Convert the named RAM byte extent to its inclusive last-byte offset. Preserve the original unsigned 32-bit wraparound in the address test. |
| `memory_malloc_checked` | 33 | `0` | `return 0;` | Null pointer result when the malloc return fails the address-range guard; not an allocation mode or numeric error code. |
| `memory_allocation_reset` | 41 | `0` | `memory_allocation_stack[KF_MEMORY_STACK_DEPTH_INDEX] = 0;` | Reset the LIFO depth to an empty count without freeing entries. |
| `memory_set_allocation_mode` | 60 | `0` | `memory_arena_cursor = 0;` | Null cursor selects checked heap allocation. This is pointer state, not another integer allocation-mode member. |
| `memory_capture_system_heap_start` | 68 | `1` | `memory_system_heap_start = memory_arena_end + 1;` | Advance from the inclusive arena last byte to the first system-heap byte. |
| `memory_allocate` | 89 | `0` | `if (*cursor == 0) {` | Null-pointer test selects the heap path; non-null selects arena bump allocation. |
| `memory_allocate` | 94 | `1` | `size = (size + (MEMORY_ALLOCATION_ALIGNMENT - 1))` | Alignment-minus-one is the ordinary rounding adjustment for the named four-byte alignment; retain signed expression grouping. |
| `memory_allocate` | 95 | `1` | `& ~(MEMORY_ALLOCATION_ALIGNMENT - 1);` | Alignment-minus-one forms the low-bit mask removed after rounding; retain the signed complement. |
| `memory_allocate` | 99 | `1` | `memory_allocation_stack[KF_MEMORY_STACK_DEPTH_INDEX] = depth + 1;` | Push one LIFO entry. The original implementation has no depth-capacity guard. |
| `memory_release_last` | 107 | `1` | `s32 depth = memory_allocation_stack[KF_MEMORY_STACK_DEPTH_INDEX] - 1;` | Pre-decrement one LIFO entry before lookup. The original implementation has no underflow guard. |
| `memory_release_last` | 112 | `0` | `if (memory_arena_cursor == 0) {` | The current null cursor chooses free rather than arena rewind; entries do not carry independent mode tags. |

## `src/psx/main.c`

8 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 8 | `20` | `const char overlay_path_GAME[20] = "cdrom:GAME.EXE;1";` | Measured twenty-byte storage: sixteen pathname characters, a terminator and three trailing zeros. The original array/alignment rationale is unproven. |
| `initializers` | 9 | `20` | `const char overlay_path_OPEN[20] = "cdrom:OPEN.EXE;1";` | Measured twenty-byte storage: sixteen pathname characters, a terminator and three trailing zeros. The original array/alignment rationale is unproven. |
| `main` | 30 | `1` | `if (Load(overlay_path_table[KF_OVERLAY_OPEN_PATH], &header) == 1) {` | Preserve the BIOS Load success result. The boundary retains the vendor integer API rather than introducing a project enum for its return. |
| `main` | 32 | `0` | `header.s_addr = 0;` | Explicitly zero the SDK EXEC stack-address field before Exec; do not reinterpret it as an overlay request mode. |
| `main` | 33 | `0` | `header.s_size = 0;` | Explicitly zero the SDK EXEC stack-size field before Exec; retain the vendor structure contract. |
| `main` | 38 | `1` | `if (Load(overlay_path_table[KF_OVERLAY_GAME_PATH], &header) == 1) {` | Preserve the BIOS Load success result, independently for the GAME load. |
| `main` | 40 | `0` | `header.s_addr = 0;` | Explicitly zero the SDK EXEC stack-address field before Exec; do not reinterpret it as an overlay request mode. |
| `main` | 41 | `0` | `header.s_size = 0;` | Explicitly zero the SDK EXEC stack-size field before Exec; retain the vendor structure contract. |

## `src/game/main.c`

4 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `repeat_store_word` | 19 | `0` | `while (count-- != 0) {` | Repeat-count exhaustion with post-decrement. The store destination never advances; this is not a BSS byte-length loop. |
| `main` | 32 | `0` | `repeat_store_word((int *)STARTUP_STORE_ADDRESS, STARTUP_REPEAT_STORE_COUNT, 0);` | The repeated word value is arithmetic zero. Destination and count have separate names; the repetition purpose remains unresolved. |
| `main` | 35 | `0` | `PadInit(0);` | Retain the legacy SDK selector: the linked PadInit routes zero to PAD_init2 and other identifiers to its diagnostic path. |
| `main` | 36 | `1` | `InitCARD2(1);` | Enable controller sharing through the existing BIOS pad-enable parameter; retain its vendor integer/Boolean interface. |

## `src/open/main.c`

2 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `repeat_store_word` | 19 | `0` | `while (count-- != 0) {` | Repeat-count exhaustion with post-decrement. The store destination never advances; this is not a BSS byte-length loop. |
| `main` | 27 | `0` | `repeat_store_word((int *)STARTUP_STORE_ADDRESS, STARTUP_REPEAT_STORE_COUNT, 0);` | The repeated word value is arithmetic zero, matching the GAME helper; no broader initialization purpose is inferred. |
