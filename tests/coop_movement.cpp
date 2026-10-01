#include <kf/platform/prelude.h>
#include <kf/game/game.h>
#include <kf/game/player.h>
#include <kf/game/world.h>
#include <kf/game/party_runtime.h>

#include <cstdio>
#include <cstdlib>
#include <memory>

static void require(bool value, const char *message)
{
    if (!value) {
        std::fprintf(stderr, "%s\n", message);
        std::abort();
    }
}

struct Fixture {
    std::unique_ptr<WorldState> world = std::make_unique<WorldState>();
    PlayerContext &player = world->party.members[0].player;
    Fixture()
    {
        actor_pool_clear(*world);
        map_object_pool_clear(*world);
        for (auto &event : world->map.events) event.state = KF_MAP_EVENT_FREE;
        for (auto &cell : world->collision.linear) cell = KF_MAP_CELL_FLOOR;
        // Party queries scan bodies even when the occupancy broad phase is empty.
        world->party.enabled = true;
        place(10000, 10000);
    }
    void place(s32 x, s32 z)
    {
        player.state.camera_position = {x, -KF_COLLISION_PLAYER_HEIGHT, z};
        player.state.foot_height = 0;
        player.state.motion_state.map_cell.x = x / KF_MAP_TILE_SIZE;
        player.state.motion_state.map_cell.z = z / KF_MAP_TILE_SIZE;
        player.state.camera_rotation = {37, 1234, -29};
    }
    void obstacle(s32 x, s32 z, u16 radius = 1000)
    {
        auto &event = world->map.events[0];
        event.state = KF_MAP_EVENT_ACTIVE;
        event.reference_position = {x, 0, z};
        event.radius = radius;
    }
    VECTOR move(s32 heading, s32 distance)
    {
        const auto before = player.state.camera_position;
        const auto rotation = player.state.camera_rotation;
        player_move_horizontal(*world, player, heading, distance);
        const auto after = player.state.camera_position;
        const auto dx = std::int64_t(after.vx) - before.vx;
        const auto dz = std::int64_t(after.vz) - before.vz;
        const auto limit = std::int64_t(distance) + 2;
        require(dx * dx + dz * dz <= limit * limit, "Collision accelerated the requested movement");
        require(after.vy == before.vy, "Horizontal movement changed camera height");
        require(player.state.camera_rotation.vx == rotation.vx &&
            player.state.camera_rotation.vy == rotation.vy &&
            player.state.camera_rotation.vz == rotation.vz, "Collision changed camera orientation");
        require(player.state.motion_state.map_cell.x == after.vx / KF_MAP_TILE_SIZE &&
            player.state.motion_state.map_cell.z == after.vz / KF_MAP_TILE_SIZE,
            "Movement left an inconsistent map cell");
        return {static_cast<s32>(dx), 0, static_cast<s32>(dz)};
    }
};

static void unobstructed()
{
    Fixture fixture;
    for (const s32 distance : {1, 10, 40, 300}) {
        for (s32 heading = 0; heading < KF_ANGLE_FULL_TURN; heading += 256) {
            fixture.place(10000, 10000);
            const auto moved = fixture.move(heading, distance);
            require(moved.vx == (-kf::angle_sine(heading) * distance >> KF_FIXED12_BITS) &&
                moved.vz == (kf::angle_cosine(heading) * distance >> KF_FIXED12_BITS),
                "Unobstructed movement changed");
        }
    }
}

static void round_obstacles()
{
    Fixture fixture;
    for (const s32 distance : {1, 10, 40}) {
        fixture.place(10000, 10000);
        fixture.obstacle(10000, 11800 + distance / 2);
        const auto moved = fixture.move(0, distance);
        require(!moved.vx && !moved.vz, "Head-on collision moved the player sideways");
    }
    fixture.place(10000, 10000);
    fixture.obstacle(11288, 11288);
    const auto slide = fixture.move(0, 40);
    require(slide.vx < 0 && slide.vz > 0, "Oblique collision did not slide along the obstacle");
    const auto &event = fixture.world->map.events[0];
    const auto &position = fixture.player.state.camera_position;
    require(map_event_distance_to_point(&event, position.vx, position.vz,
        event.radius + KF_COLLISION_PLAYER_RADIUS) == KF_PROXIMITY_NONE,
        "Sliding penetrated the obstacle");

    // Initial overlap must not eject the player by the obstacle radius.
    for (const s32 offset : {0, 500, 1790}) {
        fixture.place(10000, 10000);
        fixture.obstacle(10000, 10000 + offset);
        const auto moved = fixture.move(0, 10);
        require(!moved.vx && !moved.vz, "An overlapping body caused a positional jump");
    }
    fixture.place(10000, 10000);
    fixture.obstacle(10000, 11790);
    require(fixture.move(KF_ANGLE_HALF_TURN, 40).vz < 0,
        "Player could not retreat out of an initial overlap");

    // The same response must apply without party mode through cell occupancy.
    fixture.world->party.enabled = false;
    for (auto &cell : fixture.world->collision_flags.linear) cell = 1;
    fixture.place(10000, 10000);
    fixture.obstacle(11288, 11288);
    const auto offline = fixture.move(0, 40);
    require(offline.vx == slide.vx && offline.vz == slide.vz,
        "Party mode changed obstacle sliding");
}

static void map_edges_and_terrain()
{
    Fixture fixture;
    constexpr s32 extent_x = KF_MAP_COLUMNS * KF_MAP_TILE_SIZE;
    constexpr s32 extent_z = KF_MAP_ROWS * KF_MAP_TILE_SIZE;
    struct Edge { s32 x, z, heading; };
    for (const auto edge : {Edge{1, 10000, 1024}, Edge{extent_x - 1, 10000, 3072},
            Edge{10000, 1, 2048}, Edge{10000, extent_z - 1, 0}}) {
        for (const auto kind : {KF_MAP_CELL_FLOOR, KF_MAP_CELL_X_GE_Z, KF_MAP_CELL_SUM_LE_SIZE,
                KF_MAP_CELL_Z_GE_X, KF_MAP_CELL_SUM_GE_SIZE}) {
            fixture.place(edge.x, edge.z);
            const auto cell = fixture.player.state.motion_state.map_cell;
            fixture.world->collision.cells[cell.z][cell.x] = kind;
            fixture.world->collision_target = {{50000, 0, 50000}, 5000};
            const auto moved = fixture.move(edge.heading, 40);
            require(!moved.vx && !moved.vz, "Map edge used a stale collision target");
        }
    }
    fixture.place(10000, 11990);
    fixture.world->collision.cells[6][5] = KF_MAP_CELL_BLOCKED;
    require(!fixture.move(0, 40).vz, "Player entered blocked terrain");
    fixture.world->collision.cells[6][5] = KF_MAP_CELL_FLOOR;
    fixture.world->floor_height.cells[6][5] = 10;
    require(!fixture.move(0, 40).vz, "Player climbed an excessive step");
}

static kf::net::Identity identity(u8 value)
{
    kf::net::Identity result {};
    result[0] = value;
    return result;
}

static void prepare_party(Fixture &fixture)
{
    fixture.world->floor = KF_FLOOR_1;
    fixture.place(31000, 4000); // Authored start is on a tile edge, not its center.
    auto &host = fixture.world->party.members[0];
    host.presence = PartyPresence::Living;
    host.connected = true;
    host.player.party_slot = 0;
    host.player.state.vitals.current_hp = host.player.state.vitals.maximum_hp = 100;
    collision_adjust_cell_occupancy(*fixture.world, 15, 2, 1);
    for (u8 slot = 1; slot < party_capacity; ++slot) {
        auto &member = fixture.world->party.members[slot];
        member.presence = PartyPresence::Disconnected;
        member.character_id = identity(slot);
        member.player.party_slot = slot;
        member.player.state.vitals.current_hp = member.player.state.vitals.maximum_hp = 100;
    }
}

static void require_separate(const WorldState &world)
{
    for (u8 first = 0; first < party_capacity; ++first)
        for (u8 second = first + 1; second < party_capacity; ++second) {
            if (!party_member_alive(world.party.members[first]) || !party_member_alive(world.party.members[second])) continue;
            const auto &a = world.party.members[first].player.state.camera_position;
            const auto &b = world.party.members[second].player.state.camera_position;
            const auto dx = std::int64_t(a.vx) - b.vx, dz = std::int64_t(a.vz) - b.vz;
            require(dx * dx + dz * dz > 4 * KF_COLLISION_PLAYER_RADIUS * KF_COLLISION_PLAYER_RADIUS,
                "Nearby party bodies overlap");
        }
}

static void require_occupancy(const WorldState &world, unsigned bodies)
{
    unsigned living = 0;
    for (const auto &member : world.party.members) living += party_member_alive(member);
    require(living == bodies, "Unexpected number of living party bodies");
    for (s32 z = 0; z < KF_MAP_ROWS; ++z) for (s32 x = 0; x < KF_MAP_COLUMNS; ++x) {
        unsigned expected = 0;
        for (const auto &member : world.party.members) {
            const auto cell = member.player.state.motion_state.map_cell;
            expected += party_member_alive(member) && std::abs(x - cell.x) <= KF_OCCUPANCY_CELL_RADIUS &&
                std::abs(z - cell.z) <= KF_OCCUPANCY_CELL_RADIUS;
        }
        require((world.collision_flags.cells[z][x] & KF_CELL_OCCUPANT_COUNT_MASK) == expected,
            "Party placement left missing or duplicate collision occupancy");
    }
}

static void nearby_spawn()
{
    Fixture fixture;
    prepare_party(fixture);
    const auto host_position = fixture.player.state.camera_position;
    const auto heading = fixture.player.state.camera_rotation;
    fixture.obstacle(32700, 4000);
    for (u8 slot = 1; slot < party_capacity; ++slot) {
        require(party_admit(*fixture.world, slot, identity(slot), true), "No nearby party spawn found on open floor");
        const auto &player = fixture.world->party.members[slot].player;
        const auto &position = player.state.camera_position;
        require(position.vx != 32700 || position.vz != 4000, "Party spawned inside a map event");
        require(std::abs(position.vx - host_position.vx) <= 5100 && std::abs(position.vz - host_position.vz) <= 5100,
            "Party admission spawned too far from the host");
        require(player.state.camera_rotation.vx == heading.vx && player.state.camera_rotation.vy == heading.vy &&
            player.state.camera_rotation.vz == heading.vz, "Party spawn changed host-facing direction");
        require_separate(*fixture.world);
    }
    require(fixture.player.state.camera_position.vx == host_position.vx &&
        fixture.player.state.camera_position.vz == host_position.vz, "Guest admission moved the host to tile center");
    require_occupancy(*fixture.world, 4);

    fixture.world->map.events[0].state = KF_MAP_EVENT_FREE;
    party_move_to_floor_entry(*fixture.world);
    require_separate(*fixture.world);
    require_occupancy(*fixture.world, 4);
    party_move_to_floor_entry(*fixture.world);
    require_separate(*fixture.world);
    require_occupancy(*fixture.world, 4);

    Fixture blocked;
    prepare_party(blocked);
    for (auto &cell : blocked.world->collision.linear) cell = KF_MAP_CELL_BLOCKED;
    auto &waiting = blocked.world->party.members[1];
    const auto generation = waiting.generation;
    require(!party_admit(*blocked.world, 1, identity(1), true), "Blocked party admission used an unsafe fallback");
    require(waiting.presence == PartyPresence::Disconnected && waiting.generation == generation,
        "Failed admission changed membership");
    require_occupancy(*blocked.world, 1);
    party_move_to_floor_entry(*blocked.world);
    require(blocked.player.state.camera_position.vz == 4000,
        "Failed group placement partially moved the party");
    require_occupancy(*blocked.world, 1);

    Fixture revived;
    prepare_party(revived);
    auto &spectator = revived.world->party.members[1];
    spectator.presence = PartyPresence::Spectating;
    spectator.connected = true;
    spectator.player.state.vitals.current_hp = 0;
    party_revive_spectators(*revived.world, revived.player);
    require(party_member_alive(spectator), "Nearby revival did not restore one body");
    require_occupancy(*revived.world, 2);
    require_separate(*revived.world);
}

int main()
{
    unobstructed();
    round_obstacles();
    map_edges_and_terrain();
    nearby_spawn();
    std::puts("Bounded movement, obstacle sliding, terrain bounds and nearby party spawning passed");
}
