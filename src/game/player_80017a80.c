#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

ADDRESS(0x80017a80, 0x278)
void player_update_vertical_motion(void)
{
    s32 target;
    s32 view_offset;

    target = -(map_floor_height_grid[player_state.map_cell.z][player_state.map_cell.x] * 100);
    if (player_state.update_state != 0xff) {
        if (player_state.floor_height - target < -3000) {
            if (player_state.equipped_leg_armor_id == 0x26
                && map_cell_attribute_grid[player_state.map_cell.z][player_state.map_cell.x]
                    == 0x5d) {
                goto done;
            }
            player_death_begin();
        } else if (target >= -6999
                   && map_cell_attribute_grid[player_state.map_cell.z][player_state.map_cell.x]
                       == 0x52) {
            player_death_begin();
        }
    }
    switch (player_state.vertical_state) {
    case 0x10:
    falling:
        player_state.floor_height += player_state.vertical_velocity;
        player_state.vertical_velocity += 40;
        if (target + 100 < player_state.floor_height) {
            player_state.floor_height = target;
            player_state.vertical_state = 0;
        }
        break;
    case 0x20:
    jumping:
        player_state.floor_height += player_state.vertical_velocity;
        player_state.vertical_velocity += 5;
        if (player_state.floor_height <= target) {
            player_state.floor_height = target;
            player_state.vertical_state = 0;
        }
        break;
    case 0:
        if (target < player_state.floor_height) {
            player_state.vertical_state = 0x20;
            if ((s16)player_state.motion_state.movement_speed >= 181) {
                player_state.vertical_velocity = -300;
            } else {
                player_state.vertical_velocity = -100;
            }
            goto jumping;
        }
        if (player_state.floor_height < target) {
            player_state.vertical_state = 0x10;
            player_state.vertical_velocity = 0;
            goto falling;
        }
        break;
    }
done:
    view_offset = player_state.view_bob_offset - 1500;
    player_state.camera_position.vy = view_offset + player_state.floor_height;
}
