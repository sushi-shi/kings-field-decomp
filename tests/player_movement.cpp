#include "../src/game/player_core.cpp"
#include "../src/game/collision.cpp"
#include "../src/lib/fixed_math.cpp"
#include "../src/lib/vector_math.cpp"

#include <cassert>

KfMapCollisionGrid map_collision_grid {};
KfMapGrid map_collision_flag_grid {}, map_floor_height_grid {};
KfMapAttributeGrid map_cell_attribute_grid {};
KfActorState actor_state {};
KfMapRuntimeState map_runtime_state {};
KfMapObjectState map_object_state {};
s32 actor_pool_find_overlap(s32, s32, s32, s32, s32) { return -1; }
s32 map_object_pool_find_near_point(s32, s32, s32) { return -1; }
s32 map_event_pool_find_overlap(s32, s32, s32) { return -1; }
s32 map_floor_height_for_cell_position(u16, s32, s32) { return 0; }

static void place(s32 x, s32 z)
{
    player_state = {};
    player_state.camera_position = {x, 0, z};
    player_state.motion_state.map_cell.x = x / KF_MAP_TILE_SIZE;
    player_state.motion_state.map_cell.z = z / KF_MAP_TILE_SIZE;
    map_collision_grid.cells[z / KF_MAP_TILE_SIZE][x / KF_MAP_TILE_SIZE] = KF_MAP_CELL_X_GE_Z;
}

int main()
{
    struct Movement { s32 x, z, heading; };
    for (const auto move : {Movement{199990, 101000, 3072}, Movement{10, 100005, 1024},
                            Movement{101995, 199990, 0}, Movement{101000, 10, 2048},
                            Movement{199990, 199990, 3584}}) {
        place(move.x, move.z);
        const auto before = player_state.camera_position;
        const auto cell = player_state.motion_state.map_cell;
        // Boundary rejection must not use the stale target from a prior query.
        collision_target.position = {50000, 0, 50000};
        collision_target.radius = 1000;
        player_move_horizontal(move.heading, 100);
        assert(player_state.camera_position.vx == before.vx);
        assert(player_state.camera_position.vz == before.vz);
        assert(player_state.motion_state.map_cell.x == cell.x);
        assert(player_state.motion_state.map_cell.z == cell.z);
    }
    // Interior diagonal correction remains active.
    place(101000, 101000);
    player_move_horizontal(1024, 100);
    assert(player_state.camera_position.vx == 100950);
    assert(player_state.camera_position.vz == 100950);
    // Ordinary movement into an adjacent open cell remains available.
    place(101990, 101000);
    map_collision_grid.cells[50][50] = KF_MAP_CELL_FLOOR;
    map_collision_grid.cells[50][51] = KF_MAP_CELL_FLOOR;
    player_move_horizontal(3072, 100);
    assert(player_state.camera_position.vx == 102090);
    assert(player_state.motion_state.map_cell.x == 51);
}
