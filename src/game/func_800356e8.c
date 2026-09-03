#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

ADDRESS(0x800356e8, 0x20)
void map_event_timers_reset(void)
{
    map_event_animation_gate = 3;
    map_ambient_script_countdown = 10;
}
