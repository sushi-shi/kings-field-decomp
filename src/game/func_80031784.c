typedef unsigned char u8;
typedef signed short s16;

struct Fields80031784 {
    u8 unknown[40];
    u8 state;
    u8 alignment;
    s16 counter;
};

void func_80031784(struct Fields80031784 *object, u8 state)
{
    if (object->state == 0xff) {
        object->state = state;
        object->counter = 0;
    }
}
