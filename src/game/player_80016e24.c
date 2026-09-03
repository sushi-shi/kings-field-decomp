#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

ADDRESS(0x80016e24, 0x94)
void game_initialize_session(void)
{
    player_state.camera_rotation.vz = 0;
    player_state.camera_rotation.vy = 0;
    player_state.camera_rotation.vx = 0;
    player_state.camera_position.vx = 0x7918;
    player_state.camera_position.vy = 0;
    player_state.camera_position.vz = 0xfa0;
    player_state.weapon_asset_buffer = memory_allocate(0xc000);
    game_state_initialize();
    player_state.update_state = 0;
    player_state.audio_effects_enabled = 1;
    player_state.audio_music_enabled = 1;
    player_state.unknown_98[0] = 1;
    player_state.unknown_98[1] = 1;
}

ADDRESS(0x80016eb8, 0x30)
void player_clear_motion(void)
{
    player_state.motion_state.yaw_step = 0;
    player_state.motion_state.pitch_step = 0;
    player_state.motion_state.movement_speed = 0;
    player_state.motion_state.forward_velocity = 0;
    player_state.motion_state.strafe_velocity = 0;
}
