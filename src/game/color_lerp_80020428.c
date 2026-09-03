#include <kf/address.h>
#include <kf/semantic_types.h>

/*
 * Per-channel linear interpolation between two RGB555 colors.  Each 5-bit
 * channel is blended toward the second color by a 12-bit fraction (0x1000 ==
 * full second color); the STP bit is carried from the first color and the
 * channels are packed without individual clamping (the caller supplies an
 * in-range fraction).  No direct caller in GAME.EXE -- reached indirectly.
 */
ADDRESS(0x80020428, 0x98)
u16 func_80020428(u16 c0, u16 c1, s32 frac)
{
    s32 r0 = c0 & 0x1f;
    s32 r1 = c1 & 0x1f;
    s32 r = r0 + (((r1 - r0) * frac) >> 12);
    s32 g0 = (c0 >> 5) & 0x1f;
    s32 g1 = (c1 >> 5) & 0x1f;
    s32 g = g0 + (((g1 - g0) * frac) >> 12);
    s32 b0 = (c0 >> 10) & 0x1f;
    s32 b1 = (c1 >> 10) & 0x1f;
    s32 b = b0 + (((b1 - b0) * frac) >> 12);

    return r | ((c0 & 0x8000) | (b << 10) | (g << 5));
}
