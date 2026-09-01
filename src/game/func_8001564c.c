#include <kf/semantic_types.h>

extern KfPlayerVitals player_vitals;
extern void func_80015164(void);

void func_8001564c(s32 delta)
{
    s32 value = player_vitals.current_hp;

    value += delta;

    if (value <= 0) {
        player_vitals.current_hp = 0;
        func_80015164();
        return;
    }
    if (player_vitals.maximum_hp < value) {
        player_vitals.current_hp = player_vitals.maximum_hp;
    } else {
        player_vitals.current_hp = value;
    }
}
