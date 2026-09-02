#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;
extern KfWeaponRecord weapon_records[16];
/* "...\0" image path whose two decimal digits at [9] and [10] select the weapon. */
extern char weapon_image_path_template[15];
extern s32 func_8001af9c(u8 *buffer, const char *path);
extern void exit(s32 status);
extern void asset_registry_set(u16 index, void *asset);
extern void player_recalculate_combat_stats(void);

ADDRESS(0x80016a30, 0xf4)
void player_equip_weapon(u8 weapon_id)
{
    player_state.attack_charge_state.current = 0;
    player_state.attack_charge_state.committed = 0;
    player_state.weapon_charge_delay = 10;
    player_state.equipped_weapon_id = weapon_id;
    if (weapon_id != 0xff) {
        player_state.equipped_weapon_record = &weapon_records[weapon_id];
        weapon_image_path_template[9] = '0' + weapon_id / 10;
        weapon_image_path_template[10] = '0' + weapon_id % 10;
        if (func_8001af9c(player_state.weapon_asset_buffer, weapon_image_path_template) != 0) {
            exit(1);
        }
        asset_registry_set(0x14, player_state.weapon_asset_buffer);
    }
    player_state.weapon_attack_phase = -1;
    player_state.unknown_74 = 0;
    player_recalculate_combat_stats();
}
