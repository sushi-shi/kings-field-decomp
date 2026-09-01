#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerVitals player_vitals;
extern void player_death_begin(void);

ADDRESS(0x8001564c)
void player_adjust_hp(s32 delta)
{
    s32 value = player_vitals.current_hp;

    value += delta;

    if (value <= 0) {
        player_vitals.current_hp = 0;
        player_death_begin();
        return;
    }
    if (player_vitals.maximum_hp < value) {
        player_vitals.current_hp = player_vitals.maximum_hp;
    } else {
        player_vitals.current_hp = value;
    }
}

ADDRESS(0x800156bc)
void player_adjust_mp(s32 delta)
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
