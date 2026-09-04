#include <kf/address.h>
#include <kf/psyq.h>
#include <kf/resources.h>

/* Uploads every CLUT and pixel image in a Psy-Q TIM stream. */
ADDRESS(0x80016298, 0x80)
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

/* Copies WORD_COUNT words and returns the first unread source word. */
ADDRESS(0x80016318, 0x30)
const u32 *resource_stream_copy_words(
    u32 *destination, const u32 *source, s32 word_count)
{
    while (word_count-- != 0) {
        *destination++ = *source++;
    }
    return source;
}
