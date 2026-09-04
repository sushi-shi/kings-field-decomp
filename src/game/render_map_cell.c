#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

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
    s32 attr;
    s32 tex;
    s16 light;

    attr = map_cell_attribute_grid[row][col];
    if (attr == 0xff) {
        return;
    }
    light = player_state.light_effect_timer;
    if (light != -1 && (light & 3) < 2) {
        if (attr == 0x45) {
            attr = 0x18;
        } else if (attr < 0x46) {
            if (attr == 0x44) {
                attr = 0x17;
            }
        } else {
            if (attr == 0x46) {
                attr = 0x19;
            }
        }
    }
    tex = attr - 1;
    if ((u8)tex > 99) {
        return;
    }
    orient = map_cell_orientation_grid[row][col] - 1;
    if (cell == 1) {
        tex += 100;
    }
    position.vx = col * 2000 - (u16)render_state.view_position.vx;
    position.vz = row * 2000 - (u16)render_state.view_position.vz;
    position.vy = map_floor_height_grid[row][col] * -100 - (u16)render_state.view_position.vy;
    if (orient == 1) {
        position.vz += 2000;
    } else if (orient == 2) {
        position.vx += 2000;
        position.vz += 2000;
    } else if (orient == 3) {
        position.vx += 2000;
    }

    SetRotMatrix((MATRIX *)&render_state.view_matrix);
    SetTransMatrix((MATRIX *)&render_state.view_matrix);
    RotTrans(&position, (VECTOR *)&cell_matrix.t, &flag);
    MulMatrix0((MATRIX *)&render_state.view_matrix,
               &render_state.quadrant_matrices[orient], &cell_matrix);
    SetRotMatrix(&cell_matrix);
    SetTransMatrix(&cell_matrix);
    SetLightMatrix(&light_quadrant_matrices[orient]);
    tmd_select_object_vertices((u8)tex);
    render_enqueue_map((u8)tex);
}
