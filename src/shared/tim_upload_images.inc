ADDRESS_AT("GAME", 0x8001b100, 0x80)
ADDRESS_AT("OPEN", 0x80016298, 0x80)
void tim_upload_images(u8 *tim_data)
{
    TIM_IMAGE image;

    OpenTIM((u_long *)tim_data);
    while (ReadTIM(&image) != NULL) {
        if (image.caddr != NULL) {
            LoadImage(image.crect, image.caddr);
            DrawSync(0);
        }
        if (image.paddr != NULL) {
            LoadImage(image.prect, image.paddr);
            DrawSync(0);
        }
    }
}
