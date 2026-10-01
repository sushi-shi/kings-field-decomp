#include <kf/platform/prelude.h>
#include <kf/game/avatar.h>
#include <kf/lib/math.h>
#include <kf/renderer/projection.h>
#include <algorithm>

namespace {
constexpr unsigned index(AvatarBone bone) { return static_cast<unsigned>(bone); }
struct Arm { SVECTOR shoulder, elbow, hand; };
struct Rig {
    s16 neck_y;
    Arm arms[2]; // Right, left; imported bodies need not be symmetric.
    SVECTOR hip, knee;
    s16 arm_inner_x, arm_bottom_y;
    s16 shoulder_blend_y, shoulder_blend_x, shoulder_blend_width;
    s16 elbow_blend_y, elbow_blend_height;
    s16 leg_blend_y, leg_blend_height, knee_blend_y, knee_blend_height;
    s16 foot_half_width;
    s16 forearm_inner_x;
    s16 leg_inner_x=40, foot_back_z=100;
    s16 neck_z=0, foot_front_z=-250;
    s16 foot_center_x=0; // Zero uses the hip center; splayed feet need their own bound.
};
// Pivots and skinning boundaries use the imported, 2000-unit-tall meshes.
// Keep each body's clothing on the torso rather than applying a generic rig.
constexpr Rig standing1 {
    .neck_y=-1700,
    .arms={{{-240,-1580,0},{-240,-1280,-95},{-225,-900,-100}},
           {{240,-1580,0},{215,-1290,-40},{235,-930,-20}}},
    .hip={90,-1070,0}, .knee={90,-590,0},
    .arm_inner_x=215, .arm_bottom_y=-700,
    .shoulder_blend_y=-1470, .shoulder_blend_x=210, .shoulder_blend_width=65,
    .elbow_blend_y=-1320, .elbow_blend_height=160,
    .leg_blend_y=-1030, .leg_blend_height=160, .knee_blend_y=-650, .knee_blend_height=120,
    .foot_half_width=65, .forearm_inner_x=170,
    .leg_inner_x=20, .foot_back_z=110, .neck_z=0, .foot_front_z=-260,
};
constexpr Rig standing41 {
    .neck_y=-1640,
    .arms={{{-275,-1530,0},{-275,-1200,0},{-275,-860,0}},
           {{275,-1530,0},{275,-1200,0},{275,-860,0}}},
    .hip={140,-710,0}, .knee={140,-350,0},
    .arm_inner_x=190, .arm_bottom_y=-717,
    .shoulder_blend_y=-1400, .shoulder_blend_x=210, .shoulder_blend_width=40,
    .elbow_blend_y=-1240, .elbow_blend_height=80,
    .leg_blend_y=-690, .leg_blend_height=50, .knee_blend_y=-390, .knee_blend_height=120,
    .foot_half_width=110, .forearm_inner_x=190,
};
constexpr Rig standing39 {
    .neck_y=-1640,
    .arms={{{-250,-1560,0},{-265,-1270,0},{-275,-1000,0}},
           {{250,-1560,0},{265,-1270,0},{275,-1000,0}}},
    .hip={120,-710,0}, .knee={120,-350,0},
    .arm_inner_x=190, .arm_bottom_y=-900,
    .shoulder_blend_y=-1480, .shoulder_blend_x=195, .shoulder_blend_width=45,
    .elbow_blend_y=-1310, .elbow_blend_height=80,
    .leg_blend_y=-365, .leg_blend_height=60, .knee_blend_y=-390, .knee_blend_height=120,
    .foot_half_width=75, .forearm_inner_x=190,
};
constexpr Rig standing24 {
    .neck_y=-1690,
    .arms={{{-260,-1580,10},{-330,-1220,10},{-400,-835,18}},
           {{260,-1580,10},{300,-1220,10},{320,-820,18}}},
    .hip={110,-710,0}, .knee={110,-350,0},
    .arm_inner_x=190, .arm_bottom_y=-700,
    .shoulder_blend_y=-1450, .shoulder_blend_x=200, .shoulder_blend_width=70,
    .elbow_blend_y=-1260, .elbow_blend_height=100,
    .leg_blend_y=-690, .leg_blend_height=100, .knee_blend_y=-390, .knee_blend_height=120,
    .foot_half_width=100, .forearm_inner_x=245,
    .leg_inner_x=20, .foot_back_z=150,
};
constexpr Rig standing14 {
    .neck_y=-1690,
    .arms={{{-265,-1600,-40},{-365,-1170,-250},{-395,-1090,-625}},
           {{265,-1600,-40},{325,-1160,-50},{345,-800,-135}}},
    .hip={115,-710,0}, .knee={115,-350,0},
    .arm_inner_x=190, .arm_bottom_y=-650,
    .shoulder_blend_y=-1470, .shoulder_blend_x=200, .shoulder_blend_width=70,
    .elbow_blend_y=-1210, .elbow_blend_height=100,
    .leg_blend_y=-320, .leg_blend_height=60, .knee_blend_y=-390, .knee_blend_height=120,
    .foot_half_width=100, .forearm_inner_x=245,
    .leg_inner_x=10, .foot_back_z=100, .neck_z=-70,
};
constexpr Rig standing22 {
    .neck_y=-1700,
    .arms={{{-350,-1620,-240},{-365,-1320,-60},{-410,-840,-70}},
           {{350,-1620,-240},{360,-1320,-50},{360,-830,-50}}},
    .hip={130,-710,0}, .knee={130,-350,0},
    .arm_inner_x=275, .arm_bottom_y=-700,
    .shoulder_blend_y=-1540, .shoulder_blend_x=280, .shoulder_blend_width=70,
    .elbow_blend_y=-1350, .elbow_blend_height=100,
    .leg_blend_y=-190, .leg_blend_height=60, .knee_blend_y=-390, .knee_blend_height=120,
    .foot_half_width=90, .forearm_inner_x=300,
    .leg_inner_x=40, .foot_back_z=180, .neck_z=-400,
};
constexpr Rig standing5 {
    .neck_y=-1660,
    .arms={{{-260,-1510,20},{-250,-1310,-180},{-240,-907,-194}},
           {{275,-1490,40},{300,-1200,100},{240,-800,110}}},
    .hip={125,-840,0}, .knee={125,-500,0},
    .arm_inner_x=224, .arm_bottom_y=-650,
    .shoulder_blend_y=-1420, .shoulder_blend_x=220, .shoulder_blend_width=70,
    .elbow_blend_y=-1250, .elbow_blend_height=100,
    .leg_blend_y=-800, .leg_blend_height=80, .knee_blend_y=-550, .knee_blend_height=100,
    .foot_half_width=100, .forearm_inner_x=240,
    .leg_inner_x=40, .foot_back_z=140, .neck_z=-45, .foot_front_z=-260,
};
constexpr Rig standing19 {
    .neck_y=-1690,
    .arms={{{-230,-1510,-20},{-235,-1240,-105},{-219,-891,-100}},
           {{230,-1510,-20},{235,-1240,-105},{230,-880,-111}}},
    .hip={100,-710,0}, .knee={100,-350,0},
    .arm_inner_x=165, .arm_bottom_y=-780,
    .shoulder_blend_y=-1450, .shoulder_blend_x=180, .shoulder_blend_width=85,
    .elbow_blend_y=-1280, .elbow_blend_height=120,
    .leg_blend_y=-275, .leg_blend_height=50, .knee_blend_y=-390, .knee_blend_height=120,
    .foot_half_width=90, .forearm_inner_x=165,
    .leg_inner_x=8, .foot_back_z=100, .neck_z=-30,
};
constexpr Rig standing26 {
    .neck_y=-1680,
    .arms={{{-235,-1550,-20},{-305,-1360,-160},{-325,-1016,-217}},
           {{235,-1550,-20},{340,-1350,-135},{334,-1044,-200}}},
    .hip={90,-930,0}, .knee={90,-570,0},
    .arm_inner_x=190, .arm_bottom_y=-900,
    .shoulder_blend_y=-1460, .shoulder_blend_x=210, .shoulder_blend_width=60,
    .elbow_blend_y=-1390, .elbow_blend_height=120,
    .leg_blend_y=-1020, .leg_blend_height=100, .knee_blend_y=-640, .knee_blend_height=120,
    .foot_half_width=75, .forearm_inner_x=210,
    .leg_inner_x=20, .foot_back_z=75, .neck_z=-30, .foot_front_z=-200,
};
constexpr Rig standing27 {
    .neck_y=-1720,
    .arms={{{-365,-1530,0},{-375,-1350,-40},{-440,-1000,-170}},
           {{365,-1530,0},{375,-1350,-40},{390,-1030,-160}}},
    .hip={130,-950,0}, .knee={190,-630,0},
    .arm_inner_x=275, .arm_bottom_y=-780,
    .shoulder_blend_y=-1480, .shoulder_blend_x=270, .shoulder_blend_width=100,
    .elbow_blend_y=-1370, .elbow_blend_height=160,
    .leg_blend_y=-1010, .leg_blend_height=120, .knee_blend_y=-710, .knee_blend_height=140,
    .foot_half_width=110, .forearm_inner_x=280,
    .leg_inner_x=8, .foot_back_z=110, .neck_z=-35, .foot_front_z=-175, .foot_center_x=270,
};
const Rig *rig(unsigned avatar)
{
    switch (avatar) {
    case 1: return &standing1;
    case 5: return &standing5;
    case 14: return &standing14;
    case 19: return &standing19;
    case 22: return &standing22;
    case 24: return &standing24;
    case 26: return &standing26;
    case 27: return &standing27;
    case 39: return &standing39;
    case 41: return &standing41;
    default: return nullptr;
    }
}
bool head_vertex(unsigned avatar, const Rig &body, const KfAvatarVertex &vertex)
{
    // Beard/cap boundaries keep the hunched heads separate from raised collars.
    if (avatar==14) return vertex.y<-1460 && std::abs(vertex.x)<145 &&
        vertex.z<(vertex.y<-1750 ? 0 : -40) &&
        (vertex.y<-1650 || vertex.z<-175 || (std::abs(vertex.x)<65 && vertex.y<-1635 && vertex.z<-150));
    if (avatar==22) return vertex.y<-1550 && std::abs(vertex.x)<160 && vertex.z<-345;
    if (avatar==27) {
        if (vertex.y<-1840) return true;
        // The lower head overlaps the raised collar in XYZ and atlas bounds.
        // Keep the complete head component rigid using its actual neck outline.
        constexpr SVECTOR outline[] {
            {-111,-1831,13},{-100,-1831,13},{-97,-1819,-10},{-82,-1819,-70},
            {-81,-1770,-74},{-68,-1807,56},{-67,-1794,-85},{-59,-1770,-107},
            {-44,-1720,-100},{-22,-1807,-120},{0,-1807,-144},{0,-1801,68},
            {0,-1788,-152},{0,-1788,-120},{0,-1781,-13},{0,-1775,-68},{0,-1714,-111},
            {22,-1807,-120},{44,-1720,-100},{52,-1770,-107},{67,-1794,-85},
            {68,-1807,56},{81,-1770,-74},{82,-1819,-70},{97,-1819,-10},
            {100,-1831,13},{111,-1831,13},
        };
        for (const auto &point : outline)
            if (vertex.x==point.vx && vertex.y==point.vy && vertex.z==point.vz) return true;
        return false;
    }
    return vertex.y<body.neck_y;
}
MATRIX compose(const MATRIX &parent, const MATRIX &local)
{
    MATRIX result{};
    kf::matrix_multiply_rotation(parent, local, result);
    const auto offset = kf::render_transform_point(parent,
        {static_cast<s16>(local.t[0]),static_cast<s16>(local.t[1]),static_cast<s16>(local.t[2])});
    result.t[0]=offset.vx; result.t[1]=offset.vy; result.t[2]=offset.vz;
    return result;
}
void joint(AvatarPose &pose, AvatarBone bone, AvatarBone parent, SVECTOR pivot, SVECTOR angles)
{
    MATRIX local{};
    kf::matrix_set_rotation_xyz(angles,local);
    const auto rotated = kf::matrix_apply_rotation(local,pivot);
    local.t[0]=pivot.vx-rotated.vx; local.t[1]=pivot.vy-rotated.vy; local.t[2]=pivot.vz-rotated.vz;
    pose.bones[index(bone)] = compose(pose.bones[index(parent)],local);
}
struct AttackKey { s16 phase, shoulder, elbow, torso; };
constexpr AttackKey slash[] {{0,0,-200,0},{1600,-1300,650,-80},{2600,-1900,800,-120},
    {3072,-700,-100,100},{3500,100,0,80},{4096,0,-200,0}};
constexpr AttackKey thrust[] {{0,0,-200,0},{600,-600,450,-60},{1000,-1024,0,70},
    {1700,-1024,0,70},{2700,-500,350,-20},{4096,0,-200,0}};
AttackKey attack(s16 phase, bool stabbing)
{
    if (phase < 0) return {0,0,-200,0};
    const auto &keys = stabbing ? thrust : slash;
    phase = std::min<s16>(phase,4096);
    for (unsigned i=1; i<std::size(slash); ++i) {
        if (phase > keys[i].phase) continue;
        const auto &a=keys[i-1], &b=keys[i];
        const auto blend = [&](s16 from,s16 to) { return static_cast<s16>(from+(to-from)*(phase-a.phase)/(b.phase-a.phase)); };
        return {phase,blend(a.shoulder,b.shoulder),blend(a.elbow,b.elbow),blend(a.torso,b.torso)};
    }
    return {};
}
SVECTOR blend(SVECTOR a, SVECTOR b, s32 weight)
{
    return {static_cast<s16>(a.vx+((b.vx-a.vx)*weight>>12)),
        static_cast<s16>(a.vy+((b.vy-a.vy)*weight>>12)),static_cast<s16>(a.vz+((b.vz-a.vz)*weight>>12))};
}
} // namespace

bool avatar_has_rig(unsigned avatar) { return rig(avatar) != nullptr; }

MATRIX avatar_body_rotation(s16 camera_yaw)
{
    MATRIX rotation {};
    // Imported bodies face -Z; player movement faces (-sin(yaw), +cos(yaw)).
    matrix_set_rotation_y((KF_ANGLE_HALF_TURN + camera_yaw) & KF_ANGLE_WRAP_MASK, &rotation);
    return rotation;
}

void avatar_build_pose(u8 avatar, const AvatarMotion &motion, AvatarPose &pose)
{
    pose = {};
    for (auto &bone : pose.bones) bone.m[0][0]=bone.m[1][1]=bone.m[2][2]=4096;
    const auto *body = rig(avatar);
    if (!body) return;
    pose.rigged=true;
    pose.avatar=avatar;
    const auto forward=std::clamp<s32>(motion.forward,-180,180);
    const auto strafe=std::clamp<s32>(motion.strafe,-180,180);
    const auto speed=std::min<s32>(180,kf::length_square_root(forward*forward+strafe*strafe));
    const auto stride = kf::angle_sine(motion.walk_phase)*450/4096;
    const auto bend = attack(motion.attack_phase,motion.thrust);
    // Raise the free arm on release, hold briefly, then return to the walking
    // pose. The weapon arm retains its attack and its equipment attachment.
    const auto cast_ticks = std::min<u8>(motion.cast_ticks,avatar_cast_ticks);
    const s32 cast_weight = cast_ticks >= 8 ? (avatar_cast_ticks-cast_ticks)*1024
        : cast_ticks >= 4 ? 4096 : cast_ticks*1024;
    auto &root = pose.bones[index(AvatarBone::Root)];
    matrix_set_rotation_y(bend.torso,&root);
    root.t[1]=(-body->hip.vy*(4096-kf::angle_cosine(stride*speed/180)))>>12;
    for (bool left : {false,true}) {
        const s32 sign=left ? 1 : -1;
        const auto upper=left ? AvatarBone::LeftLeg : AvatarBone::RightLeg;
        const auto lower=left ? AvatarBone::LeftShin : AvatarBone::RightShin;
        const auto phase=motion.walk_phase+(left ? 2048 : 0);
        const auto wave=kf::angle_sine(phase);
        joint(pose,upper,AvatarBone::Root,{static_cast<s16>(sign*body->hip.vx),body->hip.vy,0},
            {static_cast<s16>(wave*450*forward/(4096*180)),0,
             static_cast<s16>(-wave*450*strafe/(4096*180))});
        const auto knee=std::max<s32>(0,-kf::angle_cosine(phase))*350*speed/(4096*180);
        joint(pose,lower,upper,{static_cast<s16>(sign*body->knee.vx),body->knee.vy,0},{static_cast<s16>(knee),0,0});
        // Check both sole edges as well as heel/toe: strafing rotates the foot
        // sideways. The hip seam stays under the tunic; collision is unchanged.
        s32 penetration=0;
        for (s16 dx : {static_cast<s16>(-body->foot_half_width),body->foot_half_width})
        for (s16 z : {body->foot_front_z,body->foot_back_z}) penetration=std::max(penetration,
            kf::render_transform_point(pose.bones[index(lower)],
                {static_cast<s16>(sign*(body->foot_center_x ? body->foot_center_x : body->hip.vx)+dx),0,z}).vy);
        pose.bones[index(upper)].t[1]-=penetration;
        pose.bones[index(lower)].t[1]-=penetration;
        const auto arm=left ? AvatarBone::LeftArm : AvatarBone::RightArm;
        const auto forearm=left ? AvatarBone::LeftForearm : AvatarBone::RightForearm;
        const auto swing=-wave*120*speed/(4096*180);
        const auto shoulder = !left && motion.attack_phase>=0 ? bend.shoulder : swing;
        const auto &limb=body->arms[left];
        joint(pose,arm,AvatarBone::Root,limb.shoulder,
            {static_cast<s16>(left ? shoulder+(-1024-shoulder)*cast_weight/4096 : shoulder),0,0});
        joint(pose,forearm,arm,limb.elbow,
            {static_cast<s16>(left ? -120+120*cast_weight/4096 : bend.elbow),0,0});
    }
    joint(pose,AvatarBone::Head,AvatarBone::Root,{0,body->neck_y,body->neck_z},
        {static_cast<s16>(std::clamp<s32>(motion.look_pitch,-256,256)),0,0});
}

void avatar_pose_vertex(const AvatarPose &pose, const KfAvatarVertex &vertex, SVECTOR &position, SVECTOR &normal)
{
    position={vertex.x,vertex.y,vertex.z}; normal={vertex.nx,vertex.ny,vertex.nz};
    if (!pose.rigged) return;
    const auto *body=rig(pose.avatar);
    if (!body) return;
    auto a=AvatarBone::Root, b=AvatarBone::Root;
    s32 part_weight=4096, joint_weight=0;
    const bool left=vertex.x>=0;
    // The flared sleeves need a wider waist boundary than the upper arms.
    const auto arm_inner=vertex.y < body->elbow_blend_y+body->elbow_blend_height
        ? body->arm_inner_x : body->forearm_inner_x;
    // Slot 5's hands lie in front of/behind the coat rather than beyond its
    // width. Include the inner fingers without dragging the coat's side panels.
    const bool inner_hand=pose.avatar==5 &&
        ((vertex.x<-140 && vertex.y>-1450 && vertex.z<-100) ||
         (vertex.x>170 && vertex.y>-1000 && vertex.x+vertex.z>285) ||
         (vertex.x>155 && vertex.y>-840));
    bool arm=(std::abs(vertex.x)>arm_inner || inner_hand) && vertex.y<body->arm_bottom_y;
    // Slot 1's opened forearms curve inward at the elbow and thumb. Include
    // their skin/bracelet surfaces without assigning the overlapping waist.
    if (pose.avatar==1) arm=vertex.y<body->arm_bottom_y &&
        (std::abs(vertex.x)>215 || (vertex.y>-1470 &&
         ((vertex.v>=190 && vertex.v<=221) || (vertex.v>=350 && vertex.v<=413)) &&
         (std::abs(vertex.x)>175 || (std::abs(vertex.x)>130 && vertex.z<-60) ||
          (std::abs(vertex.x)>150 && vertex.z<-20) ||
          (std::abs(vertex.x)>110 && vertex.z<-50 && vertex.y>-1160))));
    // The opened hands sit just in front of slot 19's dress at hip height.
    // Keep the dress panels fixed while including the inner thumb surfaces.
    if (pose.avatar==19 && !(vertex.y<-950 || std::abs(vertex.x)>230 || vertex.z<-95 ||
        (std::abs(vertex.x)>200 && vertex.z<-80))) arm=false;
    if (head_vertex(pose.avatar,*body,vertex)) {
        b=AvatarBone::Head;
        joint_weight=pose.avatar==14 || pose.avatar==22 || pose.avatar==27 ? 4096 : std::clamp((body->neck_y-vertex.y)*4096/60,0,4096);
    }
    else if (arm) {
        a=left ? AvatarBone::LeftArm : AvatarBone::RightArm;
        b=left ? AvatarBone::LeftForearm : AvatarBone::RightForearm;
        part_weight=vertex.y < body->shoulder_blend_y ?
            std::clamp((std::abs(vertex.x)-body->shoulder_blend_x)*4096/body->shoulder_blend_width,0,4096) : 4096;
        joint_weight=std::clamp((vertex.y-body->elbow_blend_y)*4096/body->elbow_blend_height,0,4096);
    } else if (vertex.y > body->leg_blend_y && std::abs(vertex.x)>body->leg_inner_x &&
        // Slot 1's long cloth panels and hip scabbard stay on the torso while
        // the skin, anklets and shoes move as complete legs underneath them.
        (pose.avatar!=1 || vertex.v<286 || (vertex.v>=350 && vertex.v<414)) &&
        (pose.avatar!=27 || vertex.y>-950 || vertex.v>=383)) {
        a=left ? AvatarBone::LeftLeg : AvatarBone::RightLeg;
        b=left ? AvatarBone::LeftShin : AvatarBone::RightShin;
        part_weight=std::clamp((vertex.y-body->leg_blend_y)*4096/body->leg_blend_height,0,4096);
        joint_weight=std::clamp((vertex.y-body->knee_blend_y)*4096/body->knee_blend_height,0,4096);
    }
    const auto point=[&](AvatarBone bone) { return kf::render_transform_point(pose.bones[index(bone)],position).narrowed(); };
    const auto direction=[&](AvatarBone bone) { return kf::matrix_apply_rotation(pose.bones[index(bone)],normal).narrowed(); };
    const auto p=blend(point(a),point(b),joint_weight);
    const auto n=blend(direction(a),direction(b),joint_weight);
    normal=blend(direction(AvatarBone::Root),n,part_weight);
    position=blend(point(AvatarBone::Root),p,part_weight);
    const auto length=kf::length_square_root(s32(normal.vx)*normal.vx+s32(normal.vy)*normal.vy+s32(normal.vz)*normal.vz);
    if (length) normal={static_cast<s16>(normal.vx*4096/length),static_cast<s16>(normal.vy*4096/length),static_cast<s16>(normal.vz*4096/length)};
}

MATRIX avatar_equipment_transform(const AvatarPose &pose, bool shield)
{
    MATRIX local{};
    const auto *body=rig(pose.avatar);
    if (!body) return local;
    const auto &hand=body->arms[shield].hand;
    if (shield) {
        kf::matrix_set_rotation_xyz({},local);
        local.t[0]=hand.vx+10; local.t[1]=hand.vy-240; local.t[2]=hand.vz-130;
    } else {
        // Inventory weapons point along -Y, with the grip at +350. In the
        // neutral hand pose their blade points down; arm animation carries it.
        kf::matrix_set_rotation_xyz({0,0,2048},local);
        local.t[0]=hand.vx; local.t[1]=hand.vy+350; local.t[2]=hand.vz;
    }
    return compose(pose.bones[index(shield ? AvatarBone::LeftForearm : AvatarBone::RightForearm)],local);
}
