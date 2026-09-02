#include <kf/address.h>
#include <SYS/TYPES.H>
#include <LIBGTE.H>
#include <LIBGPU.H>

ADDRESS(0x8001b100, 0x80)
void tim_upload_images(u_long *tim_data)
{
    TIM_IMAGE image;

    OpenTIM(tim_data);
    while (ReadTIM(&image) != 0) {
        if (image.caddr != 0) {
            LoadImage(image.crect, image.caddr);
            DrawSync(0);
        }
        if (image.paddr != 0) {
            LoadImage(image.prect, image.paddr);
            DrawSync(0);
        }
    }
}
