#include <kf/address.h>
#include <kf/cd_file.h>
#include <kf/memory.h>
#include <kf/open_audio.h>
#include <kf/psyq_audio.h>
#include <kf/psyq_libc.h>

RODATA(0x8001227c, 0x34)

DATA(0x80037808, 0x158)
u8 audio_sequence_table[SS_SEQ_TABSIZ * 2];

DATA(0x8006e1d0, 0x90)
KfAudioState audio_state;

ADDRESS(0x80019ba4, 0xb8)
void audio_initialize(void)
{
    s32 index;
    s16 inactive_voice_id;

    SsInit();
    SsSetTableSize((char *)audio_sequence_table, 2, 1);
    SsSetTickMode(1);
    SsStart();
    SsSetMVol(0, 0);
    SsUtSetReverbType(5);
    SsUtReverbOn();
    SsUtSetReverbDepth(0x30, 0x30);
    audio_state.sequence_buffer = memory_allocate(0x4800);
    audio_state.sequence_active = 0;
    inactive_voice_id = -1;
    index = 9;
    do {
        audio_state.voice_slots.voice_ids[index] = inactive_voice_id;
    } while (--index >= 0);
    audio_state.active_vab_id = -1;
    audio_state.sequence_active = 0;
}

ADDRESS(0x80019c5c, 0xf4)
void audio_load_vab(u8 *vab_header, u8 *vab_body)
{
    s32 frame;

    audio_stop_sequence(0);
    SsSetMVol(0, 0);
    audio_state.active_vab_id = SsVabOpenHead(vab_header, -1);
    if (audio_state.active_vab_id == -1) {
        printf("VAB headder open failed\n");
        exit(0);
    }
    audio_state.vab_header = vab_header;
    audio_state.active_vab_id =
        SsVabTransBody(vab_body, audio_state.active_vab_id);
    if (audio_state.active_vab_id == -1) {
        printf("VAB body open failed\n");
        exit(0);
    }
    SsVabTransCompleted(1);
    frame = 99;
    do {
        VSync(0);
    } while (--frame != -1);
}

ADDRESS(0x80019d50, 0xd4)
void audio_play_sequence_file(const char *path)
{
    s32 volume;

    audio_stop_sequence(0);
    if (cd_file_load_into(audio_state.sequence_buffer, path) != 0) {
        return;
    }
    audio_state.sequence_id = SsSeqOpen(
        (u32 *)audio_state.sequence_buffer, audio_state.active_vab_id);
    SsSeqSetVol(audio_state.sequence_id, 0x50, 0x50);
    SsSetMVol(0, 0);
    SsSeqPlay(audio_state.sequence_id, 1, 0);
    volume = 0;
    do {
        VSync(0);
        SsSetMVol(volume, volume);
        volume += 8;
    } while (volume < 0x7f);
    SsSetMVol(0x7f, 0x7f);
    audio_state.sequence_active = 1;
}

ADDRESS(0x80019e24, 0x9c)
void audio_stop_sequence(s32 fade)
{
    s32 volume;

    if (audio_state.sequence_active == 1) {
        if (fade == 1) {
            volume = 0x7f;
            do {
                VSync(0);
                SsSetMVol(volume, volume);
                volume -= 4;
            } while (volume >= 0);
        }
        SsSetMVol(0, 0);
        SsSeqSetVol(audio_state.sequence_id, 0, 0);
        SsSeqStop(audio_state.sequence_id);
        SsSeqClose(audio_state.sequence_id);
        audio_state.sequence_active = 0;
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

    if (*vab_id != -1) {
        SsVabClose(*vab_id);
    }
    *vab_id = -1;
    audio_state.vab_header = 0;
}
