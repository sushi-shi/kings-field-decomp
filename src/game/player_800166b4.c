#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/* Psy-Q LIBC: int rand(void). */
extern s32 rand(void);

/*
 * Applies the four status bits (poison, curse blocked by accessory 0x31,
 * a resisted effect, and a short one), combines the five defended damage
 * components in tenths, scales the sum, and subtracts it from the hit
 * points, flagging the update state on any damage.
 */
ADDRESS(0x80016324, 0x390)
void player_apply_damage(
    u16 component0,
    u16 component1,
    u16 component2,
    u16 status_effect_flags,
    u16 component3,
    u16 component4,
    u16 scale_q12,
    u16 multiplier_tenths)
{
    s32 damage;
    s32 loss;
    s32 remaining;

    if (status_effect_flags & 1) {
        player_state.status_effect0_timer = 600;
        player_state.status_effect_flags |= 1;
    }
    if ((status_effect_flags & 2) && player_state.equipped_accessory_id != 0x31) {
        if (player_state.status_effect1_timer != -1) {
            if (player_state.status_effect1_timer < 970) {
                player_state.status_effect1_timer = 970;
            }
        } else {
            player_state.status_effect1_timer = 1000;
        }
        player_state.status_effect_flags |= 2;
    }
    if (status_effect_flags & 4) {
        if (player_state.status_effect2_resistance < (rand() * 100) >> 15) {
            player_state.status_effect2_timer = 600;
            player_state.status_effect_flags |= 4;
        }
    }
    if (status_effect_flags & 8) {
        player_state.status_effect3_timer = 300;
        player_state.status_effect_flags |= 8;
    }
    damage = player_calculate_damage_component(
        player_state.physical_power * 10, player_state.damage_defense_component0 * 10, component0 * 10);
    damage += player_calculate_damage_component(
        player_state.physical_power * 10, player_state.damage_defense_component1 * 10, component1 * 10);
    damage += player_calculate_damage_component(
        player_state.physical_power * 10, player_state.damage_defense_component2 * 10, component2 * 10);
    damage += player_calculate_damage_component(
        player_state.physical_power * 10, player_state.damage_defense_component3 * 10, component3 * 10);
    damage += player_calculate_damage_component(
        player_state.physical_power * 10, player_state.damage_defense_component4 * 10, component4 * 10);
    damage += 5;
    damage = (scale_q12 * (damage / 10)) >> 12;
    loss = (multiplier_tenths * damage) / 10;
    if (loss != 0) {
        remaining = player_state.vitals.current_hp - loss;
        if (remaining <= 0) {
            remaining = 0;
        }
        player_state.vitals.current_hp = remaining;
        if (player_state.update_state != 0xff) {
            player_state.update_state = 1;
        }
    }
}

ADDRESS(0x800166b4, 0x130)
void player_apply_radial_damage(
    const struct KfVec3i *origin,
    u32 radius,
    u16 falloff_q12,
    u16 base_power,
    u16 component0,
    u16 component1,
    u16 component2,
    u16 component3,
    u16 component4,
    u16 scale_q12,
    u16 multiplier_tenths)
{
    s32 distance;
    u16 attenuation;
    u32 value;

    distance = player_distance_to_point(origin->x, origin->y, origin->z, radius, radius);
    if (distance == -1) {
        return;
    }
    if (falloff_q12 != 0x1000) {
        attenuation = (distance << 12) / radius;
        value = attenuation * (0x1000 - falloff_q12);
        attenuation = 0x1000 - (value >> 12);
        value = scale_q12 * attenuation;
        attenuation = value >> 12;
    } else {
        attenuation = scale_q12;
    }
    player_apply_damage(
        component0, component1, component2, 0, component3, component4,
        attenuation, multiplier_tenths);
}
