#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;
/* Entry cell per floor, one-based floor number; stored x first, unlike KfMapCell. */
extern const KfFloorEntryCell floor_entry_cells[5];
extern void func_80036618(s16 object, void *arg1);
extern void func_8001b49c(void);
extern void func_80020a2c(void);
extern void func_8001b414(void);
extern void player_sync_position_to_map(void);

ADDRESS(0x80017cf8, 0x144)
void player_warp_to_floor_entry(void)
{
    struct KfVec3i position;
    const KfFloorEntryCell *entry;
    u8 floor;

    position.x = player_state.camera_position.x;
    position.z = player_state.camera_position.z;
    position.y = player_state.floor_height;
    func_80036618(0, &position);
    floor = player_state.progress_state.current_floor;
    entry = &floor_entry_cells[floor - 1];
    player_state.previous_map_cell.x = entry->x;
    player_state.previous_map_cell.z = entry->z;
    player_state.camera_position.x = player_state.previous_map_cell.x * 2000 + 1000;
    position.x = player_state.camera_position.x;
    player_state.camera_position.z = player_state.previous_map_cell.z * 2000 + 1000;
    position.z = player_state.camera_position.z;
    if (floor == 5 && player_state.unknown_0c != 1) {
        if (player_state.unknown_0c == 3) {
            func_8001b49c();
            func_80020a2c();
        }
        player_state.unknown_0c = 1;
        func_8001b414();
    }
    player_sync_position_to_map();
    position.y = player_state.floor_height;
    func_80036618(1, &position);
}
