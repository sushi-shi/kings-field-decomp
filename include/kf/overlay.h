#ifndef KF_OVERLAY_H
#define KF_OVERLAY_H

#include <kf/enum.h>
#include <kf/game_types.h>
#include <kf/memory_layout.h>

/* Boundaries supplied by the program's link layout. */
extern u8 BSS_START[];
extern u8 BSS_END[];

enum {
    OVERLAY_STACK_BYTES = 0x8000
};

#define OVERLAY_RAM_END (0x80000000u + KF_MAIN_RAM_BYTES)
#define OVERLAY_STACK_BOTTOM (OVERLAY_RAM_END - OVERLAY_STACK_BYTES)

KF_ENUM_BEGIN(KfOverlayMode, s32)
    KF_OVERLAY_MODE_NONE = 0,
    KF_OVERLAY_MODE_INTRO = 1,
    KF_OVERLAY_MODE_ENDING = 0xfe
KF_ENUM_END(KfOverlayMode)

typedef KF_ENUM_STORAGE(KfOverlayMode, u32) KfOverlayResultWord;

/* Shared Exec argument block: OPEN reads request; GAME writes result.
 * The loader passes the result directly as the next OPEN request. */
typedef struct KfOverlayArguments {
    KfOverlayMode request;
    KfOverlayResultWord result;
} KfOverlayArguments;

/* The SDK transports this block through Exec's char ** argument. */
enum {
    KF_OVERLAY_ENTRY_ARGC = 1,
    KF_OVERLAY_PATH_COUNT = 2,
    KF_OVERLAY_OPEN_PATH = 0,
    KF_OVERLAY_GAME_PATH = 1
};

#endif
