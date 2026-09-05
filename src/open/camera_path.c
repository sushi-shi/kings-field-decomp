#include <kf/address.h>
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
    if (point->position.vx == -1) {
        opening_camera_path_state.frames_remaining = -1;
        return;
    }
    dx = point->position.vx - opening_camera_path_state.position.vx;
    dy = point->position.vy - opening_camera_path_state.position.vy;
    dz = point->position.vz - opening_camera_path_state.position.vz;
    distance = SquareRoot0(
        (dx >> 3) * (dx >> 3) +
        (dy >> 3) * (dy >> 3) +
        (dz >> 3) * (dz >> 3)) << 3;
    opening_camera_path_state.position_delta.vx =
        (dx << 4) * point->speed / distance;
    opening_camera_path_state.position_delta.vy =
        (dy << 4) * point->speed / distance;
    opening_camera_path_state.position_delta.vz =
        (dz << 4) * point->speed / distance;
    opening_camera_path_state.frames_remaining = distance / point->speed;
    dx = angle_shortest_delta(
        opening_camera_path_state.rotation.vx, point->rotation.vx);
    dy = angle_shortest_delta(
        opening_camera_path_state.rotation.vy, point->rotation.vy);
    az = angle_shortest_delta(
        opening_camera_path_state.rotation.vz, point->rotation.vz);
    opening_camera_path_state.rotation_delta.vx =
        (dx << 4) / opening_camera_path_state.frames_remaining;
    opening_camera_path_state.rotation_delta.vy =
        (dy << 4) / opening_camera_path_state.frames_remaining;
    opening_camera_path_state.rotation_delta.vz =
        (az << 4) / opening_camera_path_state.frames_remaining;
}

ADDRESS(0x80014004, 0xfc)
void opening_camera_path_begin(const KfCameraPathPoint *points)
{
    opening_camera_path_state.points = points;
    opening_camera_path_state.position = points[0].position;
    opening_camera_path_state.rotation = points[0].rotation;
    opening_camera_path_state.point_index = 0;
    opening_camera_path_state.position_fixed.vx =
        opening_camera_path_state.position.vx << 4;
    opening_camera_path_state.position_fixed.vy =
        opening_camera_path_state.position.vy << 4;
    opening_camera_path_state.position_fixed.vz =
        opening_camera_path_state.position.vz << 4;
    opening_camera_path_state.rotation_fixed.vx =
        opening_camera_path_state.rotation.vx << 4;
    opening_camera_path_state.rotation_fixed.vy =
        opening_camera_path_state.rotation.vy << 4;
    opening_camera_path_state.rotation_fixed.vz =
        opening_camera_path_state.rotation.vz << 4;
    opening_camera_path_compute_segment();
}

ADDRESS(0x80014100, 0x168)
void opening_camera_path_step(s32 y_offset)
{
    if (opening_camera_path_state.frames_remaining == -1) {
        return;
    }
    opening_camera_path_state.frames_remaining--;
    if (opening_camera_path_state.frames_remaining == -1) {
        opening_camera_path_compute_segment();
        if (opening_camera_path_state.frames_remaining == -1) {
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
        opening_camera_path_state.position_fixed.vx >> 4;
    opening_camera_path_state.position.vy =
        (opening_camera_path_state.position_fixed.vy >> 4) + y_offset;
    opening_camera_path_state.position.vz =
        opening_camera_path_state.position_fixed.vz >> 4;
    opening_camera_path_state.rotation.vx =
        (opening_camera_path_state.rotation_fixed.vx >> 4) & 0xfff;
    opening_camera_path_state.rotation.vy =
        (opening_camera_path_state.rotation_fixed.vy >> 4) & 0xfff;
    opening_camera_path_state.rotation.vz =
        (opening_camera_path_state.rotation_fixed.vz >> 4) & 0xfff;
}
