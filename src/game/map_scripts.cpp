#include <kf/game/system.h>
#include <kf/game/world.h>
#include <kf/game/player.h>
#include <kf/game/audio.h>
#include <kf/lib/random.hpp>
#include <kf/lib/null.h>
#include <kf/lib/bool.h>
#include <kf/game/graphics.h>

#include <kf/game/actor.h>
#include <kf/game/map_data.h>
#include <kf/game/map.h>
#include <kf/game/collision.h>
#include <kf/game/notify.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <kf/game/game.h>
#include <kf/game/player_actions.h>

namespace {
constexpr unsigned floor2_harp_ambient_stage = 2, floor2_harp_ambient_page_end = 3;
constexpr unsigned floor2_reveal_stage = 2, floor3_fire_ball_stage = 3, floor5_weapon_transform_stage = 5;
struct ExchangeDialogue {
    unsigned event_slot, stage, page_end, last_page_slot, last_page, response_page, next_page, stage_limit;
};
constexpr ExchangeDialogue key_exchange = {2, 1, 3, 0, 7, 3, 4, 5};
constexpr ExchangeDialogue healing_exchange = {2, 2, 2, 1, 7, 2, 3, 5};
constexpr ExchangeDialogue harp_exchange = {1, 2, 2, 1, 5, 2, 3, 2};
struct ScriptCellRegion { int x_min, z_min, x_end, z_end; };
struct ScriptPointXZ { int x, z; };
constexpr ScriptCellRegion floor1_actor_entry = {7, 31, 12, 41};
constexpr ScriptCellRegion floor1_actor_exit = {2, 25, 14, 46};
constexpr ScriptCellRegion floor1_removal_entry = {2, 27, 5, 30};
constexpr ScriptCellRegion floor1_removal_exit = {2, 11, 28, 41};
constexpr ScriptPointXZ floor1_removed_object = {9000, 57000};
constexpr ScriptPointXZ floor1_cross = {21000, 67000};
constexpr int floor3_healing_x_min = 15, floor3_healing_x_end = 18, floor3_healing_z = 64;
constexpr int floor5_boss_trigger_x_min = 38, floor5_boss_trigger_x_end = 41, floor5_boss_trigger_z = 7;
constexpr ScriptPointXZ weapon_transform_effect_cell = {85, 40};
}

enum class KfMapWeaponTransformPhase : s32 {
    MAP_WEAPON_TRANSFORM_SPIN_UP = 0,
    MAP_WEAPON_TRANSFORM_SPIN_DOWN = 1
}; using enum KfMapWeaponTransformPhase;

enum class KfMapImageGroup : s32 {
    MAP_IMAGE_GROUP_SIGNBOARD = 0,
    MAP_IMAGE_GROUP_INSCRIPTION = 1
}; using enum KfMapImageGroup;

enum {
    MAP_WEAPON_TRANSFORM_HOLD_UPDATES = 40,
    MAP_WEAPON_TRANSFORM_SWAP_COUNTDOWN = 20,
    MAP_SHOP_SEQUENCE_INDEX = 2,
    MAP_FLOOR3_DIALOGUE_DOOR_LINK = 0x37
};

enum {
    MAP_FLOOR1_AMBIENT_VOLUME = 115,
    MAP_SCRIPT_OBJECT_SEARCH_PADDING = 3000,
    MAP_FLOOR2_AMBIENT_RANDOM_LIMIT = 4000,
    MAP_BOSS_REVEAL_YAW_MIN = 1808,
    MAP_BOSS_REVEAL_YAW_END = 2289,
    MAP_PASSAGE_OPEN_SOUND_VOLUME = 100,
    MAP_TRANSFER_FADE_IN_STEP = 128,
    MAP_TRANSFER_LIGHT_BLEND_SHIFT = 2,
    MAP_TRANSFER_RISE_STEP = 130,
    MAP_TRANSFER_YAW_STEP = 128,
    MAP_TRANSFER_FADE_OUT_STEP = 256,
    MAP_WEAPON_TRANSFORM_HEIGHT = 1300,
    MAP_WEAPON_TRANSFORM_BLAST_HEIGHT = 600,
    MAP_WEAPON_TRANSFORM_MAX_YAW_STEP = 240,
    MAP_WEAPON_TRANSFORM_YAW_ACCELERATION = 1,
    MAP_ATTRIBUTE_PROBE_DISTANCE = 1500,
    MAP_CONTAINER_CAMERA_PITCH_SPAN = KF_ANGLE_HALF_TURN - KF_PLAYER_CAMERA_PITCH_LIMIT + 1,
    MAP_CONTAINER_CAMERA_PITCH_STEP = 16,
    MAP_CONTAINER_OPEN_PITCH_STEP = 32,
    MAP_DOOR_PARTNER_SEARCH_PADDING = 6000
};

static constexpr bool map_dialogue_just_started(const KfDialogueState &dialogue, unsigned stage)
{
    return dialogue.stage == stage && dialogue.page == KF_DIALOGUE_FIRST_PAGE
        && dialogue.page_delay == KF_DIALOGUE_PAGE_DELAY_TICKS;
}

static constexpr bool script_region_contains_cell(const ScriptCellRegion &region, KfMapCellCoordinates cell)
{
    return cell.x >= region.x_min && cell.z >= region.z_min
        && cell.x < region.x_end && cell.z < region.z_end;
}

static KfCameraPathPoint map_floor5_camera_path[2] = {
    {{173000, -11500, 85000, 0}, {0, KF_ANGLE_HALF_TURN, 0, 0}, 100, 0},
    {{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0}
};

static VECTOR map_floor1_sound_position = {65000, -10000, 25000, 0};

static MATRIX map_transfer_light_matrix = {
    {{0, -KF_FIXED12_ONE, 0}, {0, -KF_FIXED12_ONE, 0}, {0, -KF_FIXED12_ONE, 0}}, {0, 0, 0}
};

static constexpr unsigned map_screen_path_capacity = 16;
static constexpr unsigned map_screen_floor_offset = 5, map_screen_group_offset = 8, map_screen_number_offset = 9;
static char map_screen_image_path[map_screen_path_capacity] = "KAN/B0/K000.TIM";

s32 actor_pool_find_at_tile(WorldState &world, u8 tile_x, u8 tile_z)
{
    KfActor *actor = world.actors.actors;
    s16 index;

    for (index = 0; index < KF_ACTOR_CAPACITY; index++, actor++) {
        if (actor->slot_state != KF_ACTOR_SLOT_FREE && actor->tile_x == tile_x
            && actor->tile_z == tile_z) {
            return index;
        }
    }
    return -1;
}

void map_ambient_script_floor1(WorldState &world, PlayerContext &player)
{
    if (world.party.enabled && world.prediction) return;
    const auto occupied = [&](const ScriptCellRegion &region) {
        if (!world.party.enabled) return script_region_contains_cell(region, player.state.motion_state.map_cell);
        for (const auto &member : world.party.members)
            if (party_member_alive(member) && script_region_contains_cell(region, member.player.state.motion_state.map_cell))
                return true;
        return false;
    };
    if (map_floor_script(world, KF_FLOOR_1).floor1.revival_enabled == KF_MAP_SCRIPT_SET) {
        audio_play_spatial_default_range(player,
            &gameplay_sound_refs[KF_GAMEPLAY_SOUND_FLOOR1_REVIVAL], &map_floor1_sound_position, MAP_FLOOR1_AMBIENT_VOLUME);
    }

    switch (map_floor_script(world, KF_FLOOR_1).floor1.actor_activation_stage) {
    case KF_MAP_TRIGGER_COMPLETE:
        break;
    case KF_MAP_TRIGGER_AWAIT_ENTRY:
        if (occupied(floor1_actor_entry)) {
            map_floor_script(world, KF_FLOOR_1).floor1.actor_activation_stage = KF_MAP_TRIGGER_AWAIT_EXIT;
        }
        break;
    case KF_MAP_TRIGGER_AWAIT_EXIT:
        if (!occupied(floor1_actor_exit)) {
            s32 actor_index;
            s32 object_index;

            map_floor_script(world, KF_FLOOR_1).floor1.actor_activation_stage = KF_MAP_TRIGGER_COMPLETE;
            actor_index = actor_pool_find_at_tile(world, KF_FLOOR1_TRIGGER_ACTOR_TILE_X, KF_FLOOR1_TRIGGER_ACTOR_TILE_Z);
            if (actor_index != -1) {
                world.actors.actors[actor_index].lifecycle = KF_ACTOR_LIFECYCLE_DORMANT;
                actor_initialize_slot(world, actor_index);
            }
            object_index = map_floor1_cross_index(world);
            if (object_index != -1) {
                world.objects.objects[object_index].object_id = KF_MAP_OBJECT_BROKEN_STONE_CROSS;
            }
        }
        break;
    }

    switch (map_floor_script(world, KF_FLOOR_1).floor1.object_removal_stage) {
    case KF_MAP_TRIGGER_COMPLETE:
        break;
    case KF_MAP_TRIGGER_AWAIT_ENTRY:
        if (occupied(floor1_removal_entry)) {
            map_floor_script(world, KF_FLOOR_1).floor1.object_removal_stage = KF_MAP_TRIGGER_AWAIT_EXIT;
        }
        break;
    case KF_MAP_TRIGGER_AWAIT_EXIT:
        if (!occupied(floor1_removal_exit)) {
            s32 object_index;

            map_floor_script(world, KF_FLOOR_1).floor1.object_removal_stage = KF_MAP_TRIGGER_COMPLETE;
            object_index = map_object_pool_find_near_point(world, floor1_removed_object.x, floor1_removed_object.z, MAP_SCRIPT_OBJECT_SEARCH_PADDING);
            if (object_index != -1) {
                world.objects.objects[object_index].object_id = KF_OBJECT_NONE;
            }
        }
        break;
    }
}

s32 map_floor1_cross_index(WorldState &world)
{
    return map_object_pool_find_near_point(world, floor1_cross.x, floor1_cross.z, MAP_SCRIPT_OBJECT_SEARCH_PADDING);
}

void map_ambient_script_floor2(WorldState &world, PlayerContext &player)
{
    // Ambient sound timing must not consume online simulation randomness.
    if (world.map.events[KF_FLOOR2_HARP_EVENT].dialogue.stage == floor2_harp_ambient_stage && world.map.events[KF_FLOOR2_HARP_EVENT].dialogue.page < floor2_harp_ambient_page_end
        && kf::random_next() < MAP_FLOOR2_AMBIENT_RANDOM_LIMIT) {
        audio_play_spatial_default_range(player,
            &gameplay_sound_refs[KF_GAMEPLAY_SOUND_HARP], &world.map.events[KF_FLOOR2_HARP_EVENT].reference_position, KF_AUDIO_MAX_VOLUME);
    }
}

kf::FrameTask<void> map_ambient_script_floor3(WorldState &world, PlayerContext &player)
{
    if (world.party.enabled) {
        if (world.prediction) co_return;
        for (auto &member : world.party.members) {
            const auto &cell = member.player.state.motion_state.map_cell;
            if (!member.connected || !party_member_alive(member) || cell.x < floor3_healing_x_min ||
                cell.x >= floor3_healing_x_end || cell.z != floor3_healing_z) continue;
            co_await player_restore_vitals_with_color_cycle(world, member.player);
            party_complete_quest(world, PartyReward::SanctuaryMagic);
        }
        co_return;
    }
    const auto &cell = player.state.motion_state.map_cell;
    if (cell.x >= floor3_healing_x_min && cell.x < floor3_healing_x_end
        && cell.z == floor3_healing_z) {
        (co_await player_restore_vitals_with_color_cycle(world, player));
        if (player.learned_magic[kf_enum_encode<u8>(KF_MAGIC_RESIST_FIRE)] == KF_MAGIC_UNLEARNED || player.learned_magic[kf_enum_encode<u8>(KF_MAGIC_BLESS)] == KF_MAGIC_UNLEARNED) {
            player.learned_magic[kf_enum_encode<u8>(KF_MAGIC_RESIST_FIRE)] = KF_MAGIC_LEARNED;
            player.learned_magic[kf_enum_encode<u8>(KF_MAGIC_BLESS)] = KF_MAGIC_LEARNED;
            notify_enqueue(KF_NOTIFICATION_MAGIC_LEARNED);
        }
    }
}

void map_ambient_script_floor4(void)
{
}

static bool map_boss_trigger(const PlayerContext &player)
{
    const auto &cell = player.state.motion_state.map_cell;
    return cell.x >= floor5_boss_trigger_x_min && cell.x < floor5_boss_trigger_x_end && cell.z == floor5_boss_trigger_z
        && (u16)player.state.camera_rotation.vy >= MAP_BOSS_REVEAL_YAW_MIN
        && (u16)player.state.camera_rotation.vy < MAP_BOSS_REVEAL_YAW_END;
}

static void map_boss_activate(WorldState &world)
{
    auto &animations = world.actors.definitions.entries[KF_FLOOR5_BOSS_DEFINITION].action_animations;
    animations[KF_ACTOR_ANIM_SLOT_MELEE] = KF_ANIMATION_CLIP_THIRD;
    animations[KF_ACTOR_ANIM_SLOT_EFFECT0] = KF_ANIMATION_CLIP_FOURTH;
    animations[KF_ACTOR_ANIM_SLOT_EFFECT1] = KF_ANIMATION_CLIP_FOURTH;
    animations[KF_ACTOR_ANIM_SLOT_EFFECT2] = KF_ANIMATION_CLIP_FOURTH;
    animations[KF_ACTOR_ANIM_SLOT_MULTI_HIT_ATTACK] = KF_ANIMATION_CLIP_SECOND;
    map_apply_copy_region(world, KF_MAP_COPY_FLOOR5_BOSS_ENCOUNTER);
}

kf::FrameTask<void> map_ambient_script_floor5(WorldState &world, PlayerContext &player)
{
    KfMapScriptFlag *encounter_started = &map_floor_script(world, KF_FLOOR_5).floor5.boss_encounter_started;
    if (world.party.enabled) {
        if (world.prediction || world.story.kind || *encounter_started != KF_MAP_SCRIPT_UNSET) co_return;
        for (auto &member : world.party.members) {
            if (!member.connected || !party_member_alive(member) || !map_boss_trigger(member.player)) continue;
            const auto &p = member.player;
            world.story = {};
            world.story.kind = party_story_boss;
            world.story.initiator = p.party_slot;
            world.story.effect = 255;
            world.story.page = 1;
            world.story.camera_x = p.state.camera_position.vx;
            world.story.camera_y = p.state.camera_position.vy;
            world.story.camera_z = p.state.camera_position.vz;
            *encounter_started = KF_MAP_SCRIPT_SET;
            break;
        }
        co_return;
    }
    if (*encounter_started == KF_MAP_SCRIPT_UNSET && map_boss_trigger(player)) {
        *encounter_started = KF_MAP_SCRIPT_SET;
        (co_await screen_show_image_until_input("TALK/C17/T55171.TIM"));
        (co_await render_frame(world, player, NULL, NULL));
        (co_await render_frame(world, player, NULL, NULL));
        (co_await screen_show_image_until_input("TALK/C17/T55172.TIM"));
        map_boss_activate(world);
    }
}

void map_action_script_floor1(WorldState &world, PlayerContext &player)
{
    if (player.item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(KF_ITEM_DRAGON_CHALICE)] != 0
        && map_floor_script(world, KF_FLOOR_1).floor1.passage_opened == KF_MAP_SCRIPT_UNSET) {
        map_floor_script(world, KF_FLOOR_1).floor1.passage_opened = KF_MAP_SCRIPT_SET;
        map_apply_copy_region(world, KF_MAP_COPY_FLOOR1_PASSAGE);
        sound_ref_play(audio_playback(player), &gameplay_sound_refs[KF_GAMEPLAY_SOUND_STONE_PASSAGE], MAP_PASSAGE_OPEN_SOUND_VOLUME);
    }
}

kf::FrameTask<void> map_floor2_event_transfer_fade(WorldState &world, PlayerContext &player)
{
    kf::InputContextScope input_context(kf::InputContext::Scripted);
    MATRIX saved;
    s32 blend;

    saved = game_graphics_runtime.map_event_light_matrix;

    for (blend = 0; blend < KF_FIXED12_ONE + 1; blend += MAP_TRANSFER_FADE_IN_STEP) {
        lighting_set_color_matrix(game_graphics_runtime.render_state, &color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_DEFAULT)], &color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_WHITE)], blend);
        if (blend >= KF_FIXED12_ONE / 4 + 1) {
            world.map.events[KF_FLOOR2_REVEAL_EVENT].reference_position.vy -= MAP_TRANSFER_RISE_STEP;
            world.map.events[KF_FLOOR2_REVEAL_EVENT].rotation.vy += MAP_TRANSFER_YAW_STEP;
        } else {
            matrix_interpolate(&saved, &map_transfer_light_matrix,
                               &game_graphics_runtime.map_event_light_matrix, blend << MAP_TRANSFER_LIGHT_BLEND_SHIFT);
        }
        (co_await render_frame(world, player, NULL, NULL));
    }

    world.map.events[KF_FLOOR2_REVEAL_EVENT].state = KF_MAP_EVENT_DISABLED;
    map_floor_script(world, KF_FLOOR_5).floor5.character_arrived = KF_MAP_SCRIPT_SET;

    for (blend = KF_FIXED12_ONE; blend >= 0; blend -= MAP_TRANSFER_FADE_OUT_STEP) {
        lighting_set_color_matrix(game_graphics_runtime.render_state, &color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_DEFAULT)], &color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_WHITE)], blend);
        (co_await render_frame(world, player, NULL, NULL));
    }

    lighting_set_active_color_matrix(KF_GAME_COLOR_DEFAULT);
    game_graphics_runtime.map_event_light_matrix = saved;

}

kf::FrameTask<void> map_action_script_floor2(WorldState &world, PlayerContext &player)
{
    if (world.party.enabled) co_return; // The authenticated dialogue completion starts the shared scene.
    if (map_dialogue_just_started(world.map.events[KF_FLOOR2_REVEAL_EVENT].dialogue, floor2_reveal_stage)
        && world.map.events[KF_FLOOR2_REVEAL_EVENT].state == KF_MAP_EVENT_ACTIVE) {
        (co_await map_floor2_event_transfer_fade(world, player));
    }
}

void map_action_script_floor3(WorldState &world, PlayerContext &player)
{
    if (player.item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(KF_ITEM_WIND_BLADE_BRACELET)] != 0) {
        if (player.learned_magic[kf_enum_encode<u8>(KF_MAGIC_WIND_CUTTER)] == KF_MAGIC_UNLEARNED) {
            player.learned_magic[kf_enum_encode<u8>(KF_MAGIC_WIND_CUTTER)] = KF_MAGIC_LEARNED;
            if (player.local_view) notify_enqueue(KF_NOTIFICATION_MAGIC_LEARNED);
        }
    }
    if (map_dialogue_just_started(world.map.events[KF_FLOOR3_FIRE_BALL_EVENT].dialogue, floor3_fire_ball_stage)) {
        if (world.party.enabled) {
            party_complete_quest(world, PartyReward::FireBall);
        } else if (player.learned_magic[kf_enum_encode<u8>(KF_MAGIC_FIRE_BALL)] == KF_MAGIC_UNLEARNED) {
            player.learned_magic[kf_enum_encode<u8>(KF_MAGIC_FIRE_BALL)] = KF_MAGIC_LEARNED;
            notify_enqueue(KF_NOTIFICATION_MAGIC_LEARNED);
        }
    }
}

void map_action_script_floor4(void)
{
}

kf::FrameTask<void> map_floor5_weapon_transform_cutscene(WorldState &world, PlayerContext &player)
{
    kf::InputContextScope input_context(kf::InputContext::Scripted);
    MATRIX color_matrix;
    KfCameraPathState path;
    KfMapObject *sword;
    SVECTOR direction {};
    VECTOR spawn;
    s32 grid_height;
    s32 spin;
    s32 hold;
    KfMapWeaponTransformPhase phase;

    if (player.state.equipped_weapon_id == KF_ITEM_DRAGON_SWORD) {
        player_equip_weapon(player, KF_OBJECT_NONE);
    }
    player.item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<u8>(KF_ITEM_DRAGON_SWORD)] = 0;
    collision_adjust_cell_occupancy(world, player.state.motion_state.map_cell.x, player.state.motion_state.map_cell.z, -1);

    camera_path_begin(player, &path, map_floor5_camera_path);
    for (;;) {
        camera_path_step(&path, 0);
        if (path.frames_remaining == KF_CAMERA_PATH_FINISHED) {
            break;
        }
        (co_await render_frame(world, player, &path.position, &path.rotation));
    }

    player.state.camera_position = path.position;
    player.state.camera_rotation = path.rotation;
    player.state.motion_state.map_cell.x = player.state.camera_position.vx / KF_MAP_TILE_SIZE;
    player.state.motion_state.map_cell.z = player.state.camera_position.vz / KF_MAP_TILE_SIZE;
    collision_adjust_cell_occupancy(world, player.state.motion_state.map_cell.x, player.state.motion_state.map_cell.z, 1);

    color_matrix = game_graphics_runtime.render_state.lighting.color_matrix;
    sword = map_object_effect_pool_acquire(world,
        KF_MAP_OBJECT_PLACEMENT_DROP_FIRST, KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY, world.objects.placement_drop_sequence);
    sword->object_id = KF_ITEM_DRAGON_SWORD;
    sword->cell_x = weapon_transform_effect_cell.x;
    sword->cell_z = weapon_transform_effect_cell.z;
    sword->position.vx = sword->cell_x * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    sword->position.vz = sword->cell_z * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    grid_height = world.floor_height.cells[sword->cell_z][sword->cell_x];
    sword->rotation.angles.z = 0;
    sword->rotation.angles.x = 0;
    sword->rotation.angles.y = KF_ANGLE_HALF_TURN;
    sword->action = KF_MAP_OBJECT_OP_NONE;
    sword->position.vy = -(grid_height * KF_MAP_HEIGHT_STEP) - MAP_WEAPON_TRANSFORM_HEIGHT;

    spin = 0;
    hold = 0;
    phase = MAP_WEAPON_TRANSFORM_SPIN_UP;
    for (;;) {
        switch (phase) {
        case MAP_WEAPON_TRANSFORM_SPIN_UP:
            sword->rotation.angles.y += spin;
            if (hold != 0) {
                hold -= 1;
                if (hold == 1) {
                    phase = MAP_WEAPON_TRANSFORM_SPIN_DOWN;
                } else if (hold == MAP_WEAPON_TRANSFORM_SWAP_COUNTDOWN) {
                    spawn = sword->position;
                    spawn.vy -= MAP_WEAPON_TRANSFORM_BLAST_HEIGHT;
                    effect_pool_construct(world, player,
                        0, KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,
                        KF_EFFECT_KIND_RADIAL_BLAST, &spawn, &direction, KfEffectSoundArguments{KF_EFFECT_SOUND_PLAY});
                    sword->object_id = KF_ITEM_MOONLIGHT_SWORD;
                }
            } else if (spin < MAP_WEAPON_TRANSFORM_MAX_YAW_STEP) {
                spin += MAP_WEAPON_TRANSFORM_YAW_ACCELERATION;
            } else {
                hold = MAP_WEAPON_TRANSFORM_HOLD_UPDATES;
            }
            break;
        case MAP_WEAPON_TRANSFORM_SPIN_DOWN:
            sword->rotation.angles.y += spin;
            if (spin > 0) {
                spin -= MAP_WEAPON_TRANSFORM_YAW_ACCELERATION;
            } else {
                map_object_start_action_if_idle(sword, KF_MAP_OBJECT_OP_FALL_AND_TIP);
                sword->link.fields.vertical_velocity = 0;

                co_return;
            }
            break;
        default:
            break;
        }
        effect_pool_update(world, player);
        (co_await render_frame(world, player, NULL, NULL));
    }
}

kf::FrameTask<void> map_action_script_floor5(WorldState &world, PlayerContext &player)
{
    if (world.party.enabled) co_return;
    if (map_dialogue_just_started(world.map.events[KF_FLOOR5_WEAPON_TRANSFORM_EVENT].dialogue, floor5_weapon_transform_stage)) {
        (co_await map_floor5_weapon_transform_cutscene(world, player));
        map_floor_script(world, KF_FLOOR_5).floor5.weapon_transformed = KF_MAP_SCRIPT_SET;
    }
}

bool party_story_begin(WorldState &world, PlayerContext &player, u16 event)
{
    if (!world.party.enabled || world.prediction || world.story.kind || player.party_slot >= party_capacity ||
        &world.party.members[player.party_slot].player != &player ||
        !world.party.members[player.party_slot].connected || !party_member_alive(world.party.members[player.party_slot])) return false;
    const bool transfer = world.floor == KF_FLOOR_2 && event == KF_FLOOR2_REVEAL_EVENT;
    const bool weapon = world.floor == KF_FLOOR_5 && event == KF_FLOOR5_WEAPON_TRANSFORM_EVENT;
    if (!transfer && !weapon) return false;
    const auto &npc = world.map.events[event];
    if (npc.state != KF_MAP_EVENT_ACTIVE || !map_dialogue_just_started(npc.dialogue,
        transfer ? floor2_reveal_stage : floor5_weapon_transform_stage)) return false;
    if (transfer && map_floor_script(world, KF_FLOOR_5).floor5.character_arrived == KF_MAP_SCRIPT_SET) return false;
    if (weapon && (map_floor_script(world, KF_FLOOR_5).floor5.weapon_transformed == KF_MAP_SCRIPT_SET ||
        !player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_DRAGON_SWORD)])) return false;
    auto &story = world.story;
    story = {};
    story.kind = transfer ? party_story_transfer : party_story_weapon;
    story.initiator = player.party_slot;
    story.effect = 255;
    story.camera_x = player.state.camera_position.vx;
    story.camera_y = player.state.camera_position.vy;
    story.camera_z = player.state.camera_position.vz;
    story.pitch = player.state.camera_rotation.vx;
    story.yaw = player.state.camera_rotation.vy;
    story.roll = player.state.camera_rotation.vz;
    if (weapon) {
        auto *sword = map_object_effect_pool_acquire(world, KF_MAP_OBJECT_PLACEMENT_DROP_FIRST,
            KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY, world.objects.placement_drop_sequence);
        story.object = sword - world.objects.objects;
        story.generation = sword->generation;
        sword->object_id = KF_OBJECT_NONE;
        sword->action = KF_MAP_OBJECT_OP_NONE;
        // Reserve before spending the initiating player's offering. Other
        // characters keep their inventory; the result uses personal loot claims.
        if (!--player.item_stock[0][kf_enum_encode<u8>(KF_ITEM_DRAGON_SWORD)] &&
            player.state.equipped_weapon_id == KF_ITEM_DRAGON_SWORD)
            player_equip_weapon(player, KF_OBJECT_NONE);
    }
    return true;
}

static void party_story_camera(const KfNetWorldStory &story, KfCameraPathState &path)
{
    path = {};
    path.points = map_floor5_camera_path;
    path.position = {story.camera_x, story.camera_y, story.camera_z};
    path.rotation = {story.pitch, story.yaw, story.roll};
    camera_path_publish_fixed(&path);
    camera_path_compute_segment(&path);
    for (unsigned frame = 0; frame < std::min<unsigned>(story.tick, 100); ++frame) camera_path_step(&path, 0);
}

bool party_ending_begin(WorldState &world)
{
    if (!world.party.enabled || world.prediction || world.story.kind || (world.floor != KF_FLOOR_1 && world.floor != KF_FLOOR_5) ||
        map_floor_script(world, KF_FLOOR_5).floor5.boss_defeat != KF_MAP_SCRIPT_SET) return false;
    world.story = {};
    world.story.kind = party_story_ending;
    world.story.effect = 255;
    world.story.ready_mask = 1; // Host owns the terminal state; peers acknowledge its full snapshot.
    return true;
}

bool party_ending_waiting(const WorldState &world)
{
    if (world.story.kind != party_story_ending) return false;
    for (u8 slot = 0; slot < party_capacity; ++slot) {
        const auto presence = world.party.members[slot].presence;
        if ((presence == PartyPresence::Living || presence == PartyPresence::Spectating) &&
            !(world.story.ready_mask & (1u << slot))) return true;
    }
    return false;
}

bool party_story_ready(WorldState &world, u8 slot, u16 page)
{
    if (!world.party.enabled || world.prediction || world.story.kind != party_story_boss ||
        page < 1 || page > 2 || world.story.page != page || slot >= party_capacity) return false;
    const auto &member = world.party.members[slot];
    if (!member.connected || (member.presence != PartyPresence::Living && member.presence != PartyPresence::Spectating) ||
        (world.story.ready_mask & (1u << slot))) return false;
    world.story.ready_mask |= 1u << slot;
    return true;
}

void party_story_tick(WorldState &world)
{
    auto &story = world.story;
    if (!world.party.enabled || world.prediction || !story.kind) return;
    if (story.kind == party_story_ending) return;
    if (story.kind == party_story_boss) {
        if (!story.page) {
            if (++story.tick == 2) { story.page = 2; story.tick = 0; }
            return;
        }
        u8 required = 0;
        for (u8 slot = 0; slot < party_capacity; ++slot) {
            const auto &member = world.party.members[slot];
            if (member.connected && (member.presence == PartyPresence::Living || member.presence == PartyPresence::Spectating))
                required |= 1u << slot;
        }
        if (!required || (story.ready_mask & required) != required) return;
        if (story.page == 1) { story.page = 0; story.ready_mask = 0; }
        else { map_boss_activate(world); story = {}; }
        return;
    }
    auto &player = world.party.members[story.initiator].player;
    const unsigned tick = story.tick++;
    if (story.kind == party_story_transfer) {
        auto &npc = world.map.events[KF_FLOOR2_REVEAL_EVENT];
        if (tick >= 9 && tick <= 32) {
            npc.reference_position.vy -= MAP_TRANSFER_RISE_STEP;
            npc.rotation.vy += MAP_TRANSFER_YAW_STEP;
        }
        if (tick == 33) {
            npc.state = KF_MAP_EVENT_DISABLED;
            map_floor_script(world, KF_FLOOR_5).floor5.character_arrived = KF_MAP_SCRIPT_SET;
        }
        if (tick >= 49) story = {};
        return;
    }
    auto &sword = world.objects.objects[story.object];
    if (sword.generation != story.generation) kf::host_fail("Story object was replaced during a shared scene");
    if (tick == 100) {
        KfCameraPathState camera;
        party_story_camera(story, camera);
        collision_adjust_cell_occupancy(world, player.state.motion_state.map_cell.x, player.state.motion_state.map_cell.z, -1);
        player.state.camera_position = camera.position;
        player.state.camera_rotation = camera.rotation;
        player_clear_motion(player);
        player_sync_position_to_map(world, player);
        player.state.previous_map_cell = player.state.motion_state.map_cell;
        sword.object_id = KF_ITEM_DRAGON_SWORD;
        sword.cell_x = weapon_transform_effect_cell.x;
        sword.cell_z = weapon_transform_effect_cell.z;
        sword.position = {sword.cell_x * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER,
            -(world.floor_height.cells[sword.cell_z][sword.cell_x] * KF_MAP_HEIGHT_STEP) - MAP_WEAPON_TRANSFORM_HEIGHT,
            sword.cell_z * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER};
        sword.rotation.angles = {};
        sword.rotation.angles.y = KF_ANGLE_HALF_TURN;
        sword.action = KF_MAP_OBJECT_OP_NONE;
    }
    if (tick < 100) return;
    const unsigned frame = tick - 100;
    const unsigned spin = frame <= 240 ? frame : frame <= 279 ? 240 : frame <= 519 ? 520-frame : 0;
    sword.rotation.angles.y += spin;
    if (frame == 260) {
        auto spawn = sword.position;
        spawn.vy -= MAP_WEAPON_TRANSFORM_BLAST_HEIGHT;
        SVECTOR direction {};
        auto *blast = effect_pool_construct(world, player, 0, KF_EFFECT_TYPE_NONE,
            KF_EFFECT_KIND_RADIAL_BLAST, &spawn, &direction, KfEffectSoundArguments{KF_EFFECT_SOUND_PLAY});
        if (blast) story.effect = blast - world.effects.records;
        sword.object_id = KF_ITEM_MOONLIGHT_SWORD;
    }
    // Only the scene's decorative blast advances while combat is paused.
    // Dispatching the normal radial effect would apply damage to the party.
    if (story.effect < KF_EFFECT_CAPACITY) {
        auto &blast = world.effects.records[story.effect];
        if (++blast.phase < KF_EFFECT_RADIAL_BLAST_PHASE_END) {
            blast.scale_x += KF_FIXED12_ONE / 4;
            blast.scale_y = blast.scale_z = blast.scale_x;
        } else {
            blast.type = KF_EFFECT_SLOT_FREE;
            story.effect = 255;
        }
    }
    if (frame >= 520) {
        map_object_start_action_if_idle(&sword, KF_MAP_OBJECT_OP_FALL_AND_TIP);
        sword.link.fields.vertical_velocity = 0;
        map_floor_script(world, KF_FLOOR_5).floor5.weapon_transformed = KF_MAP_SCRIPT_SET;
        story = {};
    }
}

void party_story_render(WorldState &world, PlayerContext &viewer)
{
    const auto &story = world.story;
    const auto saved_color = game_graphics_runtime.render_state.lighting.color_matrix;
    const auto saved_light = game_graphics_runtime.map_event_light_matrix;
    if (story.kind == party_story_transfer) {
        const auto blend = story.tick <= 33 ? std::min<unsigned>(4096, story.tick * 128)
            : (50 - std::min<unsigned>(50, story.tick)) * 256;
        lighting_set_color_matrix(game_graphics_runtime.render_state,
            &color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_DEFAULT)],
            &color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_WHITE)], blend);
        matrix_interpolate(&saved_light, &map_transfer_light_matrix, &game_graphics_runtime.map_event_light_matrix,
            std::min<unsigned>(4096, story.tick * 512));
        render_world_frame(world, viewer, &viewer.state.camera_position, &viewer.state.camera_rotation);
    } else if (story.kind == party_story_weapon) {
        KfCameraPathState camera;
        party_story_camera(story, camera);
        render_world_frame(world, world.party.members[story.initiator].player, &camera.position, &camera.rotation);
    } else {
        render_world_frame(world, viewer, &viewer.state.camera_position, &viewer.state.camera_rotation);
    }
    game_graphics_runtime.render_state.lighting.color_matrix = saved_color;
    game_graphics_runtime.map_event_light_matrix = saved_light;
}

static bool map_exchange_is_available(WorldState &world, PlayerContext &player, const ExchangeDialogue &exchange,
    KfObjectId offered_item, PartyReward reward)
{
    const auto &inventory = player.item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)];
    const auto &dialogue = world.map.events[exchange.event_slot].dialogue;
    return (!world.party.enabled || !(world.party.quest_rewards & static_cast<u32>(reward)))
        && inventory[kf_enum_encode<u8>(offered_item)] != 0
        && dialogue.stage == exchange.stage && dialogue.page < exchange.page_end;
}

static kf::FrameTask<void> map_dialogue_page(PlayerContext &player, const KfMapEvent &speaker, u8 page, DialoguePage *capture)
{
    if (capture) {
        *capture = {player.state.progress_state.current_floor, speaker.character_id, speaker.dialogue.stage, page};
        co_return;
    }
    co_await talk_show_dialogue_page(player.state.progress_state.current_floor,
        speaker.dialogue.stage, speaker.character_id, page);
}

static kf::FrameTask<void> map_finish_exchange_dialogue(WorldState &world, PlayerContext &player, KfMapEvent &speaker, const ExchangeDialogue &exchange, DialoguePage *capture)
{
    co_await map_dialogue_page(player, speaker, exchange.response_page, capture);
    auto &target = world.map.events[exchange.event_slot];
    target.dialogue.page = exchange.next_page;
    target.dialogue.page_delay = 0;
    target.dialogue.stage_limit = exchange.stage_limit;
    map_event_refresh_dialogue_stage(player, &target);
}

kf::FrameTask<void> map_event_interact(WorldState &world, PlayerContext &player, KfMapEvent *event, DialoguePage *capture)
{
    auto &inventory = player.item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)];

    switch (event->character_id) {
    case KF_CHARACTER_KEY_OF_THE_DEAD_EXCHANGE:
        if (map_exchange_is_available(world, player, key_exchange, KF_ITEM_GOLD_CROSS, PartyReward::KeyOfTheDead)) {
            if (world.party.enabled) party_complete_quest(world, PartyReward::KeyOfTheDead);
            else inventory[kf_enum_encode<u8>(KF_ITEM_KEY_OF_THE_DEAD)] = 1;
            world.map.events[key_exchange.event_slot].dialogue_pages
                .last_page[key_exchange.last_page_slot] = key_exchange.last_page;
            inventory[kf_enum_encode<u8>(KF_ITEM_GOLD_CROSS)]--;
            (co_await map_finish_exchange_dialogue(world, player, *event, key_exchange, capture));
            co_return;
        }
        break;
    case KF_CHARACTER_HEALING_EXCHANGE:
        if (map_exchange_is_available(world, player, healing_exchange, KF_ITEM_MIRROR_OF_TRUTH, PartyReward::Healing)) {
            if (world.party.enabled) party_complete_quest(world, PartyReward::Healing);
            else player.learned_magic[kf_enum_encode<u8>(KF_MAGIC_HEALING)] = KF_MAGIC_LEARNED;
            inventory[kf_enum_encode<u8>(KF_ITEM_MIRROR_OF_TRUTH)]--;
            if (!world.party.enabled && player.local_view) notify_enqueue(KF_NOTIFICATION_MAGIC_LEARNED);
            world.map.events[healing_exchange.event_slot].dialogue_pages
                .last_page[healing_exchange.last_page_slot] = healing_exchange.last_page;
            (co_await map_finish_exchange_dialogue(world, player, *event, healing_exchange, capture));
            co_return;
        }
        break;
    case KF_CHARACTER_HARP_EXCHANGE:
        if (map_exchange_is_available(world, player, harp_exchange, KF_ITEM_DRAGON_KING_GRASS_FRUIT, PartyReward::Harp)) {
            if (world.party.enabled) party_complete_quest(world, PartyReward::Harp);
            else inventory[kf_enum_encode<u8>(KF_ITEM_HARP)] = 1;
            world.map.events[harp_exchange.event_slot].dialogue_pages
                .last_page[harp_exchange.last_page_slot] = harp_exchange.last_page;
            inventory[kf_enum_encode<u8>(KF_ITEM_DRAGON_KING_GRASS_FRUIT)]--;
            (co_await map_finish_exchange_dialogue(world, player, *event, harp_exchange, capture));
            co_return;
        }
        break;
    case KF_CHARACTER_FLOOR3_DOOR_UNLOCKER:
        map_object_pool_clear_link(world, MAP_FLOOR3_DIALOGUE_DOOR_LINK);
        break;
    default:
        break;
    }

    if (event->dialogue.stage_limit != 0) {
        if (event->dialogue_pages.last_page[event->dialogue.stage - 1] != 0) {
            co_await map_dialogue_page(player, *event, event->dialogue.page, capture);
            if (!capture && event->dialogue.page_delay == 0) {
                event->dialogue.page_delay = KF_DIALOGUE_PAGE_DELAY_TICKS;
            }
        }
    }
}

kf::FrameTask<void> map_show_screen_image(PlayerContext &player, KfMapImageGroup group, s32 index)
{
    char *directory_floor = &map_screen_image_path[map_screen_floor_offset];

    *directory_floor = kf_enum_encode<u8>(player.state.progress_state.current_floor) + '0';
    map_screen_image_path[map_screen_group_offset] = kf_enum_encode<s32>(group) + '0';
    map_screen_image_path[map_screen_number_offset] = index / 10 + '0';
    map_screen_image_path[map_screen_number_offset + 1] = index % 10 + '0';
    (co_await screen_show_image_until_input(directory_floor - map_screen_floor_offset));
}

bool map_object_image_valid(KfObjectId object, u8 image)
{
    return (object == KF_MAP_OBJECT_SIGNBOARD || object == KF_MAP_OBJECT_INSCRIPTION_PANEL) && image < 100;
}

kf::FrameTask<void> map_show_object_image(PlayerContext &player, KfObjectId object, u8 image)
{
    if (!map_object_image_valid(object, image)) co_return;
    co_await map_show_screen_image(player,
        object == KF_MAP_OBJECT_SIGNBOARD ? MAP_IMAGE_GROUP_SIGNBOARD : MAP_IMAGE_GROUP_INSCRIPTION, image);
    player_clear_motion(player);
}

static void map_finish_event_interaction(PlayerContext &player, KfMapEvent *event)
{
    event->animation_phase = 0;
    player_clear_motion(player);
}

static kf::FrameTask<void> map_interact_hinged_container(WorldState &world, PlayerContext &player, KfMapObject *object,
    const VECTOR *position, SVECTOR *rotation)
{
    u16 saved_pitch;
    KfMenuResult pickup_result;
    s16 item_index;
    KfObjectId *item_id;

    if (object->link.fields.link_id != KF_MAP_LINK_NONE) {
        notify_enqueue(object->link.fields.linked_notification);
        co_return;
    }
    if (!angle_within_tolerance(rotation->vy, object->rotation.angles.y, KF_ANGLE_EIGHTH_TURN)) {
        co_return;
    }

    item_index = KF_MAP_CONTAINER_ITEM_COUNT - 1;
    for (;;) {
        KfObjectId item_parameter = object->link.hinged_container.item_ids[0];
        s32 item_scan_end;

        if (item_parameter != KF_OBJECT_NONE) {
            break;
        }
        item_scan_end = -1;
        if (--item_index == item_scan_end) {
            notify_enqueue(object->link.fields.default_notification);
            co_return;
        }
    }

    saved_pitch = rotation->vx;
    audio_play_spatial_default_range(player,
        &gameplay_sound_refs[KF_GAMEPLAY_SOUND_CONTAINER_OPEN], &object->position, KF_AUDIO_MAX_VOLUME);
    while (object->rotation.angles.x >= -(KF_ANGLE_QUARTER_TURN - 1)) {
        u16 current_pitch = rotation->vx;
        u16 relative_pitch = current_pitch;

        relative_pitch -= KF_PLAYER_CAMERA_PITCH_LIMIT;
        if (relative_pitch >= MAP_CONTAINER_CAMERA_PITCH_SPAN) {
            rotation->vx = current_pitch + MAP_CONTAINER_CAMERA_PITCH_STEP;
        }
        object->rotation.angles.x -= MAP_CONTAINER_OPEN_PITCH_STEP;
        (co_await render_frame(world, player, position, rotation));
    }

    item_id = object->link.hinged_container.item_ids;
    item_index = KF_MAP_CONTAINER_ITEM_COUNT - 1;
    for (;;) {
        if (*item_id != KF_OBJECT_NONE) {
            pickup_result = kf_enum_decode<KfMenuResult>((co_await menu_enter_mode(world, player, KF_MENU_MODE_ITEM_PICKUP, *item_id)));
            switch (pickup_result) {
            case KF_MENU_RESULT_ACCEPTED:
                *item_id = KF_OBJECT_NONE;
                break;
            case KF_MENU_RESULT_STACK_FULL:
                notify_enqueue(KF_NOTIFICATION_CANNOT_CARRY_MORE);
                break;
            default:
                break;
            }
        }
        item_index--;
        if (item_index == -1) {
            break;
        }
        item_id++;
    }
    object->rotation.angles.x = 0;
    rotation->vx = saved_pitch;
}

bool map_start_hinged_door_pair(WorldState &world, KfMapObject *object,
    const KfMapObjectDefinition *definition, s32 index)
{
    s32 neighbor_index;
    KfMapObject *neighbor;
    KfMapObjectDefinition *neighbor_definition;

    neighbor_index = 0;
    for (;;) {
        neighbor_index = map_object_pool_find_interaction_from(world,
            neighbor_index, object->position.vx, object->position.vz, MAP_DOOR_PARTNER_SEARCH_PADDING);
        if (neighbor_index == -1) {
            break;
        }
        if (neighbor_index != index) {
            neighbor = &world.objects.objects[neighbor_index];
            neighbor_definition =
                &world.objects.definitions.entries[kf_enum_encode<u8>(neighbor->object_id)];
            if (neighbor_definition->behavior_type == KF_MAP_OBJECT_OP_HINGED_DOOR
                || neighbor_definition->behavior_type == KF_MAP_OBJECT_OP_HINGED_DOOR_PARTNER) {
                if (neighbor->link.fields.link_id != KF_MAP_LINK_NONE
                    && neighbor_definition->behavior_type == KF_MAP_OBJECT_OP_HINGED_DOOR) {
                    return false;
                }
                map_object_start_action_if_idle(
                    neighbor, neighbor_definition->behavior_type);
                object->link.fields.action_parameter.object_index = neighbor_index;
                neighbor->link.fields.action_parameter.object_index = index;
                map_object_start_action_if_idle(object, definition->behavior_type);
                return true;
            }
        }
        neighbor_index++;
    }
    object->link.fields.action_parameter.object_index = KF_MAP_OBJECT_PARAMETER_NONE;
    map_object_start_action_if_idle(object, definition->behavior_type);
    return true;
}

kf::FrameTask<void> map_interaction_dispatch(WorldState &world, PlayerContext &player, const VECTOR *position, SVECTOR *rotation)
{
    kf::InputContextScope input_context(kf::InputContext::Scripted);
    s32 index;
    s32 result;
    KfMenuResult pickup_result;
    KfBool8 found_item;
    KfMapEvent *event;
    KfMapObject *object;
    KfMapObjectDefinition *definition;

    const auto attribute_probe = vector_yaw_probe_xz(
        *position, rotation->vy, MAP_ATTRIBUTE_PROBE_DISTANCE);
    switch (map_attribute_at_cell(world,
        attribute_probe.x / KF_MAP_TILE_SIZE, attribute_probe.z / KF_MAP_TILE_SIZE)) {
    case KF_MAP_ATTRIBUTE_PITFALL:
        notify_enqueue(KF_NOTIFICATION_PITFALL);
        break;
    case KF_MAP_ATTRIBUTE_POISON_HOLE:
        notify_enqueue(KF_NOTIFICATION_POISON_HOLE);
        break;
    case KF_MAP_ATTRIBUTE_BOTTOMLESS_PIT:
        notify_enqueue(KF_NOTIFICATION_BOTTOMLESS_PIT);
        break;
    case KF_MAP_ATTRIBUTE_HIDDEN_DOOR:
        notify_enqueue(KF_NOTIFICATION_HIDDEN_DOOR);
        break;
    default:
        break;
    }

    const auto interaction_probe = vector_yaw_probe_xz(
        *position, rotation->vy, MAP_INTERACTION_PROBE_DISTANCE);
    if (game_graphics_runtime.notification_state.control.effect_phase == KF_NOTIFICATION_IDLE
        && (index = map_event_pool_find_overlap(world,
                interaction_probe.x, interaction_probe.z, MAP_INTERACTION_RADIUS_PADDING)) != -1) {
        event = &world.map.events[index];
        switch (event->behavior) {
            case KF_MAP_EVENT_BEHAVIOR_SHOP:
                event->animation_phase = 0;
                event->animation_clip = KF_ANIMATION_CLIP_FIRST;
                (co_await map_event_advance_animation_blocking(world, player, event, KF_MAP_EVENT_ANIMATION_TALK_POSE, KF_MAP_EVENT_ANIMATION_TALK_STEP));
                (co_await audio_play_map_sequence(player, MAP_SHOP_SEQUENCE_INDEX));
                (co_await map_event_interact(world, player, event));
                (co_await menu_enter_mode(world, player, KF_MENU_MODE_SHOP, kf_enum_decode<KfItemStockBank>(kf_enum_encode<u8>(event->character_id))));
                (co_await audio_play_current_map_sequence(player));
                (co_await map_event_advance_animation_blocking(world, player, event, KF_MAP_EVENT_ANIMATION_PHASE_MASK, KF_MAP_EVENT_ANIMATION_TALK_STEP));
                map_finish_event_interaction(player, event);
                break;
            case KF_MAP_EVENT_BEHAVIOR_ANIMATION_LOOP:
                result = game_graphics_runtime.asset_registry_entries[
                    event->model_index + KF_ASSET_MAP_EVENT_FIRST]->animation_clip_count;
                (co_await map_event_advance_animation_blocking(world, player, event, KF_MAP_EVENT_ANIMATION_PHASE_MASK, KF_MAP_EVENT_ANIMATION_FINISH_STEP));
                result = result < 2;
                if (result == 0) {
                    event->animation_phase = 0;
                    event->animation_clip = KF_ANIMATION_CLIP_SECOND;
                    (co_await map_event_advance_animation_blocking(world, player, event, KF_MAP_EVENT_ANIMATION_TALK_POSE, KF_MAP_EVENT_ANIMATION_TALK_STEP));
                }
                (co_await map_event_interact(world, player, event));
                if (result == 0) {
                    (co_await map_event_advance_animation_blocking(world, player, event, KF_MAP_EVENT_ANIMATION_PHASE_MASK, KF_MAP_EVENT_ANIMATION_TALK_STEP));
                }
                event->animation_clip = KF_ANIMATION_CLIP_FIRST;
                map_finish_event_interaction(player, event);
                break;
            case KF_MAP_EVENT_BEHAVIOR_WANDER:
                (co_await map_event_interact(world, player, event));
                player_clear_motion(player);
                break;
            default:
                break;
        }
    } else {
        for (index = 0;; index++) {
            index = map_object_pool_find_interaction_from(world,
                index, interaction_probe.x, interaction_probe.z, MAP_INTERACTION_RADIUS_PADDING);
            if (index == -1) {
                break;
            }
            object = &world.objects.objects[index];
            definition = &world.objects.definitions.entries[
                kf_enum_encode<u8>(object->object_id)];
            switch (definition->behavior_type) {
            case KF_MAP_OBJECT_OP_HINGED_CONTAINER:
                if (world.party.enabled) {
                    co_await player_loot_interact(world, player, index);
                    break;
                }
                (co_await map_interact_hinged_container(world, player, object, position, rotation));
                break;

            case KF_MAP_OBJECT_OP_ITEM_CONTAINER: {
                if (world.party.enabled) {
                    co_await player_loot_interact(world, player, index);
                    break;
                }
                s16 item_index;
                KfObjectId *item_id;

                item_id = object->link.item_ids;
                found_item = false;
                item_index = KF_MAP_CONTAINER_ITEM_COUNT - 1;
                for (;;) {
                    if (*item_id != KF_OBJECT_NONE) {
                        found_item = true;
                        pickup_result = kf_enum_decode<KfMenuResult>((co_await menu_enter_mode(world, player, KF_MENU_MODE_ITEM_PICKUP, *item_id)));
                        switch (pickup_result) {
                        case KF_MENU_RESULT_ACCEPTED:
                            *item_id = KF_OBJECT_NONE;
                            break;
                        case KF_MENU_RESULT_STACK_FULL:
                            notify_enqueue(KF_NOTIFICATION_CANNOT_CARRY_MORE);
                            break;
                        default:
                            break;
                        }
                    }
                    item_index--;
                    if (item_index == -1) {
                        break;
                    }
                    item_id++;
                }
                if (found_item != false) {
                    break;
                }
                notify_enqueue(object->link.fields.default_notification);
                break;

            }

            case KF_MAP_OBJECT_OP_LIFT_DOOR:
                if (!angle_within_tolerance(rotation->vy, object->rotation.angles.y, MAP_DOOR_FACING_TOLERANCE)
                    && !angle_within_tolerance(
                        rotation->vy, object->rotation.angles.y + KF_ANGLE_HALF_TURN, MAP_DOOR_FACING_TOLERANCE)) {
                    break;
                }
                if (object->action != KF_MAP_OBJECT_OP_NONE) {
                    break;
                }
                if (object->link.fields.link_id != KF_MAP_LINK_NONE) {
                    notify_enqueue(object->link.fields.default_notification);
                    break;
                }
                map_object_start_action_if_idle(object, KF_MAP_OBJECT_OP_LIFT_DOOR);
                continue;

            case KF_MAP_OBJECT_OP_HINGED_DOOR:
            case KF_MAP_OBJECT_OP_HINGED_DOOR_PARTNER:
                if (!angle_within_tolerance(rotation->vy, object->rotation.angles.y, MAP_DOOR_FACING_TOLERANCE)
                    && !angle_within_tolerance(
                        rotation->vy, object->rotation.angles.y + KF_ANGLE_HALF_TURN, MAP_DOOR_FACING_TOLERANCE)) {
                    break;
                }
                if (object->link.fields.link_id != KF_MAP_LINK_NONE && definition->behavior_type == KF_MAP_OBJECT_OP_HINGED_DOOR) {
                    notify_enqueue(object->link.fields.default_notification);
                    break;
                }

                if (!map_start_hinged_door_pair(world, object, definition, index))
                    notify_enqueue(object->link.fields.default_notification);
                continue;

            case KF_MAP_OBJECT_OP_ITEM_PICKUP:
                if (world.party.enabled) {
                    co_await player_loot_interact(world, player, index);
                    break;
                }
                pickup_result = kf_enum_decode<KfMenuResult>((co_await menu_enter_mode(world, player, KF_MENU_MODE_ITEM_PICKUP, object->object_id)));
                switch (pickup_result) {
                case KF_MENU_RESULT_ACCEPTED:
                    object->object_id = KF_OBJECT_NONE;
                    break;
                case KF_MENU_RESULT_STACK_FULL:
                    notify_enqueue(KF_NOTIFICATION_CANNOT_CARRY_MORE);
                    continue;
                default:
                    break;
                }
                break;

            case KF_MAP_OBJECT_OP_GOLD_PICKUP:
                if (world.party.enabled) {
                    co_await player_loot_interact(world, player, index);
                    break;
                }
                result = object->link.gold_amount;
                notify_enqueue(KF_NOTIFICATION_GOLD, result);
                result += player.state.gold;
                player.state.gold = result;
                object->object_id = KF_OBJECT_NONE;
                break;

            case KF_MAP_OBJECT_OP_EFFECT_SWITCH:
                if (object->link.fields.link_id == KF_MAP_LINK_NONE) {
                    notify_enqueue(object->link.fields.default_notification);
                } else {
                    object->action_timer = KF_MAP_OBJECT_SWITCH_FORWARD;
                }
                break;

            case KF_MAP_OBJECT_OP_RESTORE_POINT:
                if (object->link.fields.link_id != KF_MAP_LINK_NONE) {
                    notify_enqueue(object->link.fields.default_notification);
                    break;
                }
                (co_await player_restore_vitals_with_color_cycle(world, player));
                continue;

            case KF_MAP_OBJECT_OP_SCREEN_IMAGE: {
                KfMapImageGroup image_group;

                if (game_graphics_runtime.notification_state.control.effect_phase != KF_NOTIFICATION_IDLE) {
                    break;
                }
                if (object->object_id == KF_MAP_OBJECT_SIGNBOARD) {
                    image_group = MAP_IMAGE_GROUP_SIGNBOARD;
                } else {
                    if (object->object_id != KF_MAP_OBJECT_INSCRIPTION_PANEL) {

                        co_return;
                    }
                    image_group = MAP_IMAGE_GROUP_INSCRIPTION;
                }
                (co_await map_show_screen_image(player, image_group, object->link.fields.link_id));
                player_clear_motion(player);
                continue;
            }

            case KF_MAP_OBJECT_OP_SAVE_POINT:
                map_world_state_persist(world, player);
                (co_await menu_save_confirm(world, player));
                continue;

            default:
                notify_enqueue(object->link.fields.default_notification);
                break;
            }
        }
    }

    switch (player.state.progress_state.current_floor) {
    case KF_FLOOR_1:
        map_action_script_floor1(world, player);
        break;
    case KF_FLOOR_2:
        (co_await map_action_script_floor2(world, player));
        break;
    case KF_FLOOR_3:
        map_action_script_floor3(world, player);
        break;
    case KF_FLOOR_4:
        map_action_script_floor4();
        break;
    case KF_FLOOR_5:
        (co_await map_action_script_floor5(world, player));
        break;
    default:
        break;
    }

}

void map_scripts_reset_module_state(void)
{
    kf::restore_initial_value<map_floor5_camera_path>();
    kf::restore_initial_value<map_floor1_sound_position>();
    kf::restore_initial_value<map_transfer_light_matrix>();
    kf::restore_initial_value<map_screen_image_path>();
}
