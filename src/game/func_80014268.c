#include <kf/address.h>

ADDRESS(0x80014268, 0x24)
void func_80014268(volatile int *destination, int count, int value)
{
    while (count-- != 0) {
        *destination = value;
    }
}
