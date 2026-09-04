#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/*
 * Visible-cell dispatcher for the map geometry pass.  It selects the cell
 * window for the current view (a per-yaw window from render_cell_windows
 * when the pitch is near level, otherwise the fixed DAT_80055e9c window),
 * publishes it through active_cell_window for render_entities' cull tests,
 * then walks the
 * window's cell grid and hands every populated, in-range cell to the per-cell
 * emitter render_map_cell.
 */

ADDRESS(0x8001e83c, 0x168)
void render_map_cells(void)
{
    u8 *cell;
    int row;
    int col_base;
    int col;
    u8 rows;
    u8 cols;

    if ((u16)((u16)render_state.view_rotation.vx + 0x1ff) >= 0x3ff) {
        active_cell_window = (const KfCellWindow *)&DAT_80055e9c;
    } else {
        active_cell_window =
            &render_cell_windows[15 - (render_state.view_rotation.vy >> 8)];
    }

    row = (u16)render_state.view_cell.z - active_cell_window->origin_z;
    col_base = (u16)render_state.view_cell.x - active_cell_window->origin_x;
    cell = active_cell_window->cells;
    tmd_select(0);

    rows = active_cell_window->height;
    do {
        if ((u32)row < 100) {
            col = col_base;
            cols = active_cell_window->width;
            do {
                if ((u32)col < 100 && *cell != 0) {
                    render_map_cell(col, row, *cell);
                }
                cell++;
                col++;
                cols--;
            } while (cols != 0);
        } else {
            cell += active_cell_window->width;
        }
        row++;
        rows--;
    } while (rows != 0);
}
