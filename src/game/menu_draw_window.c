#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/* Current textured-quad cursor in the shared primitive workspace. */

/* Font/sprite atlas descriptor: atlas page/clut, texel origin and cell size. */
typedef struct MenuSpriteDef {
    u16 tpage;
    u16 clut;
    u16 u;
    u16 v;
    u16 width;
    u16 height;
} MenuSpriteDef;

/* Screen origin shared by the sprite blitters and the glyph drawers. */
typedef struct MenuPoint {
    u16 x;
    u16 y;
} MenuPoint;

/*
 * Window/menu-list descriptor bank: nine 0x108-byte records copied in from the
 * layout resource.  Each record is a title label followed by ten 0x18-byte row
 * labels (a {x, y} origin plus its glyph codes), consumed here as MenuPoints
 * for the row backgrounds and as glyph strings for the text.
 */
extern u8 DAT_80058478[];
extern MenuSpriteDef DAT_80058424;   /* default row-background sprite */
extern MenuSpriteDef DAT_80058430;   /* confirmed-selection row background */
extern MenuSpriteDef DAT_8005846c;   /* selection-cursor sprite */
extern MenuSpriteDef DAT_800583f4;   /* menu glyph atlas */

extern void menu_blit_sprite_translucent(const MenuSpriteDef *sprite, const MenuPoint *pos);
extern void menu_blit_sprite(const MenuSpriteDef *sprite, const MenuPoint *pos);
extern void menu_draw_string(const MenuSpriteDef *font, const MenuPoint *str);
extern void menu_draw_window_backdrop(void);

/*
 * Draw one menu window: an optional title label (drawn when the record's first
 * halfword is non-zero), then `count` selectable rows, then the shared
 * translucent backdrop for every window kind but 6.  The highlighted row
 * (index `highlight`) takes the confirmed-selection background when `flag` is
 * 1 and always gets the selection-cursor sprite overlaid.  Rows advance one
 * 0x18-byte label per step starting at the record's second label.
 *
 * Residue: call set, referents, control flow and the recomputed
 * highlight*0x18+0x18 offsets all match retail, but gcc257-o2 CSEs the
 * per-row `record + off` pointer and the loop bounds into two extra
 * callee-saved registers (56-byte frame vs retail's 48), where retail's
 * compiler recomputes `record + off` before each call.  An unattributed
 * CSE/register-allocation wall for this many-call drawing loop; no faithful
 * source form suppresses the CSE without distorting the code.
 */
ADDRESS(0x80028914, 0x15c)
void menu_draw_window(s32 kind, s32 count, s32 highlight, s32 flag)
{
    u8 *record;
    s32 off;

    record = &DAT_80058478[kind * 0x108];
    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;
    if (*(s16 *)record != 0) {
        menu_blit_sprite_translucent(&DAT_80058424, (const MenuPoint *)record);
        menu_draw_string(&DAT_800583f4, (const MenuPoint *)record);
    }
    if (count > 0) {
        off = 0x18;
        do {
            const MenuSpriteDef *box = &DAT_80058424;

            if (off == highlight * 0x18 + 0x18 && flag == 1) {
                box = &DAT_80058430;
            }
            menu_blit_sprite_translucent(box, (const MenuPoint *)(record + off));
            if (off == highlight * 0x18 + 0x18) {
                menu_blit_sprite(&DAT_8005846c, (const MenuPoint *)(record + off));
            }
            menu_draw_string(&DAT_800583f4, (const MenuPoint *)(record + off));
            off += 0x18;
        } while (off < count * 0x18 + 0x18);
    }
    if (kind != 6) {
        menu_draw_window_backdrop();
    }
}
