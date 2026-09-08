#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/open_render.h>
#include <kf/open_scene0.h>

DATA(0x800439d8, 0xcc0)
KfCellWindow render_cell_windows[KF_CELL_WINDOW_YAW_COUNT];

ADDRESS(0x80018bbc, 0x1d0)
void render_map_cell(s32 col, s32 row, KfCellVisibility cell)
{
    MATRIX cell_matrix;
    SVECTOR position;
    long flag;
    s32 orientation;
    u8 object_index;

    object_index = map_cell_attribute_grid.cells[row][col];
    if (object_index == KF_MAP_ATTRIBUTE_NONE) {
        return;
    }
    object_index--;
    if (object_index > KF_MAP_MESHES_PER_BANK - 1) {
        return;
    }
    orientation = map_cell_orientation_grid.cells[row][col] - 1;
    if (cell == KF_CELL_WINDOW_DISTANT) {
        object_index += KF_MAP_MESHES_PER_BANK;
    }
    setVector(&position,
        col * KF_MAP_TILE_SIZE - (u16)open_graphics_runtime.render_state.view_position.vx,
        map_floor_height_grid.cells[row][col] * -KF_MAP_HEIGHT_STEP -
            (u16)open_graphics_runtime.render_state.view_position.vy,
        row * KF_MAP_TILE_SIZE - (u16)open_graphics_runtime.render_state.view_position.vz);
    if (orientation == KF_MAP_ORIENT_QUARTER_TURN - 1) {
        position.vz += KF_MAP_TILE_SIZE;
    } else if (orientation == KF_MAP_ORIENT_HALF_TURN - 1) {
        position.vx += KF_MAP_TILE_SIZE;
        position.vz += KF_MAP_TILE_SIZE;
    } else if (orientation == KF_MAP_ORIENT_THREE_QUARTER_TURN - 1) {
        position.vx += KF_MAP_TILE_SIZE;
    }

    SetRotMatrix(&open_graphics_runtime.render_state.view_matrix);
    SetTransMatrix(&open_graphics_runtime.render_state.view_matrix);
    /* RotTrans writes three words into the matrix's translation fields. */
    RotTrans(&position, (VECTOR *)&cell_matrix.t, &flag);
    MulMatrix0(&open_graphics_runtime.render_state.view_matrix,
        &open_graphics_runtime.render_state.quadrant_matrices[orientation], &cell_matrix);
    SetRotMatrix(&cell_matrix);
    SetTransMatrix(&cell_matrix);
    SetLightMatrix(&open_graphics_runtime.light_quadrant_matrices[orientation]);
    tmd_select_object_vertices(object_index);
    render_enqueue_map(object_index);
}

ADDRESS(0x80018d8c, 0x140)
void opening_render_map_cells(void)
{
    const KfCellVisibility *cell;
    s32 row;
    s32 col_base;
    s32 col;
    u8 rows;
    u8 cols;

    open_graphics_runtime.active_cell_window =
        &render_cell_windows[KF_CELL_WINDOW_YAW_COUNT - 1
            - (open_graphics_runtime.render_state.view_rotation.vy
                >> KF_CELL_WINDOW_YAW_SHIFT)];
    cell = open_graphics_runtime.active_cell_window->cells;
    row = (u16)open_graphics_runtime.render_state.view_cell.z - open_graphics_runtime.active_cell_window->origin_z;
    col_base = (u16)open_graphics_runtime.render_state.view_cell.x - open_graphics_runtime.active_cell_window->origin_x;
    tmd_select(KF_TMD_SLOT_MAP);

    rows = open_graphics_runtime.active_cell_window->height;
    do {
        if ((u32)row < KF_MAP_ROWS) {
            col = col_base;
            cols = open_graphics_runtime.active_cell_window->width;
            do {
                if ((u32)col < KF_MAP_COLUMNS && *cell != KF_CELL_WINDOW_HIDDEN) {
                    render_map_cell(col, row, *cell);
                }
                cell++;
                col++;
                cols--;
            } while (cols != 0);
        } else {
            cell += open_graphics_runtime.active_cell_window->width;
        }
        row++;
        rows--;
    } while (rows != 0);
}
