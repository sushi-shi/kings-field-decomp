#ifndef KF_MEMORY_H
#define KF_MEMORY_H

#include <kf/lib/types.h>
#include <kf/lib/enum.h>
#include <cstddef>

enum class KfMemoryAllocationMode : s32 {
    KF_MEMORY_CREATE_ARENA = 0,
    KF_MEMORY_REBASE_ARENA = 1,
    KF_MEMORY_USE_HEAP = 2
}; using enum KfMemoryAllocationMode;

enum { KF_MEMORY_ALLOCATION_CAPACITY = 16 };

enum class KfMemoryStorage { Arena, Heap };

struct KfMemoryAllocation {
    KfMemoryStorage storage;
    void *block;
    u8 *previous_cursor;
};

typedef struct KfMemoryAllocationState {
    u8 *cursor;
    std::size_t depth;
    KfMemoryAllocation stack[KF_MEMORY_ALLOCATION_CAPACITY];
} KfMemoryAllocationState;

typedef struct KfMemoryArena {
    u8 *start;
    u8 *end;
    KfMemoryAllocationState allocation;
} KfMemoryArena;

extern KfMemoryArena memory_arena;

extern void *memory_allocate(std::size_t size);
extern void memory_allocation_reset(void);
extern void memory_destroy_arena(void);
extern void memory_set_allocation_mode(KfMemoryAllocationMode allocation_mode);
extern void *memory_malloc_checked(std::size_t size);
extern void memory_release_last(void);

#endif
