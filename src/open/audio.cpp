#include <kf/open/resources.h>
#include <kf/lib/null.h>

#include <kf/lib/resource_file.h>
#include <kf/lib/memory.h>
#include <kf/open/audio.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

enum {
    OPEN_SEQUENCE_BUFFER_BYTES = 0x4800,
    OPEN_SEQUENCE_VOLUME = 0x50,
    OPEN_REVERB_DEPTH = 0x30,
    OPEN_VAB_SETTLE_FRAMES = 100,
    OPEN_SEQUENCE_FADE_IN_STEP = 8,
    OPEN_SEQUENCE_FADE_OUT_STEP = 4
};

KfAudioState audio_state;
s32 audio_voice_slot_index = KF_AUDIO_VOICE_SLOTS - 1;

void audio_initialize(void)
{
    kf::sound_reset(kf::ReverbPreset::Hall, OPEN_REVERB_DEPTH, OPEN_REVERB_DEPTH);
    kf::sound_master_volume(0, 0);
    audio_state.bank = nullptr;
    audio_state.sequence = nullptr;
    audio_voice_slot_index = KF_AUDIO_VOICE_SLOTS - 1;
    audio_state.sequence_buffer = (u8 *)memory_allocate(memory_arena, OPEN_SEQUENCE_BUFFER_BYTES);
    audio_state.sequence_active = KF_AUDIO_SEQUENCE_INACTIVE;
    audio_reset_voice_slots(audio_state);
}

void audio_load_vab(KfAudioBankResource resource)
{
    audio_stop_sequence(KF_AUDIO_STOP_IMMEDIATE);
    kf::sound_master_volume(0, 0);
    audio_close_vab(audio_state);
    audio_state.bank = kf::sound_bank_load(resource.header, resource.header_size, resource.body, resource.body_size);
    if (!audio_state.bank)
        kf::host_fail("Cannot decode sound bank");
    s32 frame = OPEN_VAB_SETTLE_FRAMES - 1;
    do {
        kf::host_wait_frame();
    } while (--frame != -1);
}

void audio_play_sequence_file(const char *path)
{
    s32 volume;
    std::size_t sequence_size;

    audio_stop_sequence(KF_AUDIO_STOP_IMMEDIATE);
    if (resource_file_load_into(audio_state.sequence_buffer, OPEN_SEQUENCE_BUFFER_BYTES, path, &sequence_size) != KF_RESOURCE_LOADED) {
        return;
    }
    audio_state.sequence = kf::sound_sequence_load(audio_state.sequence_buffer, sequence_size, audio_state.bank);
    if (!audio_state.sequence)
        kf::host_fail("Cannot decode music sequence");
    kf::sound_sequence_volume(audio_state.sequence, OPEN_SEQUENCE_VOLUME, OPEN_SEQUENCE_VOLUME);
    kf::sound_master_volume(0, 0);
    kf::sound_sequence_play(audio_state.sequence);
    volume = 0;
    do {
        kf::host_wait_frame();
        kf::sound_master_volume(volume, volume);
        volume += OPEN_SEQUENCE_FADE_IN_STEP;
    } while (volume < KF_AUDIO_MAX_VOLUME);
    kf::sound_master_volume(KF_AUDIO_MAX_VOLUME, KF_AUDIO_MAX_VOLUME);
    audio_state.sequence_active = KF_AUDIO_SEQUENCE_ACTIVE;
}

void audio_stop_sequence(KfAudioStopMode stop_mode)
{
    s32 volume;

    if (audio_state.sequence_active == KF_AUDIO_SEQUENCE_ACTIVE) {
        if (stop_mode == KF_AUDIO_STOP_FADE) {
            volume = KF_AUDIO_MAX_VOLUME;
            do {
                kf::host_wait_frame();
                kf::sound_master_volume(volume, volume);
                volume -= OPEN_SEQUENCE_FADE_OUT_STEP;
            } while (volume >= 0);
        }
        kf::sound_master_volume(0, 0);
        kf::sound_sequence_volume(audio_state.sequence, 0, 0);
        audio_release_sequence(audio_state);
    }
}

void audio_reset_module_state(void)
{
    kf::restore_initial_value<audio_state>();
    kf::restore_initial_value<audio_voice_slot_index>();
}

KfAudioPlayback audio_playback()
{
    return {audio_state, audio_voice_slot_index, true};
}
