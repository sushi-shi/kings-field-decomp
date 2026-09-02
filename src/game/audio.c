#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfAudioState audio_state;
extern u8 audio_sequence_table[];
extern u8 audio_music_enabled;
extern u8 audio_effects_enabled;
extern s32 audio_voice_slot_index;
extern KfPlayerProgressState player_progress_state;
extern const char DAT_80012a14[];
extern const char DAT_80012a30[];

/* Unresolved helpers: the LIBSND-shaped table/initialization pair, a file loader,
 * a sequence opener/stopper, and a frame wait. */
extern void func_8003e350(void);
extern void func_8003e440(u8 *table, s16 sequence_count, s16 track_count);
extern void *memory_allocate(s32 size);
extern s16 func_800446a8(u8 *vab_header, s16 vab_id);
extern u32 func_8001af9c(void *destination, const char *path);
extern s16 func_800468d8(u8 *sequence, s16 vab_id);
extern void func_8004a344(s16 sequence_id);
extern void func_8004b6e0(s16 sequence_id);
extern s32 func_800555e0(s32 mode);

/* Psy-Q Release 2.5 LIBSND.H / LIBGTE.H / LIBC prototypes. */
extern void SsSetTickMode(s32 tick_mode);
extern void SsStart(void);
extern void SsEnd(void);
extern void SsSetMVol(s16 left, s16 right);
extern s16 SsUtSetReverbType(s16 type);
extern void SsUtReverbOn(void);
extern void SsUtSetReverbDepth(s16 left, s16 right);
extern s16 SsVabTransBody(u8 *body, s16 vab_id);
extern s16 SsVabTransCompleted(s16 immediate);
extern void SsVabClose(s16 vab_id);
extern void SsSeqSetVol(s16 sequence_id, s16 left, s16 right);
extern void SsSeqPlay(s16 sequence_id, s8 play_mode, s16 loop_count);
extern s16 SsUtKeyOn(
    s16 vab_id, s16 program, s16 tone, s16 note, s16 fine, s16 left, s16 right);
extern s16 SsUtKeyOff(s16 voice, s16 vab_id, s16 program, s16 tone, s16 note);
extern s32 SsVoKeyOff(s32 voice, s32 program_tone);
extern s32 SquareRoot0(s32 value);
extern s32 rsin(s32 angle);
extern s32 rcos(s32 angle);
/* Declared with an int result here: retail uses the returned angle unmasked. */
extern s32 vector_xz_to_angle(s32 x, s32 z);
extern int printf();
extern void *memset();

extern void audio_close_vab(void);
extern void audio_stop_sequence_fade(void);
extern u32 audio_play_spatial(
    const SoundRef *sound,
    const struct KfVec4i *position,
    s16 volume,
    s32 max_distance,
    s32 attenuation_distance);
extern void audio_play_voice(
    s16 vab_id, s16 program, s16 tone, s16 note, s16 left_volume, s16 right_volume);

ADDRESS(0x800328e0, 0xa4)
void audio_initialize(void)
{
    s32 index;

    func_8003e350();
    func_8003e440(audio_sequence_table, 2, 1);
    SsSetTickMode(1);
    SsStart();
    SsSetMVol(0x7f, 0x7f);
    SsUtSetReverbType(4);
    SsUtReverbOn();
    SsUtSetReverbDepth(0x10, 0x10);
    audio_state.sequence_buffer = memory_allocate(0x3000);
    audio_state.sequence_active = 0;
    index = 9;
    do {
        audio_state.voice_slots.voice_ids[index] = -1;
    } while (--index >= 0);
}

ADDRESS(0x80032984, 0xc8)
void audio_load_vab(u8 *vab_header, u8 *vab_body)
{
    audio_stop_sequence_fade();
    audio_state.active_vab_id = func_800446a8(vab_header, -1);
    if (audio_state.active_vab_id == -1) {
        printf(DAT_80012a14);
        return;
    }
    audio_state.vab_header = vab_header;
    audio_state.active_vab_id = SsVabTransBody(vab_body, audio_state.active_vab_id);
    if (audio_state.active_vab_id == -1) {
        printf(DAT_80012a30);
        return;
    }
    SsVabTransCompleted(1);
}

ADDRESS(0x80032a4c, 0x110)
void audio_play_map_sequence(u8 sequence_id)
{
    char path[20] = "B0\\SND0.SEQ";

    audio_stop_sequence_fade();
    if (audio_music_enabled) {
        path[6] = sequence_id + '0';
        path[1] = player_progress_state.current_floor + '0';
        if (func_8001af9c(audio_state.sequence_buffer, path) == 0) {
            audio_state.sequence_id =
                func_800468d8(audio_state.sequence_buffer, audio_state.active_vab_id);
            SsSeqSetVol(audio_state.sequence_id, 0x4b, 0x4b);
            SsSeqPlay(audio_state.sequence_id, 1, 0);
            audio_state.sequence_active = 1;
        }
    }
}

ADDRESS(0x80032b5c, 0x80)
void audio_stop_sequence_fade(void)
{
    s32 volume;

    if (audio_state.sequence_active == 1) {
        volume = 0x4b;
        do {
            func_800555e0(0);
            SsSeqSetVol(audio_state.sequence_id, volume, volume);
        } while (--volume >= 0);
        func_8004a344(audio_state.sequence_id);
        func_8004b6e0(audio_state.sequence_id);
        audio_state.sequence_active = 0;
    }
}

ADDRESS(0x80032bdc, 0x9c)
void audio_stop_sequence_master_fade(s32 fade_step)
{
    s32 volume;

    if (audio_state.sequence_active == 1) {
        volume = 0x4b00;
        do {
            func_800555e0(0);
            SsSetMVol(volume >> 8, volume >> 8);
            volume -= fade_step;
        } while (volume > 0);
        SsSetMVol(0, 0);
        SsSeqSetVol(audio_state.sequence_id, 0, 0);
        func_8004a344(audio_state.sequence_id);
        func_8004b6e0(audio_state.sequence_id);
        audio_state.sequence_active = 0;
    }
}

ADDRESS(0x80032c78, 0x38)
void audio_shutdown(void)
{
    audio_close_vab();
    func_8004b6e0(audio_state.sequence_id);
    SsEnd();
}

ADDRESS(0x80032cb0, 0x40)
void audio_close_vab(void)
{
    s16 *vab_id = &audio_state.active_vab_id;

    SsVabClose(*vab_id);
    *vab_id = -1;
    audio_state.vab_header = 0;
}

ADDRESS(0x80032cf0, 0x2c8)
u32 audio_play_spatial(
    const SoundRef *sound,
    const struct KfVec4i *position,
    s16 volume,
    s32 max_distance,
    s32 attenuation_distance)
{
    s32 delta_x = (position->x - audio_state.listener_position.x) >> 3;
    s32 delta_y = (position->y - audio_state.listener_position.y) >> 3;
    s32 delta_z = (position->z - audio_state.listener_position.z) >> 3;
    s32 distance;
    s32 attenuation;
    s32 level;
    s32 angle;
    s32 left;
    s32 right;

    distance = SquareRoot0(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z) << 3;
    if (distance >= max_distance) {
        return 0;
    }
    attenuation = ((attenuation_distance - distance) << 7) / attenuation_distance;
    level = (attenuation * volume) >> 7;
    if (level < 0) {
        level = 0;
    } else if (level >= 128) {
        level = 127;
    }
    angle = vector_xz_to_angle(
        position->x - audio_state.listener_position.x,
        audio_state.listener_position.z - position->z);
    angle = (angle - audio_state.listener_rotation.y + 1024) & 0xfff;
    if (angle >= 2048) {
        angle = 4096 - angle;
    }
    angle >>= 1;
    if ((sound->tone & 0x80) == 1) {
        attenuation += 36;
        if (attenuation >= 128) {
            attenuation = 127;
        }
    }
    if (attenuation >= 64) {
        angle = (((angle - 512) * (256 - attenuation * 2)) >> 7) + 512;
    }
    left = (level * rsin(angle)) / 3000;
    if (left >= 128) {
        left = 127;
    }
    right = (level * rcos(angle)) / 3000;
    if (right >= 128) {
        right = 127;
    }
    audio_play_voice(
        audio_state.active_vab_id,
        sound->program,
        sound->tone & 0xf,
        sound->note,
        left,
        right);
    return 1;
}

ADDRESS(0x80032fb8, 0x30)
void audio_play_spatial_default_range(
    const SoundRef *sound,
    const struct KfVec4i *position,
    s16 volume)
{
    audio_play_spatial(sound, position, volume, 0x3e80, 0x6d60);
}

ADDRESS(0x80032fe8, 0x2c)
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

ADDRESS(0x80033014, 0x28)
void audio_key_off_mask(const u8 *voice_mask)
{
    SsVoKeyOff(voice_mask[0], voice_mask[2] << 8);
}

ADDRESS(0x8003303c, 0x70)
void audio_set_listener_transform(
    const struct KfVec4i *position_or_null,
    const struct KfVec4s *rotation_or_null)
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
    if (!audio_effects_enabled) {
        return;
    }
    audio_voice_slot_index++;
    if (audio_voice_slot_index == 10) {
        audio_voice_slot_index = 0;
    }
    if (audio_state.voice_slots.voice_ids[audio_voice_slot_index] != -1) {
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

    first &= 0xfff;
    second &= 0xfff;
    difference = second - first;
    signed_difference = difference;
    if (signed_difference >= 2048) {
        return difference - 4096;
    }
    if (signed_difference < -2047) {
        return difference + 4096;
    }
    return signed_difference;
}
