#include <kf/address.h>
#include <kf/semantic_types.h>

ADDRESS(0x8002cc54, 0x10)
void actor_set_rotation(
    KfActor *actor,
    s16 x,
    s16 y,
    s16 z)
{
    actor->rotation.x = x;
    actor->rotation.y = y;
    actor->rotation.z = z;
}
