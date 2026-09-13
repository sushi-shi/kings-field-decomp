ADDRESS_AT("GAME", 0x800203bc, 0x44)
ADDRESS_AT("OPEN", 0x80019658, 0x44)
void fog_interpolate_near(s32 start, s32 end, s32 ratio)
{
    s32 distance = (((end - start) * ratio) >> KF_FIXED12_BITS) + start;

    KF_GRAPHICS_RUNTIME.render_state.fog_near_distance = distance;
    SetFogNear(distance, KF_DEFAULT_PROJECTION_DISTANCE);
}

ADDRESS_AT("GAME", 0x80020400, 0x28)
ADDRESS_AT("OPEN", 0x8001969c, 0x28)
void fog_set_near(s32 distance)
{
    KF_GRAPHICS_RUNTIME.render_state.fog_near_distance = distance;
    SetFogNear(distance, KF_DEFAULT_PROJECTION_DISTANCE);
}
