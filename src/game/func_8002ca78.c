#include <kf/semantic_types.h>

extern KfActor actor_pool[128];

KfActor *func_8002ca78(void)
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
