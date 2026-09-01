#include <kf/address.h>
#include <kf/game_types.h>

extern s16 audio_sequence_id;
extern void audio_close_vab(void);
extern void func_8004b6e0(s32 value);
extern void SsEnd(void);

extern s32 audio_vab_header;
extern s16 audio_active_vab_id;
extern void SsVabClose(s16 vab_id);

ADDRESS(0x80032c78)
void audio_shutdown(void)
{
    audio_close_vab();
    func_8004b6e0(audio_sequence_id);
    SsEnd();
}

ADDRESS(0x80032cb0)
void audio_close_vab(void)
{
    s16 *vab_id = &audio_active_vab_id;

    SsVabClose(*vab_id);
    *vab_id = -1;
    audio_vab_header = 0;
}
