#include <kf/game/system.h>
#include <kf/game/world.h>
#include <kf/lib/null.h>
#include <kf/game/graphics.h>

#include <kf/lib/math.h>
#include <kf/game/player.h>
#include <kf/game/game.h>

enum {
    PLAYER_DEATH_BOB_THRESHOLD = 1000,
    PLAYER_DEATH_BOB_REST = 1060,
    PLAYER_DEATH_INITIAL_PITCH_STEP = 10,
    PLAYER_DEATH_REST_PITCH_ACCELERATION = 10,
    PLAYER_DEATH_FALL_ACCELERATION = 15,
    PLAYER_DEATH_PITCH_MIN = -800,
    PLAYER_DEATH_FADE_STEP = 100
};

void player_death_apply_visual_fade(PlayerContext &player, const MATRIX *color_from, s32 blend)
{
    lighting_set_color_matrix(game_graphics_runtime.render_state, color_from, &color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_BLACK)], blend);
    matrix_interpolate(&color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_WHITE)],
        &color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_BLACK)], &game_graphics_runtime.hud_model_color_matrix, blend);
    fog_interpolate_near(game_graphics_runtime.render_state, player.presentation.death_saved_fog_near, 0, blend);
    game_graphics_runtime.hud_brightness = ((blend * -KF_HUD_DEFAULT_BRIGHTNESS) >> KF_FIXED12_BITS)
        + KF_HUD_DEFAULT_BRIGHTNESS;
}

kf::FrameTask<void> player_death_update(WorldState &world, PlayerContext &player)
{
    s16 *bob = &player.state.view_bob_offset;
    s16 previous = *bob;
    s32 blend;
    s32 camera_y;

    if (previous >= PLAYER_DEATH_BOB_THRESHOLD) {
        *bob = PLAYER_DEATH_BOB_THRESHOLD;
        player.state.camera_rotation.vx -= player.state.death_camera_pitch_step;
        *bob = PLAYER_DEATH_BOB_REST;
        player.state.death_camera_pitch_step += PLAYER_DEATH_REST_PITCH_ACCELERATION;
    } else {
        player.state.camera_rotation.vx -= PLAYER_DEATH_INITIAL_PITCH_STEP;
        player.state.death_camera_pitch_step += PLAYER_DEATH_FALL_ACCELERATION;
        *bob = previous + player.state.death_camera_pitch_step;
        if (*bob >= PLAYER_DEATH_BOB_THRESHOLD) {
            player.state.death_camera_pitch_step = PLAYER_DEATH_INITIAL_PITCH_STEP;
        }
    }
    if (player.state.camera_rotation.vx < PLAYER_DEATH_PITCH_MIN) {
        player.state.camera_rotation.vx = PLAYER_DEATH_PITCH_MIN;
        player.state.death_camera_pitch_step = 0;
    }
    camera_y = player.state.view_bob_offset - KF_PLAYER_CAMERA_HEIGHT;
    camera_y += player.state.foot_height;
    player.state.camera_position.vy = camera_y;
    player_update_vertical_motion(world, player);
    player.state.death_visual_blend += PLAYER_DEATH_FADE_STEP;
    blend = player.state.death_visual_blend;
    if (blend >= KF_FIXED12_ONE) {
        player_death_apply_visual_fade(player, &player.presentation.death_saved_color_matrix, KF_FIXED12_ONE);
        (co_await render_frame(world, player, NULL, NULL));
        (co_await render_frame(world, player, NULL, NULL));
        (co_await player_death_restart(world, player));
    } else {
        player_death_apply_visual_fade(player, &player.presentation.death_saved_color_matrix, blend);
    }
}

void player_death_update_reverse_fade(PlayerContext &player)
{
    s16 *blend = &player.state.death_visual_blend;

    lighting_set_color_matrix(game_graphics_runtime.render_state, &color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_BLACK)],
        &color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_DEFAULT)], *blend);
    matrix_interpolate(&color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_BLACK)],
        &color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_WHITE)], &game_graphics_runtime.hud_model_color_matrix, *blend);
    fog_interpolate_near(game_graphics_runtime.render_state, 0, player.presentation.death_saved_fog_near, *blend);
    game_graphics_runtime.hud_brightness = (*blend * KF_HUD_DEFAULT_BRIGHTNESS) >> KF_FIXED12_BITS;
    *blend += PLAYER_DEATH_FADE_STEP;
    if (*blend >= KF_FIXED12_ONE) {
        player_death_apply_visual_fade(player, &color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_DEFAULT)], 0);
        player.state.update_state = KF_PLAYER_UPDATE_NORMAL;
    } else {
        player_death_apply_visual_fade(player,
            &color_matrix_table[kf_enum_encode<s32>(KF_GAME_COLOR_DEFAULT)], KF_FIXED12_ONE - *blend);
    }
}
