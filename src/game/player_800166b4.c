#include <kf/address.h>
#include <kf/semantic_types.h>

extern s32 player_distance_to_point(
    s32 point_x, s32 point_y, s32 point_z, s32 max_distance, s32 point_height);
extern void player_apply_damage(
    u16 component0, u16 component1, u16 component2, u16 status_effect_flags,
    u16 component3, u16 component4, u16 scale_q12, u16 multiplier_tenths);

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
