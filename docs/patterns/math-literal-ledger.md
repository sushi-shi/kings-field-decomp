# Matrix and vector retained-literal ledger

Complete current accounting for nine files and **189 retained occurrences**.
See the [math evidence review](math-literal-coverage.md). Each duplicate token
has its own row, including matrix row/column coordinates and zero coefficients.
Claims, named definitions and string contents are excluded.

## `src/game/matrix.c`

2 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `matrix_interpolate` | 22 | `1` | `s16 count = KF_MATRIX_ROTATION_ELEMENTS - 1;` | Convert the named nine-element rotation extent to the last zero-based counter value, eight. |
| `matrix_interpolate` | 29 | `1` | `} while (--count != -1);` | The signed counter exits at minus one after visiting all nine coefficients; one is the countdown endpoint adjustment. |

## `src/game/matrix_rotation.c`

82 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `matrix_set_rotation_x` | 74 | `0` | `matrix->m[0][0] = KF_FIXED12_ONE;` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 74 | `0` | `matrix->m[0][0] = KF_FIXED12_ONE;` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 75 | `0` | `matrix->m[0][1] = 0;` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 75 | `1` | `matrix->m[0][1] = 0;` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 75 | `0` | `matrix->m[0][1] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_x` | 76 | `0` | `matrix->m[0][2] = 0;` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 76 | `2` | `matrix->m[0][2] = 0;` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 76 | `0` | `matrix->m[0][2] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_x` | 77 | `1` | `matrix->m[1][0] = 0;` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 77 | `0` | `matrix->m[1][0] = 0;` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 77 | `0` | `matrix->m[1][0] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_x` | 78 | `1` | `matrix->m[1][1] = cos;` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 78 | `1` | `matrix->m[1][1] = cos;` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 79 | `1` | `matrix->m[1][2] = -sin;` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 79 | `2` | `matrix->m[1][2] = -sin;` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 80 | `2` | `matrix->m[2][0] = 0;` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 80 | `0` | `matrix->m[2][0] = 0;` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 80 | `0` | `matrix->m[2][0] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_x` | 81 | `2` | `matrix->m[2][1] = sin;` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 81 | `1` | `matrix->m[2][1] = sin;` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 82 | `2` | `matrix->m[2][2] = cos;` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 82 | `2` | `matrix->m[2][2] = cos;` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 91 | `0` | `matrix->m[0][0] = cos;` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 91 | `0` | `matrix->m[0][0] = cos;` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 92 | `0` | `matrix->m[0][1] = 0;` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 92 | `1` | `matrix->m[0][1] = 0;` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 92 | `0` | `matrix->m[0][1] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_y` | 93 | `0` | `matrix->m[0][2] = -sin;` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 93 | `2` | `matrix->m[0][2] = -sin;` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 94 | `1` | `matrix->m[1][0] = 0;` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 94 | `0` | `matrix->m[1][0] = 0;` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 94 | `0` | `matrix->m[1][0] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_y` | 95 | `1` | `matrix->m[1][1] = KF_FIXED12_ONE;` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 95 | `1` | `matrix->m[1][1] = KF_FIXED12_ONE;` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 96 | `1` | `matrix->m[1][2] = 0;` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 96 | `2` | `matrix->m[1][2] = 0;` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 96 | `0` | `matrix->m[1][2] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_y` | 97 | `2` | `matrix->m[2][0] = sin;` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 97 | `0` | `matrix->m[2][0] = sin;` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 98 | `2` | `matrix->m[2][1] = 0;` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 98 | `1` | `matrix->m[2][1] = 0;` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 98 | `0` | `matrix->m[2][1] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_y` | 99 | `2` | `matrix->m[2][2] = cos;` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 99 | `2` | `matrix->m[2][2] = cos;` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 108 | `0` | `matrix->m[0][0] = cos;` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 108 | `0` | `matrix->m[0][0] = cos;` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 109 | `0` | `matrix->m[0][1] = -sin;` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 109 | `1` | `matrix->m[0][1] = -sin;` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 110 | `0` | `matrix->m[0][2] = 0;` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 110 | `2` | `matrix->m[0][2] = 0;` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 110 | `0` | `matrix->m[0][2] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_z` | 111 | `1` | `matrix->m[1][0] = sin;` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 111 | `0` | `matrix->m[1][0] = sin;` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 112 | `1` | `matrix->m[1][1] = cos;` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 112 | `1` | `matrix->m[1][1] = cos;` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 113 | `1` | `matrix->m[1][2] = 0;` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 113 | `2` | `matrix->m[1][2] = 0;` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 113 | `0` | `matrix->m[1][2] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_z` | 114 | `2` | `matrix->m[2][0] = 0;` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 114 | `0` | `matrix->m[2][0] = 0;` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 114 | `0` | `matrix->m[2][0] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_z` | 115 | `2` | `matrix->m[2][1] = 0;` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 115 | `1` | `matrix->m[2][1] = 0;` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 115 | `0` | `matrix->m[2][1] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_z` | 116 | `2` | `matrix->m[2][2] = KF_FIXED12_ONE;` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 116 | `2` | `matrix->m[2][2] = KF_FIXED12_ONE;` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `pitch_yaw_to_forward_vector` | 143 | `0` | `source.vx = 0;` | The unit forward vector starts on the Z axis, so this perpendicular component is mathematically zero. |
| `pitch_yaw_to_forward_vector` | 144 | `0` | `source.vy = 0;` | The unit forward vector starts on the Z axis, so this perpendicular component is mathematically zero. |
| `vector2s_scale_shift12` | 184 | `0` | `s32 x = vector[0] * scale;` | The first signed-halfword component in this raw vector view. No validated caller establishes a more specific coordinate plane; preserve the ordinary index. |
| `vector2s_scale_shift12` | 185 | `1` | `s32 y = vector[1] * scale;` | The second signed-halfword component in this raw vector view. No validated caller establishes a more specific coordinate plane; preserve the ordinary index. |
| `vector2s_scale_shift12` | 187 | `0` | `vector[0] = x >> KF_FIXED12_BITS;` | The first signed-halfword component in this raw vector view. No validated caller establishes a more specific coordinate plane; preserve the ordinary index. |
| `vector2s_scale_shift12` | 188 | `1` | `vector[1] = y >> KF_FIXED12_BITS;` | The second signed-halfword component in this raw vector view. No validated caller establishes a more specific coordinate plane; preserve the ordinary index. |
| `vector3s_scale_shift12_alt` | 194 | `0` | `s32 x = vector[0] * scale;` | The first signed-halfword component in this raw vector view. No validated caller establishes a more specific coordinate plane; preserve the ordinary index. |
| `vector3s_scale_shift12_alt` | 195 | `1` | `s32 y = vector[1] * scale;` | The second signed-halfword component in this raw vector view. No validated caller establishes a more specific coordinate plane; preserve the ordinary index. |
| `vector3s_scale_shift12_alt` | 196 | `2` | `s32 z = vector[2] * scale;` | The third signed-halfword component in this raw vector view. No validated caller establishes a more specific coordinate plane; preserve the ordinary index. |
| `vector3s_scale_shift12_alt` | 198 | `0` | `vector[0] = x >> KF_FIXED12_BITS;` | The first signed-halfword component in this raw vector view. No validated caller establishes a more specific coordinate plane; preserve the ordinary index. |
| `vector3s_scale_shift12_alt` | 199 | `1` | `vector[1] = y >> KF_FIXED12_BITS;` | The second signed-halfword component in this raw vector view. No validated caller establishes a more specific coordinate plane; preserve the ordinary index. |
| `vector3s_scale_shift12_alt` | 200 | `2` | `vector[2] = z >> KF_FIXED12_BITS;` | The third signed-halfword component in this raw vector view. No validated caller establishes a more specific coordinate plane; preserve the ordinary index. |
| `angle_mod_delta_le_half_turn` | 222 | `1` | `return ((lhs - rhs) & KF_ANGLE_WRAP_MASK) < (KF_ANGLE_HALF_TURN + 1);` | Add one to turn the inclusive half-turn boundary into the original exclusive comparison bound. |
| `vector_xz_to_angle` | 234 | `0` | `if (z > 0) {` | Arithmetic sign boundary selects the heading quadrant or axis case. For x=z=0 the existing fallthrough returns a quarter turn. |
| `vector_xz_to_angle` | 237 | `0` | `if (z < 0) {` | Arithmetic sign boundary selects the heading quadrant or axis case. For x=z=0 the existing fallthrough returns a quarter turn. |
| `vector_xz_to_angle` | 240 | `0` | `if (x > 0) {` | Arithmetic sign boundary selects the heading quadrant or axis case. For x=z=0 the existing fallthrough returns a quarter turn. |

## `src/game/color_lerp_rgb555.c`

0 retained occurrences.

All format/scale conventions already use named constants; this module has no retained numeric or character tokens.

## `src/open/matrix.c`

8 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `matrix_interpolate` | 16 | `0` | `const s16 *source = &from->m[0][0];` | Start at row zero, column zero of the rotation coefficients; this is the first matrix element, not a state value. |
| `matrix_interpolate` | 16 | `0` | `const s16 *source = &from->m[0][0];` | Start at row zero, column zero of the rotation coefficients; this is the first matrix element, not a state value. |
| `matrix_interpolate` | 17 | `0` | `const s16 *target = &to->m[0][0];` | Start at row zero, column zero of the rotation coefficients; this is the first matrix element, not a state value. |
| `matrix_interpolate` | 17 | `0` | `const s16 *target = &to->m[0][0];` | Start at row zero, column zero of the rotation coefficients; this is the first matrix element, not a state value. |
| `matrix_interpolate` | 18 | `0` | `s16 *destination = &output->m[0][0];` | Start at row zero, column zero of the rotation coefficients; this is the first matrix element, not a state value. |
| `matrix_interpolate` | 18 | `0` | `s16 *destination = &output->m[0][0];` | Start at row zero, column zero of the rotation coefficients; this is the first matrix element, not a state value. |
| `matrix_interpolate` | 19 | `1` | `s16 count = KF_MATRIX_ROTATION_ELEMENTS - 1;` | Convert the named nine-element rotation extent to the last zero-based counter value, eight. |
| `matrix_interpolate` | 26 | `1` | `} while (--count != -1);` | The signed counter exits at minus one after visiting all nine coefficients; one is the countdown endpoint adjustment. |

## `src/open/matrix_rotation.c`

87 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 8 | `16` | `static const char debug_matrix_label[16] = "Dump Matrix\n";` | Declared sixteen-byte diagnostic string storage: twelve text bytes, one terminator and three zero bytes. Its extent is not a gameplay limit; the original declaration/alignment rationale is unproven. |
| `matrix_set_rotation_x` | 16 | `0` | `matrix->m[0][0] = KF_FIXED12_ONE;` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 16 | `0` | `matrix->m[0][0] = KF_FIXED12_ONE;` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 17 | `0` | `matrix->m[0][1] = 0;` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 17 | `1` | `matrix->m[0][1] = 0;` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 17 | `0` | `matrix->m[0][1] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_x` | 18 | `0` | `matrix->m[0][2] = 0;` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 18 | `2` | `matrix->m[0][2] = 0;` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 18 | `0` | `matrix->m[0][2] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_x` | 19 | `1` | `matrix->m[1][0] = 0;` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 19 | `0` | `matrix->m[1][0] = 0;` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 19 | `0` | `matrix->m[1][0] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_x` | 20 | `1` | `matrix->m[1][1] = cos;` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 20 | `1` | `matrix->m[1][1] = cos;` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 21 | `1` | `matrix->m[1][2] = -sin;` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 21 | `2` | `matrix->m[1][2] = -sin;` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 22 | `2` | `matrix->m[2][0] = 0;` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 22 | `0` | `matrix->m[2][0] = 0;` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 22 | `0` | `matrix->m[2][0] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_x` | 23 | `2` | `matrix->m[2][1] = sin;` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 23 | `1` | `matrix->m[2][1] = sin;` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 24 | `2` | `matrix->m[2][2] = cos;` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_x` | 24 | `2` | `matrix->m[2][2] = cos;` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 33 | `0` | `matrix->m[0][0] = cos;` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 33 | `0` | `matrix->m[0][0] = cos;` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 34 | `0` | `matrix->m[0][1] = 0;` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 34 | `1` | `matrix->m[0][1] = 0;` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 34 | `0` | `matrix->m[0][1] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_y` | 35 | `0` | `matrix->m[0][2] = -sin;` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 35 | `2` | `matrix->m[0][2] = -sin;` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 36 | `1` | `matrix->m[1][0] = 0;` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 36 | `0` | `matrix->m[1][0] = 0;` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 36 | `0` | `matrix->m[1][0] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_y` | 37 | `1` | `matrix->m[1][1] = KF_FIXED12_ONE;` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 37 | `1` | `matrix->m[1][1] = KF_FIXED12_ONE;` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 38 | `1` | `matrix->m[1][2] = 0;` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 38 | `2` | `matrix->m[1][2] = 0;` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 38 | `0` | `matrix->m[1][2] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_y` | 39 | `2` | `matrix->m[2][0] = sin;` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 39 | `0` | `matrix->m[2][0] = sin;` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 40 | `2` | `matrix->m[2][1] = 0;` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 40 | `1` | `matrix->m[2][1] = 0;` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 40 | `0` | `matrix->m[2][1] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_y` | 41 | `2` | `matrix->m[2][2] = cos;` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_y` | 41 | `2` | `matrix->m[2][2] = cos;` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 50 | `0` | `matrix->m[0][0] = cos;` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 50 | `0` | `matrix->m[0][0] = cos;` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 51 | `0` | `matrix->m[0][1] = -sin;` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 51 | `1` | `matrix->m[0][1] = -sin;` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 52 | `0` | `matrix->m[0][2] = 0;` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 52 | `2` | `matrix->m[0][2] = 0;` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 52 | `0` | `matrix->m[0][2] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_z` | 53 | `1` | `matrix->m[1][0] = sin;` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 53 | `0` | `matrix->m[1][0] = sin;` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 54 | `1` | `matrix->m[1][1] = cos;` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 54 | `1` | `matrix->m[1][1] = cos;` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 55 | `1` | `matrix->m[1][2] = 0;` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 55 | `2` | `matrix->m[1][2] = 0;` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 55 | `0` | `matrix->m[1][2] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_z` | 56 | `2` | `matrix->m[2][0] = 0;` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 56 | `0` | `matrix->m[2][0] = 0;` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 56 | `0` | `matrix->m[2][0] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_z` | 57 | `2` | `matrix->m[2][1] = 0;` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 57 | `1` | `matrix->m[2][1] = 0;` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 57 | `0` | `matrix->m[2][1] = 0;` | Zero cross-axis coupling in the mathematical axis-rotation matrix. |
| `matrix_set_rotation_z` | 58 | `2` | `matrix->m[2][2] = KF_FIXED12_ONE;` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `matrix_set_rotation_z` | 58 | `2` | `matrix->m[2][2] = KF_FIXED12_ONE;` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `debug_dump_matrix` | 78 | `0` | `matrix->m[0][0], matrix->m[0][1], matrix->m[0][2]);` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `debug_dump_matrix` | 78 | `0` | `matrix->m[0][0], matrix->m[0][1], matrix->m[0][2]);` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `debug_dump_matrix` | 78 | `0` | `matrix->m[0][0], matrix->m[0][1], matrix->m[0][2]);` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `debug_dump_matrix` | 78 | `1` | `matrix->m[0][0], matrix->m[0][1], matrix->m[0][2]);` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `debug_dump_matrix` | 78 | `0` | `matrix->m[0][0], matrix->m[0][1], matrix->m[0][2]);` | Matrix row 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `debug_dump_matrix` | 78 | `2` | `matrix->m[0][0], matrix->m[0][1], matrix->m[0][2]);` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `debug_dump_matrix` | 80 | `1` | `matrix->m[1][0], matrix->m[1][1], matrix->m[1][2]);` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `debug_dump_matrix` | 80 | `0` | `matrix->m[1][0], matrix->m[1][1], matrix->m[1][2]);` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `debug_dump_matrix` | 80 | `1` | `matrix->m[1][0], matrix->m[1][1], matrix->m[1][2]);` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `debug_dump_matrix` | 80 | `1` | `matrix->m[1][0], matrix->m[1][1], matrix->m[1][2]);` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `debug_dump_matrix` | 80 | `1` | `matrix->m[1][0], matrix->m[1][1], matrix->m[1][2]);` | Matrix row 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `debug_dump_matrix` | 80 | `2` | `matrix->m[1][0], matrix->m[1][1], matrix->m[1][2]);` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `debug_dump_matrix` | 82 | `2` | `matrix->m[2][0], matrix->m[2][1], matrix->m[2][2]);` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `debug_dump_matrix` | 82 | `0` | `matrix->m[2][0], matrix->m[2][1], matrix->m[2][2]);` | Matrix column 0 (X); explicit numeric coordinates keep the rotation or display formula readable. |
| `debug_dump_matrix` | 82 | `2` | `matrix->m[2][0], matrix->m[2][1], matrix->m[2][2]);` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `debug_dump_matrix` | 82 | `1` | `matrix->m[2][0], matrix->m[2][1], matrix->m[2][2]);` | Matrix column 1 (Y); explicit numeric coordinates keep the rotation or display formula readable. |
| `debug_dump_matrix` | 82 | `2` | `matrix->m[2][0], matrix->m[2][1], matrix->m[2][2]);` | Matrix row 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `debug_dump_matrix` | 82 | `2` | `matrix->m[2][0], matrix->m[2][1], matrix->m[2][2]);` | Matrix column 2 (Z); explicit numeric coordinates keep the rotation or display formula readable. |
| `pitch_yaw_to_forward_vector` | 94 | `0` | `source.vx = 0;` | The unit forward vector starts on the Z axis, so this perpendicular component is mathematically zero. |
| `pitch_yaw_to_forward_vector` | 95 | `0` | `source.vy = 0;` | The unit forward vector starts on the Z axis, so this perpendicular component is mathematically zero. |

## `src/open/vector_math.c`

6 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `vector3s_scale_shift12_alt` | 19 | `0` | `s32 x = vector[0] * scale;` | The first signed-halfword component in this raw vector view. No validated caller establishes a more specific coordinate plane; preserve the ordinary index. |
| `vector3s_scale_shift12_alt` | 20 | `1` | `s32 y = vector[1] * scale;` | The second signed-halfword component in this raw vector view. No validated caller establishes a more specific coordinate plane; preserve the ordinary index. |
| `vector3s_scale_shift12_alt` | 21 | `2` | `s32 z = vector[2] * scale;` | The third signed-halfword component in this raw vector view. No validated caller establishes a more specific coordinate plane; preserve the ordinary index. |
| `vector3s_scale_shift12_alt` | 23 | `0` | `vector[0] = x >> KF_FIXED12_BITS;` | The first signed-halfword component in this raw vector view. No validated caller establishes a more specific coordinate plane; preserve the ordinary index. |
| `vector3s_scale_shift12_alt` | 24 | `1` | `vector[1] = y >> KF_FIXED12_BITS;` | The second signed-halfword component in this raw vector view. No validated caller establishes a more specific coordinate plane; preserve the ordinary index. |
| `vector3s_scale_shift12_alt` | 25 | `2` | `vector[2] = z >> KF_FIXED12_BITS;` | The third signed-halfword component in this raw vector view. No validated caller establishes a more specific coordinate plane; preserve the ordinary index. |

## `src/open/vector_xz_to_angle.c`

3 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `vector_xz_to_angle` | 8 | `0` | `if (z > 0) {` | Arithmetic sign boundary selects the heading quadrant or axis case. For x=z=0 the existing fallthrough returns a quarter turn. |
| `vector_xz_to_angle` | 11 | `0` | `if (z < 0) {` | Arithmetic sign boundary selects the heading quadrant or axis case. For x=z=0 the existing fallthrough returns a quarter turn. |
| `vector_xz_to_angle` | 14 | `0` | `if (x > 0) {` | Arithmetic sign boundary selects the heading quadrant or axis case. For x=z=0 the existing fallthrough returns a quarter turn. |

## `src/open/angle_mod_delta.c`

1 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `angle_mod_delta_le_half_turn` | 7 | `1` | `return ((lhs - rhs) & KF_ANGLE_WRAP_MASK) < (KF_ANGLE_HALF_TURN + 1);` | Add one to turn the inclusive half-turn boundary into the original exclusive comparison bound. |

## `src/open/fixed_vector2_length.c`

0 retained occurrences.

All format/scale conventions already use named constants; this module has no retained numeric or character tokens.
