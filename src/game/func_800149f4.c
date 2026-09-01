#include <kf/game_types.h>

extern u32 frame_pacer_last_vsync;
extern u32 frame_pacer_vsync_count;
extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);

void func_800149f4(void)
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
