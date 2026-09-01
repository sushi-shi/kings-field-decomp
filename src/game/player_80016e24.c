#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;
extern void *func_8001ac0c(s32 size);
extern void game_state_initialize(void);

ADDRESS(0x80016e24, 0x94)
void game_initialize_session(void)
{
    player_state.camera_rotation.z = 0;
    player_state.camera_rotation.y = 0;
    player_state.camera_rotation.x = 0;
    player_state.camera_position.x = 0x7918;
    player_state.camera_position.y = 0;
    player_state.camera_position.z = 0xfa0;
    player_state.weapon_asset_buffer = func_8001ac0c(0xc000);
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
