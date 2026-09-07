#include <kf/address.h>
#include <kf/game_math.h>
#include <kf/render_types.h>

/*
 * Blend the RGB555 channels without individual clamping. The packed result
 * includes the STP bit from the first color; the blend uses a Q12 fraction.
 */
ADDRESS(0x80020428, 0x98)
u16 color_lerp_rgb555(u16 color0, u16 color1, s32 blend)
{
    s32 r0 = color0 & KF_RGB555_CHANNEL_MASK;
    s32 r1 = color1 & KF_RGB555_CHANNEL_MASK;
    s32 r = r0 + (((r1 - r0) * blend) >> KF_FIXED12_BITS);
    s32 g0 = (color0 >> KF_RGB555_GREEN_SHIFT) & KF_RGB555_CHANNEL_MASK;
    s32 g1 = (color1 >> KF_RGB555_GREEN_SHIFT) & KF_RGB555_CHANNEL_MASK;
    s32 g = g0 + (((g1 - g0) * blend) >> KF_FIXED12_BITS);
    s32 b0 = (color0 >> KF_RGB555_BLUE_SHIFT) & KF_RGB555_CHANNEL_MASK;
    s32 b1 = (color1 >> KF_RGB555_BLUE_SHIFT) & KF_RGB555_CHANNEL_MASK;
    s32 b = b0 + (((b1 - b0) * blend) >> KF_FIXED12_BITS);

    return r | ((color0 & KF_RGB555_STP) | (b << KF_RGB555_BLUE_SHIFT) | (g << KF_RGB555_GREEN_SHIFT));
}
