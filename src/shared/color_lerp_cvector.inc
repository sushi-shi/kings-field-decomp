ADDRESS(0x800196c4, 0x88)
void color_lerp_cvector(
    const CVECTOR *from,
    const CVECTOR *to,
    CVECTOR *output,
    s32 blend)
{
    output->r = (((to->r - from->r) * blend) >> KF_FIXED12_BITS) + from->r;
    output->g = (((to->g - from->g) * blend) >> KF_FIXED12_BITS) + from->g;
    output->b = (((to->b - from->b) * blend) >> KF_FIXED12_BITS) + from->b;
}
