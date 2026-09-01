#include <kf/address.h>
#include <kf/game_types.h>

extern void memory_card_shutdown_events(void);
extern void audio_shutdown(void);
extern void func_80050170(void);
extern u32 func_80050544(u32 object);
extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);

/*
 * Both counters live in the retail load image four bytes apart, so they were
 * explicitly initialized data, not commons; static linkage is the curated
 * TU-ownership candidate for this frame pacer.
 */
DATA(0x80057b0c, 0x4)
static u32 frame_pacer_vsync_count = 0;

DATA(0x80057b10, 0x4)
static u32 frame_pacer_last_vsync = 0;

ADDRESS(0x8001499c, 0x38)
void game_shutdown(void)
{
    memory_card_shutdown_events();
    audio_shutdown();
    func_80050170();
    func_80050544(3);
}

ADDRESS(0x800149d4, 0x20)
void frame_pacer_vsync_callback(void)
{
    frame_pacer_vsync_count++;
}

ADDRESS(0x800149f4, 0x70)
void frame_pacer_wait(void)
{
    for (;;) {
        EnterCriticalSection();
        if (frame_pacer_last_vsync + 2 < frame_pacer_vsync_count
            || frame_pacer_vsync_count < frame_pacer_last_vsync) {
            frame_pacer_last_vsync = frame_pacer_vsync_count;
            ExitCriticalSection();
            return;
        }
        ExitCriticalSection();
    }
}
