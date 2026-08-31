#include <kf/game_types.h>

struct SoundRef800330ac {
    u8 bank;
    u8 program;
    u8 note;
};

extern s16 DAT_8009586c;
extern void func_800330f4(s16 vab_id, u8 bank, u8 program, u8 note, s16 left, s16 right);

void func_800330ac(const struct SoundRef800330ac *sound, s16 volume)
{
    func_800330f4(
        DAT_8009586c,
        sound->bank,
        sound->program,
        sound->note,
        volume,
        volume);
}
