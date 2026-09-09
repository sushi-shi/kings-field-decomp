#include <kf/address.h>
#include <kf/game_math.h>
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
    if (point->position.vx == KF_CAMERA_PATH_END_X) {
        path->frames_remaining = KF_CAMERA_PATH_FINISHED;
        return;
    }
    dx = point->position.vx - path->position.vx;
    dy = point->position.vy - path->position.vy;
    dz = point->position.vz - path->position.vz;
    distance = SquareRoot0((dx >> KF_LENGTH_SQUARE_DOWNSHIFT) * (dx >> KF_LENGTH_SQUARE_DOWNSHIFT) + (dy >> KF_LENGTH_SQUARE_DOWNSHIFT) * (dy >> KF_LENGTH_SQUARE_DOWNSHIFT) + (dz >> KF_LENGTH_SQUARE_DOWNSHIFT) * (dz >> KF_LENGTH_SQUARE_DOWNSHIFT))
        << KF_LENGTH_SQUARE_DOWNSHIFT;
    path->position_delta.vx = (dx << KF_FIXED4_BITS) * point->speed / distance;
    path->position_delta.vy = (dy << KF_FIXED4_BITS) * point->speed / distance;
    path->position_delta.vz = (dz << KF_FIXED4_BITS) * point->speed / distance;
    path->frames_remaining = distance / point->speed;
    dx = angle_shortest_delta(path->rotation.vx, point->rotation.vx);
    dy = angle_shortest_delta(path->rotation.vy, point->rotation.vy);
    az = angle_shortest_delta(path->rotation.vz, point->rotation.vz);
    path->rotation_delta.vx = (dx << KF_FIXED4_BITS) / path->frames_remaining;
    path->rotation_delta.vy = (dy << KF_FIXED4_BITS) / path->frames_remaining;
    path->rotation_delta.vz = (az << KF_FIXED4_BITS) / path->frames_remaining;
}

ADDRESS(0x800335c0, 0xc0)
void camera_path_begin(KfCameraPathState *path, const KfCameraPathPoint *points)
{
    path->points = points;
    path->position = player_state.camera_position;
    path->rotation = player_state.camera_rotation;
    path->point_index = 0;
    path->position_fixed.vx = path->position.vx << KF_FIXED4_BITS;
    path->position_fixed.vy = path->position.vy << KF_FIXED4_BITS;
    path->position_fixed.vz = path->position.vz << KF_FIXED4_BITS;
    path->rotation_fixed.vx = path->rotation.vx << KF_FIXED4_BITS;
    path->rotation_fixed.vy = path->rotation.vy << KF_FIXED4_BITS;
    path->rotation_fixed.vz = path->rotation.vz << KF_FIXED4_BITS;
    camera_path_compute_segment(path);
}

ADDRESS(0x80033680, 0x11c)
void camera_path_step(KfCameraPathState *path, s32 y_offset)
{
    if (path->frames_remaining == KF_CAMERA_PATH_FINISHED) {
        return;
    }
    path->frames_remaining--;
    if (path->frames_remaining == KF_CAMERA_PATH_FINISHED) {
        camera_path_compute_segment(path);
        if (path->frames_remaining == KF_CAMERA_PATH_FINISHED) {
            return;
        }
    }
    addVector(&path->position_fixed, &path->position_delta);
    addVector(&path->rotation_fixed, &path->rotation_delta);
    setVector(&path->position,
        path->position_fixed.vx >> KF_FIXED4_BITS,
        (path->position_fixed.vy >> KF_FIXED4_BITS) + y_offset,
        path->position_fixed.vz >> KF_FIXED4_BITS);
    setVector(&path->rotation,
        (path->rotation_fixed.vx >> KF_FIXED4_BITS) & KF_ANGLE_WRAP_MASK,
        (path->rotation_fixed.vy >> KF_FIXED4_BITS) & KF_ANGLE_WRAP_MASK,
        (path->rotation_fixed.vz >> KF_FIXED4_BITS) & KF_ANGLE_WRAP_MASK);
}
