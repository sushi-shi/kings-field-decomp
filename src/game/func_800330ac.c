#include <kf/semantic_types.h>

extern s16 DAT_8009586c;
extern void func_800330f4(
    s16 vab_id,
    s16 program,
    s16 tone,
    s16 note,
    s16 left_volume,
    s16 right_volume);

void func_800330ac(const SoundRef *sound, s16 volume)
{
    func_800330f4(
        DAT_8009586c,
        sound->program,
        sound->tone,
        sound->note,
        volume,
        volume);
}
