#include <kf/address.h>
#include <kf/game_map.h>
#include <kf/game_player.h>

/* Psy-Q LIBGTE: SquareRoot0. */

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
    if (point->position.vx == -1) {
        path->frames_remaining = -1;
        return;
    }
    dx = point->position.vx - path->position.vx;
    dy = point->position.vy - path->position.vy;
    dz = point->position.vz - path->position.vz;
    distance = SquareRoot0((dx >> 3) * (dx >> 3) + (dy >> 3) * (dy >> 3) + (dz >> 3) * (dz >> 3))
        << 3;
    path->position_delta.vx = (dx << 4) * point->speed / distance;
    path->position_delta.vy = (dy << 4) * point->speed / distance;
    path->position_delta.vz = (dz << 4) * point->speed / distance;
    path->frames_remaining = distance / point->speed;
    dx = angle_shortest_delta(path->rotation.vx, point->rotation.vx);
    dy = angle_shortest_delta(path->rotation.vy, point->rotation.vy);
    az = angle_shortest_delta(path->rotation.vz, point->rotation.vz);
    path->rotation_delta.vx = (dx << 4) / path->frames_remaining;
    path->rotation_delta.vy = (dy << 4) / path->frames_remaining;
    path->rotation_delta.vz = (az << 4) / path->frames_remaining;
}

ADDRESS(0x800335c0, 0xc0)
void camera_path_begin(KfCameraPathState *path, const KfCameraPathPoint *points)
{
    path->points = points;
    path->position = player_state.camera_position;
    path->rotation = player_state.camera_rotation;
    path->point_index = 0;
    path->position_fixed.vx = path->position.vx << 4;
    path->position_fixed.vy = path->position.vy << 4;
    path->position_fixed.vz = path->position.vz << 4;
    path->rotation_fixed.vx = path->rotation.vx << 4;
    path->rotation_fixed.vy = path->rotation.vy << 4;
    path->rotation_fixed.vz = path->rotation.vz << 4;
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
    path->position_fixed.vx += path->position_delta.vx;
    path->position_fixed.vy += path->position_delta.vy;
    path->position_fixed.vz += path->position_delta.vz;
    path->rotation_fixed.vx += path->rotation_delta.vx;
    path->rotation_fixed.vy += path->rotation_delta.vy;
    path->rotation_fixed.vz += path->rotation_delta.vz;
    setVector(&path->position,
        path->position_fixed.vx >> 4,
        (path->position_fixed.vy >> 4) + y_offset,
        path->position_fixed.vz >> 4);
    setVector(&path->rotation,
        (path->rotation_fixed.vx >> 4) & 0xfff,
        (path->rotation_fixed.vy >> 4) & 0xfff,
        (path->rotation_fixed.vz >> 4) & 0xfff);
}
