#include <kf/address.h>
#include <kf/game_math.h>
#include <kf/render_types.h>

/*
 * Per-channel linear interpolation between two RGB555 colors.  Each 5-bit
 * channel is blended toward the second color by a 12-bit fraction (0x1000 ==
 * full second color); the STP bit is carried from the first color and the
 * channels are packed without individual clamping (the caller supplies an
 * in-range fraction).  No direct caller in GAME.EXE -- reached indirectly.
 */
ADDRESS(0x80020428, 0x98)
u16 color_lerp_rgb555(u16 c0, u16 c1, s32 frac)
{
    s32 r0 = c0 & KF_RGB555_CHANNEL_MASK;
    s32 r1 = c1 & KF_RGB555_CHANNEL_MASK;
    s32 r = r0 + (((r1 - r0) * frac) >> KF_FIXED12_BITS);
    s32 g0 = (c0 >> KF_RGB555_GREEN_SHIFT) & KF_RGB555_CHANNEL_MASK;
    s32 g1 = (c1 >> KF_RGB555_GREEN_SHIFT) & KF_RGB555_CHANNEL_MASK;
    s32 g = g0 + (((g1 - g0) * frac) >> KF_FIXED12_BITS);
    s32 b0 = (c0 >> KF_RGB555_BLUE_SHIFT) & KF_RGB555_CHANNEL_MASK;
    s32 b1 = (c1 >> KF_RGB555_BLUE_SHIFT) & KF_RGB555_CHANNEL_MASK;
    s32 b = b0 + (((b1 - b0) * frac) >> KF_FIXED12_BITS);

    return r | ((c0 & KF_RGB555_STP) | (b << KF_RGB555_BLUE_SHIFT) | (g << KF_RGB555_GREEN_SHIFT));
}
