#include <kf/address.h>
#include <kf/semantic_types.h>

/*
 * Visible-cell dispatcher for the map geometry pass.  It selects the cell
 * window for the current view (a per-yaw window from the DAT_80065be8 table
 * when the pitch is near level, otherwise the fixed DAT_80055e9c window),
 * publishes it at DAT_80095860 for render_entities' cull tests, then walks the
 * window's cell grid and hands every populated, in-range cell to the per-cell
 * emitter func_8001e5ec.
 *
 * WIP: func_8001e5ec (the per-cell wall/floor emitter) is not yet reconstructed
 * and is reached by its address identity.  The window table DAT_80065be8 is a
 * 16-entry, 204-byte-stride block; its interior beyond the grid header is
 * unresolved, so the entry is reached as a byte offset and the grid is a local
 * KfCellWindow view.
 */

extern KfRenderState render_state;
extern u8 DAT_80065be8[]; /* 16 x 204-byte per-yaw cell windows */
extern u16 DAT_80055e9c;  /* fixed fallback cell window */

/* Visible map cell window: dimensions, window origin, then the cell flags. */
typedef struct KfCellWindow {
    u16 width;
    u16 height;
    u16 origin_x;
    u16 origin_z;
    u8 cells[1];
} KfCellWindow;

extern KfCellWindow *DAT_80095860;

extern void tmd_select(u16 slot);
extern void func_8001e5ec(s32 col, s32 row, char cell);

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
        DAT_80095860 = (KfCellWindow *)&DAT_80055e9c;
    } else {
        DAT_80095860 = (KfCellWindow *)&DAT_80065be8[(15 - (render_state.view_rotation.vy >> 8)) * 204];
    }

    row = (u16)render_state.view_cell.z - DAT_80095860->origin_z;
    col_base = (u16)render_state.view_cell.x - DAT_80095860->origin_x;
    cell = DAT_80095860->cells;
    tmd_select(0);

    rows = DAT_80095860->height;
    do {
        if ((u32)row < 100) {
            col = col_base;
            cols = DAT_80095860->width;
            do {
                if ((u32)col < 100 && *cell != 0) {
                    func_8001e5ec(col, row, *cell);
                }
                cell++;
                col++;
                cols--;
            } while (cols != 0);
        } else {
            cell += DAT_80095860->width;
        }
        row++;
        rows--;
    } while (rows != 0);
}
