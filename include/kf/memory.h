#ifndef KF_MEMORY_H
#define KF_MEMORY_H

/* Shared GAME/OPEN LIFO arena and heap-allocation interface. */

#include <kf/game_types.h>
#include <kf/enum.h>

KF_ENUM_BEGIN(KfMemoryAllocationMode, s32)
    KF_MEMORY_CREATE_ARENA = 0,
    KF_MEMORY_REBASE_ARENA = 1,
    KF_MEMORY_USE_HEAP = 2
KF_ENUM_END(KfMemoryAllocationMode)

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
} KfMemoryArena;

extern KfMemoryArena memory_arena;
extern u8 *memory_system_heap_start;
extern s32 memory_system_heap_size;

extern void *memory_allocate(s32 size);
extern void memory_allocation_reset(void);
extern void memory_capture_system_heap_start(void);
extern void memory_set_allocation_mode(KfMemoryAllocationMode mode);
extern void *memory_malloc_checked(s32 size);
extern void memory_release_last(void);
extern void memory_reset_system_heap(void);

#endif
