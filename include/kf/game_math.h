#ifndef KF_GAME_MATH_H
#define KF_GAME_MATH_H

/* Game-owned fixed-point angle, vector, and matrix helpers. */

#include <kf/semantic_types.h>

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
    const struct KfPitchYaw *angles, struct KfVec3s *direction);
extern void vector2s_scale_shift11(s16 scale, struct KfVecXZs *vector);
extern void vector2s_scale_shift12(s16 scale, s16 *vector);
extern void vector3i_add_xz(
    struct KfVec3i *vector, const struct KfVecXZs *delta);
extern void vector3s_scale_shift12(s16 scale, struct KfVec3s *vector);
extern void vector3s_scale_shift12_alt(s16 scale, s16 *vector);
extern s32 vector_xz_to_angle(s32 x, s32 z);

#endif
