#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

enum {
    MENU_LIST_TEXT_INSET = 3,
    MENU_LIST_ROW_HEIGHT = 12,
    MENU_LIST_QUANTITY_X_OFFSET = 110,
    MENU_LIST_QUANTITY_Y_OFFSET = 1
};

/*
 * Render a scrollable menu list: the optional title label, the visible glyph
 * rows (each with an optional two-digit quantity), the list backdrop tile, the
 * per-row highlight tiles (the cursor row taking the selected descriptor), the
 * scroll-end tile, and the shared window backdrop.  Reached from the item
 * buy/sell hubs and the other list menus.
 */
ADDRESS(0x80028a70, 0x77c)
void menu_list_render(const KfMenuList *list)
{
    MenuGlyphString gs;
    const MenuTileSprite *tile;
    s16 *src;
    u8 *counts;
    s32 row;
    s32 i;
    s32 yoff;
    u32 tens;
    u32 ones;

    src = list->glyph_rows;
    counts = list->quantities;
    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

    if (list->title.position.x != 0) {
        menu_blit_sprite_translucent(
            &menu_assets.row_background, &list->title.position);
        menu_draw_string(
            &menu_assets.glyph_atlas, &list->title);
    }

    counts = counts + list->scroll_offset;
    src = src + list->scroll_offset * list->glyphs_per_entry;
    row = 0;
    if (row < list->visible_rows && row < list->entry_count) {
        do {
            gs.position.x = list->list_x + MENU_LIST_TEXT_INSET;
            gs.position.y = list->list_y + MENU_LIST_TEXT_INSET;
            gs.position.y += row * MENU_LIST_ROW_HEIGHT;
            for (i = 0; i < list->glyphs_per_entry; i++) {
                gs.glyphs.codes[i] = *src++;
            }
            menu_draw_string(
                &menu_assets.glyph_atlas, &gs);
            if (list->quantities != 0) {
                tens = *counts / 10u;
                ones = *counts % 10u;
                gs.position.x += MENU_LIST_QUANTITY_X_OFFSET;
                gs.position.y += MENU_LIST_QUANTITY_Y_OFFSET;
                gs.glyphs.codes[0] = tens;
                if (tens == 0) {
                    gs.glyphs.codes[0] = MENU_NUMBER_BLANK;
                }
                gs.glyphs.codes[1] = ones;
                gs.glyphs.codes[2] = MENU_TEXT_END;
                menu_draw_number(
                    &menu_assets.number_atlas, &gs);
                counts++;
            }
        } while (++row < list->visible_rows && row < list->entry_count);
    }

    tile = &menu_assets.list_tiles[MENU_LIST_TILE_BACKDROP];
    primitive_buffer_begin_poly_ft4();
    current_poly_ft4->tpage = tile->tpage;
    current_poly_ft4->clut = tile->clut;
    current_poly_ft4->x0 = list->list_x;
    current_poly_ft4->y0 = list->list_y;
    current_poly_ft4->x1 = list->list_x + tile->width;
    current_poly_ft4->y1 = list->list_y;
    current_poly_ft4->x2 = list->list_x;
    current_poly_ft4->y2 = list->list_y + tile->height;
    current_poly_ft4->x3 = list->list_x + tile->width;
    current_poly_ft4->y3 = list->list_y + tile->height;
    current_poly_ft4->u0 = tile->u;
    current_poly_ft4->v0 = tile->v;
    current_poly_ft4->u1 = tile->u + tile->width;
    current_poly_ft4->v1 = tile->v;
    current_poly_ft4->u2 = tile->u;
    current_poly_ft4->v2 = tile->v + tile->height;
    current_poly_ft4->u3 = tile->u + tile->width;
    current_poly_ft4->v3 = tile->v + tile->height;
    SetSemiTrans(current_poly_ft4, 1);
    primitive_buffer_commit_poly_ft4(MENU_WIDGET_OT_DEPTH);

    row = 0;
    if (row < list->visible_rows) {
        yoff = 0;
        do {
            tile = &menu_assets.list_tiles[MENU_LIST_TILE_ROW];
            if (row == list->cursor_row) {
                tile = &menu_assets.list_tiles[MENU_LIST_TILE_SELECTED];
            }
            row++;
            primitive_buffer_begin_poly_ft4();
            current_poly_ft4->tpage = tile->tpage;
            current_poly_ft4->clut = tile->clut;
            current_poly_ft4->x0 = list->list_x;
            current_poly_ft4->y0 = list->list_y + yoff + MENU_LIST_TEXT_INSET;
            current_poly_ft4->x1 = list->list_x + tile->width;
            current_poly_ft4->y1 = list->list_y + yoff + MENU_LIST_TEXT_INSET;
            current_poly_ft4->x2 = list->list_x;
            current_poly_ft4->y2 = list->list_y + yoff + (tile->height + MENU_LIST_TEXT_INSET);
            current_poly_ft4->x3 = list->list_x + tile->width;
            current_poly_ft4->y3 = list->list_y + yoff + (tile->height + MENU_LIST_TEXT_INSET);
            current_poly_ft4->u0 = tile->u;
            current_poly_ft4->v0 = tile->v;
            current_poly_ft4->u1 = tile->u + tile->width;
            current_poly_ft4->v1 = tile->v;
            current_poly_ft4->u2 = tile->u;
            current_poly_ft4->v2 = tile->v + tile->height;
            current_poly_ft4->u3 = tile->u + tile->width;
            current_poly_ft4->v3 = tile->v + tile->height;
            SetSemiTrans(current_poly_ft4, 1);
            primitive_buffer_commit_poly_ft4(MENU_WIDGET_OT_DEPTH);
            yoff += MENU_LIST_ROW_HEIGHT;
        } while (row < list->visible_rows);
    }

    tile = &menu_assets.list_tiles[MENU_LIST_TILE_END];
    primitive_buffer_begin_poly_ft4();
    current_poly_ft4->tpage = tile->tpage;
    current_poly_ft4->clut = tile->clut;
    current_poly_ft4->x0 = list->list_x;
    current_poly_ft4->y0 = list->list_y + list->visible_rows * MENU_LIST_ROW_HEIGHT + MENU_LIST_TEXT_INSET;
    current_poly_ft4->x1 = list->list_x + tile->width;
    current_poly_ft4->y1 = list->list_y + list->visible_rows * MENU_LIST_ROW_HEIGHT + MENU_LIST_TEXT_INSET;
    current_poly_ft4->x2 = list->list_x;
    current_poly_ft4->y2 = list->list_y + list->visible_rows * MENU_LIST_ROW_HEIGHT +
                           (tile->height + MENU_LIST_TEXT_INSET);
    current_poly_ft4->x3 = list->list_x + tile->width;
    current_poly_ft4->y3 = list->list_y + list->visible_rows * MENU_LIST_ROW_HEIGHT +
                           (tile->height + MENU_LIST_TEXT_INSET);
    current_poly_ft4->u0 = tile->u;
    current_poly_ft4->v0 = tile->v;
    current_poly_ft4->u1 = tile->u + tile->width;
    current_poly_ft4->v1 = tile->v;
    current_poly_ft4->u2 = tile->u;
    current_poly_ft4->v2 = tile->v + tile->height;
    current_poly_ft4->u3 = tile->u + tile->width;
    current_poly_ft4->v3 = tile->v + tile->height;
    SetSemiTrans(current_poly_ft4, 1);
    primitive_buffer_commit_poly_ft4(MENU_WIDGET_OT_DEPTH);

    menu_draw_window_backdrop();
}
