#ifndef KF_OVERLAY_H
#define KF_OVERLAY_H

#include <kf/enum.h>
#include <kf/game_types.h>

KF_ENUM_BEGIN(KfOpenMode, s32)
    KF_OPEN_MODE_INTRO = 1,
    KF_OPEN_MODE_ENDING = 0xfe
KF_ENUM_END(KfOpenMode)

KF_ENUM_BEGIN(KfGameExitCode, u32)
    KF_GAME_EXIT_NONE = 0,
    KF_GAME_EXIT_INTRO = 1,
    KF_GAME_EXIT_ENDING = 0xfe
KF_ENUM_END(KfGameExitCode)

/* Shared Exec argument block: OPEN reads request; GAME writes result.
 * The loader copies the returned encoding into the next OPEN request. */
typedef struct KfOverlayArguments {
    KfOpenMode request;
    KfGameExitCode result;
} KfOverlayArguments;
typedef char check_overlay_arguments_size[
    sizeof(KfOverlayArguments) == 8 ? 1 : -1];
typedef char check_overlay_arguments_result_offset[
    (unsigned long)&((KfOverlayArguments *)0)->result == 4 ? 1 : -1];

/* The SDK transports this block through Exec's char ** argument. */
enum {
    KF_OVERLAY_ENTRY_ARGC = 1,
    KF_OVERLAY_PATH_COUNT = 2,
    KF_OVERLAY_OPEN_PATH = 0,
    KF_OVERLAY_GAME_PATH = 1
};

#endif
