# Menu panel, map and list literal ledger

Historical ledger from `5659505` for `menu_status_panel.c`, `menu_map_viewer.c`, and
`menu_list_render.c`, including its local glyph-workspace extent.
All 190 remaining numeric/character occurrences have a reason; named
constant definitions and retail claims are excluded. See the
[drawing-layer review](game-menu-drawing-layers.md) for the source and asset evidence.
The list's later [exact reconstruction](game-menu-list-control-flow.md)
supersedes its workspace assumption and loop/quantity expressions below;
the numerical geometry, decimal radix and drawing-layer contracts are unchanged.

## menu_status_panel

| Lines | Tokens | Expression | Reason |
| --- | --- | --- | --- |
| 19 | `0` | `frame = 0;` | Zero-based panel draw counter starts before the first draw; the named release frame is tested after presentation. |
| 20 | `1` | `while (1) {` | Unconditional panel loop; the explicit input-controlled return exits it. |
| 25, 47, 69, 91 | `1 × 4` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean 1 enables semi-transparency. |
| 28, 72 | `6 × 2` | `current_poly_ft4->x0 = 6;` | Authored X anchor 6 screen pixels for a mirrored status-window quadrant. The four anchors are X=6/77 and Y=16/120; descriptor width/height form the opposite edges. Preserve the layout and UV flips. |
| 29, 51 | `0x10 × 2` | `current_poly_ft4->y0 = 0x10;` | Authored Y anchor 16 screen pixels for a mirrored status-window quadrant. The four anchors are X=6/77 and Y=16/120; descriptor width/height form the opposite edges. Preserve the layout and UV flips. |
| 30, 74 | `6 × 2` | `current_poly_ft4->x1 = menu_assets.window_backdrop.width + 6;` | Authored X anchor 6 screen pixels for a mirrored status-window quadrant. The four anchors are X=6/77 and Y=16/120; descriptor width/height form the opposite edges. Preserve the layout and UV flips. |
| 31, 53 | `0x10 × 2` | `current_poly_ft4->y1 = 0x10;` | Authored Y anchor 16 screen pixels for a mirrored status-window quadrant. The four anchors are X=6/77 and Y=16/120; descriptor width/height form the opposite edges. Preserve the layout and UV flips. |
| 32, 76 | `6 × 2` | `current_poly_ft4->x2 = 6;` | Authored X anchor 6 screen pixels for a mirrored status-window quadrant. The four anchors are X=6/77 and Y=16/120; descriptor width/height form the opposite edges. Preserve the layout and UV flips. |
| 33, 55 | `0x10 × 2` | `current_poly_ft4->y2 = menu_assets.window_backdrop.height + 0x10;` | Authored Y anchor 16 screen pixels for a mirrored status-window quadrant. The four anchors are X=6/77 and Y=16/120; descriptor width/height form the opposite edges. Preserve the layout and UV flips. |
| 34, 78 | `6 × 2` | `current_poly_ft4->x3 = menu_assets.window_backdrop.width + 6;` | Authored X anchor 6 screen pixels for a mirrored status-window quadrant. The four anchors are X=6/77 and Y=16/120; descriptor width/height form the opposite edges. Preserve the layout and UV flips. |
| 35, 57 | `0x10 × 2` | `current_poly_ft4->y3 = menu_assets.window_backdrop.height + 0x10;` | Authored Y anchor 16 screen pixels for a mirrored status-window quadrant. The four anchors are X=6/77 and Y=16/120; descriptor width/height form the opposite edges. Preserve the layout and UV flips. |
| 50, 94 | `0x4d × 2` | `current_poly_ft4->x0 = 0x4d;` | Authored X anchor 77 screen pixels for a mirrored status-window quadrant. The four anchors are X=6/77 and Y=16/120; descriptor width/height form the opposite edges. Preserve the layout and UV flips. |
| 52, 96 | `0x4d × 2` | `current_poly_ft4->x1 = menu_assets.window_backdrop.width + 0x4d;` | Authored X anchor 77 screen pixels for a mirrored status-window quadrant. The four anchors are X=6/77 and Y=16/120; descriptor width/height form the opposite edges. Preserve the layout and UV flips. |
| 54, 98 | `0x4d × 2` | `current_poly_ft4->x2 = 0x4d;` | Authored X anchor 77 screen pixels for a mirrored status-window quadrant. The four anchors are X=6/77 and Y=16/120; descriptor width/height form the opposite edges. Preserve the layout and UV flips. |
| 56, 100 | `0x4d × 2` | `current_poly_ft4->x3 = menu_assets.window_backdrop.width + 0x4d;` | Authored X anchor 77 screen pixels for a mirrored status-window quadrant. The four anchors are X=6/77 and Y=16/120; descriptor width/height form the opposite edges. Preserve the layout and UV flips. |
| 73, 95 | `0x78 × 2` | `current_poly_ft4->y0 = 0x78;` | Authored Y anchor 120 screen pixels for a mirrored status-window quadrant. The four anchors are X=6/77 and Y=16/120; descriptor width/height form the opposite edges. Preserve the layout and UV flips. |
| 75, 97 | `0x78 × 2` | `current_poly_ft4->y1 = 0x78;` | Authored Y anchor 120 screen pixels for a mirrored status-window quadrant. The four anchors are X=6/77 and Y=16/120; descriptor width/height form the opposite edges. Preserve the layout and UV flips. |
| 77, 99 | `0x78 × 2` | `current_poly_ft4->y2 = menu_assets.window_backdrop.height + 0x78;` | Authored Y anchor 120 screen pixels for a mirrored status-window quadrant. The four anchors are X=6/77 and Y=16/120; descriptor width/height form the opposite edges. Preserve the layout and UV flips. |
| 79, 101 | `0x78 × 2` | `current_poly_ft4->y3 = menu_assets.window_backdrop.height + 0x78;` | Authored Y anchor 120 screen pixels for a mirrored status-window quadrant. The four anchors are X=6/77 and Y=16/120; descriptor width/height form the opposite edges. Preserve the layout and UV flips. |
| 118, 126 | `1 × 2, 0 × 2` | `while (PadRead(1) != 0) {` | Retain SDK PadRead selector 1; zero means no reported button bits. Release waits test nonzero, while dismissal tests a new nonzero sample. |
| 123 | `1` | `input = PadRead(1);` | Retain SDK PadRead selector 1; zero means no reported button bits. Release waits test nonzero, while dismissal tests a new nonzero sample. |
| 124 | `0` | `if (input != 0) {` | Any reported button bit dismisses the status panel after its release gate. |

## menu_map_viewer

| Lines | Tokens | Expression | Reason |
| --- | --- | --- | --- |
| 19 | `0` | `s32 frame = 0;` | Zero-based panel draw counter starts before the first draw; the named release frame is tested after presentation. |
| 22 | `16` | `char path[16] = "MAP\\M00.";` | Sixteen-byte local path workspace initialized with the eight-character map template and trailing zeros; retain its retail stack footprint. |
| 26 | `2` | `map_number = 2;` | Filename map-set digit: Watchman Map selects set 1; every other argument selects set 2. Both shipped sets contain floor files 1..5; retain the original default for all other IDs. |
| 28 | `1` | `map_number = 1;` | Filename map-set digit: Watchman Map selects set 1; every other argument selects set 2. Both shipped sets contain floor files 1..5; retain the original default for all other IDs. |
| 29 | `5, '0'` | `path[5] = map_number + '0';` | Byte five is the map-set digit in MAP/Msf.; adding ASCII zero encodes the numeric selector as a path character. |
| 30 | `6, '0'` | `path[6] = player_state.progress_state.current_floor + '0';` | Byte six is the current-floor digit in MAP/Msf.; adding ASCII zero encodes the numeric floor as a path character. |
| 33 | `0` | `if (cd_file_load_into(buffer, path) != 0)` | Nonzero file-loader status returns before the TIM upload and rendering. |
| 37 | `0` | `SetPolyFT4(&poly_bg[0]);` | Initialize packet element zero before copying it to element one for double buffering. |
| 38 | `0, 1` | `SetSemiTrans(&poly_bg[0], 1);` | SDK Boolean 1 enables semi-transparency. Packet index 0 selects the initialized copy before double-buffer duplication. |
| 39 | `0, 0x40` | `poly_bg[0].r0 = 0x40;` | Packet index zero is the copy initialized before double-buffer duplication. RGB modulation 0x40 is half the SDK neutral-texture value 0x80 on each channel; it is not an opacity byte. |
| 40 | `0, 0x40` | `poly_bg[0].g0 = 0x40;` | Packet index zero is the copy initialized before double-buffer duplication. RGB modulation 0x40 is half the SDK neutral-texture value 0x80 on each channel; it is not an opacity byte. |
| 41 | `0, 0x40` | `poly_bg[0].b0 = 0x40;` | Packet index zero is the copy initialized before double-buffer duplication. RGB modulation 0x40 is half the SDK neutral-texture value 0x80 on each channel; it is not an opacity byte. |
| 42 | `0, 0x7d40` | `poly_bg[0].clut = 0x7d40;` | Packet index zero is the copy initialized before double-buffer duplication. Packed CLUT 0x7d40 selects VRAM (0,501), the first palette row in every shipped map TIM. |
| 43 | `0, 0x1f` | `poly_bg[0].tpage = 0x1f;` | Packet index zero is the copy initialized before double-buffer duplication. Packed texture page 0x1f selects 4-bit texels at VRAM (960,256), average semi-transparency mode, matching each map TIM. |
| 44 | `0 × 2` | `poly_bg[0].u0 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| 45 | `0 × 2` | `poly_bg[0].v0 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| 46 | `0, 0xdb` | `poly_bg[0].u1 = 0xdb;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| 47 | `0 × 2` | `poly_bg[0].v1 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| 48 | `0 × 2` | `poly_bg[0].u2 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| 49 | `0, 0xdb` | `poly_bg[0].v2 = 0xdb;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| 50 | `0, 0xdb` | `poly_bg[0].u3 = 0xdb;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| 51 | `0, 0xdb` | `poly_bg[0].v3 = 0xdb;` | Packet index zero is the copy initialized before double-buffer duplication. UV endpoints 0 and 219 crop a 219-unit span from the 256-by-256 map texture; keep the authored crop. |
| 52 | `0, 0x32` | `poly_bg[0].x0 = 0x32;` | Packet index zero is the copy initialized before double-buffer duplication. Map-quad screen endpoints X=50/269 and Y=10/229 give 219-pixel coordinate spans. Preserve the authored placement, separately from the full texture extent. |
| 53 | `0, 0xa` | `poly_bg[0].y0 = 0xa;` | Packet index zero is the copy initialized before double-buffer duplication. Map-quad screen endpoints X=50/269 and Y=10/229 give 219-pixel coordinate spans. Preserve the authored placement, separately from the full texture extent. |
| 54 | `0, 0x10d` | `poly_bg[0].x1 = 0x10d;` | Packet index zero is the copy initialized before double-buffer duplication. Map-quad screen endpoints X=50/269 and Y=10/229 give 219-pixel coordinate spans. Preserve the authored placement, separately from the full texture extent. |
| 55 | `0, 0xa` | `poly_bg[0].y1 = 0xa;` | Packet index zero is the copy initialized before double-buffer duplication. Map-quad screen endpoints X=50/269 and Y=10/229 give 219-pixel coordinate spans. Preserve the authored placement, separately from the full texture extent. |
| 56 | `0, 0x32` | `poly_bg[0].x2 = 0x32;` | Packet index zero is the copy initialized before double-buffer duplication. Map-quad screen endpoints X=50/269 and Y=10/229 give 219-pixel coordinate spans. Preserve the authored placement, separately from the full texture extent. |
| 57 | `0, 0xe5` | `poly_bg[0].y2 = 0xe5;` | Packet index zero is the copy initialized before double-buffer duplication. Map-quad screen endpoints X=50/269 and Y=10/229 give 219-pixel coordinate spans. Preserve the authored placement, separately from the full texture extent. |
| 58 | `0, 0x10d` | `poly_bg[0].x3 = 0x10d;` | Packet index zero is the copy initialized before double-buffer duplication. Map-quad screen endpoints X=50/269 and Y=10/229 give 219-pixel coordinate spans. Preserve the authored placement, separately from the full texture extent. |
| 59 | `0, 0xe5` | `poly_bg[0].y3 = 0xe5;` | Packet index zero is the copy initialized before double-buffer duplication. Map-quad screen endpoints X=50/269 and Y=10/229 give 219-pixel coordinate spans. Preserve the authored placement, separately from the full texture extent. |
| 60 | `1, 0` | `poly_bg[1] = poly_bg[0];` | Copy the complete first initialized GPU packet to the second display-buffer packet; these are array positions, not layer IDs. |
| 62 | `0` | `SetPolyFT4(&poly_marker[0]);` | Initialize packet element zero before copying it to element one for double buffering. |
| 63 | `0, 0x40` | `poly_marker[0].r0 = 0x40;` | Packet index zero is the copy initialized before double-buffer duplication. RGB modulation 0x40 is half the SDK neutral-texture value 0x80 on each channel; it is not an opacity byte. |
| 64 | `0, 0x40` | `poly_marker[0].g0 = 0x40;` | Packet index zero is the copy initialized before double-buffer duplication. RGB modulation 0x40 is half the SDK neutral-texture value 0x80 on each channel; it is not an opacity byte. |
| 65 | `0, 0x40` | `poly_marker[0].b0 = 0x40;` | Packet index zero is the copy initialized before double-buffer duplication. RGB modulation 0x40 is half the SDK neutral-texture value 0x80 on each channel; it is not an opacity byte. |
| 66 | `0, 0x7c40` | `poly_marker[0].clut = 0x7c40;` | Packet index zero is the copy initialized before double-buffer duplication. Packed CLUT 0x7c40 selects VRAM (0,497), the marker page palette in COM/MIX.TIM image 3. |
| 67 | `0, 0x1b` | `poly_marker[0].tpage = 0x1b;` | Packet index zero is the copy initialized before double-buffer duplication. Packed texture page 0x1b selects 4-bit texels at VRAM (704,256), matching COM/MIX.TIM image 3; the marker packet retains its opaque default. |
| 68 | `0 × 2` | `poly_marker[0].u0 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| 69 | `0 × 2` | `poly_marker[0].v0 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| 70 | `0` | `poly_marker[0].u1 = MENU_MAP_MARKER_SPAN;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| 71 | `0 × 2` | `poly_marker[0].v1 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| 72 | `0 × 2` | `poly_marker[0].u2 = 0;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| 73 | `0` | `poly_marker[0].v2 = MENU_MAP_MARKER_SPAN;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| 74 | `0` | `poly_marker[0].u3 = MENU_MAP_MARKER_SPAN;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| 75 | `0` | `poly_marker[0].v3 = MENU_MAP_MARKER_SPAN;` | Packet index zero is the copy initialized before double-buffer duplication. UV origin zero and the named marker span select the marker at the page origin; retain each explicit vertex assignment. |
| 76 | `0, 58` | `poly_marker[0].x0 = player_state.map_cell.x * MENU_MAP_PIXELS_PER_CELL + 58;` | Packet index zero is the copy initialized before double-buffer duplication. Marker endpoint origins X=58/62 and Y=216/220 surround the cell center by two pixels; named cell scale controls translation and Z reverses screen Y. |
| 77 | `0, 0xd8` | `poly_marker[0].y0 = 0xd8 - player_state.map_cell.z * MENU_MAP_PIXELS_PER_CELL;` | Packet index zero is the copy initialized before double-buffer duplication. Marker endpoint origins X=58/62 and Y=216/220 surround the cell center by two pixels; named cell scale controls translation and Z reverses screen Y. |
| 78 | `0, 62` | `poly_marker[0].x1 = player_state.map_cell.x * MENU_MAP_PIXELS_PER_CELL + 62;` | Packet index zero is the copy initialized before double-buffer duplication. Marker endpoint origins X=58/62 and Y=216/220 surround the cell center by two pixels; named cell scale controls translation and Z reverses screen Y. |
| 79 | `0, 0xd8` | `poly_marker[0].y1 = 0xd8 - player_state.map_cell.z * MENU_MAP_PIXELS_PER_CELL;` | Packet index zero is the copy initialized before double-buffer duplication. Marker endpoint origins X=58/62 and Y=216/220 surround the cell center by two pixels; named cell scale controls translation and Z reverses screen Y. |
| 80 | `0, 58` | `poly_marker[0].x2 = player_state.map_cell.x * MENU_MAP_PIXELS_PER_CELL + 58;` | Packet index zero is the copy initialized before double-buffer duplication. Marker endpoint origins X=58/62 and Y=216/220 surround the cell center by two pixels; named cell scale controls translation and Z reverses screen Y. |
| 81 | `0, 0xdc` | `poly_marker[0].y2 = 0xdc - player_state.map_cell.z * MENU_MAP_PIXELS_PER_CELL;` | Packet index zero is the copy initialized before double-buffer duplication. Marker endpoint origins X=58/62 and Y=216/220 surround the cell center by two pixels; named cell scale controls translation and Z reverses screen Y. |
| 82 | `0, 62` | `poly_marker[0].x3 = player_state.map_cell.x * MENU_MAP_PIXELS_PER_CELL + 62;` | Packet index zero is the copy initialized before double-buffer duplication. Marker endpoint origins X=58/62 and Y=216/220 surround the cell center by two pixels; named cell scale controls translation and Z reverses screen Y. |
| 83 | `0, 0xdc` | `poly_marker[0].y3 = 0xdc - player_state.map_cell.z * MENU_MAP_PIXELS_PER_CELL;` | Packet index zero is the copy initialized before double-buffer duplication. Marker endpoint origins X=58/62 and Y=216/220 surround the cell center by two pixels; named cell scale controls translation and Z reverses screen Y. |
| 84 | `1, 0` | `poly_marker[1] = poly_marker[0];` | Copy the complete first initialized GPU packet to the second display-buffer packet; these are array positions, not layer IDs. |
| 93 | `3` | `&menu_assets.background_quads[display_state.buffer_index][3]);` | Persistent background packet indices are submitted 3,2,1,0. Preserve their explicit order because AddPrim prepends within the shared bucket. |
| 95 | `2` | `&menu_assets.background_quads[display_state.buffer_index][2]);` | Persistent background packet indices are submitted 3,2,1,0. Preserve their explicit order because AddPrim prepends within the shared bucket. |
| 97 | `1` | `&menu_assets.background_quads[display_state.buffer_index][1]);` | Persistent background packet indices are submitted 3,2,1,0. Preserve their explicit order because AddPrim prepends within the shared bucket. |
| 99 | `0` | `&menu_assets.background_quads[display_state.buffer_index][0]);` | Persistent background packet indices are submitted 3,2,1,0. Preserve their explicit order because AddPrim prepends within the shared bucket. |
| 104, 110 | `1 × 2, 0 × 2` | `while (PadRead(1) != 0)` | Retain SDK PadRead selector 1; zero means no reported button bits. Release waits test nonzero, while dismissal tests a new nonzero sample. |
| 108 | `1, 0` | `if (PadRead(1) == 0)` | Retain SDK PadRead selector 1; zero means no reported button bits. Release waits test nonzero, while dismissal tests a new nonzero sample. |

## initializers

| Lines | Tokens | Expression | Reason |
| --- | --- | --- | --- |
| 9 | `22` | `s16 codes[22];` | Superseded reconstruction assumption, not a proved extent. The exact renderer uses the shared ten-code `MenuGlyphString`, supported by all seven panel producers. It still copies the caller-supplied row length without adding a terminator. |

## menu_list_render

| Lines | Tokens | Expression | Reason |
| --- | --- | --- | --- |
| 36 | `0` | `if (list->title_x != 0) {` | Zero tests optional title/quantity presence or an empty visible-row/entry count; these guards retain their original pointer and field semantics. |
| 45 | `0` | `row = 0;` | Zero starts the row, pixel-offset, tile-index or glyph-copy iteration; ordinary zero-based indexing. |
| 46 | `0 × 2` | `if (list->visible_rows != 0 && list->entry_count != 0) {` | Zero tests optional title/quantity presence or an empty visible-row/entry count; these guards retain their original pointer and field semantics. |
| 47, 101 | `0 × 2` | `yoff = 0;` | Zero starts the row, pixel-offset, tile-index or glyph-copy iteration; ordinary zero-based indexing. |
| 49 | `3` | `gs.x = list->list_x + 3;` | Inset the glyph origin three screen pixels inside the list tile, adding the accumulated row offset on Y. |
| 50 | `3` | `gs.y = list->list_y + 3 + yoff;` | Inset the glyph origin three screen pixels inside the list tile, adding the accumulated row offset on Y. |
| 51 | `0` | `for (i = 0; i < list->glyphs_per_entry; i++) {` | Zero starts the row, pixel-offset, tile-index or glyph-copy iteration; ordinary zero-based indexing. |
| 57 | `0` | `if (list->quantities != 0) {` | Zero tests optional title/quantity presence or an empty visible-row/entry count; these guards retain their original pointer and field semantics. |
| 58 | `10` | `tens = *counts / 10;` | Decimal radix ten separates quantity tens and units; glyph index one is the units position. |
| 59 | `1, 10` | `gs.codes[1] = *counts % 10;` | Decimal radix ten separates quantity tens and units; glyph index one is the units position. |
| 60 | `1` | `gs.y += 1;` | Align the eleven-pixel number glyph one pixel below the text baseline within the twelve-pixel row. |
| 61 | `0x6e` | `gs.x += 0x6e;` | Place the quantity column 110 screen pixels right of the row text origin; preserve the authored column spacing. |
| 62 | `0` | `gs.codes[0] = tens;` | Quantity glyph position zero holds the tens digit or a leading blank; ordinary first-cell index. |
| 63 | `0` | `if (tens == 0) {` | A zero tens digit is suppressed with the named blank rather than shown as a leading zero. |
| 64 | `0` | `gs.codes[0] = MENU_NUMBER_BLANK;` | Quantity glyph position zero holds the tens digit or a leading blank; ordinary first-cell index. |
| 66 | `2` | `gs.codes[2] = MENU_TEXT_END;` | Terminate immediately after the two quantity digits at zero-based position two. |
| 72, 129 | `0xc × 2` | `yoff += 0xc;` | Advance one twelve-pixel list row; this measured row pitch also positions the highlight and end tiles. |
| 96, 127, 155 | `1 × 3` | `SetSemiTrans(current_poly_ft4, 1);` | SDK Boolean 1 enables semi-transparency. |
| 99 | `0` | `if (list->visible_rows != 0) {` | Zero tests optional title/quantity presence or an empty visible-row/entry count; these guards retain their original pointer and field semantics. |
| 100 | `0` | `slot = 0;` | Zero starts the row, pixel-offset, tile-index or glyph-copy iteration; ordinary zero-based indexing. |
| 112 | `3` | `current_poly_ft4->y0 = list->list_y + yoff + 3;` | Highlight rows begin three pixels below list Y plus row offset; the end tile uses visible_rows times the twelve-pixel pitch, with descriptor height forming lower edges. |
| 114 | `3` | `current_poly_ft4->y1 = list->list_y + yoff + 3;` | Highlight rows begin three pixels below list Y plus row offset; the end tile uses visible_rows times the twelve-pixel pitch, with descriptor height forming lower edges. |
| 116 | `3` | `current_poly_ft4->y2 = list->list_y + yoff + tile->height + 3;` | Highlight rows begin three pixels below list Y plus row offset; the end tile uses visible_rows times the twelve-pixel pitch, with descriptor height forming lower edges. |
| 118 | `3` | `current_poly_ft4->y3 = list->list_y + yoff + tile->height + 3;` | Highlight rows begin three pixels below list Y plus row offset; the end tile uses visible_rows times the twelve-pixel pitch, with descriptor height forming lower edges. |
| 138 | `0xc, 3` | `current_poly_ft4->y0 = list->list_y + list->visible_rows * 0xc + 3;` | Highlight rows begin three pixels below list Y plus row offset; the end tile uses visible_rows times the twelve-pixel pitch, with descriptor height forming lower edges. |
| 140 | `0xc, 3` | `current_poly_ft4->y1 = list->list_y + list->visible_rows * 0xc + 3;` | Highlight rows begin three pixels below list Y plus row offset; the end tile uses visible_rows times the twelve-pixel pitch, with descriptor height forming lower edges. |
| 142 | `0xc` | `current_poly_ft4->y2 = list->list_y + list->visible_rows * 0xc +` | Highlight rows begin three pixels below list Y plus row offset; the end tile uses visible_rows times the twelve-pixel pitch, with descriptor height forming lower edges. |
| 143, 146 | `3 × 2` | `tile->height + 3;` | Highlight rows begin three pixels below list Y plus row offset; the end tile uses visible_rows times the twelve-pixel pitch, with descriptor height forming lower edges. |
| 145 | `0xc` | `current_poly_ft4->y3 = list->list_y + list->visible_rows * 0xc +` | Highlight rows begin three pixels below list Y plus row offset; the end tile uses visible_rows times the twelve-pixel pitch, with descriptor height forming lower edges. |
