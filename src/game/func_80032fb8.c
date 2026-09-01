#include <kf/semantic_types.h>

extern u32 func_80032cf0(
    const SoundRef *sound,
    const struct KfVec4i *position,
    s16 volume,
    s32 max_distance,
    s32 attenuation_distance);

void func_80032fb8(
    const SoundRef *sound,
    const struct KfVec4i *position,
    s16 volume)
{
    func_80032cf0(sound, position, volume, 0x3e80, 0x6d60);
}
