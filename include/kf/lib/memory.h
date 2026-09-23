#ifndef KF_MEMORY_H
#define KF_MEMORY_H

#include <kf/lib/types.h>

typedef s32 KfMemoryAllocationMode; enum {
    KF_MEMORY_CREATE_ARENA = 0,
    KF_MEMORY_REBASE_ARENA = 1,
    KF_MEMORY_USE_HEAP = 2
};

enum {
    KF_MEMORY_STACK_WORDS = 17,
    KF_MEMORY_STACK_DEPTH_INDEX = 0,
    KF_MEMORY_STACK_FIRST_ENTRY = 1
};

typedef struct KfMemoryAllocationState {
    u8 *cursor;
    u32 stack[KF_MEMORY_STACK_WORDS];
} KfMemoryAllocationState;

typedef struct KfMemoryArena {
    u8 *start;
    u8 *end;
    KfMemoryAllocationState allocation;
    u8 *system_heap_start;
    s32 system_heap_size;
} KfMemoryArena;

extern KfMemoryArena memory_arena;

extern void *memory_allocate(s32 size);
extern void memory_allocation_reset(void);
extern void memory_capture_system_heap_start(void);
extern void memory_set_allocation_mode(KfMemoryAllocationMode allocation_mode);
extern void *memory_malloc_checked(s32 size);
extern void memory_release_last(void);
extern void memory_reset_system_heap(void);

#endif
