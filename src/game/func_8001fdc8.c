#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfDisplayState display_state;

ADDRESS(0x8001fdc8, 0x1c)
void display_flip_buffer_index(void)
{
    display_state.buffer_index = (display_state.buffer_index == 0);
}
