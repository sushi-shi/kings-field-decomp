#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfMapObject map_object_pool[190];

ADDRESS(0x80031784, 0x20)
void map_object_start_action_if_idle(KfMapObject *object, u8 action)
{
    if (object->action == 0xff) {
        object->action = action;
        object->action_timer = 0;
    }
}

ADDRESS(0x800317a4, 0x90)
KfMapObject *map_object_effect_pool_acquire(u16 first_index, u16 count, u16 sequence)
{
    KfMapObject *object = &map_object_pool[first_index];
    KfMapObject *oldest = 0;
    s32 oldest_age = 0;
    s32 age;

    do {
        if (object->object_id == 0xff) {
            return object;
        }
        age = sequence - object->spawn_sequence;
        if (age < 0) {
            age += 0x10000;
        }
        if (oldest_age < age) {
            oldest = object;
            oldest_age = age;
        }
        object++;
    } while (--count != 0);
    return oldest;
}
