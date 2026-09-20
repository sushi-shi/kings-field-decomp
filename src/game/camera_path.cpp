#include <kf/lib/math.h>
#include <kf/lib/map.h>
#include <kf/game/player.h>

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
    distance = fixed_vector3_length(dx, dy, dz);
    vector_set_xyz(path->position_delta,
        (dx << KF_FIXED4_BITS) * point->speed / distance,
        (dy << KF_FIXED4_BITS) * point->speed / distance,
        (dz << KF_FIXED4_BITS) * point->speed / distance);
    path->frames_remaining = distance / point->speed;
    dx = angle_shortest_delta(path->rotation.vx, point->rotation.vx);
    dy = angle_shortest_delta(path->rotation.vy, point->rotation.vy);
    az = angle_shortest_delta(path->rotation.vz, point->rotation.vz);
    vector_set_xyz(path->rotation_delta,
        (dx << KF_FIXED4_BITS) / path->frames_remaining,
        (dy << KF_FIXED4_BITS) / path->frames_remaining,
        (az << KF_FIXED4_BITS) / path->frames_remaining);
}

void camera_path_begin(KfCameraPathState *path, const KfCameraPathPoint *points)
{
    path->points = points;
    path->position = player_state.camera_position;
    path->rotation = player_state.camera_rotation;
    path->point_index = 0;
    camera_path_publish_fixed(path);
    camera_path_compute_segment(path);
}

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
    camera_path_advance_pose(path, y_offset);
}
