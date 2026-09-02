#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;
/* Psy-Q LIBGTE: RotMatrix(SVECTOR *r, MATRIX *m); ApplyMatrix(MATRIX *m, SVECTOR *v, VECTOR *rv). */
extern s32 actor_pool_find_overlap(s32 x, s32 y, s32 z, s32 extra_radius, s32 point_height);
/*
 * The actor index reaches actor_apply_damage untruncated, so this call site
 * did not go through a halfword-typed prototype.
 */
extern void actor_apply_damage();
extern s32 fixed6_ratio_step(s32 value, s32 span);

ADDRESS(0x80016bc0, 0x264)
void player_update_weapon_attack(void)
{
    struct KfVec4s offset;
    struct KfVec4s rotation;
    struct KfVec4i result;
    struct KfMatrix matrix;
    u16 window;
    s32 actor;

    if (player_state.equipped_weapon_id == 0xff) {
        return;
    }
    if (player_state.weapon_attack_phase != -1) {
        player_state.weapon_attack_phase += 300;
        window = player_state.weapon_attack_phase;
        if (player_state.equipped_weapon_id == 3
                ? (u16)(window - 1000) < 300
                : (u16)(window - 3072) < 300) {
            offset.x = 0;
            offset.y = 1000;
            offset.z = player_state.equipped_weapon_record->attack_z_offset;
            rotation.x = 0;
            rotation.y = -player_state.camera_rotation.y;
            rotation.z = 0;
            RotMatrix(&rotation, &matrix);
            ApplyMatrix(&matrix, &offset, &result);
            result.x += player_state.camera_position.x;
            result.y += player_state.camera_position.y;
            result.z += player_state.camera_position.z;
            actor = actor_pool_find_overlap(result.x, result.y, result.z, 800, 1000);
            if (actor != -1) {
                actor_apply_damage(
                    actor,
                    player_state.physical_power,
                    player_state.attack_component0,
                    player_state.attack_component1,
                    player_state.attack_component2,
                    player_state.attack_component3,
                    player_state.attack_component4,
                    player_state.attack_charge_state.committed,
                    0x10);
            }
            player_state.attack_charge_state.committed = 0;
            player_state.attack_charge_state.current = 0;
            player_state.weapon_charge_delay = 10;
        }
        if (player_state.weapon_attack_phase >= 4096) {
            player_state.weapon_attack_phase = -1;
        }
    } else if (player_state.weapon_charge_delay == 0) {
        player_state.attack_charge_state.current +=
            fixed6_ratio_step(
                player_state.physical_power,
                player_state.equipped_weapon_record->charge_rate) * 2;
        if (player_state.attack_charge_state.current >= 5001) {
            player_state.attack_charge_state.current = 5000;
        }
    } else {
        player_state.weapon_charge_delay--;
    }
}
