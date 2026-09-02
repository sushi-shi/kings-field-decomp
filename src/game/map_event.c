#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfMapEvent *current_map_event;
extern KfMapEvent map_event_pool[8];
extern KfPlayerProgressState player_progress_state;
extern u8 map_floor_height_grid[100][100];
extern void func_8001fde4(s32 first, s32 second);
extern void frame_pacer_wait(void);
extern s32 SquareRoot0(s32 value);
/* Declared with an int result here: retail uses the returned angle unmasked. */
extern s32 vector_xz_to_angle(s32 x, s32 z);
/* Called with the cell pair only; its third candidate parameter is not passed here. */
extern void collision_adjust_cell_occupancy(s32 cell_x, s32 cell_z);

extern s32 map_event_distance_to_point(
    const KfMapEvent *event, s32 point_x, s32 point_z, s32 max_distance);

ADDRESS(0x8003379c, 0x10)
void map_event_set_current(KfMapEvent *event)
{
    current_map_event = event;
}

ADDRESS(0x800337ac, 0x74)
void map_event_refresh_image_for_progress(KfMapEvent *event)
{
    if (event->image_index < event->image_limit) {
        if (player_progress_state.highest_floor < event->image_limit) {
            if (event->image_index != player_progress_state.highest_floor) {
                event->image_index = player_progress_state.highest_floor;
                event->image_dirty = 1;
                event->image_delay = 0;
            }
        } else if (event->image_index != event->image_limit) {
            event->image_index = event->image_limit;
            event->image_dirty = 1;
            event->image_delay = 0;
        }
    }
}

ADDRESS(0x80033820, 0x98)
void map_event_advance_rotation_blocking(KfMapEvent *event, u16 target, s16 step)
{
    while (event->rotation_phase < target) {
        event->rotation_phase += step;
        func_8001fde4(0, 0);
        frame_pacer_wait();
    }
    event->rotation_phase = target;
    func_8001fde4(0, 0);
}

ADDRESS(0x800338b8, 0x22c)
void map_event_pool_load(const KfMapEventDefinition *definitions)
{
    KfMapEvent *event = map_event_pool;
    u8 exhausted = 0;
    u16 count = 7;

    do {
        if (exhausted == 1) {
            event->state = 0xff;
        } else {
            event->state = definitions->state;
            if (event->state == 0xff) {
                exhausted = 1;
                event->state = 0xff;
            } else {
                event->kind = definitions->kind;
                event->variant = definitions->variant;
                event->tag = definitions->tag;
                event->image_limit = definitions->image_limit;
                event->unknown_0c = definitions->unknown_0b;
                event->unknown_0d = definitions->unknown_0c;
                event->unknown_0e = definitions->unknown_0d;
                event->position_x = definitions->cell_x * 2000 + definitions->position_x_offset;
                event->reference_x = event->position_x;
                event->position_z = definitions->cell_z * 2000 + definitions->position_z_offset;
                event->reference_z = event->position_z;
                event->cell_x = definitions->cell_x;
                event->cell_z = definitions->cell_z;
                event->radius = definitions->radius;
                event->position_y =
                    -(map_floor_height_grid[event->cell_z][event->cell_x] * 100);
                definitions++;
                event->rotation = definitions[-1].initial_rotation;
                event->unknown_38 = 0;
                event->unknown_34 = 0;
                event->image_dirty = 1;
                event->image_index = 1;
                event->image_delay = 0;
                event->unknown_0f = 0;
                event->rotation_phase = 0;
                event->rotation_target = 0;
                event->unknown_10 = 0;
                collision_adjust_cell_occupancy(event->cell_x, event->cell_z);
            }
        }
        event++;
    } while (count-- != 0);
}

ADDRESS(0x80033ae4, 0xa8)
s32 map_event_distance_to_point(
    const KfMapEvent *event, s32 point_x, s32 point_z, s32 max_distance)
{
    s32 delta_x = event->reference_x - point_x;
    s32 delta_z;
    s32 distance;

    if (delta_x >= -max_distance && delta_x <= max_distance) {
        delta_z = event->reference_z - point_z;
        if (delta_z >= -max_distance && delta_z <= max_distance) {
            delta_x >>= 3;
            delta_z >>= 3;
            distance = SquareRoot0(delta_x * delta_x + delta_z * delta_z) << 3;
            if (distance <= max_distance) {
                return distance;
            }
        }
    }
    return -1;
}

ADDRESS(0x80033b8c, 0x144)
KfMapEvent *map_event_pool_find_target_in_cone(
    const struct KfVec3i *origin,
    s16 facing,
    s32 max_distance,
    s32 angle_tolerance,
    s32 *distance_out)
{
    KfMapEvent *event = map_event_pool;
    KfMapEvent *found = 0;
    s16 best_angle = 30000;
    s32 found_distance = 0;
    u16 count = 7;
    s32 distance;
    s32 angle;
    s16 delta;

    do {
        if (event->state == 1) {
            distance = map_event_distance_to_point(event, origin->x, origin->z, max_distance);
            if (distance != -1) {
                angle = vector_xz_to_angle(
                    event->reference_x - origin->x, origin->z - event->reference_z);
                angle = (angle - facing) & 0xfff;
                if (angle >= 2049) {
                    angle = 4096 - angle;
                }
                delta = angle;
                if (delta <= angle_tolerance && delta < best_angle) {
                    best_angle = angle;
                    found = event;
                    found_distance = distance;
                }
            }
        }
        event++;
    } while (count-- != 0);
    *distance_out = found_distance;
    return found;
}

ADDRESS(0x80033cd0, 0xb0)
s32 map_event_pool_find_overlap(s32 point_x, s32 point_z, s32 radius_padding)
{
    KfMapEvent *event = map_event_pool;
    s16 index = 0;

    do {
        if (event->state == 1
            && map_event_distance_to_point(
                   event, point_x, point_z, event->radius + radius_padding) != -1) {
            return index;
        }
        index++;
        event++;
    } while (index < 8);
    return -1;
}
