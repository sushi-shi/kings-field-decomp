#include <kf/lib/math.h>
#include <kf/open/camera_path.h>
#include <kf/open/opening_helpers.h>

KfCameraPathState opening_camera_path_state;

void opening_camera_path_compute_segment(void)
{
    const KfCameraPathPoint *point;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 distance;
    s16 az;

    opening_camera_path_state.point_index++;
    point = &opening_camera_path_state.points[
        opening_camera_path_state.point_index];
    if (point->position.vx == KF_CAMERA_PATH_END_X) {
        opening_camera_path_state.frames_remaining = KF_CAMERA_PATH_FINISHED;
        return;
    }
    dx = point->position.vx - opening_camera_path_state.position.vx;
    dy = point->position.vy - opening_camera_path_state.position.vy;
    dz = point->position.vz - opening_camera_path_state.position.vz;
    distance = fixed_vector3_length(dx, dy, dz);
    opening_camera_path_state.position_delta = {
        (dx << KF_FIXED4_BITS) * point->speed / distance,
        (dy << KF_FIXED4_BITS) * point->speed / distance,
        (dz << KF_FIXED4_BITS) * point->speed / distance};
    opening_camera_path_state.frames_remaining = distance / point->speed;
    dx = angle_shortest_delta(
        opening_camera_path_state.rotation.vx, point->rotation.vx);
    dy = angle_shortest_delta(
        opening_camera_path_state.rotation.vy, point->rotation.vy);
    az = angle_shortest_delta(
        opening_camera_path_state.rotation.vz, point->rotation.vz);
    opening_camera_path_state.rotation_delta = {
        (dx << KF_FIXED4_BITS) / opening_camera_path_state.frames_remaining,
        (dy << KF_FIXED4_BITS) / opening_camera_path_state.frames_remaining,
        (az << KF_FIXED4_BITS) / opening_camera_path_state.frames_remaining};
}

void opening_camera_path_begin(const KfCameraPathPoint *points)
{
    opening_camera_path_state.points = points;
    opening_camera_path_state.position = points[0].position;
    opening_camera_path_state.rotation = points[0].rotation;
    opening_camera_path_state.point_index = 0;
    camera_path_publish_fixed(&opening_camera_path_state);
    opening_camera_path_compute_segment();
}

void opening_camera_path_step(s32 y_offset)
{
    if (opening_camera_path_state.frames_remaining == KF_CAMERA_PATH_FINISHED) {
        return;
    }
    opening_camera_path_state.frames_remaining--;
    if (opening_camera_path_state.frames_remaining == KF_CAMERA_PATH_FINISHED) {
        opening_camera_path_compute_segment();
        if (opening_camera_path_state.frames_remaining == KF_CAMERA_PATH_FINISHED) {
            return;
        }
    }
    camera_path_advance_pose(&opening_camera_path_state, y_offset);
}


void camera_path_reset_module_state(void)
{
    kf::restore_initial_value<opening_camera_path_state>();
}
