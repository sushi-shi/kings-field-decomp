KfAudioPlaybackResult audio_play_spatial_default_range(
    const SoundRef *sound,
    const VECTOR *position,
    s16 volume)
{
    return audio_play_spatial(sound, position, volume,
        KF_AUDIO_DEFAULT_MAX_DISTANCE, KF_AUDIO_DEFAULT_ATTENUATION_DISTANCE);
}

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

void sound_ref_key_off_bank0(const SoundRef *sound)
{

    SsVoKeyOff(sound->program, sound->note << KF_SOUND_PACKED_NOTE_SHIFT);
}

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
