# Map-cell retained-literal ledger

Companion to [the visibility-domain review](visibility-screen-domains.md).
All 28 retained numeric occurrences in these two modules are listed below.
The named fixed-window classes are excluded. Repeated tokens have separate rows.

## `src/game/render_map_cells.c`

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `render_fixed_cell_window` | 12 | `13` | `13, 13, 6, 6,` | Authored fixed-window width in cells; preserve the explicit table dimensions. |
| `render_fixed_cell_window` | 12 | `13` | `13, 13, 6, 6,` | Authored fixed-window height in cells; preserve the explicit table dimensions. |
| `render_fixed_cell_window` | 12 | `6` | `13, 13, 6, 6,` | Authored horizontal origin, centered within the 13-column pattern. |
| `render_fixed_cell_window` | 12 | `6` | `13, 13, 6, 6,` | Authored depth origin, centered within the 13-row pattern. |
| `render_map_cell` | 58 | `0x44` | `case 0x44:` | Authored source attribute in the staff remapping; its specific wall/door identity remains unproved, so do not invent a semantic mesh name. |
| `render_map_cell` | 59 | `0x17` | `object_index = 0x17;` | Authored replacement attribute in the staff remapping, before conversion to a zero-based mesh index; no independent semantic identity for this replacement is established. |
| `render_map_cell` | 62 | `0x18` | `object_index = 0x18;` | Authored replacement attribute in the staff remapping, before conversion to a zero-based mesh index; no independent semantic identity for this replacement is established. |
| `render_map_cell` | 64 | `0x46` | `case 0x46:` | Authored source attribute in the staff remapping; its specific wall/door identity remains unproved, so do not invent a semantic mesh name. |
| `render_map_cell` | 65 | `0x19` | `object_index = 0x19;` | Authored replacement attribute in the staff remapping, before conversion to a zero-based mesh index; no independent semantic identity for this replacement is established. |
| `render_map_cell` | 70 | `1` | `if (object_index > KF_MAP_MESHES_PER_BANK - 1) {` | Convert the bank count to the maximum zero-based mesh index. |
| `render_map_cell` | 73 | `1` | `orient = map_cell_orientation_grid[row][col] - 1;` | Convert the stored one-based orientation to the zero-based quadrant index. |
| `render_map_cell` | 82 | `1` | `if (orient == KF_MAP_ORIENT_QUARTER_TURN - 1) {` | Compare the zero-based quadrant with the corresponding one-based orientation member minus one. |
| `render_map_cell` | 84 | `1` | `} else if (orient == KF_MAP_ORIENT_HALF_TURN - 1) {` | Compare the zero-based quadrant with the corresponding one-based orientation member minus one. |
| `render_map_cell` | 87 | `1` | `} else if (orient == KF_MAP_ORIENT_THREE_QUARTER_TURN - 1) {` | Compare the zero-based quadrant with the corresponding one-based orientation member minus one. |
| `render_map_cells` | 123 | `1` | `if ((u16)((u16)render_state.view_rotation.vx + (KF_ANGLE_EIGHTH_TURN - 1))` | Endpoint adjustment preserves the strict -45 to +45 degree directional-window interval in integer angle units. |
| `render_map_cells` | 124 | `2` | `>= 2 * KF_ANGLE_EIGHTH_TURN - 1) {` | Symmetric pitch interval spans twice the named eighth-turn angle. |
| `render_map_cells` | 124 | `1` | `>= 2 * KF_ANGLE_EIGHTH_TURN - 1) {` | Endpoint adjustment preserves the strict -45 to +45 degree directional-window interval in integer angle units. |
| `render_map_cells` | 128 | `1` | `&render_cell_windows[KF_CELL_WINDOW_YAW_COUNT - 1` | Convert the yaw-window count to its last index before reversing the yaw order. |
| `render_map_cells` | 149 | `0` | `} while (cols != 0);` | Exhausted byte countdown terminates the column loop. |
| `render_map_cells` | 155 | `0` | `} while (rows != 0);` | Exhausted byte countdown terminates the row loop. |

## `src/open/render_map_cells.c`

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `render_map_cell` | 23 | `1` | `if (object_index > KF_MAP_MESHES_PER_BANK - 1) {` | Convert the bank count to the maximum zero-based mesh index. |
| `render_map_cell` | 26 | `1` | `orientation = map_cell_orientation_grid[row][col] - 1;` | Convert the stored one-based orientation to the zero-based quadrant index. |
| `render_map_cell` | 35 | `1` | `if (orientation == KF_MAP_ORIENT_QUARTER_TURN - 1) {` | Compare the zero-based quadrant with the corresponding one-based orientation member minus one. |
| `render_map_cell` | 37 | `1` | `} else if (orientation == KF_MAP_ORIENT_HALF_TURN - 1) {` | Compare the zero-based quadrant with the corresponding one-based orientation member minus one. |
| `render_map_cell` | 40 | `1` | `} else if (orientation == KF_MAP_ORIENT_THREE_QUARTER_TURN - 1) {` | Compare the zero-based quadrant with the corresponding one-based orientation member minus one. |
| `opening_render_map_cells` | 68 | `1` | `&render_cell_windows[KF_CELL_WINDOW_YAW_COUNT - 1` | Convert the yaw-window count to its last index before reversing the yaw order. |
| `opening_render_map_cells` | 88 | `0` | `} while (cols != 0);` | Exhausted byte countdown terminates the column loop. |
| `opening_render_map_cells` | 94 | `0` | `} while (rows != 0);` | Exhausted byte countdown terminates the row loop. |
