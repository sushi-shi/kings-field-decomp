#pragma once
#include <kf/lib/types.h>

enum class KfMenuResult : s32 {
    KF_MENU_RESULT_PENDING = -99,
    KF_MENU_RESULT_GAME_LOADED = -3,
    KF_MENU_RESULT_RETURN_TO_INTRO = -2,
    KF_MENU_RESULT_CANCELLED = -1,
    KF_MENU_RESULT_ACCEPTED = 0,
    KF_MENU_RESULT_DECLINED = 1,
    KF_MENU_RESULT_STACK_FULL = 2
}; using enum KfMenuResult;

enum class KfMenuConfirmState : s32 {
    KF_MENU_CONFIRM_IDLE = 0,
    KF_MENU_CONFIRM_REQUESTED = 1
}; using enum KfMenuConfirmState;

enum class KfMenuConfirmChoice : s32 {
    KF_MENU_CHOICE_ACCEPT = 0,
    KF_MENU_CHOICE_DECLINE = 1
}; using enum KfMenuConfirmChoice;
