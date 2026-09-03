#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;
/* Entry cell per floor, one-based floor number; stored x first, unlike KfMapCell. */
extern const KfFloorEntryCell floor_entry_cells[5];
extern void player_warp_shimmer(s16 object, void *arg1);
extern void audio_play_current_map_sequence(void);
extern void pool_release_all(void);
extern void map_variant_assets_load(void);
extern void player_sync_position_to_map(void);

ADDRESS(0x80017cf8, 0x144)
void player_warp_to_floor_entry(void)
{
    struct KfVec3i position;
    const KfFloorEntryCell *entry;
    u8 floor;

    position.x = player_state.camera_position.vx;
    position.z = player_state.camera_position.vz;
    position.y = player_state.floor_height;
    player_warp_shimmer(0, &position);
    floor = player_state.progress_state.current_floor;
    entry = &floor_entry_cells[floor - 1];
    player_state.previous_map_cell.x = entry->x;
    player_state.previous_map_cell.z = entry->z;
    player_state.camera_position.vx = player_state.previous_map_cell.x * 2000 + 1000;
    position.x = player_state.camera_position.vx;
    player_state.camera_position.vz = player_state.previous_map_cell.z * 2000 + 1000;
    position.z = player_state.camera_position.vz;
    if (floor == 5 && player_state.map_variant != 1) {
        if (player_state.map_variant == 3) {
            audio_play_current_map_sequence();
        }
        pool_release_all();
        player_state.map_variant = 1;
        map_variant_assets_load();
    }
    player_sync_position_to_map();
    position.y = player_state.floor_height;
    player_warp_shimmer(1, &position);
}
