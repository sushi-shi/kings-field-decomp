#include <kf/address.h>
#include <kf/audio.h>
#include <kf/game_math.h>
#include <kf/open_audio.h>
#include <kf/psyq_audio.h>

enum {
    OPEN_SOUND_EQUAL_PAN_THRESHOLD = 96
};

ADDRESS(0x80019f44, 0x1e8)
u32 audio_play_spatial(
    const SoundRef *sound,
    const VECTOR *position,
    s16 volume,
    s32 max_distance,
    s32 attenuation_distance)
{
    s32 delta_x = (position->vx - audio_state.listener_position.vx) >> KF_LENGTH_SQUARE_DOWNSHIFT;
    s32 delta_y = (position->vy - audio_state.listener_position.vy) >> KF_LENGTH_SQUARE_DOWNSHIFT;
    s32 delta_z = (position->vz - audio_state.listener_position.vz) >> KF_LENGTH_SQUARE_DOWNSHIFT;
    s32 attenuation;
    s32 level;
    s32 angle;
    s32 left;
    s32 right;

    attenuation =
        SquareRoot0(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z)
        << KF_LENGTH_SQUARE_DOWNSHIFT;
    if (attenuation >= max_distance) {
        return 0;
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
    return 1;
}

ADDRESS(0x8001a12c, 0x30)
u32 audio_play_spatial_default_range(
    const SoundRef *sound,
    const VECTOR *position,
    s16 volume)
{
    return audio_play_spatial(sound, position, volume,
        KF_AUDIO_DEFAULT_MAX_DISTANCE, KF_AUDIO_DEFAULT_ATTENUATION_DISTANCE);
}

ADDRESS(0x8001a15c, 0x2c)
u32 audio_play_spatial_range(
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

ADDRESS(0x8001a188, 0x28)
void audio_key_off_mask(const u8 *voice_mask)
{
    SsVoKeyOff(voice_mask[0], voice_mask[2] << 8);
}

ADDRESS(0x8001a1b0, 0x70)
void audio_set_listener_transform(
    const VECTOR *position_or_null,
    const SVECTOR *rotation_or_null)
{
    if (position_or_null != 0) {
        audio_state.listener_position = *position_or_null;
    }
    if (rotation_or_null != 0) {
        audio_state.listener_rotation = *rotation_or_null;
    }
}
