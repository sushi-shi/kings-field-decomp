#ifndef KF_INPUT_H
#define KF_INPUT_H
#include <kf/platform/input.hpp>

#define BUTTON_PRESSED(current, previous, button) \
    (((current) & (button)) != 0 && ((previous) & (button)) == 0)

#endif
