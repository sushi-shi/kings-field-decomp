#include <kf/platform/prelude.hpp>
#include <kf/lib/memory.h>
#include <cstdlib>
#include <cstdio>
#include <cstdint>

enum {
    MEMORY_INITIAL_ARENA_BYTES = 16 * 1024 * 1024,
    MEMORY_ALLOCATION_ALIGNMENT = alignof(std::max_align_t)
};

void *memory_malloc_checked(std::size_t size)
{
    return size ? malloc(size) : NULL;
}

static void memory_forget_allocations(KfMemoryArena &arena)
{
    auto *allocation = &arena.allocation;
    while (allocation->depth) {
        const auto entry = allocation->stack[--allocation->depth];
        if (entry.storage == KfMemoryStorage::Heap)
            free(entry.block);
        allocation->stack[allocation->depth] = {};
    }
}

void memory_allocation_reset(KfMemoryArena &arena)
{
    memory_forget_allocations(arena);
    arena.allocation.cursor = arena.start;
}

void memory_destroy_arena(KfMemoryArena &arena)
{
    memory_forget_allocations(arena);
    free(arena.storage);
    arena.storage = NULL;
    arena = {};
}

void memory_set_allocation_mode(KfMemoryArena &arena, KfMemoryAllocationMode allocation_mode)
{
    switch (allocation_mode) {
    case KF_MEMORY_CREATE_ARENA:
        memory_destroy_arena(arena);
        arena.storage = static_cast<u8 *>(memory_malloc_checked(MEMORY_INITIAL_ARENA_BYTES));
        if (!arena.storage) {
            fprintf(stderr, "Cannot allocate the resource arena.\n");
            exit(1);
        }
        arena.start = arena.storage;
        arena.end = arena.storage + MEMORY_INITIAL_ARENA_BYTES;
        memory_allocation_reset(arena);
        break;
    case KF_MEMORY_REBASE_ARENA:
        arena.start = arena.allocation.cursor;
        memory_allocation_reset(arena);
        break;
    case KF_MEMORY_USE_HEAP:
        arena.allocation.cursor = NULL;
        break;
    }
}

void *memory_allocate(KfMemoryArena &arena, std::size_t size)
{
    auto *allocation = &arena.allocation;
    if (!size || allocation->depth == KF_MEMORY_ALLOCATION_CAPACITY)
        return NULL;
    KfMemoryAllocation entry {};
    if (!allocation->cursor) {
        entry.storage = KfMemoryStorage::Heap;
        entry.block = memory_malloc_checked(size);
        if (!entry.block)
            return NULL;
    } else {
        const auto address = reinterpret_cast<std::uintptr_t>(allocation->cursor);
        const auto start = reinterpret_cast<std::uintptr_t>(arena.start);
        const auto end = reinterpret_cast<std::uintptr_t>(arena.end);
        if (address < start || address > end)
            return NULL;
        const auto padding = (MEMORY_ALLOCATION_ALIGNMENT - address % MEMORY_ALLOCATION_ALIGNMENT)
                             % MEMORY_ALLOCATION_ALIGNMENT;
        if (padding > end - address || size > end - address - padding)
            return NULL;
        entry.storage = KfMemoryStorage::Arena;
        entry.previous_cursor = allocation->cursor;
        entry.block = allocation->cursor + padding;
        allocation->cursor += padding + size;
    }
    allocation->stack[allocation->depth++] = entry;
    return entry.block;
}

void memory_release_last(KfMemoryArena &arena)
{
    auto *allocation = &arena.allocation;
    if (!allocation->depth)
        return;
    const auto entry = allocation->stack[--allocation->depth];
    allocation->stack[allocation->depth] = {};
    if (entry.storage == KfMemoryStorage::Heap)
        free(entry.block);
    else
        allocation->cursor = entry.previous_cursor;
}
