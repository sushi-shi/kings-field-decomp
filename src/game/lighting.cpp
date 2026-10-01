#include <kf/platform/frame_task.hpp>
#include <kf/game/system.h>
#include <kf/game/world.h>
#include <kf/game/party_runtime.h>
#include <kf/game/player.h>
#include <kf/lib/null.h>
#include <kf/game/graphics.h>

#include <kf/lib/math.h>
#include <kf/game/render.h>
#include <kf/platform/prelude.h>
#include <kf/game/game.h>

enum {
    LIGHTING_COLOR_BLEND_STEP = 0x400,
    VITAL_RESTORE_COLOR_LEVEL = 0xfff
};

kf::FrameTask<void> lighting_transition_color_matrix(WorldState &world, PlayerContext &player, const MATRIX *from, const MATRIX *to)
{
    kf::InputContextScope input_context(kf::InputContext::Scripted);
    s32 blend = 0;

    do {
        lighting_set_color_matrix(game_graphics_runtime.render_state, from, to, blend);
        (co_await render_frame(world, player, NULL, NULL));
        blend += LIGHTING_COLOR_BLEND_STEP;
    } while (blend <= KF_FIXED12_ONE);

}

void color_matrix_set_rgb(s16 red, s16 green, s16 blue, MATRIX *matrix)
{
    matrix->m[0][2] = red;
    matrix->m[0][1] = red;
    matrix->m[0][0] = red;
    matrix->m[1][2] = green;
    matrix->m[1][1] = green;
    matrix->m[1][0] = green;
    matrix->m[2][2] = blue;
    matrix->m[2][1] = blue;
    matrix->m[2][0] = blue;
}

kf::FrameTask<void> player_restore_vitals_with_color_cycle(WorldState &world, PlayerContext &player)
{
    if (world.party.enabled) {
        if (!world.prediction) {
            player.state.vitals.current_hp = player.state.vitals.maximum_hp;
            player.state.vitals.current_mp = player.state.vitals.maximum_mp;
            player.state.status_effect_flags &= KF_PLAYER_STATUS_KEEP_UPPER;
            party_revive_spectators(world, player);
        }
        co_return;
    }
    MATRIX saved;
    MATRIX first;
    MATRIX second;

    saved = game_graphics_runtime.render_state.lighting.color_matrix;
    color_matrix_set_rgb(0, VITAL_RESTORE_COLOR_LEVEL, 0, &first);
    (co_await lighting_transition_color_matrix(world, player, &saved, &first));
    color_matrix_set_rgb(
        0, VITAL_RESTORE_COLOR_LEVEL, VITAL_RESTORE_COLOR_LEVEL, &second);
    (co_await lighting_transition_color_matrix(world, player, &first, &second));
    color_matrix_set_rgb(VITAL_RESTORE_COLOR_LEVEL, VITAL_RESTORE_COLOR_LEVEL,
        VITAL_RESTORE_COLOR_LEVEL, &first);
    (co_await lighting_transition_color_matrix(world, player, &second, &first));
    (co_await lighting_transition_color_matrix(world, player, &first, &saved));
    player.state.vitals.current_hp = player.state.vitals.maximum_hp;
    player.state.vitals.current_mp = player.state.vitals.maximum_mp;
    player.state.status_effect_flags &= KF_PLAYER_STATUS_KEEP_UPPER;
}
