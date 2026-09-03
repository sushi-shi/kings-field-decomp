#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

extern const SoundRef player_sound_refs[3];

ADDRESS(0x80016b24, 0x9c)
void player_begin_weapon_attack(void)
{
    if (player_state.weapon_attack_phase == -1 && player_state.equipped_weapon_id != 0xff) {
        player_state.weapon_attack_phase = 0;
        sound_ref_play(&player_sound_refs[0], 0x7f);
        player_state.attack_charge_state.committed = player_state.attack_charge_state.current;
        if (player_state.attack_charge_state.current == 5000) {
            player_state.weapon_attack_fully_charged = 1;
        } else {
            player_state.weapon_attack_fully_charged = 0;
        }
        player_state.attack_charge_state.current = 0;
    }
}
