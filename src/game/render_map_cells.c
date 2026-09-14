#include <kf/game/graphics.h>
#include <kf/lib/address.h>
#include <kf/lib/map_data.h>
#include <kf/game/render.h>
#include <kf/game/game.h>

enum {
    ILLUSION_STAFF_REMAP_PHASE_MASK = 3,
    ILLUSION_STAFF_REMAP_ACTIVE_UPDATES = 2
};

/* Local legend preserves the authored grid shape. */
#define H KF_CELL_WINDOW_HIDDEN
#define D KF_CELL_WINDOW_DISTANT
#define N KF_CELL_WINDOW_NEAR
DATA(0x80055e9c, 0xcc)
static KfCellWindow render_fixed_cell_window = {
    13, 13, 6, 6,
    {
        H, H, H, H, D, D, D, D, D, D, H, H, H,
        H, H, H, D, D, D, D, D, D, D, D, H, H,
        H, H, D, D, D, D, D, D, D, D, D, D, H,
        H, D, D, D, D, D, D, D, D, D, D, D, D,
        D, D, D, D, D, D, D, D, D, D, D, D, D,
        D, D, D, D, D, N, N, N, D, D, D, D, D,
        D, D, D, D, D, N, N, N, D, D, D, D, D,
        D, D, D, D, D, N, N, N, D, D, D, D, D,
        D, D, D, D, D, D, D, D, D, D, D, D, D,
        H, D, D, D, D, D, D, D, D, D, D, D, D,
        H, H, D, D, D, D, D, D, D, D, D, D, H,
        H, H, H, D, D, D, D, D, D, D, D, H, H,
        H, H, H, H, D, D, D, D, D, D, H, H, H,
    }
};
#undef H
#undef D
#undef N

/*
 * The SVECTOR destination narrows the view-relative coordinates. RotTrans
 * writes MATRIX.t before MulMatrix0 fills its rotation.
 */
ADDRESS(0x8001e5ec, 0x250)
void render_map_cell(s32 col, s32 row, KF_ENUM_PARAM(KfCellVisibility, char) visibility)
{
    MATRIX cell_matrix;
    SVECTOR position;
    long flag;
    s32 orient;
    u8 object_index;
    s16 staff_timer;

    object_index = KF_ENUM_ENCODE(u8, map_cell_attribute_grid.cells[row][col]);
    if (object_index == KF_ENUM_ENCODE(u8, KF_MAP_ATTRIBUTE_NONE)) {
        return;
    }
    staff_timer = player_state.illusion_staff_timer;
    /* Two-on/two-off updates; -1 disables the authored mesh remapping. */
    if (staff_timer != KF_ILLUSION_STAFF_INACTIVE
        && (staff_timer & ILLUSION_STAFF_REMAP_PHASE_MASK) < ILLUSION_STAFF_REMAP_ACTIVE_UPDATES) {
        switch (object_index) {
        case KF_ENUM_ENCODE(u8, KF_MAP_ATTRIBUTE_44):
            object_index = KF_ENUM_ENCODE(u8, KF_MAP_ATTRIBUTE_17);
            break;
        case KF_ENUM_ENCODE(u8, KF_MAP_ATTRIBUTE_HIDDEN_DOOR):
            object_index = KF_ENUM_ENCODE(u8, KF_MAP_ATTRIBUTE_18);
            break;
        case KF_ENUM_ENCODE(u8, KF_MAP_ATTRIBUTE_46):
            object_index = KF_ENUM_ENCODE(u8, KF_MAP_ATTRIBUTE_19);
            break;
        }
    }
    object_index = object_index - 1;
    if (object_index > KF_MAP_MESHES_PER_BANK - 1) {
        return;
    }
    orient = KF_ENUM_ENCODE(u8, map_cell_orientation_grid.cells[row][col]) - 1;
    if (visibility == KF_CELL_WINDOW_DISTANT) {
        object_index += KF_MAP_MESHES_PER_BANK;
    }
    setVector(&position,
        col * KF_MAP_TILE_SIZE - game_graphics_runtime.render_state.view_position.vx,
        map_floor_height_grid.cells[row][col] * -KF_MAP_HEIGHT_STEP
            - game_graphics_runtime.render_state.view_position.vy,
        row * KF_MAP_TILE_SIZE - game_graphics_runtime.render_state.view_position.vz);
    if (orient == KF_ENUM_ENCODE(u8, KF_MAP_ORIENT_QUARTER_TURN) - 1) {
        position.vz += KF_MAP_TILE_SIZE;
    } else if (orient == KF_ENUM_ENCODE(u8, KF_MAP_ORIENT_HALF_TURN) - 1) {
        position.vx += KF_MAP_TILE_SIZE;
        position.vz += KF_MAP_TILE_SIZE;
    } else if (orient == KF_ENUM_ENCODE(u8, KF_MAP_ORIENT_THREE_QUARTER_TURN) - 1) {
        position.vx += KF_MAP_TILE_SIZE;
    }

    SetRotMatrix(&game_graphics_runtime.render_state.view_matrix);
    SetTransMatrix(&game_graphics_runtime.render_state.view_matrix);
    RotTrans(&position, (VECTOR *)&cell_matrix.t, &flag);
    MulMatrix0(&game_graphics_runtime.render_state.view_matrix,
               &game_graphics_runtime.render_state.quadrant_matrices[orient], &cell_matrix);
    SetRotMatrix(&cell_matrix);
    SetTransMatrix(&cell_matrix);
    SetLightMatrix(&game_graphics_runtime.light_quadrant_matrices[orient]);
    tmd_select_object_vertices(object_index);
    render_enqueue_map(object_index);
}

/*
 * Visible-cell dispatcher for the map geometry pass.  It selects the cell
 * window for the current view (a per-yaw window from render_cell_windows
 * when the pitch is near level, otherwise the fixed window),
 * publishes the selected window for render_entities' cull tests,
 * then walks the window's cell grid and hands every populated, in-range cell
 * to the per-cell emitter render_map_cell.
 */

ADDRESS(0x8001e83c, 0x168)
void render_map_cells(void)
{
    const KfCellVisibility *cell;
    int row;
    int col_base;
    int col;
    u8 rows;
    u8 cols;

    /* Directional windows apply only for -45 degrees < pitch < 45 degrees. */
    if ((u16)(game_graphics_runtime.render_state.view_rotation.vx + (KF_ANGLE_EIGHTH_TURN - 1))
        >= 2 * KF_ANGLE_EIGHTH_TURN - 1) {
        game_graphics_runtime.active_cell_window = &render_fixed_cell_window;
    } else {
        game_graphics_runtime.active_cell_window =
            &render_cell_windows[KF_CELL_WINDOW_YAW_COUNT - 1
                - (game_graphics_runtime.render_state.view_rotation.vy >> KF_CELL_WINDOW_YAW_SHIFT)];
    }

    row = (u16)game_graphics_runtime.render_state.view_cell.z - game_graphics_runtime.active_cell_window->origin_z;
    col_base = (u16)game_graphics_runtime.render_state.view_cell.x - game_graphics_runtime.active_cell_window->origin_x;
    cell = game_graphics_runtime.active_cell_window->cells;
    tmd_select(KF_TMD_SLOT_MAP);

    rows = game_graphics_runtime.active_cell_window->height;
    do {
        if ((u32)row < KF_MAP_ROWS) {
            col = col_base;
            cols = game_graphics_runtime.active_cell_window->width;
            do {
                if ((u32)col < KF_MAP_COLUMNS && *cell != KF_CELL_WINDOW_HIDDEN) {
                    render_map_cell(col, row, *cell);
                }
                cell++;
                col++;
                cols--;
            } while (cols != 0);
        } else {
            cell += game_graphics_runtime.active_cell_window->width;
        }
        row++;
        rows--;
    } while (rows != 0);
}
