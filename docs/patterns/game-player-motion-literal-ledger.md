# Player motion literal ledger

Complete remainder for `player_sync_position_to_map`, `player_update_view_bob`
and `player_update_vertical_motion` in `src/game/player_core.c`: **7 retained
occurrences**. Named definitions and retail claims are excluded. Bob has no
remaining numeric literals. The other player-core functions and initializers
are covered by the [core remainder ledger](game-player-core-literal-ledger.md).
See the [state and units review](game-player-vertical-state.md).

| Function | Line | Token | Expression | Reason |
| --- | --- | --- | --- | --- |
| `player_sync_position_to_map` | 253 | `0` | `player_state.equipment_effect_ticks = 0;` | Restart the equipment-effect timer when synchronizing to the map; zero is the initial counter value, not a vertical-state encoding. |
| `player_sync_position_to_map` | 257 | `1` | `player_state.allow_near_actor_spawn = 1;` | Enable the existing near-actor-spawn boolean after relocating to a map cell; the main loop clears it after the actor sweep. |
| `player_sync_position_to_map` | 263 | `1` | `collision_adjust_cell_occupancy(player_state.map_cell.x, player_state.map_cell.z, 1);` | Add one occupant to the destination cell. This is a signed occupancy delta, paired with decrements at movement/warp callers, not a state identifier. |
| `player_sync_position_to_map` | 266 | `0` | `player_state.vertical_velocity = 0;` | Reset signed vertical velocity to rest during map synchronization or entry to falling. Landing only resets state and foot height; it does not clear this field. |
| `player_update_vertical_motion` | 531 | `6999` | `} else if (target >= -6999` | Signed world-Y cutoff for the separate attribute-0x52 death rule. Sampled target heights are multiples of 100, so this admits targets -6900 or greater and excludes -7000. Keep the exact retail comparison until the associated environmental rule is identified. |
| `player_update_vertical_motion` | 533 | `0x52` | `== 0x52) {` | Map attribute 82 selects the height-gated death path. Its environmental identity is not established by this consumer; do not label it lava, water or another pit without resource/interaction evidence. |
| `player_update_vertical_motion` | 568 | `0` | `player_state.vertical_velocity = 0;` | Reset signed vertical velocity to rest during map synchronization or entry to falling. Landing only resets state and foot height; it does not clear this field. |
