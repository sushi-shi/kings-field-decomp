#include <kf/address.h>
#include <kf/game_collision.h>
#include <kf/game_effect.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

DATA(0x80056268, 0x23)
static KfFloorDeformSegment floor_deform_segments[5] = {
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
    if (effect->type & 0x10) {
        return player_state.magic;
    }
    return 5;
}

ADDRESS(0x80037fe0, 0x2b8)
void effect_projectile_update_3d(SVECTOR *velocity, s32 frame_limit)
{
    KfEffectRecord *record = current_effect;
    KfMagicRecord *magic = current_effect_magic_record;
    u8 life = record->unknown_07;
    MATRIX rotation_matrix;
    MATRIX yaw_matrix;
    VECTOR world;
    u32 collision;
    s16 pitch;
    s16 next_pitch;

    if (life < 2u) {
        RotMatrix((SVECTOR *)&record->rotation_x, &rotation_matrix);
        matrix_set_rotation_x(record->rotation_x, &rotation_matrix);
        matrix_set_rotation_y(record->rotation_y, &yaw_matrix);
        MulMatrix2(&yaw_matrix, &rotation_matrix);
        ApplyMatrix(&rotation_matrix, velocity, &world);
        world.vx += record->position.vx;
        world.vy += record->position.vy;
        world.vz += record->position.vz;
        collision = effect_map_collision(&world, 0x78);
        if (collision != 0xffffffff) {
            if ((collision >> 16) == 0x10) {
                actor_apply_damage(collision & 0xffff, 0, magic->damage_components[0],
                    magic->damage_components[2], magic->damage_components[1],
                    0, 0, 0x1388, record->type);
            } else if ((collision >> 16) == 0x80) {
                player_apply_damage(magic->damage_components[0],
                    magic->damage_components[2], magic->damage_components[1],
                    0, 0, 0, 0x1000, record->id);
            }
            record->direction_x = -record->direction_x;
        }
        if (record->unknown_05 == 0) {
            if (rand() < 8192) {
                record->unknown_05 = audio_play_spatial_range(
                    &magic->sounds[0], &world, 0x7f,
                    0xbb8, 0x36b0);
            }
        }
        if (record->rotation_x >= 512) {
            record->rotation_x = 512;
            record->direction_x = 0;
        } else if (record->rotation_y < -511) {
            record->rotation_x = -512;
            record->direction_x = 0;
        }
        if (record->rotation_x > 0) {
            record->direction_x -= 10;
        } else {
            record->direction_x += 10;
        }
        pitch = record->rotation_x;
        next_pitch = (s16)(record->rotation_x + record->direction_x);
        if ((next_pitch <= 0 && pitch >= 0) || (next_pitch >= 0 && pitch <= 0)) {
            if (life == 1) {
                next_pitch = 0;
                record->unknown_07 = 10;
            } else {
                record->unknown_05 = 0;
            }
        }
        record->rotation_x = next_pitch;
    } else if (life >= 10u && (s16)frame_limit >= life) {
        record->position.vy -= 60;
        record->unknown_07++;
    }
}

ADDRESS(0x80038298, 0x260)
void effect_projectile_update_2d(s32 speed, s32 frame_limit)
{
    KfEffectRecord *record = current_effect;
    KfMagicRecord *magic = current_effect_magic_record;
    u32 life = record->unknown_07;
    u32 collision;

    if ((life & 0xff) < 2) {
        record->position.vx = ((s16)record->direction_x << 8)
            + (rsin((s16)record->unknown_38) * speed >> 12);
        record->position.vz = ((s16)record->direction_z << 8)
            + (rcos((s16)record->unknown_38) * speed >> 12);
        record->position.vy = (s16)record->direction_y
            + (rsin((s16)record->unknown_38 << 1) >> 2);
        record->unknown_38 = (record->unknown_38 + 64) & 0xfff;
        collision = effect_map_collision(&record->position, 0x96);
        if (collision != 0xffffffff) {
            if ((collision >> 16) == 0x10) {
                actor_apply_damage(collision & 0xffff, 0, magic->damage_components[0],
                    magic->damage_components[2], magic->damage_components[1],
                    0, 0, 0x1388, record->type);
            } else if ((collision >> 16) == 0x80) {
                player_apply_damage(magic->damage_components[0],
                    magic->damage_components[2], magic->damage_components[1],
                    0, 0, 0, 0x1000, record->id);
            }
        }
        if (record->unknown_05 == 0) {
            if (rand() < 8192) {
                record->unknown_05 = audio_play_spatial_range(
                    &magic->sounds[0], &record->position,
                    0x7f, 0x1388, 0x36b0);
            }
        } else {
            s32 dx = (record->position.vx - player_state.camera_position.vx) >> 3;
            s32 dy = (record->position.vy - player_state.camera_position.vy) >> 3;
            s32 dz = (record->position.vz - player_state.camera_position.vz) >> 3;
            if ((SquareRoot0(dx * dx + dy * dy + dz * dz) << 3) >= 0x1388) {
                record->unknown_05 = 0;
            }
        }
    } else if ((life & 0xff) != 0 && (s16)frame_limit >= (int)(life & 0xff)) {
        record->position.vy -= 60;
        record->unknown_07++;
    }
}

ADDRESS(0x800384f8, 0x1cc)
void effect_floor_deform_line(s32 segment_index, s32 progress_start, s32 progress_step)
{
    KfFloorDeformSegment *segment = &floor_deform_segments[segment_index];
    int range = progress_step < 0 ? -progress_step : progress_step;
    VECTOR sound_position;
    int col;
    int row;
    int count;
    int height_delta;

    sound_position.vy = -(segment->end_height * 100);
    col = segment->column;
    row = segment->row;
    count = segment->cell_count - 1;
    height_delta = segment->end_height - segment->start_height;
    for (; count != -1; count--) {
        int progress = progress_start;
        progress_start += progress_step;
        if (progress < 0) {
            progress = 0;
        } else if (progress >= 4097) {
            progress = 4096;
        } else if (progress >= 3900 && progress < range + 3900) {
            sound_position.vx = 2000 * (col & 0xff) + 1000;
            sound_position.vz = 2000 * (row & 0xff) + 1000;
            audio_play_spatial_default_range(&gameplay_sound_ref_4,
                &sound_position, 0x7f);
        }
        map_floor_height_grid[row & 0xff][col & 0xff] =
            ((height_delta * progress) >> 12) + segment->start_height;
        col += segment->column_step;
        row += segment->row_step;
    }
}

ADDRESS(0x800386c4, 0x68)
void effect_scatter_triple(u16 *values)
{
    values[0] = values[0] + (rand() >> 8) - 64;
    values[1] = values[1] + (rand() >> 8) - 64;
    values[2] = values[2] + (rand() >> 8) - 64;
}

ADDRESS(0x8003872c, 0x90)
void effect_rotate_scale_offset_y(SVECTOR *offset, VECTOR *out, s16 angle, s32 scale)
{
    SVECTOR scaled;
    SVECTOR rotation;
    MATRIX matrix;

    scaled.vx = (offset->vx * scale) >> 12;
    scaled.vy = 0;
    scaled.vz = (offset->vz * scale) >> 12;
    rotation.vx = 0;
    rotation.vy = angle;
    rotation.vz = 0;
    RotMatrix(&rotation, &matrix);
    ApplyMatrix(&matrix, &scaled, out);
}

ADDRESS(0x800387bc, 0xf8)
void effect_spawn_trail_kind13(u8 id, KfEffectRecord *record, s16 angle, s32 distance)
{
    VECTOR position;
    s32 index;
    s32 scale = (distance << 12) / 800;

    effect_rotate_scale_offset_y((SVECTOR *)&record->direction_x, &position, angle, scale);
    index = record - effect_pool_records;
    position.vx += record->position.vx;
    position.vz += record->position.vz;
    effect_pool_construct(id, record->type, 0x13, &position,
        (SVECTOR *)&record->direction_x, index);
}

ADDRESS(0x800388b4, 0x184)
void effect_spawn_ground_kind6(u8 id, KfEffectRecord *record, s16 angle_offset, s32 arg6)
{
    VECTOR position;
    s32 angle = -(s16)(record->direction_y + angle_offset);
    s32 cell_x;
    s32 cell_z;

    position.vx = record->position.vx + (1500 * rsin(angle) >> 12);
    position.vz = record->position.vz + (1500 * rcos(angle) >> 12);
    cell_z = position.vz / 2000;
    cell_x = position.vx / 2000;
    position.vy = -(map_floor_height_grid[cell_z][cell_x] * 100);
    effect_pool_construct(id, record->type, 6, &position,
        (SVECTOR *)&record->direction_x, arg6);
}
