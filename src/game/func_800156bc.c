#include <kf/semantic_types.h>

extern KfPlayerVitals player_vitals;

void func_800156bc(s32 delta)
{
    s32 value = player_vitals.current_mp;

    value += delta;

    if (value <= 0) {
        player_vitals.current_mp = 0;
        return;
    }
    if (player_vitals.maximum_mp < value) {
        player_vitals.current_mp = player_vitals.maximum_mp;
    } else {
        player_vitals.current_mp = value;
    }
}
