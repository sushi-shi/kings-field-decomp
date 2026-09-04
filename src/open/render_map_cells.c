#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/open_render.h>
#include <kf/open_scene0.h>

DATA(0x800439d8, 0xcc0)
KfCellWindow render_cell_windows[16];

DATA(0x8006e1c8, 0x4)
const KfCellWindow *active_cell_window;

ADDRESS(0x80018bbc, 0x1d0)
void render_map_cell(s32 col, s32 row, u8 cell)
{
    MATRIX cell_matrix;
    SVECTOR position;
    long flag;
    s32 orientation;
    u8 object_index;

    object_index = map_cell_attribute_grid[row][col];
    if (object_index == 0xff) {
        return;
    }
    object_index--;
    if (object_index > 99) {
        return;
    }
    orientation = map_cell_orientation_grid[row][col] - 1;
    if (cell == 1) {
        object_index += 100;
    }
    position.vx = col * 2000 - (u16)render_state.view_position.vx;
    position.vz = row * 2000 - (u16)render_state.view_position.vz;
    position.vy = map_floor_height_grid[row][col] * -100 -
        (u16)render_state.view_position.vy;
    if (orientation == 1) {
        position.vz += 2000;
    } else if (orientation == 2) {
        position.vx += 2000;
        position.vz += 2000;
    } else if (orientation == 3) {
        position.vx += 2000;
    }

    SetRotMatrix(&render_state.view_matrix);
    SetTransMatrix(&render_state.view_matrix);
    /* RotTrans writes three words into the matrix's translation fields. */
    RotTrans(&position, (VECTOR *)&cell_matrix.t, &flag);
    MulMatrix0(&render_state.view_matrix,
        &render_state.quadrant_matrices[orientation], &cell_matrix);
    SetRotMatrix(&cell_matrix);
    SetTransMatrix(&cell_matrix);
    SetLightMatrix(&light_quadrant_matrices[orientation]);
    tmd_select_object_vertices(object_index);
    render_enqueue_map(object_index);
}

ADDRESS(0x80018d8c, 0x140)
void opening_render_map_cells(void)
{
    const u8 *cell;
    s32 row;
    s32 col_base;
    s32 col;
    u8 rows;
    u8 cols;

    active_cell_window =
        &render_cell_windows[15 - (render_state.view_rotation.vy >> 8)];
    cell = active_cell_window->cells;
    row = (u16)render_state.view_cell.z - active_cell_window->origin_z;
    col_base = (u16)render_state.view_cell.x - active_cell_window->origin_x;
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
