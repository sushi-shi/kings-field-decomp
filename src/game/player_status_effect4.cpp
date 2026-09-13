#include <kf/game_player.h>

void player_apply_fire_defense_boost(void)
{
    player_state.fire_defense_timer = KF_FIRE_DEFENSE_DURATION_UPDATES;
    player_state.status_effect_flags |= KF_PLAYER_STATUS_FIRE_DEFENSE_BOOST;
}
