#include <kf/null.h>
#include <kf/address.h>
#include <kf/cd_file.h>
#include <kf/memory.h>
#include <kf/open_audio.h>
#include <psyq/audio.h>
#include <psyq/libc.h>

enum {
    OPEN_SEQUENCE_BUFFER_BYTES = 0x4800,
    OPEN_SEQUENCE_VOLUME = 0x50,
    OPEN_REVERB_DEPTH = 0x30,
    OPEN_VAB_SETTLE_FRAMES = 100,
    OPEN_SEQUENCE_FADE_IN_STEP = 8,
    OPEN_SEQUENCE_FADE_OUT_STEP = 4
};

RODATA(0x8001227c, 0x32)

DATA(0x80037808, 0x158)
static char audio_sequence_table[SS_SEQ_TABSIZ * KF_AUDIO_SEQUENCE_CAPACITY];

DATA(0x8006e1d0, 0x90)
KfAudioState audio_state;

ADDRESS(0x80019ba4, 0xb8)
void audio_initialize(void)
{
    s32 index;
    s16 inactive_voice_id;

    SsInit();
    SsSetTableSize(audio_sequence_table,
        KF_AUDIO_SEQUENCE_CAPACITY, KF_AUDIO_TRACKS_PER_SEQUENCE);
    SsSetTickMode(SS_TICK60);
    SsStart();
    SsSetMVol(0, 0);
    SsUtSetReverbType(SS_REV_TYPE_HALL);
    SsUtReverbOn();
    SsUtSetReverbDepth(OPEN_REVERB_DEPTH, OPEN_REVERB_DEPTH);
    audio_state.sequence_buffer = (u_long *)memory_allocate(OPEN_SEQUENCE_BUFFER_BYTES);
    audio_state.sequence_active = KF_AUDIO_SEQUENCE_INACTIVE;
    inactive_voice_id = KF_AUDIO_VOICE_INACTIVE;
    index = KF_AUDIO_VOICE_SLOTS - 1;
    do {
        audio_state.voice_slots.voice_ids[index] = inactive_voice_id;
    } while (--index >= 0);
    audio_state.active_vab_id = KF_AUDIO_VAB_UNAVAILABLE;
    audio_state.sequence_active = KF_AUDIO_SEQUENCE_INACTIVE;
}

ADDRESS(0x80019c5c, 0xf4)
void audio_load_vab(u8 *vab_header, u8 *vab_body)
{
    s32 frame;

    audio_stop_sequence(KF_AUDIO_STOP_IMMEDIATE);
    SsSetMVol(0, 0);
    audio_state.active_vab_id = SsVabOpenHead(vab_header, KF_AUDIO_VAB_AUTO);
    if (audio_state.active_vab_id == KF_AUDIO_VAB_UNAVAILABLE) {
        printf("VAB headder open failed\n");
        exit(0);
    }
    audio_state.vab_header = vab_header;
    audio_state.active_vab_id =
        SsVabTransBody(vab_body, audio_state.active_vab_id);
    if (audio_state.active_vab_id == KF_AUDIO_VAB_UNAVAILABLE) {
        printf("VAB body open failed\n");
        exit(0);
    }
    SsVabTransCompleted(SS_WAIT_COMPLETED);
    frame = OPEN_VAB_SETTLE_FRAMES - 1;
    do {
        VSync(0);
    } while (--frame != -1);
}

ADDRESS(0x80019d50, 0xd4)
void audio_play_sequence_file(const char *path)
{
    s32 volume;

    audio_stop_sequence(KF_AUDIO_STOP_IMMEDIATE);
    if (cd_file_load_into(audio_state.sequence_buffer, path) != KF_RESOURCE_LOADED) {
        return;
    }
    audio_state.sequence_id = SsSeqOpen(
        audio_state.sequence_buffer, audio_state.active_vab_id);
    SsSeqSetVol(audio_state.sequence_id, OPEN_SEQUENCE_VOLUME, OPEN_SEQUENCE_VOLUME);
    SsSetMVol(0, 0);
    SsSeqPlay(audio_state.sequence_id, SSPLAY_PLAY, SSPLAY_INFINITY);
    volume = 0;
    do {
        VSync(0);
        SsSetMVol(volume, volume);
        volume += OPEN_SEQUENCE_FADE_IN_STEP;
    } while (volume < KF_AUDIO_MAX_VOLUME);
    SsSetMVol(KF_AUDIO_MAX_VOLUME, KF_AUDIO_MAX_VOLUME);
    audio_state.sequence_active = KF_AUDIO_SEQUENCE_ACTIVE;
}

ADDRESS(0x80019e24, 0x9c)
void audio_stop_sequence(KfAudioStopMode mode)
{
    s32 volume;

    if (audio_state.sequence_active == KF_AUDIO_SEQUENCE_ACTIVE) {
        if (mode == KF_AUDIO_STOP_FADE) {
            volume = KF_AUDIO_MAX_VOLUME;
            do {
                VSync(0);
                SsSetMVol(volume, volume);
                volume -= OPEN_SEQUENCE_FADE_OUT_STEP;
            } while (volume >= 0);
        }
        SsSetMVol(0, 0);
        SsSeqSetVol(audio_state.sequence_id, 0, 0);
        SsSeqStop(audio_state.sequence_id);
        SsSeqClose(audio_state.sequence_id);
        audio_state.sequence_active = KF_AUDIO_SEQUENCE_INACTIVE;
    }
}

ADDRESS(0x80019ec0, 0x38)
void audio_shutdown(void)
{
    audio_close_vab();
    SsSeqClose(audio_state.sequence_id);
    SsEnd();
}

ADDRESS(0x80019ef8, 0x4c)
void audio_close_vab(void)
{
    s16 *vab_id = &audio_state.active_vab_id;

    if (*vab_id != KF_AUDIO_VAB_UNAVAILABLE) {
        SsVabClose(*vab_id);
    }
    *vab_id = KF_AUDIO_VAB_UNAVAILABLE;
    audio_state.vab_header = NULL;
}
