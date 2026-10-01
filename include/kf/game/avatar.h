#pragma once
#include <kf/lib/geometry_types.h>
#include <kf/lib/avatar.h>

enum class AvatarBone : u8 { Root, RightArm, RightForearm, LeftArm, LeftForearm,
    RightLeg, RightShin, LeftLeg, LeftShin, Head, Count };
inline constexpr u8 avatar_cast_ticks = 12;
struct AvatarMotion {
    u16 walk_phase;
    s16 forward, strafe, attack_phase, look_pitch;
    bool thrust;
    u8 cast_ticks {};
};
struct AvatarPose {
    MATRIX bones[static_cast<unsigned>(AvatarBone::Count)] {};
    bool rigged {};
    u8 avatar {};
};
bool avatar_has_rig(unsigned avatar);
void avatar_build_pose(u8 avatar, const AvatarMotion &motion, AvatarPose &pose);
void avatar_pose_vertex(const AvatarPose &pose, const KfAvatarVertex &vertex,
    SVECTOR &position, SVECTOR &normal);
MATRIX avatar_equipment_transform(const AvatarPose &pose, bool shield);
