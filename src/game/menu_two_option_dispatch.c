#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfDisplayState display_state;
extern POLY_FT4 *current_poly_ft4;

/* Sprite descriptor and destination point shared with the menu blitters. */
typedef struct MenuSpriteDef {
    u16 tpage;
    u16 clut;
    u16 u;
    u16 v;
    u16 width;
    u16 height;
} MenuSpriteDef;

typedef struct MenuPoint {
    u16 x;
    u16 y;
} MenuPoint;

/* Shared two-option sprite descriptors (BSS, filled by the menu setup). */
extern MenuSpriteDef DAT_8005840c; /* option box, normal */
extern MenuSpriteDef DAT_80058418; /* option box, highlighted */
extern MenuSpriteDef DAT_8005846c; /* window frame */
extern u8 DAT_800583f4[];          /* label descriptor for menu_draw_string */

extern void menu_blit_sprite(const MenuSpriteDef *sprite, const MenuPoint *pos);
extern void menu_blit_sprite_translucent(const MenuSpriteDef *sprite, const MenuPoint *pos);
extern void menu_draw_string(u8 *desc, const MenuPoint *pos);

/*
 * Draw a two-option selection widget: one window frame around the selected
 * option, an option box on each of the two points (the selected point getting
 * the highlight box when highlighting is active), and a label under each.
 */
ADDRESS(0x800291ec, 0x10c)
void menu_draw_two_option(const MenuPoint *point0, const MenuPoint *point1,
                   s32 selected, s32 highlight)
{
    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;
    if (selected == 0) {
        menu_blit_sprite(&DAT_8005846c, point0);
    } else {
        menu_blit_sprite(&DAT_8005846c, point1);
    }
    if (highlight == 1) {
        if (selected == 0) {
            menu_blit_sprite_translucent(&DAT_80058418, point0);
            menu_blit_sprite_translucent(&DAT_8005840c, point1);
        } else {
            menu_blit_sprite_translucent(&DAT_8005840c, point0);
            menu_blit_sprite_translucent(&DAT_80058418, point1);
        }
    } else {
        menu_blit_sprite_translucent(&DAT_8005840c, point0);
        menu_blit_sprite_translucent(&DAT_8005840c, point1);
    }
    menu_draw_string(DAT_800583f4, point0);
    menu_draw_string(DAT_800583f4, point1);
}
