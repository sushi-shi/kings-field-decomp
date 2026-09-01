#include <kf/game_types.h>

/* LIBSND.H (Psy-Q Release 2.5): extern int SsVoKeyOff(long, long); */
extern s32 SsVoKeyOff(s32 voice, s32 program_tone);

void func_80033014(const u8 *voice_mask)
{
    SsVoKeyOff(voice_mask[0], voice_mask[2] << 8);
}
