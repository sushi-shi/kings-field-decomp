#include <kf/address.h>
#include <kf/game_types.h>

/*
 * The parameters are reused as the working values: retail keeps the
 * threshold in $a0 and the excess in $a1 for the whole body.
 */
ADDRESS(0x8001627c, 0xa8)
s32 player_calculate_damage_component(s32 base_power, s32 defense, s32 attack)
{
    if (attack == 0) {
        return 0;
    }
    base_power = defense + base_power / 5;
    defense = attack - base_power;
    if (defense < 0) {
        defense = 0;
    }
    if (base_power == 0) {
        base_power = 1;
    }
    return defense + (attack * attack) / (base_power * 2);
}
