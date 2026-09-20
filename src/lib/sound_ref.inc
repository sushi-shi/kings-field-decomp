void sound_ref_play(const SoundRef *sound, s16 volume)
{
    audio_play_voice(
        audio_state.bank,
        sound->program,
        sound->tone,
        sound->note,
        volume,
        volume);
}
