#pragma once
#include <kf/lib/codec.h>
#ifdef __cplusplus
extern "C" {
#endif

enum { KF_AUDIO_PROGRAM_COUNT = 128, KF_AUDIO_TONE_COUNT = 16, KF_AUDIO_SAMPLE_COUNT = 256 };
enum { KF_AUDIO_ADPCM_BLOCK_BYTES = 16, KF_AUDIO_ADPCM_BLOCK_FRAMES = 28 };
typedef struct KfAudioEnvelope {
    uint8_t attack_shift, attack_step, decay_shift, sustain_shift, sustain_step, release_shift;
    uint16_t sustain_level;
    uint8_t attack_exponential, sustain_exponential, sustain_decreasing, release_exponential;
} KfAudioEnvelope;
typedef struct KfAudioTone {
    uint8_t priority, reverb, volume, pan, center_note, center_shift, minimum_note, maximum_note;
    uint8_t vibrato_width, vibrato_time, portamento_width, portamento_time, bend_down, bend_up;
    uint16_t sample_index;
    KfAudioEnvelope envelope;
} KfAudioTone;
typedef struct KfAudioProgram {
    uint8_t tone_count, volume, pan, priority;
    KfAudioTone tones[KF_AUDIO_TONE_COUNT];
} KfAudioProgram;
typedef struct KfAudioSampleRange { uint32_t offset, size; } KfAudioSampleRange;
typedef struct KfAudioBankData {
    uint8_t volume, pan;
    uint16_t sample_count;
    KfAudioProgram programs[KF_AUDIO_PROGRAM_COUNT];
    KfAudioSampleRange samples[KF_AUDIO_SAMPLE_COUNT];
} KfAudioBankData;
typedef struct KfAudioSampleInfo {
    uint32_t frames, loop_begin, loop_end;
} KfAudioSampleInfo;
typedef struct KfAudioPredictor { int32_t previous, older; } KfAudioPredictor;

#ifdef __cplusplus
enum class KfMusicEventKind : uint32_t {
    NoteOff, NoteOn, Volume, Program, PitchBend, Tempo, End
};
#else
typedef enum KfMusicEventKind {
    KF_MUSIC_NOTE_OFF, KF_MUSIC_NOTE_ON, KF_MUSIC_VOLUME, KF_MUSIC_PROGRAM,
    KF_MUSIC_PITCH_BEND, KF_MUSIC_TEMPO, KF_MUSIC_END
} KfMusicEventKind;
#endif
typedef struct KfMusicEvent {
    uint32_t delta;
    KfMusicEventKind kind;
    uint32_t value;
    uint8_t channel, note, velocity, reserved;
} KfMusicEvent;
typedef struct KfMusicInfo { uint32_t resolution, tempo, event_count; } KfMusicInfo;

// Inputs and caller-owned outputs are disjoint. No result retains input pointers.
KfCodecResult kf_audio_bank_decode(const uint8_t *header, size_t header_size,
    const uint8_t *body, size_t body_size, KfAudioBankData *output);
// Positions are mono frames; zero loop_end denotes a one-shot sample.
KfCodecResult kf_audio_sample_info(const uint8_t *data, size_t size, KfAudioSampleInfo *info);
// Zero predictor state at key-on, not at loop jumps. Decode 16 bytes to 28 frames.
KfCodecResult kf_audio_decode_block(const uint8_t *data, size_t size,
    KfAudioPredictor *predictor, int16_t *pcm, size_t capacity);
// Pass null events and zero capacity to obtain the required event count.
KfCodecResult kf_music_decode(const uint8_t *data, size_t size,
    KfMusicEvent *events, size_t capacity, KfMusicInfo *info);
#ifdef __cplusplus
}
static_assert(sizeof(KfAudioEnvelope) == 12 && sizeof(KfAudioTone) == 28);
static_assert(sizeof(KfMusicEvent) == 16);
#endif
