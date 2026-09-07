#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

enum {
    MENU_TRANSLUCENT_SPRITE_X_OFFSET = 4,
    MENU_TRANSLUCENT_SPRITE_Y_OFFSET = 3,
    MENU_OPAQUE_SPRITE_X_OFFSET = 18,
    MENU_OPAQUE_SPRITE_Y_OFFSET = 2,
    MENU_PRIMITIVE_BRIGHTNESS = 96,
    MENU_LIST_DEFAULT_VISIBLE_ROWS = 11,
    MENU_LIST_DEFAULT_GLYPHS_PER_ENTRY = 8,
    MENU_BACKDROP_LEFT_X = 166,
    MENU_BACKDROP_RIGHT_X = MENU_BACKDROP_LEFT_X + MENU_BACKDROP_COLUMN_STEP,
    MENU_PICKUP_BACKDROP_LEFT_X = 118,
    MENU_PICKUP_BACKDROP_RIGHT_X = MENU_PICKUP_BACKDROP_LEFT_X + MENU_BACKDROP_COLUMN_STEP
};

/* COM\\MIX.TIM font geometry; STAT.DAT descriptors confirm the cell extents. */
enum {
    MENU_FONT_COLUMNS = 16,
    MENU_FONT_CELL_WIDTH = 14,
    MENU_FONT_CELL_HEIGHT = 12,
    MENU_NUMBER_CELL_HEIGHT = 11,
    /* Kana marks occupy columns 14/15 of row 2 in the text atlas. */
    MENU_DAKUTEN_U = 14 * MENU_FONT_CELL_WIDTH,
    MENU_HANDAKUTEN_U = 15 * MENU_FONT_CELL_WIDTH,
    MENU_KANA_MARK_V = 2 * MENU_FONT_CELL_HEIGHT,
    MENU_TEXT_OT_DEPTH = 1000
};

typedef char menu_window_row_codes_offset[
    (u32)&((MenuWindowLayout *)0)->rows[0].codes == 0x1c ? 1 : -1];

/*
 * Contiguous GAME.EXE menu presentation/runtime run
 * 0x800291ec..0x8002b078. The original source boundary is WIP.
 */

/* The numbered-item TIM path template used by menu_load_item_texture. */
RODATA(0x80012350, 0xc)

DATA(0x80057e88, 0x4)
POLY_FT4 *current_poly_ft4 = 0;

/*
 * Draw a two-option selection widget: one window frame around the selected
 * option, an option box on each of the two points (the selected point getting
 * the highlight box when highlighting is active), and a label under each.
 */
ADDRESS(0x800291ec, 0x10c)
void menu_draw_two_option(
    const MenuGlyphString *option0, const MenuGlyphString *option1,
    KfMenuConfirmChoice selected, s32 highlight)
{
    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;
    if (selected == KF_MENU_CHOICE_ACCEPT) {
        menu_blit_sprite(&menu_assets.selection_cursor, (const MenuPoint *)option0);
    } else {
        menu_blit_sprite(&menu_assets.selection_cursor, (const MenuPoint *)option1);
    }
    if (highlight == 1) {
        if (selected == KF_MENU_CHOICE_ACCEPT) {
            menu_blit_sprite_translucent(
                &menu_assets.option_highlight, (const MenuPoint *)option0);
            menu_blit_sprite_translucent(
                &menu_assets.option_background, (const MenuPoint *)option1);
        } else {
            menu_blit_sprite_translucent(
                &menu_assets.option_background, (const MenuPoint *)option0);
            menu_blit_sprite_translucent(
                &menu_assets.option_highlight, (const MenuPoint *)option1);
        }
    } else {
        menu_blit_sprite_translucent(
            &menu_assets.option_background, (const MenuPoint *)option0);
        menu_blit_sprite_translucent(
            &menu_assets.option_background, (const MenuPoint *)option1);
    }
    menu_draw_string(&menu_assets.glyph_atlas, option0);
    menu_draw_string(&menu_assets.glyph_atlas, option1);
}

/*
 * Draw the item-pickup preview: a spinning 3D model, its ten-glyph name, and four
 * mirrored window-sprite quads. The shared spin angle advances eight units.
 */
ADDRESS(0x800292f8, 0x7b8)
void menu_draw_item_name_frame(s32 item_id)
{
    MenuGlyphString string;
    MATRIX rotation;
    MATRIX light_source;
    MATRIX light_result;
    s16 *name;
    s32 i;

    rotation.t[0] = 0xdc;
    rotation.t[1] = 0x8c;
    rotation.t[2] = 0x5dc;
    menu_item_preview_rotation.vy =
        (menu_item_preview_rotation.vy + MENU_PICKUP_PREVIEW_YAW_STEP)
        & KF_ANGLE_WRAP_MASK;
    RotMatrix(&menu_item_preview_rotation, &rotation);

    light_source.m[0][0] = -KF_FIXED12_ONE;
    light_source.m[0][1] = -KF_FIXED12_ONE;
    light_source.m[0][2] = -KF_FIXED12_ONE;
    light_source.m[1][0] = -KF_FIXED12_ONE;
    light_source.m[1][1] = -KF_FIXED12_ONE;
    light_source.m[1][2] = -KF_FIXED12_ONE;
    light_source.m[2][0] = 0;
    light_source.m[2][1] = 0;
    light_source.m[2][2] = 0;
    MulMatrix0(&light_source, &rotation, &light_result);
    SetLightMatrix(&light_result);
    SetRotMatrix(&rotation);
    SetTransMatrix(&rotation);
    menu_render_item_model();

    name = item_name_rows[item_id].codes;
    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;
    string.x = 0x80;
    string.y = 0x24;
    for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {
        string.codes[i] = name[i];
    }
    menu_draw_string(&menu_assets.glyph_atlas, &string);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    current_poly_ft4->x0 = MENU_PICKUP_BACKDROP_LEFT_X;
    current_poly_ft4->y0 = MENU_BACKDROP_TOP_Y;
    current_poly_ft4->y1 = MENU_BACKDROP_TOP_Y;
    current_poly_ft4->x2 = MENU_PICKUP_BACKDROP_LEFT_X;
    current_poly_ft4->x1 = menu_assets.window_backdrop.width + MENU_PICKUP_BACKDROP_LEFT_X;
    current_poly_ft4->y2 = menu_assets.window_backdrop.height + MENU_BACKDROP_TOP_Y;
    current_poly_ft4->x3 = menu_assets.window_backdrop.width + MENU_PICKUP_BACKDROP_LEFT_X;
    current_poly_ft4->y3 = menu_assets.window_backdrop.height + MENU_BACKDROP_TOP_Y;
    current_poly_ft4->u0 = menu_assets.window_backdrop.u;
    current_poly_ft4->v0 = menu_assets.window_backdrop.v;
    current_poly_ft4->u1 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v1 = menu_assets.window_backdrop.v;
    current_poly_ft4->u2 = menu_assets.window_backdrop.u;
    current_poly_ft4->v2 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u3 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v3 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    current_poly_ft4->x0 = MENU_PICKUP_BACKDROP_RIGHT_X;
    current_poly_ft4->y0 = MENU_BACKDROP_TOP_Y;
    current_poly_ft4->y1 = MENU_BACKDROP_TOP_Y;
    current_poly_ft4->x2 = MENU_PICKUP_BACKDROP_RIGHT_X;
    current_poly_ft4->x1 = menu_assets.window_backdrop.width + MENU_PICKUP_BACKDROP_RIGHT_X;
    current_poly_ft4->y2 = menu_assets.window_backdrop.height + MENU_BACKDROP_TOP_Y;
    current_poly_ft4->x3 = menu_assets.window_backdrop.width + MENU_PICKUP_BACKDROP_RIGHT_X;
    current_poly_ft4->y3 = menu_assets.window_backdrop.height + MENU_BACKDROP_TOP_Y;
    current_poly_ft4->u0 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v0 = menu_assets.window_backdrop.v;
    current_poly_ft4->u1 = menu_assets.window_backdrop.u;
    current_poly_ft4->v1 = menu_assets.window_backdrop.v;
    current_poly_ft4->u2 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v2 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u3 = menu_assets.window_backdrop.u;
    current_poly_ft4->v3 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    current_poly_ft4->x0 = MENU_PICKUP_BACKDROP_LEFT_X;
    current_poly_ft4->y0 = MENU_BACKDROP_BOTTOM_Y;
    current_poly_ft4->y1 = MENU_BACKDROP_BOTTOM_Y;
    current_poly_ft4->x2 = MENU_PICKUP_BACKDROP_LEFT_X;
    current_poly_ft4->x1 = menu_assets.window_backdrop.width + MENU_PICKUP_BACKDROP_LEFT_X;
    current_poly_ft4->y2 = menu_assets.window_backdrop.height + MENU_BACKDROP_BOTTOM_Y;
    current_poly_ft4->x3 = menu_assets.window_backdrop.width + MENU_PICKUP_BACKDROP_LEFT_X;
    current_poly_ft4->y3 = menu_assets.window_backdrop.height + MENU_BACKDROP_BOTTOM_Y;
    current_poly_ft4->u0 = menu_assets.window_backdrop.u;
    current_poly_ft4->v0 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u1 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v1 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u2 = menu_assets.window_backdrop.u;
    current_poly_ft4->v2 = menu_assets.window_backdrop.v;
    current_poly_ft4->u3 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v3 = menu_assets.window_backdrop.v;
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    current_poly_ft4->x0 = MENU_PICKUP_BACKDROP_RIGHT_X;
    current_poly_ft4->y0 = MENU_BACKDROP_BOTTOM_Y;
    current_poly_ft4->y1 = MENU_BACKDROP_BOTTOM_Y;
    current_poly_ft4->x2 = MENU_PICKUP_BACKDROP_RIGHT_X;
    current_poly_ft4->x1 = menu_assets.window_backdrop.width + MENU_PICKUP_BACKDROP_RIGHT_X;
    current_poly_ft4->y2 = menu_assets.window_backdrop.height + MENU_BACKDROP_BOTTOM_Y;
    current_poly_ft4->x3 = menu_assets.window_backdrop.width + MENU_PICKUP_BACKDROP_RIGHT_X;
    current_poly_ft4->y3 = menu_assets.window_backdrop.height + MENU_BACKDROP_BOTTOM_Y;
    current_poly_ft4->u0 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v0 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u1 = menu_assets.window_backdrop.u;
    current_poly_ft4->v1 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u2 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v2 = menu_assets.window_backdrop.v;
    current_poly_ft4->u3 = menu_assets.window_backdrop.u;
    current_poly_ft4->v3 = menu_assets.window_backdrop.v;
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    AddPrim(
        display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[display_state.buffer_index][3]);
    AddPrim(
        display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[display_state.buffer_index][2]);
    AddPrim(
        display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[display_state.buffer_index][1]);
    AddPrim(
        display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[display_state.buffer_index][0]);
}

/*
 * Build a translucent textured quad four pixels left and three above a menu point, and
 * link it at ordering-table depth 2000.
 */
ADDRESS(0x80029ab0, 0x1a0)
void menu_blit_sprite_translucent(
    const MenuSpriteDef *sprite, const MenuPoint *position)
{
    primitive_buffer_begin_poly_ft4();
    current_poly_ft4->tpage = sprite->tpage;
    current_poly_ft4->clut = sprite->clut;
    setXYWH(current_poly_ft4, position->x - MENU_TRANSLUCENT_SPRITE_X_OFFSET, position->y - MENU_TRANSLUCENT_SPRITE_Y_OFFSET,
        sprite->width, sprite->height);
    current_poly_ft4->u0 = sprite->u;
    current_poly_ft4->v0 = sprite->v;
    current_poly_ft4->u1 = sprite->u + sprite->width;
    current_poly_ft4->v1 = sprite->v;
    current_poly_ft4->u2 = sprite->u;
    current_poly_ft4->v2 = sprite->v + sprite->height;
    current_poly_ft4->u3 = sprite->u + sprite->width;
    current_poly_ft4->v3 = sprite->v + sprite->height;
    SetSemiTrans(current_poly_ft4, 1);
    primitive_buffer_commit_poly_ft4(MENU_WIDGET_OT_DEPTH);
}

/*
 * Build an opaque textured quad eighteen pixels left and two above a menu point, and link
 * it at ordering-table depth 2000.
 */
ADDRESS(0x80029c50, 0x190)
void menu_blit_sprite(
    const MenuSpriteDef *sprite, const MenuPoint *position)
{
    primitive_buffer_begin_poly_ft4();
    current_poly_ft4->tpage = sprite->tpage;
    current_poly_ft4->clut = sprite->clut;
    setXYWH(current_poly_ft4, position->x - MENU_OPAQUE_SPRITE_X_OFFSET, position->y - MENU_OPAQUE_SPRITE_Y_OFFSET,
        sprite->width, sprite->height);
    current_poly_ft4->u0 = sprite->u;
    current_poly_ft4->v0 = sprite->v;
    current_poly_ft4->u1 = sprite->u + sprite->width;
    current_poly_ft4->v1 = sprite->v;
    current_poly_ft4->u2 = sprite->u;
    current_poly_ft4->v2 = sprite->v + sprite->height;
    current_poly_ft4->u3 = sprite->u + sprite->width;
    current_poly_ft4->v3 = sprite->v + sprite->height;
    primitive_buffer_commit_poly_ft4(MENU_WIDGET_OT_DEPTH);
}

/*
 * Render a positioned glyph string. The low 12 code bits select a 14x12 atlas
 * cell; bits 0x1000 and 0x2000 overlay the two decoration cells. Each emitted
 * quad links at ordering-table depth 1000.
 */
ADDRESS(0x80029de0, 0x530)
void menu_draw_string(
    const MenuSpriteDef *font, const MenuGlyphString *string)
{
    s32 i;
    s32 x_offset;

    for (i = 0; string->codes[i] != MENU_TEXT_END; i++) {
        s32 glyph;

        x_offset = i * MENU_FONT_CELL_WIDTH;
        primitive_buffer_begin_poly_ft4();
        current_poly_ft4->tpage = font->tpage;
        current_poly_ft4->clut = font->clut;
        current_poly_ft4->x0 = string->x + x_offset;
        current_poly_ft4->y0 = string->y;
        current_poly_ft4->x1 = string->x + x_offset + font->width;
        current_poly_ft4->y1 = string->y;
        current_poly_ft4->x2 = string->x + x_offset;
        current_poly_ft4->y2 = string->y + font->height;
        current_poly_ft4->x3 = string->x + x_offset + font->width;
        current_poly_ft4->y3 = string->y + font->height;
        glyph = string->codes[i] & MENU_TEXT_GLYPH_MASK;
        current_poly_ft4->u0 = (glyph % MENU_FONT_COLUMNS) * MENU_FONT_CELL_WIDTH;
        current_poly_ft4->v0 = (glyph / MENU_FONT_COLUMNS) * MENU_FONT_CELL_HEIGHT;
        current_poly_ft4->u1 = (glyph % MENU_FONT_COLUMNS) * MENU_FONT_CELL_WIDTH + font->width;
        current_poly_ft4->v1 = (glyph / MENU_FONT_COLUMNS) * MENU_FONT_CELL_HEIGHT;
        current_poly_ft4->u2 = (glyph % MENU_FONT_COLUMNS) * MENU_FONT_CELL_WIDTH;
        current_poly_ft4->v2 = (glyph / MENU_FONT_COLUMNS) * MENU_FONT_CELL_HEIGHT + font->height;
        current_poly_ft4->u3 = (glyph % MENU_FONT_COLUMNS) * MENU_FONT_CELL_WIDTH + font->width;
        current_poly_ft4->v3 = (glyph / MENU_FONT_COLUMNS) * MENU_FONT_CELL_HEIGHT + font->height;
        primitive_buffer_commit_poly_ft4(MENU_TEXT_OT_DEPTH);

        if (string->codes[i] & MENU_TEXT_DAKUTEN) {
            primitive_buffer_begin_poly_ft4();
            current_poly_ft4->tpage = font->tpage;
            current_poly_ft4->clut = font->clut;
            current_poly_ft4->x0 = string->x + x_offset;
            current_poly_ft4->y0 = string->y;
            current_poly_ft4->x1 = string->x + x_offset + font->width;
            current_poly_ft4->y1 = string->y;
            current_poly_ft4->x2 = string->x + x_offset;
            current_poly_ft4->y2 = string->y + font->height;
            current_poly_ft4->x3 = string->x + x_offset + font->width;
            current_poly_ft4->y3 = string->y + font->height;
            current_poly_ft4->u0 = MENU_DAKUTEN_U;
            current_poly_ft4->v0 = MENU_KANA_MARK_V;
            current_poly_ft4->u1 = MENU_DAKUTEN_U + font->width;
            current_poly_ft4->v1 = MENU_KANA_MARK_V;
            current_poly_ft4->u2 = MENU_DAKUTEN_U;
            current_poly_ft4->v2 = MENU_KANA_MARK_V + font->height;
            current_poly_ft4->u3 = MENU_DAKUTEN_U + font->width;
            current_poly_ft4->v3 = MENU_KANA_MARK_V + font->height;
            primitive_buffer_commit_poly_ft4(MENU_TEXT_OT_DEPTH);
        }

        if (string->codes[i] & MENU_TEXT_HANDAKUTEN) {
            primitive_buffer_begin_poly_ft4();
            current_poly_ft4->tpage = font->tpage;
            current_poly_ft4->clut = font->clut;
            current_poly_ft4->x0 = string->x + x_offset;
            current_poly_ft4->y0 = string->y;
            current_poly_ft4->x1 = string->x + x_offset + font->width;
            current_poly_ft4->y1 = string->y;
            current_poly_ft4->x2 = string->x + x_offset;
            current_poly_ft4->y2 = string->y + font->height;
            current_poly_ft4->x3 = string->x + x_offset + font->width;
            current_poly_ft4->y3 = string->y + font->height;
            current_poly_ft4->u0 = MENU_HANDAKUTEN_U;
            current_poly_ft4->v0 = MENU_KANA_MARK_V;
            current_poly_ft4->u1 = MENU_HANDAKUTEN_U + font->width;
            current_poly_ft4->v1 = MENU_KANA_MARK_V;
            current_poly_ft4->u2 = MENU_HANDAKUTEN_U;
            current_poly_ft4->v2 = MENU_KANA_MARK_V + font->height;
            current_poly_ft4->u3 = MENU_HANDAKUTEN_U + font->width;
            current_poly_ft4->v3 = MENU_KANA_MARK_V + font->height;
            primitive_buffer_commit_poly_ft4(MENU_TEXT_OT_DEPTH);
        }
    }
}

/*
 * Render a fixed-pitch digit run from a single-column atlas. Each signed code
 * selects an eleven-texel row, screen positions advance seven pixels, and all
 * quads link at ordering-table depth 1000.
 */
ADDRESS(0x8002a310, 0x200)
void menu_draw_number(
    const MenuSpriteDef *atlas, const MenuGlyphString *string)
{
    const MenuSpriteDef *font = atlas;
    const MenuGlyphString *label = string;
    s32 i;
    s32 xoff;

    for (i = 0; label->codes[i] != MENU_TEXT_END; i++) {
        xoff = i * MENU_NUMBER_ADVANCE;
        primitive_buffer_begin_poly_ft4();
        current_poly_ft4->tpage = font->tpage;
        current_poly_ft4->clut = font->clut;
        current_poly_ft4->x0 = label->x + xoff;
        current_poly_ft4->y0 = label->y;
        current_poly_ft4->x1 = label->x + xoff + font->width;
        current_poly_ft4->y1 = label->y;
        current_poly_ft4->x2 = label->x + xoff;
        current_poly_ft4->y2 = label->y + font->height;
        current_poly_ft4->x3 = label->x + xoff + font->width;
        current_poly_ft4->y3 = label->y + font->height;
        current_poly_ft4->u0 = font->u;
        current_poly_ft4->v0 = label->codes[i] * MENU_NUMBER_CELL_HEIGHT;
        current_poly_ft4->u1 = font->u + font->width;
        current_poly_ft4->v1 = label->codes[i] * MENU_NUMBER_CELL_HEIGHT;
        current_poly_ft4->u2 = font->u;
        current_poly_ft4->v2 = label->codes[i] * MENU_NUMBER_CELL_HEIGHT + font->height;
        current_poly_ft4->u3 = font->u + font->width;
        current_poly_ft4->v3 = label->codes[i] * MENU_NUMBER_CELL_HEIGHT + font->height;
        primitive_buffer_commit_poly_ft4(MENU_TEXT_OT_DEPTH);
    }
}

/*
 * Draw the shared translucent menu-window backdrop.  Four semi-transparent
 * textured tiles built in the primitive workspace cover the window interior in
 * a 2x2 grid at (166,16), (237,16), (166,120), (237,120) -- all linked at
 * ordering-table depth 2900 -- then the four persistent border quads are
 * enqueued at depth 3000.  Shared by the status panel, the option-window
 * renderer and the scrollable list widget.
 */
ADDRESS(0x8002a510, 0x6a4)
void menu_draw_window_backdrop(void)
{
    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    current_poly_ft4->x0 = MENU_BACKDROP_LEFT_X;
    current_poly_ft4->y0 = MENU_BACKDROP_TOP_Y;
    current_poly_ft4->x1 = menu_assets.window_backdrop.width + MENU_BACKDROP_LEFT_X;
    current_poly_ft4->y1 = MENU_BACKDROP_TOP_Y;
    current_poly_ft4->x2 = MENU_BACKDROP_LEFT_X;
    current_poly_ft4->y2 = menu_assets.window_backdrop.height + MENU_BACKDROP_TOP_Y;
    current_poly_ft4->x3 = menu_assets.window_backdrop.width + MENU_BACKDROP_LEFT_X;
    current_poly_ft4->y3 = menu_assets.window_backdrop.height + MENU_BACKDROP_TOP_Y;
    current_poly_ft4->u0 = menu_assets.window_backdrop.u;
    current_poly_ft4->v0 = menu_assets.window_backdrop.v;
    current_poly_ft4->u1 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v1 = menu_assets.window_backdrop.v;
    current_poly_ft4->u2 = menu_assets.window_backdrop.u;
    current_poly_ft4->v2 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u3 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v3 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    current_poly_ft4->x0 = MENU_BACKDROP_RIGHT_X;
    current_poly_ft4->y0 = MENU_BACKDROP_TOP_Y;
    current_poly_ft4->x1 = menu_assets.window_backdrop.width + MENU_BACKDROP_RIGHT_X;
    current_poly_ft4->y1 = MENU_BACKDROP_TOP_Y;
    current_poly_ft4->x2 = MENU_BACKDROP_RIGHT_X;
    current_poly_ft4->y2 = menu_assets.window_backdrop.height + MENU_BACKDROP_TOP_Y;
    current_poly_ft4->x3 = menu_assets.window_backdrop.width + MENU_BACKDROP_RIGHT_X;
    current_poly_ft4->y3 = menu_assets.window_backdrop.height + MENU_BACKDROP_TOP_Y;
    current_poly_ft4->u0 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v0 = menu_assets.window_backdrop.v;
    current_poly_ft4->u1 = menu_assets.window_backdrop.u;
    current_poly_ft4->v1 = menu_assets.window_backdrop.v;
    current_poly_ft4->u2 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v2 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u3 = menu_assets.window_backdrop.u;
    current_poly_ft4->v3 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    current_poly_ft4->x0 = MENU_BACKDROP_LEFT_X;
    current_poly_ft4->y0 = MENU_BACKDROP_BOTTOM_Y;
    current_poly_ft4->x1 = menu_assets.window_backdrop.width + MENU_BACKDROP_LEFT_X;
    current_poly_ft4->y1 = MENU_BACKDROP_BOTTOM_Y;
    current_poly_ft4->x2 = MENU_BACKDROP_LEFT_X;
    current_poly_ft4->y2 = menu_assets.window_backdrop.height + MENU_BACKDROP_BOTTOM_Y;
    current_poly_ft4->x3 = menu_assets.window_backdrop.width + MENU_BACKDROP_LEFT_X;
    current_poly_ft4->y3 = menu_assets.window_backdrop.height + MENU_BACKDROP_BOTTOM_Y;
    current_poly_ft4->u0 = menu_assets.window_backdrop.u;
    current_poly_ft4->v0 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u1 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v1 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u2 = menu_assets.window_backdrop.u;
    current_poly_ft4->v2 = menu_assets.window_backdrop.v;
    current_poly_ft4->u3 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v3 = menu_assets.window_backdrop.v;
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    current_poly_ft4->x0 = MENU_BACKDROP_RIGHT_X;
    current_poly_ft4->y0 = MENU_BACKDROP_BOTTOM_Y;
    current_poly_ft4->x1 = menu_assets.window_backdrop.width + MENU_BACKDROP_RIGHT_X;
    current_poly_ft4->y1 = MENU_BACKDROP_BOTTOM_Y;
    current_poly_ft4->x2 = MENU_BACKDROP_RIGHT_X;
    current_poly_ft4->y2 = menu_assets.window_backdrop.height + MENU_BACKDROP_BOTTOM_Y;
    current_poly_ft4->x3 = menu_assets.window_backdrop.width + MENU_BACKDROP_RIGHT_X;
    current_poly_ft4->y3 = menu_assets.window_backdrop.height + MENU_BACKDROP_BOTTOM_Y;
    current_poly_ft4->u0 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v0 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u1 = menu_assets.window_backdrop.u;
    current_poly_ft4->v1 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u2 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v2 = menu_assets.window_backdrop.v;
    current_poly_ft4->u3 = menu_assets.window_backdrop.u;
    current_poly_ft4->v3 = menu_assets.window_backdrop.v;
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    AddPrim(
        display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[display_state.buffer_index][3]);
    AddPrim(
        display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[display_state.buffer_index][2]);
    AddPrim(
        display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[display_state.buffer_index][1]);
    AddPrim(
        display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[display_state.buffer_index][0]);
}

/*
 * Begin a menu frame: flip the double buffer, re-anchor the active primitive
 * buffer and ordering table for the new buffer index, clear the ordering
 * table, and rewind the primitive-buffer cursor.
 */
ADDRESS(0x8002abb4, 0x80)
void menu_frame_begin(void)
{
    display_state.buffer_index = display_state.buffer_index == 0;
    display_state.primitive_buffer =
        &display_state.primitive_buffers[display_state.buffer_index];
    display_state.ordering_table =
        display_state.ordering_tables[display_state.buffer_index].entries;
    ClearOTagR(display_state.ordering_table, KF_ORDERING_TABLE_LENGTH);
    display_state.primitive_buffer->cursor =
        display_state.primitive_buffer->start;
}

/* Wait for the GPU/retrace, install the active environments, and submit OT. */
ADDRESS(0x8002ac34, 0x98)
void menu_present_frame(void)
{
    DrawSync(0);
    VSync(0);
    PutDrawEnv(&display_draw_environments[display_state.buffer_index]);
    PutDispEnv(&display_disp_environments[display_state.buffer_index]);
    DrawOTag(display_state.ordering_table + (KF_ORDERING_TABLE_LENGTH - 1));
}

ADDRESS(0x8002accc, 0x50)
void primitive_buffer_begin_poly_ft4(void)
{
    SetPolyFT4(current_poly_ft4);
    current_poly_ft4->r0 = MENU_PRIMITIVE_BRIGHTNESS;
    current_poly_ft4->g0 = MENU_PRIMITIVE_BRIGHTNESS;
    current_poly_ft4->b0 = MENU_PRIMITIVE_BRIGHTNESS;
}

ADDRESS(0x8002ad1c, 0x50)
void primitive_buffer_commit_poly_ft4(s32 depth)
{
    depth <<= 2;
    AddPrim(
        (u32 *)((u8 *)display_state.ordering_table + depth),
        current_poly_ft4);
    current_poly_ft4++;
    display_state.primitive_buffer->cursor = (u8 *)current_poly_ft4;
}

/* Initialize a menu list header and copy its label glyphs from the table. */
ADDRESS(0x8002ad6c, 0x8c)
void menu_list_init(KfMenuList *list, KfMenuWindowKind kind, s32 row)
{
    s32 i;

    list->title_x = 12;
    list->title_y = 19;
    for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {
        list->title_glyphs[i] = menu_window_layouts[KF_ENUM_ENCODE(s32, kind)].rows[row].codes[i];
    }
    list->list_x = 0x16;
    list->list_y = 0x26;
    list->entry_count = 0;
    list->visible_rows = MENU_LIST_DEFAULT_VISIBLE_ROWS;
    list->scroll_offset = 0;
    list->selected_index = 0;
    list->cursor_row = 0;
    list->glyphs_per_entry = MENU_LIST_DEFAULT_GLYPHS_PER_ENTRY;
}

/* Expand a signed value into decimal glyphs and a trailing -1 sentinel. */
ADDRESS(0x8002adf8, 0xac)
void menu_format_number(s32 value, s32 count, s32 pad_zero, s16 *out)
{
    s32 i = 0;
    s32 blank;

    blank = (pad_zero == 0) ? MENU_NUMBER_BLANK : 0;
    for (; i < count; i++) {
        out[i] = blank;
    }
    out[count] = MENU_TEXT_END;
    for (i = count - 1; i >= 0; i--) {
        out[i] = value % 10;
        value /= 10;
        if (value == 0) {
            i = -1;
        }
    }
}

/* Release the previous item model, then load and register the selection. */
ADDRESS(0x8002aea4, 0x68)
u32 menu_load_item_model(s32 id)
{
    void *asset;

    menu_release_item_model();
    if (id != KF_ITEM_NONE) {
        if (cd_file_load_table_entry(&asset, id) != 0) {
            return 1;
        }
        tmd_register(KF_TMD_SLOT_MENU_ITEM, asset);
        menu_item_model_allocation_pending = 1;
    }
    menu_item_preview_rotation.vy = 0;
    return 0;
}

ADDRESS(0x8002af0c, 0x3c)
void menu_release_item_model(void)
{
    if (menu_item_model_allocation_pending == 1) {
        tmd_release_last_allocation(KF_TMD_SLOT_MENU_ITEM);
        menu_item_model_allocation_pending = 0;
    }
}

/* Load the numbered item TIM into the active primitive buffer and VRAM. */
ADDRESS(0x8002af48, 0x130)
u32 menu_load_item_texture(s32 id)
{
    char name[16] = "TIM\\M000.";
    void *destination;
    s32 number;
    s32 remainder;

    if (id != KF_MENU_TEXTURE_NONE) {
        number = id + 1;
        name[5] = number / 100 + '0';
        remainder = number % 100;
        name[6] = remainder / 10 + '0';
        name[7] = remainder % 10 + '0';
        destination = display_state.primitive_buffer->cursor;
        if (cd_file_load_into(destination, name) != 0) {
            return 1;
        }
        tim_upload_images((u_long *)destination);
    }
    return 0;
}
