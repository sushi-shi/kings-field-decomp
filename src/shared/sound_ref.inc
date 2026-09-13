ADDRESS_AT("GAME", 0x800330ac, 0x48)
ADDRESS_AT("OPEN", 0x8001a220, 0x48)
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
