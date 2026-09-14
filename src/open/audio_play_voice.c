#include <kf/lib/address.h>
#include <kf/lib/audio.h>
#include <kf/open/audio.h>
#include <psyq/audio.h>

ADDRESS(0x8001a268, 0x194)
void audio_play_voice(
    s16 vab_id, s16 program, s16 tone, s16 note, s16 left_volume, s16 right_volume)
{
    if (program == 0 && tone == 0 && note == 0) {
        return;
    }
    audio_key_on_next_slot(vab_id, program, tone, note, left_volume, right_volume);
}

DATA(0x80037304, 0x4)
s32 audio_voice_slot_index = KF_AUDIO_VOICE_SLOTS - 1;
