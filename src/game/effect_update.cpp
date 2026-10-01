#include <kf/game/world.h>
#include <kf/game/player.h>
#include <kf/lib/random.hpp>
#include <kf/game/audio.h>
#include <kf/game/map_data.h>
#include <kf/game/collision.h>
#include <kf/game/effect.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>

static constexpr u32 EFFECT_PHASE_BYTE_MASK = 0xff;
#include <kf/game/game.h>

enum {
    EFFECT_FIXED_MAGIC_POWER = 5,
    EFFECT_SWING_COLLISION_RADIUS = 120,
    EFFECT_ORBIT_COLLISION_RADIUS = 150,
    EFFECT_SWING_ANGULAR_ACCEL = 10,
    EFFECT_HAZARD_RISE_STEP = 60,
    EFFECT_HAZARD_SOUND_RANDOM_CUTOFF = (kf::random_max + 1) / 4,
    EFFECT_SWING_SOUND_MAX_DISTANCE = 3000,
    EFFECT_ORBIT_SOUND_MAX_DISTANCE = 5000,
    EFFECT_HAZARD_SOUND_ATTENUATION_DISTANCE = 14000,
    EFFECT_ORBIT_UPDATES_PER_TURN = 64,
    FLOOR_DEFORM_SOUND_PROGRESS = 3900,
    FLOOR_DEFORM_SEGMENT_COUNT = 5,
    TRAIL_UNIT_SCALE_DISTANCE = 800,
    GROUND_BRANCH_CHILD_SPACING = 1500
};

static KfFloorDeformSegment floor_deform_segments[FLOOR_DEFORM_SEGMENT_COUNT] = {
    {65, 80, 1, 0, 2, 0, 100},
    {61, 73, 0, 255, 2, 0, 100},
    {75, 56, 1, 0, 3, 0, 100},
    {37, 27, 0, 255, 3, 0, 100},
    {32, 82, 0, 1, 12, 0, 100}
};

int effect_magic_power(PlayerContext &player, KfEffectRecord *effect)
{
    if ((effect->type & KF_EFFECT_USE_PLAYER_MAGIC) != KF_EFFECT_TYPE_NONE) {
        return player.state.magic;
    }
    return EFFECT_FIXED_MAGIC_POWER;
}

void effect_update_swinging_hazard(WorldState &world, PlayerContext &player, SVECTOR *probe_offset, KfEffectPhase phase_limit)
{
    KfEffectRecord *record = world.effects.current_record;
    KfMagicRecord *magic = world.effects.current_magic;
    KfEffectPhase life = record->phase;
    MATRIX rotation_matrix;
    MATRIX yaw_matrix;
    VECTOR world_position;
    u32 collision;
    s16 pitch;
    s16 next_pitch;

    if (kf_enum_encode<u8>(life) < kf_enum_encode<u8>(KF_EFFECT_HAZARD_RELEASE_REQUEST) + 1u) {
        kf::matrix_set_rotation_xyz(record->rotation.vector, rotation_matrix);
        matrix_set_rotation_x(record->rotation.vector.vx, &rotation_matrix);
        matrix_set_rotation_y(record->rotation.vector.vy, &yaw_matrix);
        kf::matrix_multiply_rotation(yaw_matrix, rotation_matrix, rotation_matrix);
        world_position = kf::matrix_apply_rotation(rotation_matrix, *probe_offset);
        world_position += record->position;
        collision = effect_map_collision(world, player, &world_position, EFFECT_SWING_COLLISION_RADIUS);
        if (collision != KF_COLLISION_NONE) {
            if ((collision >> KF_COLLISION_KIND_SHIFT) == (KF_COLLISION_ACTOR >> KF_COLLISION_KIND_SHIFT)) {
                actor_apply_damage(world, player, collision & KF_COLLISION_DETAIL_MASK, 0, magic->damage_components[0],
                    magic->damage_components[2], magic->damage_components[1],
                    0, 0, KF_ACTOR_DAMAGE_SCALE_ONE, record->type);
            } else if ((collision >> KF_COLLISION_KIND_SHIFT) == (KF_COLLISION_PLAYER >> KF_COLLISION_KIND_SHIFT)) {
                player_apply_damage(party_collision_player(world, player, collision), magic->damage_components[0],
                    magic->damage_components[2], magic->damage_components[1],
                    KF_PLAYER_STATUS_NONE, 0, 0, KF_FIXED12_ONE, record->id);
            }
            record->direction.words.x = -record->direction.words.x;
        }
        if (record->sound_played == KF_AUDIO_NOT_PLAYED) {
            if (effect_random_next(world) < EFFECT_HAZARD_SOUND_RANDOM_CUTOFF) {
                record->sound_played = audio_play_spatial_range(player,
                    &magic->sounds[0], &world_position, KF_AUDIO_MAX_VOLUME,
                    EFFECT_SWING_SOUND_MAX_DISTANCE, EFFECT_HAZARD_SOUND_ATTENUATION_DISTANCE);
            }
        }
        if (record->rotation.vector.vx >= KF_ANGLE_EIGHTH_TURN) {
            record->rotation.vector.vx = KF_ANGLE_EIGHTH_TURN;
            record->direction.words.x = 0;
        } else if (record->rotation.vector.vy < -KF_ANGLE_EIGHTH_TURN + 1) {
            record->rotation.vector.vx = -KF_ANGLE_EIGHTH_TURN;
            record->direction.words.x = 0;
        }
        if (record->rotation.vector.vx > 0) {
            record->direction.words.x -= EFFECT_SWING_ANGULAR_ACCEL;
        } else {
            record->direction.words.x += EFFECT_SWING_ANGULAR_ACCEL;
        }
        pitch = record->rotation.vector.vx;
        next_pitch = record->rotation.vector.vx + record->direction.words.x;
        if ((next_pitch <= 0 && pitch >= 0) || (next_pitch >= 0 && pitch <= 0)) {
            if (life == KF_EFFECT_HAZARD_RELEASE_REQUEST) {
                next_pitch = 0;
                record->phase = KF_EFFECT_HAZARD_RISE_FIRST;
            } else {
                record->sound_played = KF_AUDIO_NOT_PLAYED;
            }
        }
        record->rotation.vector.vx = next_pitch;
    } else if (kf_enum_encode<u8>(life) >= kf_enum_encode<u8>(KF_EFFECT_HAZARD_RISE_FIRST) && kf_enum_encode<s16>(phase_limit) >= kf_enum_encode<u8>(life)) {
        record->position.vy -= EFFECT_HAZARD_RISE_STEP;
        record->phase++;
    }
}

void effect_update_orbiting_projectile(WorldState &world, PlayerContext &player, s32 orbit_radius, KfEffectPhase phase_limit)
{
    KfEffectRecord *record = world.effects.current_record;
    KfMagicRecord *magic = world.effects.current_magic;
    KfEnumStorage<KfEffectPhase, u32> life = record->phase;
    u32 collision;

    if ((kf_enum_encode<u32>(life) & EFFECT_PHASE_BYTE_MASK) < kf_enum_encode<u8>(KF_EFFECT_HAZARD_RELEASE_REQUEST) + 1) {
        record->position.vx = (record->direction.vector.vx << KF_EFFECT_ORBIT_CENTER_SHIFT)
            + (kf::angle_sine((s16)record->control.orbit_angle) * orbit_radius >> KF_FIXED12_BITS);
        record->position.vz = (record->direction.vector.vz << KF_EFFECT_ORBIT_CENTER_SHIFT)
            + (kf::angle_cosine((s16)record->control.orbit_angle) * orbit_radius >> KF_FIXED12_BITS);
        record->position.vy = record->direction.vector.vy
            + (kf::angle_sine((s16)record->control.orbit_angle << 1) >> 2);
        record->control.orbit_angle = (record->control.orbit_angle
            + KF_ANGLE_FULL_TURN / EFFECT_ORBIT_UPDATES_PER_TURN) & KF_ANGLE_WRAP_MASK;
        collision = effect_map_collision(world, player, &record->position, EFFECT_ORBIT_COLLISION_RADIUS);
        if (collision != KF_COLLISION_NONE) {
            if ((collision >> KF_COLLISION_KIND_SHIFT) == (KF_COLLISION_ACTOR >> KF_COLLISION_KIND_SHIFT)) {
                actor_apply_damage(world, player, collision & KF_COLLISION_DETAIL_MASK, 0, magic->damage_components[0],
                    magic->damage_components[2], magic->damage_components[1],
                    0, 0, KF_ACTOR_DAMAGE_SCALE_ONE, record->type);
            } else if ((collision >> KF_COLLISION_KIND_SHIFT) == (KF_COLLISION_PLAYER >> KF_COLLISION_KIND_SHIFT)) {
                player_apply_damage(party_collision_player(world, player, collision), magic->damage_components[0],
                    magic->damage_components[2], magic->damage_components[1],
                    KF_PLAYER_STATUS_NONE, 0, 0, KF_FIXED12_ONE, record->id);
            }
        }
        if (record->sound_played == KF_AUDIO_NOT_PLAYED) {
            if (effect_random_next(world) < EFFECT_HAZARD_SOUND_RANDOM_CUTOFF) {
                record->sound_played = audio_play_spatial_range(player,
                    &magic->sounds[0], &record->position,
                    KF_AUDIO_MAX_VOLUME, EFFECT_ORBIT_SOUND_MAX_DISTANCE,
                    EFFECT_HAZARD_SOUND_ATTENUATION_DISTANCE);
            }
        } else {
            const auto in_range = [&](const PlayerContext &listener) {
                const auto dx = std::int64_t(record->position.vx) - listener.state.camera_position.vx;
                const auto dy = std::int64_t(record->position.vy) - listener.state.camera_position.vy;
                const auto dz = std::int64_t(record->position.vz) - listener.state.camera_position.vz;
                return std::abs(dx) < EFFECT_ORBIT_SOUND_MAX_DISTANCE &&
                    std::abs(dy) < EFFECT_ORBIT_SOUND_MAX_DISTANCE && std::abs(dz) < EFFECT_ORBIT_SOUND_MAX_DISTANCE &&
                    fixed_vector3_length(dx, dy, dz) < EFFECT_ORBIT_SOUND_MAX_DISTANCE;
            };
            bool heard = in_range(player);
            if (world.party.enabled) {
                heard = false;
                for (const auto &member : world.party.members)
                    if (member.connected && (member.presence == PartyPresence::Living || member.presence == PartyPresence::Spectating))
                        heard |= in_range(member.player);
            }
            if (!heard) {
                record->sound_played = KF_AUDIO_NOT_PLAYED;
            }
        }
    } else if ((kf_enum_encode<u32>(life) & EFFECT_PHASE_BYTE_MASK) != kf_enum_encode<u8>(KF_EFFECT_HAZARD_RUNNING) && kf_enum_encode<s16>(phase_limit) >= (int)(kf_enum_encode<u32>(life) & EFFECT_PHASE_BYTE_MASK)) {
        record->position.vy -= EFFECT_HAZARD_RISE_STEP;
        record->phase++;
    }
}

void effect_floor_deform_line(WorldState &world, PlayerContext &player, s32 segment_index, s32 progress_start, s32 progress_step)
{
    KfFloorDeformSegment *segment = &floor_deform_segments[segment_index];
    int range = progress_step;
    VECTOR sound_position;
    u8 col;
    u8 row;
    int count;
    int height_delta;

    if (range < 0) {
        range = -range;
    }
    sound_position.vy = -(segment->end_height * KF_MAP_HEIGHT_STEP);
    col = segment->column;
    row = segment->row;
    count = segment->cell_count;
    height_delta = segment->end_height - segment->start_height;
    while (--count != -1) {
        int progress = progress_start;
        progress_start += progress_step;
        if (progress < 0) {
            progress = 0;
        } else if (progress >= KF_FIXED12_ONE + 1) {
            progress = KF_FIXED12_ONE;
        } else if (progress >= FLOOR_DEFORM_SOUND_PROGRESS && progress < range + FLOOR_DEFORM_SOUND_PROGRESS) {
            sound_position.vx = KF_MAP_TILE_SIZE * col + KF_MAP_TILE_CENTER;
            sound_position.vz = KF_MAP_TILE_SIZE * row + KF_MAP_TILE_CENTER;
            audio_play_spatial_default_range(player, &gameplay_sound_refs[KF_GAMEPLAY_SOUND_FLOOR_DEFORM],
                &sound_position, KF_AUDIO_MAX_VOLUME);
        }
        world.floor_height.cells[row][col] =
            ((height_delta * progress) >> KF_FIXED12_BITS) + segment->start_height;
        col += segment->column_step;
        row += segment->row_step;
    }
}

enum {
    SCATTER_RANDOM_SHIFT = 8,
    SCATTER_VELOCITY_BIAS = ((kf::random_max >> SCATTER_RANDOM_SHIFT) + 1) / 2
};

s32 effect_random_next(WorldState &world)
{
    if (!world.party.enabled) return kf::random_next();
    if (!world.effects.current_record) kf::host_fail("Effect random draw without an effect");
    return kf::random_next(world.effects.current_record->random);
}

void effect_scatter_triple(WorldState &world, KfEffectDirectionWords *velocity)
{
    int random;
    int centered;

    random = effect_random_next(world);
    centered = velocity->x - SCATTER_VELOCITY_BIAS;
    centered += random >> SCATTER_RANDOM_SHIFT;
    velocity->x = centered;
    random = effect_random_next(world);
    centered = velocity->y - SCATTER_VELOCITY_BIAS;
    centered += random >> SCATTER_RANDOM_SHIFT;
    velocity->y = centered;
    random = effect_random_next(world);
    centered = velocity->z - SCATTER_VELOCITY_BIAS;
    centered += random >> SCATTER_RANDOM_SHIFT;
    velocity->z = centered;
}

void effect_rotate_scale_offset_y(SVECTOR *offset, VECTOR *output, s16 angle, s32 scale)
{
    SVECTOR scaled;
    SVECTOR rotation;
    MATRIX matrix;

    scaled = VECTOR{
        (offset->vx * scale) >> KF_FIXED12_BITS,
        0,
        (offset->vz * scale) >> KF_FIXED12_BITS}.narrowed();
    rotation = {0, angle, 0};
    kf::matrix_set_rotation_xyz(rotation, matrix);
    *output = kf::matrix_apply_rotation(matrix, scaled);
}

void effect_spawn_ground_trail(WorldState &world, PlayerContext &player, u8 id, KfEffectRecord *parent_effect, s16 angle, s32 distance)
{
    VECTOR position;
    s32 index;
    s32 scale = (distance << KF_FIXED12_BITS) / TRAIL_UNIT_SCALE_DISTANCE;

    effect_rotate_scale_offset_y(&parent_effect->direction.vector, &position, angle, scale);
    index = parent_effect - world.effects.records;
    position.vx += parent_effect->position.vx;
    position.vz += parent_effect->position.vz;
    effect_pool_construct(world, player, id, parent_effect->type, KF_EFFECT_KIND_GROUND_TRAIL, &position,
        &parent_effect->direction.vector, KfEffectParentArguments{index});
}

void effect_spawn_ground_branch(WorldState &world, PlayerContext &player, u8 id, KfEffectRecord *parent_effect, s16 angle_offset, KfEffectGroundBranchRole branch_role)
{
    VECTOR position;
    s32 angle = -(s16)(parent_effect->direction.words.y + angle_offset);
    s32 cell_x;
    s32 cell_z;

    position.vx = parent_effect->position.vx + (GROUND_BRANCH_CHILD_SPACING * kf::angle_sine(angle) >> KF_FIXED12_BITS);
    position.vz = parent_effect->position.vz + (GROUND_BRANCH_CHILD_SPACING * kf::angle_cosine(angle) >> KF_FIXED12_BITS);
    if (position.vx < 0 || position.vx >= KF_MAP_COLUMNS * KF_MAP_TILE_SIZE ||
        position.vz < 0 || position.vz >= KF_MAP_ROWS * KF_MAP_TILE_SIZE) return;
    cell_z = position.vz / KF_MAP_TILE_SIZE;
    cell_x = position.vx / KF_MAP_TILE_SIZE;
    position.vy = -(world.floor_height.cells[cell_z][cell_x] * KF_MAP_HEIGHT_STEP);
    effect_pool_construct(world, player, id, parent_effect->type, KF_MAGIC_FIRE_WALL, &position,
        &parent_effect->direction.vector, KfEffectBranchArguments{branch_role});
}

void effect_update_reset_module_state(void)
{
    kf::restore_initial_value<floor_deform_segments>();
}
