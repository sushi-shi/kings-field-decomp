#ifndef KF_INPUT_H
#define KF_INPUT_H

#define PAD_PRESSED(current, previous, button) \
    (((current) & (button)) != 0 && ((previous) & (button)) == 0)

#endif
