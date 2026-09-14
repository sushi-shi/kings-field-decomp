ADDRESS_AT("GAME", 0x80032fb8, 0x30)
ADDRESS_AT("OPEN", 0x8001a12c, 0x30)
KfAudioPlaybackResult audio_play_spatial_default_range(
    const SoundRef *sound,
    const VECTOR *position,
    s16 volume)
{
    return audio_play_spatial(sound, position, volume,
        KF_AUDIO_DEFAULT_MAX_DISTANCE, KF_AUDIO_DEFAULT_ATTENUATION_DISTANCE);
}

ADDRESS_AT("GAME", 0x80032fe8, 0x2c)
ADDRESS_AT("OPEN", 0x8001a15c, 0x2c)
KfAudioPlaybackResult audio_play_spatial_range(
    const SoundRef *sound,
    const VECTOR *position,
    s16 volume,
    s32 max_distance,
    s32 attenuation_distance)
{
    return audio_play_spatial(
        sound,
        position,
        volume,
        max_distance,
        attenuation_distance);
}

ADDRESS_AT("GAME", 0x80033014, 0x28)
ADDRESS_AT("OPEN", 0x8001a188, 0x28)
void sound_ref_key_off_bank0(const SoundRef *sound)
{
    /* Bank 0; note occupies the upper byte, with zero fine pitch. */
    SsVoKeyOff(sound->program, sound->note << KF_SOUND_PACKED_NOTE_SHIFT);
}

ADDRESS_AT("GAME", 0x8003303c, 0x70)
ADDRESS_AT("OPEN", 0x8001a1b0, 0x70)
void audio_set_listener_transform(
    const VECTOR *position_or_null,
    const SVECTOR *rotation_or_null)
{
    if (position_or_null != NULL) {
        audio_state.listener_position = *position_or_null;
    }
    if (rotation_or_null != NULL) {
        audio_state.listener_rotation = *rotation_or_null;
    }
}
