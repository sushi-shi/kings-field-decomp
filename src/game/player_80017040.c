#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;
extern KfCollisionTarget collision_target;
extern u8 map_collision_grid[100][100];
extern u8 map_floor_height_grid[100][100];
extern s32 vector_xz_to_angle(s32 x, s32 z);
/* Psy-Q LIBGTE: rsin, rcos. */
extern s32 rsin(s32 angle);
extern s32 rcos(s32 angle);
extern u32 collision_query_world(
    s32 point_x, s32 point_y, s32 point_z, s32 radius, s32 height, u32 flags);
extern int angle_mod_delta_le_half_turn(int lhs, int rhs);
/* Psy-Q LIBGTE: long SquareRoot0(long a); */
extern s32 SquareRoot0(s32 value);
extern s32 player_distance_to_point(
    s32 point_x, s32 point_y, s32 point_z, s32 max_distance, s32 point_height);

ADDRESS(0x80017040, 0xc8)
s32 player_distance_to_point_in_cone(
    const struct KfVec3i *point, s16 facing, s32 max_distance, s32 angle_tolerance)
{
    s32 distance;
    s16 delta;

    distance = player_distance_to_point(point->x, 0xffff, point->z, max_distance, 0);
    if (distance != -1) {
        delta = (vector_xz_to_angle(
                     player_state.camera_position.x - point->x,
                     point->z - player_state.camera_position.z)
                 - facing) & 0xfff;
        if (delta > 2048) {
            delta = 0x1000 - delta;
        }
        if (angle_tolerance < delta) {
            distance = -1;
        }
    }
    return distance;
}

/* point_height is reused as the vertical tolerance, as retail keeps it in $a3. */
ADDRESS(0x80017108, 0xf4)
s32 player_distance_to_point(
    s32 point_x, s32 point_y, s32 point_z, s32 max_distance, s32 point_height)
{
    s32 dx;
    s32 dz;
    s32 center;
    s32 dy;
    s32 distance;

    dx = player_state.camera_position.x - point_x;
    if (dx < -max_distance || max_distance < dx) {
        return -1;
    }
    dz = player_state.camera_position.z - point_z;
    if (dz < -max_distance || max_distance < dz) {
        return -1;
    }
    dx >>= 3;
    if (point_y != 0xffff) {
        point_height >>= 1;
        center = point_y - point_height;
        point_height += 850;
        center += 850;
        dy = player_state.floor_height - center;
        if (dy < -point_height || point_height < dy) {
            return -1;
        }
    }
    dz >>= 3;
    distance = SquareRoot0(dx * dx + dz * dz) << 3;
    if (max_distance < distance) {
        return -1;
    }
    return distance;
}

/*
 * Moves the camera by DISTANCE along HEADING: a wall hit slides the target
 * around the collision point once, each axis is accepted only into a
 * passable cell no more than 699 units above the floor, a diagonal cell
 * recentres the position on its wall line, and a blocked corner retries
 * along the wall diagonal. Every exit returns 1.
 */
ADDRESS(0x800171fc, 0x828)
/* The heading arrives already extended; retail never re-extends it. */
s32 player_move_horizontal(s32 heading, s32 distance)
{
    s32 cell_z0 = player_state.map_cell.z;
    s32 cell_x0 = player_state.map_cell.x;
    s32 dz;
    s32 dx;
    s32 new_z;
    s32 new_x;
    s32 angle;
    s32 radius;
    s32 remainder_z;
    s32 remainder_x;
    s32 half;
    s32 step;
    u32 cell_z;
    u32 cell_x;
    s16 delta_x;
    s16 delta_z;
    s16 attempt = 1;
    u8 type;

    dz = (rcos(heading) * distance) >> 12;
    dx = (-rsin(heading) * distance) >> 12;
    new_z = dz + player_state.camera_position.z;
    new_x = dx + player_state.camera_position.x;
    for (;;) {
        if (collision_query_world(new_x, player_state.floor_height, new_z, 800, 1700, 2177) == -1) {
            break;
        }
        delta_x = collision_target.position.x - player_state.camera_position.x;
        delta_z = collision_target.position.z - player_state.camera_position.z;
        angle = vector_xz_to_angle(delta_x, delta_z);
        angle = (angle_mod_delta_le_half_turn(heading, angle) == 0 ? angle + 2112 : angle + 1984)
            & 0xfff;
        radius = collision_target.radius + 900;
        delta_z = (rcos(angle) * radius) >> 12;
        delta_x = (-rsin(angle) * radius) >> 12;
        attempt--;
        new_z = collision_target.position.z + delta_z;
        dz = new_z - player_state.camera_position.z;
        new_x = collision_target.position.x + delta_x;
        dx = new_x - player_state.camera_position.x;
        if (attempt == -1) {
            return 1;
        }
    }
    cell_z = new_z / 2000;
    if (cell_z < 100 && map_collision_grid[cell_z][player_state.map_cell.x] != 0
        && -(map_floor_height_grid[cell_z][player_state.map_cell.x] * 100) - player_state.floor_height
               >= -699) {
        player_state.camera_position.z = new_z;
        player_state.map_cell.z = cell_z;
    }
    cell_x = new_x / 2000;
    if (cell_x < 100 && map_collision_grid[player_state.map_cell.z][cell_x] != 0
        && -(map_floor_height_grid[player_state.map_cell.z][cell_x] * 100) - player_state.floor_height
               >= -699) {
        player_state.camera_position.x = new_x;
        player_state.map_cell.x = cell_x;
    }
    type = map_collision_grid[cell_z0][cell_x0];
    if (type >= 2 && type <= 5) {
        if (player_state.map_cell.x == cell_x0 && player_state.map_cell.z == cell_z0) {
            remainder_z = player_state.camera_position.z % 2000;
            remainder_x = player_state.camera_position.x % 2000;
            if (type == 2) {
                if (remainder_x < remainder_z) {
                    half = (remainder_z - remainder_x) / 2;
                    player_state.camera_position.z -= half;
                    player_state.camera_position.x += half;
                }
            } else if (type == 3) {
                if (remainder_z + remainder_x >= 2001) {
                    half = (remainder_z + remainder_x - 2000) / 2;
                    player_state.camera_position.z -= half;
                    player_state.camera_position.x -= half;
                }
            } else if (type == 4) {
                if (remainder_z < remainder_x) {
                    half = (remainder_x - remainder_z) / 2;
                    player_state.camera_position.z += half;
                    player_state.camera_position.x -= half;
                }
            } else if (type == 5) {
                if (remainder_z + remainder_x < 2000) {
                    half = (2000 - (remainder_z + remainder_x)) / 2;
                    player_state.camera_position.z += half;
                    player_state.camera_position.x += half;
                }
            }
            player_state.map_cell.z = player_state.camera_position.z / 2000;
            player_state.map_cell.x = player_state.camera_position.x / 2000;
        }
        if (map_collision_grid[cell_z][cell_x] == 0) {
            if (dz < 0) {
                dz = -dz;
            }
            if (dx < 0) {
                dx = -dx;
            }
            type = map_collision_grid[player_state.map_cell.z][player_state.map_cell.x];
            if (type == 2) {
                if (dz < dx) {
                    dx = -(distance * 2896) >> 12;
                    dz = dx;
                } else {
                    dx = (distance * 2896) >> 12;
                    dz = dx;
                }
            } else if (type == 3) {
                if (dz < dx) {
                    dz = -(distance * 2896) >> 12;
                    dx = (distance * 2896) >> 12;
                } else {
                    dz = (distance * 2896) >> 12;
                    dx = -(distance * 2896) >> 12;
                }
            } else if (type == 4) {
                if (dx < dz) {
                    dx = -(distance * 2896) >> 12;
                    dz = dx;
                } else {
                    dx = (distance * 2896) >> 12;
                    dz = dx;
                }
            } else if (type == 5) {
                if (dx < dz) {
                    dz = -(distance * 2896) >> 12;
                    dx = (distance * 2896) >> 12;
                } else {
                    dz = (distance * 2896) >> 12;
                    dx = -(distance * 2896) >> 12;
                }
            }
            new_z = dz + player_state.camera_position.z;
            cell_z = new_z / 2000;
            new_x = dx + player_state.camera_position.x;
            cell_x = new_x / 2000;
            if (cell_z < 100 && cell_x < 100 && map_collision_grid[cell_z][cell_x] != 0) {
                player_state.camera_position.z = new_z;
                player_state.camera_position.x = new_x;
                player_state.map_cell.z = cell_z;
                player_state.map_cell.x = cell_x;
            }
        }
    }
    return 1;
}
