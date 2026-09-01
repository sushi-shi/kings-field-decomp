#include <kf/address.h>

ADDRESS(0x8001b3e4)
unsigned int *func_8001b3e4(
    unsigned int *destination,
    unsigned int *source,
    unsigned int count)
{
    while (count-- != 0) {
        *destination++ = *source++;
    }
    return source;
}
