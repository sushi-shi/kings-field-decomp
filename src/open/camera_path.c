#include <kf/address.h>
#include <kf/game_math.h>
#include <kf/open_camera_path.h>
#include <kf/open_opening_helpers.h>

DATA(0x800757d8, 0x64)
KfCameraPathState opening_camera_path_state;

ADDRESS(0x80013cf4, 0x310)
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
    distance = SquareRoot0(
        (dx >> KF_LENGTH_SQUARE_DOWNSHIFT) * (dx >> KF_LENGTH_SQUARE_DOWNSHIFT) +
        (dy >> KF_LENGTH_SQUARE_DOWNSHIFT) * (dy >> KF_LENGTH_SQUARE_DOWNSHIFT) +
        (dz >> KF_LENGTH_SQUARE_DOWNSHIFT) * (dz >> KF_LENGTH_SQUARE_DOWNSHIFT)) << KF_LENGTH_SQUARE_DOWNSHIFT;
    opening_camera_path_state.position_delta.vx =
        (dx << KF_FIXED4_BITS) * point->speed / distance;
    opening_camera_path_state.position_delta.vy =
        (dy << KF_FIXED4_BITS) * point->speed / distance;
    opening_camera_path_state.position_delta.vz =
        (dz << KF_FIXED4_BITS) * point->speed / distance;
    opening_camera_path_state.frames_remaining = distance / point->speed;
    dx = angle_shortest_delta(
        opening_camera_path_state.rotation.vx, point->rotation.vx);
    dy = angle_shortest_delta(
        opening_camera_path_state.rotation.vy, point->rotation.vy);
    az = angle_shortest_delta(
        opening_camera_path_state.rotation.vz, point->rotation.vz);
    opening_camera_path_state.rotation_delta.vx =
        (dx << KF_FIXED4_BITS) / opening_camera_path_state.frames_remaining;
    opening_camera_path_state.rotation_delta.vy =
        (dy << KF_FIXED4_BITS) / opening_camera_path_state.frames_remaining;
    opening_camera_path_state.rotation_delta.vz =
        (az << KF_FIXED4_BITS) / opening_camera_path_state.frames_remaining;
}

ADDRESS(0x80014004, 0xfc)
void opening_camera_path_begin(const KfCameraPathPoint *points)
{
    opening_camera_path_state.points = points;
    opening_camera_path_state.position = points[0].position;
    opening_camera_path_state.rotation = points[0].rotation;
    opening_camera_path_state.point_index = 0;
    opening_camera_path_state.position_fixed.vx =
        opening_camera_path_state.position.vx << KF_FIXED4_BITS;
    opening_camera_path_state.position_fixed.vy =
        opening_camera_path_state.position.vy << KF_FIXED4_BITS;
    opening_camera_path_state.position_fixed.vz =
        opening_camera_path_state.position.vz << KF_FIXED4_BITS;
    opening_camera_path_state.rotation_fixed.vx =
        opening_camera_path_state.rotation.vx << KF_FIXED4_BITS;
    opening_camera_path_state.rotation_fixed.vy =
        opening_camera_path_state.rotation.vy << KF_FIXED4_BITS;
    opening_camera_path_state.rotation_fixed.vz =
        opening_camera_path_state.rotation.vz << KF_FIXED4_BITS;
    opening_camera_path_compute_segment();
}

ADDRESS(0x80014100, 0x168)
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
    opening_camera_path_state.position_fixed.vx +=
        opening_camera_path_state.position_delta.vx;
    opening_camera_path_state.position_fixed.vy +=
        opening_camera_path_state.position_delta.vy;
    opening_camera_path_state.position_fixed.vz +=
        opening_camera_path_state.position_delta.vz;
    opening_camera_path_state.rotation_fixed.vx +=
        opening_camera_path_state.rotation_delta.vx;
    opening_camera_path_state.rotation_fixed.vy +=
        opening_camera_path_state.rotation_delta.vy;
    opening_camera_path_state.rotation_fixed.vz +=
        opening_camera_path_state.rotation_delta.vz;
    opening_camera_path_state.position.vx =
        opening_camera_path_state.position_fixed.vx >> KF_FIXED4_BITS;
    opening_camera_path_state.position.vy =
        (opening_camera_path_state.position_fixed.vy >> KF_FIXED4_BITS) + y_offset;
    opening_camera_path_state.position.vz =
        opening_camera_path_state.position_fixed.vz >> KF_FIXED4_BITS;
    opening_camera_path_state.rotation.vx =
        (opening_camera_path_state.rotation_fixed.vx >> KF_FIXED4_BITS) & KF_ANGLE_WRAP_MASK;
    opening_camera_path_state.rotation.vy =
        (opening_camera_path_state.rotation_fixed.vy >> KF_FIXED4_BITS) & KF_ANGLE_WRAP_MASK;
    opening_camera_path_state.rotation.vz =
        (opening_camera_path_state.rotation_fixed.vz >> KF_FIXED4_BITS) & KF_ANGLE_WRAP_MASK;
}
