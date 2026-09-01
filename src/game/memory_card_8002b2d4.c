#include <kf/address.h>
#include <kf/game_types.h>

extern s32 memory_card_io_end_event;
extern s32 memory_card_timeout_event;
extern s32 memory_card_new_device_event;
extern s32 memory_card_error_event;
extern void StopCARD2(void);
extern s32 CloseEvent(s32 event);

extern void memory_card_clear_events(void);
extern s32 _card_info(s32 channel);
extern s32 memory_card_wait_event(void);

extern s32 TestEvent(s32 event);

ADDRESS(0x8002b2d4)
void memory_card_shutdown_events(void)
{
    StopCARD2();
    CloseEvent(memory_card_io_end_event);
    CloseEvent(memory_card_timeout_event);
    CloseEvent(memory_card_new_device_event);
    CloseEvent(memory_card_error_event);
}

ADDRESS(0x8002b334)
s32 memory_card_begin_status_check(void)
{
    memory_card_clear_events();
    if (_card_info(0) != 0) {
        return memory_card_wait_event();
    }
    return 0;
}

ADDRESS(0x8002b36c)
void memory_card_clear_events(void)
{
    TestEvent(memory_card_io_end_event);
    TestEvent(memory_card_timeout_event);
    TestEvent(memory_card_new_device_event);
    TestEvent(memory_card_error_event);
}
