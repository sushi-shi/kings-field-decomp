#include <kf/null.h>

#include <kf/game_types.h>
#include <psyq/kernel.h>
#include <psyq/libc.h>
#include <kf/game.h>
#include <kf/memory_layout.h>

enum {
    MEMORY_INITIAL_ARENA_BYTES = 0x100000,
    MEMORY_ALLOCATION_ALIGNMENT = 4
};

#define MEMORY_CACHED_RAM_BASE 0x80000000u
#define MEMORY_INITIAL_ARENA_LAST_ADDRESS 0x801effffu
#define MEMORY_SYSTEM_HEAP_END_ADDRESS 0x801f8000u

#ifdef KF_OPEN
#define MEMORY_REBASED_ARENA_LAST_OFFSET 0x112fff
#else
#define MEMORY_REBASED_ARENA_LAST_OFFSET 0xfefff
#endif

KfMemoryArena memory_arena;

void *memory_malloc_checked(s32 size)
{
    void *block = malloc(size);

    if ((u32)block + MEMORY_CACHED_RAM_BASE > KF_MAIN_RAM_BYTES - 1) {
        return NULL;
    }
    return block;
}

void memory_allocation_reset(void)
{
    memory_arena.allocation.stack[KF_MEMORY_STACK_DEPTH_INDEX] = 0;
    memory_arena.allocation.cursor = memory_arena.start;
}

void memory_set_allocation_mode(KfMemoryAllocationMode allocation_mode)
{
    switch (allocation_mode) {
    case KF_MEMORY_CREATE_ARENA:
        memory_arena.start = (u8 *)memory_malloc_checked(MEMORY_INITIAL_ARENA_BYTES);
        memory_arena.end = (u8 *)MEMORY_INITIAL_ARENA_LAST_ADDRESS;
        memory_allocation_reset();
        break;
    case KF_MEMORY_REBASE_ARENA:
        memory_arena.start = memory_arena.allocation.cursor;
        memory_arena.end = memory_arena.allocation.cursor + MEMORY_REBASED_ARENA_LAST_OFFSET;
        memory_allocation_reset();
        break;
    case KF_MEMORY_USE_HEAP:
        memory_arena.allocation.cursor = NULL;
        break;
    }
}

void memory_capture_system_heap_start(void)
{
    memory_arena.system_heap_start = memory_arena.end + 1;
}

void memory_reset_system_heap(void)
{
    memory_arena.system_heap_size = (u8 *)MEMORY_SYSTEM_HEAP_END_ADDRESS - memory_arena.system_heap_start;
    InitHeap((void *)memory_arena.system_heap_start, memory_arena.system_heap_size);
}

void *memory_allocate(s32 size)
{
    u8 **cursor = &memory_arena.allocation.cursor;
    void *block;
    s32 depth;

    if (*cursor == NULL) {
        block = memory_malloc_checked(size);
        size = (s32)block;
    } else {
        block = (void *)*cursor;
        size = (size + (MEMORY_ALLOCATION_ALIGNMENT - 1))
            & ~(MEMORY_ALLOCATION_ALIGNMENT - 1);
        *cursor += size;
    }
    depth = memory_arena.allocation.stack[KF_MEMORY_STACK_DEPTH_INDEX];
    memory_arena.allocation.stack[KF_MEMORY_STACK_DEPTH_INDEX] = depth + 1;
    memory_arena.allocation.stack[KF_MEMORY_STACK_FIRST_ENTRY + depth] = size;
    return block;
}

void memory_release_last(void)
{
    s32 depth = memory_arena.allocation.stack[KF_MEMORY_STACK_DEPTH_INDEX] - 1;
    u32 entry;

    memory_arena.allocation.stack[KF_MEMORY_STACK_DEPTH_INDEX] = depth;
    entry = memory_arena.allocation.stack[KF_MEMORY_STACK_FIRST_ENTRY + depth];
    if (memory_arena.allocation.cursor == NULL) {
        free((void *)entry);
    } else {
        memory_arena.allocation.cursor -= entry;
    }
}
