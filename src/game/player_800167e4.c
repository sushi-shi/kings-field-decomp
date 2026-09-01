#include <kf/address.h>
#include <kf/game_types.h>

extern u16 player_magic_charge;
extern u8 player_selected_magic_id;
extern const void *player_selected_magic_record;
/* Unresolved table of 20-byte magic records; only its base identity is known. */
extern const u8 DAT_8009ce60[];

ADDRESS(0x800167e4, 0x64)
void player_select_magic(u8 magic_id)
{
    player_magic_charge = 0;
    player_selected_magic_id = magic_id;
    if (magic_id == 0xff) {
        player_selected_magic_record = 0;
    } else {
        player_selected_magic_record = DAT_8009ce60 + player_selected_magic_id * 20;
    }
}
