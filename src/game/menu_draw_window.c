#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

/*
 * Draw one menu window: an optional title label (drawn when the record's first
 * halfword is non-zero), then `count` selectable rows, then the shared
 * translucent backdrop for every window kind but 6.  The highlighted row
 * (index `highlight`) takes the confirmed-selection background when `flag` is
 * 1 and always gets the selection-cursor sprite overlaid.  Rows advance one
 * MenuGlyphString per step starting at the record's first row.
 *
 * Retail retains the layout base and a byte row offset, then recomputes their
 * sum in call delay slots.  gcc257-o2 CSEs the typed row pointer and gives the
 * flag an additional saved-register role.  The structured row model is kept;
 * the remaining difference is an unattributed CSE/register-allocation residue.
 */
ADDRESS(0x80028914, 0x15c)
void menu_draw_window(s32 kind, s32 count, s32 highlight, s32 flag)
{
    const MenuWindowLayout *layout;
    s32 row;

    layout = &menu_window_layouts[kind];
    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;
    if (layout->title.x != 0) {
        menu_blit_sprite_translucent(
            &DAT_80058424, (const MenuPoint *)&layout->title);
        menu_draw_string(&DAT_800583f4, &layout->title);
    }
    if (count > 0) {
        row = 0;
        do {
            const MenuGlyphString *label = &layout->rows[row];
            const MenuSpriteDef *box = &DAT_80058424;

            if (row == highlight && flag == 1) {
                box = &DAT_80058430;
            }
            menu_blit_sprite_translucent(box, (const MenuPoint *)label);
            if (row == highlight) {
                menu_blit_sprite(&DAT_8005846c, (const MenuPoint *)label);
            }
            menu_draw_string(&DAT_800583f4, label);
            row++;
        } while (row < count);
    }
    if (kind != 6) {
        menu_draw_window_backdrop();
    }
}
