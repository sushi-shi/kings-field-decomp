#include <kf/address.h>
#include <kf/game_types.h>
#include <kf/psyq_kernel.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

/* Bytes reserved past the arena cursor in allocation mode 1; OPEN.EXE budgets more. */
#ifdef KF_OPEN
#define MEMORY_ARENA_LIMIT 0x112fff
#else
#define MEMORY_ARENA_LIMIT 0xfefff
#endif

/* Element 0 is the depth; elements 1..16 hold each allocation's size or malloc block. */

/* Allocations must land in the 2 MiB of RAM mirrored at 0x80000000. */
ADDRESS(0x8001aab0, 0x38)
void *memory_malloc_checked(s32 size)
{
    u8 *block = malloc(size);

    if ((u32)block + 0x80000000 > 0x1fffff) {
        return 0;
    }
    return block;
}

ADDRESS(0x8001aae8, 0x20)
void memory_allocation_reset(void)
{
    memory_allocation_stack[0] = 0;
    memory_arena_cursor = memory_arena_start;
}

ADDRESS(0x8001ab08, 0xa8)
void memory_set_allocation_mode(s32 mode)
{
    switch (mode) {
    case 0:
        memory_arena_start = memory_malloc_checked(0x100000);
        memory_arena_end = (u8 *)0x801effff;
        memory_allocation_reset();
        break;
    case 1:
        memory_arena_start = memory_arena_cursor;
        memory_arena_end = memory_arena_cursor + MEMORY_ARENA_LIMIT;
        memory_allocation_reset();
        break;
    case 2:
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
    memory_system_heap_size = (u8 *)0x801f8000 - memory_system_heap_start;
    InitHeap(memory_system_heap_start, memory_system_heap_size);
}

/*
 * With an arena, an allocation bumps the cursor and records its rounded size;
 * without one (mode 2) it records the malloc block so release can free it.
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
        size = (size + 3) & ~3;
        *cursor += size;
    }
    depth = memory_allocation_stack[0];
    memory_allocation_stack[0] = depth + 1;
    memory_allocation_stack[1 + depth] = size;
    return block;
}

ADDRESS(0x8001ac8c, 0x64)
void memory_release_last(void)
{
    s32 depth = memory_allocation_stack[0] - 1;
    u32 entry;

    memory_allocation_stack[0] = depth;
    entry = memory_allocation_stack[1 + depth];
    if (memory_arena_cursor == 0) {
        free((void *)entry);
    } else {
        memory_arena_cursor -= entry;
    }
}
