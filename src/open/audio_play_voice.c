#include <kf/address.h>
#include <kf/audio.h>
#include <kf/open_audio.h>
#include <psyq/audio.h>

ADDRESS(0x8001a268, 0x194)
void audio_play_voice(
    s16 vab_id, s16 program, s16 tone, s16 note, s16 left_volume, s16 right_volume)
{
    if (program == 0 && tone == 0 && note == 0) {
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

DATA(0x80037304, 0x4)
s32 audio_voice_slot_index = KF_AUDIO_VOICE_SLOTS - 1;
