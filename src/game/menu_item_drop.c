#include <kf/address.h>
#include <kf/semantic_types.h>

/* Player object and the 240-byte inventory / progress-flag block. */
extern KfPlayerState player_state;
extern u8 DAT_800652a8[240];

/* Item-name string table: ten halfwords (a 20-byte label) per item id. */
extern s16 DAT_80058dc0[];

/* Shared menu primitives: frame begin/flush, input sound cue, vsync/pad poll,
 * and the deferred state acknowledgement. */
extern void menu_frame_begin(void);
extern void func_8002ac34(void);
extern void menu_play_input_sound(s32 cue);
extern u32 pad_read();
extern void game_state_acknowledge_pending(void);

/* Item-list widget helpers (init, render, preview, query, confirm). */
extern void func_8002ad6c(u16 *ctx, s32 arg1, s32 arg2);
extern void menu_list_render(s16 *ctx);
extern u32 func_8002aea4(s32 item_id);
extern void menu_item_model_preview(s32 item_id);
extern s32 menu_list_interact(u32 ctx, s32 arg1, s32 arg2, s32 item_id, u32 arg4,
                         u32 arg5);

/*
 * Item-list display context: a shared menu list header with a visible-window
 * cursor over the label entries that follow it on the frame.
 */
typedef struct KfItemMenu {
    u8 unknown_00[26];
    u8 count;
    u8 page;
    u8 scroll;
    u8 cursor;
    u8 window;
    u8 rows;
    s16 *entries;
    s32 unknown_24;
} KfItemMenu;

/*
 * Drop-item panel dispatched by the hub menu (slot 4).  Builds a scrollable
 * list of every held item, subtracting one copy of any item currently worn in
 * one of the seven equipment slots so the equipped instance cannot be
 * discarded, runs the windowed cursor, and on confirm removes one of the
 * chosen item from the owned-item block.
 *
 * Structurally exact; open residue is the list-widget callee-saved-register
 * permutation plus one loop delay-slot swap (see docs/patterns/
 * source-shapes-gcc257.md, "item / inventory menu panels").
 */
ADDRESS(0x800249a8, 0x4bc)
void func_800249a8(void)
{
    KfItemMenu ctx;
    s16 labels[80][10];
    u8 counts[80];
    u8 codes[80];
    u8 *inv;
    u8 *equip;
    s16 *name;
    s32 found;
    s32 code;
    s32 j;
    s32 input = 0;
    s32 prev;
    s32 confirm = 0;
    s32 selection = -99;

    while (pad_read(1) != 0)
        ;
    func_8002ad6c((u16 *)&ctx, 0, 4);

    found = 0;
    /* The seven equipment ids: [0] worn weapon, [0x2c..0x31] shield, head,
     * body, arm and leg armour, and accessory. */
    equip = &player_state.equipped_weapon_id;
    inv = DAT_800652a8;
    name = DAT_80058dc0;
    for (code = 0; code < 80; code++, name += 10) {
        if (inv[code] != 0) {
            counts[found] = inv[code];
            if (code == equip[0x00] || code == equip[0x2c] ||
                code == equip[0x2d] || code == equip[0x2e] ||
                code == equip[0x2f] || code == equip[0x30] ||
                code == equip[0x31])
                counts[found]--;
            if (counts[found] != 0) {
                for (j = 0; j < 10; j++)
                    labels[found][j] = name[j];
                codes[found] = code;
                found++;
            }
        }
    }

    ctx.count = found;
    ctx.rows = 10;
    ctx.entries = &labels[0][0];
    ctx.unknown_24 = 0;

    menu_frame_begin();
    if (ctx.count != 0) {
        if (func_8002aea4(codes[ctx.cursor]) != 0)
            return;
        menu_item_model_preview(codes[ctx.cursor]);
    }
    menu_list_render((s16 *)&ctx);

    for (;;) {
        func_8002ac34();
        if (confirm == 1) {
            if (menu_list_interact((u32)&ctx, 1, 0, codes[ctx.cursor], 0, 0) == -1)
                selection = -99;
            else
                selection = codes[ctx.cursor];
        }
        confirm = 0;
        if (selection != -99) {
            while (pad_read(1) != 0)
                ;
            break;
        }

        prev = input;
        input = pad_read(1);
        if (ctx.count == 0) {
            if (input != 0) {
                menu_play_input_sound(0);
                selection = -1;
            }
        } else if ((input & 0x1000) != 0 && (prev & 0x1000) == 0) {
            menu_play_input_sound(0);
            if (ctx.cursor != 0) {
                ctx.cursor--;
                if (ctx.window == 0)
                    ctx.scroll--;
                else
                    ctx.window--;
            } else {
                ctx.cursor = ctx.count - 1;
                if (ctx.count < ctx.page) {
                    ctx.scroll = 0;
                    ctx.window = ctx.count - 1;
                } else {
                    ctx.scroll = ctx.count - ctx.page;
                    ctx.window = ctx.page - 1;
                }
            }
            if (func_8002aea4(codes[ctx.cursor]) != 0)
                return;
        } else if ((input & 0x4000) != 0 && (prev & 0x4000) == 0) {
            menu_play_input_sound(0);
            if (ctx.cursor < ctx.count - 1) {
                ctx.cursor++;
                if (ctx.window == ctx.page - 1)
                    ctx.scroll++;
                else
                    ctx.window++;
            } else {
                ctx.cursor = 0;
                ctx.scroll = 0;
                ctx.window = 0;
            }
            if (func_8002aea4(codes[ctx.cursor]) != 0)
                return;
        } else if ((input & 0x20) != 0 && (prev & 0x20) == 0) {
            menu_play_input_sound(1);
            confirm = 1;
        } else if ((input & 0x40) != 0 && (prev & 0x40) == 0) {
            menu_play_input_sound(2);
            selection = -1;
        }

        menu_frame_begin();
        if (ctx.count != 0)
            menu_item_model_preview(codes[ctx.cursor]);
        menu_list_render((s16 *)&ctx);
    }

    game_state_acknowledge_pending();
    if (selection != -1)
        inv[selection]--;
}
