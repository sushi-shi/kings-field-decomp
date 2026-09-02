#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfMapObjectState map_object_state;
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
        age = sequence - object->spawn_sequence;
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
    object->spawn_sequence = (*sequence)++;
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
        object->vertical_velocity = 0;
    } else if (object_id < 48) {
        map_object_start_action_if_idle(object, 0x61);
        object->vertical_velocity = 0;
    } else if (object_id < 65) {
        map_object_start_action_if_idle(object, 0x62);
        object->vertical_velocity = 0;
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
    object->spawn_sequence = (*sequence)++;
    object->object_id = 39;
    /* The debris keeps its source in the link id and action parameter bytes. */
    *(u16 *)&object->link_id = source;
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
    object->vertical_velocity = -120;
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
            if (object->link_id == link_id) {
                object->link_id = MAP_OBJECT_NONE;
            }
            break;
        default:
            if (map_object_state.definitions[object->object_id].behavior_type < 8
                && !(object->link_id < 128) && object->link_id == link_id) {
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
            && object->link_id == link_id) {
            object->link_id = MAP_OBJECT_NONE;
        }
        object++;
    } while (count-- != 0);
}
