#include <kf/lib/codec.h>
#include <kf/audio/codec.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <memory>
#include <vector>

static void word(std::vector<uint8_t> &bytes, size_t at, uint32_t value)
{
    for (unsigned i = 0; i < 4; ++i) bytes[at + i] = value >> (8 * i);
}

static void audio_records()
{
    std::vector<uint8_t> header(32 + 128 * 16 + 16 * 32 + 256 * 2);
    std::array<uint8_t, 8> body {};
    word(header, 0, 0x56414270); word(header, 4, 7);
    word(header, 12, header.size() + body.size());
    word(header, 16, 1u << 16); word(header, 20, (1u << 16) | 1);
    header[24] = 99; header[25] = 63;
    header[32] = 1; header[33] = 79; header[34] = 5; header[36] = 62;
    const size_t tone = 32 + 128 * 16;
    header[tone + 2] = 89; header[tone + 4] = 60;
    header[tone + 5] = 64; header[tone + 7] = 127;
    word(header, tone + 20, 1u << 16);
    header[tone + 16 * 32 + 2] = 1;
    auto bank = std::make_unique<KfAudioBankData>();
    for (size_t size = 0; size < 32; ++size)
        assert(kf_audio_bank_decode(header.data(), size, body.data(), body.size(), bank.get()) == KF_CODEC_INVALID);
    header.insert(header.begin(), 0);
    assert(kf_audio_bank_decode(header.data() + 1, header.size() - 1, body.data(), body.size(), bank.get()) == KF_CODEC_OK);
    assert(bank->volume == 99 && bank->pan == 63 && bank->sample_count == 1);
    assert(bank->programs[0].volume == 79 && bank->programs[0].priority == 5);
    assert(bank->programs[0].tones[0].volume == 89 && bank->programs[0].tones[0].sample_index == 0);
    assert(bank->samples[0].offset == 0 && bank->samples[0].size == body.size());
    word(header, 1 + tone + 16, 0xb52fdba7);
    assert(kf_audio_bank_decode(header.data() + 1, header.size() - 1, body.data(), body.size(), bank.get()) == KF_CODEC_OK);
    const auto &envelope = bank->programs[0].tones[0].envelope;
    assert(envelope.attack_shift == 22 && envelope.attack_step == 3 && envelope.decay_shift == 10);
    assert(envelope.sustain_shift == 21 && envelope.sustain_step == 0 && envelope.release_shift == 15);
    assert(envelope.sustain_level == 16384 && envelope.attack_exponential == 1);
    assert(envelope.sustain_exponential == 1 && envelope.sustain_decreasing == 0 && envelope.release_exponential == 1);
    word(header, 1 + tone + 16, 0xffffffff);
    assert(kf_audio_bank_decode(header.data() + 1, header.size() - 1, body.data(), body.size(), bank.get()) == KF_CODEC_OK);
    assert(envelope.attack_shift == 31 && envelope.attack_step == 3 && envelope.decay_shift == 15);
    assert(envelope.sustain_shift == 31 && envelope.sustain_step == 3 && envelope.release_shift == 31);
    assert(envelope.sustain_level == 32767 && envelope.sustain_decreasing == 1);

    // An odd source address, big-endian resolution and a three-byte tempo.
    const std::array<uint8_t, 20> sequence {
        0, 'p', 'Q', 'E', 'S', 0, 0, 0, 1, 0x01, 0x23,
        0x01, 0x23, 0x45, 4, 2, 1, 0xff, 0x2f, 0,
    };
    KfMusicInfo info {};
    KfMusicEvent event {};
    for (size_t size = 0; size < sequence.size() - 1; ++size)
        assert(kf_music_decode(sequence.data() + 1, size, &event, 1, &info) == KF_CODEC_INVALID);
    assert(kf_music_decode(sequence.data() + 1, sequence.size() - 1, &event, 1, &info) == KF_CODEC_OK);
    assert(info.resolution == 0x0123 && info.tempo == 0x012345 && info.event_count == 1);
    assert(event.kind == KfMusicEventKind::End && event.delta == 1);
    auto unsupported = sequence;
    unsupported[17] = 0xa0;
    assert(kf_music_decode(unsupported.data() + 1, unsupported.size() - 1, &event, 1, &info) == KF_CODEC_INVALID);
}

static void tim_records()
{
    std::vector<uint8_t> tim(24);
    word(tim, 0, 0x10); word(tim, 4, 2); word(tim, 8, 16); word(tim, 16, (1u << 16) | 2);
    KfTimInfo info {};
    assert(kf_tim_info(tim.data(), tim.size(), 0, &info) == KF_CODEC_OK);
    assert(kf_tim_info(tim.data(), tim.size() - 1, 0, &info) == KF_CODEC_INVALID);
    assert(kf_tim_info(tim.data(), tim.size(), tim.size(), &info) == KF_CODEC_END);
    std::array<uint8_t, 8> rgba {};
    assert(kf_tim_rgba(tim.data(), tim.size(), 0, 0, rgba.data(), rgba.size() - 1) == KF_CODEC_OUTPUT_FULL);
    word(tim, 16, 0x0001ffff);
    assert(kf_tim_info(tim.data(), tim.size(), 0, &info) == KF_CODEC_INVALID);
}

int main()
{
    audio_records();
    tim_records();
}
