#include <kf/address.h>
#include <kf/game_math.h>
#include <kf/audio.h>
#include <kf/psyq_audio.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

enum {
    GAME_SEQUENCE_BUFFER_BYTES = 0x3000,
    GAME_SEQUENCE_VOLUME = 0x4b,
    GAME_REVERB_DEPTH = 0x10,
    GAME_SOUND_ATTENUATION_BOOST = 36,
    GAME_SOUND_PAN_NARROW_THRESHOLD = 64,
    GAME_SOUND_PAN_DIVISOR = 3000
};

/* Error messages and the sequence path template of this unit in the retail data region. */
RODATA(0x80012a14, 0x40)

/* Sequence and VAB operations come from the pinned Psy-Q LIBSND.H. */

ADDRESS(0x800328e0, 0xa4)
void audio_initialize(void)
{
    s32 index;

    SsInit();
    SsSetTableSize((char *)audio_sequence_table,
        KF_AUDIO_SEQUENCE_CAPACITY, KF_AUDIO_TRACKS_PER_SEQUENCE);
    SsSetTickMode(SS_TICK60);
    SsStart();
    SsSetMVol(KF_AUDIO_MAX_VOLUME, KF_AUDIO_MAX_VOLUME);
    SsUtSetReverbType(SS_REV_TYPE_STUDIO_C);
    SsUtReverbOn();
    SsUtSetReverbDepth(GAME_REVERB_DEPTH, GAME_REVERB_DEPTH);
    audio_state.sequence_buffer = memory_allocate(GAME_SEQUENCE_BUFFER_BYTES);
    audio_state.sequence_active = 0;
    index = KF_AUDIO_VOICE_SLOTS - 1;
    do {
        audio_state.voice_slots.voice_ids[index] = KF_AUDIO_VOICE_INACTIVE;
    } while (--index >= 0);
}

ADDRESS(0x80032984, 0xc8)
void audio_load_vab(u8 *vab_header, u8 *vab_body)
{
    audio_stop_sequence_fade();
    audio_state.active_vab_id = SsVabOpenHead(vab_header, KF_AUDIO_VAB_AUTO);
    if (audio_state.active_vab_id == KF_AUDIO_VAB_UNAVAILABLE) {
        printf("VAB headder open failed\n");
        return;
    }
    audio_state.vab_header = vab_header;
    audio_state.active_vab_id = SsVabTransBody(vab_body, audio_state.active_vab_id);
    if (audio_state.active_vab_id == KF_AUDIO_VAB_UNAVAILABLE) {
        printf("VAB body open failed\n");
        return;
    }
    SsVabTransCompleted(SS_WAIT_COMPLETED);
}

ADDRESS(0x80032a4c, 0x110)
void audio_play_map_sequence(u8 sequence_id)
{
    char path[20] = "B0\\SND0.SEQ";

    audio_stop_sequence_fade();
    if (player_state.audio_music_enabled) {
        path[6] = sequence_id + '0';
        path[1] = player_state.progress_state.current_floor + '0';
        if (cd_file_load_into(audio_state.sequence_buffer, path) == 0) {
            audio_state.sequence_id = SsSeqOpen(
                (u32 *)audio_state.sequence_buffer, audio_state.active_vab_id);
            SsSeqSetVol(audio_state.sequence_id, GAME_SEQUENCE_VOLUME, GAME_SEQUENCE_VOLUME);
            SsSeqPlay(audio_state.sequence_id, SSPLAY_PLAY, SSPLAY_INFINITY);
            audio_state.sequence_active = 1;
        }
    }
}

ADDRESS(0x80032b5c, 0x80)
void audio_stop_sequence_fade(void)
{
    s32 volume;

    if (audio_state.sequence_active == 1) {
        volume = GAME_SEQUENCE_VOLUME;
        do {
            VSync(0);
            SsSeqSetVol(audio_state.sequence_id, volume, volume);
        } while (--volume >= 0);
        SsSeqStop(audio_state.sequence_id);
        SsSeqClose(audio_state.sequence_id);
        audio_state.sequence_active = 0;
    }
}

ADDRESS(0x80032bdc, 0x9c)
void audio_stop_sequence_master_fade(s32 fade_step)
{
    s32 volume;

    if (audio_state.sequence_active == 1) {
        volume = GAME_SEQUENCE_VOLUME << KF_FIXED8_BITS;
        do {
            VSync(0);
            SsSetMVol(volume >> KF_FIXED8_BITS, volume >> KF_FIXED8_BITS);
            volume -= fade_step;
        } while (volume > 0);
        SsSetMVol(0, 0);
        SsSeqSetVol(audio_state.sequence_id, 0, 0);
        SsSeqStop(audio_state.sequence_id);
        SsSeqClose(audio_state.sequence_id);
        audio_state.sequence_active = 0;
    }
}

ADDRESS(0x80032c78, 0x38)
void audio_shutdown(void)
{
    audio_close_vab();
    SsSeqClose(audio_state.sequence_id);
    SsEnd();
}

ADDRESS(0x80032cb0, 0x40)
void audio_close_vab(void)
{
    s16 *vab_id = &audio_state.active_vab_id;

    SsVabClose(*vab_id);
    *vab_id = KF_AUDIO_VAB_UNAVAILABLE;
    audio_state.vab_header = 0;
}

ADDRESS(0x80032cf0, 0x2c8)
u32 audio_play_spatial(
    const SoundRef *sound,
    const VECTOR *position,
    s16 volume,
    s32 max_distance,
    s32 attenuation_distance)
{
    s32 delta_x = (position->vx - audio_state.listener_position.vx)
        >> KF_LENGTH_SQUARE_DOWNSHIFT;
    s32 delta_y = (position->vy - audio_state.listener_position.vy)
        >> KF_LENGTH_SQUARE_DOWNSHIFT;
    s32 delta_z = (position->vz - audio_state.listener_position.vz)
        >> KF_LENGTH_SQUARE_DOWNSHIFT;
    s32 distance;
    s32 attenuation;
    s32 level;
    s32 angle;
    s32 left;
    s32 right;

    distance = SquareRoot0(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z)
        << KF_LENGTH_SQUARE_DOWNSHIFT;
    if (distance >= max_distance) {
        return 0;
    }
    attenuation = ((attenuation_distance - distance) << KF_FIXED7_BITS) / attenuation_distance;
    level = (attenuation * volume) >> KF_FIXED7_BITS;
    if (level < 0) {
        level = 0;
    } else if (level >= KF_AUDIO_MAX_VOLUME + 1) {
        level = KF_AUDIO_MAX_VOLUME;
    }
    angle = vector_xz_to_angle(
        position->vx - audio_state.listener_position.vx,
        audio_state.listener_position.vz - position->vz);
    angle = (angle - audio_state.listener_rotation.vy + KF_ANGLE_QUARTER_TURN)
        & KF_ANGLE_WRAP_MASK;
    if (angle >= KF_ANGLE_HALF_TURN) {
        angle = KF_ANGLE_FULL_TURN - angle;
    }
    angle >>= 1;
    if ((sound->tone & 0x80) == 1) {
        attenuation += GAME_SOUND_ATTENUATION_BOOST;
        if (attenuation >= KF_AUDIO_MAX_VOLUME + 1) {
            attenuation = KF_AUDIO_MAX_VOLUME;
        }
    }
    if (attenuation >= GAME_SOUND_PAN_NARROW_THRESHOLD) {
        angle = (((angle - KF_ANGLE_EIGHTH_TURN)
            * (KF_FIXED7_ONE * 2 - attenuation * 2)) >> KF_FIXED7_BITS)
            + KF_ANGLE_EIGHTH_TURN;
    }
    left = (level * rsin(angle)) / GAME_SOUND_PAN_DIVISOR;
    if (left >= KF_AUDIO_MAX_VOLUME + 1) {
        left = KF_AUDIO_MAX_VOLUME;
    }
    right = (level * rcos(angle)) / GAME_SOUND_PAN_DIVISOR;
    if (right >= KF_AUDIO_MAX_VOLUME + 1) {
        right = KF_AUDIO_MAX_VOLUME;
    }
    audio_play_voice(
        audio_state.active_vab_id,
        sound->program,
        sound->tone & KF_SOUND_TONE_INDEX_MASK,
        sound->note,
        left,
        right);
    return 1;
}

ADDRESS(0x80032fb8, 0x30)
u32 audio_play_spatial_default_range(
    const SoundRef *sound,
    const VECTOR *position,
    s16 volume)
{
    return audio_play_spatial(sound, position, volume,
        KF_AUDIO_DEFAULT_MAX_DISTANCE, KF_AUDIO_DEFAULT_ATTENUATION_DISTANCE);
}

ADDRESS(0x80032fe8, 0x2c)
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

ADDRESS(0x80033014, 0x28)
void sound_ref_key_off_bank0(const SoundRef *sound)
{
    /* Bank 0; note occupies the upper byte, with zero fine pitch. */
    SsVoKeyOff(sound->program, sound->note << 8);
}

ADDRESS(0x8003303c, 0x70)
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

ADDRESS(0x800330ac, 0x48)
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

ADDRESS(0x800330f4, 0x1a8)
void audio_play_voice(
    s16 vab_id, s16 program, s16 tone, s16 note, s16 left_volume, s16 right_volume)
{
    if (program == 0 && tone == 0 && note == 0) {
        return;
    }
    if (!player_state.audio_effects_enabled) {
        return;
    }
    audio_voice_slot_index++;
    if (audio_voice_slot_index == KF_AUDIO_VOICE_SLOTS) {
        audio_voice_slot_index = 0;
    }
    if (audio_state.voice_slots.voice_ids[audio_voice_slot_index] != KF_AUDIO_VOICE_INACTIVE) {
        SsUtKeyOff(
            audio_state.voice_slots.voice_ids[audio_voice_slot_index],
            audio_state.voice_slots.vab_ids[audio_voice_slot_index],
            audio_state.voice_slots.programs[audio_voice_slot_index],
            audio_state.voice_slots.tones[audio_voice_slot_index],
            audio_state.voice_slots.notes[audio_voice_slot_index]);
    }
    audio_state.voice_slots.vab_ids[audio_voice_slot_index] = vab_id;
    audio_state.voice_slots.programs[audio_voice_slot_index] = program;
    audio_state.voice_slots.tones[audio_voice_slot_index] = tone;
    audio_state.voice_slots.notes[audio_voice_slot_index] = note;
    audio_state.voice_slots.voice_ids[audio_voice_slot_index] =
        SsUtKeyOn(vab_id, program, tone, note, 0, left_volume, right_volume);
}

ADDRESS(0x8003329c, 0x48)
s16 angle_shortest_delta(s32 first, s32 second)
{
    s32 difference;
    s16 signed_difference;

    first &= KF_ANGLE_WRAP_MASK;
    second &= KF_ANGLE_WRAP_MASK;
    difference = second - first;
    signed_difference = difference;
    if (signed_difference >= KF_ANGLE_HALF_TURN) {
        return difference - KF_ANGLE_FULL_TURN;
    }
    if (signed_difference < -KF_ANGLE_HALF_TURN + 1) {
        return difference + KF_ANGLE_FULL_TURN;
    }
    return signed_difference;
}
