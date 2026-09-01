#include <kf/address.h>

typedef unsigned char u8;
typedef signed short s16;

struct Fields80031784 {
    u8 unknown[40];
    u8 state;
    u8 alignment;
    s16 counter;
};

ADDRESS(0x80031784, 0x20)
void map_object_start_action_if_idle(struct Fields80031784 *object, u8 state)
{
    if (object->state == 0xff) {
        object->state = state;
        object->counter = 0;
    }
}
