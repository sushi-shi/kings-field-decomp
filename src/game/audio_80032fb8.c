#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game_types.h>

extern u32 audio_play_spatial(
    const SoundRef *sound,
    const struct KfVec4i *position,
    s16 volume,
    s32 max_distance,
    s32 attenuation_distance);

/* LIBSND.H (Psy-Q Release 2.5): extern int SsVoKeyOff(long, long); */
extern s32 SsVoKeyOff(s32 voice, s32 program_tone);

extern struct KfVec4i audio_listener_position;
extern struct KfVec4s audio_listener_rotation;

extern s16 audio_active_vab_id;
extern void audio_play_voice(
    s16 vab_id,
    s16 program,
    s16 tone,
    s16 note,
    s16 left_volume,
    s16 right_volume);

ADDRESS(0x80032fb8)
void audio_play_spatial_default_range(
    const SoundRef *sound,
    const struct KfVec4i *position,
    s16 volume)
{
    audio_play_spatial(sound, position, volume, 0x3e80, 0x6d60);
}

ADDRESS(0x80032fe8)
void audio_play_spatial_range(
    const SoundRef *sound,
    const struct KfVec4i *position,
    s16 volume,
    s32 max_distance,
    s32 attenuation_distance)
{
    audio_play_spatial(
        sound,
        position,
        volume,
        max_distance,
        attenuation_distance);
}

ADDRESS(0x80033014)
void audio_key_off_mask(const u8 *voice_mask)
{
    SsVoKeyOff(voice_mask[0], voice_mask[2] << 8);
}

ADDRESS(0x8003303c)
void audio_set_listener_transform(
    const struct KfVec4i *position_or_null,
    const struct KfVec4s *rotation_or_null)
{
    if (position_or_null != 0) {
        audio_listener_position = *position_or_null;
    }
    if (rotation_or_null != 0) {
        audio_listener_rotation = *rotation_or_null;
    }
}

ADDRESS(0x800330ac)
void sound_ref_play(const SoundRef *sound, s16 volume)
{
    audio_play_voice(
        audio_active_vab_id,
        sound->program,
        sound->tone,
        sound->note,
        volume,
        volume);
}
