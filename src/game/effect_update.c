#include <kf/address.h>
#include <kf/audio.h>
#include <kf/map_data.h>
#include <kf/game_collision.h>
#include <kf/game_effect.h>
#include <psyq/libc.h>
#include <kf/game.h>

enum {
    EFFECT_FIXED_MAGIC_POWER = 5,
    EFFECT_SWING_COLLISION_RADIUS = 120,
    EFFECT_ORBIT_COLLISION_RADIUS = 150,
    EFFECT_SWING_ANGULAR_ACCEL = 10,
    EFFECT_HAZARD_RISE_STEP = 60,
    EFFECT_HAZARD_SOUND_RANDOM_CUTOFF = (RAND_MAX + 1) / 4,
    EFFECT_SWING_SOUND_MAX_DISTANCE = 3000,
    EFFECT_ORBIT_SOUND_MAX_DISTANCE = 5000,
    EFFECT_ORBIT_SOUND_RESET_DISTANCE = 5000,
    EFFECT_HAZARD_SOUND_ATTENUATION_DISTANCE = 14000,
    EFFECT_ORBIT_UPDATES_PER_TURN = 64,
    FLOOR_DEFORM_SOUND_PROGRESS = 3900,
    FLOOR_DEFORM_SEGMENT_COUNT = 5,
    TRAIL_UNIT_SCALE_DISTANCE = 800,
    GROUND_BRANCH_CHILD_SPACING = 1500
};

DATA(0x80056268, 0x23)
static KfFloorDeformSegment floor_deform_segments[FLOOR_DEFORM_SEGMENT_COUNT] = {
    {65, 80, 1, 0, 2, 0, 100},
    {61, 73, 0, 255, 2, 0, 100},
    {75, 56, 1, 0, 3, 0, 100},
    {37, 27, 0, 255, 3, 0, 100},
    {32, 82, 0, 1, 12, 0, 100}
};

/* Per-kind helpers share the current effect/magic context. The dispatcher
 * is a separate working owner; the original file boundary remains unknown. */

ADDRESS(0x80037fbc, 0x24)
int effect_magic_power(KfEffectRecord *effect)
{
    if ((effect->type & KF_EFFECT_USE_PLAYER_MAGIC) != KF_EFFECT_TYPE_NONE) {
        return player_state.magic;
    }
    return EFFECT_FIXED_MAGIC_POWER;
}

ADDRESS(0x80037fe0, 0x2b8)
void effect_projectile_update_3d(SVECTOR *probe_offset, KF_ENUM_PARAM(KfEffectPhase, s32) phase_limit)
{
    KfEffectRecord *record = current_effect;
    KfMagicRecord *magic = current_effect_magic_record;
    KfEffectPhase life = record->phase;
    MATRIX rotation_matrix;
    MATRIX yaw_matrix;
    VECTOR world;
    u32 collision;
    s16 pitch;
    s16 next_pitch;

    if (KF_ENUM_ENCODE(u8, life) < KF_ENUM_ENCODE(u8, KF_EFFECT_HAZARD_RELEASE_REQUEST) + 1u) {
        RotMatrix(&record->rotation.vector, &rotation_matrix);
        matrix_set_rotation_x(record->rotation.vector.vx, &rotation_matrix);
        matrix_set_rotation_y(record->rotation.vector.vy, &yaw_matrix);
        MulMatrix2(&yaw_matrix, &rotation_matrix);
        ApplyMatrix(&rotation_matrix, probe_offset, &world);
        addVector(&world, &record->position);
        collision = effect_map_collision(&world, EFFECT_SWING_COLLISION_RADIUS);
        if (collision != KF_COLLISION_NONE) {
            if ((collision >> KF_COLLISION_KIND_SHIFT) == (KF_COLLISION_ACTOR >> KF_COLLISION_KIND_SHIFT)) {
                actor_apply_damage(collision & KF_COLLISION_DETAIL_MASK, 0, magic->damage_components[0],
                    magic->damage_components[2], magic->damage_components[1],
                    0, 0, KF_ACTOR_DAMAGE_SCALE_ONE, record->type);
            } else if ((collision >> KF_COLLISION_KIND_SHIFT) == (KF_COLLISION_PLAYER >> KF_COLLISION_KIND_SHIFT)) {
                player_apply_damage(magic->damage_components[0],
                    magic->damage_components[2], magic->damage_components[1],
                    KF_PLAYER_STATUS_NONE, 0, 0, KF_FIXED12_ONE, record->id);
            }
            record->direction.words.x = -record->direction.words.x;
        }
        if (record->sound_played == KF_AUDIO_NOT_PLAYED) {
            if (rand() < EFFECT_HAZARD_SOUND_RANDOM_CUTOFF) {
                record->sound_played = audio_play_spatial_range(
                    &magic->sounds[0], &world, KF_AUDIO_MAX_VOLUME,
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
    } else if (KF_ENUM_ENCODE(u8, life) >= KF_ENUM_ENCODE(u8, KF_EFFECT_HAZARD_RISE_FIRST) && KF_ENUM_ENCODE(s16, phase_limit) >= KF_ENUM_ENCODE(u8, life)) {
        record->position.vy -= EFFECT_HAZARD_RISE_STEP;
        record->phase++;
    }
}

ADDRESS(0x80038298, 0x260)
void effect_projectile_update_2d(s32 orbit_radius, KF_ENUM_PARAM(KfEffectPhase, s32) phase_limit)
{
    KfEffectRecord *record = current_effect;
    KfMagicRecord *magic = current_effect_magic_record;
    KF_ENUM_STORAGE(KfEffectPhase, u32) life = record->phase;
    u32 collision;

    if ((KF_ENUM_ENCODE(u32, life) & 0xff) < KF_ENUM_ENCODE(u8, KF_EFFECT_HAZARD_RELEASE_REQUEST) + 1) {
        record->position.vx = (record->direction.vector.vx << KF_EFFECT_ORBIT_CENTER_SHIFT)
            + (rsin((s16)record->control.orbit_angle) * orbit_radius >> KF_FIXED12_BITS);
        record->position.vz = (record->direction.vector.vz << KF_EFFECT_ORBIT_CENTER_SHIFT)
            + (rcos((s16)record->control.orbit_angle) * orbit_radius >> KF_FIXED12_BITS);
        record->position.vy = record->direction.vector.vy
            + (rsin((s16)record->control.orbit_angle << 1) >> 2);
        record->control.orbit_angle = (record->control.orbit_angle
            + KF_ANGLE_FULL_TURN / EFFECT_ORBIT_UPDATES_PER_TURN) & KF_ANGLE_WRAP_MASK;
        collision = effect_map_collision(&record->position, EFFECT_ORBIT_COLLISION_RADIUS);
        if (collision != KF_COLLISION_NONE) {
            if ((collision >> KF_COLLISION_KIND_SHIFT) == (KF_COLLISION_ACTOR >> KF_COLLISION_KIND_SHIFT)) {
                actor_apply_damage(collision & KF_COLLISION_DETAIL_MASK, 0, magic->damage_components[0],
                    magic->damage_components[2], magic->damage_components[1],
                    0, 0, KF_ACTOR_DAMAGE_SCALE_ONE, record->type);
            } else if ((collision >> KF_COLLISION_KIND_SHIFT) == (KF_COLLISION_PLAYER >> KF_COLLISION_KIND_SHIFT)) {
                player_apply_damage(magic->damage_components[0],
                    magic->damage_components[2], magic->damage_components[1],
                    KF_PLAYER_STATUS_NONE, 0, 0, KF_FIXED12_ONE, record->id);
            }
        }
        if (record->sound_played == KF_AUDIO_NOT_PLAYED) {
            if (rand() < EFFECT_HAZARD_SOUND_RANDOM_CUTOFF) {
                record->sound_played = audio_play_spatial_range(
                    &magic->sounds[0], &record->position,
                    KF_AUDIO_MAX_VOLUME, EFFECT_ORBIT_SOUND_MAX_DISTANCE,
                    EFFECT_HAZARD_SOUND_ATTENUATION_DISTANCE);
            }
        } else {
            s32 dx = (record->position.vx - player_state.camera_position.vx) >> KF_LENGTH_SQUARE_DOWNSHIFT;
            s32 dy = (record->position.vy - player_state.camera_position.vy) >> KF_LENGTH_SQUARE_DOWNSHIFT;
            s32 dz = (record->position.vz - player_state.camera_position.vz) >> KF_LENGTH_SQUARE_DOWNSHIFT;
            if ((SquareRoot0(dx * dx + dy * dy + dz * dz) << KF_LENGTH_SQUARE_DOWNSHIFT) >= EFFECT_ORBIT_SOUND_RESET_DISTANCE) {
                record->sound_played = KF_AUDIO_NOT_PLAYED;
            }
        }
    } else if ((KF_ENUM_ENCODE(u32, life) & 0xff) != KF_ENUM_ENCODE(u8, KF_EFFECT_HAZARD_RUNNING) && KF_ENUM_ENCODE(s16, phase_limit) >= (int)(KF_ENUM_ENCODE(u32, life) & 0xff)) {
        record->position.vy -= EFFECT_HAZARD_RISE_STEP;
        record->phase++;
    }
}

ADDRESS(0x800384f8, 0x1cc)
void effect_floor_deform_line(s32 segment_index, s32 progress_start, s32 progress_step)
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
            audio_play_spatial_default_range(&gameplay_sound_refs[4],
                &sound_position, KF_AUDIO_MAX_VOLUME);
        }
        map_floor_height_grid.cells[row][col] =
            ((height_delta * progress) >> KF_FIXED12_BITS) + segment->start_height;
        col += segment->column_step;
        row += segment->row_step;
    }
}

/* Kind 10 perturbs world-units-per-update velocity by [-64, 63].
 * The SDK RNG has 15 bits; discard eight and center the remaining range.
 * Stores wrap to 16 bits. The original spread tuning is unresolved. */
enum {
    SCATTER_RANDOM_SHIFT = 8,
    SCATTER_VELOCITY_BIAS = ((RAND_MAX >> SCATTER_RANDOM_SHIFT) + 1) / 2
};

ADDRESS(0x800386c4, 0x68)
void effect_scatter_triple(KfEffectDirectionWords *values)
{
    int random;
    int centered;

    random = rand();
    centered = values->x - SCATTER_VELOCITY_BIAS;
    centered += random >> SCATTER_RANDOM_SHIFT;
    values->x = centered;
    random = rand();
    centered = values->y - SCATTER_VELOCITY_BIAS;
    centered += random >> SCATTER_RANDOM_SHIFT;
    values->y = centered;
    random = rand();
    centered = values->z - SCATTER_VELOCITY_BIAS;
    centered += random >> SCATTER_RANDOM_SHIFT;
    values->z = centered;
}

ADDRESS(0x8003872c, 0x90)
void effect_rotate_scale_offset_y(SVECTOR *offset, VECTOR *out, s16 angle, s32 scale)
{
    SVECTOR scaled;
    SVECTOR rotation;
    MATRIX matrix;

    setVector(&scaled,
        (offset->vx * scale) >> KF_FIXED12_BITS,
        0,
        (offset->vz * scale) >> KF_FIXED12_BITS);
    setVector(&rotation, 0, angle, 0);
    RotMatrix(&rotation, &matrix);
    ApplyMatrix(&matrix, &scaled, out);
}

ADDRESS(0x800387bc, 0xf8)
void effect_spawn_ground_trail(u8 id, KfEffectRecord *record, s16 angle, s32 distance)
{
    VECTOR position;
    s32 index;
    s32 scale = (distance << KF_FIXED12_BITS) / TRAIL_UNIT_SCALE_DISTANCE;

    effect_rotate_scale_offset_y(&record->direction.vector, &position, angle, scale);
    index = record - effect_pool_records;
    position.vx += record->position.vx;
    position.vz += record->position.vz;
    effect_pool_construct(id, record->type, KF_EFFECT_KIND_GROUND_TRAIL, &position,
        &record->direction.vector, KF_EFFECT_ARGS_PARENT(index));
}

ADDRESS(0x800388b4, 0x184)
void effect_spawn_ground_branch(u8 id, KfEffectRecord *record, s16 angle_offset, KF_ENUM_PARAM(KfEffectGroundBranchRole, s32) branch_role)
{
    VECTOR position;
    s32 angle = -(s16)(record->direction.words.y + angle_offset);
    s32 cell_x;
    s32 cell_z;

    position.vx = record->position.vx + (GROUND_BRANCH_CHILD_SPACING * rsin(angle) >> KF_FIXED12_BITS);
    position.vz = record->position.vz + (GROUND_BRANCH_CHILD_SPACING * rcos(angle) >> KF_FIXED12_BITS);
    cell_z = position.vz / KF_MAP_TILE_SIZE;
    cell_x = position.vx / KF_MAP_TILE_SIZE;
    position.vy = -(map_floor_height_grid.cells[cell_z][cell_x] * KF_MAP_HEIGHT_STEP);
    effect_pool_construct(id, record->type, KF_EFFECT_KIND_GROUND_BRANCH, &position,
        &record->direction.vector, KF_EFFECT_ARGS_BRANCH(branch_role));
}
