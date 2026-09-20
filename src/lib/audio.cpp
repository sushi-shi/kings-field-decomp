#include <kf/lib/audio.h>

static constexpr std::size_t sound_chunk_header_bytes = 4;

static u32 audio_chunk_size(const u8 *data)
{
    return static_cast<u32>(data[0]) | (static_cast<u32>(data[1]) << 8)
        | (static_cast<u32>(data[2]) << 16) | (static_cast<u32>(data[3]) << 24);
}

void audio_load_vab_resource(const u8 *data, std::size_t size)
{
    if (size < sound_chunk_header_bytes)
        kf::host_fail("Truncated sound-bank resource");
    const std::size_t header_size = audio_chunk_size(data);
    if (header_size > size - sound_chunk_header_bytes || size - sound_chunk_header_bytes - header_size < sound_chunk_header_bytes)
        kf::host_fail("Truncated sound-bank header");
    const u8 *body_chunk = data + sound_chunk_header_bytes + header_size;
    const std::size_t body_size = audio_chunk_size(body_chunk);
    if (body_size > size - 2 * sound_chunk_header_bytes - header_size)
        kf::host_fail("Truncated sound-bank samples");
    audio_load_vab(data + sound_chunk_header_bytes, header_size, body_chunk + sound_chunk_header_bytes, body_size);
}

void audio_shutdown(void)
{
    audio_close_vab();
}

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

    kf::sound_note_release(audio_state.bank, sound->program, sound->note);
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

void sound_ref_play(const SoundRef *sound, s16 volume)
{
    audio_play_voice(
        audio_state.bank,
        sound->program,
        sound->tone_and_flags,
        sound->note,
        volume,
        volume);
}
