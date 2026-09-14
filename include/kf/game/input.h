#ifndef KF_INPUT_H
#define KF_INPUT_H

/* Test one button's rising edge. button must have no side effects; previous
 * is read only when the button is present in current. */
#define PAD_PRESSED(current, previous, button) \
    (((current) & (button)) != 0 && ((previous) & (button)) == 0)

#endif
