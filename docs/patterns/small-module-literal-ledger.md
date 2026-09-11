# Small-module retained literals

Complete accounting for the nine sources below: **59 retained occurrences**.
See the [menu optional argument](menu-optional-argument.md),
[floor-item random scale](floor-item-random-frame.md),
[opening input domain](open-input-action-domain.md), and
[OPEN TMD countdown](open-tmd-countdown.md) evidence. Claims, comments, strings
and named constant definitions are excluded. Both modern and retail conditional
source branches are included in the census; only the applicable branch compiles.

Vendored PAD/interrupt modules are included solely to cover all source literals.
Their protocol values and K&R fallback shapes are preserved as SDK evidence,
not renamed into game modes or counted as game reconstruction. See
[PAD storage and linkage](../../config/evidence/pad_storage_and_linkage.md).
The zero-literal fire-defense wrapper already uses named timer and status bits.
Zero-literal modules have no rows; that does not resolve their external identities.

## `src/game/menu_enter_mode.c`

2 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `menu_enter_mode` | 24 | `0` | `DrawSync(0);` | SDK blocking GPU synchronization selector; drain pending work before changing frame or heap state. |
| `menu_enter_mode` | 58 | `0` | `result = 0;` | The void shop path returns a zero word; its caller ignores the value, and it is not a pickup-result enum. |

## `src/game/player_status_effect4.c`

0 retained occurrences.


## `src/open/item.c`

1 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `item_load_floor_placements` | 13 | `0` | `open_graphics_runtime.floor_item_state.count = 0;` | Begin counting the current placement stream from zero before expansion. |

## `src/open/opening_helpers.c`

4 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `opening_poll_input` | 12 | `1` | `u32 input = PadRead(1);` | The linked Sony routine ignores this call-site argument and uses global PadIdentifier; retain the literal without inventing a controller-port meaning. |
| `opening_poll_input` | 14 | `0` | `if (input != 0) {` | No returned button bits leave the previous opening action latched; only nonzero input updates it. |
| `opening_poll_input` | 15 | `0` | `if ((input & PADk) != 0) {` | Test whether the named skip-button bit is present; zero is the empty bit intersection. |
| `angle_shortest_delta` | 36 | `1` | `if (signed_difference < -KF_ANGLE_HALF_TURN + 1) {` | The unit adjustment includes exactly negative half-turn in this strict-less-than wrap branch, preserving the tie behavior. |

## `src/open/display_adjust.c`

19 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `display_adjust_vram_view` | 16 | `0` | `open_graphics_runtime.display_draw_environments[0].isbg = 0;` | Select display-environment record 0 in the explicit two-buffer pair. |
| `display_adjust_vram_view` | 16 | `0` | `open_graphics_runtime.display_draw_environments[0].isbg = 0;` | Boolean false disables the SDK automatic background clear for VRAM inspection. |
| `display_adjust_vram_view` | 17 | `1` | `open_graphics_runtime.display_draw_environments[1].isbg = 0;` | Select display-environment record 1 in the explicit two-buffer pair. |
| `display_adjust_vram_view` | 17 | `0` | `open_graphics_runtime.display_draw_environments[1].isbg = 0;` | Boolean false disables the SDK automatic background clear for VRAM inspection. |
| `display_adjust_vram_view` | 18 | `0` | `open_graphics_runtime.display_state.buffer_index = open_graphics_runtime.display_state.buffer_index == 0;` | Boolean inversion toggles the active index between the two display buffers. |
| `display_adjust_vram_view` | 19 | `0` | `DrawSync(0);` | SDK blocking GPU synchronization selector; drain pending work before changing frame or heap state. |
| `display_adjust_vram_view` | 20 | `0` | `VSync(0);` | SDK next-vertical-blank wait selector used for frame pacing. |
| `display_adjust_vram_view` | 22 | `0` | `open_graphics_runtime.display_draw_environments[open_graphics_runtime.display_state.buffer_index].dfe = 0;` | Boolean false disables the SDK draw-to-display-area flag for the active draw environment. |
| `display_adjust_vram_view` | 27 | `1` | `while (PadRead(1) & PADh) {` | The linked Sony routine ignores this call-site argument and uses global PadIdentifier; retain the literal without inventing a controller-port meaning. |
| `display_adjust_vram_view` | 30 | `1` | `buttons = PadRead(1);` | The linked Sony routine ignores this call-site argument and uses global PadIdentifier; retain the literal without inventing a controller-port meaning. |
| `display_adjust_vram_view` | 46 | `1` | `display->disp.x &= KF_VRAM_WIDTH - 1;` | Subtract one from the power-of-two VRAM width to form the X wrapping mask. |
| `display_adjust_vram_view` | 47 | `1` | `display->disp.y &= KF_VRAM_HEIGHT - 1;` | Subtract one from the power-of-two VRAM height to form the Y wrapping mask. |
| `display_adjust_vram_view` | 48 | `0` | `VSync(0);` | SDK next-vertical-blank wait selector used for frame pacing. |
| `display_adjust_vram_view` | 51 | `1` | `while (PadRead(1) & PADh) {` | The linked Sony routine ignores this call-site argument and uses global PadIdentifier; retain the literal without inventing a controller-port meaning. |
| `display_adjust_vram_view` | 54 | `1` | `open_graphics_runtime.display_draw_environments[open_graphics_runtime.display_state.buffer_index].dfe = 1;` | Boolean true restores the SDK draw-to-display-area flag for the active draw environment. |
| `display_adjust_vram_view` | 55 | `0` | `open_graphics_runtime.display_draw_environments[0].isbg = 1;` | Select display-environment record 0 in the explicit two-buffer pair. |
| `display_adjust_vram_view` | 55 | `1` | `open_graphics_runtime.display_draw_environments[0].isbg = 1;` | Boolean true restores the SDK automatic background clear for VRAM inspection. |
| `display_adjust_vram_view` | 56 | `1` | `open_graphics_runtime.display_draw_environments[1].isbg = 1;` | Select display-environment record 1 in the explicit two-buffer pair. |
| `display_adjust_vram_view` | 56 | `1` | `open_graphics_runtime.display_draw_environments[1].isbg = 1;` | Boolean true restores the SDK automatic background clear for VRAM inspection. |

## `src/open/render.c`

19 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `display_begin_frame` | 22 | `0` | `open_graphics_runtime.display_state.buffer_index = open_graphics_runtime.display_state.buffer_index == 0;` | Boolean inversion toggles the active index between the two display buffers. |
| `display_begin_frame` | 27 | `0` | `primitive_allocation_count = 0;` | Reset the per-frame primitive-allocation count before new allocations. |
| `display_begin_frame` | 28 | `0` | `open_graphics_runtime.DAT_8006e044 = 0;` | Observed per-frame zero reset of this unresolved graphics word; no unsupported operational role is inferred from the reset. |
| `display_begin_frame` | 29 | `0` | `open_graphics_runtime.DAT_8006e040 = 0;` | Observed per-frame zero reset of this unresolved graphics word; no unsupported operational role is inferred from the reset. |
| `display_present_frame` | 35 | `0` | `DrawSync(0);` | SDK blocking GPU synchronization selector; drain pending work before changing frame or heap state. |
| `display_present_frame` | 36 | `0` | `VSync(0);` | SDK next-vertical-blank wait selector used for frame pacing. |
| `display_present_frame` | 39 | `1` | `DrawOTag(open_graphics_runtime.ordering_table + (KF_ORDERING_TABLE_LENGTH - 1));` | Start traversal at the last word of the reversed ordering table; one converts the length to its last index. |
| `render_set_view_transform` | 74 | `0` | `if (position != 0) {` | Null optional transform input retains the corresponding current view value. |
| `render_set_view_transform` | 79 | `0` | `if (rotation != 0) {` | Null optional transform input retains the corresponding current view value. |
| `render_set_view_transform` | 83 | `0` | `angles.vz = 0;` | The pitch-only matrix has no rotation on this other axis. |
| `render_set_view_transform` | 84 | `0` | `angles.vy = 0;` | The pitch-only matrix has no rotation on this other axis. |
| `tmd_prepare_primitive_indices` | 108 | `1` | `objects_left = object_count - 1;` | Initialize the halfword post-decrement countdown to count minus one; the separate zero-count guard handles empty assets. |
| `tmd_prepare_primitive_indices` | 110 | `0` | `if (object_count == 0) {` | Empty TMD object list exits before the object-body loop. |
| `tmd_prepare_primitive_indices` | 119 | `0` | `if (primitive_count != 0) {` | Empty primitive list skips its packet-body loop. |
| `tmd_prepare_primitive_indices` | 204 | `0` | `} while (primitives_left-- != 0);` | Zero is the terminal value of the halfword post-decrement packet countdown. |
| `tmd_project_vertices` | 239 | `1` | `for (count--; count != -1; count--) {` | Minus one terminates the count-minus-one vertex loop after its final element. |
| `tmd_project_vertices_perspective_right` | 261 | `1` | `for (count--; count != -1; count--) {` | Minus one terminates the count-minus-one vertex loop after its final element. |
| `tmd_project_vertices_shift` | 283 | `1` | `for (count--; count != -1; count--) {` | Minus one terminates the count-minus-one vertex loop after its final element. |
| `tmd_transform_vertices` | 303 | `1` | `for (count--; count != -1; count--) {` | Minus one terminates the count-minus-one vertex loop after its final element. |

## `vendor/src/game_libetc_pad.c`

7 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `PadInit` | 39 | `0` | `pad_status = 0;` | Zero reset retained from the attributed Sony PAD front end; no game-state meaning is assigned. |
| `PadInit` | 40 | `1` | `pad_buf = -1;` | All bits set initialize the active-low SDK pad word; complementing it yields no pressed buttons. |
| `PadInit` | 41 | `0` | `if (identifier == 0) {` | The attributed Sony front end accepts identifier zero for its legacy driver path; other values enter the diagnostic stub. |
| `PadInit` | 42 | `0x20000001` | `result = PAD_init2(0x20000001, &pad_buf);` | Fixed low-level setup word retained from the attributed SDK front end; its subfields are not reinterpreted as game selectors. |
| `PadInit` | 44 | `0x20000001` | `result = pad_init_bad_identifier(0x20000001, &pad_buf);` | Same SDK setup word is passed to the K&R diagnostic fallback, which ignores the arguments; preserve the provider calling shape. |
| `PadRead` | 53 | `0` | `if (PadIdentifier == 0) {` | The attributed Sony front end accepts identifier zero for its legacy driver path; other values enter the diagnostic stub. |
| `PadStop` | 64 | `0` | `if (PadIdentifier == 0) {` | The attributed Sony front end accepts identifier zero for its legacy driver path; other values enter the diagnostic stub. |

## `vendor/src/open_libetc_pad.c`

7 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `PadInit` | 34 | `0` | `pad_status = 0;` | Zero reset retained from the attributed Sony PAD front end; no game-state meaning is assigned. |
| `PadInit` | 35 | `1` | `pad_buf = -1;` | All bits set initialize the active-low SDK pad word; complementing it yields no pressed buttons. |
| `PadInit` | 36 | `0` | `if (identifier == 0) {` | The attributed Sony front end accepts identifier zero for its legacy driver path; other values enter the diagnostic stub. |
| `PadInit` | 37 | `0x20000001` | `result = PAD_init2(0x20000001, &pad_buf);` | Fixed low-level setup word retained from the attributed SDK front end; its subfields are not reinterpreted as game selectors. |
| `PadInit` | 39 | `0x20000001` | `result = pad_init_bad_identifier(0x20000001, &pad_buf);` | Same SDK setup word is passed to the K&R diagnostic fallback, which ignores the arguments; preserve the provider calling shape. |
| `PadRead` | 48 | `0` | `if (PadIdentifier == 0) {` | The attributed Sony front end accepts identifier zero for its legacy driver path; other values enter the diagnostic stub. |
| `PadStop` | 59 | `0` | `if (PadIdentifier == 0) {` | The attributed Sony front end accepts identifier zero for its legacy driver path; other values enter the diagnostic stub. |

## `vendor/src/game_libetc_intr_tail.c`

0 retained occurrences.
