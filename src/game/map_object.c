#include <kf/address.h>
#include <kf/semantic_types.h>

RODATA(0x80012888, 0x18c)

extern KfMapObjectState map_object_state;
extern KfPlayerState player_state;
extern const SoundRef gameplay_sound_ref_0;
extern const SoundRef gameplay_sound_ref_1;
extern const SoundRef gameplay_sound_ref_3;
extern const SoundRef gameplay_sound_ref_5;
extern const SoundRef gameplay_sound_ref_7;
extern const SoundRef gameplay_sound_ref_11;
extern u8 map_floor_height_grid[100][100];
extern KfEffectRecord DAT_8009d040[];
extern u8 DAT_8009f846;
extern u8 DAT_8009eafc;
extern u8 DAT_8009ddb4[4];
extern u16 map_object_effect_sequence_160;
extern u16 map_object_effect_sequence_170;
extern u16 map_object_effect_sequence_180;

/* Psy-Q LIBC: int rand(void); LIBGTE: rsin, rcos, ApplyMatrix. */
extern s32 rand(void);
extern s32 rsin(s32 angle);
extern s32 rcos(s32 angle);
extern struct KfVec4i *ApplyMatrix(
    struct KfMatrix *matrix, struct KfVec4s *vector, struct KfVec4i *result);
extern void matrix_set_rotation_y(s16 angle, struct KfMatrix *matrix);
extern s32 map_object_distance_to_point(
    const KfMapObject *object, s32 point_x, s32 point_z, s32 max_distance);
/* Effect spawner called with six or seven arguments; declared without a prototype. */
extern KfEffectRecord *func_80036f44();
extern void audio_play_spatial_default_range(
    const SoundRef *sound, const struct KfVec4i *position, s16 volume);
extern void sound_ref_play(const SoundRef *sound, s16 volume);
extern void map_apply_copy_region(u8 region_id);
extern s32 map_object_probe_forward(const KfMapObject *object, u16 yaw);
extern void map_object_mark_collision_edge(const KfMapObject *object, u8 value, u16 yaw);
extern void map_object_pool_trigger_link(u8 link_id);
extern s32 player_distance_to_point(
    s32 point_x, s32 point_y, s32 point_z, s32 max_distance, s32 point_height);

#define MAP_OBJECT_COUNT 190
#define MAP_OBJECT_NONE 0xff
#define MAP_TILE_SIZE 2000

/*
 * Finds the first object from START_INDEX whose interaction radius (plus
 * EXTRA_RADIUS) contains the point; behaviour types 0 and 1 test a point
 * offset 2000 units to the object's left or right.
 */
ADDRESS(0x800315c4, 0x1c0)
s32 map_object_pool_find_interaction_from(s32 start_index, s32 x, s32 z, s32 extra_radius)
{
    KfMapObject *object = &map_object_state.objects[start_index];
    s16 index = start_index;
    KfMapObjectDefinition *definition;
    struct KfVec4s offset;
    struct KfVec4i point;
    struct KfMatrix matrix;

    for (; index < MAP_OBJECT_COUNT; index++, object++) {
        if (object->object_id == MAP_OBJECT_NONE) {
            continue;
        }
        definition = &map_object_state.definitions[object->object_id];
        if (definition->behavior_type == 0) {
            offset.x = -MAP_TILE_SIZE;
            offset.y = 0;
            offset.z = 0x226;
            matrix_set_rotation_y(object->rotation.y, &matrix);
            ApplyMatrix(&matrix, &offset, &point);
            point.x += x;
            point.z += z;
            if (map_object_distance_to_point(
                    object, point.x, point.z, definition->interaction_radius + extra_radius)
                != -1) {
                return index;
            }
        } else if (definition->behavior_type == 1) {
            offset.x = MAP_TILE_SIZE;
            offset.y = 0;
            offset.z = 0x226;
            matrix_set_rotation_y(object->rotation.y, &matrix);
            ApplyMatrix(&matrix, &offset, &point);
            point.x += x;
            point.z += z;
            if (map_object_distance_to_point(
                    object, point.x, point.z, definition->interaction_radius + extra_radius)
                != -1) {
                return index;
            }
        } else if (map_object_distance_to_point(
                       object, x, z, definition->interaction_radius + extra_radius)
                   != -1) {
            return index;
        }
    }
    return -1;
}

ADDRESS(0x80031784, 0x20)
void map_object_start_action_if_idle(KfMapObject *object, u8 action)
{
    if (object->action == 0xff) {
        object->action = action;
        object->action_timer = 0;
    }
}

ADDRESS(0x800317a4, 0x90)
KfMapObject *map_object_effect_pool_acquire(u16 first_index, u16 count, u16 sequence)
{
    KfMapObject *object = &map_object_state.objects[first_index];
    KfMapObject *oldest = 0;
    s32 oldest_age = 0;
    s32 age;

    do {
        if (object->object_id == 0xff) {
            return object;
        }
        age = sequence - object->link.spawn_sequence;
        if (age < 0) {
            age += 0x10000;
        }
        if (oldest_age < age) {
            oldest = object;
            oldest_age = age;
        }
        object++;
    } while (--count != 0);
    return oldest;
}

/*
 * Spawns an effect object of OBJECT_ID at POSITION (raised by Y_OFFSET) in
 * the 170.. or 180.. effect range, then starts its action by id band.
 */
ADDRESS(0x80031834, 0x194)
void map_object_spawn_effect(u8 kind, u8 object_id, const struct KfVec3i *position, s32 y_offset)
{
    u16 *sequence;
    u16 first_index;
    KfMapObject *object;

    if (kind == 0) {
        sequence = &map_object_effect_sequence_180;
        first_index = 180;
    } else {
        first_index = 170;
        sequence = &map_object_effect_sequence_170;
    }
    object = map_object_effect_pool_acquire(first_index, 10, *sequence);
    object->link.spawn_sequence = (*sequence)++;
    object->object_id = object_id;
    object->position_x = position->x;
    object->position_y = y_offset + position->y;
    object->position_z = position->z;
    object->cell_x = object->position_x / MAP_TILE_SIZE;
    object->cell_z = object->position_z / MAP_TILE_SIZE;
    object->rotation.z = 0;
    object->rotation.x = 0;
    object->rotation.y = rand() >> 3;
    object->action = MAP_OBJECT_NONE;
    if (object_id < 43) {
        map_object_start_action_if_idle(object, 0x60);
        object->link.vertical_velocity = 0;
    } else if (object_id < 48) {
        map_object_start_action_if_idle(object, 0x61);
        object->link.vertical_velocity = 0;
    } else if (object_id < 65) {
        map_object_start_action_if_idle(object, 0x62);
        object->link.vertical_velocity = 0;
    }
}

/* Spawns debris object 39 for SOURCE at a random bearing 600 units from POSITION. */
ADDRESS(0x800319c8, 0x18c)
void map_object_spawn_actor_debris(u16 source, const struct KfVec3i *position, s32 y_offset)
{
    KfMapObject *object;
    u16 *sequence;
    u16 angle;

    sequence = &map_object_effect_sequence_160;
    object = map_object_effect_pool_acquire(160, 10, *sequence);
    object->link.spawn_sequence = (*sequence)++;
    object->object_id = 39;
    /* The debris keeps its source in the link id and action parameter bytes. */
    *(u16 *)&object->link.link_id = source;
    angle = (u32)rand() >> 3;
    object->position_x = ((rsin(angle) * 600) >> 12) + position->x;
    object->position_y = y_offset + position->y;
    object->position_z = ((rcos(angle) * 600) >> 12) + position->z;
    object->cell_x = object->position_x / MAP_TILE_SIZE;
    object->cell_z = object->position_z / MAP_TILE_SIZE;
    object->rotation.z = 0;
    object->rotation.x = 0;
    object->rotation.y = rand() >> 3;
    object->action = MAP_OBJECT_NONE;
    map_object_start_action_if_idle(object, 0x62);
    object->link.vertical_velocity = -120;
}

/*
 * Fires LINK_ID: objects in action 11 or 80..82 linked to it drop the link,
 * and idle objects of behaviour types below 8 with that (128..) link start.
 */
ADDRESS(0x80031b54, 0xf0)
void map_object_pool_trigger_link(u8 link_id)
{
    KfMapObject *object = map_object_state.objects;
    u16 count = MAP_OBJECT_COUNT - 1;

    do {
        switch (object->action) {
        case 11:
        case 80:
        case 81:
        case 82:
            if (object->link.link_id == link_id) {
                object->link.link_id = MAP_OBJECT_NONE;
            }
            break;
        default:
            if (map_object_state.definitions[object->object_id].behavior_type < 8
                && !(object->link.link_id < 128) && object->link.link_id == link_id) {
                map_object_start_action_if_idle(
                    object, map_object_state.definitions[object->object_id].behavior_type);
            }
            break;
        }
        object++;
    } while (count-- != 0);
}

/* Drops LINK_ID from every behaviour-type-8 object that carries it. */
ADDRESS(0x80031c44, 0x84)
void map_object_pool_clear_link(u8 link_id)
{
    KfMapObject *object = map_object_state.objects;
    u16 count = MAP_OBJECT_COUNT - 1;
    KfMapObjectDefinition *definitions = map_object_state.definitions;

    do {
        if (definitions[object->object_id].behavior_type >= 8
            && definitions[object->object_id].behavior_type == 8
            && object->link.link_id == link_id) {
            object->link.link_id = MAP_OBJECT_NONE;
        }
        object++;
    } while (count-- != 0);
}

/*
 * Advances every map object's action: swinging and sliding doors, falling
 * and tipping debris, effect emitters keyed on the object id, levers that
 * sweep an effect record's angle and fire their link, and the floor
 * specific stair and bridge events.
 */
ADDRESS(0x80031cc8, 0xc18)
void map_object_pool_update(void)
{
    KfMapObject *object = map_object_state.objects;
    KfMapObject *pair;
    KfEffectRecord *record;
    const SoundRef *sound;
    u8 *counter;
    struct KfVec3s direction;
    struct KfVec3i point;
    s16 count;
    u16 timer;
    u16 elapsed;
    s32 floor;
    s32 tilt;

    for (count = MAP_OBJECT_COUNT; count != 0; object++, count--) {
        if (object->action == MAP_OBJECT_NONE) {
            continue;
        }
        switch (object->action) {
        case 0:
            timer = object->action_timer;
            if (object->link.action_parameter != MAP_OBJECT_NONE) {
                pair = &map_object_state.objects[object->link.action_parameter];
            } else {
                pair = 0;
            }
            object->action_timer++;
            if (timer < 32) {
                object->rotation.y += 32;
                if (pair != 0) {
                    pair->rotation.y -= 32;
                }
                if (timer == 0) {
                    if (object->object_id == 0x77) {
                        sound = &gameplay_sound_ref_1;
                    } else {
                        sound = &gameplay_sound_ref_7;
                    }
                    audio_play_spatial_default_range(
                        sound, (struct KfVec4i *)&object->position_x, 0x7f);
                }
                if (timer == 31) {
                    map_object_mark_collision_edge(object, 1, object->rotation.y - 1024);
                    object->action_timer = 250;
                }
            } else if (timer >= 300) {
                if (timer >= 332) {
                    goto finish;
                }
                if (timer == 300) {
                    if (map_object_probe_forward(object, object->rotation.y - 1024) != -1) {
                        object->action_timer = 300;
                        break;
                    }
                    map_object_mark_collision_edge(object, 0, object->rotation.y - 1024);
                    if (object->object_id == 0x77) {
                        sound = &gameplay_sound_ref_1;
                    } else {
                        sound = &gameplay_sound_ref_7;
                    }
                    audio_play_spatial_default_range(
                        sound, (struct KfVec4i *)&object->position_x, 0x7f);
                }
                object->rotation.y -= 32;
                if (pair != 0) {
                    pair->rotation.y += 32;
                }
            }
            break;
        case 2:
            elapsed = object->action_timer++;
            if (elapsed < 41) {
                object->position_y -= 60;
                if (elapsed == 0) {
                    audio_play_spatial_default_range(
                        &gameplay_sound_ref_0, (struct KfVec4i *)&object->position_x, 0x7f);
                }
                if (elapsed == 40) {
                    map_object_mark_collision_edge(object, 1, object->rotation.y);
                    object->action_timer = 250;
                }
            } else if (elapsed >= 300) {
                if (elapsed >= 341) {
                    goto finish;
                }
                if (elapsed == 300) {
                    if (map_object_probe_forward(object, object->rotation.y) != -1) {
                        object->action_timer = 300;
                        break;
                    }
                    map_object_mark_collision_edge(object, 0, object->rotation.y);
                    audio_play_spatial_default_range(
                        &gameplay_sound_ref_0, (struct KfVec4i *)&object->position_x, 0x7f);
                }
                object->position_y += 60;
            }
            break;
        case 96:
            if (object->action_timer == 0) {
                s32 attribute = map_floor_height_grid[object->cell_z][object->cell_x];

                object->position_y += object->link.vertical_velocity;
                object->link.vertical_velocity += 20;
                if (object->position_y < -(attribute * 100)) {
                    break;
                }
                object->position_y = -(attribute * 100);
                object->link.vertical_velocity = 16;
                object->action_timer = 1;
            } else {
                object->rotation.x += object->link.vertical_velocity;
                object->link.vertical_velocity += 16;
                if (object->rotation.x >= 1024) {
                    object->rotation.x = 1024;
                finish:
                    object->action = MAP_OBJECT_NONE;
                }
            }
            break;
        case 97: {
            s32 attribute = map_floor_height_grid[object->cell_z][object->cell_x];

            object->position_y += 20;
            object->rotation.y = (object->rotation.y + 256) & 0xfff;
            if (object->position_y < -(attribute * 100)) {
                break;
            }
            object->position_y = -(attribute * 100);
            object->action_timer = 1;
            goto finish;
        }
        case 98: {
            s32 attribute = map_floor_height_grid[object->cell_z][object->cell_x];

            object->position_y += object->link.vertical_velocity;
            floor = -(attribute * 100);
            tilt = object->rotation.x;
            if (object->action_timer == 0) {
                object->rotation.x = (tilt + 160) & 0xfff;
            } else {
                object->rotation.x = (tilt - 160) & 0xfff;
            }
            object->link.vertical_velocity += 30;
            if (object->position_y < floor) {
                break;
            }
            object->position_y = floor;
            if (object->link.vertical_velocity < 120) {
                object->rotation.x = 0;
                goto finish;
            }
            object->link.vertical_velocity = -(object->link.vertical_velocity >> 1);
            object->action_timer = object->action_timer == 0;
            break;
        }
        case 80:
            if (object->link.link_id == MAP_OBJECT_NONE) {
                break;
            }
            if (object->action_timer == 0) {
                if (player_distance_to_point(
                        object->position_x, 0xffff, object->position_z, 30000, 0)
                    == -1) {
                    break;
                }
                switch (object->object_id) {
            case 137:
                direction.y = 0;
                direction.x = (rsin(object->rotation.y) * 175u) >> 10;
                direction.z = (-rcos(object->rotation.y) * 175u) >> 10;
                func_80036f44(
                    *(u8 *)&object->link.spawn_sequence,
                    0x23,
                    0xe,
                    &object->position_x,
                    &direction,
                    &object->rotation);
                object->action_timer = (rand() >> 12) + 10;
                break;
            case 124:
                direction.y = 0;
                direction.x = (rsin(object->rotation.y) * 25u) >> 7;
                direction.z = (-rcos(object->rotation.y) * 25u) >> 7;
                point.x = object->position_x;
                point.z = object->position_z;
                point.y = object->position_y - 1400;
                func_80036f44(
                    *(u8 *)&object->link.spawn_sequence,
                    0x23,
                    5,
                    &point,
                    &direction,
                    &object->rotation);
                object->action_timer = (rand() >> 12) + 10;
                break;
            case 125:
                direction.y = 0;
                direction.x = (rsin(object->rotation.y) * 175u) >> 10;
                direction.z = (-rcos(object->rotation.y) * 175u) >> 10;
                point.x = object->position_x;
                point.z = object->position_z;
                point.y = object->position_y + 600;
                func_80036f44(
                    *(u8 *)&object->link.spawn_sequence,
                    0x23,
                    7,
                    &point,
                    &direction,
                    &object->rotation,
                    1);
                object->action_timer = (rand() >> 12) + 10;
                break;
            case 115:
                if (DAT_8009f846 == 0) {
                    break;
                }
                direction.y = 0;
                direction.x = (rsin(object->rotation.y + 1024) * 225u) >> 10;
                direction.z = (-rcos(object->rotation.y + 1024) * 225u) >> 10;
                switch (object->rotation.y) {
                case 0:
                    point.x = object->position_x + 1100;
                    point.z = object->position_z + 1000;
                    break;
                case 0x800:
                    point.x = object->position_x - 1100;
                    point.z = object->position_z - 1000;
                    break;
                }
                point.y = object->position_y - 1000;
                func_80036f44(
                    *(u8 *)&object->link.spawn_sequence,
                    0x23,
                    7,
                    &point,
                    &direction,
                    &object->rotation,
                    rand() < 4096);
                object->action_timer = (rand() >> 12) + 20;
                break;
            }
            } else {
                object->action_timer--;
            }
            break;
        case 81:
        case 82:
            if (object->link.link_id == MAP_OBJECT_NONE && object->action_timer == 0) {
                DAT_8009d040[object->link.action_parameter].unknown_07 = 1;
                object->action_timer = 1;
            }
            break;
        case 83:
            if (object->action_timer == 2) {
                break;
            }
            if (object->link.link_id == MAP_OBJECT_NONE) {
                DAT_8009d040[object->link.action_parameter].unknown_08 = 0xfff;
                object->action_timer = 2;
                break;
            }
            if (object->action_timer == 1) {
                record = &DAT_8009d040[object->link.action_parameter];
                if (record->unknown_08 == 0) {
                    audio_play_spatial_default_range(
                        &gameplay_sound_ref_3, (struct KfVec4i *)&object->position_x, 0x7f);
                }
                record->unknown_08 += 128;
                if (record->unknown_08 >= 4096) {
                    record->unknown_08 = 0xfff;
                    map_object_pool_trigger_link(object->link.link_id);
                    if (object->link.link_id >= 128) {
                        object->action_timer = 3;
                    } else {
                        object->link.link_id = MAP_OBJECT_NONE;
                    }
                }
            } else if (object->action_timer == 3) {
                record = &DAT_8009d040[object->link.action_parameter];
                if (record->unknown_08 == 0xfff) {
                    audio_play_spatial_default_range(
                        &gameplay_sound_ref_3, (struct KfVec4i *)&object->position_x, 0x7f);
                }
                record->unknown_08 -= 128;
                if (record->unknown_08 > 4096) {
                    record->unknown_08 = 0;
                    object->action_timer = 0;
                }
            }
            break;
        case 10:
            if (object->link.link_id == MAP_OBJECT_NONE && object->action_timer == 0) {
                if (object->link.action_parameter != MAP_OBJECT_NONE) {
                    map_apply_copy_region(object->link.action_parameter);
                }
                object->action_timer = 1;
            }
            break;
        case 12:
            if (object->link.link_id != MAP_OBJECT_NONE) {
                break;
            }
            if (object->action_timer == 0) {
                object->position_y -= 10200;
                object->action_timer = 1;
            } else if (object->action_timer < 6) {
                object->position_y += 40;
                object->action_timer++;
            } else if (object->action_timer == 6) {
                if (player_state.progress_state.current_floor == 3) {
                    audio_play_spatial_default_range(
                        &gameplay_sound_ref_11, (struct KfVec4i *)&object->position_x, 0x7f);
                    counter = &DAT_8009eafc;
                    if (*counter != 4) {
                        (*counter)++;
                        if (*counter >= 4) {
                            map_apply_copy_region(2);
                            map_apply_copy_region(3);
                            sound_ref_play(&gameplay_sound_ref_7, 0x7f);
                            *counter = 4;
                        }
                    }
                } else if (player_state.progress_state.current_floor == 1) {
                    if (DAT_8009ddb4[3] == 0) {
                        audio_play_spatial_default_range(
                            &gameplay_sound_ref_5, (struct KfVec4i *)&object->position_x, 0x7f);
                        DAT_8009ddb4[3] = 1;
                    }
                }
                object->action_timer++;
            }
            break;
        case 11:
            if (object->link.link_id == MAP_OBJECT_NONE) {
                object->object_id = 0x7b;
                object->rotation.y = (object->rotation.y + 8) & 0xfff;
            }
            break;
        }
    }
}
