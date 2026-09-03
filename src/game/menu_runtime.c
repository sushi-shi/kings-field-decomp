#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/*
 * Contiguous GAME.EXE menu-runtime run 0x8002abb4..0x8002b078.  The original
 * source-file boundary is unresolved; this WIP module replaces four matching
 * units that split a gapless function run at reconstruction-only boundaries.
 */

/* The numbered-item TIM path template used by menu_load_item_texture. */
RODATA(0x80012350, 0xc)

DATA(0x80057e88, 0x4)
POLY_FT4 *current_poly_ft4 = 0;

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
    u16 *src;
    s32 i;

    list->title_x = 12;
    list->title_y = 19;
    src = DAT_80058494[row * 33 + column * 3].halfwords;
    for (i = 0; i < 10; i++) {
        list->title_glyphs[i] = src[i];
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

/* Load and register the selected item model, preserving the pending owner. */
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

ADDRESS(0x8002af0c, 0x3c)
void game_state_acknowledge_pending(void)
{
    if (pending_game_state == 1) {
        tmd_release_last_allocation(4);
        pending_game_state = 0;
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
