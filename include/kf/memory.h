#ifndef KF_MEMORY_H
#define KF_MEMORY_H

/* Shared GAME/OPEN LIFO arena and heap-allocation interface. */

#include <kf/semantic_types.h>

extern u8 *memory_arena_start;
extern u8 *memory_arena_end;
extern u8 *memory_arena_cursor;
extern u32 memory_allocation_stack[17];
extern u8 *memory_system_heap_start;
extern s32 memory_system_heap_size;

extern void *memory_allocate(s32 size);
extern void memory_allocation_reset(void);
extern void memory_capture_system_heap_start(void);
extern void memory_set_allocation_mode(s32 mode);
extern void *memory_malloc_checked(s32 size);
extern void memory_release_last(void);
extern void memory_reset_system_heap(void);

#endif
