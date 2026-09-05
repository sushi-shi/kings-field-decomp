#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

typedef char menu_glyph_string_size[sizeof(MenuGlyphString) == 0x18 ? 1 : -1];
typedef char menu_window_layout_size[sizeof(MenuWindowLayout) == 0x108 ? 1 : -1];
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
    s32 selected, s32 highlight)
{
    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;
    if (selected == 0) {
        menu_blit_sprite(&menu_assets.selection_cursor, (const MenuPoint *)option0);
    } else {
        menu_blit_sprite(&menu_assets.selection_cursor, (const MenuPoint *)option1);
    }
    if (highlight == 1) {
        if (selected == 0) {
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
 * Draw the item-use preview: a spinning 3D model, its ten-glyph name, and four
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
    DAT_80057b72 = (DAT_80057b72 + 8) & 0xfff;
    RotMatrix((SVECTOR *)((s16 *)&DAT_80057b72 - 1), &rotation);

    light_source.m[0][0] = -4096;
    light_source.m[0][1] = -4096;
    light_source.m[0][2] = -4096;
    light_source.m[1][0] = -4096;
    light_source.m[1][1] = -4096;
    light_source.m[1][2] = -4096;
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
    for (i = 0; i < 10; i++) {
        string.codes[i] = name[i];
    }
    menu_draw_string(&menu_assets.glyph_atlas, &string);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    current_poly_ft4->x0 = 0x76;
    current_poly_ft4->y0 = 0x10;
    current_poly_ft4->y1 = 0x10;
    current_poly_ft4->x2 = 0x76;
    current_poly_ft4->x1 = menu_assets.window_backdrop.width + 0x76;
    current_poly_ft4->y2 = menu_assets.window_backdrop.height + 0x10;
    current_poly_ft4->x3 = menu_assets.window_backdrop.width + 0x76;
    current_poly_ft4->y3 = menu_assets.window_backdrop.height + 0x10;
    current_poly_ft4->u0 = menu_assets.window_backdrop.u;
    current_poly_ft4->v0 = menu_assets.window_backdrop.v;
    current_poly_ft4->u1 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v1 = menu_assets.window_backdrop.v;
    current_poly_ft4->u2 = menu_assets.window_backdrop.u;
    current_poly_ft4->v2 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u3 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v3 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    primitive_buffer_commit_poly_ft4(0xb54);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    current_poly_ft4->x0 = 0xbd;
    current_poly_ft4->y0 = 0x10;
    current_poly_ft4->y1 = 0x10;
    current_poly_ft4->x2 = 0xbd;
    current_poly_ft4->x1 = menu_assets.window_backdrop.width + 0xbd;
    current_poly_ft4->y2 = menu_assets.window_backdrop.height + 0x10;
    current_poly_ft4->x3 = menu_assets.window_backdrop.width + 0xbd;
    current_poly_ft4->y3 = menu_assets.window_backdrop.height + 0x10;
    current_poly_ft4->u0 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v0 = menu_assets.window_backdrop.v;
    current_poly_ft4->u1 = menu_assets.window_backdrop.u;
    current_poly_ft4->v1 = menu_assets.window_backdrop.v;
    current_poly_ft4->u2 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v2 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u3 = menu_assets.window_backdrop.u;
    current_poly_ft4->v3 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    primitive_buffer_commit_poly_ft4(0xb54);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    current_poly_ft4->x0 = 0x76;
    current_poly_ft4->y0 = 0x78;
    current_poly_ft4->y1 = 0x78;
    current_poly_ft4->x2 = 0x76;
    current_poly_ft4->x1 = menu_assets.window_backdrop.width + 0x76;
    current_poly_ft4->y2 = menu_assets.window_backdrop.height + 0x78;
    current_poly_ft4->x3 = menu_assets.window_backdrop.width + 0x76;
    current_poly_ft4->y3 = menu_assets.window_backdrop.height + 0x78;
    current_poly_ft4->u0 = menu_assets.window_backdrop.u;
    current_poly_ft4->v0 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u1 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v1 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u2 = menu_assets.window_backdrop.u;
    current_poly_ft4->v2 = menu_assets.window_backdrop.v;
    current_poly_ft4->u3 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v3 = menu_assets.window_backdrop.v;
    primitive_buffer_commit_poly_ft4(0xb54);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    current_poly_ft4->x0 = 0xbd;
    current_poly_ft4->y0 = 0x78;
    current_poly_ft4->y1 = 0x78;
    current_poly_ft4->x2 = 0xbd;
    current_poly_ft4->x1 = menu_assets.window_backdrop.width + 0xbd;
    current_poly_ft4->y2 = menu_assets.window_backdrop.height + 0x78;
    current_poly_ft4->x3 = menu_assets.window_backdrop.width + 0xbd;
    current_poly_ft4->y3 = menu_assets.window_backdrop.height + 0x78;
    current_poly_ft4->u0 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v0 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u1 = menu_assets.window_backdrop.u;
    current_poly_ft4->v1 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u2 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v2 = menu_assets.window_backdrop.v;
    current_poly_ft4->u3 = menu_assets.window_backdrop.u;
    current_poly_ft4->v3 = menu_assets.window_backdrop.v;
    primitive_buffer_commit_poly_ft4(0xb54);

    AddPrim(
        display_state.ordering_table + 3000,
        &menu_assets.background_quads[display_state.buffer_index][3]);
    AddPrim(
        display_state.ordering_table + 3000,
        &menu_assets.background_quads[display_state.buffer_index][2]);
    AddPrim(
        display_state.ordering_table + 3000,
        &menu_assets.background_quads[display_state.buffer_index][1]);
    AddPrim(
        display_state.ordering_table + 3000,
        &menu_assets.background_quads[display_state.buffer_index][0]);
}

/*
 * Build a translucent textured quad at a menu point, inset by (4, 3), and
 * link it at ordering-table depth 2000.
 */
ADDRESS(0x80029ab0, 0x1a0)
void menu_blit_sprite_translucent(
    const MenuSpriteDef *sprite, const MenuPoint *position)
{
    primitive_buffer_begin_poly_ft4();
    current_poly_ft4->tpage = sprite->tpage;
    current_poly_ft4->clut = sprite->clut;
    current_poly_ft4->x0 = position->x - 4;
    current_poly_ft4->y0 = position->y - 3;
    current_poly_ft4->x1 = position->x + (sprite->width - 4);
    current_poly_ft4->y1 = position->y - 3;
    current_poly_ft4->x2 = position->x - 4;
    current_poly_ft4->y2 = position->y + (sprite->height - 3);
    current_poly_ft4->x3 = position->x + (sprite->width - 4);
    current_poly_ft4->y3 = position->y + (sprite->height - 3);
    current_poly_ft4->u0 = sprite->u;
    current_poly_ft4->v0 = sprite->v;
    current_poly_ft4->u1 = sprite->u + sprite->width;
    current_poly_ft4->v1 = sprite->v;
    current_poly_ft4->u2 = sprite->u;
    current_poly_ft4->v2 = sprite->v + sprite->height;
    current_poly_ft4->u3 = sprite->u + sprite->width;
    current_poly_ft4->v3 = sprite->v + sprite->height;
    SetSemiTrans(current_poly_ft4, 1);
    primitive_buffer_commit_poly_ft4(2000);
}

/*
 * Build an opaque textured quad at a menu point, inset by (18, 2), and link
 * it at ordering-table depth 2000.
 */
ADDRESS(0x80029c50, 0x190)
void menu_blit_sprite(
    const MenuSpriteDef *sprite, const MenuPoint *position)
{
    primitive_buffer_begin_poly_ft4();
    current_poly_ft4->tpage = sprite->tpage;
    current_poly_ft4->clut = sprite->clut;
    current_poly_ft4->x0 = position->x - 18;
    current_poly_ft4->y0 = position->y - 2;
    current_poly_ft4->x1 = position->x + (sprite->width - 18);
    current_poly_ft4->y1 = position->y - 2;
    current_poly_ft4->x2 = position->x - 18;
    current_poly_ft4->y2 = position->y + (sprite->height - 2);
    current_poly_ft4->x3 = position->x + (sprite->width - 18);
    current_poly_ft4->y3 = position->y + (sprite->height - 2);
    current_poly_ft4->u0 = sprite->u;
    current_poly_ft4->v0 = sprite->v;
    current_poly_ft4->u1 = sprite->u + sprite->width;
    current_poly_ft4->v1 = sprite->v;
    current_poly_ft4->u2 = sprite->u;
    current_poly_ft4->v2 = sprite->v + sprite->height;
    current_poly_ft4->u3 = sprite->u + sprite->width;
    current_poly_ft4->v3 = sprite->v + sprite->height;
    primitive_buffer_commit_poly_ft4(2000);
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

    for (i = 0; string->codes[i] != -1; i++) {
        s32 glyph;

        x_offset = i * 14;
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
        glyph = string->codes[i] & 0xfff;
        current_poly_ft4->u0 = (glyph % 16) * 14;
        current_poly_ft4->v0 = (glyph / 16) * 12;
        current_poly_ft4->u1 = (glyph % 16) * 14 + font->width;
        current_poly_ft4->v1 = (glyph / 16) * 12;
        current_poly_ft4->u2 = (glyph % 16) * 14;
        current_poly_ft4->v2 = (glyph / 16) * 12 + font->height;
        current_poly_ft4->u3 = (glyph % 16) * 14 + font->width;
        current_poly_ft4->v3 = (glyph / 16) * 12 + font->height;
        primitive_buffer_commit_poly_ft4(1000);

        if (string->codes[i] & 0x1000) {
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
            current_poly_ft4->u0 = 196;
            current_poly_ft4->v0 = 24;
            current_poly_ft4->u1 = 196 + font->width;
            current_poly_ft4->v1 = 24;
            current_poly_ft4->u2 = 196;
            current_poly_ft4->v2 = 24 + font->height;
            current_poly_ft4->u3 = 196 + font->width;
            current_poly_ft4->v3 = 24 + font->height;
            primitive_buffer_commit_poly_ft4(1000);
        }

        if (string->codes[i] & 0x2000) {
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
            current_poly_ft4->u0 = 210;
            current_poly_ft4->v0 = 24;
            current_poly_ft4->u1 = 210 + font->width;
            current_poly_ft4->v1 = 24;
            current_poly_ft4->u2 = 210;
            current_poly_ft4->v2 = 24 + font->height;
            current_poly_ft4->u3 = 210 + font->width;
            current_poly_ft4->v3 = 24 + font->height;
            primitive_buffer_commit_poly_ft4(1000);
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

    for (i = 0; label->codes[i] != -1; i++) {
        xoff = i * 7;
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
        current_poly_ft4->v0 = label->codes[i] * 11;
        current_poly_ft4->u1 = font->u + font->width;
        current_poly_ft4->v1 = label->codes[i] * 11;
        current_poly_ft4->u2 = font->u;
        current_poly_ft4->v2 = label->codes[i] * 11 + font->height;
        current_poly_ft4->u3 = font->u + font->width;
        current_poly_ft4->v3 = label->codes[i] * 11 + font->height;
        primitive_buffer_commit_poly_ft4(1000);
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
    current_poly_ft4->x0 = 166;
    current_poly_ft4->y0 = 16;
    current_poly_ft4->x1 = menu_assets.window_backdrop.width + 166;
    current_poly_ft4->y1 = 16;
    current_poly_ft4->x2 = 166;
    current_poly_ft4->y2 = menu_assets.window_backdrop.height + 16;
    current_poly_ft4->x3 = menu_assets.window_backdrop.width + 166;
    current_poly_ft4->y3 = menu_assets.window_backdrop.height + 16;
    current_poly_ft4->u0 = menu_assets.window_backdrop.u;
    current_poly_ft4->v0 = menu_assets.window_backdrop.v;
    current_poly_ft4->u1 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v1 = menu_assets.window_backdrop.v;
    current_poly_ft4->u2 = menu_assets.window_backdrop.u;
    current_poly_ft4->v2 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u3 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v3 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    primitive_buffer_commit_poly_ft4(2900);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    current_poly_ft4->x0 = 237;
    current_poly_ft4->y0 = 16;
    current_poly_ft4->x1 = menu_assets.window_backdrop.width + 237;
    current_poly_ft4->y1 = 16;
    current_poly_ft4->x2 = 237;
    current_poly_ft4->y2 = menu_assets.window_backdrop.height + 16;
    current_poly_ft4->x3 = menu_assets.window_backdrop.width + 237;
    current_poly_ft4->y3 = menu_assets.window_backdrop.height + 16;
    current_poly_ft4->u0 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v0 = menu_assets.window_backdrop.v;
    current_poly_ft4->u1 = menu_assets.window_backdrop.u;
    current_poly_ft4->v1 = menu_assets.window_backdrop.v;
    current_poly_ft4->u2 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v2 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u3 = menu_assets.window_backdrop.u;
    current_poly_ft4->v3 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    primitive_buffer_commit_poly_ft4(2900);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    current_poly_ft4->x0 = 166;
    current_poly_ft4->y0 = 120;
    current_poly_ft4->x1 = menu_assets.window_backdrop.width + 166;
    current_poly_ft4->y1 = 120;
    current_poly_ft4->x2 = 166;
    current_poly_ft4->y2 = menu_assets.window_backdrop.height + 120;
    current_poly_ft4->x3 = menu_assets.window_backdrop.width + 166;
    current_poly_ft4->y3 = menu_assets.window_backdrop.height + 120;
    current_poly_ft4->u0 = menu_assets.window_backdrop.u;
    current_poly_ft4->v0 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u1 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v1 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u2 = menu_assets.window_backdrop.u;
    current_poly_ft4->v2 = menu_assets.window_backdrop.v;
    current_poly_ft4->u3 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v3 = menu_assets.window_backdrop.v;
    primitive_buffer_commit_poly_ft4(2900);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    current_poly_ft4->x0 = 237;
    current_poly_ft4->y0 = 120;
    current_poly_ft4->x1 = menu_assets.window_backdrop.width + 237;
    current_poly_ft4->y1 = 120;
    current_poly_ft4->x2 = 237;
    current_poly_ft4->y2 = menu_assets.window_backdrop.height + 120;
    current_poly_ft4->x3 = menu_assets.window_backdrop.width + 237;
    current_poly_ft4->y3 = menu_assets.window_backdrop.height + 120;
    current_poly_ft4->u0 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v0 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u1 = menu_assets.window_backdrop.u;
    current_poly_ft4->v1 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
    current_poly_ft4->u2 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
    current_poly_ft4->v2 = menu_assets.window_backdrop.v;
    current_poly_ft4->u3 = menu_assets.window_backdrop.u;
    current_poly_ft4->v3 = menu_assets.window_backdrop.v;
    primitive_buffer_commit_poly_ft4(2900);

    AddPrim(
        display_state.ordering_table + 3000,
        &menu_assets.background_quads[display_state.buffer_index][3]);
    AddPrim(
        display_state.ordering_table + 3000,
        &menu_assets.background_quads[display_state.buffer_index][2]);
    AddPrim(
        display_state.ordering_table + 3000,
        &menu_assets.background_quads[display_state.buffer_index][1]);
    AddPrim(
        display_state.ordering_table + 3000,
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
    ClearOTagR(display_state.ordering_table, 0x4000);
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
    DrawOTag(display_state.ordering_table + 0x3fff);
}

ADDRESS(0x8002accc, 0x50)
void primitive_buffer_begin_poly_ft4(void)
{
    SetPolyFT4(current_poly_ft4);
    current_poly_ft4->r0 = 0x60;
    current_poly_ft4->g0 = 0x60;
    current_poly_ft4->b0 = 0x60;
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
void menu_list_init(KfMenuList *list, s32 row, s32 column)
{
    s32 i;

    list->title_x = 12;
    list->title_y = 19;
    for (i = 0; i < 10; i++) {
        list->title_glyphs[i] = menu_window_layouts[row].rows[column].codes[i];
    }
    list->list_x = 0x16;
    list->list_y = 0x26;
    list->entry_count = 0;
    list->visible_rows = 0x0b;
    list->scroll_offset = 0;
    list->selected_index = 0;
    list->cursor_row = 0;
    list->glyphs_per_entry = 8;
}

/* Expand a signed value into decimal glyphs and a trailing -1 sentinel. */
ADDRESS(0x8002adf8, 0xac)
void menu_format_number(s32 value, s32 count, s32 pad_zero, s16 *out)
{
    s32 i = 0;
    s32 blank;

    blank = (pad_zero == 0) ? 10 : 0;
    for (; i < count; i++) {
        out[i] = blank;
    }
    out[count] = -1;
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
    if (id != 0xff) {
        if (cd_file_load_table_entry(&asset, id) != 0) {
            return 1;
        }
        tmd_register(4, asset);
        menu_item_model_allocation_pending = 1;
    }
    DAT_80057b72 = 0;
    return 0;
}

ADDRESS(0x8002af0c, 0x3c)
void menu_release_item_model(void)
{
    if (menu_item_model_allocation_pending == 1) {
        tmd_release_last_allocation(4);
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

    if (id != 0xff) {
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
