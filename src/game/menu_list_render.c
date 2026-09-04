#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

/* Positioned glyph run: origin then codes terminated by -1. */
typedef struct MenuGlyphBuffer {
    u16 x;
    u16 y;
    s16 codes[22];
} MenuGlyphBuffer;

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
    MenuGlyphBuffer gs;
    const MenuTileSprite *tile;
    s16 *src;
    u8 *counts;
    s32 row;
    s32 i;
    s32 yoff;
    u32 slot;
    u32 tens;

    src = list->glyph_rows;
    counts = list->quantities;
    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;

    if (list->title_x != 0) {
        menu_blit_sprite_translucent(
            &DAT_80058424, (const MenuPoint *)list);
        menu_draw_string(
            &DAT_800583f4, (const MenuGlyphString *)list);
    }

    counts = counts + list->scroll_offset;
    src = src + list->scroll_offset * list->glyphs_per_entry;
    row = 0;
    if (list->visible_rows != 0 && list->entry_count != 0) {
        yoff = 0;
        do {
            gs.x = list->list_x + 3;
            gs.y = list->list_y + 3 + yoff;
            for (i = 0; i < list->glyphs_per_entry; i++) {
                gs.codes[i] = src[i];
            }
            src += list->glyphs_per_entry;
            menu_draw_string(
                &DAT_800583f4, (const MenuGlyphString *)&gs);
            if (list->quantities != 0) {
                tens = *counts / 10;
                gs.codes[1] = *counts % 10;
                gs.y += 1;
                gs.x += 0x6e;
                gs.codes[0] = tens;
                if (tens == 0) {
                    gs.codes[0] = 10;
                }
                gs.codes[2] = -1;
                menu_draw_number(
                    &DAT_800583e8, (const MenuGlyphString *)&gs);
                counts++;
            }
            row++;
            yoff += 0xc;
        } while (row < list->visible_rows && row < list->entry_count);
    }

    tile = &menu_list_tiles[MENU_LIST_TILE_BACKDROP];
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
    primitive_buffer_commit_poly_ft4(2000);

    if (list->visible_rows != 0) {
        slot = 0;
        yoff = 0;
        do {
            tile = &menu_list_tiles[MENU_LIST_TILE_ROW];
            if (slot == list->cursor_row) {
                tile = &menu_list_tiles[MENU_LIST_TILE_SELECTED];
            }
            slot++;
            primitive_buffer_begin_poly_ft4();
            current_poly_ft4->tpage = tile->tpage;
            current_poly_ft4->clut = tile->clut;
            current_poly_ft4->x0 = list->list_x;
            current_poly_ft4->y0 = list->list_y + yoff + 3;
            current_poly_ft4->x1 = list->list_x + tile->width;
            current_poly_ft4->y1 = list->list_y + yoff + 3;
            current_poly_ft4->x2 = list->list_x;
            current_poly_ft4->y2 = list->list_y + yoff + tile->height + 3;
            current_poly_ft4->x3 = list->list_x + tile->width;
            current_poly_ft4->y3 = list->list_y + yoff + tile->height + 3;
            current_poly_ft4->u0 = tile->u;
            current_poly_ft4->v0 = tile->v;
            current_poly_ft4->u1 = tile->u + tile->width;
            current_poly_ft4->v1 = tile->v;
            current_poly_ft4->u2 = tile->u;
            current_poly_ft4->v2 = tile->v + tile->height;
            current_poly_ft4->u3 = tile->u + tile->width;
            current_poly_ft4->v3 = tile->v + tile->height;
            SetSemiTrans(current_poly_ft4, 1);
            primitive_buffer_commit_poly_ft4(2000);
            yoff += 0xc;
        } while ((s32)slot < list->visible_rows);
    }

    tile = &menu_list_tiles[MENU_LIST_TILE_END];
    primitive_buffer_begin_poly_ft4();
    current_poly_ft4->tpage = tile->tpage;
    current_poly_ft4->clut = tile->clut;
    current_poly_ft4->x0 = list->list_x;
    current_poly_ft4->y0 = list->list_y + list->visible_rows * 0xc + 3;
    current_poly_ft4->x1 = list->list_x + tile->width;
    current_poly_ft4->y1 = list->list_y + list->visible_rows * 0xc + 3;
    current_poly_ft4->x2 = list->list_x;
    current_poly_ft4->y2 = list->list_y + list->visible_rows * 0xc +
                           tile->height + 3;
    current_poly_ft4->x3 = list->list_x + tile->width;
    current_poly_ft4->y3 = list->list_y + list->visible_rows * 0xc +
                           tile->height + 3;
    current_poly_ft4->u0 = tile->u;
    current_poly_ft4->v0 = tile->v;
    current_poly_ft4->u1 = tile->u + tile->width;
    current_poly_ft4->v1 = tile->v;
    current_poly_ft4->u2 = tile->u;
    current_poly_ft4->v2 = tile->v + tile->height;
    current_poly_ft4->u3 = tile->u + tile->width;
    current_poly_ft4->v3 = tile->v + tile->height;
    SetSemiTrans(current_poly_ft4, 1);
    primitive_buffer_commit_poly_ft4(2000);

    menu_draw_window_backdrop();
}
