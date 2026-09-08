#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

/*
 * Draw one menu window: an optional title label (drawn when the record's first
 * halfword is non-zero), then `count` selectable rows, then the shared
 * translucent backdrop for every window kind but the configuration window.  The highlighted row
 * (index `highlight`) takes the confirmed-selection background when confirmation is
 * requested and always gets the selection-cursor sprite overlaid.  Rows advance one
 * MenuGlyphString per step starting at the record's first row.
 */
ADDRESS(0x80028914, 0x15c)
void menu_draw_window(KfMenuWindowKind kind, s32 count, s32 highlight, KfMenuConfirmState confirmation)
{
    const MenuWindowLayout *layout;
    s32 row;

    layout = &menu_window_layouts[KF_ENUM_ENCODE(s32, kind)];
    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;
    if (layout->title.position.x != 0) {
        menu_blit_sprite_translucent(
            &menu_assets.row_background, &layout->title.position);
        menu_draw_string(&menu_assets.glyph_atlas, &layout->title);
    }
    if (count > 0) {
        row = 0;
        do {
            const MenuGlyphString *label = &layout->rows[row];
            const MenuSpriteDef *box = &menu_assets.row_background;

            if (row == highlight && confirmation == KF_MENU_CONFIRM_REQUESTED) {
                box = &menu_assets.row_confirmed_background;
            }
            menu_blit_sprite_translucent(box, &label->position);
            if (row == highlight) {
                menu_blit_sprite(&menu_assets.selection_cursor, &label->position);
            }
            menu_draw_string(&menu_assets.glyph_atlas, label);
            row++;
        } while (row < count);
    }
    if (kind != KF_MENU_WINDOW_CONFIG) {
        menu_draw_window_backdrop();
    }
}
