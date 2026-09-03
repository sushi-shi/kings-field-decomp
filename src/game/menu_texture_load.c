#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfDisplayState display_state;

extern s32 cd_file_load_into(void *destination, const char *relative_path);
extern void tim_upload_images(u_long *tim_data);

/* The "TIM\Mnnn." disc-name template literal lives in this unit's rodata. */
RODATA(0x80012350, 0xc)

/*
 * Load the numbered TIM texture file for the selected item into the active
 * primitive buffer and upload its images to VRAM.  The disc name "TIM\Mnnn."
 * is built from (id + 1) as three decimal digits.  Returns 1 when the load
 * fails, otherwise 0.
 */
ADDRESS(0x8002af48, 0x130)
u32 menu_load_item_texture(s32 id)
{
    char name[16] = "TIM\\M000.";
    void *destination;
    s32 number;
    s32 remainder;

    if (id != 0xff) {
        number = id + 1;
        name[5] = number / 100 + '0';
        remainder = number % 100;
        name[6] = remainder / 10 + '0';
        name[7] = remainder % 10 + '0';
        destination = display_state.primitive_buffer->cursor;
        if (cd_file_load_into(destination, name) != 0) {
            return 1;
        }
        tim_upload_images((u_long *)destination);
    }
    return 0;
}
