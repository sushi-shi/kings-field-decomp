#include <kf/lib/null.h>

#include <kf/lib/math.h>
#include <kf/lib/audio.h>
#include <kf/lib/audio_sequence.h>
#include <psyq/audio.h>
#include <psyq/libc.h>
#include <kf/game/game.h>

enum {
    GAME_SEQUENCE_BUFFER_BYTES = 0x3000,
    GAME_SEQUENCE_VOLUME = 0x4b,
    GAME_REVERB_DEPTH = 0x10,
    GAME_SOUND_ATTENUATION_BOOST = 36,
    GAME_SOUND_PAN_NARROW_THRESHOLD = 64,
    GAME_SOUND_PAN_DIVISOR = 3000
};

s32 audio_voice_slot_index = 9;

static char audio_sequence_table[SS_SEQ_TABSIZ * KF_AUDIO_SEQUENCE_CAPACITY];

KfAudioState audio_state;

void audio_initialize(void)
{
    SsInit();
    SsSetTableSize(audio_sequence_table,
        KF_AUDIO_SEQUENCE_CAPACITY, KF_AUDIO_TRACKS_PER_SEQUENCE);
    SsSetTickMode(SS_TICK60);
    SsStart();
    SsSetMVol(KF_AUDIO_MAX_VOLUME, KF_AUDIO_MAX_VOLUME);
    SsUtSetReverbType(SS_REV_TYPE_STUDIO_C);
    SsUtReverbOn();
    SsUtSetReverbDepth(GAME_REVERB_DEPTH, GAME_REVERB_DEPTH);
    audio_state.sequence_buffer = (u_long *)memory_allocate(GAME_SEQUENCE_BUFFER_BYTES);
    audio_state.sequence_active = KF_AUDIO_SEQUENCE_INACTIVE;
    audio_reset_voice_slots();
}

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

void audio_play_map_sequence(u8 sequence_id)
{
    char path[20] = "B0\\SND0.SEQ";

    audio_stop_sequence_fade();
    if (player_state.audio_music_enabled != KF_PLAYER_OPTION_OFF) {
        path[6] = sequence_id + '0';
        path[1] = kf_enum_encode<u8>(player_state.progress_state.current_floor) + '0';
        if (cd_file_load_into((void *)audio_state.sequence_buffer, path) == KF_RESOURCE_LOADED) {
            audio_state.sequence_id = SsSeqOpen(
                audio_state.sequence_buffer, audio_state.active_vab_id);
            SsSeqSetVol(audio_state.sequence_id, GAME_SEQUENCE_VOLUME, GAME_SEQUENCE_VOLUME);
            SsSeqPlay(audio_state.sequence_id, SSPLAY_PLAY, SSPLAY_INFINITY);
            audio_state.sequence_active = KF_AUDIO_SEQUENCE_ACTIVE;
        }
    }
}

void audio_stop_sequence_fade(void)
{
    s32 volume;

    if (audio_state.sequence_active == KF_AUDIO_SEQUENCE_ACTIVE) {
        volume = GAME_SEQUENCE_VOLUME;
        do {
            VSync(0);
            SsSeqSetVol(audio_state.sequence_id, volume, volume);
        } while (--volume >= 0);
        AUDIO_SEQUENCE_STOP_AND_CLOSE();
    }
}

void audio_stop_sequence_master_fade(s32 fade_step)
{
    s32 volume;

    if (audio_state.sequence_active == KF_AUDIO_SEQUENCE_ACTIVE) {
        volume = GAME_SEQUENCE_VOLUME << KF_FIXED8_BITS;
        do {
            VSync(0);
            SsSetMVol(volume >> KF_FIXED8_BITS, volume >> KF_FIXED8_BITS);
            volume -= fade_step;
        } while (volume > 0);
        SsSetMVol(0, 0);
        SsSeqSetVol(audio_state.sequence_id, 0, 0);
        AUDIO_SEQUENCE_STOP_AND_CLOSE();
    }
}

#include "../lib/audio_shutdown.inc"

void audio_close_vab(void)
{
    s16 *vab_id = &audio_state.active_vab_id;

    SsVabClose(*vab_id);
    *vab_id = KF_AUDIO_VAB_UNAVAILABLE;
    audio_state.vab_header = NULL;
}

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

    attenuation = fixed_vector3_length(delta_x, delta_y, delta_z);
    if (attenuation >= max_distance) {
        return KF_AUDIO_NOT_PLAYED;
    }
    attenuation = ((attenuation_distance - attenuation) << KF_FIXED7_BITS) / attenuation_distance;
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
    return KF_AUDIO_PLAYED;
}

#include "../lib/audio_spatial_helpers.inc"

#include "../lib/sound_ref.inc"

void audio_play_voice(
    s16 vab_id, s16 program, s16 tone, s16 note, s16 left_volume, s16 right_volume)
{
    if (program == 0 && tone == 0 && note == 0) {
        return;
    }
    if (player_state.audio_effects_enabled == KF_PLAYER_OPTION_OFF) {
        return;
    }
    audio_key_on_next_slot(vab_id, program, tone, note, left_volume, right_volume);
}

#include "../lib/angle_shortest_delta.inc"
