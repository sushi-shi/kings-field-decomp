#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_render.h>
#include <kf/game.h>

DATA(0x80055e9c, 0xcc)
static KfCellWindow render_fixed_cell_window = {
    13, 13, 6, 6,
    {
        0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
        0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
        0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
    }
};

DATA(0x80095860, 0x4)
const KfCellWindow *active_cell_window;

/*
 * Contiguous visible-map-cell render path 0x8001e5ec..0x8001e9a4. The
 * dispatcher is the emitter's sole external caller, and both functions share
 * the same cell grids, render state, and map-render call chain.
 */

/*
 * Per-cell wall/floor geometry emitter for the map pass, called by
 * render_map_cells for each visible, populated cell.  Reads the cell's
 * attribute byte to pick the wall TMD object (remapping the three door codes
 * 0x44/0x45/0x46 to 0x17/0x18/0x19 while a light effect is active and level),
 * builds the cell's world position relative to the view origin, applies the
 * per-orientation quadrant rotation/light matrices, and enqueues the object.
 *
 * The view world position is read as its low 16 bits (`(u16)view_position.*`),
 * matching the wrapped-coordinate reads shared with the actor/object emitters.
 * RotTrans writes the transformed origin straight into the cell matrix's
 * translation slot before MulMatrix0 fills its rotation (the shared
 * &model.t idiom from render_actor).
 */
ADDRESS(0x8001e5ec, 0x250)
void render_map_cell(s32 col, s32 row, char cell)
{
    MATRIX cell_matrix;
    SVECTOR position;
    long flag;
    s32 orient;
    u8 object_index;
    s16 light;

    object_index = map_cell_attribute_grid[row][col];
    if (object_index == 0xff) {
        return;
    }
    light = player_state.light_effect_timer;
    if (light != -1 && (light & 3) < 2) {
        switch (object_index) {
        case 0x44:
            object_index = 0x17;
            break;
        case 0x45:
            object_index = 0x18;
            break;
        case 0x46:
            object_index = 0x19;
            break;
        }
    }
    object_index--;
    if (object_index > 99) {
        return;
    }
    orient = map_cell_orientation_grid[row][col] - 1;
    if (cell == 1) {
        object_index += 100;
    }
    setVector(&position,
        col * KF_MAP_TILE_SIZE - (u16)render_state.view_position.vx,
        map_floor_height_grid[row][col] * -KF_MAP_HEIGHT_STEP
            - (u16)render_state.view_position.vy,
        row * KF_MAP_TILE_SIZE - (u16)render_state.view_position.vz);
    if (orient == 1) {
        position.vz += KF_MAP_TILE_SIZE;
    } else if (orient == 2) {
        position.vx += KF_MAP_TILE_SIZE;
        position.vz += KF_MAP_TILE_SIZE;
    } else if (orient == 3) {
        position.vx += KF_MAP_TILE_SIZE;
    }

    SetRotMatrix((MATRIX *)&render_state.view_matrix);
    SetTransMatrix((MATRIX *)&render_state.view_matrix);
    RotTrans(&position, (VECTOR *)&cell_matrix.t, &flag);
    MulMatrix0((MATRIX *)&render_state.view_matrix,
               &render_state.quadrant_matrices[orient], &cell_matrix);
    SetRotMatrix(&cell_matrix);
    SetTransMatrix(&cell_matrix);
    SetLightMatrix(&light_quadrant_matrices[orient]);
    tmd_select_object_vertices(object_index);
    render_enqueue_map(object_index);
}

/*
 * Visible-cell dispatcher for the map geometry pass.  It selects the cell
 * window for the current view (a per-yaw window from render_cell_windows
 * when the pitch is near level, otherwise the fixed window),
 * publishes it through active_cell_window for render_entities' cull tests,
 * then walks the window's cell grid and hands every populated, in-range cell
 * to the per-cell emitter render_map_cell.
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
        active_cell_window = &render_fixed_cell_window;
    } else {
        active_cell_window =
            &render_cell_windows[KF_CELL_WINDOW_YAW_COUNT - 1
                - (render_state.view_rotation.vy >> KF_CELL_WINDOW_YAW_SHIFT)];
    }

    row = (u16)render_state.view_cell.z - active_cell_window->origin_z;
    col_base = (u16)render_state.view_cell.x - active_cell_window->origin_x;
    cell = active_cell_window->cells;
    tmd_select(KF_TMD_SLOT_MAP);

    rows = active_cell_window->height;
    do {
        if ((u32)row < KF_MAP_ROWS) {
            col = col_base;
            cols = active_cell_window->width;
            do {
                if ((u32)col < KF_MAP_COLUMNS && *cell != 0) {
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
