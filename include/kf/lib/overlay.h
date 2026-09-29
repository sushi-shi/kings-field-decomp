#ifndef KF_OVERLAY_H
#define KF_OVERLAY_H

#include <kf/lib/types.h>
#include <kf/lib/memory_layout.h>

extern u8 BSS_START[];
extern u8 BSS_END[];

enum {
    OVERLAY_STACK_BYTES = 0x8000
};

#define OVERLAY_RAM_END (0x80000000u + KF_MAIN_RAM_BYTES)
#define OVERLAY_STACK_BOTTOM (OVERLAY_RAM_END - OVERLAY_STACK_BYTES)

typedef s32 KfOverlayMode; enum {
    KF_OVERLAY_MODE_NONE = 0,
    KF_OVERLAY_MODE_INTRO = 1,
    KF_OVERLAY_MODE_ENDING = 0xfe
};

typedef u32 KfOverlayResultWord;

typedef struct KfOverlayArguments {
    KfOverlayMode request;
    KfOverlayResultWord result;
} KfOverlayArguments;

enum {
    KF_OVERLAY_ENTRY_ARGC = 1,
    KF_OVERLAY_PATH_COUNT = 2,
    KF_OVERLAY_OPEN_PATH = 0,
    KF_OVERLAY_GAME_PATH = 1
};

#endif
