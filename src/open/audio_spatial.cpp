#include <kf/lib/null.h>

#include <kf/open/audio.h>
#include <kf/lib/math.h>

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
    s32 distance_gain_q7;
    s32 attenuated_volume;
    s32 angle;
    s32 left;
    s32 right;

    const s32 listener_distance =
        fixed_vector3_length(delta_x, delta_y, delta_z);
    if (listener_distance >= max_distance) {
        return KF_AUDIO_NOT_PLAYED;
    }
    distance_gain_q7 =
        ((attenuation_distance - listener_distance) << KF_FIXED7_BITS) / attenuation_distance;
    attenuated_volume = (distance_gain_q7 * volume) >> KF_FIXED7_BITS;
    angle = vector_xz_to_angle(
        position->vx - audio_state.listener_position.vx,
        audio_state.listener_position.vz - position->vz);
    angle = (angle - audio_state.listener_rotation.vy + KF_ANGLE_QUARTER_TURN) & KF_ANGLE_WRAP_MASK;
    if (angle >= KF_ANGLE_HALF_TURN) {
        angle = KF_ANGLE_FULL_TURN - angle;
    }
    angle >>= 1;
    if (distance_gain_q7 >= OPEN_SOUND_EQUAL_PAN_THRESHOLD) {
        left = distance_gain_q7;
        right = distance_gain_q7;
    } else {
        left = (attenuated_volume * kf::angle_sine(angle)) >> KF_FIXED12_BITS;
        right = (attenuated_volume * kf::angle_cosine(angle)) >> KF_FIXED12_BITS;
    }
    audio_play_voice(audio_playback(),
        audio_state.bank,
        sound->program,
        sound->tone_and_flags,
        sound->note,
        left,
        right);
    return KF_AUDIO_PLAYED;
}
