#ifndef KF_OVERLAY_H
#define KF_OVERLAY_H

#include <kf/enum.h>
#include <kf/game_types.h>
#include <kf/memory_layout.h>

/* Boundaries supplied by the program's link layout. */
extern u8 BSS_START[];
extern u8 BSS_END[];

enum {
    OVERLAY_RAM_BYTES = KF_MAIN_RAM_BYTES,
    OVERLAY_STACK_BYTES = 0x8000
};

#define OVERLAY_RAM_END (0x80000000u + OVERLAY_RAM_BYTES)
#define OVERLAY_STACK_BOTTOM (OVERLAY_RAM_END - OVERLAY_STACK_BYTES)

KF_ENUM_BEGIN(KfOpenMode, s32)
    KF_OPEN_MODE_INTRO = 1,
    KF_OPEN_MODE_ENDING = 0xfe
KF_ENUM_END(KfOpenMode)

KF_ENUM_BEGIN(KfGameExitCode, u32)
    KF_GAME_EXIT_NONE = 0,
    KF_GAME_EXIT_INTRO = KF_ENUM_ENCODE(s32, KF_OPEN_MODE_INTRO),
    KF_GAME_EXIT_ENDING = KF_ENUM_ENCODE(s32, KF_OPEN_MODE_ENDING)
KF_ENUM_END(KfGameExitCode)

/* Shared Exec argument block: OPEN reads request; GAME writes result.
 * The loader copies the returned encoding into the next OPEN request. */
typedef struct KfOverlayArguments {
    KfOpenMode request;
    KfGameExitCode result;
} KfOverlayArguments;

/* The SDK transports this block through Exec's char ** argument. */
enum {
    KF_OVERLAY_ENTRY_ARGC = 1,
    KF_OVERLAY_PATH_COUNT = 2,
    KF_OVERLAY_OPEN_PATH = 0,
    KF_OVERLAY_GAME_PATH = 1
};

#endif
