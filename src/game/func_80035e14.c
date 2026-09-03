#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

ADDRESS(0x80035e14, 0x30)
void map_unload_floor(void)
{
    pool_release_all();
    audio_close_vab();
    map_world_state_persist();
}
