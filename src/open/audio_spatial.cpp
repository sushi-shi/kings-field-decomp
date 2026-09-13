#include <kf/null.h>

#include <kf/audio.h>
#include <kf/game_math.h>
#include <kf/open_audio.h>
#include <psyq/audio.h>

enum {
    OPEN_SOUND_EQUAL_PAN_THRESHOLD = 96
};

KfAudioPlaybackResult audio_play_spatial(
    const SoundRef *sound,
    const VECTOR *position,
    s16 volume,
    s32 max_distance,
    s32 attenuation_distance)
{
    s32 delta_x = position->vx - audio_state.listener_position.vx;
    s32 delta_y = position->vy - audio_state.listener_position.vy;
    s32 delta_z = position->vz - audio_state.listener_position.vz;
    s32 attenuation;
    s32 level;
    s32 angle;
    s32 left;
    s32 right;

    attenuation =
        fixed_vector3_length(delta_x, delta_y, delta_z);
    if (attenuation >= max_distance) {
        return KF_AUDIO_NOT_PLAYED;
    }
    attenuation =
        ((attenuation_distance - attenuation) << KF_FIXED7_BITS) / attenuation_distance;
    level = (attenuation * volume) >> KF_FIXED7_BITS;
    angle = vector_xz_to_angle(
        position->vx - audio_state.listener_position.vx,
        audio_state.listener_position.vz - position->vz);
    angle = (angle - audio_state.listener_rotation.vy + KF_ANGLE_QUARTER_TURN) & KF_ANGLE_WRAP_MASK;
    if (angle >= KF_ANGLE_HALF_TURN) {
        angle = KF_ANGLE_FULL_TURN - angle;
    }
    angle >>= 1;
    if (attenuation >= OPEN_SOUND_EQUAL_PAN_THRESHOLD) {
        left = attenuation;
        right = attenuation;
    } else {
        left = (level * rsin(angle)) >> KF_FIXED12_BITS;
        right = (level * rcos(angle)) >> KF_FIXED12_BITS;
    }
    audio_play_voice(
        audio_state.active_vab_id,
        sound->program,
        sound->tone,
        sound->note,
        left,
        right);
    return KF_AUDIO_PLAYED;
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
