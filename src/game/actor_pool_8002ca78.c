#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfActor actor_pool[128];

ADDRESS(0x8002ca78)
KfActor *actor_pool_find_free(void)
{
    KfActor *actor = actor_pool;
    s32 count = 127;

    do {
        if (actor->slot_state == 0xff) {
            return actor;
        }
        actor++;
    } while (count-- != 0);
    return 0;
}
