#include <kf/bool.h>
#include <kf/address.h>
#include <kf/game_math.h>


/*
 * Fixed-point matrix and vector math, one contiguous run
 * 0x80014a64..0x800150a8 (GAME.EXE): axis rotation-matrix builders and the
 * vector/scalar fixed-point helpers. Module boundary is WIP.
 */


/* Psy-Q LIBGTE: int rsin(int a); int rcos(int a). */

/*
 * Psy-Q LIBGTE MATRIX/SVECTOR/VECTOR have the MATRIX/SVECTOR/VECTOR layouts:
 * MulMatrix(m0, m1), MulMatrix2(m0, m1), ApplyMatrix(m, v0, v1).
 */

/*
 * Angles are 12-bit (0..0xfff). Within a half turn the step is applied
 * directly and clamped at the target; beyond it the angle wraps the other
 * way and the wrapped value is clamped only while it stays on the target's
 * side of the half-turn boundary.
 */
ADDRESS(0x80014a64, 0xc8)
s16 angle_approach(s16 current, s16 target, s32 step)
{
    s16 result;

    if (target == current) {
        return target;
    }
    if (current < target) {
        if (target - current <= KF_ANGLE_HALF_TURN) {
            result = current + step;
            if (target < result) {
                return target;
            }
        } else {
            result = (current - step) & KF_ANGLE_WRAP_MASK;
            if (result >= KF_ANGLE_HALF_TURN && result <= target) {
                return target;
            }
        }
    } else {
        if (current - target <= KF_ANGLE_HALF_TURN) {
            result = current - step;
            if (result < target) {
                return target;
            }
        } else {
            result = (current + step) & KF_ANGLE_WRAP_MASK;
            if (result <= KF_ANGLE_HALF_TURN && result >= target) {
                return target;
            }
        }
    }
    return result;
}

ADDRESS(0x80014b2c, 0x50)
void angle_to_forward_xz(s16 angle, struct KfVecXZs *direction)
{
    direction->x = -rsin(angle);
    direction->z = -rcos(angle);
}

#include "matrix_rotation.inc"

/*
 * Rotates the unit forward vector (0, 0, 0x1000) by pitch then yaw. ApplyMatrix
 * writes a VECTOR of longs; only their low halves are carried on.
 */
#include "pitch_yaw_to_forward_vector.inc"


ADDRESS(0x80014e08, 0x40)
void vector2s_scale_shift11(s16 scale, struct KfVecXZs *vector)
{
    s32 x = vector->x * scale;
    s32 z = vector->z * scale;

    vector->x = x >> KF_FIXED11_BITS;
    vector->z = z >> KF_FIXED11_BITS;
}

#include "vector3s_scale_shift12.inc"

ADDRESS(0x80014ea4, 0x40)
void vector2s_scale_shift12(s16 scale, s16 *vector)
{
    s32 x = vector[0] * scale;
    s32 y = vector[1] * scale;

    vector[0] = x >> KF_FIXED12_BITS;
    vector[1] = y >> KF_FIXED12_BITS;
}

#include "vector3s_scale_shift12_alt.inc"

ADDRESS(0x80014f40, 0x2c)
void vector3i_add_xz(
    VECTOR *destination, const struct KfVecXZs *delta)
{
    destination->vx += delta->x;
    destination->vz += delta->z;
}

#include "angle_within_tolerance.inc"

#include "angle_mod_delta.inc"

/* Psy-Q LIBGTE: catan(long) returns a 12-bit angle for a 12-bit fixed ratio. */

/*
 * Heading of the (x, z) offset as a 12-bit angle; the division carries the
 * checked expansion retail keeps for both signs of z.
 */
#include "vector_xz_to_angle.inc"


#include "fixed_vector2_length.inc"
