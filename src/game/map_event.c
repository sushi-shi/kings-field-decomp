#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfMapEvent *current_map_event;

ADDRESS(0x8003379c)
void map_event_set_current(KfMapEvent *event)
{
    current_map_event = event;
}
