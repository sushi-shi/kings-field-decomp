#ifndef KF_GAME_MATH_H
#define KF_GAME_MATH_H

/* Game-owned fixed-point angle, vector, and matrix types and helpers. */

#include <kf/game_types.h>
#include <kf/psyq.h>

enum {
    KF_FIXED4_BITS = 4,
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

struct KfVecXZs {
    s16 x;
    s16 z;
};

struct KfVec3s {
    s16 x;
    s16 y;
    s16 z;
};

struct KfVec3i {
    s32 x;
    s32 y;
    s32 z;
};

struct KfPitchYaw {
    s16 pitch;
    s16 yaw;
};

struct KfEulerAngles {
    s16 x;
    s16 y;
    s16 z;
};

extern s16 angle_approach(s16 current, s16 target, s32 step);
extern int angle_mod_delta_le_half_turn(int lhs, int rhs);
extern s16 angle_shortest_delta(s32 first, s32 second);
extern void angle_to_forward_xz(s16 angle, struct KfVecXZs *direction);
extern int angle_within_tolerance(int lhs, int rhs, s16 range);
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
    const struct KfPitchYaw *angles, SVECTOR *direction);
extern void vector2s_scale_shift11(s16 scale, struct KfVecXZs *vector);
extern void vector2s_scale_shift12(s16 scale, s16 *vector);
extern void vector3i_add_xz(
    struct KfVec3i *vector, const struct KfVecXZs *delta);
extern void vector3s_scale_shift12(s16 scale, SVECTOR *vector);
extern void vector3s_scale_shift12_alt(s16 scale, s16 *vector);
extern s32 vector_xz_to_angle(s32 x, s32 z);

#endif
