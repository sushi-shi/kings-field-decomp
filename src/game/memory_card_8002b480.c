#include <kf/address.h>
#include <kf/game_types.h>

extern s32 memory_card_io_end_event;
extern s32 memory_card_timeout_event;
extern s32 memory_card_new_device_event;
extern s32 memory_card_error_event;
extern s32 UnDeliverEvent(s32 event);

ADDRESS(0x8002b480)
void memory_card_undeliver_events(void)
{
    UnDeliverEvent(memory_card_io_end_event);
    UnDeliverEvent(memory_card_timeout_event);
    UnDeliverEvent(memory_card_new_device_event);
    UnDeliverEvent(memory_card_error_event);
}
