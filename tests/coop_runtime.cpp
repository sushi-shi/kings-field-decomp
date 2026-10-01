#include <kf/platform/prelude.h>
#include <filesystem>
#include <chrono>
#include <thread>
#include <GLES3/gl3.h>
#include <kf/game/game.h>
#include <kf/game/player.h>
#include <kf/game/world.h>
#include <kf/game/snapshot.h>
#include <kf/game/animation_cache.h>
#include <kf/game/party_runtime.h>
#include <kf/game/campaign.h>
#include <kf/game/player_actions.h>
#include <kf/game/avatar.h>
#include <kf/game/audio.h>
#include <kf/game/menu.h>
#include <kf/game/state.h>
#include <kf/game/graphics.h>
void map_event_update_wander(WorldState &, PlayerContext &);
void actor_spawn_action_effect(WorldState &, PlayerContext &, KfActorEffectCode, KfActorEffectSlot);

static void require(bool condition, const char *message)
{
    if (!condition) {
        std::fprintf(stderr, "%s\n", message);
        std::abort();
    }
}

// Snapshot fixtures need registered clip metadata, without loading or drawing
// resource models. Restore both owned animation data and the borrowed TMD view.
struct SnapshotAssetFixture {
    unsigned slot;
    KfAnimationData previous_animation;
    KfTmdResource previous_tmd;
    KfTmdHeader tmd {0x41, 0, 0};
    SnapshotAssetFixture(unsigned index, unsigned clips) : slot(index),
        previous_animation(std::move(game_graphics_runtime.asset_animations[index])),
        previous_tmd(game_graphics_runtime.asset_registry_tmds[index])
    {
        game_graphics_runtime.asset_animations[slot] = {};
        game_graphics_runtime.asset_animations[slot].clips.resize(clips);
        game_graphics_runtime.asset_registry_tmds[slot] = {&tmd, sizeof(tmd)};
    }
    ~SnapshotAssetFixture()
    {
        game_graphics_runtime.asset_animations[slot] = std::move(previous_animation);
        game_graphics_runtime.asset_registry_tmds[slot] = previous_tmd;
    }
    SnapshotAssetFixture(const SnapshotAssetFixture &) = delete;
    SnapshotAssetFixture &operator=(const SnapshotAssetFixture &) = delete;
};

static kf::net::Identity character_identity(std::uint64_t value)
{
    kf::net::Identity result {};
    for (unsigned byte = 0; byte < 8; ++byte) result[byte] = value >> (byte * 8);
    result.back() = 0xa5;
    return result;
}

struct Lifetime {
    int &count;
    explicit Lifetime(int &value) : count(value) { ++count; }
    ~Lifetime() { --count; }
};

static kf::FrameTask<int> delayed_value(int &alive)
{
    Lifetime lifetime(alive);
    co_await kf::FrameDelay {3};
    co_return 42;
}

static kf::FrameTask<void> nested_task(int &alive, int &result)
{
    Lifetime lifetime(alive);
    result = co_await delayed_value(alive);
    co_await kf::FrameDelay {1};
    ++result;
}

static void task_lifecycle()
{
    int alive = 0, result = 0;
    auto task = nested_task(alive, result);
    require(alive == 0, "Task construction must not run gameplay");
    task.advance();
    require(alive == 2 && result == 0, "Nested task did not suspend");
    task.advance();
    task.advance();
    require(result == 0, "A task resumed before its deadline");
    task.advance();
    require(result == 42 && alive == 1, "Nested result/continuation was lost");
    task.advance();
    require(task.done() && result == 43 && alive == 0, "Root did not finish");
    task = nested_task(alive, result);
    task.advance();
    task = {};
    require(alive == 0, "Cancelling a task leaked suspended nested state");

    int other_alive = 0, other_result = 0;
    task = nested_task(alive, result);
    auto other = nested_task(other_alive, other_result);
    task.advance();
    for (int i = 0; i < 5; ++i) other.advance();
    require(other.done() && other_result == 43 && alive == 2,
            "Independent tasks share a continuation/deadline");
}

static void persistent_online_profile()
{
    char path[] = "/tmp/kf-profile-XXXXXX";
    require(mkdtemp(path), "Cannot create isolated profile storage");
    require(kf::save_storage_start(path), "Cannot open isolated profile storage");
    std::array<u8,32> first, second;
    require(kf::online_profile_load(first) && first != std::array<u8,32>{}, "Cannot create a random profile");
    kf::save_storage_shutdown();
    require(kf::save_storage_start(path) && kf::online_profile_load(second) && first == second,
        "Reopening save storage changed the online identity");
    const auto file = std::filesystem::path(path) / "online-profile";
    const auto permissions = std::filesystem::status(file).permissions();
    require((permissions & (std::filesystem::perms::group_all | std::filesystem::perms::others_all)) == std::filesystem::perms::none,
        "Online credential was not stored privately");
    std::filesystem::resize_file(file, 1);
    require(!kf::online_profile_load(second) && std::filesystem::file_size(file) == 1,
        "Damaged profile was silently replaced with another identity");
    kf::save_storage_shutdown();
    std::filesystem::remove_all(path);
}

static void independent_characters_and_worlds()
{
    PlayerContext first {}, second {};
    first.state.vitals.current_hp = second.state.vitals.current_hp = 100;
    first.state.vitals.maximum_hp = second.state.vitals.maximum_hp = 100;
    first.state.physical_power = second.state.physical_power = 10;
    player_apply_damage(second, 20, 0, 0, KF_PLAYER_STATUS_CURSE, 0, 0, 4096, 10);
    require(second.state.vitals.current_hp < 100, "Real combat did not damage its target");
    require(first.state.vitals.current_hp == 100 && first.state.status_effect_flags == KF_PLAYER_STATUS_NONE,
            "Damage/status escaped the target character");
    player_adjust_hp(first, -25);
    require(first.state.vitals.current_hp == 75, "HP adjustment did not use the supplied character");
    player_apply_damage(second, 100, 0, 0, KF_PLAYER_STATUS_NONE, 0, 0, 4096, 10);
    require(second.state.vitals.current_hp == 0 && first.state.vitals.current_hp == 75,
            "Lethal damage affected another character");

    auto authority = std::make_unique<WorldState>();
    auto prediction = std::make_unique<WorldState>();
    collision_adjust_cell_occupancy(*authority, 5, 5, 1);
    require(authority->collision_flags.cells[5][5] == 1 && prediction->collision_flags.cells[5][5] == 0,
            "Collision updates escaped the supplied world");
    map_floor_script(*authority, KF_FLOOR_1).floor1.revival_enabled = KF_MAP_SCRIPT_SET;
    require(map_floor_script(*prediction, KF_FLOOR_1).floor1.revival_enabled == KF_MAP_SCRIPT_UNSET,
            "Campaign scripts share mutable world state");
}

static bool same_matrix(const MATRIX &a, const MATRIX &b)
{
    return a.m == b.m && a.t == b.t;
}

static bool same_pose(const AvatarPose &a, const AvatarPose &b)
{
    return a.rigged == b.rigged && a.avatar == b.avatar &&
        std::equal(std::begin(a.bones), std::end(a.bones), std::begin(b.bones), same_matrix);
}

static void avatar_animation_and_attachments()
{
    for (s16 yaw = 0; yaw < 4096; yaw += 128) {
        const auto forward = kf::matrix_apply_rotation(avatar_body_rotation(yaw), {0,0,-4096});
        require(std::abs(forward.vx + kf::angle_sine(yaw)) <= 2 &&
            std::abs(forward.vz - kf::angle_cosine(yaw)) <= 2,
            "Remote body faces a different direction from player movement");
    }
    AvatarPose idle, walking, swing;
    avatar_build_pose(41,{0,0,0,-1,0,false},idle);
    require(idle.rigged,"Standing character has no rig");
    avatar_build_pose(0,{1024,180,0,3072,0,false},swing);
    require(!swing.rigged,"Standing rig distorted a partial character");
    KfAvatarVertex vertex{275,-860,0,0,0,-4096,0,0,128,128,128,0};
    SVECTOR position, normal;
    avatar_pose_vertex(swing,vertex,position,normal);
    require(position.vx==275 && position.vy==-860 && normal.vz==-4096,"Unrigged body was deformed");
    avatar_build_pose(41,{0,0,0,2600,0,false},swing);
    const auto windup=kf::render_transform_point(avatar_equipment_transform(swing,false),{0,350,0});
    avatar_build_pose(41,{0,0,0,3072,0,false},swing);
    const auto strike=kf::render_transform_point(avatar_equipment_transform(swing,false),{0,-505,0});
    require(windup.vy < -1800 && strike.vz < -1000,"Weapon swing missed the authored windup/hit poses");
    vertex.x=-275;
    avatar_pose_vertex(swing,vertex,position,normal);
    const auto grip=kf::render_transform_point(avatar_equipment_transform(swing,false),{0,350,0});
    require(std::abs(position.vx-grip.vx)<4 && std::abs(position.vy-grip.vy)<4 && std::abs(position.vz-grip.vz)<4,
        "Weapon detached from the animated hand");
    avatar_build_pose(41,{0,0,0,1000,0,true},swing);
    require(kf::render_transform_point(avatar_equipment_transform(swing,false),{0,-470,0}).vz < -1000,
        "Thrust animation was not aligned with its early hit phase");
    for (const SVECTOR velocity : {SVECTOR{180,0,0},SVECTOR{0,0,180},SVECTOR{180,0,180},SVECTOR{-180,0,-180}})
    for (u16 phase=0; phase<4096; phase+=128) {
        avatar_build_pose(41,{phase,velocity.vx,velocity.vz,-1,0,false},walking);
        for (s16 x : {-241,-70,70,241}) for (s16 z : {-231,60}) {
            vertex={x,0,z,0,-4096,0,0,0,128,128,128,0};
            avatar_pose_vertex(walking,vertex,position,normal);
            require(position.vy<=2,"Animated boot penetrated the floor");
            require(std::abs(kf::length_square_root(s32(normal.vx)*normal.vx+s32(normal.vy)*normal.vy+s32(normal.vz)*normal.vz)-4096)<8,
                "Skinning changed normal length");
        }
    }
    avatar_build_pose(41,{1024,180,0,-1,0,false},walking);
    require(!same_pose(idle,walking),"Walking pose stayed static");
    AvatarPose casting;
    avatar_build_pose(41,{0,0,0,-1,0,false,6},casting);
    vertex={275,-860,0,0,0,-4096,0,0,128,128,128,0};
    avatar_pose_vertex(casting,vertex,position,normal);
    require(position.vz < -600 && std::abs(position.vy+1530)<8,
        "Casting hand did not extend in front of the body");
    vertex={239,-714,0,0,0,-4096,0,0,128,128,128,0};
    avatar_pose_vertex(casting,vertex,position,normal);
    require(position.vx==vertex.x && position.vy==vertex.y && position.vz==vertex.z,
        "Casting arm pulled the tunic hem away from the body");
    vertex={205,-858,-17,0,0,-4096,0,0,128,128,128,0};
    avatar_pose_vertex(casting,vertex,position,normal);
    require(position.vz < -500 && position.vy < -1400,
        "Inner hand vertices stayed behind when the arm cast");
    const auto cast_weapon = avatar_equipment_transform(casting,false);
    const auto idle_weapon = avatar_equipment_transform(idle,false);
    require(same_matrix(cast_weapon,idle_weapon),
        "Casting moved the weapon out of its idle hand");
    avatar_build_pose(41,{0,0,0,-1,0,false,0},casting);
    require(same_pose(idle,casting),"Casting did not return to idle");
    avatar_build_pose(41,{65535,-32768,32767,32767,32767,false},swing);
    vertex={275,-860,0,0,0,-4096,0,0,128,128,128,0};
    avatar_pose_vertex(swing,vertex,position,normal);
    require(std::abs(position.vx)<8192 && std::abs(position.vy)<8192 && std::abs(position.vz)<8192,
        "Extreme snapshot motion escaped presentation bounds");

    avatar_build_pose(39,{0,0,0,-1,0,false},idle);
    require(idle.rigged && avatar_has_rig(39) && !avatar_has_rig(43),
        "Standing body rig availability is incorrect");
    avatar_build_pose(39,{0,0,0,-1,0,false,6},casting);
    // These points lie on the imported body's inner hand, waist and long hem.
    vertex={221,-1004,-56,0,0,-4096,0,0,128,128,128,0};
    avatar_pose_vertex(casting,vertex,position,normal);
    require(position.vz < -500 && position.vy < -1450,
        "Second standing body's inner hand was not included in the casting arm");
    for (const SVECTOR point : {SVECTOR{195,-732,0},SVECTOR{195,-366,0},SVECTOR{171,-1073,0}}) {
        vertex.x=point.vx; vertex.y=point.vy; vertex.z=point.vz;
        avatar_pose_vertex(casting,vertex,position,normal);
        require(position.vx==point.vx && position.vy==point.vy && position.vz==point.vz,
            "Second standing body's casting arm pulled its tunic");
    }
    for (const auto motion : {AvatarMotion{1024,180,0,-1,0,false},
            AvatarMotion{0,0,0,3072,0,false},AvatarMotion{0,0,0,1000,0,true}}) {
        avatar_build_pose(39,motion,swing);
        vertex={-275,-1000,0,0,0,-4096,0,0,128,128,128,0};
        avatar_pose_vertex(swing,vertex,position,normal);
        const auto hand=kf::render_transform_point(avatar_equipment_transform(swing,false),{0,350,0});
        require(std::abs(position.vx-hand.vx)<4 && std::abs(position.vy-hand.vy)<4 && std::abs(position.vz-hand.vz)<4,
            "Second standing body's weapon detached from its hand");
        require(!same_pose(idle,swing),"Second standing body did not animate");
    }
    for (const SVECTOR velocity : {SVECTOR{180,0,0},SVECTOR{0,0,180},SVECTOR{180,0,180},SVECTOR{-180,0,-180}})
    for (u16 phase=0; phase<4096; phase+=128) {
        avatar_build_pose(39,{phase,velocity.vx,velocity.vz,-1,0,false},walking);
        for (s16 x : {-171,-49,49,171}) for (s16 z : {-195,37}) {
            vertex={x,0,z,0,-4096,0,0,0,128,128,128,0};
            avatar_pose_vertex(walking,vertex,position,normal);
            require(position.vy<=2,"Second standing body's foot penetrated the floor");
        }
    }
    avatar_build_pose(24,{0,0,0,-1,0,false},idle);
    require(idle.rigged && avatar_has_rig(24),"Coated standing body has no rig");
    avatar_build_pose(24,{0,0,0,-1,0,false,6},casting);
    for (const SVECTOR point : {SVECTOR{258,-1031,-37},SVECTOR{262,-733,-24},SVECTOR{320,-820,18}}) {
        vertex={point.vx,point.vy,point.vz,0,0,-4096,0,0,128,128,128,0};
        avatar_pose_vertex(casting,vertex,position,normal);
        require(position.vz < -500 && position.vy < -1500,
            "Coated body's inner sleeve or hand stayed behind while casting");
    }
    for (const SVECTOR point : {SVECTOR{230,-1068,0},SVECTOR{165,-1363,-111},SVECTOR{128,-718,111}}) {
        vertex={point.vx,point.vy,point.vz,0,0,-4096,0,0,128,128,128,0};
        avatar_pose_vertex(casting,vertex,position,normal);
        require(position.vx==point.vx && position.vy==point.vy && position.vz==point.vz,
            "Coated body's casting arm pulled a coat panel");
    }
    for (const auto motion : {AvatarMotion{1024,180,0,-1,0,false},
            AvatarMotion{0,0,0,3072,0,false},AvatarMotion{0,0,0,1000,0,true}}) {
        avatar_build_pose(24,motion,swing);
        vertex={-400,-835,18,0,0,-4096,0,0,128,128,128,0};
        avatar_pose_vertex(swing,vertex,position,normal);
        const auto hand=kf::render_transform_point(avatar_equipment_transform(swing,false),{0,350,0});
        require(std::abs(position.vx-hand.vx)<4 && std::abs(position.vy-hand.vy)<4 && std::abs(position.vz-hand.vz)<4,
            "Coated body's weapon detached from its asymmetric hand");
        require(!same_pose(idle,swing),"Coated body did not animate");
    }
    for (const SVECTOR velocity : {SVECTOR{180,0,0},SVECTOR{0,0,180},SVECTOR{180,0,180},SVECTOR{-180,0,-180}})
    for (u16 phase=0; phase<4096; phase+=128) {
        avatar_build_pose(24,{phase,velocity.vx,velocity.vz,-1,0,false},walking);
        for (s16 x : {-188,-23,23,188}) for (s16 z : {-187,127}) {
            vertex={x,0,z,0,-4096,0,0,0,128,128,128,0};
            avatar_pose_vertex(walking,vertex,position,normal);
            require(position.vy<=2,"Coated body's foot penetrated the floor");
        }
    }
    avatar_build_pose(14,{0,0,0,-1,0,false},idle);
    require(idle.rigged && avatar_has_rig(14),"Cane-bearing standing body has no rig");
    avatar_build_pose(14,{0,0,0,-1,0,false,6},casting);
    for (const SVECTOR point : {SVECTOR{306,-810,-198},SVECTOR{373,-825,-136},SVECTOR{345,-800,-135}}) {
        vertex={point.vx,point.vy,point.vz,0,0,-4096,0,0,128,128,128,0};
        avatar_pose_vertex(casting,vertex,position,normal);
        require(position.vz < -650 && position.vy < -1500,
            "Cane-bearing body's inner hand stayed behind while casting");
    }
    for (const auto motion : {AvatarMotion{1024,180,0,-1,0,false},
            AvatarMotion{0,0,0,3072,0,false},AvatarMotion{0,0,0,1000,0,true}}) {
        avatar_build_pose(14,motion,swing);
        vertex={-395,-1090,-625,0,0,-4096,0,0,128,128,128,0};
        avatar_pose_vertex(swing,vertex,position,normal);
        const auto hand=kf::render_transform_point(avatar_equipment_transform(swing,false),{0,350,0});
        require(std::abs(position.vx-hand.vx)<4 && std::abs(position.vy-hand.vy)<4 && std::abs(position.vz-hand.vz)<4,
            "Weapon detached from the extended hand");
    }
    avatar_build_pose(14,{0,0,0,-1,256,false},swing);
    for (const SVECTOR point : {SVECTOR{0,-1473,-236},SVECTOR{-62,-1638,-162},SVECTOR{-56,-1614,-188}}) {
        vertex={point.vx,point.vy,point.vz,0,0,-4096,0,0,128,128,128,0};
        avatar_pose_vertex(swing,vertex,position,normal);
        const auto head=kf::render_transform_point(swing.bones[static_cast<unsigned>(AvatarBone::Head)],point);
        require(position.vx==head.vx && position.vy==head.vy && position.vz==head.vz,
            "Beard separated from the head while looking");
    }
    for (const SVECTOR point : {SVECTOR{-55,-1781,49},SVECTOR{-79,-1639,-168},
            SVECTOR{0,-1473,-175},SVECTOR{43,-1683,-32}}) {
        vertex={point.vx,point.vy,point.vz,0,0,-4096,0,0,128,128,128,0};
        avatar_pose_vertex(swing,vertex,position,normal);
        require(position.vx==point.vx && position.vy==point.vy && position.vz==point.vz,
            "Looking pulled the raised collar into the head");
    }
    for (const SVECTOR velocity : {SVECTOR{180,0,0},SVECTOR{0,0,180},SVECTOR{180,0,180},SVECTOR{-180,0,-180}})
    for (u16 phase=0; phase<4096; phase+=128) {
        avatar_build_pose(14,{phase,velocity.vx,velocity.vz,-1,0,false},walking);
        for (const SVECTOR point : {SVECTOR{-29,0,5},SVECTOR{-132,-1,-200},SVECTOR{194,-2,-163},SVECTOR{58,-1,43}}) {
            vertex={point.vx,point.vy,point.vz,0,-4096,0,0,0,128,128,128,0};
            avatar_pose_vertex(walking,vertex,position,normal);
            require(position.vy<=2,"Cane-bearing body's foot penetrated the floor");
        }
    }
}

static void hunched_avatar_animation()
{
    AvatarPose idle, pose;
    avatar_build_pose(22,{0,0,0,-1,0,false},idle);
    require(idle.rigged && avatar_has_rig(22),"Hunched standing body has no rig");
    const auto point = [&](SVECTOR source) {
        KfAvatarVertex vertex{source.vx,source.vy,source.vz,0,0,-4096,0,0,128,128,128,0};
        SVECTOR position, normal;
        avatar_pose_vertex(pose,vertex,position,normal);
        return position;
    };
    avatar_build_pose(22,{0,0,0,-1,0,false,6},pose);
    for (const SVECTOR source : {SVECTOR{310,-813,-68},SVECTOR{324,-750,6},SVECTOR{360,-830,-50}}) {
        const auto hand=point(source);
        require(hand.vz<-850 && hand.vy<-1350,"Hunched body's casting arm left hand vertices behind");
    }
    for (const SVECTOR source : {SVECTOR{288,-803,-127},SVECTOR{285,-702,14},SVECTOR{195,-1822,-248}}) {
        const auto coat=point(source);
        require(coat.vx==source.vx && coat.vy==source.vy && coat.vz==source.vz,
            "Casting pulled the hunched body's coat or collar");
    }
    for (const auto motion : {AvatarMotion{1024,180,0,-1,0,false},
            AvatarMotion{0,0,0,3072,0,false},AvatarMotion{0,0,0,1000,0,true}}) {
        avatar_build_pose(22,motion,pose);
        const auto hand=point({-410,-840,-70});
        const auto grip=kf::render_transform_point(avatar_equipment_transform(pose,false),{0,350,0});
        require(std::abs(hand.vx-grip.vx)<4 && std::abs(hand.vy-grip.vy)<4 && std::abs(hand.vz-grip.vz)<4,
            "Hunched body's weapon detached from its hand");
    }
    avatar_build_pose(22,{0,0,0,-1,256,false},pose);
    for (const SVECTOR source : {SVECTOR{0,-1556,-596},SVECTOR{0,-1890,-412},SVECTOR{-147,-1860,-490}}) {
        const auto head=point(source);
        const auto expected=kf::render_transform_point(pose.bones[static_cast<unsigned>(AvatarBone::Head)],source);
        require(head.vx==expected.vx && head.vy==expected.vy && head.vz==expected.vz,
            "Hunched body's cap or beard separated while looking");
    }
    for (const SVECTOR source : {SVECTOR{195,-1822,-248},SVECTOR{0,-1742,86},SVECTOR{-127,-1473,-340}}) {
        const auto coat=point(source);
        require(coat.vx==source.vx && coat.vy==source.vy && coat.vz==source.vz,
            "Looking pulled the hunched body's collar");
    }
    for (const SVECTOR velocity : {SVECTOR{180,0,0},SVECTOR{0,0,180},SVECTOR{180,0,180},SVECTOR{-180,0,-180}})
    for (u16 phase=0; phase<4096; phase+=128) {
        avatar_build_pose(22,{phase,velocity.vx,velocity.vz,-1,0,false},pose);
        for (const SVECTOR source : {SVECTOR{-209,0,-89},SVECTOR{-56,0,-138},
                SVECTOR{56,0,-138},SVECTOR{209,0,-89},SVECTOR{-138,0,165},SVECTOR{138,0,165}})
            require(point(source).vy<=2,"Hunched body's foot penetrated the floor");
    }
}

static void raised_arm_avatar_animation()
{
    AvatarPose pose;
    const auto point = [&](SVECTOR source) {
        KfAvatarVertex vertex{source.vx,source.vy,source.vz,0,0,-4096,0,0,128,128,128,0};
        SVECTOR position, normal;
        avatar_pose_vertex(pose,vertex,position,normal);
        return position;
    };
    avatar_build_pose(5,{0,0,0,-1,0,false},pose);
    require(pose.rigged && avatar_has_rig(5),"Raised-arm body has no standing rig");
    require(point({-240,-907,-194}).vy>-1100,"Weapon hand stayed above the head at rest");
    avatar_build_pose(5,{0,0,0,-1,0,false,6},pose);
    for (const SVECTOR source : {SVECTOR{166,-725,187},SVECTOR{175,-927,139},
            SVECTOR{230,-915,62},SVECTOR{240,-800,110}}) {
        const auto hand=point(source);
        require(hand.vz<-500 && hand.vy<-1300,"Casting left inner hand vertices on the coat");
    }
    for (const SVECTOR source : {SVECTOR{234,-863,46},SVECTOR{162,-846,174},SVECTOR{-176,-846,-81}}) {
        const auto coat=point(source);
        require(coat.vx==source.vx && coat.vy==source.vy && coat.vz==source.vz,
            "Casting pulled the recentered body's coat");
    }
    for (const auto motion : {AvatarMotion{1024,180,0,-1,0,false},
            AvatarMotion{0,0,0,3072,0,false},AvatarMotion{0,0,0,1000,0,true}}) {
        avatar_build_pose(5,motion,pose);
        const auto hand=point({-240,-907,-194});
        const auto grip=kf::render_transform_point(avatar_equipment_transform(pose,false),{0,350,0});
        require(std::abs(hand.vx-grip.vx)<4 && std::abs(hand.vy-grip.vy)<4 && std::abs(hand.vz-grip.vz)<4,
            "Weapon detached from the lowered hand");
    }
    for (const SVECTOR velocity : {SVECTOR{180,0,0},SVECTOR{0,0,180},SVECTOR{180,0,180},SVECTOR{-180,0,-180}})
    for (u16 phase=0; phase<4096; phase+=128) {
        avatar_build_pose(5,{phase,velocity.vx,velocity.vz,-1,0,false},pose);
        for (s16 x : {-209,-58,58,209}) for (s16 z : {-255,125})
            require(point({x,0,z}).vy<=2,"Recentered body's long shoe penetrated the floor");
    }
}

static void clasped_avatar_animation()
{
    AvatarPose pose;
    const auto point = [&](SVECTOR source) {
        KfAvatarVertex vertex{source.vx,source.vy,source.vz,0,0,-4096,0,0,128,128,128,0};
        SVECTOR position, normal;
        avatar_pose_vertex(pose,vertex,position,normal);
        return position;
    };
    avatar_build_pose(19,{0,0,0,-1,0,false,6},pose);
    require(pose.rigged && avatar_has_rig(19),"Clasped-hands body has no standing rig");
    for (const SVECTOR source : {SVECTOR{179,-921,-109},SVECTOR{193,-915,-133},SVECTOR{230,-880,-111}}) {
        const auto hand=point(source);
        require(hand.vz<-550 && hand.vy<-1500,"Casting left opened-hand vertices on the dress");
    }
    for (const SVECTOR source : {SVECTOR{186,-923,-93},SVECTOR{-186,-923,-93},
            SVECTOR{223,-929,25},SVECTOR{-223,-929,25}}) {
        const auto dress=point(source);
        require(dress.vx==source.vx && dress.vy==source.vy && dress.vz==source.vz,
            "Casting pulled the dress panels beside the hands");
    }
    for (const auto motion : {AvatarMotion{1024,180,0,-1,0,false},
            AvatarMotion{0,0,0,3072,0,false},AvatarMotion{0,0,0,1000,0,true}}) {
        avatar_build_pose(19,motion,pose);
        const auto hand=point({-219,-891,-100});
        const auto grip=kf::render_transform_point(avatar_equipment_transform(pose,false),{0,350,0});
        require(std::abs(hand.vx-grip.vx)<4 && std::abs(hand.vy-grip.vy)<4 && std::abs(hand.vz-grip.vz)<4,
            "Weapon detached from the opened hand");
        for (const SVECTOR source : {SVECTOR{-203,-828,-85},SVECTOR{-175,-921,-98}}) {
            const auto finger=point(source);
            const auto expected=kf::render_transform_point(pose.bones[static_cast<unsigned>(AvatarBone::RightForearm)],source);
            require(finger.vx==expected.vx && finger.vy==expected.vy && finger.vz==expected.vz,
                "Inner fingers were left behind during an attack");
        }
    }
    for (const SVECTOR velocity : {SVECTOR{180,0,0},SVECTOR{0,0,180},SVECTOR{180,0,180},SVECTOR{-180,0,-180}})
    for (u16 phase=0; phase<4096; phase+=128) {
        avatar_build_pose(19,{phase,velocity.vx,velocity.vz,-1,0,false},pose);
        for (const SVECTOR source : {SVECTOR{-185,0,-160},SVECTOR{-99,0,-197},SVECTOR{-21,0,28},
                SVECTOR{21,0,28},SVECTOR{99,0,-197},SVECTOR{185,0,-160},SVECTOR{114,0,69}})
            require(point(source).vy<=2,"Dress body's shoe penetrated the floor");
    }
}

static void elf_avatar_animation()
{
    AvatarPose pose;
    const auto point = [&](SVECTOR source, u16 atlas_v=350) {
        KfAvatarVertex vertex{source.vx,source.vy,source.vz,0,0,-4096,0,atlas_v,128,128,128,0};
        SVECTOR position, normal;
        avatar_pose_vertex(pose,vertex,position,normal);
        return position;
    };
    avatar_build_pose(1,{0,0,0,-1,0,false,6},pose);
    require(pose.rigged && avatar_has_rig(1),"Standing elf has no animation rig");
    for (const SVECTOR source : {SVECTOR{163,-1193,-33},SVECTOR{187,-960,-77},SVECTOR{235,-970,16},SVECTOR{244,-823,22}}) {
        const auto hand=point(source);
        const auto expected=kf::render_transform_point(pose.bones[static_cast<unsigned>(AvatarBone::LeftForearm)],source);
        require(hand.vx==expected.vx && hand.vy==expected.vy && hand.vz==expected.vz && hand.vz<-350,
            "Elf's casting hand did not follow the complete forearm");
    }
    for (const SVECTOR source : {SVECTOR{-161,-971,15},SVECTOR{-103,-1022,-43},
            SVECTOR{-168,-1204,15},SVECTOR{100,-500,-90}}) {
        const auto cloth=point(source);
        require(cloth.vx==source.vx && cloth.vy==source.vy && cloth.vz==source.vz,
            "Elf's arm motion pulled the waist or long clothing");
    }
    for (const auto motion : {AvatarMotion{1024,180,0,-1,0,false},
            AvatarMotion{0,0,0,3072,0,false},AvatarMotion{0,0,0,1000,0,true}}) {
        avatar_build_pose(1,motion,pose);
        const auto hand=point({-225,-900,-100});
        const auto grip=kf::render_transform_point(avatar_equipment_transform(pose,false),{0,350,0});
        require(std::abs(hand.vx-grip.vx)<4 && std::abs(hand.vy-grip.vy)<4 && std::abs(hand.vz-grip.vz)<4,
            "Weapon detached from the elf's hand");
        for (const SVECTOR source : {SVECTOR{-115,-956,-82},SVECTOR{-281,-799,-81}}) {
            const auto finger=point(source);
            const auto expected=kf::render_transform_point(pose.bones[static_cast<unsigned>(AvatarBone::RightForearm)],source);
            require(finger.vx==expected.vx && finger.vy==expected.vy && finger.vz==expected.vz,
                "Elf's thumb or fingertips stayed behind during an attack");
        }
    }
    avatar_build_pose(1,{1024,180,0,-1,0,false},pose);
    const SVECTOR leg_source{100,-700,60};
    const auto leg=point(leg_source);
    const auto root=kf::render_transform_point(pose.bones[static_cast<unsigned>(AvatarBone::Root)],leg_source);
    require(std::abs(leg.vz-root.vz)>100,"Elf walking only moved the feet, leaving the legs fixed");
    for (const u16 material : {u16{500},u16{300}}) {
        const auto cloth=point(leg_source,material);
        require(cloth.vx==root.vx && cloth.vy==root.vy && cloth.vz==root.vz,
            "Elf walking pulled the long cloth or hip scabbard into a leg");
    }
    for (const SVECTOR velocity : {SVECTOR{180,0,0},SVECTOR{0,0,180},SVECTOR{180,0,180},SVECTOR{-180,0,-180}})
    for (u16 phase=0; phase<4096; phase+=128) {
        avatar_build_pose(1,{phase,velocity.vx,velocity.vz,-1,0,false},pose);
        for (s16 x : {-135,-45,45,135}) for (s16 z : {-252,100})
            require(point({x,0,z}).vy<=2,"Elf's foot penetrated the floor");
    }
    // Two actual Lyn triangles exercise import-time pose correction as well
    // as skinning: the upper seam must stay on the arm, not pin to the torso.
    constexpr std::array<KfAvatarVertex,6> source {{
        {-194,-1413,-88,0,0,-4096,0,350,128,128,128,0},
        {-192,-1388,-35,0,0,-4096,31,350,128,128,128,0},
        {-196,-1331,-99,0,0,-4096,9,413,128,128,128,0},
        {-163,-1316,-204,0,0,-4096,31,350,128,128,128,0},
        {-235,-1345,-145,0,0,-4096,23,413,128,128,128,0},
        {-139,-1295,-150,0,0,-4096,0,350,128,128,128,0}
    }};
    std::vector<u8> bytes {'K','F','A','1',1,0,1,0,1,0,158,1,2,0,0,0};
    const auto word=[&](u16 value) { bytes.push_back(value); bytes.push_back(value>>8); };
    for (const auto &vertex : source) {
        for (s16 value : {vertex.x,vertex.y,vertex.z,vertex.nx,vertex.ny,vertex.nz}) word(static_cast<u16>(value));
        word(vertex.u); word(vertex.v);
        bytes.insert(bytes.end(),{vertex.r,vertex.g,vertex.b,vertex.unlit});
    }
    bytes.resize(bytes.size()+256*414*4,255);
    char directory[]="/tmp/kf-elf-seam-XXXXXX";
    require(mkdtemp(directory),"Cannot create isolated elf mesh fixture");
    const auto path=std::filesystem::path(directory)/"characters.kfa";
    auto *file=std::fopen(path.c_str(),"wb");
    require(file && std::fwrite(bytes.data(),1,bytes.size(),file)==bytes.size(),"Cannot write elf mesh fixture");
    std::fclose(file);
    require(kf::avatars_load(path.c_str()),"Cannot load actual elf triangles");
    const auto *mesh=kf::avatar_mesh(1);
    require(mesh && mesh->info.triangles==2,"Elf correction changed mesh topology");
    require(mesh->vertices[3].x<-175 && mesh->vertices[3].y>-1220,
        "Elf's crossed forearm still pinches inward at the elbow");
    for (const s16 phase : {s16{-1},s16{1600},s16{2600},s16{3072},s16{3500}}) {
        avatar_build_pose(1,{0,0,0,phase,0,false},pose);
        const auto &seam=mesh->vertices[1];
        SVECTOR actual,normal;
        avatar_pose_vertex(pose,seam,actual,normal);
        const auto expected=kf::render_transform_point(pose.bones[static_cast<unsigned>(AvatarBone::RightArm)],
            {seam.x,seam.y,seam.z});
        require(actual.vx==expected.vx && actual.vy==expected.vy && actual.vz==expected.vz,
            "Elf's corrected elbow seam detached from the animated upper arm");
    }
    kf::avatars_release();
    std::filesystem::remove_all(directory);
}

static void armored_avatar_animation()
{
    AvatarPose pose;
    const auto point = [&](SVECTOR source, u16 atlas_v=32) {
        KfAvatarVertex vertex{source.vx,source.vy,source.vz,0,0,-4096,0,atlas_v,128,128,128,0};
        SVECTOR position, normal;
        avatar_pose_vertex(pose,vertex,position,normal);
        return position;
    };
    avatar_build_pose(27,{0,0,0,-1,0,false,6},pose);
    require(pose.rigged && avatar_has_rig(27),"Armored body has no animation rig");
    for (const SVECTOR source : {SVECTOR{284,-1010,-170},SVECTOR{480,-1065,-153},SVECTOR{390,-1030,-160}}) {
        const auto hand=point(source);
        const auto expected=kf::render_transform_point(pose.bones[static_cast<unsigned>(AvatarBone::LeftForearm)],source);
        require(hand.vx==expected.vx && hand.vy==expected.vy && hand.vz==expected.vz,
            "Armored body's casting hand did not move as a complete component");
    }
    for (const SVECTOR source : {SVECTOR{273,-981,37},SVECTOR{-273,-981,37},SVECTOR{242,-1154,37}}) {
        const auto armor=point(source);
        require(armor.vx==source.vx && armor.vy==source.vy && armor.vz==source.vz,
            "Casting pulled the armored body's waist plates");
    }
    for (const auto motion : {AvatarMotion{1024,180,0,-1,0,false},
            AvatarMotion{0,0,0,3072,0,false},AvatarMotion{0,0,0,1000,0,true}}) {
        avatar_build_pose(27,motion,pose);
        const auto hand=point({-440,-1000,-170});
        const auto grip=kf::render_transform_point(avatar_equipment_transform(pose,false),{0,350,0});
        require(std::abs(hand.vx-grip.vx)<4 && std::abs(hand.vy-grip.vy)<4 && std::abs(hand.vz-grip.vz)<4,
            "Weapon detached from the armored body's hand");
        for (const SVECTOR source : {SVECTOR{-372,-943,-169},SVECTOR{-516,-919,-251}}) {
            const auto finger=point(source);
            const auto expected=kf::render_transform_point(pose.bones[static_cast<unsigned>(AvatarBone::RightForearm)],source);
            require(finger.vx==expected.vx && finger.vy==expected.vy && finger.vz==expected.vz,
                "Armored body's fingertips stayed behind during an attack");
        }
    }
    avatar_build_pose(27,{0,0,0,-1,256,false},pose);
    for (const SVECTOR source : {SVECTOR{100,-1825,-87},SVECTOR{74,-1770,-118},SVECTOR{0,-1720,-81}}) {
        const auto collar=point(source);
        require(collar.vx==source.vx && collar.vy==source.vy && collar.vz==source.vz,
            "Looking up pulled the armored collar with the head");
    }
    const SVECTOR chin{0,-1714,-111};
    const auto head=point(chin);
    const auto expected=kf::render_transform_point(pose.bones[static_cast<unsigned>(AvatarBone::Head)],chin);
    require(head.vx==expected.vx && head.vy==expected.vy && head.vz==expected.vz,
        "Armored body's chin separated from its head");
    for (const SVECTOR velocity : {SVECTOR{180,0,0},SVECTOR{0,0,180},SVECTOR{180,0,180},SVECTOR{-180,0,-180}})
    for (u16 phase=0; phase<4096; phase+=128) {
        avatar_build_pose(27,{phase,velocity.vx,velocity.vz,-1,0,false},pose);
        for (s16 x : {-378,-197,170,347}) for (s16 z : {-163,107})
            require(point({x,0,z}).vy<=2,"Armored body's splayed foot penetrated the floor");
    }
}

static void vested_avatar_animation()
{
    AvatarPose pose;
    const auto point = [&](SVECTOR source) {
        KfAvatarVertex vertex{source.vx,source.vy,source.vz,0,0,-4096,0,0,128,128,128,0};
        SVECTOR position, normal;
        avatar_pose_vertex(pose,vertex,position,normal);
        return position;
    };
    avatar_build_pose(26,{0,0,0,-1,0,false,6},pose);
    require(pose.rigged && avatar_has_rig(26),"Vested body has no standing rig");
    for (const SVECTOR source : {SVECTOR{283,-1038,-177},SVECTOR{397,-1101,-243},SVECTOR{334,-1044,-200}}) {
        const auto hand=point(source);
        const auto expected=kf::render_transform_point(pose.bones[static_cast<unsigned>(AvatarBone::LeftForearm)],source);
        require(hand.vx==expected.vx && hand.vy==expected.vy && hand.vz==expected.vz && hand.vz<-400,
            "Casting left the vested body's fingers behind");
    }
    for (const SVECTOR source : {SVECTOR{0,-1502,-155},SVECTOR{20,-1483,-155},
            SVECTOR{0,-1463,-151},SVECTOR{160,-1100,0},SVECTOR{-160,-1100,0}}) {
        const auto vest=point(source);
        require(vest.vx==source.vx && vest.vy==source.vy && vest.vz==source.vz,
            "Casting pulled the vest or chest ornament");
    }
    for (const auto motion : {AvatarMotion{1024,180,0,-1,0,false},
            AvatarMotion{0,0,0,3072,0,false},AvatarMotion{0,0,0,1000,0,true}}) {
        avatar_build_pose(26,motion,pose);
        const auto hand=point({-325,-1016,-217});
        const auto grip=kf::render_transform_point(avatar_equipment_transform(pose,false),{0,350,0});
        require(std::abs(hand.vx-grip.vx)<4 && std::abs(hand.vy-grip.vy)<4 && std::abs(hand.vz-grip.vz)<4,
            "Weapon detached from the vested body's hand");
        for (const SVECTOR source : {SVECTOR{-370,-991,-158},SVECTOR{-265,-952,-214}}) {
            const auto finger=point(source);
            const auto expected=kf::render_transform_point(pose.bones[static_cast<unsigned>(AvatarBone::RightForearm)],source);
            require(finger.vx==expected.vx && finger.vy==expected.vy && finger.vz==expected.vz,
                "Vested body's fingertips did not follow the weapon arm");
        }
    }
    for (const SVECTOR velocity : {SVECTOR{180,0,0},SVECTOR{0,0,180},SVECTOR{180,0,180},SVECTOR{-180,0,-180}})
    for (u16 phase=0; phase<4096; phase+=128) {
        avatar_build_pose(26,{phase,velocity.vx,velocity.vz,-1,0,false},pose);
        for (s16 x : {-141,-21,21,141}) for (s16 z : {-191,70})
            require(point({x,0,z}).vy<=2,"Vested body's shoe penetrated the floor");
    }
}

static void character_pack_and_selection()
{
    using namespace kf::net;
    std::vector<u8> bytes{'K','F','A','1',1,0,1,0,23,0,1,0,1,0,0,0};
    bytes.resize(16 + 60, 0);
    bytes.resize(16 + 60 + 1024, 255);
    const std::vector<u8> second(bytes.begin()+8,bytes.end());
    const auto second_at=bytes.size();
    bytes.insert(bytes.end(),second.begin(),second.end());
    bytes[6]=2;
    bytes[second_at]=43;
    char directory[] = "/tmp/kf-avatars-XXXXXX";
    require(mkdtemp(directory), "Cannot create isolated character resources");
    const auto path = std::filesystem::path(directory) / "characters.kfa";
    auto *file = std::fopen(path.c_str(), "wb");
    require(file && std::fwrite(bytes.data(), 1, bytes.size(), file) == bytes.size(), "Cannot write synthetic character");
    std::fclose(file);
    require(kf::avatars_load(path.c_str()), "Cannot load validated character pack");
    require(kf::avatar_mesh(23) && !kf::avatar_mesh(41) && kf::avatar_mesh(43), "Pack lost stable sparse model IDs");
    const std::string hash = kf::avatars_hash();
    require(hash.size() == 64 && !kf::avatars_load("/missing/character/pack") && hash == kf::avatars_hash(),
        "Failed pack replacement changed the active compatibility hash");
    auto world = std::make_unique<WorldState>();
    world->party.enabled = true;
    world->epoch = 1;
    auto &member = world->party.members[1];
    member.generation = 7;
    member.presence = PartyPresence::Living;
    member.connected = true;
    member.player.state.vitals.current_hp = 100;
    member.player.state.weapon_attack_phase = KF_WEAPON_ATTACK_INACTIVE;
    PlayerCommandState state;
    Command request{{MessageKind::Command, 1, 1, 7}, CommandKind::ChooseAvatar, 23, 0}, decoded;
    std::vector<u8> packet;
    require(command_encode(request, packet) && command_decode(packet, decoded) && party_apply_command(*world, 1, state, decoded),
        "Validated guest character selection failed");
    require(member.avatar == 23 && world->party.members[0].avatar == 41, "Character selection affected another member");
    request.header.sequence = 2;
    request.object = 41;
    require(!party_apply_command(*world, 1, state, request) && member.avatar == 23, "Missing character was accepted");
    request.header.sequence = 3;
    request.object = 43;
    require(command_encode(request, packet) && command_decode(packet, decoded) && party_apply_command(*world, 1, state, decoded)
            && member.avatar == 43, "Appended map-object character could not be selected");
    request.object = KF_AVATAR_SLOTS;
    require(!command_encode(request, packet), "Protocol accepted an out-of-range character");
    kf::avatars_release();
    require(!kf::avatar_mesh(23) && std::string(kf::avatars_hash()) == "no-avatars-v1", "Character release left stale views");
    std::filesystem::remove_all(directory);
}

static void authoritative_menu_actions()
{
    using namespace kf::net;
    auto world = std::make_unique<WorldState>();
    world->party.enabled = true;
    world->party.local_slot = 1;
    auto &member = world->party.members[1];
    member.presence = PartyPresence::Living;
    member.connected = true;
    member.generation = 7;
    auto &player = member.player;
    player.local_view = false;
    player.party_slot = 1;
    player.state.vitals = {100, 20, 50, 50};
    player.state.magic = player.state.physical_power = 10;
    player.state.weapon_attack_phase = KF_WEAPON_ATTACK_INACTIVE;
    player.state.equipped_weapon_id = player.state.equipped_head_armor_id = player.state.equipped_body_armor_id =
        player.state.equipped_shield_id = player.state.equipped_arm_armor_id = player.state.equipped_leg_armor_id =
        player.state.equipped_accessory_id = KF_OBJECT_NONE;
    player.state.selected_magic_id = KF_MAGIC_NONE;
    player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_MEDICINAL_HERB)] = 3;
    world->party.members[0].player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_MEDICINAL_HERB)] = 9;
    u32 sequence = 0;
    PlayerCommandState command_state;
    auto apply = [&](CommandKind kind, u16 object, u16 argument = 0) {
        Command command {{MessageKind::Command, world->epoch, ++sequence, member.generation}, kind, object, argument}, decoded;
        std::vector<u8> bytes;
        require(command_encode(command, bytes) && command_decode(bytes, decoded), "Menu command did not cross the protocol codec");
        return party_apply_command(*world, 1, command_state, decoded);
    };
    const auto herb = kf_enum_encode<u16>(KF_ITEM_MEDICINAL_HERB);
    require(apply(CommandKind::UseItem, herb) && player.state.vitals.current_hp == 45 && player.item_stock[0][herb] == 2,
            "Host did not atomically consume the guest's herb and heal that character");
    Command replay {{MessageKind::Command, world->epoch, sequence, member.generation}, CommandKind::UseItem, herb, 0};
    require(!party_apply_command(*world, 1, command_state, replay) && player.item_stock[0][herb] == 2,
            "Duplicate command spent another item");
    require(!apply(CommandKind::UseItem, 256 + herb) && player.item_stock[0][herb] == 2,
            "Wide wire item wrapped into a valid inventory entry");
    replay.header.sequence = sequence + 1;
    ++replay.header.epoch;
    require(!party_apply_command(*world, 1, command_state, replay) && command_state.last_sequence == sequence, "Stale epoch changed command state");
    replay.header.epoch = world->epoch;
    ++replay.header.generation;
    require(!party_apply_command(*world, 1, command_state, replay), "Another character generation spent inventory");
    player.state.weapon_attack_phase = 1;
    require(!apply(CommandKind::UseItem, herb), "Menu action bypassed an active attack");
    player.state.weapon_attack_phase = KF_WEAPON_ATTACK_INACTIVE;
    member.connected = false;
    require(!apply(CommandKind::UseItem, herb), "Disconnected character executed a command");
    member.connected = true;
    player.state.vitals.current_hp = 0;
    require(!apply(CommandKind::UseItem, herb) && player.item_stock[0][herb] == 2, "Consumable revived a dead character");
    player.state.vitals.current_hp = 90;
    require(apply(CommandKind::UseItem, herb) && player.state.vitals.current_hp == 100, "Healing exceeded maximum HP");
    const auto healing = kf_enum_encode<u16>(KF_MAGIC_HEALING);
    world->effects.magic.entries[healing].mp_cost = 7;
    require(!apply(CommandKind::UseMagic, healing), "Unlearned support magic was accepted");
    require(player.cast_pose_ticks == 0, "Rejected spell started a cast gesture");
    player.learned_magic[healing] = KF_MAGIC_LEARNED;
    require(apply(CommandKind::UseMagic, healing) && player.state.vitals.current_mp == 43, "Support magic did not spend personal MP");
    require(player.cast_pose_ticks == avatar_cast_ticks, "Support spell lost its cast gesture");
    player.cast_pose_ticks = 3;
    player.state.vitals.current_mp = 6;
    require(!apply(CommandKind::UseMagic, healing) && player.state.vitals.current_mp == 6, "Insufficient MP underflowed");
    require(player.cast_pose_ticks == 3, "Rejected spell restarted a cast gesture");
    const auto fire = kf_enum_encode<u16>(KF_MAGIC_FIRE_BALL);
    require(!apply(CommandKind::SelectMagic, fire), "Unlearned combat spell was equipped");
    player.learned_magic[fire] = KF_MAGIC_LEARNED;
    require(apply(CommandKind::SelectMagic, fire) && player.state.selected_magic_record == &world->effects.magic.entries[fire],
            "Combat spell selection lost its authoritative record");
    const auto plate = kf_enum_encode<u16>(KF_ITEM_FULL_PLATE);
    require(!apply(CommandKind::Equip, plate, kf_enum_encode<u16>(KF_EQUIP_MENU_BODY)), "Unowned armor was equipped");
    player.item_stock[0][plate] = 1;
    player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_GAUNTLET)] = 1;
    require(!apply(CommandKind::Equip, plate, kf_enum_encode<u16>(KF_EQUIP_MENU_WEAPON)), "Armor was accepted as a weapon");
    require(apply(CommandKind::Equip, plate, kf_enum_encode<u16>(KF_EQUIP_MENU_BODY)), "Owned body armor was rejected");
    require(!apply(CommandKind::Equip, kf_enum_encode<u16>(KF_ITEM_GAUNTLET), kf_enum_encode<u16>(KF_EQUIP_MENU_ARM)),
            "Full plate allowed separate arm armor");
    require(!apply(CommandKind::DropItem, plate), "Last equipped item was discarded");
    player.item_stock[0][plate] = 2;
    require(apply(CommandKind::DropItem, plate) && player.item_stock[0][plate] == 1, "Spare equipped item was not discarded");
    require(world->party.members[0].player.item_stock[0][herb] == 9, "Guest action changed host inventory");

    auto &shop = world->map.events[0];
    shop.state = KF_MAP_EVENT_ACTIVE;
    shop.behavior = KF_MAP_EVENT_BEHAVIOR_SHOP;
    shop.character_id = kf_enum_decode<KfCharacterId>(1);
    shop.dialogue = {1, 1, 1, 0};
    shop.dialogue_pages.last_page[0] = 3;
    shop.radius = 400;
    const auto probe = vector_yaw_probe_xz(player.state.camera_position,
        player.state.camera_rotation.vy, MAP_INTERACTION_PROBE_DISTANCE);
    shop.reference_position = {probe.x, 0, probe.z};
    player.state.gold = 100;
    player.item_stock[1][herb] = 1;
    item_buy_prices[herb][0] = 30;
    item_sell_prices[herb][0] = 10;
    require(!apply(CommandKind::Buy, herb, 1), "Trade without an authorized interaction succeeded");
    require(apply(CommandKind::Interact, 0) && command_state.event == 0 &&
        command_state.interaction.shop == 1 && command_state.interaction.page == 1 && !shop.dialogue.page_delay,
        "Host did not capture dialogue/shop context before presentation");
    const auto before_buy = player.item_stock[0][herb];
    require(apply(CommandKind::Buy, herb, 1) && player.state.gold == 70 &&
        player.item_stock[0][herb] == before_buy + 1 && player.item_stock[1][herb] == 1,
        "Trade changed the wrong inventory, price or replenishing shop stock");
    require(!apply(CommandKind::Buy, herb, 2), "Client switched to an unvisited shop");
    player.state.camera_position.vx += 10000;
    require(!apply(CommandKind::Buy, herb, 1) && player.state.gold == 70, "Trade continued after leaving the merchant");
    player.state.camera_position.vx -= 10000;
    require(apply(CommandKind::Sell, herb, 1) && player.state.gold == 80 && player.item_stock[0][herb] == before_buy,
        "Selling did not update personal stock and gold atomically");
    player.state.gold = 0;
    require(!apply(CommandKind::Buy, herb, 1), "Trade underflowed personal gold");
    player.state.gold = 100;
    const auto cross = kf_enum_encode<u16>(KF_ITEM_GOLD_CROSS);
    player.item_stock[1][cross] = 1;
    item_buy_prices[cross][0] = 40;
    require(apply(CommandKind::Buy, cross, 1) && !player.item_stock[1][cross] &&
        !apply(CommandKind::Buy, cross, 1), "Limited merchant stock was duplicated");
    require(!apply(CommandKind::Sell, plate, 1), "Last equipped armor was sold");
    player.state.gold = std::numeric_limits<u32>::max();
    require(!apply(CommandKind::Sell, herb, 1), "Sale overflowed gold");
    require(apply(CommandKind::Cancel, 0) && command_state.event == -1 &&
        shop.dialogue.page_delay == KF_DIALOGUE_PAGE_DELAY_TICKS, "Closing dialogue did not release interaction state");
    require(!apply(CommandKind::Buy, herb, 1), "Closed shop authorized another purchase");
    require(world->party.members[0].player.item_stock[0][herb] == 9, "Trading changed another player's stock");

    PlayerActions actions;
    player.actions = &actions;
    auto menu = player_request_action(player, CommandKind::UseItem, herb);
    menu.advance();
    require(!menu.done() && actions.pending, "Menu did not wait for host authorization");
    auto snapshot = std::make_unique<WorldState>();
    world_clone_simulation(*world, *snapshot);
    require(!snapshot->party.members[1].player.actions, "Prediction copy retained the live menu action channel");
    player.state.hud_gauges_enabled = KF_PLAYER_OPTION_ON;
    snapshot->party.members[1].player.state.hud_gauges_enabled = KF_PLAYER_OPTION_OFF;
    world_apply_snapshot(*snapshot, *world);
    require(player.actions == &actions && !menu.done() && actions.pending, "Snapshot destroyed local menu state");
    require(player.state.hud_gauges_enabled == KF_PLAYER_OPTION_ON, "Host snapshot overwrote local presentation settings");
    actions.accepted = actions.completed = true;
    menu.advance();
    require(menu.done() && menu.await_resume() && !actions.pending && actions.next_sequence == 2,
            "Menu confirmation did not resume after the host response");
    world->prediction = true;
    require(!apply(CommandKind::UseItem, herb), "Prediction executed an authoritative inventory action");
}

static void authoritative_object_activation()
{
    using namespace kf::net;
    auto world = std::make_unique<WorldState>();
    world->party.enabled = true;
    auto &member = world->party.members[1];
    member.presence = PartyPresence::Living;
    member.connected = true;
    member.generation = 2;
    auto &player = member.player;
    player.local_view = false;
    player.state.vitals = {100, 100, 50, 50};
    player.state.camera_position = {10000, 0, 10000};
    player.state.weapon_attack_phase = KF_WEAPON_ATTACK_INACTIVE;
    for (auto &event : world->map.events) event.state = KF_MAP_EVENT_FREE;
    for (auto &object : world->objects.objects) object.object_id = KF_OBJECT_NONE;
    const auto probe = vector_yaw_probe_xz(player.state.camera_position, 0, MAP_INTERACTION_PROBE_DISTANCE);
    PlayerCommandState commands;
    u32 sequence = 0;
    auto activate = [&](u16 index, KfObjectId expected) {
        Command command {{MessageKind::Command, world->epoch, ++sequence, member.generation},
            CommandKind::ActivateObject, index, kf_enum_encode<u16>(expected)}, decoded;
        std::vector<u8> bytes;
        require(command_encode(command, bytes) && command_decode(bytes, decoded), "Object activation did not cross the protocol codec");
        return party_apply_command(*world, 1, commands, decoded);
    };
    auto &door = world->objects.objects[4];
    door.object_id = KF_MAP_OBJECT_LIFTING_GATE;
    door.action = KF_MAP_OBJECT_OP_NONE;
    door.position = {probe.x, 0, probe.z};
    door.link.fields.link_id = 7;
    auto &definition = world->objects.definitions.entries[kf_enum_encode<u8>(door.object_id)];
    definition.behavior_type = KF_MAP_OBJECT_OP_LIFT_DOOR;
    definition.interaction_radius = 200;
    require(!activate(4, door.object_id) && door.action == KF_MAP_OBJECT_OP_NONE, "Guest opened a locked door");
    door.link.fields.link_id = KF_MAP_LINK_NONE;
    require(!activate(KF_MAP_OBJECT_CAPACITY, door.object_id), "Out-of-range object slot was accepted");
    require(!activate(4, KF_MAP_OBJECT_HINGED_DOOR), "Stale object identity activated a replacement");
    door.position.vx += 20000;
    require(!activate(4, door.object_id), "Guest activated a distant object");
    door.position.vx -= 20000;
    door.rotation.angles.y = KF_ANGLE_QUARTER_TURN;
    require(!activate(4, door.object_id), "Guest activated a door while facing across it");
    door.rotation.angles.y = 0;
    auto &event = world->map.events[0];
    event.state = KF_MAP_EVENT_ACTIVE;
    event.radius = 400;
    event.reference_position = {probe.x, 0, probe.z};
    require(!activate(4, door.object_id), "Object activation bypassed the NPC interaction priority");
    event.state = KF_MAP_EVENT_FREE;
    require(activate(4, door.object_id) && door.action == KF_MAP_OBJECT_OP_LIFT_DOOR,
        "Guest could not start an unlocked nearby lift door");
    door.action_timer = KF_MAP_OBJECT_LIFT_OPEN_LAST;
    require(!activate(4, door.object_id) && door.action_timer == KF_MAP_OBJECT_LIFT_OPEN_LAST,
        "Repeated door activation restarted an in-flight action");

    door = {};
    door.object_id = KF_MAP_OBJECT_HINGED_DOOR;
    door.action = KF_MAP_OBJECT_OP_NONE;
    door.position = {probe.x - KF_MAP_TILE_SIZE, 0, probe.z + 550};
    door.link.fields.link_id = KF_MAP_LINK_NONE;
    auto &hinged = world->objects.definitions.entries[kf_enum_encode<u8>(door.object_id)];
    hinged.behavior_type = KF_MAP_OBJECT_OP_HINGED_DOOR;
    hinged.interaction_radius = 200;
    auto &partner = world->objects.objects[5];
    partner = door;
    partner.object_id = KF_MAP_OBJECT_HINGED_DOOR_PARTNER;
    partner.position.vx = probe.x + KF_MAP_TILE_SIZE;
    auto &partner_definition = world->objects.definitions.entries[kf_enum_encode<u8>(partner.object_id)];
    partner_definition.behavior_type = KF_MAP_OBJECT_OP_HINGED_DOOR_PARTNER;
    partner_definition.interaction_radius = 200;
    door.link.fields.link_id = 7;
    require(!activate(5, partner.object_id) && partner.action == KF_MAP_OBJECT_OP_NONE && door.action == KF_MAP_OBJECT_OP_NONE,
        "Guest bypassed a paired door's lock through its partner");
    door.link.fields.link_id = KF_MAP_LINK_NONE;
    require(activate(5, partner.object_id) && door.action == KF_MAP_OBJECT_OP_HINGED_DOOR &&
        partner.action == KF_MAP_OBJECT_OP_HINGED_DOOR_PARTNER &&
        door.link.fields.action_parameter.object_index == 5 && partner.link.fields.action_parameter.object_index == 4,
        "Guest activation did not start and link both door leaves");

    door.object_id = partner.object_id = KF_OBJECT_NONE;
    auto &switch_object = world->objects.objects[6];
    switch_object.object_id = KF_MAP_OBJECT_EFFECT_SWITCH;
    switch_object.position = {probe.x, 0, probe.z};
    switch_object.action = KF_MAP_OBJECT_OP_EFFECT_SWITCH;
    switch_object.link.fields.link_id = 9;
    auto &switch_definition = world->objects.definitions.entries[kf_enum_encode<u8>(switch_object.object_id)];
    switch_definition.behavior_type = KF_MAP_OBJECT_OP_EFFECT_SWITCH;
    switch_definition.interaction_radius = 200;
    require(activate(6, switch_object.object_id) && switch_object.action_timer == KF_MAP_OBJECT_SWITCH_FORWARD,
        "Guest could not activate a linked switch");
    switch_object.link.fields.link_id = KF_MAP_LINK_NONE;
    switch_object.action_timer = KF_MAP_OBJECT_SWITCH_DISABLED;
    require(!activate(6, switch_object.object_id) && switch_object.action_timer == KF_MAP_OBJECT_SWITCH_DISABLED,
        "Guest reactivated a disabled switch");
    switch_definition.behavior_type = KF_MAP_OBJECT_OP_ITEM_PICKUP;
    require(!activate(6, switch_object.object_id), "Activation command accepted an unsupported object operation");
    switch_definition.behavior_type = KF_MAP_OBJECT_OP_RESTORE_POINT;
    player.state.vitals.current_hp = 17;
    switch_object.link.fields.link_id = 9;
    require(!activate(6, switch_object.object_id) && player.state.vitals.current_hp == 17, "Locked restoration point healed a guest");
    switch_object.link.fields.link_id = KF_MAP_LINK_NONE;
    require(activate(6, switch_object.object_id) && player.state.vitals.current_hp == player.state.vitals.maximum_hp,
        "Guest activation did not restore vitals at an unlocked restoration point");
    switch_definition.behavior_type = KF_MAP_OBJECT_OP_SAVE_POINT;
    player.party_slot = 1;
    player.state.vitals.current_hp = 17;
    CampaignRuntime campaign;
    campaign.checkpoint = {1, 2, 3};
    world->campaign = &campaign;
    auto &spectator = world->party.members[2];
    spectator.presence = PartyPresence::Spectating;
    require(activate(6, switch_object.object_id) && player.state.vitals.current_hp == 17 &&
        spectator.presence == PartyPresence::Spectating && !campaign.writing &&
        campaign.checkpoint == std::vector<u8>({1, 2, 3}),
        "Guest save-point inspection wrote a save or changed party health");
    switch_object.position.vx += 20000;
    require(!activate(6, switch_object.object_id), "Guest inspected a distant save point");
    switch_object.position.vx -= 20000;
    world->campaign = nullptr;
    switch_object.object_id = KF_MAP_OBJECT_SIGNBOARD;
    auto &sign = world->objects.definitions.entries[kf_enum_encode<u8>(switch_object.object_id)];
    sign.behavior_type = KF_MAP_OBJECT_OP_SCREEN_IMAGE;
    switch_object.link.fields.link_id = 3;
    require(activate(6, switch_object.object_id), "Guest could not read a nearby sign");
    switch_object.link.fields.link_id = 100;
    require(!activate(6, switch_object.object_id), "Sign accepted an out-of-range image number");
    require(map_object_image_valid(KF_MAP_OBJECT_INSCRIPTION_PANEL, 99) &&
        !map_object_image_valid(KF_MAP_OBJECT_LIFTING_GATE, 3), "Image presentation accepted an unrelated object identity");
}

static void personal_loot()
{
    using namespace kf::net;
    auto world = std::make_unique<WorldState>();
    world->party.enabled = true;
    map_object_pool_clear(*world);
    for (auto &event : world->map.events) event.state = KF_MAP_EVENT_FREE;
    for (u8 slot = 0; slot < 2; ++slot) {
        auto &member = world->party.members[slot];
        member.presence = PartyPresence::Living;
        member.connected = true;
        member.generation = 1;
        member.player.party_slot = slot;
        member.player.state.vitals = {100, 100, 50, 50};
        member.player.state.weapon_attack_phase = KF_WEAPON_ATTACK_INACTIVE;
        member.player.state.camera_position = {10000, 0, 10000};
    }
    auto &first = world->party.members[0];
    auto &second = world->party.members[1];
    const auto probe = vector_yaw_probe_xz(first.player.state.camera_position, 0, MAP_INTERACTION_PROBE_DISTANCE);
    const VECTOR position {probe.x, 0, probe.z};
    const auto herb = kf_enum_encode<u16>(KF_ITEM_MEDICINAL_HERB);
    world->objects.definitions.entries[herb].behavior_type = KF_MAP_OBJECT_OP_ITEM_PICKUP;
    world->objects.definitions.entries[herb].interaction_radius = 200;
    PlayerCommandState commands[2];
    u32 sequence[2] {};
    auto take = [&](u8 slot, u16 index, u8 component, KfObjectId item, u32 generation) {
        Command command {{MessageKind::Command, world->epoch, ++sequence[slot], 1}, CommandKind::TakeLoot,
            index, static_cast<u16>((kf_enum_encode<u16>(item) << 8) | component), generation}, decoded;
        std::vector<u8> bytes;
        require(command_encode(command, bytes) && command_decode(bytes, decoded), "Loot command did not cross the protocol codec");
        return party_apply_command(*world, slot, commands[slot], decoded);
    };
    auto &object = world->objects.objects[4];
    object.object_id = KF_ITEM_MEDICINAL_HERB;
    object.position = position;
    require(take(0, 4, 0, object.object_id, object.generation) && first.player.item_stock[0][herb] == 1 &&
        object.object_id == KF_ITEM_MEDICINAL_HERB && !player_loot_visible(*world, 0, 4) && player_loot_visible(*world, 1, 4),
        "A personal pickup removed another player's copy or remained visible to its claimant");
    require(!take(0, 4, 0, object.object_id, object.generation) && first.player.item_stock[0][herb] == 1,
        "Repeated pickup request duplicated personal loot");
    second.player.item_stock[0][herb] = KF_ITEM_STACK_CAPACITY;
    require(!take(1, 4, 0, object.object_id, object.generation) && second.loot_claims[0][4] == 0,
        "Full inventory consumed a loot claim");
    second.player.item_stock[0][herb] = 0;
    require(!take(1, 4, 0, KF_ITEM_ANTIDOTE_HERB, object.generation), "Pickup granted a different item from the confirmed preview");
    object.position.vx += 20000;
    require(!take(1, 4, 0, object.object_id, object.generation), "Pickup ignored current proximity");
    object.position = position;
    require(take(1, 4, 0, object.object_id, object.generation) && second.player.item_stock[0][herb] == 1,
        "Second party member could not take their copy");
    world->floor = KF_FLOOR_2;
    require(take(0, 4, 0, object.object_id, object.generation), "Same object slot on another floor shared its claim");
    world->floor = KF_FLOOR_1;
    require(!take(0, 4, 0, object.object_id, object.generation), "Returning to a floor forgot personal loot");

    auto &container = world->objects.objects[5];
    container.object_id = KF_MAP_OBJECT_STONE_CONTAINER_LID;
    container.position = position;
    auto &definition = world->objects.definitions.entries[kf_enum_encode<u8>(container.object_id)];
    definition.behavior_type = KF_MAP_OBJECT_OP_HINGED_CONTAINER;
    definition.interaction_radius = 200;
    container.link.hinged_container.link_id = 3;
    for (auto &item : container.link.hinged_container.item_ids) item = KF_OBJECT_NONE;
    container.link.hinged_container.item_ids[2] = KF_ITEM_MEDICINAL_HERB;
    require(!take(0, 5, 2, KF_ITEM_MEDICINAL_HERB, container.generation), "Locked chest granted loot");
    container.link.hinged_container.link_id = KF_MAP_LINK_NONE;
    container.rotation.angles.y = KF_ANGLE_HALF_TURN;
    require(!take(0, 5, 2, KF_ITEM_MEDICINAL_HERB, container.generation), "Chest pickup bypassed facing");
    container.rotation.angles.y = 0;
    require(take(0, 5, 2, KF_ITEM_MEDICINAL_HERB, container.generation) && first.loot_claims[0][5] == 4 &&
        container.link.hinged_container.item_ids[2] == KF_ITEM_MEDICINAL_HERB,
        "Partially empty chest lost its later item or cleared shared contents");
    require(take(1, 5, 2, KF_ITEM_MEDICINAL_HERB, container.generation), "Chest item was not personal");

    auto &gold = world->objects.objects[6];
    gold.object_id = KF_ITEM_GOLD_COIN;
    gold.position = position;
    gold.link.gold_amount = 73;
    world->objects.definitions.entries[kf_enum_encode<u8>(gold.object_id)].behavior_type = KF_MAP_OBJECT_OP_GOLD_PICKUP;
    first.player.state.gold = std::numeric_limits<u32>::max();
    require(!take(0, 6, 0, KF_ITEM_GOLD_COIN, gold.generation) && !first.loot_claims[0][6], "Gold overflow consumed a claim");
    first.player.state.gold = 0;
    require(take(0, 6, 0, KF_ITEM_GOLD_COIN, gold.generation) && take(1, 6, 0, KF_ITEM_GOLD_COIN, gold.generation) &&
        first.player.state.gold == 73 && second.player.state.gold == 73, "Gold was not granted independently");

    map_object_spawn_drop(*world, KF_MAP_OBJECT_DROP_FROM_DEFINITION, KF_ITEM_MEDICINAL_HERB, &position, 0);
    auto &drop = world->objects.objects[KF_MAP_OBJECT_DEFINITION_DROP_FIRST];
    const auto old_generation = drop.generation;
    require(take(0, KF_MAP_OBJECT_DEFINITION_DROP_FIRST, 0, drop.object_id, old_generation), "Fresh enemy drop was unavailable");
    for (int i = KF_MAP_OBJECT_DEFINITION_DROP_FIRST; i < KF_MAP_OBJECT_PLACEMENT_DROP_FIRST; ++i) {
        world->objects.objects[i] = drop;
        world->objects.objects[i].link.fields.spawn.sequence = 0;
    }
    world->objects.definition_drop_sequence = 0;
    map_object_spawn_drop(*world, KF_MAP_OBJECT_DROP_FROM_DEFINITION, KF_ITEM_MEDICINAL_HERB, &position, 0);
    require(drop.generation != old_generation && !first.loot_claims[0][KF_MAP_OBJECT_DEFINITION_DROP_FIRST],
        "Reused enemy-drop slot retained the previous generation or claims");
    require(!take(0, KF_MAP_OBJECT_DEFINITION_DROP_FIRST, 0, drop.object_id, old_generation) &&
        take(0, KF_MAP_OBJECT_DEFINITION_DROP_FIRST, 0, drop.object_id, drop.generation),
        "Reused drop accepted an old confirmation or rejected its fresh generation");
}

static void authoritative_world_items()
{
    using namespace kf::net;
    auto world = std::make_unique<WorldState>();
    world->party.enabled = true;
    map_object_pool_clear(*world);
    effect_pool_reset(*world);
    auto &member = world->party.members[1];
    member.presence = PartyPresence::Living;
    member.connected = true;
    member.generation = 1;
    auto &player = member.player;
    player.party_slot = 1;
    player.local_view = false;
    player.state.audio_effects_enabled = KF_PLAYER_OPTION_OFF;
    player.state.vitals = {100, 100, 50, 50};
    player.state.weapon_attack_phase = KF_WEAPON_ATTACK_INACTIVE;
    player.state.camera_position = {10000, 0, 10000};
    player.state.progress_state.current_floor = KF_FLOOR_1;
    const auto probe = vector_yaw_probe_xz(player.state.camera_position, 0, MAP_INTERACTION_PROBE_DISTANCE);
    auto &gate = world->objects.objects[4];
    gate.object_id = KF_MAP_OBJECT_LIFTING_GATE;
    gate.position = {probe.x, 0, probe.z};
    world->objects.definitions.entries[kf_enum_encode<u8>(gate.object_id)].behavior_type = KF_MAP_OBJECT_OP_LIFT_DOOR;
    gate.link.fields.link_id = kf_enum_encode<u8>(KF_ITEM_DUNGEON_KEY);
    u32 sequence = 0;
    PlayerCommandState state;
    auto use = [&](KfObjectId item) {
        Command command {{MessageKind::Command, world->epoch, ++sequence, 1}, CommandKind::UseItem, kf_enum_encode<u16>(item), 0}, decoded;
        std::vector<u8> bytes;
        require(command_encode(command, bytes) && command_decode(bytes, decoded), "World-item command did not cross the protocol codec");
        return party_apply_command(*world, 1, state, decoded);
    };
    require(!use(KF_ITEM_DUNGEON_KEY), "Unowned key unlocked a shared object");
    player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_KEY_OF_THE_DEAD)] = 1;
    require(!use(KF_ITEM_KEY_OF_THE_DEAD) && gate.link.fields.link_id == kf_enum_encode<u8>(KF_ITEM_DUNGEON_KEY),
        "Wrong key bypassed an object lock");
    player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_DUNGEON_KEY)] = 1;
    gate.position.vx += 20000;
    require(!use(KF_ITEM_DUNGEON_KEY), "Key ignored interaction distance");
    gate.position.vx -= 20000;
    require(use(KF_ITEM_DUNGEON_KEY) && gate.link.fields.link_id == KF_MAP_LINK_NONE &&
        player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_DUNGEON_KEY)] == 1, "Guest key failed to unlock the shared gate or was consumed");
    require(!use(KF_ITEM_DUNGEON_KEY), "Repeated key use claimed to unlock an already open lock");

    auto &seal = world->objects.objects[5];
    seal.object_id = KF_ITEM_WATER_SEAL_STONE;
    seal.position = {probe.x, 0, probe.z};
    world->objects.definitions.entries[kf_enum_encode<u8>(seal.object_id)].behavior_type = KF_MAP_OBJECT_OP_REVEAL_MAP_PIECE;
    seal.link.fields.link_id = 130;
    gate.link.fields.link_id = 130;
    gate.position.vx += 20000;
    player.item_stock[0][kf_enum_encode<u8>(seal.object_id)] = 2;
    world->party.members[0].player.item_stock[0][kf_enum_encode<u8>(seal.object_id)] = 1;
    require(use(seal.object_id) && seal.link.fields.link_id == KF_MAP_LINK_NONE && gate.action == KF_MAP_OBJECT_OP_LIFT_DOOR &&
        player.item_stock[0][kf_enum_encode<u8>(seal.object_id)] == 0 &&
        world->party.members[0].player.item_stock[0][kf_enum_encode<u8>(seal.object_id)] == 1,
        "Seal failed to trigger its shared link or consumed another member's inventory");
    map_object_pool_clear_link(*world, 130);
    require(gate.link.fields.link_id == KF_MAP_LINK_NONE, "Link clearing skipped a live door");

    player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_ILLUSION_STAFF)] = 1;
    require(use(KF_ITEM_ILLUSION_STAFF) && player.state.illusion_staff_timer == 1000 &&
        player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_ILLUSION_STAFF)] == 0, "Guest illusion staff did not apply and consume atomically");
    require(!use(KF_ITEM_ILLUSION_STAFF), "Empty illusion staff stack was reused");
    require(!use(KF_ITEM_GREEN_DRAGON_STAFF) && !state.return_to_entry, "Unowned return staff queued party travel");
    player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_GREEN_DRAGON_STAFF)] = 1;
    const auto before = player.state.camera_position;
    require(use(KF_ITEM_GREEN_DRAGON_STAFF) && state.return_to_entry &&
        player.state.camera_position.vx == before.vx && player.state.camera_position.vz == before.vz &&
        player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_GREEN_DRAGON_STAFF)] == 1,
        "Return staff failed to queue shared travel, moved before the barrier, or was consumed");
    require(!use(KF_ITEM_GREEN_DRAGON_STAFF), "Repeated return-staff use queued another pending transition");
    player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_HARP)] = 1;
    require(!use(KF_ITEM_HARP), "Harp changed an unsupported floor");
    world->floor = player.state.progress_state.current_floor = KF_FLOOR_2;
    require(use(KF_ITEM_HARP) && world->effects.records[0].kind == KF_EFFECT_KIND_FLOOR_DEFORMATION &&
        world->effects.records[0].rotation.vector.vy == 4, "Guest harp did not start the original floor-two deformation");
    require(!use(KF_ITEM_HARP), "Repeated harp use created overlapping floor deformations");
    effect_pool_reset(*world);
    world->floor = player.state.progress_state.current_floor = KF_FLOOR_3;
    require(use(KF_ITEM_HARP) && world->effects.records[0].rotation.vector.vx == 4 &&
        world->effects.records[0].rotation.vector.vy == 1, "Guest harp used the wrong floor-three segments");
    effect_pool_reset(*world);
    for (auto &effect : world->effects.records) { effect.type = KF_EFFECT_FLOOR_DEFORM_TYPE; effect.kind = KF_MAGIC_FIRE_BALL; }
    require(!use(KF_ITEM_HARP), "Harp reported success with an exhausted effect pool");
}

static void shared_quest_rewards()
{
    using namespace kf::net;
    auto world = std::make_unique<WorldState>();
    world->party.enabled = true;
    map_object_pool_clear(*world);
    for (auto &event : world->map.events) event.state = KF_MAP_EVENT_FREE;
    for (u8 slot = 0; slot < 3; ++slot) {
        auto &member = world->party.members[slot];
        member.character_id = character_identity(slot + 1);
        member.generation = 1;
        member.presence = slot == 2 ? PartyPresence::Disconnected : PartyPresence::Living;
        member.connected = slot != 2;
        auto &player = member.player;
        player.party_slot = slot;
        player.local_view = false;
        player.state.vitals = {100, 100, 50, 50};
        player.state.weapon_attack_phase = KF_WEAPON_ATTACK_INACTIVE;
        player.state.progress_state.highest_floor = player.state.progress_state.current_floor = KF_FLOOR_1;
        player.state.camera_position = {10000, 0, 10000};
    }
    auto &initiator = world->party.members[1].player;
    const auto probe = vector_yaw_probe_xz(initiator.state.camera_position, 0, MAP_INTERACTION_PROBE_DISTANCE);
    PlayerCommandState commands;
    u32 sequence = 0;
    auto interact = [&](u16 index, CommandKind kind = CommandKind::Interact) {
        Command command {{MessageKind::Command, world->epoch, ++sequence, 1}, kind, index, 0}, decoded;
        std::vector<u8> bytes;
        require(command_encode(command, bytes) && command_decode(bytes, decoded), "Quest command did not cross the protocol codec");
        return party_apply_command(*world, 1, commands, decoded);
    };
    auto set_event = [&](int index, KfCharacterId character, u8 stage) {
        for (auto &event : world->map.events) event.state = KF_MAP_EVENT_FREE;
        auto &event = world->map.events[index];
        event = {};
        event.state = KF_MAP_EVENT_ACTIVE;
        event.behavior = KF_MAP_EVENT_BEHAVIOR_WANDER;
        event.character_id = character;
        event.radius = 400;
        event.reference_position = {probe.x, 0, probe.z};
        event.dialogue = {stage, stage, 1, 0};
        event.dialogue_pages.last_page[stage - 1] = 7;
    };
    const auto key = kf_enum_encode<u8>(KF_ITEM_KEY_OF_THE_DEAD);
    const auto cross = kf_enum_encode<u8>(KF_ITEM_GOLD_CROSS);
    set_event(2, KF_CHARACTER_KEY_OF_THE_DEAD_EXCHANGE, 1);
    initiator.item_stock[0][cross] = 2;
    world->party.members[0].player.item_stock[0][cross] = 3;
    require(interact(2) && world->party.quest_rewards == static_cast<u32>(PartyReward::KeyOfTheDead),
        "NPC exchange did not complete the shared quest");
    for (unsigned slot = 0; slot < 3; ++slot)
        require(world->party.members[slot].player.item_stock[0][key] == 1 &&
            world->party.members[slot].quest_rewards == world->party.quest_rewards,
            "Exchange skipped a present or disconnected campaign character");
    require(initiator.item_stock[0][cross] == 1 && world->party.members[0].player.item_stock[0][cross] == 3,
        "Shared quest consumed another member's offering");
    set_event(2, KF_CHARACTER_KEY_OF_THE_DEAD_EXCHANGE, 1);
    require(interact(2) && initiator.item_stock[0][cross] == 1 && initiator.item_stock[0][key] == 1,
        "A repeated dialogue state duplicated the reward or spent another offering");
    initiator.item_stock[0][key] = 0;
    party_grant_quest_rewards(*world);
    require(!initiator.item_stock[0][key], "Previously received quest item was regenerated after being spent");

    auto &late = world->party.members[3];
    late.presence = PartyPresence::Waiting;
    party_grant_quest_rewards(*world);
    require(!late.quest_rewards && !late.player.item_stock[0][key], "Waiting slot received rewards before character initialization");
    late.presence = PartyPresence::Spectating;
    late.character_id = character_identity(4);
    late.player.local_view = false;
    late.player.item_stock[0][key] = KF_ITEM_STACK_CAPACITY;
    party_grant_quest_rewards(*world);
    require(!late.quest_rewards && late.player.item_stock[0][key] == KF_ITEM_STACK_CAPACITY,
        "Full stack lost its pending reward or overflowed");
    --late.player.item_stock[0][key];
    party_grant_quest_rewards(*world);
    require(late.quest_rewards == world->party.quest_rewards && late.player.item_stock[0][key] == KF_ITEM_STACK_CAPACITY,
        "Late spectator did not receive a pending reward after making room");

    set_event(2, KF_CHARACTER_HEALING_EXCHANGE, 2);
    world->floor = initiator.state.progress_state.current_floor = initiator.state.progress_state.highest_floor = KF_FLOOR_2;
    initiator.item_stock[0][kf_enum_encode<u8>(KF_ITEM_MIRROR_OF_TRUTH)] = 1;
    require(interact(2), "Healing exchange was rejected");
    set_event(1, KF_CHARACTER_HARP_EXCHANGE, 2);
    initiator.item_stock[0][kf_enum_encode<u8>(KF_ITEM_DRAGON_KING_GRASS_FRUIT)] = 1;
    require(interact(1), "Harp exchange was rejected");
    for (const auto &member : world->party.members)
        require(member.player.learned_magic[kf_enum_encode<u8>(KF_MAGIC_HEALING)] == KF_MAGIC_LEARNED &&
            member.player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_HARP)] == 1,
            "Healing/harp exchange did not reach every admitted character");

    world->floor = initiator.state.progress_state.current_floor = initiator.state.progress_state.highest_floor = KF_FLOOR_3;
    set_event(KF_FLOOR3_FIRE_BALL_EVENT, kf_enum_decode<KfCharacterId>(10), 3);
    require(interact(KF_FLOOR3_FIRE_BALL_EVENT) && interact(KF_FLOOR3_FIRE_BALL_EVENT, CommandKind::Cancel),
        "Guest dialogue could not complete its fire-ball reward trigger");
    for (const auto &member : world->party.members)
        require(member.player.learned_magic[kf_enum_encode<u8>(KF_MAGIC_FIRE_BALL)] == KF_MAGIC_LEARNED,
            "Guest fire-ball dialogue failed to reward the party");
    world->prediction = true;
    party_complete_quest(*world, PartyReward::SanctuaryMagic);
    require(!(world->party.quest_rewards & static_cast<u32>(PartyReward::SanctuaryMagic)), "Prediction completed a shared quest");
    world->prediction = false;
    initiator.state.motion_state.map_cell.x = 15;
    initiator.state.motion_state.map_cell.z = 64;
    initiator.state.vitals.current_hp = 37;
    auto sanctuary = map_ambient_script_floor3(*world, world->party.members[0].player);
    sanctuary.advance();
    require(sanctuary.done() && initiator.state.vitals.current_hp == initiator.state.vitals.maximum_hp,
        "Guest reaching the sanctuary did not trigger restoration independently of the host");
    for (const auto &member : world->party.members)
        require(member.player.learned_magic[kf_enum_encode<u8>(KF_MAGIC_BLESS)] == KF_MAGIC_LEARNED &&
            member.player.learned_magic[kf_enum_encode<u8>(KF_MAGIC_RESIST_FIRE)] == KF_MAGIC_LEARNED,
            "Sanctuary reward did not teach both spells");
    late.presence = PartyPresence::Waiting;
    const auto growth = player_level_growth_table[0];
    for (auto &cell : world->collision.linear) cell = KF_MAP_CELL_FLOOR;
    actor_pool_clear(*world);
    map_object_pool_clear(*world);
    player_level_growth_table[0].maximum_hp = 100;
    player_level_growth_table[0].maximum_mp = 50;
    require(party_admit(*world, 3, character_identity(99), false) && late.character_id == character_identity(99) &&
        late.player.state.progress_state.level == 1 && late.player.state.experience == 0 &&
        late.player.state.progress_state.highest_floor == KF_FLOOR_3 &&
        late.quest_rewards == world->party.quest_rewards && late.player.item_stock[0][key] == 1 &&
        late.player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_HARP)] == 1 &&
        late.player.learned_magic[kf_enum_encode<u8>(KF_MAGIC_HEALING)] == KF_MAGIC_LEARNED,
        "New character admission did not initialize level one and catch up completed quests");
    player_level_growth_table[0] = growth;
    late.presence = PartyPresence::Disconnected;
    require(!party_admit(*world, 3, character_identity(100), true), "Returning character inherited another identity's reward ledger");
    require(party_admit(*world, 3, character_identity(99), true) && late.player.item_stock[0][key] == 1,
        "Returning character received a second copy of a completed quest reward");
}

static void party_entry_return()
{
    auto world = std::make_unique<WorldState>();
    world->party.enabled = true;
    world->floor = KF_FLOOR_5;
    world->variant = KF_FLOOR5_ENTRY_VARIANT;
    const auto &entry = floor_entry_cells[4];
    for (auto &cell : world->floor_height.linear) cell = 3;
    for (auto &cell : world->collision.linear) cell = KF_MAP_CELL_FLOOR;
    actor_pool_clear(*world);
    map_object_pool_clear(*world);
    for (u8 slot = 0; slot < party_capacity; ++slot) {
        auto &member = world->party.members[slot];
        member.presence = slot == 2 ? PartyPresence::Spectating : slot == 3 ? PartyPresence::Disconnected : PartyPresence::Living;
        member.connected = slot != 3;
        member.character_id = character_identity(slot + 1);
        member.generation = 7;
        auto &player = member.player;
        player.party_slot = slot;
        player.local_view = false;
        player.state.vitals = {100, static_cast<u16>(slot == 2 ? 0 : 37), 50, 11};
        player.state.status_effect_flags = KF_PLAYER_STATUS_POISON;
        player.state.motion_state.map_cell.x = slot + 5;
        player.state.motion_state.map_cell.z = 5;
        player.state.camera_position = {(slot + 5) * KF_MAP_TILE_SIZE, 0, 5 * KF_MAP_TILE_SIZE};
        player.state.weapon_attack_phase = 3;
        player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_GREEN_DRAGON_STAFF)] = 1;
        if (party_member_alive(member)) collision_adjust_cell_occupancy(*world, slot + 5, 5, 1);
    }
    world->prediction = true;
    party_move_to_floor_entry(*world);
    require(world->party.members[0].player.state.motion_state.map_cell.x == 5, "Prediction committed a party return");
    world->prediction = false;
    party_move_to_floor_entry(*world);
    require(world->collision_flags.cells[entry.z][entry.x] == 2, "Party return duplicated living occupancy or made dead bodies solid");
    for (u8 slot = 0; slot < party_capacity; ++slot) {
        const auto &member = world->party.members[slot];
        const auto &player = member.player;
        require(!world->collision_flags.cells[5][slot + 5] && std::abs(player.state.motion_state.map_cell.x - entry.x) <= 3 &&
            std::abs(player.state.motion_state.map_cell.z - entry.z) <= 3 && map_cells_equal(player.state.previous_map_cell, player.state.motion_state.map_cell),
            "Party return left old occupancy or immediately retriggered an entrance");
        require(player.state.map_variant == KF_FLOOR5_ENTRY_VARIANT && player.state.foot_height == -3 * KF_MAP_HEIGHT_STEP &&
            player.state.weapon_attack_phase == KF_WEAPON_ATTACK_INACTIVE && player.state.vitals.current_hp == (slot == 2 ? 0 : 37) &&
            player.state.vitals.current_mp == 11 && player.state.status_effect_flags == KF_PLAYER_STATUS_POISON &&
            member.generation == 7 && player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_GREEN_DRAGON_STAFF)] == 1,
            "Party return changed health, statuses, inventory or identity instead of position and motion");
    }
    require(world->party.members[2].presence == PartyPresence::Spectating && world->party.members[3].presence == PartyPresence::Disconnected,
        "Party return revived or reconnected a character");
    party_move_to_floor_entry(*world);
    require(world->collision_flags.cells[entry.z][entry.x] == 2, "Repeated return accumulated collision occupancy");
}

static void party_terminal_travel()
{
    for (const bool dead_host : {false, true}) {
        auto world = std::make_unique<WorldState>();
        world->party.enabled = true;
        world->cell_attribute.cells[2][15] = KF_MAP_ATTRIBUTE_WARP;
        map_floor_script(*world, KF_FLOOR_5).floor5.boss_defeat = KF_MAP_SCRIPT_SET;
        for (u8 slot = 0; slot < 3; ++slot) {
            auto &member = world->party.members[slot];
            member.presence = slot == 2 || (!slot && dead_host) ? PartyPresence::Spectating : PartyPresence::Living;
            member.connected = true;
            member.character_id = character_identity(slot + 1);
            member.player.state.progress_state.current_floor = KF_FLOOR_1;
            member.player.state.motion_state.map_cell.x = 15;
            member.player.state.motion_state.map_cell.z = 2;
            member.player.state.camera_position = {31000, -1700, 5000};
            member.player.state.vitals.current_hp = member.presence == PartyPresence::Living ? 17 : 0;
            if (party_member_alive(member)) collision_adjust_cell_occupancy(*world, 15, 2, 1);
        }
        const auto before = world->collision_flags;
        require(party_at_same_entrance(*world), "Terminal fixture is not at a shared entrance");
        world->prediction = true;
        auto predicted = party_travel(*world);
        predicted.advance();
        require(predicted.done() && !predicted.await_resume() &&
            !std::memcmp(&before, &world->collision_flags, sizeof before), "Prediction committed a party exit");
        world->prediction = false;
        auto terminal = party_travel(*world);
        terminal.advance();
        require(terminal.done() && terminal.await_resume(), "Party did not reach the terminal exit");
        require(!std::memcmp(&before, &world->collision_flags, sizeof before),
            "Ending handoff left temporary travel occupancy in the world");
    }
}

static void party_lifecycle()
{
    auto world = std::make_unique<WorldState>();
    for (auto &cell : world->collision.linear) cell = KF_MAP_CELL_FLOOR;
    actor_pool_clear(*world);
    map_object_pool_clear(*world);
    PartyRuntime runtime;
    world->party.enabled = true;
    for (u8 slot = 0; slot < 2; ++slot) {
        auto &member = world->party.members[slot];
        member.presence = PartyPresence::Living;
        member.connected = true;
        member.player.local_view = false;
        member.player.party_slot = slot;
        member.player.state.motion_state.map_cell = {5, 5};
        member.player.state.camera_position = {10000, 0, 10000};
        member.player.state.vitals = {100, 100, 20, 20};
        collision_adjust_cell_occupancy(*world, 5, 5, 1);
    }
    int alive = 0, result = 0;
    runtime.controls[1] = nested_task(alive, result);
    runtime.controls[1].advance();
    world->party.members[1].player.state.vitals.current_hp = 0;
    party_settle_deaths(*world, runtime);
    require(alive == 0 && world->party.members[1].presence == PartyPresence::Spectating && !runtime.wiped,
            "Death did not cancel the menu and enter spectator mode");
    require(world->collision_flags.cells[5][5] == 1, "Dead body retained collision occupancy");
    party_settle_deaths(*world, runtime);
    require(world->collision_flags.cells[5][5] == 1, "Death settled twice");
    party_revive_spectators(*world, world->party.members[0].player);
    require(party_member_alive(world->party.members[1]) && world->collision_flags.cells[5][5] == 2,
            "Healing-point revival lost a party member or duplicated occupancy");
    for (auto &member : world->party.members) member.player.state.vitals.current_hp = 0;
    party_settle_deaths(*world, runtime);
    require(runtime.wiped, "All-party death did not request checkpoint restoration");
}

static void party_reconnect_lifecycle()
{
    auto world = std::make_unique<WorldState>();
    PartyRuntime runtime;
    world->party.enabled = true;
    auto &member = world->party.members[1];
    member.presence = PartyPresence::Living;
    member.connected = true;
    member.character_id = character_identity(98765);
    member.generation = 17;
    member.player.party_slot = 1;
    member.player.state.vitals = {100, 37, 20, 13};
    member.player.state.camera_position = {10000, -1700, 10000};
    member.player.state.motion_state.map_cell = {5, 5};
    member.player.state.gold = 1234;
    member.player.item_stock[0][0] = 2;
    member.loot_claims[2][17] = 5;
    collision_adjust_cell_occupancy(*world, 5, 5, 1);
    runtime.tick = 10;
    party_set_connected(*world, runtime, 1, false);
    runtime.tick = 50;
    party_set_connected(*world, runtime, 1, false);
    require(runtime.disconnected_at[1] == 10, "Repeated disconnect extended the vulnerable-body grace period");
    require(party_resume(*world, runtime, 1) && member.connected &&
            member.player.state.vitals.current_hp == 37 && member.player.state.gold == 1234 &&
            member.player.item_stock[0][0] == 2 && member.character_id == character_identity(98765) && member.generation == 17 &&
            member.loot_claims[2][17] == 5 &&
            world->collision_flags.cells[5][5] == 1,
            "Reconnect reset character state, targeting generation, or collision occupancy");
    party_set_connected(*world, runtime, 1, false);
    member.player.state.vitals.current_hp = 0;
    require(party_resume(*world, runtime, 1) && member.presence == PartyPresence::Spectating &&
            member.player.state.vitals.current_hp == 0 && world->collision_flags.cells[5][5] == 0,
            "Reconnect revived a body killed during the grace period");
    member.presence = PartyPresence::Disconnected;
    member.connected = false;
    require(!party_resume(*world, runtime, 1) && !member.connected,
            "An expired body bypassed safe re-admission");
}

static void party_combat()
{
    auto world = std::make_unique<WorldState>();
    world->party.enabled = true;
    for (u8 slot = 0; slot < 3; ++slot) {
        auto &member = world->party.members[slot];
        member.presence = PartyPresence::Living;
        member.connected = true;
        member.generation = 1;
        member.player.party_slot = slot;
        member.player.local_view = slot == 0;
        member.player.state.camera_position = {10000 + slot * 1000, 0, 10000};
        member.player.state.vitals.current_hp = 100;
        member.player.state.physical_power = 10;
        member.player.state.next_level_experience = 10000;
    }
    auto &attacker = world->party.members[0].player;
    auto &victim = world->party.members[1].player;
    // Retail level-one stats and short-sword attack components, without assets.
    attacker.state.cutting_attack = 2;
    attacker.state.striking_attack = attacker.state.piercing_attack = 1;
    attacker.state.attack_charge_state.committed = KF_ACTOR_DAMAGE_SCALE_ONE;
    victim.state.physical_power = 20;
    victim.state.vitals.current_hp = victim.state.vitals.maximum_hp = 30;
    const VECTOR starter_impact {11000, KF_COLLISION_IGNORE_HEIGHT, 10000};
    for (unsigned hit = 1; hit <= 3; ++hit) {
        require(party_melee_hit(*world, attacker, starter_impact, 100, 1000), "Starter melee missed");
        require(victim.state.vitals.current_hp == 30 - hit * 10,
            "Three charged starting sword hits must kill a fresh character");
    }
    victim.state.vitals.current_hp = 30;
    attacker.state.attack_charge_state.committed = KF_ACTOR_DAMAGE_SCALE_ONE / 2;
    require(party_melee_hit(*world, attacker, starter_impact, 100, 1000) && victim.state.vitals.current_hp == 25,
        "Partial melee charge was rounded away or ignored");
    attacker.state.striking_attack = attacker.state.piercing_attack = 0;
    victim.state.physical_power = 10;
    attacker.state.cutting_attack = 30;
    attacker.state.attack_charge_state.committed = 4096;
    victim.state.vitals.current_hp = 1;
    const VECTOR impact {11000, KF_COLLISION_IGNORE_HEIGHT, 10000};
    require(party_melee_hit(*world, attacker, impact, 100, 1000), "Party melee failed to hit a teammate");
    require(victim.state.vitals.current_hp == 0 && attacker.state.vitals.current_hp == 100,
            "Friendly fire failed to kill the target or hit its owner");
    require(victim.state.experience == 0 && attacker.state.experience == 0 &&
            world->party.members[1].presence == PartyPresence::Living,
            "Friendly fire changed party membership or awarded PvP XP");

    world->party.members[1].presence = PartyPresence::Spectating;
    world->party.members[2].presence = PartyPresence::Waiting;
    auto &enemy = world->actors.actors[0];
    enemy.slot_state = KF_ACTOR_SLOT_DYNAMIC;
    enemy.lifecycle = KF_ACTOR_LIFECYCLE_ACTIVE;
    enemy.health = 1;
    auto &definition = world->actors.definitions.entries[enemy.definition_id];
    definition.experience_reward = 10;
    for (auto &clip : definition.action_animations) clip = KF_ANIMATION_CLIP_NONE;
    actor_apply_damage(*world, attacker, 0, 10, 30, 0, 0, 0, 0,
        KF_ACTOR_DAMAGE_SCALE_ONE, KF_ACTOR_DAMAGE_CREDIT_PLAYER);
    require(enemy.health == 0, "Enemy kill fixture did not deal lethal damage");
    require(attacker.state.experience == 10 && victim.state.experience == 10 &&
            world->party.members[2].player.state.experience == 0,
            "Enemy XP must include admitted spectators and exclude waiting joins");
    actor_apply_damage(*world, attacker, 0, 10, 30, 0, 0, 0, 0,
        KF_ACTOR_DAMAGE_SCALE_ONE, KF_ACTOR_DAMAGE_CREDIT_PLAYER);
    require(attacker.state.experience == 10 && victim.state.experience == 10,
        "An already dead enemy awarded party XP twice");

    auto &third = world->party.members[2];
    third.presence = PartyPresence::Living;
    const auto hit = collision_query_world(*world, attacker, 12000, KF_COLLISION_IGNORE_HEIGHT,
        10000, 1, 0, KF_COLLISION_SKIP_TERRAIN | KF_COLLISION_SKIP_ACTORS |
        KF_COLLISION_SKIP_MAP_OBJECTS | KF_COLLISION_SKIP_MAP_EVENTS, 0);
    require(hit.kind == KfCollisionKind::Player && hit.detail == 2,
        "Projectile collision did not retain the struck player slot");
    require(&party_collision_player(*world, attacker, hit) == &third.player,
            "Projectile damage selected the source rather than the collision target");
    party_apply_radial_damage(*world, attacker, &impact, 10000, 4096, 30, 0, 0, 0, 0, 4096, 10);
    require(third.player.state.vitals.current_hp < 100, "Area damage did not reach party members");
}

static void party_spell_combat()
{
    // Exercise the real effect dispatcher and its children in a small empty room.
    // No level navigation, renderer or live resource files are needed.
    for (const auto kind : {KF_MAGIC_FIRE_BALL, KF_MAGIC_WIND_CUTTER, KF_MAGIC_LIGHT_NEEDLE,
            KF_MAGIC_LIGHTNING_BOLT, KF_MAGIC_FIRE_WALL, KF_EFFECT_KIND_MOONLIGHT_PROJECTILE,
            KF_EFFECT_KIND_GROUND_TRAIL, KF_EFFECT_KIND_RADIAL_BLAST}) {
        const bool area = kind == KF_MAGIC_LIGHTNING_BOLT || kind == KF_MAGIC_FIRE_WALL ||
            kind == KF_EFFECT_KIND_MOONLIGHT_PROJECTILE || kind == KF_EFFECT_KIND_RADIAL_BLAST;
        auto world = std::make_unique<WorldState>();
        world->party.enabled = true;
        actor_pool_clear(*world);
        map_object_pool_clear(*world);
        effect_pool_reset(*world);
        for (auto &cell : world->collision.linear) cell = KF_MAP_CELL_FLOOR;
        for (auto &magic : world->effects.magic.entries) {
            magic.damage_components[0] = 30;
            magic.damage_components[1] = 20;
            magic.damage_components[2] = 10;
        }
        for (u8 slot = 0; slot < 4; ++slot) {
            auto &member = world->party.members[slot];
            member.presence = slot == 2 ? PartyPresence::Spectating : PartyPresence::Living;
            member.connected = true;
            member.generation = 1;
            member.character_id = character_identity(slot + 1);
            auto &p = member.player;
            p.party_slot = slot;
            p.local_view = false;
            p.state.camera_position = {slot == 0 ? 20000 : 11000, -1700, 10000};
            if (slot == 3) p.state.camera_position.vx += 200;
            p.state.foot_height = 0;
            p.state.physical_power = 10;
            p.state.vitals.current_hp = slot == 0 ? 100 : 1;
            p.state.experience = 23;
            p.state.gold = 19;
            p.item_stock[0][0] = 2;
        }
        auto &source = world->party.members[0].player;
        auto &victim = world->party.members[1].player;
        if (!area) source.state.camera_position = victim.state.camera_position;
        const VECTOR position {11000, 0, 10000};
        const SVECTOR direction {};
        auto *effect = effect_pool_construct(*world, source, KF_PLAYER_DAMAGE_MULTIPLIER_ONE,
            KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS, kind,
            &position, &direction, KfEffectRotationSoundArguments{&direction, KF_EFFECT_SOUND_SILENT});
        require(effect && effect->owner_player_slot == 0, "Player spell lost its owner");
        if (kind == KF_MAGIC_WIND_CUTTER) {
            victim.state.vitals.current_hp = 1000;
            victim.state.magic_defense = victim.state.fire_defense = 100;
            auto expected = victim;
            player_apply_damage(expected, 30, 10, 20, KF_PLAYER_STATUS_NONE, 0, 0,
                KF_FIXED12_ONE, KF_PLAYER_DAMAGE_MULTIPLIER_ONE);
            effect_pool_update(*world, source);
            require(victim.state.vitals.current_hp == expected.state.vitals.current_hp,
                "Wind Cutter did not use its physical damage components against a teammate");
            require(effect->phase == KF_EFFECT_PROJECTILE_TRAVEL, "Wind Cutter stopped piercing after a party hit");
            victim.state.vitals.current_hp = 1;
        }
        for (unsigned tick = 0; tick < 40 && (victim.state.vitals.current_hp ||
                (area && world->party.members[3].player.state.vitals.current_hp)); ++tick)
            effect_pool_update(*world, source);
        if (victim.state.vitals.current_hp) std::fprintf(stderr, "Nonlethal party spell kind %u\n", kf_enum_encode<unsigned>(kind));
        require(victim.state.vitals.current_hp == 0, "A damaging spell could not kill a teammate");
        require(!area || world->party.members[3].player.state.vitals.current_hp == 0,
            "Spell area damage failed to hit multiple nearby teammates");
        require(source.state.vitals.current_hp == 100 && world->party.members[2].player.state.vitals.current_hp == 1,
            "Spell hit its projectile caster, distant blast caster or admitted spectator");
        for (const auto &child : world->effects.records)
            require(child.type == KF_EFFECT_SLOT_FREE || (child.owner_player_slot == 0 && child.owner_player_generation == 1),
                "A spell child lost the caster's identity");
        for (u8 slot = 0; slot < 4; ++slot) {
            const auto &member = world->party.members[slot];
            const auto &p = member.player;
            require(member.character_id == character_identity(slot + 1) && member.connected &&
                p.state.experience == 23 && p.state.gold == 19 && p.item_stock[0][0] == 2 &&
                p.state.magic_training == 0 && p.state.physical_power_training == 0,
                "Party spell damage changed membership or awarded PvP rewards/training");
        }
    }
}

static void enemy_target_and_random_state()
{
    auto world = std::make_unique<WorldState>();
    world->party.enabled = true;
    for (u8 slot = 0; slot < 2; ++slot) {
        auto &member = world->party.members[slot];
        member.presence = PartyPresence::Living;
        member.player.party_slot = slot;
        member.generation = 7;
        member.player.state.vitals.current_hp = 100;
        member.player.state.camera_position = {1000 + slot * 1000, 0, 0};
    }
    auto &actor = world->actors.actors[0];
    actor.target_player_slot = no_player;
    auto &first = world->party.members[0].player;
    auto &second = world->party.members[1].player;
    require(party_actor_target(*world, actor, *party_nearest_player(*world, actor.position)) == &first,
            "Enemy failed to acquire the nearest living player");
    actor.action = KF_ACTOR_ACTION_MELEE_ATTACK;
    second.state.camera_position.vx = 100;
    require(party_actor_target(*world, actor, second) == &first,
            "A committed attack changed targets during its windup");
    actor.action = KF_ACTOR_ACTION_IDLE;
    require(party_actor_target(*world, actor, second) == &second,
            "An idle enemy did not reconsider a substantially closer player");

    actor.random.state = 123;
    auto replay = actor.random;
    actor_bind_current(*world, &actor);
    require(actor_random_next(*world) == kf::random_next(replay), "Enemy random stream diverged");
    actor_bind_current(*world, &world->actors.actors[1]);
    for (int i = 0; i < 10; ++i) actor_random_next(*world);
    actor_bind_current(*world, &actor);
    require(actor_random_next(*world) == kf::random_next(replay), "Another enemy consumed this enemy's random stream");

    actor_pool_clear(*world);
    map_object_pool_clear(*world);
    effect_pool_reset(*world);
    for (auto &event : world->map.events) event.state = KF_MAP_EVENT_FREE;
    for (auto &cell : world->collision.linear) cell = KF_MAP_CELL_FLOOR;
    for (auto &member : world->party.members) {
        member.presence = PartyPresence::Empty;
        member.player.local_view = false;
    }
    actor = {};
    actor.slot_state = KF_ACTOR_SLOT_DYNAMIC;
    actor.lifecycle = KF_ACTOR_LIFECYCLE_ACTIVE;
    actor.action = KF_ACTOR_ACTION_WANDER;
    actor.position = {10000, 0, 10000};
    actor.health = 100;
    actor.random.state = 123;
    auto &definition = world->actors.definitions.entries[0];
    definition.move_speed = 40;
    definition.turn_rate = 64;
    definition.collision_radius = 200;
    definition.collision_height = 1800;
    for (auto &clip : definition.action_animations) clip = KF_ANIMATION_CLIP_NONE;
    const VECTOR position {20000, -1000, 20000};
    const SVECTOR direction {0, 0, 100};
    effect_pool_construct(*world, first, 10, KF_EFFECT_COLLISION_TARGET_PLAYER,
        KF_EFFECT_KIND_SCATTER_PROJECTILE, &position, &direction, KfEffectScatterArguments{1, 1, 4096});
    auto one = std::make_unique<WorldState>(), two = std::make_unique<WorldState>();
    world_clone_simulation(*world, *one);
    world_clone_simulation(*world, *two);
    const auto advance = [](WorldState &copy) {
        auto &player = copy.party.members[0].player;
        actor_bind_current(copy, &copy.actors.actors[0]);
        auto task = actor_update_current_action(copy, player);
        task.advance();
        require(task.done(), "Predicted enemy movement suspended");
        actor_bind_current(copy, nullptr);
        effect_pool_update(copy, player);
    };
    advance(*one);
    // Unrelated enemy work must not perturb replaying this snapshot.
    actor_bind_current(*world, &world->actors.actors[1]);
    for (int i = 0; i < 20; ++i) actor_random_next(*world);
    advance(*two);
    require(one->actors.actors[0].random.state != 123 &&
        one->actors.actors[0].random.state == two->actors.actors[0].random.state &&
        one->actors.actors[0].position.vx == two->actors.actors[0].position.vx &&
        one->actors.actors[0].position.vz == two->actors.actors[0].position.vz,
        "Real enemy movement was not deterministic across prediction clones");
    require(one->effects.records[1].type != KF_EFFECT_SLOT_FREE &&
        one->effects.records[0].random.state == two->effects.records[0].random.state &&
        one->effects.records[1].direction.vector.vx == two->effects.records[1].direction.vector.vx &&
        one->effects.records[1].direction.vector.vz == two->effects.records[1].direction.vector.vz,
        "Real scatter-projectile propagation diverged across prediction clones");
    require(actor.random.state == 123 && actor.position.vx == 10000 && actor.position.vz == 10000 &&
        world->effects.records[0].control.frames_remaining == 1 && world->effects.records[1].type == KF_EFFECT_SLOT_FREE,
        "Predicting enemy movement or spell propagation mutated the source world");
}

static void prediction_budget_and_view()
{
    auto world = std::make_unique<WorldState>();
    world->party.enabled = world->prediction = true;
    actor_pool_clear(*world);
    map_object_pool_clear(*world);
    effect_pool_reset(*world);
    for (auto &event : world->map.events) event.state = KF_MAP_EVENT_FREE;
    PartyRuntime runtime;
    runtime.tick = 100;
    // Replayed input and subsequent live frames must share this same budget.
    for (unsigned replay = 0; replay < 3; ++replay)
        require(party_predict_tick(*world, runtime, 100), "Pending input could not use its prediction budget");
    require(party_predict_tick(*world, runtime, 100), "Live input lost the remaining prediction tick");
    for (unsigned frame = 0; frame < 60; ++frame)
        require(!party_predict_tick(*world, runtime, 100) && runtime.tick == 104,
            "Replay plus live prediction advanced beyond 200 ms");
    runtime.tick = 105;
    require(party_predict_tick(*world, runtime, 105) && runtime.tick == 106,
        "A fresh authoritative snapshot did not renew the prediction budget");
    world->prediction = false;
    require(!party_predict_tick(*world, runtime, 105), "Prediction advanced an authoritative world");

    PartyViewCorrection correction;
    const VECTOR before {10000, -1700, 10000}, after {10400, -1500, 9800};
    const SVECTOR old_rotation {0, 4090, 0}, new_rotation {0, 10, 0};
    party_correct_view(correction, before, old_rotation, after, new_rotation);
    VECTOR position;
    SVECTOR rotation;
    for (int frame = 1; frame <= 4; ++frame) {
        position = after; rotation = new_rotation;
        party_smooth_view(correction, position, rotation);
        require(position.vx == 10000 + frame * 100 && position.vy == -1700 + frame * 50 &&
            position.vz == 10000 - frame * 50 && rotation.vy == 10 - (4 - frame) * 4,
            "View correction jumped, overshot or took the long rotation path");
    }
    require(!correction.remaining && after.vx == 10400 && before.vx == 10000,
        "View smoothing retained an offset or changed its simulation inputs");
    party_correct_view(correction, before, old_rotation, after, new_rotation);
    position = after; rotation = new_rotation;
    party_smooth_view(correction, position, rotation);
    const VECTOR next {10500, -1500, 9800};
    party_correct_view(correction, after, new_rotation, next, new_rotation);
    require(next.vx + correction.position.vx == position.vx,
        "A second snapshot discarded the currently displayed correction");
    party_correct_view(correction, next, new_rotation, {20000, -1700, 10000}, new_rotation);
    require(!correction.remaining, "A teleport swept the presentation through the intervening world");
    party_correct_view(correction, before, {}, before, {0, KF_ANGLE_HALF_TURN, 0});
    require(!correction.remaining, "A large facing correction did not snap");
    party_correct_view(correction, {std::numeric_limits<s32>::min(), 0, 0}, {},
        {std::numeric_limits<s32>::max(), 0, 0}, {});
    require(!correction.remaining, "Extreme correction coordinates overflowed");
}

static void spell_snapshot_transitions()
{
    std::array<std::unique_ptr<SnapshotAssetFixture>, 18> assets;
    for (unsigned i = 0; i < assets.size(); ++i)
        assets[i] = std::make_unique<SnapshotAssetFixture>(30 + i, 1);
    for (const auto kind : {KF_MAGIC_FIRE_BALL, KF_MAGIC_LIGHTNING_BOLT,
            KF_EFFECT_KIND_LIGHTNING_BOLT_ALTERNATE, KF_MAGIC_WIND_CUTTER,
            KF_EFFECT_KIND_SCATTER_PROJECTILE, KF_EFFECT_KIND_DARKNESS_PROJECTILE,
            KF_EFFECT_KIND_CURSE_PROJECTILE, KF_EFFECT_KIND_GROUND_TRAIL,
            KF_EFFECT_KIND_LIGHTNING_IMPACT, KF_EFFECT_KIND_LIGHTNING_IMPACT_ALTERNATE}) {
        auto world = std::make_unique<WorldState>();
        actor_pool_clear(*world);
        map_object_pool_clear(*world);
        effect_pool_reset(*world);
        for (auto &cell : world->collision.linear) cell = KF_MAP_CELL_FLOOR;
        auto &player = world->party.members[0].player;
        player.local_view = false;
        const VECTOR position {11000, -500, 10000};
        const SVECTOR direction {};
        auto *effect = effect_pool_construct(*world, player, 10, KF_EFFECT_COLLISION_TARGET_ACTORS,
            kind, &position, &direction);
        require(effect, "Cannot construct spell snapshot fixture");
        if (effect->kind == KF_MAGIC_FIRE_BALL) effect->phase = KF_EFFECT_PROJECTILE_IMPACT_FIRST;
        if (effect->kind == KF_MAGIC_LIGHTNING_BOLT || effect->kind == KF_EFFECT_KIND_SCATTER_PROJECTILE)
            effect->control.frames_remaining = 2;
        if (kind == KF_EFFECT_KIND_DARKNESS_PROJECTILE || kind == KF_EFFECT_KIND_CURSE_PROJECTILE)
            effect->phase = KF_EFFECT_PROJECTILE_SHRINK;
        std::vector<u8> bytes;
        for (unsigned tick = 0; tick < 20; ++tick) {
            require(world_snapshot_encode(*world, {world->epoch, tick, tick % 2 == 0}, bytes),
                "A real spell transition failed snapshot encoding");
            WorldSnapshotInfo info;
            auto restored = world_snapshot_decode(bytes, *world, info);
            require(restored && info.tick == tick, "A real spell transition failed snapshot decoding");
            for (unsigned slot = 0; slot < KF_EFFECT_CAPACITY; ++slot) {
                const auto &before = world->effects.records[slot];
                const auto &after = restored->effects.records[slot];
                require(before.type == after.type && before.kind == after.kind &&
                    before.render_id.model == after.render_id.model && before.phase == after.phase,
                    "Snapshot changed a spell's canonical kind, appearance or phase");
            }
            effect_pool_update(*world, player);
        }
    }
}

static void map_effect_fixture(KfObjectId id, bool exhausted, bool outside)
{
    auto world = std::make_unique<WorldState>();
    auto &player = world->party.members[0].player;
    player.local_view = false;
    effect_pool_reset(*world);
    world->objects.definitions.entries[kf_enum_encode<u8>(id)].behavior_type = KF_MAP_OBJECT_OP_NONE;
    if (exhausted)
        for (auto &effect : world->effects.records) effect.type = KF_EFFECT_CLASS_20;
    std::array<u8, 21> placement {};
    placement[0] = kf_enum_encode<u8>(id);
    // Cell zero plus local x=-1 is syntactically valid but outside the world.
    if (outside) placement[8] = placement[9] = 0xff;
    placement[20] = 0xff;
    map_object_pool_load(*world, player, {placement.data(), placement.size()});
    require(!exhausted && !outside, "Invalid map effect allocation was accepted");
    const auto index = world->objects.objects[0].link.fields.action_parameter.effect_index;
    require(index < KF_EFFECT_CAPACITY, "Map hazard retained an invalid effect index");
    const auto &direction = world->effects.records[index].direction.vector;
    require(direction.vx == 0 && direction.vy == 0 && direction.vz == 0,
        "Stationary map effect has uninitialized motion");
}

static void door_edge_fixture(KfMapObjectOperation operation, u8 x, u8 z, u16 yaw)
{
    auto world = std::make_unique<WorldState>();
    for (auto &cell : world->collision.linear) cell = KF_MAP_CELL_FLOOR;
    world->objects.definitions.entries[0].behavior_type = operation;
    KfMapObject object {};
    object.object_id = kf_enum_decode<KfObjectId>(0);
    object.cell_x = x;
    object.cell_z = z;
    map_object_mark_collision_edge(*world, &object, KF_MAP_CELL_BLOCKED, yaw);
    require(std::count(std::begin(world->collision.linear), std::end(world->collision.linear),
        KF_MAP_CELL_BLOCKED) == 2, "Door collision footprint changed");
}

static void actor_attachment_bounds()
{
    auto world = std::make_unique<WorldState>();
    effect_pool_reset(*world);
    KfActor actor {};
    KfActorDefinition definition {};
    world->actors.current = &actor;
    world->actors.current_definition = &definition;
    for (auto slot : {KF_ACTOR_EFFECT_SLOT_THIRD, kf_enum_decode<KfActorEffectSlot>(-1)}) {
        actor_spawn_action_effect(*world, world->party.members[0].player,
            kf_enum_decode<KfActorEffectCode>(kf_enum_encode<u8>(KF_MAGIC_FIRE_BALL)), slot);
        for (const auto &effect : world->effects.records)
            require(effect.type == KF_EFFECT_SLOT_FREE, "Unsupported attachment launched an effect");
    }
}

static void spell_world_bounds()
{
    auto world = std::make_unique<WorldState>();
    actor_pool_clear(*world);
    map_object_pool_clear(*world);
    effect_pool_reset(*world);
    for (auto &cell : world->collision.linear) cell = KF_MAP_CELL_FLOOR;
    for (auto &cell : world->cell_attribute.linear) cell = KF_MAP_ATTRIBUTE_01;
    auto &player = world->party.members[0].player;
    player.local_view = false;
    player.state.camera_position = {11000, -1700, 10000};
    const VECTOR inside {11000, -500, 10000};
    const SVECTOR direction {};
    for (const VECTOR outside : {VECTOR{-1, 0, 10000}, VECTOR{200000, 0, 10000},
            VECTOR{10000, 0, -1}, VECTOR{10000, 0, 200000}, VECTOR{131072000, 0, 10000}}) {
        require(!effect_pool_construct(*world, player, 10, KF_EFFECT_COLLISION_TARGET_ACTORS,
            KF_MAGIC_LIGHTNING_BOLT, &outside, &direction), "Out-of-world effect was constructed");
        auto *effect = effect_pool_construct(*world, player, 10, KF_EFFECT_COLLISION_TARGET_ACTORS,
            KF_MAGIC_LIGHTNING_BOLT, &inside, &direction);
        require(effect, "Cannot create boundary fixture projectile");
        effect->position = outside; // Final movement of a prior tick / bounded remote snapshot.
        effect_pool_set_current(*world, effect);
        auto query = outside;
        require(effect_map_collision(*world, player, &query, 100).kind == KfCollisionKind::Terrain,
            "Out-of-world coordinates wrapped to an in-map collision cell");
        effect_pool_update(*world, player);
        require(effect->type == KF_EFFECT_SLOT_FREE && world->effects.records[1].type == KF_EFFECT_SLOT_FREE,
            "Out-of-world lightning read terrain or created impact children");
    }
    const VECTOR edge {199999, -500, 10000};
    const SVECTOR moving {100, 0, 0};
    auto *lightning = effect_pool_construct(*world, player, 10, KF_EFFECT_COLLISION_TARGET_ACTORS,
        KF_MAGIC_LIGHTNING_BOLT, &edge, &moving);
    require(lightning, "Cannot create expiring lightning fixture");
    lightning->control.frames_remaining = 1;
    effect_pool_update(*world, player);
    require(lightning->type == KF_EFFECT_SLOT_FREE, "Expiring lightning sampled an off-map impact cell");
    auto *trail = effect_pool_construct(*world, player, 10, KF_EFFECT_COLLISION_TARGET_ACTORS,
        KF_EFFECT_KIND_GROUND_TRAIL, &edge, &moving);
    require(trail, "Cannot create ground-trail boundary fixture");
    trail->control.parent_effect_index = 1;
    effect_pool_update(*world, player);
    require(trail->type == KF_EFFECT_SLOT_FREE, "Ground trail crossed the edge before reading floor height");
    player.state.camera_position = {1000, -1700, 199000};
    player.state.selected_magic_id = KF_MAGIC_FIRE_WALL;
    player.state.camera_rotation.vy = 0; // Probe extends beyond the last row.
    magic_cast(*world, player);
    for (const auto &effect : world->effects.records)
        require(effect.type == KF_EFFECT_SLOT_FREE, "Off-map Fire Wall probe created a hazard");
    for (auto &cell : world->collision.linear) cell = KF_MAP_CELL_BLOCKED;
    effect_pool_set_current(*world, &world->effects.records[0]);
    for (const s32 x : {0, 199999}) for (const s32 z : {0, 199999}) {
        VECTOR corner {x, -500, z};
        require(effect_map_collision(*world, player, &corner, 100).kind == KfCollisionKind::Terrain,
            "Blocked border cell read an out-of-map neighbor");
    }
    for (auto &cell : world->collision.linear) cell = KF_MAP_CELL_FLOOR;

    auto &actor = world->actors.actors[0];
    actor.slot_state = KF_ACTOR_SLOT_DYNAMIC;
    actor.lifecycle = KF_ACTOR_LIFECYCLE_ACTIVE;
    actor.position = {11000, 0, 12000};
    actor.cell_x = 5;
    actor.cell_z = 6;
    world->cell_attribute.cells[6][5] = KF_MAP_ATTRIBUTE_00;
    player.state.camera_position = {11000, -1700, 10000};
    player.state.selected_magic_id = KF_MAGIC_LIGHTNING_BOLT;
    magic_cast(*world, player);
    require(world->actors.player_target == &actor && world->effects.records[0].type != KF_EFFECT_SLOT_FREE,
        "Lightning did not exercise aiming at an attribute-zero target");
    world->actors.current = &actor;
    world->actors.current_definition = &world->actors.definitions.entries[0];
    world->actors.player_position = player.state.camera_position;
    actor.action = KF_ACTOR_ACTION_JUMP_ATTACK;
    actor.action_progress = KF_ACTOR_PROGRESS_INIT;
    auto jump = actor_update_current_action(*world, player);
    jump.advance();
    require(jump.done() && actor.vertical_state == KF_ACTOR_VERTICAL_JUMP_ATTACK &&
        actor.position.vy == -300 && actor.vertical_velocity == -280,
        "Attribute-zero jump did not use its own height entry safely");
}

static void effect_presentation_history()
{
    auto world = std::make_unique<WorldState>();
    auto presentation = std::make_unique<PartyPresentation>();
    actor_pool_clear(*world);
    map_object_pool_clear(*world);
    effect_pool_reset(*world);
    for (auto &cell : world->collision.linear) cell = KF_MAP_CELL_FLOOR;
    auto &player = world->party.members[0].player;
    player.local_view = false;
    const VECTOR position {11000, -500, 10000};
    const SVECTOR direction {};
    auto *effect = effect_pool_construct(*world, player, KF_PLAYER_DAMAGE_MULTIPLIER_ONE,
        KF_EFFECT_COLLISION_TARGET_ACTORS, KF_MAGIC_FIRE_BALL, &position, &direction,
        KfEffectRotationSoundArguments{&direction, KF_EFFECT_SOUND_SILENT});
    require(effect && !effect->age, "New effect inherited its previous age");
    const auto travelling = *effect;
    party_confirm_effects(*presentation, *world);
    auto &view = presentation->effect_visuals[0];
    PartyEffectAppearance appearance;
    effect->phase = KF_EFFECT_PROJECTILE_IMPACT_FIRST;
    effect_pool_update(*world, player);
    require(effect->age == 1 && effect->render_id.model != travelling.render_id.model,
        "Fixture did not advance a real Fire Ball impact");
    require(party_present_effect(&view, *effect, appearance) &&
        appearance.render_id.model == travelling.render_id.model && !view.displayed,
        "Speculative collision displayed an unconfirmed impact");
    *effect = travelling;
    party_confirm_effects(*presentation, *world);
    require(party_present_effect(&view, *effect, appearance),
        "Correcting a wrong predicted collision permanently hid the projectile");

    effect->phase = KF_EFFECT_PROJECTILE_IMPACT_FIRST;
    party_confirm_effects(*presentation, *world);
    const auto authoritative = *effect;
    effect_pool_update(*world, player);
    require(party_present_effect(&view, *effect, appearance) && view.displayed,
        "Confirmed Fire Ball impact did not display");
    const auto first_frame = appearance.render_id.model;
    effect_pool_update(*world, player);
    require(party_present_effect(&view, *effect, appearance) && appearance.render_id.model != first_frame,
        "Confirmed impact animation did not advance");
    const auto displayed_frame = appearance.render_id.model;
    *effect = authoritative;
    party_clear_entity_corrections(*presentation); // Ordinary full snapshots reset poses only.
    party_confirm_effects(*presentation, *world);
    require(party_present_effect(&view, *effect, appearance) && appearance.render_id.model == displayed_frame &&
        effect->render_id.model == authoritative.render_id.model && effect->age == authoritative.age,
        "Snapshot rewound a displayed impact or rendering changed simulation");
    for (unsigned tick = 0; tick < 10 && effect->type != KF_EFFECT_SLOT_FREE; ++tick) {
        effect_pool_update(*world, player);
        party_advance_presentation(*presentation, *world);
        party_present_effect(&view, *effect, appearance);
    }
    require(effect->type == KF_EFFECT_SLOT_FREE && view.finished, "Finished impact was not retired");
    *effect = authoritative;
    party_confirm_effects(*presentation, *world);
    require(!party_present_effect(&view, *effect, appearance), "Correction replayed a finished impact");
    require(party_present_effect(nullptr, *effect, appearance), "Guest history suppressed host/solo rendering");
    ++effect->generation;
    party_confirm_effects(*presentation, *world);
    require(party_present_effect(&view, *effect, appearance), "Reused slot inherited its predecessor's tombstone");
    view.finished = true;
    ++effect->owner_player_generation;
    party_confirm_effects(*presentation, *world);
    require(party_present_effect(&view, *effect, appearance), "New owner generation inherited impact history");
    effect->age = UINT32_MAX;
    effect_pool_update(*world, player);
    require(effect->age == UINT32_MAX, "Effect age wrapped");
    effect->type = KF_EFFECT_SLOT_FREE;
    auto *replacement = effect_pool_spawn_floor_deformation(*world, 0, 1, 1, 0, 1, 1);
    require(replacement == effect && !replacement->age, "Floor effect did not reset reused slot age");

    auto &child = world->effects.records[1];
    child = authoritative;
    child.kind = KF_EFFECT_KIND_RADIAL_BLAST;
    require(!party_present_effect(&presentation->effect_visuals[1], child, appearance),
        "An unconfirmed predicted child flashed in another pool slot");
    party_confirm_effects(*presentation, *world);
    require(party_present_effect(&presentation->effect_visuals[1], child, appearance),
        "Host-confirmed child failed to display");
    const auto young_child = child;
    effect_pool_update(*world, player);
    require(party_present_effect(&presentation->effect_visuals[1], child, appearance) &&
        appearance.scale_x > young_child.scale_x, "Real blast did not expand");
    const auto displayed_scale = appearance.scale_x;
    child = young_child;
    party_confirm_effects(*presentation, *world);
    require(party_present_effect(&presentation->effect_visuals[1], child, appearance) &&
        appearance.scale_x == displayed_scale && child.scale_x == young_child.scale_x,
        "Snapshot shrank a displayed blast or presentation rewrote its damage radius");
    *presentation = {}; // Reconnect/world epoch baseline.
    party_confirm_effects(*presentation, *world);
    require(party_present_effect(&presentation->effect_visuals[1], child, appearance),
        "New world inherited old effect history");
    child.kind = KF_EFFECT_KIND_EMERGING_PROJECTILE;
    child.phase = KF_EFFECT_PROJECTILE_EMERGE_FIRST;
    *presentation = {};
    require(party_present_effect(&presentation->effect_visuals[1], child, appearance),
        "Pre-launch emergence was mistaken for a speculative impact");
}

static void entity_presentation_corrections()
{
    auto world = std::make_unique<WorldState>();
    auto presentation = std::make_unique<PartyPresentation>();
    world->party.enabled = true;
    world->party.local_slot = 0;
    world->presentation = presentation.get();
    actor_pool_clear(*world);
    map_object_pool_clear(*world);
    effect_pool_reset(*world);
    for (auto &event : world->map.events) event.state = KF_MAP_EVENT_FREE;
    auto &member = world->party.members[1];
    member.presence = PartyPresence::Living;
    member.generation = 7;
    member.player.state.vitals.current_hp = 100;
    member.player.state.camera_position = {10000, -1700, 10000};
    member.player.state.camera_rotation = {0, 4090, 0};
    auto &actor = world->actors.actors[0];
    actor.slot_state = KF_ACTOR_SLOT_DYNAMIC;
    actor.lifecycle = KF_ACTOR_LIFECYCLE_ACTIVE;
    actor.generation = 8;
    actor.position = {10000, 0, 10000};
    actor.rotation.vector = {0, 4090, 0};
    auto &effect = world->effects.records[0];
    effect.type = KF_EFFECT_COLLISION_TARGET_PLAYER;
    effect.kind = KF_MAGIC_FIRE_BALL;
    effect.render_id.billboard = KF_EFFECT_BILLBOARD_FIRE_BALL;
    effect.generation = 9;
    effect.position = actor.position;
    effect.rotation.vector = actor.rotation.vector;
    auto &object = world->objects.objects[0];
    object.object_id = kf_enum_decode<KfObjectId>(0);
    object.generation = 10;
    object.position = actor.position;
    object.rotation.vector = actor.rotation.vector;
    auto &event = world->map.events[0];
    event.state = KF_MAP_EVENT_ACTIVE;
    event.reference_position = actor.position;
    event.rotation = actor.rotation.vector;
    const auto poses = [&] { return std::array {party_player_pose(member), party_actor_pose(actor),
        party_effect_pose(effect), party_object_pose(object), party_event_pose(event)}; };
    PartyEntityView *views[] = {&presentation->players[1], &presentation->actors[0], &presentation->effects[0],
        &presentation->objects[0], &presentation->events[0]};
    party_capture_presentation(*presentation, *world);
    member.player.state.camera_position.vx += 400;
    member.player.state.camera_rotation.vy = 10;
    for (auto *position : {&actor.position, &effect.position, &object.position, &event.reference_position}) position->vx += 400;
    for (auto *rotation : {&actor.rotation.vector, &effect.rotation.vector, &object.rotation.vector, &event.rotation}) rotation->vy = 10;
    party_correct_presentation(*presentation, *world);
    for (int frame = 0; frame <= 4; ++frame) {
        if (frame) party_advance_presentation(*presentation, *world);
        const auto current = poses();
        for (unsigned i = 0; i < current.size(); ++i) {
            const auto shown = party_present_entity(views[i], current[i]);
            require(current[i].position.vx == 10400 && current[i].rotation.vy == 10 &&
                shown.position.vx == 10000 + frame * 100 && shown.rotation.vy == 10 - (4 - frame) * 4,
                "Entity correction jumped, rewrote simulation or used the long rotation path");
            require(party_present_entity(nullptr, current[i]).position.vx == 10400,
                "Offline/host rendering acquired a correction");
        }
    }
    party_capture_presentation(*presentation, *world);
    member.player.state.camera_position.vx += 400;
    party_correct_presentation(*presentation, *world);
    party_advance_presentation(*presentation, *world);
    const auto before = party_present_entity(views[0], party_player_pose(member));
    party_capture_presentation(*presentation, *world);
    member.player.state.camera_position.vx += 100;
    party_correct_presentation(*presentation, *world);
    require(party_present_entity(views[0], party_player_pose(member)).position.vx == before.position.vx,
        "A subsequent snapshot discarded the displayed entity correction");
    ++member.generation;
    require(party_present_entity(views[0], party_player_pose(member)).position.vx == member.player.state.camera_position.vx,
        "A new player generation inherited the old body offset");
    party_capture_presentation(*presentation, *world);
    member.player.state.camera_position.vx += 100;
    ++member.avatar;
    object.position.vx += 100;
    effect.position.vx += 100;
    event.reference_position.vx += 100;
    ++object.generation;
    effect.owner_player_slot = 1;
    event.character_id = kf_enum_decode<KfCharacterId>(1);
    actor.position.vx += 10000;
    party_correct_presentation(*presentation, *world);
    for (const auto *view : views) require(!view->correction.remaining,
        "A replacement entity or teleport kept its previous correction");
    member.presence = PartyPresence::Spectating;
    actor.slot_state = KF_ACTOR_SLOT_FREE;
    require(!party_player_pose(member).active && !party_actor_pose(actor).active,
        "A dead player or free actor slot was offered for rendering");
    *presentation = {};
    require(party_present_entity(views[0], party_player_pose(member)).position.vx == member.player.state.camera_position.vx,
        "Reset retained old presentation offsets");
}

static void world_and_player_random_isolation()
{
    auto source = std::make_unique<WorldState>();
    source->party.enabled = true;
    source->random.state = 12345;
    actor_pool_clear(*source);
    effect_pool_reset(*source);
    map_object_pool_clear(*source);
    for (auto &cell : source->collision.linear) cell = KF_MAP_CELL_FLOOR;
    for (auto &event : source->map.events) event.state = KF_MAP_EVENT_FREE;
    auto &npc = source->map.events[0];
    npc.state = KF_MAP_EVENT_ACTIVE;
    npc.reference_position = {10000, 0, 10000};
    npc.cell_x = npc.cell_z = 5;
    npc.radius = 100;
    source->collision_flags.cells[5][5] = 1;
    auto one = std::make_unique<WorldState>(), two = std::make_unique<WorldState>();
    world_clone_simulation(*source, *one);
    world_clone_simulation(*source, *two);
    const auto wander = [](WorldState &world) {
        map_event_set_current(world, &world.map.events[0]);
        for (unsigned tick = 0; tick < 10; ++tick)
            map_event_update_wander(world, world.party.members[0].player);
    };
    wander(*one);
    for (unsigned draw = 0; draw < 30; ++draw) kf::random_next();
    wander(*two);
    require(one->random.state != 12345 && one->random.state == two->random.state &&
        one->map.events[0].rotation_target == two->map.events[0].rotation_target &&
        one->map.events[0].reference_position.vx == two->map.events[0].reference_position.vx &&
        one->map.events[0].reference_position.vz == two->map.events[0].reference_position.vz,
        "NPC prediction depended on process-global random draws");
    require(source->random.state == 12345 && npc.reference_position.vx == 10000 && npc.reference_position.vz == 10000,
        "NPC prediction consumed source-world randomness or moved its source NPC");
    one->prediction = two->prediction = false;
    map_object_spawn_gold_drop(*one, 10, &npc.reference_position, 0);
    for (unsigned draw = 0; draw < 30; ++draw) kf::random_next();
    map_object_spawn_gold_drop(*two, 10, &npc.reference_position, 0);
    const auto &a = one->objects.objects[KF_MAP_OBJECT_GOLD_DROP_FIRST];
    const auto &b = two->objects.objects[KF_MAP_OBJECT_GOLD_DROP_FIRST];
    require(a.object_id == KF_ITEM_GOLD_COIN && a.position.vx == b.position.vx && a.position.vz == b.position.vz &&
        a.rotation.angles.y == b.rotation.angles.y && one->random.state == two->random.state,
        "Map drops did not resume the cloned world's random sequence");
    PlayerContext first, second, unrelated;
    first.party_slot = second.party_slot = 1;
    unrelated.party_slot = 2;
    first.local_view = second.local_view = false;
    first.random.state = second.random.state = 6789;
    player_apply_damage(first, 0, 0, 0, KF_PLAYER_STATUS_POISON, 0, 0, 4096, 10);
    for (unsigned draw = 0; draw < 30; ++draw) { player_random_next(unrelated); kf::random_next(); }
    player_apply_damage(second, 0, 0, 0, KF_PLAYER_STATUS_POISON, 0, 0, 4096, 10);
    require(first.random.state != 6789 && first.random.state == second.random.state &&
        first.state.status_effect_flags == second.state.status_effect_flags && first.state.poison_timer == second.state.poison_timer,
        "Poison rolls depended on another player or process-global random state");
    second.prediction = true;
    player_apply_damage(second, 0, 0, 0, KF_PLAYER_STATUS_POISON, 0, 0, 4096, 10);
    require(first.random.state == second.random.state, "Suppressed predicted damage consumed a poison roll");
}

static void authoritative_audio()
{
    auto host = std::make_unique<WorldState>();
    auto guest = std::make_unique<WorldState>();
    host->party.enabled = true;
    auto &emitter = host->party.members[1].player;
    emitter.local_view = false;
    emitter.state.camera_position = {100000, -1700, 100000};
    const SoundRef sound {1, 2, 60};
    audio_state.listener_position = {};
    audio_voice_slot_index = 0;
    {
        PartyAudioScope scope(*host);
        require(audio_play_spatial_default_range(emitter, &sound, &emitter.state.camera_position, 127) == KF_AUDIO_PLAYED,
            "A distant host kept retrying the same shared sound");
        sound_ref_play(audio_playback(emitter), &sound, 100);
    }
    require(host->sound_sequence == 2 && host->sounds[1].x == 100000 && host->sounds[2].volume == 100 &&
        audio_voice_slot_index == 0, "Sound capture depended on the host's range or effects setting");
    world_clone_simulation(*host, *guest);
    guest->party.local_slot = 1;
    auto &listener = guest->party.members[1].player;
    listener.local_view = true;
    listener.state.audio_effects_enabled = KF_PLAYER_OPTION_ON;
    u32 last = 0;
    party_audio_receive(*guest, last, false);
    require(last == 2 && audio_voice_slot_index == 2, "Guest did not hear authoritative sounds at its own position");
    party_audio_receive(*guest, last, false);
    require(audio_voice_slot_index == 2, "Repeated snapshot replayed a sound");
    {
        PartyAudioScope scope(*guest);
        sound_ref_play(audio_playback(listener), &sound, 127);
        audio_play_spatial_default_range(listener, &sound, &listener.state.camera_position, 127);
    }
    require(guest->sound_sequence == 2 && audio_voice_slot_index == 2, "Prediction emitted or replayed sound");
    auto menu_sound = menu_play_input_sound(MENU_SOUND_CURSOR);
    menu_sound.advance();
    menu_sound.advance();
    require(menu_sound.done() && guest->sound_sequence == 2, "Local menu sound entered the shared history");
    {
        PartyAudioScope scope(*host);
        for (unsigned i = 0; i < 35; ++i)
            audio_play_spatial_default_range(emitter, &sound, &emitter.state.camera_position, 127);
    }
    world_clone_simulation(*host, *guest);
    guest->party.local_slot = 1;
    listener.state.audio_effects_enabled = KF_PLAYER_OPTION_ON;
    party_audio_receive(*guest, last, false);
    require(last == 37 && audio_voice_slot_index == 4, "Loss recovery did not play exactly the bounded recent history");
    last = 0;
    party_audio_receive(*guest, last, true);
    require(last == 37 && audio_voice_slot_index == 4, "Joining or reconnecting replayed old history");
    listener.state.audio_effects_enabled = KF_PLAYER_OPTION_OFF;
    last = 36;
    party_audio_receive(*guest, last, false);
    listener.state.audio_effects_enabled = KF_PLAYER_OPTION_ON;
    party_audio_receive(*guest, last, false);
    require(last == 37 && audio_voice_slot_index == 4, "Muted sound was played or replayed after unmuting");
    guest->party.members[1].presence = PartyPresence::Spectating;
    listener.state.camera_position = {};
    auto &watched = guest->party.members[0];
    watched.presence = PartyPresence::Living;
    watched.player.state.vitals.current_hp = 100;
    watched.player.state.camera_position = {100000, -1700, 100000};
    watched.player.state.audio_effects_enabled = KF_PLAYER_OPTION_OFF;
    last = 36;
    party_audio_receive(*guest, last, false);
    require(audio_voice_slot_index == 5 && audio_state.listener_position.vx == 100000 &&
        &party_view_player(*guest) == &watched.player, "Spectator audio did not follow the viewed player with local volume settings");
    {
        kf::InputContextScope menu(kf::InputContext::Menu);
        auto &local = host->party.members[0].player;
        local.local_view = true;
        PartyAudioScope scope(*host);
        auto task = menu_play_input_sound(MENU_SOUND_CONFIRM);
        task.advance();
        task.advance();
        require(task.done() && host->sound_sequence == 37, "Host menu sounds entered the shared world history");
        sound_ref_play(audio_playback(local), &sound, 127);
    }
    require(host->sound_sequence == 38, "An open menu suppressed authoritative world audio");
    actor_pool_clear(*host);
    map_object_pool_clear(*host);
    effect_pool_reset(*host);
    for (auto &cell : host->collision.linear) cell = KF_MAP_CELL_FLOOR;
    auto &member = host->party.members[1];
    member.connected = true;
    member.presence = PartyPresence::Living;
    emitter.state.vitals = {100, 100, 100, 100};
    emitter.state.camera_position = {101000, -1700, 101000};
    auto &orbit = host->effects.records[0];
    host->effects.current_record = &orbit;
    host->effects.current_magic = &host->effects.magic.entries[0];
    orbit.phase = KF_EFFECT_HAZARD_RUNNING;
    orbit.direction.vector = {390, -1700, 390};
    orbit.sound_played = KF_AUDIO_PLAYED;
    effect_update_orbiting_projectile(*host, host->party.members[0].player, 0, KF_EFFECT_HAZARD_RUNNING);
    require(orbit.sound_played == KF_AUDIO_PLAYED, "Distant host rearmed an orbiting sound beside a guest");
    emitter.state.camera_position = {};
    effect_update_orbiting_projectile(*host, host->party.members[0].player, 0, KF_EFFECT_HAZARD_RUNNING);
    require(orbit.sound_played == KF_AUDIO_NOT_PLAYED, "Orbiting sound did not rearm after the party left its range");
}

static void snapshot_validation_and_prediction_isolation()
{
    auto source = std::make_unique<WorldState>();
    source->party.enabled = true;
    source->random.state = 0xabcdef01;
    actor_pool_clear(*source);
    map_object_pool_clear(*source);
    effect_pool_reset(*source);
    for (auto &event : source->map.events) event.state = KF_MAP_EVENT_FREE;
    auto &member = source->party.members[0];
    member.presence = PartyPresence::Living;
    member.connected = true;
    member.character_id = character_identity(0x123456789abcdef0ULL);
    member.generation = 1;
    auto &player = member.player;
    player.random.state = 0x76543210;
    player.cast_pose_ticks = 7;
    player.party_slot = 0;
    player.state.progress_state.level = 1;
    player.state.progress_state.current_floor = KF_FLOOR_1;
    player.state.progress_state.highest_floor = KF_FLOOR_1;
    player.state.vitals = {100, 100, 100, 100};
    player.state.physical_power = 10;
    player.state.next_level_experience = 10000;
    player.state.camera_position = {10000, -1700, 10000};
    player.state.motion_state.map_cell = {5, 5};
    player.state.equipped_weapon_id = KF_OBJECT_NONE;
    player.state.selected_magic_id = KF_MAGIC_NONE;
    player.state.equipped_head_armor_id = player.state.equipped_body_armor_id = KF_OBJECT_NONE;
    player.state.equipped_shield_id = player.state.equipped_arm_armor_id = KF_OBJECT_NONE;
    player.state.equipped_leg_armor_id = player.state.equipped_accessory_id = KF_OBJECT_NONE;
    source->floor_height.cells[5][5] = 42;
    source->actors.actors[0].random.state = 98765;
    source->effects.records[0].age = 76543;
    source->party.quest_rewards = static_cast<u32>(PartyReward::KeyOfTheDead) | static_cast<u32>(PartyReward::Healing);
    member.quest_rewards = static_cast<u32>(PartyReward::KeyOfTheDead);
    member.loot_claims[0][4] = 9;
    member.avatar = 23;
    member.loot_claims[4][12] = 8;
    source->objects.objects[170].generation = 987;
    {
        PartyAudioScope scope(*source);
        const SoundRef sound {1, 2, 60};
        audio_play_spatial_default_range(player, &sound, &player.state.camera_position, 127);
    }
    KfAnimationCacheRecord first_cache {}, second_cache {};
    source->actors.actors[0].animation_cache = &first_cache;
    std::vector<u8> bytes, other;
    require(world_snapshot_encode(*source, {1, 25, true}, bytes), "Cannot encode a valid full world snapshot");
    PartyPresentation local_presentation;
    source->presentation = &local_presentation;
    local_presentation.players[0].correction.position = {99, 88, 77};
    source->actors.actors[0].animation_cache = &second_cache;
    require(world_snapshot_encode(*source, {1, 25, true}, other) && bytes == other,
            "Local presentation state or an animation-cache address entered the snapshot");
    WorldSnapshotInfo info;
    auto restored = world_snapshot_decode(bytes, *source, info);
    require(restored && restored->effects.records[0].age == 76543, "Full snapshot lost effect age");
    require(restored->party.members[0].player.cast_pose_ticks == 7, "Full snapshot lost cast gesture timing");
    require(restored && info.tick == 25 && restored->floor_height.cells[5][5] == 42 &&
            restored->actors.actors[0].random.state == 98765 && restored->party.members[0].character_id == member.character_id,
            "World snapshot lost simulation state or fixed-width identity");
    require(restored->random.state == source->random.state && restored->party.members[0].player.random.state == player.random.state,
        "Snapshot lost world or player random state");
    require(restored->sound_sequence == 1 && restored->sounds[1].x == 10000 && restored->sounds[1].note == 60,
        "Snapshot lost authoritative audio history");
    require(restored->party.members[0].loot_claims[0][4] == 9 && restored->party.members[0].loot_claims[4][12] == 8 &&
        restored->objects.objects[170].generation == 987, "Snapshot lost personal loot claims or object generations");
    require(restored->party.members[0].avatar == 23, "Snapshot lost the selected character body");
    require(restored->party.quest_rewards == source->party.quest_rewards && restored->party.members[0].quest_rewards == member.quest_rewards,
        "Snapshot lost campaign completion or the per-character reward ledger");
    require(!restored->actors.actors[0].animation_cache && !restored->actors.current &&
            !restored->effects.current_record && !restored->presentation, "Snapshot retained process-local bindings");
    player_apply_damage(restored->party.members[0].player, 100, 0, 0, KF_PLAYER_STATUS_NONE, 0, 0, 4096, 10);
    party_award_experience(*restored, restored->party.members[0].player, 10);
    require(restored->party.members[0].player.state.vitals.current_hp == 100 &&
            restored->party.members[0].player.state.experience == 0 && player.state.vitals.current_hp == 100,
            "Prediction committed damage or progression");
    for (const auto length : {std::size_t(0), std::size_t(4), std::size_t(20), bytes.size() - 1})
        require(!world_snapshot_decode(std::span<const u8>(bytes).first(length), *source, info),
                "Truncated snapshot was accepted");
    other = bytes;
    other[6] = 2;
    require(!world_snapshot_decode(other, *source, info), "Invalid wire boolean was accepted");
    other = bytes;
    other.push_back(0);
    require(!world_snapshot_decode(other, *source, info), "Trailing snapshot data was accepted");
    require(source->actors.actors[0].animation_cache == &second_cache && player.state.vitals.current_hp == 100,
            "Decoding modified the live source world");

    KfNetWorldLimits limits {};
    std::fill(std::begin(limits.asset_clips), std::end(limits.asset_clips), -1);
    auto typed = std::make_unique<KfNetWorld>();
    require(kf_net_world_decode(bytes.data(), bytes.size(), &limits, typed.get()) == KF_CODEC_OK,
            "C ABI rejected a valid world");
    KfNetWorldSummary summary {};
    require(kf_net_world_summary(bytes.data(), bytes.size(), &summary) == KF_CODEC_OK &&
        summary.floor == 1 && summary.level == 1 && summary.hp == 100 &&
        std::equal(std::begin(summary.owner), std::end(summary.owner), member.character_id.begin()),
        "Campaign preview lost host metadata or identity");
    summary.hp = 17;
    require(kf_net_world_summary(bytes.data(), bytes.size()-1, &summary) == KF_CODEC_INVALID && summary.hp == 17,
        "Truncated campaign preview published partial metadata");
    auto original_typed = std::make_unique<KfNetWorld>(*typed);
    other = bytes;
    other[other.size() - 35 - 3] = other[other.size() - 35 - 2] = 0; // skip the story record
    require(kf_net_world_decode(other.data(), other.size(), &limits, typed.get()) == KF_CODEC_INVALID &&
            std::memcmp(typed.get(), original_typed.get(), sizeof *typed) == 0,
            "Failed world decode published partial state");
    source->map.world_state.floors[3].records[42] = 97;
    require(world_snapshot_encode(*source, {1, 26, false}, other), "Cannot encode fast snapshot");
    require(kf_net_world_summary(other.data(), other.size(), &summary) == KF_CODEC_INVALID,
        "A partial network update was accepted as a campaign preview");
    restored = world_snapshot_decode(other, *source, info);
    require(restored && restored->effects.records[0].age == 76543, "Fast snapshot lost effect age");
    require(restored->party.members[0].player.cast_pose_ticks == 7, "Fast snapshot lost cast gesture timing");
    require(restored && !info.full && restored->map.world_state.floors[3].records[42] == 97,
            "Fast snapshot discarded persistent floor records");
    require(restored->random.state == source->random.state && restored->party.members[0].player.random.state == player.random.state,
        "Fast snapshot lost world or player random state");

    char directory[] = "/tmp/kf-coop-save-XXXXXX";
    require(mkdtemp(directory) && kf::save_storage_start(directory), "Cannot open isolated save storage");
    const u8 solo[] {1, 2, 3};
    require(kf::save_file_write(kf::SaveSlot::First, solo, sizeof solo) == kf::SaveFileResult::Ok,
            "Cannot seed isolated solo slot");
    CampaignRuntime campaign;
    source->campaign = &campaign;
    source->party.members[1] = member;
    auto &spectator = source->party.members[1];
    spectator.player.party_slot = 1;
    spectator.character_id = character_identity(2);
    spectator.presence = PartyPresence::Spectating;
    spectator.player.state.vitals.current_hp = 0;
    // The snapshot fixture above deliberately has no walkable terrain. Saving
    // revives beside the living player only when a real footprint is clear.
    for (auto &cell : source->collision.linear) cell = KF_MAP_CELL_FLOOR;
    for (auto &cell : source->floor_height.linear) cell = 0;
    auto saving = campaign_save(*source, player, kf::SaveSlot::First);
    saving.advance();
    require(campaign.writing && spectator.presence == PartyPresence::Spectating,
            "Revival occurred before durable save completion");
    campaign_poll(*source);
    saving.advance();
    require(saving.done() && saving.await_resume() == KF_SAVE_RESULT_OK && party_member_alive(spectator),
            "Successful campaign save did not revive spectators");
    require(campaign_read(kf::SaveSlot::First, other) && other == campaign.checkpoint,
            "Campaign save did not round-trip its committed checkpoint");
    auto saved_loot = world_snapshot_decode(other, *source, info);
    require(saved_loot && saved_loot->party.members[0].loot_claims[0][4] == 9 &&
        saved_loot->party.members[0].loot_claims[4][12] == 8, "Durable campaign save lost personal loot claims");
    require(saved_loot->party.quest_rewards == source->party.quest_rewards && saved_loot->party.members[0].quest_rewards == member.quest_rewards,
        "Durable campaign save lost the shared quest reward ledger");
    require(saved_loot->party.members[0].avatar == 23, "Durable campaign save lost character selection");
    auto world_sequence = source->random, player_sequence = player.random;
    require(world_random_next(*saved_loot) == kf::random_next(world_sequence) &&
        player_random_next(saved_loot->party.members[0].player) == kf::random_next(player_sequence),
        "Campaign restore did not resume world and player random sequences");
    auto alternate = std::make_unique<KfNetWorld>(*original_typed);
    alternate->header.floor = 2;
    alternate->members[0].player.progress_state_current_floor = 2;
    alternate->members[0].player.progress_state_highest_floor = 2;
    alternate->actors[0].slot_state = 0;
    alternate->actors[0].animation_clip = 3;
    auto alternate_limits = limits;
    alternate_limits.asset_clips[0] = 4;
    std::vector<u8> alternate_bytes(KF_NET_TRANSFER_LIMIT);
    std::size_t alternate_size = 0;
    require(kf_net_world_encode(alternate.get(), &alternate_limits, alternate_bytes.data(), alternate_bytes.size(), &alternate_size) == KF_CODEC_OK,
        "Cannot encode alternate-floor campaign fixture");
    alternate_bytes.resize(alternate_size);
    require(kf_net_world_decode(alternate_bytes.data(), alternate_bytes.size(), &limits, typed.get()) == KF_CODEC_INVALID,
        "Gameplay accepted an animation absent from the loaded floor");
    const auto &config = kf::net::application_config;
    const auto alternate_file = kf::campaign_file_pack(alternate_bytes, config.resources, config.avatar_recipe);
    std::vector<u8> untouched {7,8,9};
    require(!kf::campaign_file_unpack(alternate_file, "different-resources", config.avatar_recipe, untouched) &&
        untouched == std::vector<u8>({7,8,9}), "Incompatible campaign changed the output snapshot");
    auto corrupted = alternate_file;
    corrupted.back() ^= 1;
    require(!kf::campaign_file_unpack(corrupted, config.resources, config.avatar_recipe, untouched) &&
        untouched == std::vector<u8>({7,8,9}), "Campaign checksum failure published data");
    require(kf::campaign_file_write_begin(kf::SaveSlot::Second, alternate_file.data(), alternate_file.size()) &&
        kf::campaign_file_write_poll() == kf::SaveFileResult::Ok, "Cannot save alternate-floor catalogue fixture");
    const u8 damaged[] {1,2,3};
    require(kf::campaign_file_write_begin(kf::SaveSlot::Third, damaged, sizeof damaged) &&
        kf::campaign_file_write_poll() == kf::SaveFileResult::Ok, "Cannot seed damaged catalogue fixture");
    KfSaveSlotSummary catalogue[3];
    require(campaign_read_catalog(*source, catalogue) == KF_SAVE_RESULT_OK &&
        catalogue[0].state == KfSaveSlotState::Ready && catalogue[1].state == KfSaveSlotState::Ready &&
        catalogue[1].current_floor == KF_FLOOR_2 && catalogue[2].state == KfSaveSlotState::Damaged,
        "Catalogue hid another floor's save or treated a damaged slot as empty");
    std::size_t solo_size = 0;
    u8 preserved[16] {};
    require(kf::save_file_read(kf::SaveSlot::First, preserved, sizeof preserved, &solo_size) == kf::SaveFileResult::Ok &&
            solo_size == sizeof solo && std::memcmp(preserved, solo, sizeof solo) == 0,
            "Campaign write overwrote a solo save");
    kf::save_storage_shutdown();
    std::filesystem::remove_all(directory);
}

static void shared_floor1_triggers()
{
    auto world = std::make_unique<WorldState>();
    world->party.enabled = true;
    actor_pool_clear(*world);
    map_object_pool_clear(*world);
    for (u8 slot=0;slot<2;++slot) {
        auto &member = world->party.members[slot];
        member.presence = PartyPresence::Living;
        member.connected = true;
        member.player.state.vitals.current_hp = 100;
        member.player.local_view = false;
    }
    auto &host = world->party.members[0].player;
    auto &guest = world->party.members[1].player;
    host.state.motion_state.map_cell.x = 30;
    host.state.motion_state.map_cell.z = 50;
    guest.state.motion_state.map_cell.x = 8;
    guest.state.motion_state.map_cell.z = 35;
    auto &script = map_floor_script(*world,KF_FLOOR_1).floor1;
    world->prediction = true;
    map_ambient_script_floor1(*world,host);
    require(script.actor_activation_stage == KF_MAP_TRIGGER_AWAIT_ENTRY,"Prediction changed a shared floor trigger");
    world->prediction = false;
    map_ambient_script_floor1(*world,host);
    map_ambient_script_floor1(*world,host);
    require(script.actor_activation_stage == KF_MAP_TRIGGER_AWAIT_EXIT,"Host outside the area bypassed a guest's trigger");
    guest.state.motion_state.map_cell = host.state.motion_state.map_cell;
    map_ambient_script_floor1(*world,host);
    require(script.actor_activation_stage == KF_MAP_TRIGGER_COMPLETE,"Party leaving the area did not complete the trigger");
    guest.state.motion_state.map_cell.x = 3;
    guest.state.motion_state.map_cell.z = 28;
    map_ambient_script_floor1(*world,host);
    require(script.object_removal_stage == KF_MAP_TRIGGER_AWAIT_EXIT,"Guest could not arm the removal trigger");
    host.state.motion_state.map_cell = guest.state.motion_state.map_cell;
    guest.state.motion_state.map_cell.x = 30;
    guest.state.motion_state.map_cell.z = 50;
    map_ambient_script_floor1(*world,host);
    require(script.object_removal_stage == KF_MAP_TRIGGER_AWAIT_EXIT,"Objects vanished while another player remained in the area");
    world->party.members[0].presence = PartyPresence::Spectating;
    map_ambient_script_floor1(*world,host);
    require(script.object_removal_stage == KF_MAP_TRIGGER_COMPLETE,"A spectator held the area trigger open");
}

static void shared_ending(WorldState &world)
{
    require(!party_ending_begin(world),"Campaign ended before the boss was defeated");
    map_floor_script(world,KF_FLOOR_5).floor5.boss_defeat = KF_MAP_SCRIPT_SET;
    for (const auto floor : {KF_FLOOR_5,KF_FLOOR_1}) {
        world.story = {};
        world.floor = floor;
        world.variant = floor == KF_FLOOR_5 ? KF_FLOOR5_ENTRY_VARIANT : KF_MAP_VARIANT_DEFAULT;
        for (auto &member : world.party.members) {
            member.player.state.progress_state.current_floor = floor;
            member.player.state.map_variant = world.variant;
        }
        auto &host = world.party.members[0].player;
        host.state.motion_state.map_cell.x = floor == KF_FLOOR_5 ? 39 : 15;
        host.state.motion_state.map_cell.z = floor == KF_FLOOR_5 ? 47 : 2;
        auto exit = player_warp_trigger_update(world,host);
        exit.advance();
        require(exit.done() && exit.await_resume(),"Original campaign exit did not reach the ending");
        world.prediction = true;
        require(!party_ending_begin(world),"Prediction ended the campaign");
        world.prediction = false;
        require(party_ending_begin(world) && !party_ending_begin(world),"Ending was not a single terminal transition");
        world.party.members[1].connected = false;
        require(party_ending_waiting(world),"Interrupted guest lost the opportunity to receive the ending");
        for (const bool full : {false,true}) {
            std::vector<u8> bytes;
            WorldSnapshotInfo info;
            require(world_snapshot_encode(world,{world.epoch,17,full},bytes),"Cannot encode terminal campaign state");
            auto restored = world_snapshot_decode(bytes,world,info);
            require(restored && restored->story.kind == party_story_ending && party_ending_waiting(*restored),
                "Resnapshot lost the terminal state or pending readers");
            restored->prediction = false;
            for (int i=0;i<1000;++i) party_story_tick(*restored);
            require(restored->story.kind == party_story_ending && !restored->story.tick &&
                !party_story_ready(*restored,1,1),"Terminal scene advanced or accepted a dialogue command");
        }
        world.story.ready_mask |= 2;
        require(party_ending_waiting(world),"An admitted spectator was omitted from the ending handoff");
        world.story.ready_mask |= 4;
        require(!party_ending_waiting(world),"A waiting newcomer blocked the acknowledged ending");
        world.story.ready_mask = 1;
        world.party.members[1].presence = world.party.members[2].presence = PartyPresence::Disconnected;
        require(!party_ending_waiting(world),"Previously absent campaign characters blocked the ending");
        world.party.members[1].presence = PartyPresence::Living;
        world.party.members[2].presence = PartyPresence::Spectating;
    }
}

static void shared_boss_reveal(WorldState &world)
{
    using namespace kf::net;
    SnapshotAssetFixture boss_asset(0, 4);
    auto &guest = world.party.members[1].player;
    auto &spectator = world.party.members[2];
    spectator = world.party.members[0];
    spectator.character_id = character_identity(3);
    spectator.presence = PartyPresence::Spectating;
    spectator.player.party_slot = 2;
    spectator.player.state.vitals.current_hp = 0;
    world.party.members[3].presence = PartyPresence::Waiting;
    world.party.members[3].connected = true;
    world.party.members[3].generation = 1;
    guest.state.motion_state.map_cell.x = 39;
    guest.state.motion_state.map_cell.z = 7;
    guest.state.camera_position = {79000,-1700,15000};
    guest.state.camera_rotation.vy = 0;
    auto trigger = [&] {
        auto task = map_ambient_script_floor5(world,world.party.members[0].player);
        task.advance();
        require(task.done(),"Boss trigger suspended the host in a local screen loop");
    };
    trigger();
    require(!world.story.kind,"Boss trigger ignored facing");
    guest.state.camera_rotation.vy = 2048;
    world.prediction = true;
    trigger();
    require(!world.story.kind,"Prediction started the boss encounter");
    world.prediction = false;
    trigger();
    require(world.story.kind == party_story_boss && world.story.page == 1 && world.story.initiator == 1,
        "Guest entering the boss threshold did not start the shared reveal");
    PlayerCommandState commands[party_capacity];
    auto ready = [&](u8 slot,u16 page) {
        Command request {{MessageKind::Command,world.epoch,commands[slot].last_sequence+1,world.party.members[slot].generation},
            CommandKind::StoryReady,page,0}, decoded;
        std::vector<u8> bytes;
        require(command_encode(request,bytes) && command_decode(bytes,decoded),"Boss acknowledgement failed the protocol codec");
        return party_apply_command(world,slot,commands[slot],decoded);
    };
    auto snapshot = [&] {
        std::vector<u8> bytes;
        WorldSnapshotInfo info;
        require(world_snapshot_encode(world,{world.epoch,1,true},bytes),"Cannot encode boss scene state");
        auto restored = world_snapshot_decode(bytes,world,info);
        require(restored && restored->story.page == world.story.page && restored->story.ready_mask == world.story.ready_mask,
            "Resnapshot lost boss page or acknowledgements");
    };
    require(!ready(1,2) && !ready(3,1),"Wrong page or waiting player acknowledged the reveal");
    require(ready(1,1) && !ready(1,1),"Boss page acknowledgement was not once per character");
    require(ready(0,1),"Host could not acknowledge the boss page");
    for (int i=0;i<1000;++i) party_story_tick(world);
    require(world.story.page == 1 && !world.story.tick,"Boss reveal skipped a connected spectator or advanced while reading");
    snapshot();
    spectator.connected = false;
    party_story_tick(world);
    require(world.story.kind == party_story_boss && !world.story.page && !world.story.ready_mask,
        "Disconnected reader held the first page open");
    snapshot();
    party_story_tick(world);
    require(!world.story.page,"Boss reveal omitted its inter-page world frame");
    party_story_tick(world);
    require(world.story.page == 2 && !world.story.tick,"Boss reveal did not advance to page two");
    spectator.connected = true;
    require(!ready(1,1) && ready(2,2),"Stale page was accepted or a returning spectator cannot acknowledge");
    require(ready(0,2),"Host could not acknowledge page two");
    party_story_tick(world);
    require(world.story.page == 2,"Boss encounter began before the last reader finished");
    snapshot();
    const auto &region = map_copy_regions[kf_enum_encode<u8>(KF_MAP_COPY_FLOOR5_BOSS_ENCOUNTER)];
    world.floor_height.cells[region.source_z][region.source_x] = 19;
    world.party.members[1].connected = false;
    party_story_tick(world);
    require(!world.story.kind && world.floor_height.cells[region.destination_z][region.destination_x] == 19 &&
        world.actors.definitions.entries[KF_FLOOR5_BOSS_DEFINITION].action_animations[KF_ACTOR_ANIM_SLOT_MELEE] == KF_ANIMATION_CLIP_THIRD &&
        world.actors.definitions.entries[KF_FLOOR5_BOSS_DEFINITION].action_animations[KF_ACTOR_ANIM_SLOT_EFFECT0] == KF_ANIMATION_CLIP_FOURTH,
        "Boss encounter did not activate with its shared terrain and attacks");
    world.floor_height.cells[region.source_z][region.source_x] = 20;
    world.party.members[1].connected = true;
    trigger();
    require(!world.story.kind && world.floor_height.cells[region.destination_z][region.destination_x] == 19,
        "Boss reveal or terrain activation repeated");
    snapshot();
    shared_ending(world);
}

static void shared_story_scenes()
{
    using namespace kf::net;
    SnapshotAssetFixture npc_asset(10, 1), blast_asset(41, 1);
    for (const auto floor : {KF_FLOOR_2, KF_FLOOR_5}) {
        auto world = std::make_unique<WorldState>();
        world->party.enabled = true;
        world->floor = floor;
        world->variant = floor == KF_FLOOR_5 ? KF_FLOOR5_ENTRY_VARIANT : KF_MAP_VARIANT_DEFAULT;
        actor_pool_clear(*world);
        map_object_pool_clear(*world);
        effect_pool_reset(*world);
        // A full drop pool must still reserve a fresh scene object identity.
        if (floor == KF_FLOOR_5) for (unsigned slot = 180; slot < 190; ++slot) {
            auto &object = world->objects.objects[slot];
            object.object_id = KF_ITEM_DRAGON_SWORD;
            object.cell_x = object.cell_z = 5;
            for (auto &member : world->party.members) member.loot_claims[4][slot] = 15;
        }
        for (auto &event : world->map.events) event.state = KF_MAP_EVENT_FREE;
        for (u8 slot = 0; slot < 2; ++slot) {
            auto &member = world->party.members[slot];
            member.character_id = character_identity(slot + 1);
            member.generation = 1;
            member.presence = PartyPresence::Living;
            member.connected = true;
            auto &p = member.player;
            p.party_slot = slot;
            p.local_view = false;
            p.state.progress_state.level = 1;
            p.state.progress_state.current_floor = p.state.progress_state.highest_floor = floor;
            p.state.map_variant = world->variant;
            p.state.vitals = {100,100,100,100};
            p.state.camera_position = {10000,-1700,10000};
            p.state.motion_state.map_cell = {5,5};
            p.state.weapon_attack_phase = KF_WEAPON_ATTACK_INACTIVE;
            p.state.equipped_weapon_id = p.state.equipped_head_armor_id = p.state.equipped_body_armor_id =
                p.state.equipped_shield_id = p.state.equipped_arm_armor_id = p.state.equipped_leg_armor_id =
                p.state.equipped_accessory_id = KF_OBJECT_NONE;
            p.state.selected_magic_id = KF_MAGIC_NONE;
            p.item_stock[0][kf_enum_encode<u8>(KF_ITEM_DRAGON_SWORD)] = 2;
            collision_adjust_cell_occupancy(*world,5,5,1);
        }
        auto &player = world->party.members[1].player;
        const u16 index = floor == KF_FLOOR_2 ? KF_FLOOR2_REVEAL_EVENT : KF_FLOOR5_WEAPON_TRANSFORM_EVENT;
        auto &event = world->map.events[index];
        const auto probe = vector_yaw_probe_xz(player.state.camera_position,0,MAP_INTERACTION_PROBE_DISTANCE);
        event.state = KF_MAP_EVENT_ACTIVE;
        event.behavior = KF_MAP_EVENT_BEHAVIOR_WANDER;
        event.character_id = kf_enum_decode<KfCharacterId>(10);
        event.radius = 400;
        event.reference_position = {probe.x,0,probe.z};
        const u8 stage = floor == KF_FLOOR_2 ? 2 : 5;
        event.dialogue = {stage,stage,1,0};
        event.dialogue_pages.last_page[stage-1] = 7;
        PlayerCommandState commands;
        u32 sequence = 0;
        auto command = [&](CommandKind kind) {
            Command request {{MessageKind::Command,world->epoch,++sequence,1},kind,index,0}, decoded;
            std::vector<u8> bytes;
            require(command_encode(request,bytes) && command_decode(bytes,decoded),"Story command failed wire validation");
            return party_apply_command(*world,1,commands,decoded);
        };
        require(!command(CommandKind::Cancel) && !world->story.kind,"Unsolicited completion started a scene");
        require(command(CommandKind::Interact) && !world->story.kind,"Dialogue started the scene before completion");
        require(command(CommandKind::Cancel) && world->story.initiator == 1 && world->story.kind,
            "Guest dialogue completion did not start the shared scene");
        require(!command(CommandKind::Cancel) && !party_at_same_entrance(*world),"Scene accepted replay or party travel");
        const auto sword = world->story.object;
        const auto generation = world->story.generation;
        auto prediction = std::make_unique<WorldState>();
        world_clone_simulation(*world,*prediction);
        party_story_tick(*prediction);
        require(prediction->story.tick == 0,"Guest prediction advanced a story scene");
        PartyRuntime runtime;
        const unsigned duration = floor == KF_FLOOR_2 ? 50 : 621;
        for (unsigned tick = 0; tick <= duration; ++tick) {
            std::vector<u8> bytes;
            require(world_snapshot_encode(*world,{world->epoch,runtime.tick,tick%2 == 0},bytes),
                "A shared scene phase cannot cross the world codec");
            if (tick == 0 || tick == 105 || tick == 361 || tick == duration) {
                WorldSnapshotInfo info;
                auto restored = world_snapshot_decode(bytes,*world,info);
                require(restored && restored->story.kind == world->story.kind && restored->story.tick == world->story.tick &&
                    restored->story.effect == world->story.effect,"Scene state was lost in a resnapshot");
                if (tick == 105) {
                    restored->prediction = false;
                    party_story_tick(*restored);
                    require(restored->story.tick == tick+1 && restored->party.members[1].player.item_stock[0][10] == 1,
                        "Restoring a scene consumed the offering again");
                }
            }
            if (tick == duration) break;
            if (tick == 10) party_set_connected(*world,runtime,1,false);
            if (tick == 20) require(party_resume(*world,runtime,1),"Initiator cannot reconnect during the scene");
            runtime.inputs[0].buttons = 0xffff;
            party_advance_tasks(*world,runtime);
            party_tick(*world,runtime);
            require(world->party.members[0].player.state.camera_position.vx == 10000 &&
                world->party.members[0].player.state.vitals.current_hp == 100 && player.state.vitals.current_hp == 100,
                "Scene moved an uninvolved player or damaged the party");
        }
        require(!world->story.kind,"Shared scene did not finish");
        if (floor == KF_FLOOR_2) {
            require(event.state == KF_MAP_EVENT_DISABLED &&
                map_floor_script(*world,KF_FLOOR_5).floor5.character_arrived == KF_MAP_SCRIPT_SET,
                "Transfer scene did not publish shared progression");
        } else {
            const auto &object = world->objects.objects[sword];
            require(object.object_id == KF_ITEM_MOONLIGHT_SWORD && object.generation == generation &&
                object.action == KF_MAP_OBJECT_OP_FALL_AND_TIP &&
                map_floor_script(*world,KF_FLOOR_5).floor5.weapon_transformed == KF_MAP_SCRIPT_SET,
                "Transformation did not create the final shared sword");
            require(player.item_stock[0][10] == 1 && world->party.members[0].player.item_stock[0][10] == 2 &&
                !world->party.members[0].loot_claims[4][sword] && !world->party.members[1].loot_claims[4][sword],
                "Transformation consumed the wrong offering or preclaimed personal loot");
            require(!party_story_begin(*world,player,index),"Completed transformation can be repeated");
            shared_boss_reveal(*world);
        }
    }
}

static void retail_character_import(const char *disc, const char *reference)
{
    auto *file = std::fopen(disc, "rb");
    require(file && !std::fseek(file, 0, SEEK_END), "Cannot open local KFIII fixture");
    const auto size = std::ftell(file);
    require(size > 0 && size <= 800*1024*1024, "Invalid KFIII fixture size");
    auto *importer = kf_avatar_import_open(static_cast<u32>(size));
    require(importer, "Cannot create disc importer");
    u32 result_size = 0;
    require(!kf_avatar_import_result(importer, &result_size), "Importer published an incomplete pack");
    KfAvatarRead request{};
    std::vector<u8> chunk;
    unsigned reads = 0;
    std::size_t total = 0;
    int state;
    while ((state = kf_avatar_import_request(importer, &request)) == 1) {
        require(request.length <= 16*1024*1024 && std::uint64_t(request.offset)+request.length <= std::uint64_t(size),
            "Importer requested an unbounded disc read");
        chunk.resize(request.length);
        require(!std::fseek(file, request.offset, SEEK_SET) && std::fread(chunk.data(),1,chunk.size(),file) == chunk.size(),
            "Cannot read requested disc extent");
        if (!kf_avatar_import_supply(importer, chunk.data(), chunk.size())) {
            u8 error[256]{};
            kf_avatar_import_error(importer,error,sizeof error);
            std::fprintf(stderr,"Import error: %s\n",error);
            require(false,"Retail character import failed");
        }
        total += chunk.size(); ++reads;
    }
    std::fclose(file);
    require(state == 0 && reads >= 6 && total < 32*1024*1024, "Import did not stay within character archives");
    const auto *result = kf_avatar_import_result(importer,&result_size);
    require(result && result_size <= KF_AVATAR_MAX_BYTES,"Importer did not publish a bounded pack");
    file = std::fopen(reference,"rb");
    require(file && !std::fseek(file,0,SEEK_END) && std::ftell(file) == result_size && !std::fseek(file,0,SEEK_SET),
        "Imported and reference character pack sizes differ");
    std::vector<u8> expected(result_size);
    require(std::fread(expected.data(),1,expected.size(),file) == expected.size(),"Cannot read reference character pack");
    std::fclose(file);
    for (std::size_t i=0; i<expected.size(); ++i) if (expected[i] != result[i]) {
        std::fprintf(stderr,"Character pack divergence at byte %zu: imported %u, reference %u\n",i,result[i],expected[i]);
        require(false,"Character import differs from the independent Python converter");
    }
    kf_avatar_import_close(importer);
    // Exercise the ISO path against the same retail payload without making a
    // second half-gigabyte disc copy: satisfy logical reads from BIN sectors.
    importer = kf_avatar_import_open(static_cast<u32>(size / 2352 * 2048));
    require(importer,"Cannot create ISO character importer");
    file = std::fopen(disc,"rb");
    require(file,"Cannot reopen BIN payload source");
    while ((state = kf_avatar_import_request(importer,&request)) == 1) {
        require(request.offset % 2048 == 0,"ISO read was not sector aligned");
        chunk.resize(request.length);
        for (std::size_t at=0; at<chunk.size(); at+=2048) {
            const auto count = std::min<std::size_t>(2048,chunk.size()-at);
            const auto offset = (std::uint64_t(request.offset)+at)/2048*2352+24;
            require(!std::fseek(file,static_cast<long>(offset),SEEK_SET) && std::fread(chunk.data()+at,1,count,file) == count,
                "Cannot supply ISO payload sector");
        }
        require(kf_avatar_import_supply(importer,chunk.data(),chunk.size()),"ISO payload import failed");
    }
    std::fclose(file);
    result = kf_avatar_import_result(importer,&result_size);
    require(state == 0 && result && result_size == expected.size() && !std::memcmp(result,expected.data(),expected.size()),
        "ISO and BIN character conversion differ");
    kf_avatar_import_close(importer);
    require(kf::avatars_load(reference),"Cannot install reference pack");
    const std::string hash = kf::avatars_hash();
    require(kf::avatars_import_disc(disc) && hash == kf::avatars_hash(),"Native disc import changed compatibility identity");
    kf::avatars_release();
    std::fprintf(stdout,"KFIII import matched %u reference bytes using %u reads (%zu disc bytes)\n",result_size,reads,total);
}

static void resource_identity()
{
    namespace fs = std::filesystem;
    char directory[] = "/tmp/kf-resource-hash-XXXXXX";
    require(mkdtemp(directory), "Cannot create isolated resource tree");
    const fs::path root(directory);
    fs::create_directories(root / "KF/COM");
    const auto write = [&](const char *name, const char *data, std::size_t size) {
        auto *file = std::fopen((root / name).c_str(), "wb");
        require(file && std::fwrite(data, 1, size, file) == size && !std::fclose(file), "Cannot write resource fixture");
    };
    write("OPEN.EXE", "", 0);
    write("KF/COM/COM.DAT", "\0\1\xff", 3);
    require(kf::data_files_set_root(directory), "Cannot select resource fixture");
    // Independently computed with Python hashlib using the browser's manifest format.
    const std::string expected = "ddc397636618d4b7fcf85da8770ada0cb4eb6469c1b7996aa7e3ec6165a67bf3";
    require(kf::data_files_hash() == expected, "Resource identity disagrees with the canonical manifest hash");
    write("KF/COM/COM.DAT", "\0\2\xff", 3);
    require(kf::data_files_hash() != expected, "Same-size content changes did not change compatibility");
    write("KF/COM/COM.DAT", "\0\1\xff", 3);
    fs::rename(root / "OPEN.EXE", root / "GAME.EXE");
    require(kf::data_files_hash() != expected, "Resource identity ignored path changes");
    fs::rename(root / "GAME.EXE", root / "OPEN.EXE");
    fs::create_symlink(root / "KF/COM/COM.DAT", root / "LINK");
    require(kf::data_files_hash().empty(), "Resource verification followed an unenumerated file link");
    fs::remove(root / "LINK");
    fs::resize_file(root / "OPEN.EXE", kf::disc_import_limit + 1);
    require(kf::data_files_hash().empty(), "Oversized resource tree bypassed the byte limit");
    fs::resize_file(root / "OPEN.EXE", 0);
    require(kf::data_files_hash() == expected, "Resource hashing failed to recover after a rejected tree");
    fs::remove_all(root / "KF");
    fs::remove(root / "OPEN.EXE");
    require(kf::data_files_hash().empty(), "An empty directory received a resource identity");
    fs::remove(root);
}

static int ending_fixture_host(const char *url, const char *pack, const char *mode, const char *data)
{
    using namespace kf::net;
    require(kf::avatars_load(pack),"Cannot load the ending fixture's character pack");
    Config config;
    config.host = true;
    config.signaling_url = url;
    require(kf::data_files_set_root(data), "Cannot select ending fixture resources");
    config.resources = kf::data_files_hash();
    require(!config.resources.empty(), "Cannot verify ending fixture resources");
    config.avatar_recipe = kf::avatars_hash();
    config.credential = std::string(64,'1');
    auto *transport = transport_open(config);
    require(transport,"Cannot open ending fixture room");
    auto world = std::make_unique<WorldState>();
    world->party.enabled = true;
    actor_pool_clear(*world);
    map_object_pool_clear(*world);
    effect_pool_reset(*world);
    for (auto &event : world->map.events) event.state = KF_MAP_EVENT_FREE;
    for (u8 slot=0;slot<2;++slot) {
        auto &member = world->party.members[slot];
        member.presence = PartyPresence::Living;
        member.connected = true;
        member.generation = 1;
        auto &p = member.player;
        p.party_slot = slot;
        p.state.progress_state.level = 1;
        p.state.progress_state.current_floor = KF_FLOOR_1;
        p.state.progress_state.highest_floor = KF_FLOOR_5;
        p.state.vitals = {100,100,100,100};
        p.state.camera_position = {10000,-1700,10000};
        p.state.motion_state.map_cell = {5,5};
        p.state.weapon_attack_phase = KF_WEAPON_ATTACK_INACTIVE;
        p.state.equipped_weapon_id = p.state.equipped_head_armor_id = p.state.equipped_body_armor_id =
            p.state.equipped_shield_id = p.state.equipped_arm_armor_id = p.state.equipped_leg_armor_id =
            p.state.equipped_accessory_id = KF_OBJECT_NONE;
        p.state.selected_magic_id = KF_MAGIC_NONE;
    }
    map_floor_script(*world,KF_FLOOR_5).floor5.boss_defeat = KF_MAP_SCRIPT_SET;
    const bool abrupt = !std::strcmp(mode,"drop");
    if (std::strcmp(mode,"ordinary")) require(party_ending_begin(*world),"Cannot create the terminal fixture");
    std::vector<std::vector<u8>> fragments;
    std::size_t next = 0;
    bool received = false;
    const auto deadline = std::chrono::steady_clock::now()+std::chrono::seconds(30);
    while (std::chrono::steady_clock::now() < deadline) {
        Event event;
        while (transport_poll(*transport,event)) {
            if (event.kind == EventKind::Room) {
                world->party.members[0].character_id = event.identity;
                std::printf("ROOM %s\n",event.text.c_str());
                std::fflush(stdout);
            } else if (event.kind == EventKind::Connected && event.peer == 1) {
                world->party.members[1].character_id = event.identity;
                std::vector<u8> bytes;
                require(world_snapshot_encode(*world,{world->epoch,17,true},bytes),"Cannot encode ending fixture");
                fragments = fragment_encode({MessageKind::Fragment,world->epoch,17,1},bytes);
                next = 0;
            } else if (event.kind == EventKind::Packet && event.peer == 1 && event.channel == Channel::Actions) {
                MessageHeader header;
                if (packet_header(event.packet,header) && header.kind == MessageKind::Loaded && header.epoch == world->epoch &&
                    header.sequence == 17 && header.generation == 1 && event.packet.size() == message_header_bytes &&
                    next == fragments.size() && !received) {
                    received = true;
                    std::puts("SNAPSHOT RECEIVED");
                    std::fflush(stdout);
                }
            } else if (event.kind == EventKind::Error) require(false,event.text.c_str());
        }
        if (received && !abrupt) break;
        if (next < fragments.size() && transport_send(*transport,1,Channel::Actions,fragments[next])) ++next;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    transport_close(transport);
    return received ? 0 : 1;
}

static std::unique_ptr<WorldState> retail_fixture_open(const char *resources)
{
    require(kf::language_resources_start(resources, kf::Language::Japanese, nullptr),
        "Cannot open retail world resources");
    require(kf::host_start(), "Cannot start isolated world-state check");
    game_restore_initial_state();
    auto world = std::make_unique<WorldState>();
    auto &member = world->party.members[0];
    auto &player = member.player;
    memory_set_allocation_mode(memory_arena, KF_MEMORY_CREATE_ARENA);
    audio_initialize();
    display_initialize();
    item_load_database();
    actor_pool_clear(*world);
    map_object_pool_clear(*world);
    effect_pool_reset(*world);
    map_event_timers_reset(*world);
    common_resources_load(*world, player);
    game_initialize_session(*world, player);
    player.state.audio_music_enabled = KF_PLAYER_OPTION_OFF;
    player.state.audio_effects_enabled = KF_PLAYER_OPTION_OFF;
    player.local_view = false;
    member.presence = PartyPresence::Living;
    member.connected = true;
    member.generation = 1;
    member.character_id = character_identity(1);
    player.party_slot = 0;
    world->party.enabled = true;
    memory_set_allocation_mode(memory_arena, KF_MEMORY_REBASE_ARENA);
    return world;
}

static void retail_fixture_load(WorldState &world, unsigned floor, unsigned variant)
{
    auto &player = world.party.members[0].player;
    animation_cache_release_all();
    player.state.progress_state.current_floor = kf_enum_decode<KfFloorId>(floor);
    player.state.progress_state.highest_floor = kf_enum_decode<KfFloorId>(floor);
    player.state.map_variant = kf_enum_decode<KfMapVariant>(variant);
    const auto &entry = floor_entry_cells[floor - 1];
    player.state.camera_position = {entry.x * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER, 0,
        entry.z * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER};
    auto loading = map_load_floor_wrapper(world, player);
    for (unsigned frame = 0; !loading.done(); ++frame) {
        require(frame < 600, "Isolated floor load did not complete");
        loading.advance();
    }
}

static void retail_fixture_close()
{
    animation_cache_release_all();
    game_shutdown();
    memory_destroy_arena(memory_arena);
    kf::host_shutdown();
}

static int party_travel_check(const char *resources)
{
    for (const bool dead_host : {false, true}) {
        auto world = retail_fixture_open(resources);
        retail_fixture_load(*world, 5, 1);
        auto &host = world->party.members[0];
        for (u8 slot = 1; slot < party_capacity; ++slot) {
            auto &member = world->party.members[slot];
            member.player = host.player;
            member.player.party_slot = slot;
            member.player.local_view = false;
            member.player.state.weapon_animation_cache = nullptr;
            member.player.state.weapon_asset_buffer = nullptr;
            member.connected = slot != 3;
            member.character_id = character_identity(slot + 1);
            member.generation = 1;
            member.presence = slot == 1 ? PartyPresence::Living :
                slot == 2 ? PartyPresence::Spectating : PartyPresence::Disconnected;
            if (slot == 1) {
                const auto cell = member.player.state.motion_state.map_cell;
                collision_adjust_cell_occupancy(*world, cell.x, cell.z, 1);
            }
        }
        if (dead_host) {
            const auto cell = host.player.state.motion_state.map_cell;
            collision_adjust_cell_occupancy(*world, cell.x, cell.z, -1);
            host.presence = PartyPresence::Spectating;
        }
        for (u8 slot = 0; slot < party_capacity; ++slot) {
            auto &member = world->party.members[slot];
            auto &player = member.player;
            if (party_member_alive(member)) {
                const auto cell = player.state.motion_state.map_cell;
                collision_adjust_cell_occupancy(*world, cell.x, cell.z, -1);
            }
            player.state.camera_position = {70 * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER, 0,
                61 * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER};
            player_sync_position_to_map(*world, player);
            if (!party_member_alive(member)) collision_adjust_cell_occupancy(*world, 70, 61, -1);
            player.state.vitals.current_hp = member.presence == PartyPresence::Spectating ? 0 : 17;
            player.state.gold = 100 + slot;
            player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_MEDICINAL_HERB)] = slot + 1;
            player.state.weapon_attack_phase = 3;
            player.cast_pose_ticks = 6;
        }
        require(party_at_same_entrance(*world), "Travel fixture is not at a real shared entrance");
        const auto before = world->collision_flags;
        for (const unsigned variant : {2, 1}) {
            auto travel = party_travel(*world);
            for (unsigned frame = 0; !travel.done(); ++frame) {
                require(frame < 600, "Party travel did not complete");
                travel.advance();
            }
            require(!travel.await_resume() && world->floor == KF_FLOOR_5 &&
                kf_enum_encode<unsigned>(world->variant) == variant, "Party travelled to the wrong world");
            for (u8 slot = 0; slot < party_capacity; ++slot) {
                const auto &member = world->party.members[slot];
                const auto &player = member.player;
                require(player.state.motion_state.map_cell.x == (variant == 2 ? 18 : 70) &&
                    player.state.motion_state.map_cell.z == (variant == 2 ? 37 : 61) &&
                    player.state.map_variant == world->variant &&
                    player.state.weapon_attack_phase == KF_WEAPON_ATTACK_INACTIVE && !player.cast_pose_ticks &&
                    map_cells_equal(player.state.previous_map_cell, player.state.motion_state.map_cell),
                    "Travel left a member behind or retriggered an entrance");
                require(player.state.vitals.current_hp == (member.presence == PartyPresence::Spectating ? 0 : 17) &&
                    player.state.gold == 100 + slot && player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_MEDICINAL_HERB)] == slot + 1 &&
                    member.character_id == character_identity(slot + 1), "Travel changed personal state");
            }
            for (const bool full : {false, true}) {
                std::vector<u8> bytes;
                require(world_snapshot_encode(*world, {world->epoch, 1, full}, bytes), "Travel produced an invalid snapshot");
                WorldSnapshotInfo info;
                require(bool(world_snapshot_decode(bytes, *world, info)), "Travel snapshot failed full validation");
            }
        }
        require(!std::memcmp(&before, &world->collision_flags, sizeof before),
            "Party round trip left phantom collision occupancy");
        std::printf("Floor-five round trip with %s host: party, personal state and occupancy preserved\n",
            dead_host ? "spectating" : "living");
        std::fflush(stdout);
        for (auto &member : world->party.members) {
            auto &player = member.player;
            if (party_member_alive(member)) {
                const auto cell = player.state.motion_state.map_cell;
                collision_adjust_cell_occupancy(*world, cell.x, cell.z, -1);
            }
            player.state.camera_position = {39 * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER, 0,
                69 * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER};
            player_sync_position_to_map(*world, player);
            if (!party_member_alive(member)) collision_adjust_cell_occupancy(*world, 39, 69, -1);
        }
        const auto floor_before = world->collision_flags;
        for (const unsigned floor : {4, 5}) {
            auto travel = party_travel(*world);
            for (unsigned frame = 0; !travel.done(); ++frame) {
                require(frame < 600, "Cross-floor party travel did not complete");
                travel.advance();
            }
            require(!travel.await_resume() && kf_enum_encode<unsigned>(world->floor) == floor,
                "Party failed to cross the shared floor entrance");
            for (u8 slot = 0; slot < party_capacity; ++slot) {
                const auto &member = world->party.members[slot];
                const auto &player = member.player;
                require(player.state.progress_state.current_floor == world->floor && player.state.map_variant == world->variant &&
                    player.state.vitals.current_hp == (member.presence == PartyPresence::Spectating ? 0 : 17) &&
                    player.state.gold == 100 + slot && player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_MEDICINAL_HERB)] == slot + 1,
                    "Cross-floor travel lost a party member's state");
            }
            std::vector<u8> bytes;
            require(world_snapshot_encode(*world, {world->epoch, 2, true}, bytes), "Cross-floor travel produced an invalid snapshot");
        }
        require(!std::memcmp(&floor_before, &world->collision_flags, sizeof floor_before),
            "Cross-floor round trip duplicated collision occupancy");
        std::printf("Floor-five/four round trip with %s host: party and occupancy preserved\n",
            dead_host ? "spectating" : "living");
        std::fflush(stdout);
        retail_fixture_close();
    }
    return 0;
}

static int world_state_check(const char *resources)
{
    auto world = retail_fixture_open(resources);
    auto &player = world->party.members[0].player;
    for (const auto [floor, variant] : std::array<std::pair<unsigned, unsigned>, 7> {{
            {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 1}, {5, 2}, {5, 3}}}) {
        retail_fixture_load(*world, floor, variant);
        PartyRuntime runtime;
        std::vector<u8> bytes;
        // A direct cast at the entry point needs no navigation or saved game.
        if (floor == 1) {
            player.learned_magic[kf_enum_encode<u8>(KF_MAGIC_FIRE_BALL)] = KF_MAGIC_LEARNED;
            player_select_magic(*world, player, KF_MAGIC_FIRE_BALL);
            player.state.magic_charge = KF_PLAYER_CHARGE_FULL;
            player.state.vitals.current_mp = player.state.vitals.maximum_mp;
            runtime.inputs[0].buttons = kf::button_mask(kf::Button::Magic);
            runtime.controls[0] = player_update(*world, player, runtime.inputs[0]);
            party_advance_tasks(*world, runtime);
            require(player.cast_pose_ticks == avatar_cast_ticks, "Combat cast did not start its gesture");
            runtime.inputs[0] = {};
        }
        for (unsigned tick = 0; tick < 20; ++tick) {
            for (bool full : {false, true}) {
                if (!world_snapshot_encode(*world, {world->epoch, runtime.tick, full}, bytes)) {
                    std::fprintf(stderr, "Invalid retail world: floor %u tick %u full %u\n", floor, tick, full);
                    require(false, "Retail world failed snapshot encoding");
                }
                WorldSnapshotInfo info;
                auto restored = world_snapshot_decode(bytes, *world, info);
                require(restored && info.floor == world->floor && info.full == full,
                    "Retail world failed snapshot decoding");
                require(restored->party.members[0].player.cast_pose_ticks == player.cast_pose_ticks,
                    "Retail world lost cast gesture timing");
            }
            party_advance_tasks(*world, runtime);
            party_tick(*world, runtime);
            if (floor == 1)
                require(player.cast_pose_ticks == (tick < avatar_cast_ticks ? avatar_cast_ticks-tick-1 : 0),
                    "Cast gesture failed to expire after twelve world updates");
        }
        party_cancel_tasks(runtime);
        std::printf("Floor %u variant %u: loaded retail world and 20 updates passed full/fast snapshots\n", floor, variant);
        std::fflush(stdout);
    }
    retail_fixture_close();
    return 0;
}

// Prepare real interactions without navigating through the dungeon. All modes
// decode through the production snapshot boundary with retail assets.
enum class FixtureKind { Save, Travel, Wipe, Spell };
static int save_fixture(const char *resources, const char *input, const char *output, FixtureKind kind = FixtureKind::Save)
{
    auto *file = std::fopen(input, "rb");
    require(file, "Cannot open fixture snapshot");
    std::vector<u8> bytes(KF_NET_TRANSFER_LIMIT + 1);
    const auto size = std::fread(bytes.data(), 1, bytes.size(), file);
    require(!std::ferror(file), "Cannot read fixture snapshot");
    std::fclose(file);
    bytes.resize(size);
    WorldSnapshotInfo info;
    require(world_snapshot_info(bytes, info) && info.full, "Fixture needs a valid full snapshot");
    auto world = retail_fixture_open(resources);
    retail_fixture_load(*world, kf_enum_encode<unsigned>(info.floor), kf_enum_encode<unsigned>(info.variant));
    auto restored = world_snapshot_decode(bytes, *world, info);
    require(bool(restored), "Fixture snapshot failed full validation");
    world_apply_snapshot(*restored, *world);
    auto &host = world->party.members[0];
    auto &guest = world->party.members[1];
    if (output) {
        require(party_member_alive(host) && party_member_alive(guest), "Fixture requires two living players");
        if (kind == FixtureKind::Spell) {
            require(world->floor == KF_FLOOR_1, "Spell fixture needs the starting floor");
            for (const auto &actor : world->actors.actors)
                if (actor.slot_state != KF_ACTOR_SLOT_FREE && actor.lifecycle == KF_ACTOR_LIFECYCLE_ACTIVE)
                    collision_adjust_cell_occupancy(*world, actor.cell_x, actor.cell_z, -1);
            actor_pool_clear(*world);
            // Returning guests spawn at the canonical entry, facing +Z. Keep
            // the host in that same safe cell, just ahead of the spell muzzle.
            const auto entry = host.player.state.motion_state.map_cell;
            require(entry.x == 15 && entry.z == 2, "Spell host is outside the starting entry");
            host.player.state.camera_position.vz += 800;
            player_sync_position_to_map(*world, host.player);
            host.player.state.vitals.current_hp = 1;
            auto &caster=guest.player;
            caster.learned_magic[kf_enum_encode<u8>(KF_MAGIC_FIRE_BALL)] = KF_MAGIC_LEARNED;
            player_select_magic(*world,caster,KF_MAGIC_FIRE_BALL);
            caster.state.magic_charge = KF_PLAYER_CHARGE_FULL;
            caster.state.vitals.current_mp = caster.state.vitals.maximum_mp;
        } else if (kind == FixtureKind::Travel || kind == FixtureKind::Wipe) {
            retail_fixture_load(*world, 5, 1);
            if (kind != FixtureKind::Wipe) {
                // Admission/travel is a gate check, independent of nearby combat.
                for (const auto &actor : world->actors.actors)
                    if (actor.slot_state != KF_ACTOR_SLOT_FREE && actor.lifecycle == KF_ACTOR_LIFECYCLE_ACTIVE)
                        collision_adjust_cell_occupancy(*world, actor.cell_x, actor.cell_z, -1);
                actor_pool_clear(*world);
            }
            const auto entry = host.player.state.motion_state.map_cell;
            require(entry.x == 39 && entry.z == 69, "Unexpected floor-five entry");
            bool found = false;
            for (const auto [dx, dz] : std::array<std::pair<int, int>, 4>{{{0,1},{0,-1},{1,0},{-1,0}}}) {
                const auto x = entry.x + dx, z = entry.z + dz;
                if (!map_cell_has_full_floor(world->collision.cells[z][x]) ||
                    world->cell_attribute.cells[z][x] == KF_MAP_ATTRIBUTE_WARP ||
                    world->floor_height.cells[z][x] != world->floor_height.cells[entry.z][entry.x]) continue;
                host.player.state.camera_rotation = {0, static_cast<s16>(vector_xz_to_angle(dx, -dz)), 0};
                found = true;
                break;
            }
            require(found, "No level walking cell beside the real entrance");
            guest.player.state.progress_state.current_floor = world->floor;
            guest.player.state.progress_state.highest_floor = world->floor;
            guest.player.state.map_variant = world->variant;
            guest.player.state.camera_position = host.player.state.camera_position;
            player_sync_position_to_map(*world, guest.player);
            for (auto *member : {&host, &guest}) {
                player_clear_motion(member->player);
                member->player.state.previous_map_cell = entry;
            }
            require(party_at_same_entrance(*world), "Fixture is not at the real floor-five gate");
        } else {
            for (auto *member : {&host, &guest}) {
                const auto &cell = member->player.state.motion_state.map_cell;
                collision_adjust_cell_occupancy(*world, cell.x, cell.z, -1);
            }
            auto &player = host.player;
            bool found = false;
            for (unsigned index = 0; index < KF_MAP_OBJECT_CAPACITY && !found; ++index) {
                const auto &object = world->objects.objects[index];
                const auto id = kf_enum_encode<u8>(object.object_id);
                if (id >= KF_MAP_OBJECT_DEFINITION_COUNT ||
                    world->objects.definitions.entries[id].behavior_type != KF_MAP_OBJECT_OP_SAVE_POINT) continue;
                for (const auto [dx, dz] : std::array<std::pair<int, int>, 4>{{{0,1000},{0,-1000},{1000,0},{-1000,0}}}) {
                    const int x = object.position.vx + dx, z = object.position.vz + dz;
                    if (!map_position_within_grid(x, z) ||
                        !map_cell_has_full_floor(world->collision.cells[z/KF_MAP_TILE_SIZE][x/KF_MAP_TILE_SIZE])) continue;
                    player.state.camera_position.vx = x;
                    player.state.camera_position.vz = z;
                    player.state.camera_rotation = {0, static_cast<s16>(vector_xz_to_angle(-dx, dz)), 0};
                    if (player_interaction_event(*world, player) >= 0 ||
                        player_interaction_object(*world, player) != int(index)) continue;
                    player_sync_position_to_map(*world, player);
                    found = true;
                    break;
                }
            }
            require(found, "No directly reachable save-point interaction found");
            guest.presence = PartyPresence::Spectating;
            guest.player.state.vitals.current_hp = 0;
            player_clear_motion(guest.player);
            guest.player.cast_pose_ticks = 0;
            host.player.state.gold = 777;
            guest.player.state.gold = 333;
            guest.player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_GREEN_DRAGON_STAFF)] = 1;
        }
        require(world_snapshot_encode(*world, info, bytes), "Save fixture failed snapshot validation");
        file = std::fopen(output, "wb");
        require(file && std::fwrite(bytes.data(), 1, bytes.size(), file) == bytes.size(), "Cannot write save fixture");
        require(!std::fclose(file), "Cannot finish save fixture");
    }
    std::printf("FIXTURE {\"epoch\":%u,\"floor\":%u,\"variant\":%u,\"players\":[",
        world->epoch, kf_enum_encode<unsigned>(world->floor), kf_enum_encode<unsigned>(world->variant));
    unsigned printed = 0;
    for (unsigned slot = 0; slot < party_capacity; ++slot) {
        const auto &member = world->party.members[slot];
        if (member.presence == PartyPresence::Empty) continue;
        const auto &cell = member.player.state.motion_state.map_cell;
        std::printf("%s{\"presence\":%u,\"hp\":%u,\"gold\":%u,\"cell_x\":%u,\"cell_z\":%u,\"return_staff\":%u,\"mp\":%u,\"experience\":%d,\"magic_training\":%u,\"physical_training\":%u,\"identity\":\"", printed++ ? "," : "",
            unsigned(member.presence), member.player.state.vitals.current_hp, member.player.state.gold, cell.x, cell.z,
            member.player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_GREEN_DRAGON_STAFF)],
            member.player.state.vitals.current_mp, member.player.state.experience,
            member.player.state.magic_training, member.player.state.physical_power_training);
        for (const auto byte : member.character_id) std::printf("%02x", unsigned(byte));
        std::printf("\"}");
    }
    std::puts("]}");
    retail_fixture_close();
    return 0;
}

static int map_grid_check(const char *resources)
{
    require(kf::data_files_set_root(resources), "Cannot open local map resources");
    KfNetWorldLimits limits {};
    std::fill(std::begin(limits.asset_clips), std::end(limits.asset_clips), -1);
    for (unsigned floor = 1; floor <= 5; ++floor) {
        char path[32];
        std::snprintf(path, sizeof path, "KF/B%u/MIXA.DAT", floor);
        kf::DataFile file {};
        require(kf::data_file_open(&file, path) == kf::FileResult::Ok && file.size <= 32 * 1024 * 1024,
            "Cannot open map-grid source");
        std::vector<u8> bytes(file.size);
        require(kf::data_file_read(&file, bytes.data(), bytes.size()) == kf::FileResult::Ok,
            "Cannot read map-grid source");
        kf::data_file_close(&file);
        std::size_t offset = 0;
        const auto chunk = [&]() {
            require(offset <= bytes.size() && bytes.size() - offset >= 4, "Truncated map resource header");
            const auto *p = bytes.data() + offset;
            const auto size = u32(p[0]) | (u32(p[1]) << 8) | (u32(p[2]) << 16) | (u32(p[3]) << 24);
            offset += 4;
            require(size <= bytes.size() - offset, "Truncated map resource payload");
            const auto payload = std::span<const u8>(bytes).subspan(offset, size);
            offset += size;
            return payload;
        };
        chunk(); chunk(); // Sound-bank header and samples precede the five grids.
        const auto grids = chunk();
        require(grids.size() >= 5 * KF_WORLD_GRID_CELLS, "Missing map grids");
        auto world = std::make_unique<KfNetWorld>();
        world->header = {1, 1, static_cast<s32>(floor), 1, 0};
        world->header.variant = floor == 5 ? 1 : 0;
        for (auto &actor : world->actors) actor.slot_state = 255;
        for (auto &effect : world->effects) effect.slot_type = 255;
        for (auto &object : world->objects) { object.object_id = 255; object.generation = 1; }
        for (auto &event : world->events) event.state = 255;
        u8 *destinations[] = {world->cell_attribute, world->floor_height, world->cell_orientation,
            world->collision_flags, world->collision};
        for (unsigned i = 0; i < 5; ++i)
            std::memcpy(destinations[i], grids.data() + i * KF_WORLD_GRID_CELLS, KF_WORLD_GRID_CELLS);
        std::vector<u8> wire(KF_NET_TRANSFER_LIMIT);
        auto restored = std::make_unique<KfNetWorld>();
        for (u8 full : {0, 1}) {
            world->header.full = full;
            std::size_t written = 0;
            require(kf_net_world_encode(world.get(), &limits, wire.data(), wire.size(), &written) == KF_CODEC_OK,
                "Retail map grids failed snapshot encoding");
            require(kf_net_world_decode(wire.data(), written, &limits, restored.get()) == KF_CODEC_OK,
                "Retail map grids failed snapshot decoding");
            const u8 *decoded[] = {restored->cell_attribute, restored->floor_height, restored->cell_orientation,
                restored->collision_flags, restored->collision};
            for (unsigned i = 0; i < 5; ++i)
                require(!std::memcmp(destinations[i], decoded[i], KF_WORLD_GRID_CELLS), "Map-grid snapshot changed a cell");
        }
        std::printf("Floor %u: all five retail grids passed full and fast snapshot validation\n", floor);
    }
    return 0;
}

static int notice_check(const char *image_path)
{
    require(kf::host_start(), "Cannot start the isolated notice renderer");
    kf::host_set_session_running(true);
    auto *renderer = kf::host_renderer();
    kf::FaceList background {};
    background.style.blue = 0.25f;
    require(kf::renderer_draw_faces(renderer, &background, 320, 240), "Cannot draw notice background");
    const auto capture = [&] {
        kf::renderer_present_retained(renderer, 320, 240);
        std::vector<u8> pixels(320 * 240 * 4);
        glReadPixels(0, 0, 320, 240, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        require(glGetError() == GL_NO_ERROR, "Notice presentation produced a GL error");
        return pixels;
    };
    const auto before = capture();
    kf::host_notice("ONLY THE HOST CAN SAVE\nASK THEM TO USE THIS SAVE POINT");
    const auto during = capture();
    require(before != during && renderer->notice_texture, "Notice was not visible");
    require(capture() == during, "Retained presentation changed the notice");
    auto *file = std::fopen(image_path, "wb");
    require(file, "Cannot write notice preview");
    std::fprintf(file, "P6\n320 240\n255\n");
    for (int y = 239; y >= 0; --y) for (int x = 0; x < 320; ++x)
        require(std::fwrite(&during[(y * 320 + x) * 4], 1, 3, file) == 3, "Cannot write notice pixels");
    require(!std::fclose(file), "Cannot finish notice preview");
    kf::host_wait_until_tick(kf::host_clock_tick() + 301);
    require(!renderer->notice_texture && capture() == before,
        "Expired notice left pixels in the retained game frame");
    kf::host_notice("ONLY THE HOST CAN SAVE");
    kf::host_set_session_running(false);
    require(!renderer->notice_texture && capture() == before, "Notice survived session exit");
    kf::host_shutdown();
    std::puts("Notice presentation, expiration and session cleanup passed");
    return 0;
}

static int snapshot_summary(const char *path)
{
    auto *file = std::fopen(path,"rb");
    require(file,"Cannot open captured snapshot");
    std::vector<u8> bytes(KF_NET_TRANSFER_LIMIT+1);
    const auto size = std::fread(bytes.data(),1,bytes.size(),file);
    const bool good = !std::ferror(file);
    std::fclose(file);
    KfNetWorldSummary summary {};
    require(good && kf_net_world_summary(bytes.data(),size,&summary)==KF_CODEC_OK,
        "Captured snapshot failed metadata validation");
    std::printf("{\"hp\":%u,\"maximum_hp\":%u,\"mp\":%u,\"experience\":%u,\"floor\":%u}\n",
        summary.hp,summary.maximum_hp,summary.mp,summary.experience,summary.floor);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 6 && !std::strcmp(argv[1], "--door-edge")) {
        door_edge_fixture(kf_enum_decode<KfMapObjectOperation>(std::atoi(argv[2])),
            std::atoi(argv[3]), std::atoi(argv[4]), std::atoi(argv[5]));
        return 0;
    }
    if (argc == 4 && !std::strcmp(argv[1], "--invalid-map-effect")) {
        map_effect_fixture(kf_enum_decode<KfObjectId>(std::atoi(argv[2])),
            !std::strcmp(argv[3], "exhausted"), !std::strcmp(argv[3], "outside"));
        return 0;
    }
    if (argc == 3 && !std::strcmp(argv[1], "--party-travel")) return party_travel_check(argv[2]);
    if (argc == 5 && !std::strcmp(argv[1], "--save-fixture")) return save_fixture(argv[2], argv[3], argv[4]);
    if (argc == 5 && !std::strcmp(argv[1], "--travel-fixture")) return save_fixture(argv[2], argv[3], argv[4], FixtureKind::Travel);
    if (argc == 5 && !std::strcmp(argv[1], "--wipe-fixture")) return save_fixture(argv[2], argv[3], argv[4], FixtureKind::Wipe);
    if (argc == 5 && !std::strcmp(argv[1], "--spell-fixture")) return save_fixture(argv[2], argv[3], argv[4], FixtureKind::Spell);
    if (argc == 4 && !std::strcmp(argv[1], "--inspect-world")) return save_fixture(argv[2], argv[3], nullptr);
    if (argc == 3 && !std::strcmp(argv[1], "--snapshot-summary")) return snapshot_summary(argv[2]);
    if (argc == 3 && !std::strcmp(argv[1], "--world-states")) return world_state_check(argv[2]);
    if (argc == 3 && !std::strcmp(argv[1], "--map-grids")) return map_grid_check(argv[2]);
    if (argc == 3 && !std::strcmp(argv[1], "--resource-hash")) {
        if (!kf::data_files_set_root(argv[2])) return 1;
        const auto hash = kf::data_files_hash();
        if (hash.empty()) return 1;
        std::puts(hash.c_str());
        return 0;
    }
    if (argc == 3 && !std::strcmp(argv[1], "--notice-check")) return notice_check(argv[2]);
    if (argc == 6 && !std::strcmp(argv[1],"--ending-host"))
        return ending_fixture_host(argv[2],argv[3],argv[4],argv[5]);
    if (argc == 3) { retail_character_import(argv[1],argv[2]); return 0; }
    require(argc == 1,"Expected either no arguments, or KFIII disc and reference character pack");
    // First gameplay draws in this fresh process must retain the solo sequence.
    {
        auto solo = std::make_unique<WorldState>();
        kf::RandomStream expected;
        require(world_random_next(*solo) == kf::random_next(expected) &&
            player_random_next(solo->party.members[0].player) == kf::random_next(expected) &&
            kf::random_next() == kf::random_next(expected), "Solo random-call order changed");
    }
    task_lifecycle();
    resource_identity();
    persistent_online_profile();
    independent_characters_and_worlds();
    character_pack_and_selection();
    avatar_animation_and_attachments();
    hunched_avatar_animation();
    raised_arm_avatar_animation();
    clasped_avatar_animation();
    vested_avatar_animation();
    elf_avatar_animation();
    armored_avatar_animation();
    authoritative_menu_actions();
    authoritative_object_activation();
    personal_loot();
    authoritative_world_items();
    shared_quest_rewards();
    shared_story_scenes();
    shared_floor1_triggers();
    party_entry_return();
    party_terminal_travel();
    party_lifecycle();
    party_reconnect_lifecycle();
    party_combat();
    party_spell_combat();
    enemy_target_and_random_state();
    prediction_budget_and_view();
    entity_presentation_corrections();
    effect_presentation_history();
    actor_attachment_bounds();
    for (auto operation : {KF_MAP_OBJECT_OP_HINGED_DOOR, KF_MAP_OBJECT_OP_LIFT_DOOR, KF_MAP_OBJECT_OP_03})
        for (u16 yaw : {0, 1024, 2048, 3072}) door_edge_fixture(operation, 50, 50, yaw);
    for (auto id : {KF_MAP_OBJECT_ORBITING_PROJECTILE, KF_MAP_OBJECT_SHORT_SWING,
            KF_MAP_OBJECT_LONG_SWING, KF_MAP_OBJECT_EFFECT_SWITCH})
        map_effect_fixture(id, false, false);
    spell_world_bounds();
    spell_snapshot_transitions();
    world_and_player_random_isolation();
    authoritative_audio();
    snapshot_validation_and_prediction_isolation();
}
