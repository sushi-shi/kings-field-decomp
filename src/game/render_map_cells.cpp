#include <kf/game/graphics.h>

#include <kf/lib/map_data.h>
#include <kf/game/render.h>
#include <kf/game/game.h>

enum {
    ILLUSION_STAFF_REMAP_PHASE_MASK = 3,
    ILLUSION_STAFF_REMAP_ACTIVE_UPDATES = 2
};

#define H KF_CELL_WINDOW_HIDDEN
#define D KF_CELL_WINDOW_DISTANT
#define N KF_CELL_WINDOW_NEAR

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

void render_map_cell(s32 col, s32 row, KfCellVisibility visibility)
{
    MATRIX cell_matrix;
    SVECTOR position;
    s32 orient;
    u8 object_index;
    s16 staff_timer;

    object_index = kf_enum_encode<u8>(map_cell_attribute_grid.cells[row][col]);
    if (object_index == kf_enum_encode<u8>(KF_MAP_ATTRIBUTE_NONE)) {
        return;
    }
    staff_timer = player_state.illusion_staff_timer;

    if (staff_timer != KF_ILLUSION_STAFF_INACTIVE
        && (staff_timer & ILLUSION_STAFF_REMAP_PHASE_MASK) < ILLUSION_STAFF_REMAP_ACTIVE_UPDATES) {
        switch (object_index) {
        case kf_enum_encode<u8>(KF_MAP_ATTRIBUTE_44):
            object_index = kf_enum_encode<u8>(KF_MAP_ATTRIBUTE_17);
            break;
        case kf_enum_encode<u8>(KF_MAP_ATTRIBUTE_HIDDEN_DOOR):
            object_index = kf_enum_encode<u8>(KF_MAP_ATTRIBUTE_18);
            break;
        case kf_enum_encode<u8>(KF_MAP_ATTRIBUTE_46):
            object_index = kf_enum_encode<u8>(KF_MAP_ATTRIBUTE_19);
            break;
        }
    }
    object_index = object_index - 1;
    if (object_index > KF_MAP_MESHES_PER_BANK - 1) {
        return;
    }
    orient = kf_enum_encode<u8>(map_cell_orientation_grid.cells[row][col]) - 1;
    if (visibility == KF_CELL_WINDOW_DISTANT) {
        object_index += KF_MAP_MESHES_PER_BANK;
    }
    position = VECTOR{
        col * KF_MAP_TILE_SIZE - game_graphics_runtime.render_state.view_position.vx,
        map_floor_height_grid.cells[row][col] * -KF_MAP_HEIGHT_STEP
            - game_graphics_runtime.render_state.view_position.vy,
        row * KF_MAP_TILE_SIZE - game_graphics_runtime.render_state.view_position.vz}.narrowed();
    if (orient == kf_enum_encode<u8>(KF_MAP_ORIENT_QUARTER_TURN) - 1) {
        position.vz += KF_MAP_TILE_SIZE;
    } else if (orient == kf_enum_encode<u8>(KF_MAP_ORIENT_HALF_TURN) - 1) {
        position.vx += KF_MAP_TILE_SIZE;
        position.vz += KF_MAP_TILE_SIZE;
    } else if (orient == kf_enum_encode<u8>(KF_MAP_ORIENT_THREE_QUARTER_TURN) - 1) {
        position.vx += KF_MAP_TILE_SIZE;
    }

    kf::render_place_model(cell_matrix, game_graphics_runtime.render_state.view_matrix, position);
    kf::matrix_multiply_rotation(game_graphics_runtime.render_state.view_matrix, game_graphics_runtime.render_state.quadrant_matrices[orient], cell_matrix);
    tmd_select_object_vertices(tmd_context(), object_index);
    render_enqueue_map(object_index, &game_graphics_runtime.light_quadrant_matrices[orient], &cell_matrix, game_graphics_runtime.render_state.projection);
}

void render_map_cells(void)
{
    const KfCellVisibility *cell;
    int row;
    int col_base;
    int col;
    u8 rows;
    u8 cols;

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
    tmd_select(tmd_context(), KF_TMD_SLOT_MAP);

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

void render_map_cells_reset_module_state(void)
{
    kf::restore_initial_value<render_fixed_cell_window>();
}
