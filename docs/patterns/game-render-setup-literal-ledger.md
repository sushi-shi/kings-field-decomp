# GAME renderer setup retained literals

Complete accounting for **146 numeric/character occurrences** in
`src/game/render.c`. Claims, named definitions, comments and strings are
excluded. Each duplicate token has its own field/argument-specific reason.

The existing [GPU/setup review](source-constant-names.md#gpu-setup-system-screens-and-transition-fades),
[lighting review](game-lighting-literal-ledger.md),
[TMD count evidence](tmd-counts.md) and [system-screen types](visibility-screen-domains.md)
establish these meanings. This pass adds documentation without changing this
source file. Authored Q12 coefficients remain data values; indices identify
matrix components and explicit buffers rather than new state domains.

Frame-reset data with address-derived identities remain semantically unresolved.
Documenting a zero reset does not identify its complete owner or runtime role.
Source accounting is not a new binary-match result. Builds and tests remain
deferred under the naming-pass policy.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 38 | `7` | `char error_screen_path[7] = "\\E0.;1";` | Seven-byte authored path storage includes the terminating NUL copied into the CD path buffer. |
| `display_show_error_screen` | 58 | `0` | `DrawSync(0);` | Authentic SDK blocking GPU synchronization mode. |
| `display_show_error_screen` | 60 | `1` | `SetSemiTrans(&prim, 1);` | Boolean true enables the SDK primitive semi-transparency flag. |
| `display_show_error_screen` | 69 | `0` | `prim.u0 = 0;` | Zero U texture origin for this explicitly assigned corner. |
| `display_show_error_screen` | 70 | `0` | `prim.v0 = 0;` | Zero V texture origin for this explicitly assigned corner. |
| `display_show_error_screen` | 72 | `0` | `prim.v1 = 0;` | Zero V texture origin for this explicitly assigned corner. |
| `display_show_error_screen` | 73 | `0` | `prim.u2 = 0;` | Zero U texture origin for this explicitly assigned corner. |
| `display_show_error_screen` | 77 | `0` | `prim.clut = GetClut(0, KF_SYSTEM_SCREEN_CLUT_Y);` | System-screen palette begins at VRAM X zero; its row has a separate named constant. |
| `display_show_error_screen` | 83 | `'0'` | `cd_path_buffer[SYSTEM_SCREEN_PATH_DIGIT] = KF_ENUM_ENCODE(s32, stage) + '0';` | ASCII zero converts the encoded system-screen selector to its filename digit. |
| `display_show_error_screen` | 85 | `0` | `if (CdSearchFile(&cd_search_file, cd_path_buffer) == 0) {` | Null file-lookup result signals failure before a read is attempted. |
| `display_show_error_screen` | 86 | `1` | `exit(1);` | Nonzero SDK process exit status reports the failed required file lookup. |
| `display_show_error_screen` | 88 | `1` | `if (cd_search_file.size & (KF_CD_SECTOR_BYTES - 1)) {` | One below the power-of-two sector size masks the partial-sector remainder. |
| `display_show_error_screen` | 90 | `1` | `((cd_search_file.size >> KF_CD_SECTOR_SHIFT) + 1) << KF_CD_SECTOR_SHIFT;` | Add one sector when rounding a nonaligned file size up to complete CD sectors. |
| `display_show_error_screen` | 95 | `0` | `for (attempt = 0; attempt < ERROR_SCREEN_READ_ATTEMPTS; attempt++) {` | First read-attempt ordinal; the retry limit is separately named. |
| `display_show_error_screen` | 98 | `0` | `CdControl(CdlSetloc, (u_char *)&cd_read_location, 0);` | Null optional command-result buffer; the caller uses the later read status. |
| `display_show_error_screen` | 101 | `0` | `while ((result = CdReadSync(KF_CD_READ_POLL, 0)) > 0) {` | Null optional read-result buffer. |
| `display_show_error_screen` | 101 | `0` | `while ((result = CdReadSync(KF_CD_READ_POLL, 0)) > 0) {` | Positive remaining work keeps polling; zero or negative ends the wait. |
| `display_show_error_screen` | 103 | `0` | `if (result == 0) {` | Zero remaining work identifies a successful completed CD read. |
| `display_show_error_screen` | 109 | `0` | `back = display_state.buffer_index == 0;` | Boolean inversion selects the other of the two display buffers. |
| `display_show_error_screen` | 110 | `0` | `display_draw_environments[back].isbg = 0;` | Boolean false disables SDK automatic background clearing. |
| `display_show_error_screen` | 111 | `0` | `display_draw_environments[back].dfe = 0;` | Boolean false disables SDK drawing into the display area. |
| `display_show_error_screen` | 117 | `0` | `DrawSync(0);` | Authentic SDK blocking GPU synchronization mode. |
| `display_show_error_screen` | 118 | `1` | `DrawOTag(display_state.ordering_table + (KF_ORDERING_TABLE_LENGTH - 1));` | Subtract one from the reversed ordering-table length to address its last word. |
| `display_show_error_screen` | 119 | `1` | `while (PadRead(1) == 0) {` | Ignored call-site argument retained for the linked SDK interface; PadRead uses global PadIdentifier. |
| `display_show_error_screen` | 119 | `0` | `while (PadRead(1) == 0) {` | Zero returned button bits means all released; preserve the press/release wait predicate. |
| `display_show_error_screen` | 121 | `1` | `while (PadRead(1) != 0) {` | Ignored call-site argument retained for the linked SDK interface; PadRead uses global PadIdentifier. |
| `display_show_error_screen` | 121 | `0` | `while (PadRead(1) != 0) {` | Zero returned button bits means all released; preserve the press/release wait predicate. |
| `display_show_error_screen` | 123 | `1` | `display_draw_environments[back].isbg = 1;` | Boolean true enables SDK automatic background clearing. |
| `display_show_error_screen` | 124 | `1` | `display_draw_environments[back].dfe = 1;` | Boolean true restores SDK drawing into the display area. |
| `display_show_error_screen` | 125 | `0` | `DrawSync(0);` | Authentic SDK blocking GPU synchronization mode. |
| `effect5_texture_cache_prepare` | 138 | `0` | `effect5_texture_pages[0] = GetTPage(` | Texture descriptor ordinal 0; the cache stores three authored page choices in matching order. |
| `effect5_texture_cache_prepare` | 141 | `1` | `effect5_texture_pages[1] = GetTPage(` | Texture descriptor ordinal 1; the cache stores three authored page choices in matching order. |
| `effect5_texture_cache_prepare` | 144 | `2` | `effect5_texture_pages[2] = GetTPage(` | Texture descriptor ordinal 2; the cache stores three authored page choices in matching order. |
| `effect5_texture_cache_prepare` | 147 | `0` | `effect5_texture_cluts[0] = GetClut(0, EFFECT_TEXTURE_CLUT_Y);` | Texture descriptor ordinal 0 pairs this palette with its corresponding cached page. |
| `effect5_texture_cache_prepare` | 147 | `0` | `effect5_texture_cluts[0] = GetClut(0, EFFECT_TEXTURE_CLUT_Y);` | All three cached palettes begin at VRAM X zero on the named row. |
| `effect5_texture_cache_prepare` | 148 | `1` | `effect5_texture_cluts[1] = GetClut(0, EFFECT_TEXTURE_CLUT_Y);` | Texture descriptor ordinal 1 pairs this palette with its corresponding cached page. |
| `effect5_texture_cache_prepare` | 148 | `0` | `effect5_texture_cluts[1] = GetClut(0, EFFECT_TEXTURE_CLUT_Y);` | All three cached palettes begin at VRAM X zero on the named row. |
| `effect5_texture_cache_prepare` | 149 | `2` | `effect5_texture_cluts[2] = GetClut(0, EFFECT_TEXTURE_CLUT_Y);` | Texture descriptor ordinal 2 pairs this palette with its corresponding cached page. |
| `effect5_texture_cache_prepare` | 149 | `0` | `effect5_texture_cluts[2] = GetClut(0, EFFECT_TEXTURE_CLUT_Y);` | All three cached palettes begin at VRAM X zero on the named row. |
| `display_initialize` | 158 | `2` | `SetGeomOffset(KF_DISPLAY_WIDTH / 2, KF_DISPLAY_HEIGHT / 2);` | Divide the display width by two to center the projection origin. |
| `display_initialize` | 158 | `2` | `SetGeomOffset(KF_DISPLAY_WIDTH / 2, KF_DISPLAY_HEIGHT / 2);` | Divide the display height by two to center the projection origin. |
| `display_initialize` | 160 | `0` | `&display_draw_environments[0], 0, 0,` | Explicit environment record 0 of the two-buffer pair. |
| `display_initialize` | 160 | `0` | `&display_draw_environments[0], 0, 0,` | Zero X VRAM origin of this draw/display rectangle. |
| `display_initialize` | 160 | `0` | `&display_draw_environments[0], 0, 0,` | Zero Y VRAM origin of this draw/display rectangle. |
| `display_initialize` | 163 | `0` | `&display_disp_environments[0], 0, KF_DISPLAY_HEIGHT,` | Explicit environment record 0 of the two-buffer pair. |
| `display_initialize` | 163 | `0` | `&display_disp_environments[0], 0, KF_DISPLAY_HEIGHT,` | Zero X VRAM origin of this draw/display rectangle. |
| `display_initialize` | 166 | `1` | `&display_draw_environments[1], 0, KF_DISPLAY_HEIGHT,` | Explicit environment record 1 of the two-buffer pair. |
| `display_initialize` | 166 | `0` | `&display_draw_environments[1], 0, KF_DISPLAY_HEIGHT,` | Zero X VRAM origin of this draw/display rectangle. |
| `display_initialize` | 169 | `1` | `&display_disp_environments[1], 0, 0,` | Explicit environment record 1 of the two-buffer pair. |
| `display_initialize` | 169 | `0` | `&display_disp_environments[1], 0, 0,` | Zero X VRAM origin of this draw/display rectangle. |
| `display_initialize` | 169 | `0` | `&display_disp_environments[1], 0, 0,` | Zero Y VRAM origin of this draw/display rectangle. |
| `display_initialize` | 171 | `0` | `display_draw_environments[0].dtd = display_draw_environments[1].dtd = 1;` | Explicit draw-environment record 0 receives the same dithering flag. |
| `display_initialize` | 171 | `1` | `display_draw_environments[0].dtd = display_draw_environments[1].dtd = 1;` | Explicit draw-environment record 1 receives the same dithering flag. |
| `display_initialize` | 171 | `1` | `display_draw_environments[0].dtd = display_draw_environments[1].dtd = 1;` | Boolean true enables SDK dithering in both draw environments. |
| `display_initialize` | 172 | `0` | `display_draw_environments[0].isbg = 1;` | Explicit display-environment record 0 of the two-buffer pair. |
| `display_initialize` | 172 | `1` | `display_draw_environments[0].isbg = 1;` | Boolean true enables SDK automatic background clearing. |
| `display_initialize` | 173 | `1` | `display_draw_environments[1].isbg = 1;` | Explicit display-environment record 1 of the two-buffer pair. |
| `display_initialize` | 173 | `1` | `display_draw_environments[1].isbg = 1;` | Boolean true enables SDK automatic background clearing. |
| `display_initialize` | 174 | `0` | `display_draw_environments[0].r0 = 0;` | Explicit draw-environment record 0 of the two-buffer pair. |
| `display_initialize` | 174 | `0` | `display_draw_environments[0].r0 = 0;` | Black red background component. |
| `display_initialize` | 175 | `0` | `display_draw_environments[0].g0 = 0;` | Explicit draw-environment record 0 of the two-buffer pair. |
| `display_initialize` | 175 | `0` | `display_draw_environments[0].g0 = 0;` | Black green background component. |
| `display_initialize` | 176 | `0` | `display_draw_environments[0].b0 = 0;` | Explicit draw-environment record 0 of the two-buffer pair. |
| `display_initialize` | 176 | `0` | `display_draw_environments[0].b0 = 0;` | Black blue background component. |
| `display_initialize` | 177 | `1` | `display_draw_environments[1].r0 = 0;` | Explicit draw-environment record 1 of the two-buffer pair. |
| `display_initialize` | 177 | `0` | `display_draw_environments[1].r0 = 0;` | Black red background component. |
| `display_initialize` | 178 | `1` | `display_draw_environments[1].g0 = 0;` | Explicit draw-environment record 1 of the two-buffer pair. |
| `display_initialize` | 178 | `0` | `display_draw_environments[1].g0 = 0;` | Black green background component. |
| `display_initialize` | 179 | `1` | `display_draw_environments[1].b0 = 0;` | Explicit draw-environment record 1 of the two-buffer pair. |
| `display_initialize` | 179 | `0` | `display_draw_environments[1].b0 = 0;` | Black blue background component. |
| `display_initialize` | 180 | `0` | `PutDispEnv(&display_disp_environments[0]);` | Initial display-environment record zero is submitted explicitly. |
| `display_initialize` | 183 | `0` | `SetFarColor(0, 0, 0);` | Black far-fog red component supplied to the SDK. |
| `display_initialize` | 183 | `0` | `SetFarColor(0, 0, 0);` | Black far-fog green component supplied to the SDK. |
| `display_initialize` | 183 | `0` | `SetFarColor(0, 0, 0);` | Black far-fog blue component supplied to the SDK. |
| `render_initialize` | 200 | `0` | `display_state.primitive_buffers[0].start = buffer;` | Explicit primitive-buffer interval 0 in the contiguous two-buffer allocation. |
| `render_initialize` | 202 | `0` | `display_state.primitive_buffers[0].end = buffer;` | Explicit primitive-buffer interval 0 in the contiguous two-buffer allocation. |
| `render_initialize` | 203 | `1` | `display_state.primitive_buffers[1].start = buffer;` | Explicit primitive-buffer interval 1 in the contiguous two-buffer allocation. |
| `render_initialize` | 205 | `1` | `display_state.primitive_buffers[1].end = buffer;` | Explicit primitive-buffer interval 1 in the contiguous two-buffer allocation. |
| `render_initialize` | 206 | `0` | `floor_item_count = 0;` | No floor placements are loaded at renderer initialization. |
| `render_initialize` | 207 | `0` | `angles.vx = 0;` | Zero rotation on this axis; subsequent assignments select the named yaw quadrants or retain only pitch. |
| `render_initialize` | 208 | `0` | `angles.vy = 0;` | Zero rotation on this axis; subsequent assignments select the named yaw quadrants or retain only pitch. |
| `render_initialize` | 209 | `0` | `angles.vz = 0;` | Zero rotation on this axis; subsequent assignments select the named yaw quadrants or retain only pitch. |
| `render_initialize` | 210 | `0` | `RotMatrix(&angles, &render_state.quadrant_matrices[0]);` | Quarter-turn matrix ordinal 0 selects the matching yaw and derived light-matrix slot. |
| `render_initialize` | 212 | `3` | `RotMatrix(&angles, &render_state.quadrant_matrices[3]);` | Quarter-turn matrix ordinal 3 selects the matching yaw and derived light-matrix slot. |
| `render_initialize` | 214 | `2` | `RotMatrix(&angles, &render_state.quadrant_matrices[2]);` | Quarter-turn matrix ordinal 2 selects the matching yaw and derived light-matrix slot. |
| `render_initialize` | 216 | `1` | `RotMatrix(&angles, &render_state.quadrant_matrices[1]);` | Quarter-turn matrix ordinal 1 selects the matching yaw and derived light-matrix slot. |
| `render_initialize` | 217 | `0` | `render_state.light_matrix.m[0][0] = 3800;` | Light matrix row index 0 in the explicit three-by-three matrix. |
| `render_initialize` | 217 | `0` | `render_state.light_matrix.m[0][0] = 3800;` | Light matrix column index 0 in the explicit three-by-three matrix. |
| `render_initialize` | 217 | `3800` | `render_state.light_matrix.m[0][0] = 3800;` | Authored Q12 lighting-matrix coefficient; preserve its sign and magnitude without assuming a normalized direction or inventing a tuning rationale. |
| `render_initialize` | 218 | `0` | `render_state.light_matrix.m[0][1] = -2800;` | Light matrix row index 0 in the explicit three-by-three matrix. |
| `render_initialize` | 218 | `1` | `render_state.light_matrix.m[0][1] = -2800;` | Light matrix column index 1 in the explicit three-by-three matrix. |
| `render_initialize` | 218 | `2800` | `render_state.light_matrix.m[0][1] = -2800;` | Authored Q12 lighting-matrix coefficient; preserve its sign and magnitude without assuming a normalized direction or inventing a tuning rationale. |
| `render_initialize` | 219 | `0` | `render_state.light_matrix.m[0][2] = 0;` | Light matrix row index 0 in the explicit three-by-three matrix. |
| `render_initialize` | 219 | `2` | `render_state.light_matrix.m[0][2] = 0;` | Light matrix column index 2 in the explicit three-by-three matrix. |
| `render_initialize` | 219 | `0` | `render_state.light_matrix.m[0][2] = 0;` | Authored Q12 lighting-matrix coefficient; preserve its sign and magnitude without assuming a normalized direction or inventing a tuning rationale. |
| `render_initialize` | 220 | `1` | `render_state.light_matrix.m[1][0] = -3000;` | Light matrix row index 1 in the explicit three-by-three matrix. |
| `render_initialize` | 220 | `0` | `render_state.light_matrix.m[1][0] = -3000;` | Light matrix column index 0 in the explicit three-by-three matrix. |
| `render_initialize` | 220 | `3000` | `render_state.light_matrix.m[1][0] = -3000;` | Authored Q12 lighting-matrix coefficient; preserve its sign and magnitude without assuming a normalized direction or inventing a tuning rationale. |
| `render_initialize` | 221 | `1` | `render_state.light_matrix.m[1][1] = -3600;` | Light matrix row index 1 in the explicit three-by-three matrix. |
| `render_initialize` | 221 | `1` | `render_state.light_matrix.m[1][1] = -3600;` | Light matrix column index 1 in the explicit three-by-three matrix. |
| `render_initialize` | 221 | `3600` | `render_state.light_matrix.m[1][1] = -3600;` | Authored Q12 lighting-matrix coefficient; preserve its sign and magnitude without assuming a normalized direction or inventing a tuning rationale. |
| `render_initialize` | 222 | `1` | `render_state.light_matrix.m[1][2] = -3400;` | Light matrix row index 1 in the explicit three-by-three matrix. |
| `render_initialize` | 222 | `2` | `render_state.light_matrix.m[1][2] = -3400;` | Light matrix column index 2 in the explicit three-by-three matrix. |
| `render_initialize` | 222 | `3400` | `render_state.light_matrix.m[1][2] = -3400;` | Authored Q12 lighting-matrix coefficient; preserve its sign and magnitude without assuming a normalized direction or inventing a tuning rationale. |
| `render_initialize` | 223 | `2` | `render_state.light_matrix.m[2][0] = -1300;` | Light matrix row index 2 in the explicit three-by-three matrix. |
| `render_initialize` | 223 | `0` | `render_state.light_matrix.m[2][0] = -1300;` | Light matrix column index 0 in the explicit three-by-three matrix. |
| `render_initialize` | 223 | `1300` | `render_state.light_matrix.m[2][0] = -1300;` | Authored Q12 lighting-matrix coefficient; preserve its sign and magnitude without assuming a normalized direction or inventing a tuning rationale. |
| `render_initialize` | 224 | `2` | `render_state.light_matrix.m[2][1] = 2700;` | Light matrix row index 2 in the explicit three-by-three matrix. |
| `render_initialize` | 224 | `1` | `render_state.light_matrix.m[2][1] = 2700;` | Light matrix column index 1 in the explicit three-by-three matrix. |
| `render_initialize` | 224 | `2700` | `render_state.light_matrix.m[2][1] = 2700;` | Authored Q12 lighting-matrix coefficient; preserve its sign and magnitude without assuming a normalized direction or inventing a tuning rationale. |
| `render_initialize` | 225 | `2` | `render_state.light_matrix.m[2][2] = 800;` | Light matrix row index 2 in the explicit three-by-three matrix. |
| `render_initialize` | 225 | `2` | `render_state.light_matrix.m[2][2] = 800;` | Light matrix column index 2 in the explicit three-by-three matrix. |
| `render_initialize` | 225 | `800` | `render_state.light_matrix.m[2][2] = 800;` | Authored Q12 lighting-matrix coefficient; preserve its sign and magnitude without assuming a normalized direction or inventing a tuning rationale. |
| `render_initialize` | 229 | `0` | `&render_state.quadrant_matrices[0],` | Quarter-turn matrix ordinal 0 selects the matching yaw and derived light-matrix slot. |
| `render_initialize` | 230 | `0` | `&light_quadrant_matrices[0]);` | Quarter-turn matrix ordinal 0 selects the matching yaw and derived light-matrix slot. |
| `render_initialize` | 233 | `1` | `&render_state.quadrant_matrices[1],` | Quarter-turn matrix ordinal 1 selects the matching yaw and derived light-matrix slot. |
| `render_initialize` | 234 | `1` | `&light_quadrant_matrices[1]);` | Quarter-turn matrix ordinal 1 selects the matching yaw and derived light-matrix slot. |
| `render_initialize` | 237 | `2` | `&render_state.quadrant_matrices[2],` | Quarter-turn matrix ordinal 2 selects the matching yaw and derived light-matrix slot. |
| `render_initialize` | 238 | `2` | `&light_quadrant_matrices[2]);` | Quarter-turn matrix ordinal 2 selects the matching yaw and derived light-matrix slot. |
| `render_initialize` | 241 | `3` | `&render_state.quadrant_matrices[3],` | Quarter-turn matrix ordinal 3 selects the matching yaw and derived light-matrix slot. |
| `render_initialize` | 242 | `3` | `&light_quadrant_matrices[3]);` | Quarter-turn matrix ordinal 3 selects the matching yaw and derived light-matrix slot. |
| `render_initialize` | 245 | `0` | `FLOOR_ITEM_TPAGE_X, 0);` | The floor-item texture page uses the top VRAM row Y zero. |
| `render_initialize` | 258 | `0` | `notification_state.control.queue_tail = 0;` | Initialize the ring-buffer cursor to the first slot; the notification-state enum is separate. |
| `render_initialize` | 259 | `0` | `notification_state.control.queue_head = 0;` | Initialize the ring-buffer cursor to the first slot; the notification-state enum is separate. |
| `render_initialize` | 261 | `1` | `count = KF_NOTIFICATION_CAPACITY - 1;` | Initialize the post-decrement clear loop to the last notification slot ordinal. |
| `render_initialize` | 264 | `0` | `} while (count-- != 0);` | Zero terminates the post-decrement notification clear loop after its last store. |
| `display_begin_frame` | 271 | `0` | `display_state.buffer_index = display_state.buffer_index == 0;` | Boolean inversion selects the other of the two display buffers. |
| `display_begin_frame` | 277 | `0` | `DAT_800a0768 = 0;` | Observed zero reset at frame start; the graphics datum retains an unresolved identity, so no additional role is inferred. |
| `display_begin_frame` | 278 | `0` | `DAT_8009569c = 0;` | Observed zero reset at frame start; the graphics datum retains an unresolved identity, so no additional role is inferred. |
| `display_begin_frame` | 279 | `0` | `DAT_80095698 = 0;` | Observed zero reset at frame start; the graphics datum retains an unresolved identity, so no additional role is inferred. |
| `display_present_frame` | 285 | `0` | `DrawSync(0);` | Authentic SDK blocking GPU synchronization mode. |
| `display_present_frame` | 286 | `0` | `VSync(0);` | Authentic SDK next-vertical-blank wait mode for frame pacing. |
| `display_present_frame` | 289 | `1` | `DrawOTag(display_state.ordering_table + (KF_ORDERING_TABLE_LENGTH - 1));` | Subtract one from the reversed ordering-table length to address its last word. |
| `render_set_view_transform` | 324 | `0` | `if (position != 0) {` | Null optional transform input retains the corresponding current view value. |
| `render_set_view_transform` | 329 | `0` | `if (rotation != 0) {` | Null optional transform input retains the corresponding current view value. |
| `render_set_view_transform` | 333 | `0` | `angles.vz = 0;` | Zero rotation on this axis; subsequent assignments select the named yaw quadrants or retain only pitch. |
| `render_set_view_transform` | 334 | `0` | `angles.vy = 0;` | Zero rotation on this axis; subsequent assignments select the named yaw quadrants or retain only pitch. |
| `tmd_prepare_primitive_indices` | 356 | `1` | `objects_left = object_count - 1;` | Count-minus-one initializes the halfword post-decrement object traversal; the zero-count guard handles empty input. |
| `tmd_prepare_primitive_indices` | 358 | `0` | `if (object_count == 0) {` | Empty TMD object list exits before dereferencing an object. |
| `tmd_prepare_primitive_indices` | 366 | `0` | `if (primitive_count != 0) {` | Only a nonempty primitive list enters the packet-body loop. |
| `tmd_prepare_primitive_indices` | 451 | `0` | `} while (primitives_left-- != 0);` | Zero is the terminal value of the unsigned halfword post-decrement traversal. |
| `tmd_prepare_primitive_indices` | 454 | `0` | `} while (objects_left-- != 0);` | Zero is the terminal value of the unsigned halfword post-decrement traversal. |
| `tmd_project_vertices` | 483 | `1` | `for (count--; count != -1; count--) {` | Minus one terminates the count-minus-one vertex traversal after its final element. |
| `tmd_project_vertices_shift` | 505 | `1` | `for (count--; count != -1; count--) {` | Minus one terminates the count-minus-one vertex traversal after its final element. |
| `tmd_transform_vertices` | 526 | `1` | `for (remaining = count - 1; remaining != -1; remaining--) {` | Initialize the vertex countdown to its final zero-based ordinal. |
| `tmd_transform_vertices` | 526 | `1` | `for (remaining = count - 1; remaining != -1; remaining--) {` | Minus one terminates the vertex countdown. |
