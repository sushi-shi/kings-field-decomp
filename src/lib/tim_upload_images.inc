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
