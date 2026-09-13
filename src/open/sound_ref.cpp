#include <kf/audio.h>
#include <kf/open_audio.h>

void sound_ref_play(const SoundRef *sound, s16 volume)
{
    audio_play_voice(
        audio_state.active_vab_id,
        sound->program,
        sound->tone,
        sound->note,
        volume,
        volume);
}
