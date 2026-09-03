#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfDisplayState display_state;
extern POLY_FT4 *current_poly_ft4;

/* Sprite/point descriptors shared with the menu blitters. */
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

/* Option-box sprites and the font-atlas descriptor (BSS, set up elsewhere). */
extern MenuSpriteDef DAT_8005840c; /* option box, normal */
extern MenuSpriteDef DAT_80058418; /* option box, highlighted */
extern u8 DAT_800583f4[];          /* font-atlas descriptor for menu_draw_string */

/* Double-buffered map-viewer frame quads linked behind the config rows. */
extern POLY_FT4 DAT_800580e8[2][4];

extern void menu_blit_sprite_translucent(const MenuSpriteDef *sprite, const MenuPoint *pos);
extern void menu_draw_string(const MenuSpriteDef *font, const MenuPoint *str);

/*
 * Config-panel draw request passed by value.  Only the two option-row anchors
 * and the per-row selection array are consumed; the reserved gaps preserve the
 * caller's layout so the anchors and the state pointer land in the original
 * argument slots.
 */
typedef struct ConfigPanelParams {
    MenuPoint pt_a;
    u8 reserved0[20];
    MenuPoint pt_b;
    u8 reserved1[20];
    s32 *states;
} ConfigPanelParams;

/*
 * Draw four config rows.  Each row places two option boxes at the row's left
 * and right anchors -- highlighting the one whose per-row state equals 1 -- and
 * a label under each, then steps both anchors down 22 pixels.  Finally links
 * the four double-buffered map-viewer frame quads for the current buffer at
 * ordering-table slot 3000.
 */
ADDRESS(0x80025da0, 0x198)
void func_80025da0(ConfigPanelParams params)
{
    s32 i;
    s32 *states;
    const MenuSpriteDef *box_b;

    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;
    states = params.states;
    for (i = 0; i < 4; i++) {
        if (*states == 1) {
            menu_blit_sprite_translucent(&DAT_80058418, &params.pt_a);
            box_b = &DAT_8005840c;
        } else {
            menu_blit_sprite_translucent(&DAT_8005840c, &params.pt_a);
            box_b = &DAT_80058418;
        }
        menu_blit_sprite_translucent(box_b, &params.pt_b);
        menu_draw_string((const MenuSpriteDef *)DAT_800583f4, &params.pt_a);
        menu_draw_string((const MenuSpriteDef *)DAT_800583f4, &params.pt_b);
        states++;
        params.pt_a.y += 22;
        params.pt_b.y += 22;
    }
    AddPrim(&display_state.ordering_table[3000],
            &DAT_800580e8[display_state.buffer_index][3]);
    AddPrim(&display_state.ordering_table[3000],
            &DAT_800580e8[display_state.buffer_index][2]);
    AddPrim(&display_state.ordering_table[3000],
            &DAT_800580e8[display_state.buffer_index][1]);
    AddPrim(&display_state.ordering_table[3000],
            &DAT_800580e8[display_state.buffer_index][0]);
}
