#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfDisplayState display_state;

ADDRESS(0x8001fdc8, 0x1c)
void func_8001fdc8(void)
{
    display_state.buffer_index = (display_state.buffer_index == 0);
}
