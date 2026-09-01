#include <kf/semantic_types.h>

extern KfActor actor_pool[128];

void func_8002ce88(void)
{
    KfActor *actor = actor_pool;
    u16 index;

    for (index = 0; index < 128; index++, actor++) {
        actor->slot_state = 0xff;
        actor->lifecycle = 0;
        actor->unknown_34 = 0;
    }
}
