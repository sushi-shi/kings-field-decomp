#include <kf/game_graphics.h>
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

/*
 * Contiguous GAME.EXE menu presentation/runtime run
 * 0x800291ec..0x8002b078. The original source boundary is WIP.
 */

/* The numbered-item TIM path template used by menu_load_item_texture. */
RODATA(0x80012350, 0xc)

DATA(0x80057b6c, 0x4)
KfMenuModelAllocation menu_item_model_allocation_pending = KF_MENU_MODEL_RELEASED;

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
    KfMenuConfirmChoice selected, KfMenuConfirmState highlight)
{
    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;
    if (selected == KF_MENU_CHOICE_ACCEPT) {
        menu_blit_sprite(&menu_assets.selection_cursor, &option0->position);
    } else {
        menu_blit_sprite(&menu_assets.selection_cursor, &option1->position);
    }
    if (highlight == KF_MENU_CONFIRM_REQUESTED) {
        if (selected == KF_MENU_CHOICE_ACCEPT) {
            menu_blit_sprite_translucent(
                &menu_assets.option_highlight, &option0->position);
            menu_blit_sprite_translucent(
                &menu_assets.option_background, &option1->position);
        } else {
            menu_blit_sprite_translucent(
                &menu_assets.option_background, &option0->position);
            menu_blit_sprite_translucent(
                &menu_assets.option_highlight, &option1->position);
        }
    } else {
        menu_blit_sprite_translucent(
            &menu_assets.option_background, &option0->position);
        menu_blit_sprite_translucent(
            &menu_assets.option_background, &option1->position);
    }
    menu_draw_string(&menu_assets.glyph_atlas, option0);
    menu_draw_string(&menu_assets.glyph_atlas, option1);
}

/*
 * Draw the item-pickup preview: a spinning 3D model, its ten-glyph name, and four
 * mirrored window-sprite quads. The shared spin angle advances eight units.
 */
ADDRESS(0x800292f8, 0x7b8)
void menu_draw_item_name_frame(KF_ENUM_PARAM(KfItemId, s32) item_id)
{
    MenuGlyphString string;
    MATRIX rotation;
    MATRIX light_source;
    MATRIX light_result;
    const MenuGlyphRow *name;
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

    name = &item_name_rows[KF_ENUM_ENCODE(s32, item_id)];
    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;
    string.position.x = 0x80;
    string.position.y = 0x24;
    for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {
        string.glyphs.codes[i] = name->codes[i];
    }
    menu_draw_string(&menu_assets.glyph_atlas, &string);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH(current_poly_ft4,
        MENU_PICKUP_BACKDROP_LEFT_X,
        MENU_BACKDROP_TOP_Y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    setUVWH(current_poly_ft4,
        menu_assets.window_backdrop.u,
        menu_assets.window_backdrop.v,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH(current_poly_ft4,
        MENU_PICKUP_BACKDROP_RIGHT_X,
        MENU_BACKDROP_TOP_Y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    setUV4(current_poly_ft4,
        menu_assets.window_backdrop.u + menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.v,
        menu_assets.window_backdrop.u,
        menu_assets.window_backdrop.v,
        menu_assets.window_backdrop.u + menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.v + menu_assets.window_backdrop.height,
        menu_assets.window_backdrop.u,
        menu_assets.window_backdrop.v + menu_assets.window_backdrop.height);
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH(current_poly_ft4,
        MENU_PICKUP_BACKDROP_LEFT_X,
        MENU_BACKDROP_BOTTOM_Y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    setUV4(current_poly_ft4,
        menu_assets.window_backdrop.u,
        menu_assets.window_backdrop.v + menu_assets.window_backdrop.height,
        menu_assets.window_backdrop.u + menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.v + menu_assets.window_backdrop.height,
        menu_assets.window_backdrop.u,
        menu_assets.window_backdrop.v,
        menu_assets.window_backdrop.u + menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.v);
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH(current_poly_ft4,
        MENU_PICKUP_BACKDROP_RIGHT_X,
        MENU_BACKDROP_BOTTOM_Y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    setUV4(current_poly_ft4,
        menu_assets.window_backdrop.u + menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.v + menu_assets.window_backdrop.height,
        menu_assets.window_backdrop.u,
        menu_assets.window_backdrop.v + menu_assets.window_backdrop.height,
        menu_assets.window_backdrop.u + menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.v,
        menu_assets.window_backdrop.u,
        menu_assets.window_backdrop.v);
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    AddPrim(
        game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][3]);
    AddPrim(
        game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][2]);
    AddPrim(
        game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][1]);
    AddPrim(
        game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][0]);
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
    setUVWH(current_poly_ft4, sprite->u, sprite->v, sprite->width, sprite->height);
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
    setUVWH(current_poly_ft4, sprite->u, sprite->v, sprite->width, sprite->height);
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

    for (i = 0; string->glyphs.codes[i] != MENU_TEXT_END; i++) {
        s32 glyph;

        x_offset = i * MENU_FONT_CELL_WIDTH;
        primitive_buffer_begin_poly_ft4();
        current_poly_ft4->tpage = font->tpage;
        current_poly_ft4->clut = font->clut;
        setXYWH(current_poly_ft4,
            string->position.x + x_offset,
            string->position.y,
            font->width,
            font->height);
        glyph = string->glyphs.codes[i] & MENU_TEXT_GLYPH_MASK;
        setUVWH(current_poly_ft4,
            (glyph % MENU_FONT_COLUMNS) * MENU_FONT_CELL_WIDTH,
            (glyph / MENU_FONT_COLUMNS) * MENU_FONT_CELL_HEIGHT,
            font->width,
            font->height);
        primitive_buffer_commit_poly_ft4(MENU_TEXT_OT_DEPTH);

        if (string->glyphs.codes[i] & MENU_TEXT_DAKUTEN) {
            primitive_buffer_begin_poly_ft4();
            current_poly_ft4->tpage = font->tpage;
            current_poly_ft4->clut = font->clut;
            setXYWH(current_poly_ft4,
                string->position.x + x_offset,
                string->position.y,
                font->width,
                font->height);
            setUVWH(current_poly_ft4, MENU_DAKUTEN_U, MENU_KANA_MARK_V, font->width, font->height);
            primitive_buffer_commit_poly_ft4(MENU_TEXT_OT_DEPTH);
        }

        if (string->glyphs.codes[i] & MENU_TEXT_HANDAKUTEN) {
            primitive_buffer_begin_poly_ft4();
            current_poly_ft4->tpage = font->tpage;
            current_poly_ft4->clut = font->clut;
            setXYWH(current_poly_ft4,
                string->position.x + x_offset,
                string->position.y,
                font->width,
                font->height);
            setUVWH(current_poly_ft4,
                MENU_HANDAKUTEN_U,
                MENU_KANA_MARK_V,
                font->width,
                font->height);
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

    for (i = 0; label->glyphs.codes[i] != MENU_TEXT_END; i++) {
        xoff = i * MENU_NUMBER_ADVANCE;
        primitive_buffer_begin_poly_ft4();
        current_poly_ft4->tpage = font->tpage;
        current_poly_ft4->clut = font->clut;
        setXYWH(current_poly_ft4,
            label->position.x + xoff,
            label->position.y,
            font->width,
            font->height);
        setUVWH(current_poly_ft4,
            font->u,
            label->glyphs.codes[i] * MENU_NUMBER_CELL_HEIGHT,
            font->width,
            font->height);
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
    setXYWH(current_poly_ft4,
        MENU_BACKDROP_LEFT_X,
        MENU_BACKDROP_TOP_Y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    setUVWH(current_poly_ft4,
        menu_assets.window_backdrop.u,
        menu_assets.window_backdrop.v,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH(current_poly_ft4,
        MENU_BACKDROP_RIGHT_X,
        MENU_BACKDROP_TOP_Y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    setUV4(current_poly_ft4,
        menu_assets.window_backdrop.u + menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.v,
        menu_assets.window_backdrop.u,
        menu_assets.window_backdrop.v,
        menu_assets.window_backdrop.u + menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.v + menu_assets.window_backdrop.height,
        menu_assets.window_backdrop.u,
        menu_assets.window_backdrop.v + menu_assets.window_backdrop.height);
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH(current_poly_ft4,
        MENU_BACKDROP_LEFT_X,
        MENU_BACKDROP_BOTTOM_Y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    setUV4(current_poly_ft4,
        menu_assets.window_backdrop.u,
        menu_assets.window_backdrop.v + menu_assets.window_backdrop.height,
        menu_assets.window_backdrop.u + menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.v + menu_assets.window_backdrop.height,
        menu_assets.window_backdrop.u,
        menu_assets.window_backdrop.v,
        menu_assets.window_backdrop.u + menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.v);
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH(current_poly_ft4,
        MENU_BACKDROP_RIGHT_X,
        MENU_BACKDROP_BOTTOM_Y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    setUV4(current_poly_ft4,
        menu_assets.window_backdrop.u + menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.v + menu_assets.window_backdrop.height,
        menu_assets.window_backdrop.u,
        menu_assets.window_backdrop.v + menu_assets.window_backdrop.height,
        menu_assets.window_backdrop.u + menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.v,
        menu_assets.window_backdrop.u,
        menu_assets.window_backdrop.v);
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    AddPrim(
        game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][3]);
    AddPrim(
        game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][2]);
    AddPrim(
        game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][1]);
    AddPrim(
        game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][0]);
}

/*
 * Begin a menu frame: flip the double buffer, re-anchor the active primitive
 * buffer and ordering table for the new buffer index, clear the ordering
 * table, and rewind the primitive-buffer cursor.
 */
ADDRESS(0x8002abb4, 0x80)
void menu_frame_begin(void)
{
    game_graphics_runtime.display_state.buffer_index = display_next_buffer(game_graphics_runtime.display_state.buffer_index);
    game_graphics_runtime.display_state.primitive_buffer =
        &game_graphics_runtime.display_state.primitive_buffers[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)];
    game_graphics_runtime.display_state.ordering_table =
        game_graphics_runtime.display_state.ordering_tables[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)].entries;
    ClearOTagR(game_graphics_runtime.display_state.ordering_table, KF_ORDERING_TABLE_LENGTH);
    game_graphics_runtime.display_state.primitive_buffer->cursor =
        game_graphics_runtime.display_state.primitive_buffer->start;
}

/* Wait for the GPU/retrace, install the active environments, and submit OT. */
ADDRESS(0x8002ac34, 0x98)
void menu_present_frame(void)
{
    DrawSync(0);
    VSync(0);
    PutDrawEnv(&game_graphics_runtime.display_draw_environments[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)]);
    PutDispEnv(&game_graphics_runtime.display_disp_environments[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)]);
    DrawOTag(game_graphics_runtime.display_state.ordering_table + (KF_ORDERING_TABLE_LENGTH - 1));
}

ADDRESS(0x8002accc, 0x50)
void primitive_buffer_begin_poly_ft4(void)
{
    SetPolyFT4(current_poly_ft4);
    setRGB0(current_poly_ft4,
        MENU_PRIMITIVE_BRIGHTNESS,
        MENU_PRIMITIVE_BRIGHTNESS,
        MENU_PRIMITIVE_BRIGHTNESS);
}

ADDRESS(0x8002ad1c, 0x50)
void primitive_buffer_commit_poly_ft4(s32 depth)
{
    AddPrim(
        &game_graphics_runtime.display_state.ordering_table[depth],
        current_poly_ft4);
    current_poly_ft4++;
    game_graphics_runtime.display_state.primitive_buffer->cursor = (u8 *)current_poly_ft4;
}

/* Initialize a menu list header and copy its label glyphs from the table. */
ADDRESS(0x8002ad6c, 0x8c)
void menu_list_init(KfMenuList *list, KfMenuWindowKind kind, s32 row)
{
    s32 i;

    list->title.position.x = 12;
    list->title.position.y = 19;
    for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {
        list->title.glyphs.codes[i] = menu_window_layouts[KF_ENUM_ENCODE(s32, kind)].rows[row].glyphs.codes[i];
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
void menu_format_number(s32 value, s32 count, KF_ENUM_PARAM(KfFormatPaddingMode, s32) pad_zero, s16 *out)
{
    s32 i = 0;
    s32 blank;

    blank = (pad_zero == KF_FORMAT_PAD_SPACES) ? MENU_NUMBER_BLANK : 0;
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
KF_ENUM_PARAM(KfResourceLoadResult, u32) menu_load_item_model(KF_ENUM_PARAM(KfItemId, s32) id)
{
    void *asset;

    menu_release_item_model();
    if (id != KF_ITEM_NONE) {
        if (cd_file_load_table_entry(&asset, KF_ENUM_ENCODE(s32, id)) != KF_RESOURCE_LOADED) {
            return KF_RESOURCE_LOAD_FAILED;
        }
        tmd_register(KF_TMD_SLOT_MENU_ITEM, (KfTmdHeader *)asset);
        menu_item_model_allocation_pending = KF_MENU_MODEL_ALLOCATED;
    }
    menu_item_preview_rotation.vy = 0;
    return KF_RESOURCE_LOADED;
}

ADDRESS(0x8002af0c, 0x3c)
void menu_release_item_model(void)
{
    if (menu_item_model_allocation_pending == KF_MENU_MODEL_ALLOCATED) {
        tmd_release_last_allocation(KF_TMD_SLOT_MENU_ITEM);
        menu_item_model_allocation_pending = KF_MENU_MODEL_RELEASED;
    }
}

/* Load the numbered item TIM into the active primitive buffer and VRAM. */
ADDRESS(0x8002af48, 0x130)
KF_ENUM_PARAM(KfResourceLoadResult, u32) menu_load_item_texture(KfMenuTextureId id)
{
    char name[16] = "TIM\\M000.";
    void *destination;
    s32 number;
    s32 remainder;

    if (id != KF_MENU_TEXTURE_NONE) {
        number = KF_ENUM_ENCODE(s32, id) + 1;
        name[5] = number / 100 + '0';
        remainder = number % 100;
        name[6] = remainder / 10 + '0';
        name[7] = remainder % 10 + '0';
        destination = game_graphics_runtime.display_state.primitive_buffer->cursor;
        if (cd_file_load_into(destination, name) != KF_RESOURCE_LOADED) {
            return KF_RESOURCE_LOAD_FAILED;
        }
        tim_upload_images(destination);
    }
    return KF_RESOURCE_LOADED;
}
