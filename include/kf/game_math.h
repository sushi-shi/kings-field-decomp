#ifndef KF_GAME_MATH_H
#define KF_GAME_MATH_H

/* Game-owned fixed-point angle, vector, and matrix types and helpers. */

#include <kf/bool.h>
#include <kf/game_types.h>
#include <psyq/sdk.h>

enum {
    KF_FIXED4_BITS = 4,
    KF_FIXED6_BITS = 6,
    KF_FIXED7_BITS = 7,
    KF_FIXED7_ONE = 128,
    KF_FIXED8_BITS = 8,
    KF_FIXED11_BITS = 11,
    KF_FIXED12_BITS = 12,
    KF_FIXED12_ONE = 0x1000,
    KF_ANGLE_EIGHTH_TURN = 0x200,
    KF_ANGLE_QUARTER_TURN = 0x400,
    KF_ANGLE_HALF_TURN = 0x800,
    KF_ANGLE_THREE_QUARTER_TURN = 0xc00,
    KF_ANGLE_FULL_TURN = 0x1000,
    KF_ANGLE_WRAP_MASK = 0xfff,
    KF_MATRIX_ROTATION_ELEMENTS = 9,
    KF_LENGTH_SQUARE_DOWNSHIFT = 3
};

enum {
    /* Extract a full-turn angle from the SDK's 15-bit rand() result. */
    KF_RANDOM_ANGLE_SHIFT = 3,
    /* Initial signed-halfword ranking error for actor and map-event cones. */
    KF_CONE_SEARCH_INITIAL_ANGLE_ERROR = 30000
};

struct KfVecXZs {
    s16 x;
    s16 z;
};

struct KfVec3s {
    s16 x;
    s16 y;
    s16 z;
};

struct KfEulerAngles {
    s16 x;
    s16 y;
    s16 z;
};

/* SDK aggregate transfers include pad; game Euler helpers read x/y/z. */
typedef union KfRotation {
    SVECTOR vector;
    struct KfEulerAngles angles;
} KfRotation;

static inline s16 angle_error_magnitude(s16 difference)
{
    s16 folded;
    difference &= KF_ANGLE_WRAP_MASK;
    folded = difference;
    if (difference > KF_ANGLE_HALF_TURN) {
        folded = KF_ANGLE_FULL_TURN - difference;
    }
    return folded;
}

static inline u32 radial_damage_attenuated_scale(
    s32 distance, u32 radius, u16 falloff, u16 scale)
{
    u16 ratio = (distance << KF_FIXED12_BITS) / radius;
    u16 weight = KF_FIXED12_ONE
        - ((u32)(ratio * (KF_FIXED12_ONE - falloff)) >> KF_FIXED12_BITS);
    return (u32)(scale * weight) >> KF_FIXED12_BITS;
}

static inline s32 fixed_vector3_length(s32 x, s32 y, s32 z)
{
    x >>= KF_LENGTH_SQUARE_DOWNSHIFT;
    y >>= KF_LENGTH_SQUARE_DOWNSHIFT;
    z >>= KF_LENGTH_SQUARE_DOWNSHIFT;
    return SquareRoot0(x * x + y * y + z * z) << KF_LENGTH_SQUARE_DOWNSHIFT;
}

/* Side-effect-free output and pose lvalues; caller owns the probe height. */
#define VECTOR_YAW_PROBE_XZ(x, z, position, rotation, reach) ( \
    (x) = (position).vx - ((rsin((rotation).vy) * (reach)) >> KF_FIXED12_BITS), \
    (z) = (position).vz + ((rcos((rotation).vy) * (reach)) >> KF_FIXED12_BITS))

extern s16 angle_approach(s16 current, s16 target, s32 step);
extern KfBool angle_mod_delta_le_half_turn(int lhs, int rhs);
extern s16 angle_shortest_delta(s32 first, s32 second);
extern void angle_to_forward_xz(s16 angle, struct KfVecXZs *direction);
extern KfBool angle_within_tolerance(int lhs, int rhs, s16 range);
extern s32 fixed6_ratio_step(s32 value, s32 span);
extern s32 fixed_vector2_length(s32 x, s32 y);
extern void matrix_interpolate(
    const MATRIX *from, const MATRIX *to, MATRIX *output, s32 blend);
extern void matrix_set_rotation_x(s16 angle, MATRIX *matrix);
extern void matrix_set_rotation_y(s16 angle, MATRIX *matrix);
extern void matrix_set_rotation_z(s16 angle, MATRIX *matrix);
extern void matrix_set_rotation_yxz(
    const struct KfEulerAngles *angles, MATRIX *matrix);
extern void pitch_yaw_to_forward_vector(
    const struct KfEulerAngles *angles, SVECTOR *direction);
extern void vector2s_scale_shift11(s16 scale, struct KfVecXZs *vector);
extern void vector2s_scale_shift12(s16 scale, s16 *vector);
extern void vector3i_add_xz(
    VECTOR *destination, const struct KfVecXZs *delta);
extern void vector3s_scale_shift12(s16 scale, SVECTOR *vector);
extern void vector3s_scale_shift12_alt(s16 scale, s16 *vector);
extern s32 vector_xz_to_angle(s32 x, s32 z);

#endif
