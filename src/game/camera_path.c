#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;

/* Psy-Q LIBGTE: SquareRoot0. */
extern s16 angle_shortest_delta(s32 first, s32 second);
extern void camera_path_compute_segment(KfCameraPathState *path);

/*
 * Advances to the next path point and prepares the Q4 position and
 * wrapped-rotation deltas that reach it at the point's speed; a point with
 * x = -1 ends the path.
 */
ADDRESS(0x800332e4, 0x2dc)
void camera_path_compute_segment(KfCameraPathState *path)
{
    const KfCameraPathPoint *point = &path->points[path->point_index];
    s32 dx;
    s32 dy;
    s32 dz;
    s32 distance;
    s16 az;

    path->point_index++;
    if (point->position.x == -1) {
        path->frames_remaining = -1;
        return;
    }
    dx = point->position.x - path->position.x;
    dy = point->position.y - path->position.y;
    dz = point->position.z - path->position.z;
    distance = SquareRoot0((dx >> 3) * (dx >> 3) + (dy >> 3) * (dy >> 3) + (dz >> 3) * (dz >> 3))
        << 3;
    path->position_delta.x = (dx << 4) * point->speed / distance;
    path->position_delta.y = (dy << 4) * point->speed / distance;
    path->position_delta.z = (dz << 4) * point->speed / distance;
    path->frames_remaining = distance / point->speed;
    dx = angle_shortest_delta(path->rotation.x, point->rotation.x);
    dy = angle_shortest_delta(path->rotation.y, point->rotation.y);
    az = angle_shortest_delta(path->rotation.z, point->rotation.z);
    path->rotation_delta.x = (dx << 4) / path->frames_remaining;
    path->rotation_delta.y = (dy << 4) / path->frames_remaining;
    path->rotation_delta.z = (az << 4) / path->frames_remaining;
}

ADDRESS(0x800335c0, 0xc0)
void camera_path_begin(KfCameraPathState *path, const KfCameraPathPoint *points)
{
    path->points = points;
    path->position = player_state.camera_position;
    path->rotation = player_state.camera_rotation;
    path->point_index = 0;
    path->position_fixed.x = path->position.x << 4;
    path->position_fixed.y = path->position.y << 4;
    path->position_fixed.z = path->position.z << 4;
    path->rotation_fixed.x = path->rotation.x << 4;
    path->rotation_fixed.y = path->rotation.y << 4;
    path->rotation_fixed.z = path->rotation.z << 4;
    camera_path_compute_segment(path);
}

ADDRESS(0x80033680, 0x11c)
void camera_path_step(KfCameraPathState *path, s32 y_offset)
{
    if (path->frames_remaining == -1) {
        return;
    }
    path->frames_remaining--;
    if (path->frames_remaining == -1) {
        camera_path_compute_segment(path);
        if (path->frames_remaining == -1) {
            return;
        }
    }
    path->position_fixed.x += path->position_delta.x;
    path->position_fixed.y += path->position_delta.y;
    path->position_fixed.z += path->position_delta.z;
    path->rotation_fixed.x += path->rotation_delta.x;
    path->rotation_fixed.y += path->rotation_delta.y;
    path->rotation_fixed.z += path->rotation_delta.z;
    path->position.x = path->position_fixed.x >> 4;
    path->position.y = (path->position_fixed.y >> 4) + y_offset;
    path->rotation.y = (path->rotation_fixed.y >> 4) & 0xfff;
    path->position.z = path->position_fixed.z >> 4;
    path->rotation.x = (path->rotation_fixed.x >> 4) & 0xfff;
    path->rotation.z = (path->rotation_fixed.z >> 4) & 0xfff;
}
