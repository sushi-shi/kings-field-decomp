#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/*
 * Menu label glyph store, addressed in 8-byte cells.  A label occupies three
 * cells (24 bytes) and a row holds eleven labels (33 cells), so the label at
 * (row, col) starts at cell row * 33 + col * 3.  BSS, shared with the menus.
 */
typedef struct MenuLabelCell {
    u16 halfwords[4];
} MenuLabelCell;

extern MenuLabelCell DAT_80058494[];

extern u32 cd_file_load_table_entry(void **asset_out, s32 id);
extern void tmd_register(u16 slot, void *asset);

/*
 * Header block that precedes a menu list on the frame: two layout shorts, the
 * ten-glyph label copied from the (row, col) table, and the window/cursor
 * bytes that the list widget drives afterwards.
 */
typedef struct MenuListHeader {
    s16 field_00;
    s16 field_02;
    u16 glyphs[10];
    u8 field_18;
    u8 field_19;
    u8 count;
    u8 field_1b;
    u8 scroll;
    u8 cursor;
    u8 window;
    u8 rows;
} MenuListHeader;

/* Initialize a menu list header and copy its label glyphs from the table. */
ADDRESS(0x8002ad6c, 0x8c)
void menu_list_init(MenuListHeader *ctx, s32 row, s32 col)
{
    u16 *src;
    s32 i;

    ctx->field_00 = 12;
    ctx->field_02 = 19;
    src = DAT_80058494[row * 33 + col * 3].halfwords;
    for (i = 0; i < 10; i++) {
        ctx->glyphs[i] = src[i];
    }
    ctx->field_18 = 0x16;
    ctx->field_19 = 0x26;
    ctx->count = 0;
    ctx->field_1b = 0x0b;
    ctx->scroll = 0;
    ctx->cursor = 0;
    ctx->window = 0;
    ctx->rows = 8;
}

/*
 * Render a signed value into a fixed run of decimal glyph slots.  Unused
 * leading slots are blanked with glyph 10 unless zero-padding is requested, a
 * terminator (0xffff) follows the last slot, and digits are emitted low to
 * high until the value is exhausted.
 */
ADDRESS(0x8002adf8, 0xac)
void menu_format_number(s32 value, s32 count, s32 pad_zero, s16 *out)
{
    s16 blank;
    s32 i;

    blank = (pad_zero == 0) ? 10 : 0;
    for (i = 0; i < count; i++) {
        out[i] = blank;
    }
    out[count] = -1;
    for (i = count - 1; i >= 0; i--) {
        out[i] = value % 10;
        value /= 10;
        if (value == 0) {
            break;
        }
    }
}

/*
 * Commit the pending game state, then load and register the TMD asset for the
 * selected item.  Returns 1 when the asset load fails; otherwise clears the
 * pending markers and returns 0.
 */
ADDRESS(0x8002aea4, 0x68)
u32 menu_load_item_model(s32 id)
{
    void *asset;

    game_state_acknowledge_pending();
    if (id != 0xff) {
        if (cd_file_load_table_entry(&asset, id) != 0) {
            return 1;
        }
        tmd_register(4, asset);
        pending_game_state = 1;
    }
    DAT_80057b72 = 0;
    return 0;
}
