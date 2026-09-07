# Retained menu panel, map and list literals

Complete current ledger of **103 numeric/character occurrences** across
`menu_status_panel.c`, `menu_map_viewer.c` and `menu_list_render.c` after the
[geometry review](game-menu-panel-geometry.md). This replaces the historical
190-occurrence snapshot, including the superseded list workspace and loops.
Comments, strings, identifier digits, named definitions and retail claims are
excluded. Every repeated token has its own row; signs are expression operators.

## `src/game/menu_map_viewer.c`

75 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `menu_map_viewer` | 31 | `0` | `s32 frame = 0;` | Zero-based panel draw counter starts before the first draw; the named release frame is tested after presentation. |
| `menu_map_viewer` | 34 | `16` | `char path[16] = "MAP\\M00.";` | Sixteen-byte local path workspace initialized with the eight-character map template and trailing zeros; retain its retail stack footprint. |
| `menu_map_viewer` | 41 | `5` | `path[5] = map_number + '0';` | Byte five is the map-set digit in MAP/Msf.; adding ASCII zero encodes the numeric selector as a path character. |
| `menu_map_viewer` | 41 | `'0'` | `path[5] = map_number + '0';` | Byte five is the map-set digit in MAP/Msf.; adding ASCII zero encodes the numeric selector as a path character. |
| `menu_map_viewer` | 42 | `6` | `path[6] = player_state.progress_state.current_floor + '0';` | Byte six is the current-floor digit in MAP/Msf.; adding ASCII zero encodes the numeric floor as a path character. |
| `menu_map_viewer` | 42 | `'0'` | `path[6] = player_state.progress_state.current_floor + '0';` | Byte six is the current-floor digit in MAP/Msf.; adding ASCII zero encodes the numeric floor as a path character. |
| `menu_map_viewer` | 45 | `0` | `if (cd_file_load_into(buffer, path) != 0)` | Nonzero file-loader status returns before the TIM upload and rendering. |
| `menu_map_viewer` | 49 | `0` | `SetPolyFT4(&poly_bg[0]);` | Initialize packet element zero before copying it to element one for double buffering. |
| `menu_map_viewer` | 50 | `0` | `SetSemiTrans(&poly_bg[0], 1);` | SDK Boolean 1 enables semi-transparency. Packet index 0 selects the initialized copy before double-buffer duplication. |
| `menu_map_viewer` | 50 | `1` | `SetSemiTrans(&poly_bg[0], 1);` | SDK Boolean 1 enables semi-transparency. Packet index 0 selects the initialized copy before double-buffer duplication. |
| `menu_map_viewer` | 51 | `0` | `poly_bg[0].r0 = MENU_MAP_BRIGHTNESS;` | Packet index zero is the copy initialized before double-buffer duplication. RGB modulation 0x40 is half the SDK neutral-texture value 0x80 on each channel; it is not an opacity byte. |
| `menu_map_viewer` | 52 | `0` | `poly_bg[0].g0 = MENU_MAP_BRIGHTNESS;` | Packet index zero is the copy initialized before double-buffer duplication. RGB modulation 0x40 is half the SDK neutral-texture value 0x80 on each channel; it is not an opacity byte. |
| `menu_map_viewer` | 53 | `0` | `poly_bg[0].b0 = MENU_MAP_BRIGHTNESS;` | Packet index zero is the copy initialized before double-buffer duplication. RGB modulation 0x40 is half the SDK neutral-texture value 0x80 on each channel; it is not an opacity byte. |
| `menu_map_viewer` | 54 | `0` | `poly_bg[0].clut = MENU_MAP_IMAGE_CLUT;` | Packet index zero is the copy initialized before double-buffer duplication. Packed CLUT 0x7d40 selects VRAM (0,501), the first palette row in every shipped map TIM. |
| `menu_map_viewer` | 55 | `0` | `poly_bg[0].tpage = MENU_MAP_IMAGE_TPAGE;` | Packet index zero is the copy initialized before double-buffer duplication. Packed texture page 0x1f selects 4-bit texels at VRAM (960,256), average semi-transparency mode, matching each map TIM. |
| `menu_map_viewer` | 56 | `0` | `poly_bg[0].u0 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| `menu_map_viewer` | 56 | `0` | `poly_bg[0].u0 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| `menu_map_viewer` | 57 | `0` | `poly_bg[0].v0 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| `menu_map_viewer` | 57 | `0` | `poly_bg[0].v0 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| `menu_map_viewer` | 58 | `0` | `poly_bg[0].u1 = MENU_MAP_IMAGE_SPAN;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| `menu_map_viewer` | 59 | `0` | `poly_bg[0].v1 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| `menu_map_viewer` | 59 | `0` | `poly_bg[0].v1 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| `menu_map_viewer` | 60 | `0` | `poly_bg[0].u2 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| `menu_map_viewer` | 60 | `0` | `poly_bg[0].u2 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| `menu_map_viewer` | 61 | `0` | `poly_bg[0].v2 = MENU_MAP_IMAGE_SPAN;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| `menu_map_viewer` | 62 | `0` | `poly_bg[0].u3 = MENU_MAP_IMAGE_SPAN;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| `menu_map_viewer` | 63 | `0` | `poly_bg[0].v3 = MENU_MAP_IMAGE_SPAN;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| `menu_map_viewer` | 64 | `0` | `poly_bg[0].x0 = MENU_MAP_IMAGE_LEFT_X;` | Packet index zero is the copy initialized before double-buffer duplication. Map-quad screen endpoints X=50/269 and Y=10/229 give 219-pixel coordinate spans. Preserve the authored placement, separately from the full texture extent. |
| `menu_map_viewer` | 65 | `0` | `poly_bg[0].y0 = MENU_MAP_IMAGE_TOP_Y;` | Packet index zero is the copy initialized before double-buffer duplication. Map-quad screen endpoints X=50/269 and Y=10/229 give 219-pixel coordinate spans. Preserve the authored placement, separately from the full texture extent. |
| `menu_map_viewer` | 66 | `0` | `poly_bg[0].x1 = MENU_MAP_IMAGE_LEFT_X + MENU_MAP_IMAGE_SPAN;` | Packet index zero is the copy initialized before double-buffer duplication. Map-quad screen endpoints X=50/269 and Y=10/229 give 219-pixel coordinate spans. Preserve the authored placement, separately from the full texture extent. |
| `menu_map_viewer` | 67 | `0` | `poly_bg[0].y1 = MENU_MAP_IMAGE_TOP_Y;` | Packet index zero is the copy initialized before double-buffer duplication. Map-quad screen endpoints X=50/269 and Y=10/229 give 219-pixel coordinate spans. Preserve the authored placement, separately from the full texture extent. |
| `menu_map_viewer` | 68 | `0` | `poly_bg[0].x2 = MENU_MAP_IMAGE_LEFT_X;` | Packet index zero is the copy initialized before double-buffer duplication. Map-quad screen endpoints X=50/269 and Y=10/229 give 219-pixel coordinate spans. Preserve the authored placement, separately from the full texture extent. |
| `menu_map_viewer` | 69 | `0` | `poly_bg[0].y2 = MENU_MAP_IMAGE_TOP_Y + MENU_MAP_IMAGE_SPAN;` | Packet index zero is the copy initialized before double-buffer duplication. Map-quad screen endpoints X=50/269 and Y=10/229 give 219-pixel coordinate spans. Preserve the authored placement, separately from the full texture extent. |
| `menu_map_viewer` | 70 | `0` | `poly_bg[0].x3 = MENU_MAP_IMAGE_LEFT_X + MENU_MAP_IMAGE_SPAN;` | Packet index zero is the copy initialized before double-buffer duplication. Map-quad screen endpoints X=50/269 and Y=10/229 give 219-pixel coordinate spans. Preserve the authored placement, separately from the full texture extent. |
| `menu_map_viewer` | 71 | `0` | `poly_bg[0].y3 = MENU_MAP_IMAGE_TOP_Y + MENU_MAP_IMAGE_SPAN;` | Packet index zero is the copy initialized before double-buffer duplication. Map-quad screen endpoints X=50/269 and Y=10/229 give 219-pixel coordinate spans. Preserve the authored placement, separately from the full texture extent. |
| `menu_map_viewer` | 72 | `1` | `poly_bg[1] = poly_bg[0];` | Copy the complete first initialized GPU packet to the second display-buffer packet; these are array positions, not layer IDs. |
| `menu_map_viewer` | 72 | `0` | `poly_bg[1] = poly_bg[0];` | Copy the complete first initialized GPU packet to the second display-buffer packet; these are array positions, not layer IDs. |
| `menu_map_viewer` | 74 | `0` | `SetPolyFT4(&poly_marker[0]);` | Initialize packet element zero before copying it to element one for double buffering. |
| `menu_map_viewer` | 75 | `0` | `poly_marker[0].r0 = MENU_MAP_BRIGHTNESS;` | Packet index zero is the copy initialized before double-buffer duplication. RGB modulation 0x40 is half the SDK neutral-texture value 0x80 on each channel; it is not an opacity byte. |
| `menu_map_viewer` | 76 | `0` | `poly_marker[0].g0 = MENU_MAP_BRIGHTNESS;` | Packet index zero is the copy initialized before double-buffer duplication. RGB modulation 0x40 is half the SDK neutral-texture value 0x80 on each channel; it is not an opacity byte. |
| `menu_map_viewer` | 77 | `0` | `poly_marker[0].b0 = MENU_MAP_BRIGHTNESS;` | Packet index zero is the copy initialized before double-buffer duplication. RGB modulation 0x40 is half the SDK neutral-texture value 0x80 on each channel; it is not an opacity byte. |
| `menu_map_viewer` | 78 | `0` | `poly_marker[0].clut = MENU_MAP_MARKER_CLUT;` | Packet index zero is the copy initialized before double-buffer duplication. Packed CLUT 0x7c40 selects VRAM (0,497), the marker page palette in COM/MIX.TIM image 3. |
| `menu_map_viewer` | 79 | `0` | `poly_marker[0].tpage = MENU_MAP_MARKER_TPAGE;` | Packet index zero is the copy initialized before double-buffer duplication. Packed texture page 0x1b selects 4-bit texels at VRAM (704,256), matching COM/MIX.TIM image 3; the marker packet retains its opaque default. |
| `menu_map_viewer` | 80 | `0` | `poly_marker[0].u0 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| `menu_map_viewer` | 80 | `0` | `poly_marker[0].u0 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| `menu_map_viewer` | 81 | `0` | `poly_marker[0].v0 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| `menu_map_viewer` | 81 | `0` | `poly_marker[0].v0 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| `menu_map_viewer` | 82 | `0` | `poly_marker[0].u1 = MENU_MAP_MARKER_SPAN;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| `menu_map_viewer` | 83 | `0` | `poly_marker[0].v1 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| `menu_map_viewer` | 83 | `0` | `poly_marker[0].v1 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| `menu_map_viewer` | 84 | `0` | `poly_marker[0].u2 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| `menu_map_viewer` | 84 | `0` | `poly_marker[0].u2 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| `menu_map_viewer` | 85 | `0` | `poly_marker[0].v2 = MENU_MAP_MARKER_SPAN;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| `menu_map_viewer` | 86 | `0` | `poly_marker[0].u3 = MENU_MAP_MARKER_SPAN;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| `menu_map_viewer` | 87 | `0` | `poly_marker[0].v3 = MENU_MAP_MARKER_SPAN;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| `menu_map_viewer` | 88 | `0` | `poly_marker[0].x0 = player_state.map_cell.x * MENU_MAP_PIXELS_PER_CELL + MENU_MAP_MARKER_ORIGIN_X;` | Packet index zero is the copy initialized before double-buffer duplication. Marker endpoint origins X=58/62 and Y=216/220 surround the cell center by two pixels; named cell scale controls translation and Z reverses screen Y. |
| `menu_map_viewer` | 89 | `0` | `poly_marker[0].y0 = MENU_MAP_MARKER_ORIGIN_Y - player_state.map_cell.z * MENU_MAP_PIXELS_PER_CELL;` | Packet index zero is the copy initialized before double-buffer duplication. Marker endpoint origins X=58/62 and Y=216/220 surround the cell center by two pixels; named cell scale controls translation and Z reverses screen Y. |
| `menu_map_viewer` | 90 | `0` | `poly_marker[0].x1 = player_state.map_cell.x * MENU_MAP_PIXELS_PER_CELL + (MENU_MAP_MARKER_ORIGIN_X + MENU_MAP_MARKER_SPAN);` | Packet index zero is the copy initialized before double-buffer duplication. Marker endpoint origins X=58/62 and Y=216/220 surround the cell center by two pixels; named cell scale controls translation and Z reverses screen Y. |
| `menu_map_viewer` | 91 | `0` | `poly_marker[0].y1 = MENU_MAP_MARKER_ORIGIN_Y - player_state.map_cell.z * MENU_MAP_PIXELS_PER_CELL;` | Packet index zero is the copy initialized before double-buffer duplication. Marker endpoint origins X=58/62 and Y=216/220 surround the cell center by two pixels; named cell scale controls translation and Z reverses screen Y. |
| `menu_map_viewer` | 92 | `0` | `poly_marker[0].x2 = player_state.map_cell.x * MENU_MAP_PIXELS_PER_CELL + MENU_MAP_MARKER_ORIGIN_X;` | Packet index zero is the copy initialized before double-buffer duplication. Marker endpoint origins X=58/62 and Y=216/220 surround the cell center by two pixels; named cell scale controls translation and Z reverses screen Y. |
| `menu_map_viewer` | 93 | `0` | `poly_marker[0].y2 = (MENU_MAP_MARKER_ORIGIN_Y + MENU_MAP_MARKER_SPAN) - player_state.map_cell.z * MENU_MAP_PIXELS_PER_CELL;` | Packet index zero is the copy initialized before double-buffer duplication. Marker endpoint origins X=58/62 and Y=216/220 surround the cell center by two pixels; named cell scale controls translation and Z reverses screen Y. |
| `menu_map_viewer` | 94 | `0` | `poly_marker[0].x3 = player_state.map_cell.x * MENU_MAP_PIXELS_PER_CELL + (MENU_MAP_MARKER_ORIGIN_X + MENU_MAP_MARKER_SPAN);` | Packet index zero is the copy initialized before double-buffer duplication. Marker endpoint origins X=58/62 and Y=216/220 surround the cell center by two pixels; named cell scale controls translation and Z reverses screen Y. |
| `menu_map_viewer` | 95 | `0` | `poly_marker[0].y3 = (MENU_MAP_MARKER_ORIGIN_Y + MENU_MAP_MARKER_SPAN) - player_state.map_cell.z * MENU_MAP_PIXELS_PER_CELL;` | Packet index zero is the copy initialized before double-buffer duplication. Marker endpoint origins X=58/62 and Y=216/220 surround the cell center by two pixels; named cell scale controls translation and Z reverses screen Y. |
| `menu_map_viewer` | 96 | `1` | `poly_marker[1] = poly_marker[0];` | Copy the complete first initialized GPU packet to the second display-buffer packet; these are array positions, not layer IDs. |
| `menu_map_viewer` | 96 | `0` | `poly_marker[1] = poly_marker[0];` | Copy the complete first initialized GPU packet to the second display-buffer packet; these are array positions, not layer IDs. |
| `menu_map_viewer` | 105 | `3` | `&menu_assets.background_quads[display_state.buffer_index][3]);` | Persistent background packet indices are submitted 3,2,1,0. Preserve their explicit order because AddPrim prepends within the shared bucket. |
| `menu_map_viewer` | 107 | `2` | `&menu_assets.background_quads[display_state.buffer_index][2]);` | Persistent background packet indices are submitted 3,2,1,0. Preserve their explicit order because AddPrim prepends within the shared bucket. |
| `menu_map_viewer` | 109 | `1` | `&menu_assets.background_quads[display_state.buffer_index][1]);` | Persistent background packet indices are submitted 3,2,1,0. Preserve their explicit order because AddPrim prepends within the shared bucket. |
| `menu_map_viewer` | 111 | `0` | `&menu_assets.background_quads[display_state.buffer_index][0]);` | Persistent background packet indices are submitted 3,2,1,0. Preserve their explicit order because AddPrim prepends within the shared bucket. |
| `menu_map_viewer` | 116 | `1` | `while (PadRead(1) != 0)` | Retain SDK PadRead selector 1; zero means no reported button bits. Release waits test nonzero, while dismissal tests a new nonzero sample. |
| `menu_map_viewer` | 116 | `0` | `while (PadRead(1) != 0)` | Retain SDK PadRead selector 1; zero means no reported button bits. Release waits test nonzero, while dismissal tests a new nonzero sample. |
| `menu_map_viewer` | 120 | `1` | `if (PadRead(1) == 0)` | Retain SDK PadRead selector 1; zero means no reported button bits. Release waits test nonzero, while dismissal tests a new nonzero sample. |
| `menu_map_viewer` | 120 | `0` | `if (PadRead(1) == 0)` | Retain SDK PadRead selector 1; zero means no reported button bits. Release waits test nonzero, while dismissal tests a new nonzero sample. |
| `menu_map_viewer` | 122 | `1` | `while (PadRead(1) != 0)` | Retain SDK PadRead selector 1; zero means no reported button bits. Release waits test nonzero, while dismissal tests a new nonzero sample. |
| `menu_map_viewer` | 122 | `0` | `while (PadRead(1) != 0)` | Retain SDK PadRead selector 1; zero means no reported button bits. Release waits test nonzero, while dismissal tests a new nonzero sample. |

## `src/game/menu_status_panel.c`

12 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `menu_status_panel` | 24 | `0` | `frame = 0;` | Zero-based panel draw counter starts before the first draw; the named release frame is tested after presentation. |
| `menu_status_panel` | 25 | `1` | `while (1) {` | Unconditional panel loop; the explicit input-controlled return exits it. |
| `menu_status_panel` | 30 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean 1 enables semi-transparency. |
| `menu_status_panel` | 52 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean 1 enables semi-transparency. |
| `menu_status_panel` | 74 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean 1 enables semi-transparency. |
| `menu_status_panel` | 96 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean 1 enables semi-transparency. |
| `menu_status_panel` | 123 | `1` | `while (PadRead(1) != 0) {` | Retain SDK PadRead selector 1; zero means no reported button bits. Release waits test nonzero, while dismissal tests a new nonzero sample. |
| `menu_status_panel` | 123 | `0` | `while (PadRead(1) != 0) {` | Retain SDK PadRead selector 1; zero means no reported button bits. Release waits test nonzero, while dismissal tests a new nonzero sample. |
| `menu_status_panel` | 128 | `1` | `input = PadRead(1);` | Retain SDK PadRead selector 1; zero means no reported button bits. Release waits test nonzero, while dismissal tests a new nonzero sample. |
| `menu_status_panel` | 129 | `0` | `if (input != 0) {` | Any reported button bit dismisses the status panel after its release gate. |
| `menu_status_panel` | 131 | `1` | `while (PadRead(1) != 0) {` | Retain SDK PadRead selector 1; zero means no reported button bits. Release waits test nonzero, while dismissal tests a new nonzero sample. |
| `menu_status_panel` | 131 | `0` | `while (PadRead(1) != 0) {` | Retain SDK PadRead selector 1; zero means no reported button bits. Release waits test nonzero, while dismissal tests a new nonzero sample. |

## `src/game/menu_list_render.c`

16 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `menu_list_render` | 36 | `0` | `if (list->title_x != 0) {` | A nonzero title X coordinate enables the positioned title; retain this coordinate-based guard. |
| `menu_list_render` | 45 | `0` | `row = 0;` | Start at the first zero-based row; text traversal and highlight traversal initialize this index independently. |
| `menu_list_render` | 51 | `0` | `for (i = 0; i < list->glyphs_per_entry; i++) {` | Copy from the first glyph position up to the caller-supplied row width, advancing the source after each halfword; no extra terminator is appended. |
| `menu_list_render` | 56 | `0` | `if (list->quantities != 0) {` | A nonnull quantity pointer enables the optional number column. |
| `menu_list_render` | 57 | `10u` | `tens = *counts / 10u;` | Unsigned decimal radix extracts the tens digit from the quantity byte; paired division and remainder retain one retail DIVU. |
| `menu_list_render` | 58 | `10u` | `ones = *counts % 10u;` | Unsigned decimal radix extracts the units digit from the quantity byte; retain the unsigned operation paired with the quotient before glyph stores. |
| `menu_list_render` | 61 | `0` | `gs.codes[0] = tens;` | Quantity glyph position zero holds the tens digit or a leading blank; ordinary first-cell index. |
| `menu_list_render` | 62 | `0` | `if (tens == 0) {` | A zero tens digit is suppressed with the named blank rather than shown as a leading zero. |
| `menu_list_render` | 63 | `0` | `gs.codes[0] = MENU_NUMBER_BLANK;` | Quantity glyph position zero holds the tens digit or a leading blank; ordinary first-cell index. |
| `menu_list_render` | 65 | `1` | `gs.codes[1] = ones;` | The second zero-based glyph position holds the captured units digit. |
| `menu_list_render` | 66 | `2` | `gs.codes[2] = MENU_TEXT_END;` | Terminate immediately after the two quantity digits at zero-based position two. |
| `menu_list_render` | 94 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean 1 enables semi-transparency. |
| `menu_list_render` | 97 | `0` | `row = 0;` | Start at the first zero-based row; text traversal and highlight traversal initialize this index independently. |
| `menu_list_render` | 99 | `0` | `yoff = 0;` | The first row has zero additional pixel displacement from the named inset. |
| `menu_list_render` | 125 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean 1 enables semi-transparency. |
| `menu_list_render` | 153 | `1` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean 1 enables semi-transparency. |
