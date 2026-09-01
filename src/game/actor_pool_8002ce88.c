#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfActor actor_pool[128];

ADDRESS(0x8002ce88)
void actor_pool_clear(void)
{
    KfActor *actor = actor_pool;
    u16 index;

    for (index = 0; index < 128; index++, actor++) {
        actor->slot_state = 0xff;
        actor->lifecycle = 0;
        actor->unknown_34 = 0;
    }
}

ADDRESS(0x8002cec8)
void actor_set_action(KfActor *actor, u8 action)
{
    actor->action = action;
    actor->action_timer = 0;
}
