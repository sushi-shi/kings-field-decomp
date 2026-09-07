# OPEN renderer initialization retained literals

Complete accounting for all **154 retained occurrences** in
`src/open/render_init.c`, accompanying the [overlay-mode review](overlay-mode-domains.md).
Named definitions and ADDRESS/DATA/RODATA claims are excluded; duplicate tokens
have individual rows. The two selectors, buffer/geometry capacities, angles,
reset modes, texture formats, fog setup and perspective shift already have names.

The five color matrices occupy OPEN `0x80035944..0x800359e4`. Their 160 bytes
were compared with the source initializer, compiled before/after objects,
delinked target and retail during the [palette review](palette-domains.md).
Their coefficients remain authored numeric table data under the typed palette
selectors. Translation words are zero initialized SDK members, outside the nine
halfwords interpolated by the lighting helper. White is 4095, while the ending
midpoint and green matrices retain their different measured strengths.

The full initializer unit was forced through the pinned compiler for this
review. OPEN `render_initialize` (`0x80016908`, `0x1d4`) constructs four yaw
matrices, then multiplies the authored light matrix by each. The quarter-turn
indices are coordinates in that rotation family, rather than arbitrary IDs.
The buffer endpoints describe two adjacent allocations of the named size.
OPEN `display_initialize` (`0x80016adc`, `0x1d8`) uses opposite halves of VRAM
for drawing and display, centers the geometry origin, sets SDK boolean flags,
and writes explicit RGB zeros. Its program mode is now a separate enum;
its literal coordinates and flags are unaffected.

OPEN `lighting_set_active_color_matrix` (`0x800168dc`, `0x2c`) and
`primitive_buffer_allocate` (`0x80016cb4`, `0x84`) have no retained literal
tokens. The allocator's byte-count mask is represented by its u16 parameter,
and increment is an operator; the overflow string stays a source string.
The selected matrix helper, renderer initializer and allocator remain 100%.
Display initialization retains its existing partial match; see the mode review
for the current raw verification and first divergence. SDK provider bodies
remain vendored; no new game progress or original TU boundary is claimed.

## `src/open/render_init.c`

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `color_matrix_table` | 17 | `2000` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_DEFAULT, red row m[0][0]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 17 | `700` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_DEFAULT, red row m[0][1]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 17 | `4000` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_DEFAULT, red row m[0][2]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 17 | `2000` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_DEFAULT, green row m[1][0]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 17 | `700` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_DEFAULT, green row m[1][1]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 17 | `4000` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_DEFAULT, green row m[1][2]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 17 | `2000` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_DEFAULT, blue row m[2][0]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 17 | `700` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_DEFAULT, blue row m[2][1]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 17 | `4000` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_DEFAULT, blue row m[2][2]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 17 | `0` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Zero translation component t[0] of KF_OPEN_COLOR_DEFAULT; initialize the SDK member even though color interpolation does not consume it. |
| `color_matrix_table` | 17 | `0` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Zero translation component t[1] of KF_OPEN_COLOR_DEFAULT; initialize the SDK member even though color interpolation does not consume it. |
| `color_matrix_table` | 17 | `0` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Zero translation component t[2] of KF_OPEN_COLOR_DEFAULT; initialize the SDK member even though color interpolation does not consume it. |
| `color_matrix_table` | 18 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_BLACK, red row m[0][0]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 18 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_BLACK, red row m[0][1]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 18 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_BLACK, red row m[0][2]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 18 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_BLACK, green row m[1][0]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 18 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_BLACK, green row m[1][1]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 18 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_BLACK, green row m[1][2]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 18 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_BLACK, blue row m[2][0]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 18 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_BLACK, blue row m[2][1]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 18 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_BLACK, blue row m[2][2]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 18 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Zero translation component t[0] of KF_OPEN_COLOR_BLACK; initialize the SDK member even though color interpolation does not consume it. |
| `color_matrix_table` | 18 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Zero translation component t[1] of KF_OPEN_COLOR_BLACK; initialize the SDK member even though color interpolation does not consume it. |
| `color_matrix_table` | 18 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Zero translation component t[2] of KF_OPEN_COLOR_BLACK; initialize the SDK member even though color interpolation does not consume it. |
| `color_matrix_table` | 19 | `4095` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_WHITE, red row m[0][0]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 19 | `4095` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_WHITE, red row m[0][1]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 19 | `4095` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_WHITE, red row m[0][2]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 19 | `4095` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_WHITE, green row m[1][0]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 19 | `4095` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_WHITE, green row m[1][1]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 19 | `4095` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_WHITE, green row m[1][2]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 19 | `4095` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_WHITE, blue row m[2][0]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 19 | `4095` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_WHITE, blue row m[2][1]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 19 | `4095` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_WHITE, blue row m[2][2]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 19 | `0` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Zero translation component t[0] of KF_OPEN_COLOR_WHITE; initialize the SDK member even though color interpolation does not consume it. |
| `color_matrix_table` | 19 | `0` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Zero translation component t[1] of KF_OPEN_COLOR_WHITE; initialize the SDK member even though color interpolation does not consume it. |
| `color_matrix_table` | 19 | `0` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Zero translation component t[2] of KF_OPEN_COLOR_WHITE; initialize the SDK member even though color interpolation does not consume it. |
| `color_matrix_table` | 20 | `170` | `{{{170, 682, 682}, {170, 341, 341}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_ENDING_MIDPOINT, red row m[0][0]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 20 | `682` | `{{{170, 682, 682}, {170, 341, 341}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_ENDING_MIDPOINT, red row m[0][1]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 20 | `682` | `{{{170, 682, 682}, {170, 341, 341}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_ENDING_MIDPOINT, red row m[0][2]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 20 | `170` | `{{{170, 682, 682}, {170, 341, 341}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_ENDING_MIDPOINT, green row m[1][0]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 20 | `341` | `{{{170, 682, 682}, {170, 341, 341}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_ENDING_MIDPOINT, green row m[1][1]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 20 | `341` | `{{{170, 682, 682}, {170, 341, 341}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_ENDING_MIDPOINT, green row m[1][2]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 20 | `0` | `{{{170, 682, 682}, {170, 341, 341}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_ENDING_MIDPOINT, blue row m[2][0]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 20 | `0` | `{{{170, 682, 682}, {170, 341, 341}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_ENDING_MIDPOINT, blue row m[2][1]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 20 | `0` | `{{{170, 682, 682}, {170, 341, 341}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_ENDING_MIDPOINT, blue row m[2][2]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 20 | `0` | `{{{170, 682, 682}, {170, 341, 341}, {0, 0, 0}}, {0, 0, 0}},` | Zero translation component t[0] of KF_OPEN_COLOR_ENDING_MIDPOINT; initialize the SDK member even though color interpolation does not consume it. |
| `color_matrix_table` | 20 | `0` | `{{{170, 682, 682}, {170, 341, 341}, {0, 0, 0}}, {0, 0, 0}},` | Zero translation component t[1] of KF_OPEN_COLOR_ENDING_MIDPOINT; initialize the SDK member even though color interpolation does not consume it. |
| `color_matrix_table` | 20 | `0` | `{{{170, 682, 682}, {170, 341, 341}, {0, 0, 0}}, {0, 0, 0}},` | Zero translation component t[2] of KF_OPEN_COLOR_ENDING_MIDPOINT; initialize the SDK member even though color interpolation does not consume it. |
| `color_matrix_table` | 21 | `0` | `{{{0, 0, 0}, {375, 375, 375}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_ENDING_GREEN, red row m[0][0]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 21 | `0` | `{{{0, 0, 0}, {375, 375, 375}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_ENDING_GREEN, red row m[0][1]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 21 | `0` | `{{{0, 0, 0}, {375, 375, 375}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_ENDING_GREEN, red row m[0][2]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 21 | `375` | `{{{0, 0, 0}, {375, 375, 375}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_ENDING_GREEN, green row m[1][0]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 21 | `375` | `{{{0, 0, 0}, {375, 375, 375}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_ENDING_GREEN, green row m[1][1]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 21 | `375` | `{{{0, 0, 0}, {375, 375, 375}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_ENDING_GREEN, green row m[1][2]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 21 | `0` | `{{{0, 0, 0}, {375, 375, 375}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_ENDING_GREEN, blue row m[2][0]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 21 | `0` | `{{{0, 0, 0}, {375, 375, 375}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_ENDING_GREEN, blue row m[2][1]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 21 | `0` | `{{{0, 0, 0}, {375, 375, 375}, {0, 0, 0}}, {0, 0, 0}},` | Authored coefficient of KF_OPEN_COLOR_ENDING_GREEN, blue row m[2][2]. Keep the measured numeric matrix under its named preset, including zero contributions. |
| `color_matrix_table` | 21 | `0` | `{{{0, 0, 0}, {375, 375, 375}, {0, 0, 0}}, {0, 0, 0}},` | Zero translation component t[0] of KF_OPEN_COLOR_ENDING_GREEN; initialize the SDK member even though color interpolation does not consume it. |
| `color_matrix_table` | 21 | `0` | `{{{0, 0, 0}, {375, 375, 375}, {0, 0, 0}}, {0, 0, 0}},` | Zero translation component t[1] of KF_OPEN_COLOR_ENDING_GREEN; initialize the SDK member even though color interpolation does not consume it. |
| `color_matrix_table` | 21 | `0` | `{{{0, 0, 0}, {375, 375, 375}, {0, 0, 0}}, {0, 0, 0}},` | Zero translation component t[2] of KF_OPEN_COLOR_ENDING_GREEN; initialize the SDK member even though color interpolation does not consume it. |
| `render_initialize` | 54 | `0` | `open_graphics_runtime.display_state.primitive_buffers[0].start = buffer;` | Ordinal buffer 0 in the two-buffer allocation; start points to the beginning of that buffer. |
| `render_initialize` | 56 | `0` | `open_graphics_runtime.display_state.primitive_buffers[0].end = buffer;` | Ordinal buffer 0 in the two-buffer allocation; end points one past the reserved interval of that buffer. |
| `render_initialize` | 57 | `1` | `open_graphics_runtime.display_state.primitive_buffers[1].start = buffer;` | Ordinal buffer 1 in the two-buffer allocation; start points to the beginning of that buffer. |
| `render_initialize` | 59 | `1` | `open_graphics_runtime.display_state.primitive_buffers[1].end = buffer;` | Ordinal buffer 1 in the two-buffer allocation; end points one past the reserved interval of that buffer. |
| `render_initialize` | 60 | `0` | `open_graphics_runtime.floor_item_state.count = 0;` | Empty floor-item count before resource population; zero is a quantity, not a mode. |
| `render_initialize` | 61 | `0` | `angles.vx = 0;` | Zero rotation component when constructing the initial identity yaw matrix; later yaw assignments use named quarter-turn angles. |
| `render_initialize` | 62 | `0` | `angles.vy = 0;` | Zero rotation component when constructing the initial identity yaw matrix; later yaw assignments use named quarter-turn angles. |
| `render_initialize` | 63 | `0` | `angles.vz = 0;` | Zero rotation component when constructing the initial identity yaw matrix; later yaw assignments use named quarter-turn angles. |
| `render_initialize` | 64 | `0` | `RotMatrix(&angles, &open_graphics_runtime.render_state.quadrant_matrices[0]);` | Yaw-matrix coordinate 0 corresponds to zero; select the matching rotation matrix for construction or multiplication. |
| `render_initialize` | 66 | `3` | `RotMatrix(&angles, &open_graphics_runtime.render_state.quadrant_matrices[3]);` | Yaw-matrix coordinate 3 corresponds to three quarter-turns; select the matching rotation matrix for construction or multiplication. |
| `render_initialize` | 68 | `2` | `RotMatrix(&angles, &open_graphics_runtime.render_state.quadrant_matrices[2]);` | Yaw-matrix coordinate 2 corresponds to one half-turn; select the matching rotation matrix for construction or multiplication. |
| `render_initialize` | 70 | `1` | `RotMatrix(&angles, &open_graphics_runtime.render_state.quadrant_matrices[1]);` | Yaw-matrix coordinate 1 corresponds to one quarter-turn; select the matching rotation matrix for construction or multiplication. |
| `render_initialize` | 71 | `0` | `open_graphics_runtime.render_state.light_matrix.m[0][0] = 3800;` | Explicit light-matrix row 0 coordinate. |
| `render_initialize` | 71 | `0` | `open_graphics_runtime.render_state.light_matrix.m[0][0] = 3800;` | Explicit light-matrix column 0 coordinate. |
| `render_initialize` | 71 | `3800` | `open_graphics_runtime.render_state.light_matrix.m[0][0] = 3800;` | Authored signed light-matrix coefficient m[0][0]; keep the measured value and sign used by MulMatrix0, without inventing physical lighting units. |
| `render_initialize` | 72 | `0` | `open_graphics_runtime.render_state.light_matrix.m[0][1] = -2800;` | Explicit light-matrix row 0 coordinate. |
| `render_initialize` | 72 | `1` | `open_graphics_runtime.render_state.light_matrix.m[0][1] = -2800;` | Explicit light-matrix column 1 coordinate. |
| `render_initialize` | 72 | `2800` | `open_graphics_runtime.render_state.light_matrix.m[0][1] = -2800;` | Authored signed light-matrix coefficient m[0][1]; keep the measured value and sign used by MulMatrix0, without inventing physical lighting units. |
| `render_initialize` | 73 | `0` | `open_graphics_runtime.render_state.light_matrix.m[0][2] = 0;` | Explicit light-matrix row 0 coordinate. |
| `render_initialize` | 73 | `2` | `open_graphics_runtime.render_state.light_matrix.m[0][2] = 0;` | Explicit light-matrix column 2 coordinate. |
| `render_initialize` | 73 | `0` | `open_graphics_runtime.render_state.light_matrix.m[0][2] = 0;` | Authored signed light-matrix coefficient m[0][2]; keep the measured value and sign used by MulMatrix0, without inventing physical lighting units. |
| `render_initialize` | 74 | `1` | `open_graphics_runtime.render_state.light_matrix.m[1][0] = -3000;` | Explicit light-matrix row 1 coordinate. |
| `render_initialize` | 74 | `0` | `open_graphics_runtime.render_state.light_matrix.m[1][0] = -3000;` | Explicit light-matrix column 0 coordinate. |
| `render_initialize` | 74 | `3000` | `open_graphics_runtime.render_state.light_matrix.m[1][0] = -3000;` | Authored signed light-matrix coefficient m[1][0]; keep the measured value and sign used by MulMatrix0, without inventing physical lighting units. |
| `render_initialize` | 75 | `1` | `open_graphics_runtime.render_state.light_matrix.m[1][1] = -3600;` | Explicit light-matrix row 1 coordinate. |
| `render_initialize` | 75 | `1` | `open_graphics_runtime.render_state.light_matrix.m[1][1] = -3600;` | Explicit light-matrix column 1 coordinate. |
| `render_initialize` | 75 | `3600` | `open_graphics_runtime.render_state.light_matrix.m[1][1] = -3600;` | Authored signed light-matrix coefficient m[1][1]; keep the measured value and sign used by MulMatrix0, without inventing physical lighting units. |
| `render_initialize` | 76 | `1` | `open_graphics_runtime.render_state.light_matrix.m[1][2] = -3400;` | Explicit light-matrix row 1 coordinate. |
| `render_initialize` | 76 | `2` | `open_graphics_runtime.render_state.light_matrix.m[1][2] = -3400;` | Explicit light-matrix column 2 coordinate. |
| `render_initialize` | 76 | `3400` | `open_graphics_runtime.render_state.light_matrix.m[1][2] = -3400;` | Authored signed light-matrix coefficient m[1][2]; keep the measured value and sign used by MulMatrix0, without inventing physical lighting units. |
| `render_initialize` | 77 | `2` | `open_graphics_runtime.render_state.light_matrix.m[2][0] = -1300;` | Explicit light-matrix row 2 coordinate. |
| `render_initialize` | 77 | `0` | `open_graphics_runtime.render_state.light_matrix.m[2][0] = -1300;` | Explicit light-matrix column 0 coordinate. |
| `render_initialize` | 77 | `1300` | `open_graphics_runtime.render_state.light_matrix.m[2][0] = -1300;` | Authored signed light-matrix coefficient m[2][0]; keep the measured value and sign used by MulMatrix0, without inventing physical lighting units. |
| `render_initialize` | 78 | `2` | `open_graphics_runtime.render_state.light_matrix.m[2][1] = 2700;` | Explicit light-matrix row 2 coordinate. |
| `render_initialize` | 78 | `1` | `open_graphics_runtime.render_state.light_matrix.m[2][1] = 2700;` | Explicit light-matrix column 1 coordinate. |
| `render_initialize` | 78 | `2700` | `open_graphics_runtime.render_state.light_matrix.m[2][1] = 2700;` | Authored signed light-matrix coefficient m[2][1]; keep the measured value and sign used by MulMatrix0, without inventing physical lighting units. |
| `render_initialize` | 79 | `2` | `open_graphics_runtime.render_state.light_matrix.m[2][2] = 800;` | Explicit light-matrix row 2 coordinate. |
| `render_initialize` | 79 | `2` | `open_graphics_runtime.render_state.light_matrix.m[2][2] = 800;` | Explicit light-matrix column 2 coordinate. |
| `render_initialize` | 79 | `800` | `open_graphics_runtime.render_state.light_matrix.m[2][2] = 800;` | Authored signed light-matrix coefficient m[2][2]; keep the measured value and sign used by MulMatrix0, without inventing physical lighting units. |
| `render_initialize` | 82 | `0` | `&open_graphics_runtime.render_state.quadrant_matrices[0],` | Yaw-matrix coordinate 0 corresponds to zero; select the matching rotation matrix for construction or multiplication. |
| `render_initialize` | 83 | `0` | `&open_graphics_runtime.light_quadrant_matrices[0]);` | Yaw-matrix coordinate 0 corresponds to zero; store the resulting rotated light matrix in the corresponding slot. |
| `render_initialize` | 86 | `1` | `&open_graphics_runtime.render_state.quadrant_matrices[1],` | Yaw-matrix coordinate 1 corresponds to one quarter-turn; select the matching rotation matrix for construction or multiplication. |
| `render_initialize` | 87 | `1` | `&open_graphics_runtime.light_quadrant_matrices[1]);` | Yaw-matrix coordinate 1 corresponds to one quarter-turn; store the resulting rotated light matrix in the corresponding slot. |
| `render_initialize` | 90 | `2` | `&open_graphics_runtime.render_state.quadrant_matrices[2],` | Yaw-matrix coordinate 2 corresponds to one half-turn; select the matching rotation matrix for construction or multiplication. |
| `render_initialize` | 91 | `2` | `&open_graphics_runtime.light_quadrant_matrices[2]);` | Yaw-matrix coordinate 2 corresponds to one half-turn; store the resulting rotated light matrix in the corresponding slot. |
| `render_initialize` | 94 | `3` | `&open_graphics_runtime.render_state.quadrant_matrices[3],` | Yaw-matrix coordinate 3 corresponds to three quarter-turns; select the matching rotation matrix for construction or multiplication. |
| `render_initialize` | 95 | `3` | `&open_graphics_runtime.light_quadrant_matrices[3]);` | Yaw-matrix coordinate 3 corresponds to three quarter-turns; store the resulting rotated light matrix in the corresponding slot. |
| `render_initialize` | 98 | `0` | `FLOOR_ITEM_TPAGE_X, 0);` | Texture-page Y coordinate at the upper VRAM origin; the X location and texture/blend formats are already named. |
| `display_initialize` | 117 | `2` | `SetGeomOffset(KF_DISPLAY_WIDTH / 2, KF_DISPLAY_HEIGHT / 2);` | Divide the named framebuffer width by two to center the geometry origin on that axis. |
| `display_initialize` | 117 | `2` | `SetGeomOffset(KF_DISPLAY_WIDTH / 2, KF_DISPLAY_HEIGHT / 2);` | Divide the named framebuffer height by two to center the geometry origin on that axis. |
| `display_initialize` | 119 | `0` | `&open_graphics_runtime.display_draw_environments[0], 0, 0,` | Ordinal environment slot 0 in the SDK double-buffer pair; draw and display rectangles occupy opposite framebuffer halves. |
| `display_initialize` | 119 | `0` | `&open_graphics_runtime.display_draw_environments[0], 0, 0,` | Zero X origin of the upper framebuffer rectangle; the other buffer uses the named lower-buffer Y coordinate. |
| `display_initialize` | 119 | `0` | `&open_graphics_runtime.display_draw_environments[0], 0, 0,` | Zero Y origin of the upper framebuffer rectangle; the other buffer uses the named lower-buffer Y coordinate. |
| `display_initialize` | 122 | `0` | `&open_graphics_runtime.display_disp_environments[0],` | Ordinal environment slot 0 in the SDK double-buffer pair; draw and display rectangles occupy opposite framebuffer halves. |
| `display_initialize` | 123 | `0` | `0,` | Zero X origin of the lower framebuffer rectangle; its Y coordinate is supplied separately by lower_buffer_y. |
| `display_initialize` | 128 | `1` | `&open_graphics_runtime.display_draw_environments[1],` | Ordinal environment slot 1 in the SDK double-buffer pair; draw and display rectangles occupy opposite framebuffer halves. |
| `display_initialize` | 129 | `0` | `0,` | Zero X origin of the lower framebuffer rectangle; its Y coordinate is supplied separately by lower_buffer_y. |
| `display_initialize` | 134 | `1` | `&open_graphics_runtime.display_disp_environments[1], 0, 0,` | Ordinal environment slot 1 in the SDK double-buffer pair; draw and display rectangles occupy opposite framebuffer halves. |
| `display_initialize` | 134 | `0` | `&open_graphics_runtime.display_disp_environments[1], 0, 0,` | Zero X origin of the upper framebuffer rectangle; the other buffer uses the named lower-buffer Y coordinate. |
| `display_initialize` | 134 | `0` | `&open_graphics_runtime.display_disp_environments[1], 0, 0,` | Zero Y origin of the upper framebuffer rectangle; the other buffer uses the named lower-buffer Y coordinate. |
| `display_initialize` | 136 | `0` | `open_graphics_runtime.display_draw_environments[0].dtd = open_graphics_runtime.display_draw_environments[1].dtd = 1;` | Ordinal environment slot 0 in the SDK double-buffer pair; draw and display rectangles occupy opposite framebuffer halves. |
| `display_initialize` | 136 | `1` | `open_graphics_runtime.display_draw_environments[0].dtd = open_graphics_runtime.display_draw_environments[1].dtd = 1;` | Ordinal environment slot 1 in the SDK double-buffer pair; draw and display rectangles occupy opposite framebuffer halves. |
| `display_initialize` | 136 | `1` | `open_graphics_runtime.display_draw_environments[0].dtd = open_graphics_runtime.display_draw_environments[1].dtd = 1;` | SDK boolean true enables dithering for both drawing environments. |
| `display_initialize` | 137 | `0` | `open_graphics_runtime.display_draw_environments[0].isbg = 1;` | Ordinal environment slot 0 in the SDK double-buffer pair; draw and display rectangles occupy opposite framebuffer halves. |
| `display_initialize` | 137 | `1` | `open_graphics_runtime.display_draw_environments[0].isbg = 1;` | SDK boolean true enables automatic background clearing when the drawing environment is submitted. |
| `display_initialize` | 138 | `1` | `open_graphics_runtime.display_draw_environments[1].isbg = 1;` | Ordinal environment slot 1 in the SDK double-buffer pair; draw and display rectangles occupy opposite framebuffer halves. |
| `display_initialize` | 138 | `1` | `open_graphics_runtime.display_draw_environments[1].isbg = 1;` | SDK boolean true enables automatic background clearing when the drawing environment is submitted. |
| `display_initialize` | 139 | `0` | `open_graphics_runtime.display_draw_environments[0].r0 = 0;` | Ordinal environment slot 0 in the SDK double-buffer pair; draw and display rectangles occupy opposite framebuffer halves. |
| `display_initialize` | 139 | `0` | `open_graphics_runtime.display_draw_environments[0].r0 = 0;` | Zero R component of the black drawing-environment background color. |
| `display_initialize` | 140 | `0` | `open_graphics_runtime.display_draw_environments[0].g0 = 0;` | Ordinal environment slot 0 in the SDK double-buffer pair; draw and display rectangles occupy opposite framebuffer halves. |
| `display_initialize` | 140 | `0` | `open_graphics_runtime.display_draw_environments[0].g0 = 0;` | Zero G component of the black drawing-environment background color. |
| `display_initialize` | 141 | `0` | `open_graphics_runtime.display_draw_environments[0].b0 = 0;` | Ordinal environment slot 0 in the SDK double-buffer pair; draw and display rectangles occupy opposite framebuffer halves. |
| `display_initialize` | 141 | `0` | `open_graphics_runtime.display_draw_environments[0].b0 = 0;` | Zero B component of the black drawing-environment background color. |
| `display_initialize` | 142 | `1` | `open_graphics_runtime.display_draw_environments[1].r0 = 0;` | Ordinal environment slot 1 in the SDK double-buffer pair; draw and display rectangles occupy opposite framebuffer halves. |
| `display_initialize` | 142 | `0` | `open_graphics_runtime.display_draw_environments[1].r0 = 0;` | Zero R component of the black drawing-environment background color. |
| `display_initialize` | 143 | `1` | `open_graphics_runtime.display_draw_environments[1].g0 = 0;` | Ordinal environment slot 1 in the SDK double-buffer pair; draw and display rectangles occupy opposite framebuffer halves. |
| `display_initialize` | 143 | `0` | `open_graphics_runtime.display_draw_environments[1].g0 = 0;` | Zero G component of the black drawing-environment background color. |
| `display_initialize` | 144 | `1` | `open_graphics_runtime.display_draw_environments[1].b0 = 0;` | Ordinal environment slot 1 in the SDK double-buffer pair; draw and display rectangles occupy opposite framebuffer halves. |
| `display_initialize` | 144 | `0` | `open_graphics_runtime.display_draw_environments[1].b0 = 0;` | Zero B component of the black drawing-environment background color. |
| `display_initialize` | 145 | `0` | `first_draw = &open_graphics_runtime.display_draw_environments[0];` | Ordinal environment slot 0 in the SDK double-buffer pair; draw and display rectangles occupy opposite framebuffer halves. |
| `display_initialize` | 146 | `1` | `second_draw = &open_graphics_runtime.display_draw_environments[1];` | Ordinal environment slot 1 in the SDK double-buffer pair; draw and display rectangles occupy opposite framebuffer halves. |
| `display_initialize` | 148 | `0` | `PutDispEnv(&open_graphics_runtime.display_disp_environments[0]);` | Ordinal environment slot 0 in the SDK double-buffer pair; draw and display rectangles occupy opposite framebuffer halves. |
| `display_initialize` | 149 | `1` | `SetDispMask(1);` | SDK display-enable boolean: one enables display on the ending path. |
| `display_initialize` | 151 | `0` | `first_draw->dfe = 0;` | SDK boolean false disables drawing in the displayed area; preserve the stores before and after submitting both drawing environments. |
| `display_initialize` | 152 | `0` | `second_draw->dfe = 0;` | SDK boolean false disables drawing in the displayed area; preserve the stores before and after submitting both drawing environments. |
| `display_initialize` | 155 | `1` | `first_draw->dfe = 1;` | SDK boolean true enables drawing in the displayed area; preserve the stores before and after submitting both drawing environments. |
| `display_initialize` | 156 | `1` | `second_draw->dfe = 1;` | SDK boolean true enables drawing in the displayed area; preserve the stores before and after submitting both drawing environments. |
| `display_initialize` | 157 | `0` | `SetDispMask(0);` | SDK display-enable boolean: zero hides display on the normal initialization path. |
| `display_initialize` | 159 | `0` | `SetBackColor(0, 0, 0);` | Zero red component of the GTE background color, independent of the framebuffer clear color. |
| `display_initialize` | 159 | `0` | `SetBackColor(0, 0, 0);` | Zero green component of the GTE background color, independent of the framebuffer clear color. |
| `display_initialize` | 159 | `0` | `SetBackColor(0, 0, 0);` | Zero blue component of the GTE background color, independent of the framebuffer clear color. |
| `display_initialize` | 161 | `0` | `SetFarColor(0, 0, 0);` | Zero red component of the GTE far color used for depth cueing. |
| `display_initialize` | 161 | `0` | `SetFarColor(0, 0, 0);` | Zero green component of the GTE far color used for depth cueing. |
| `display_initialize` | 161 | `0` | `SetFarColor(0, 0, 0);` | Zero blue component of the GTE far color used for depth cueing. |
