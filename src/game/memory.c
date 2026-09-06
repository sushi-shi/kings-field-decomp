#include <kf/address.h>
#include <kf/game_types.h>
#include <kf/psyq_kernel.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

enum {
    MEMORY_INITIAL_ARENA_BYTES = 0x100000,
    MEMORY_MAIN_RAM_BYTES = 0x200000,
    MEMORY_ALLOCATION_ALIGNMENT = 4
};

#define MEMORY_CACHED_RAM_BASE 0x80000000u
#define MEMORY_INITIAL_ARENA_LAST_ADDRESS 0x801effffu
#define MEMORY_SYSTEM_HEAP_END_ADDRESS 0x801f8000u

/* Inclusive last-byte offset from the rebased cursor; OPEN.EXE budgets more. */
#ifdef KF_OPEN
#define MEMORY_REBASED_ARENA_LAST_OFFSET 0x112fff
#else
#define MEMORY_REBASED_ARENA_LAST_OFFSET 0xfefff
#endif

/* Element 0 is the depth; elements 1..16 hold each allocation's size or malloc block. */

/* Allocations must land in the 2 MiB of RAM mirrored at 0x80000000. */
ADDRESS(0x8001aab0, 0x38)
void *memory_malloc_checked(s32 size)
{
    u8 *block = malloc(size);

    if ((u32)block + MEMORY_CACHED_RAM_BASE > MEMORY_MAIN_RAM_BYTES - 1) {
        return 0;
    }
    return block;
}

ADDRESS(0x8001aae8, 0x20)
void memory_allocation_reset(void)
{
    memory_allocation_stack[KF_MEMORY_STACK_DEPTH_INDEX] = 0;
    memory_arena_cursor = memory_arena_start;
}

ADDRESS(0x8001ab08, 0xa8)
void memory_set_allocation_mode(s32 mode)
{
    switch (mode) {
    case KF_MEMORY_CREATE_ARENA:
        memory_arena_start = memory_malloc_checked(MEMORY_INITIAL_ARENA_BYTES);
        memory_arena_end = (u8 *)MEMORY_INITIAL_ARENA_LAST_ADDRESS;
        memory_allocation_reset();
        break;
    case KF_MEMORY_REBASE_ARENA:
        memory_arena_start = memory_arena_cursor;
        memory_arena_end = memory_arena_cursor + MEMORY_REBASED_ARENA_LAST_OFFSET;
        memory_allocation_reset();
        break;
    case KF_MEMORY_USE_HEAP:
        memory_arena_cursor = 0;
        break;
    }
}

ADDRESS(0x8001abb0, 0x20)
void memory_capture_system_heap_start(void)
{
    memory_system_heap_start = memory_arena_end + 1;
}

ADDRESS(0x8001abd0, 0x3c)
void memory_reset_system_heap(void)
{
    memory_system_heap_size = (u8 *)MEMORY_SYSTEM_HEAP_END_ADDRESS - memory_system_heap_start;
    InitHeap(memory_system_heap_start, memory_system_heap_size);
}

/*
 * With an arena, an allocation bumps the cursor and records its rounded size;
 * without one it records the malloc block so release can free it.
 */
ADDRESS(0x8001ac0c, 0x80)
void *memory_allocate(s32 size)
{
    u8 **cursor = &memory_arena_cursor;
    u8 *block;
    s32 depth;

    if (*cursor == 0) {
        block = memory_malloc_checked(size);
        size = (s32)block;
    } else {
        block = *cursor;
        size = (size + (MEMORY_ALLOCATION_ALIGNMENT - 1))
            & ~(MEMORY_ALLOCATION_ALIGNMENT - 1);
        *cursor += size;
    }
    depth = memory_allocation_stack[KF_MEMORY_STACK_DEPTH_INDEX];
    memory_allocation_stack[KF_MEMORY_STACK_DEPTH_INDEX] = depth + 1;
    memory_allocation_stack[KF_MEMORY_STACK_FIRST_ENTRY + depth] = size;
    return block;
}

ADDRESS(0x8001ac8c, 0x64)
void memory_release_last(void)
{
    s32 depth = memory_allocation_stack[KF_MEMORY_STACK_DEPTH_INDEX] - 1;
    u32 entry;

    memory_allocation_stack[KF_MEMORY_STACK_DEPTH_INDEX] = depth;
    entry = memory_allocation_stack[KF_MEMORY_STACK_FIRST_ENTRY + depth];
    if (memory_arena_cursor == 0) {
        free((void *)entry);
    } else {
        memory_arena_cursor -= entry;
    }
}
