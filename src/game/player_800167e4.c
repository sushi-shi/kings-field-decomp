#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;
extern KfMagicRecord magic_records[24];

ADDRESS(0x800167e4, 0x64)
void player_select_magic(u8 magic_id)
{
    player_state.magic_charge = 0;
    player_state.selected_magic_id = magic_id;
    if (magic_id == 0xff) {
        player_state.selected_magic_record = 0;
    } else {
        player_state.selected_magic_record =
            &magic_records[player_state.selected_magic_id];
    }
}
