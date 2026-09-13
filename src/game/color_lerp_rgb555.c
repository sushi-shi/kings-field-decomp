#include <kf/address.h>
#include <kf/game_math.h>
#include <kf/render_types.h>

/*
 * Blend the RGB555 channels without individual clamping. The packed result
 * includes the STP bit from the first color; the blend uses a Q12 fraction.
 */
#include "../shared/color_lerp_rgb555.inc"
