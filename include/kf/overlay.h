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

/* Exec receives one input argument and a second writable result word. */
enum {
    KF_OVERLAY_ARGUMENT_WORDS = 2,
    KF_OVERLAY_REQUEST_WORD = 0,
    KF_OVERLAY_RESULT_WORD = 1,
    KF_OVERLAY_ENTRY_ARGC = 1,
    KF_OVERLAY_PATH_COUNT = 2,
    KF_OVERLAY_OPEN_PATH = 0,
    KF_OVERLAY_GAME_PATH = 1
};

#endif
