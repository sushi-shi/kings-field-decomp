#include <kf/address.h>
#include <kf/game_math.h>

/* Psy-Q LIBGTE: catan(long) returns a 12-bit angle for a 12-bit fixed ratio. */
ADDRESS(0x80015ce4, 0xb0)
s32 vector_xz_to_angle(s32 x, s32 z)
{
    if (z > 0) {
        return catan((x << 12) / z) + 0x800;
    }
    if (z < 0) {
        return catan((x << 12) / z) & 0xfff;
    }
    if (x > 0) {
        return 0xc00;
    }
    return 0x400;
}
