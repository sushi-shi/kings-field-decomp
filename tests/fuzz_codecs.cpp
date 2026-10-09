#include <kf/audio/codec.h>

#include <memory>

// Malformed input is expected; unexpected exceptions must still fail the run.
template<class Operation>
static void fuzz_decode(Operation operation)
{
    try {
        operation();
    } catch (const kf::codec::Error &) {
    }
}

extern "C" int LLVMFuzzerTestOneInput(const u8 *data, std::size_t size)
{
    const std::span<const u8> bytes(data, size);
    std::array<u8, 4096> rgba{};
    fuzz_decode([&] {
        if (const auto tim = KfTimImage::parse(bytes)) {
            tim->info();
            tim->rgba(0, rgba);
        }
    });
    auto words = std::make_unique<std::array<u16, 1024 * 512>>();
    fuzz_decode([&] { kf_tim_compose(bytes, *words); });
    fuzz_decode([&] { KfAsset(bytes).animation(4096); });
    KfPlacementLimits limits{100, 12, 2000};
    std::array<KfActorPlacementData, 16> actors{};
    std::array<KfObjectPlacementData, 16> objects{};
    std::array<KfEventPlacementData, 16> events{};
    fuzz_decode([&] { kf_actor_placements_decode(bytes, limits, actors); });
    fuzz_decode([&] { kf_object_placements_decode(bytes, limits, objects); });
    fuzz_decode([&] { kf_event_placements_decode(bytes, limits, events); });
    auto bank = std::make_unique<KfAudioBankData>();
    fuzz_decode([&] { kf_audio_bank_decode(bytes, bytes, *bank); });
    fuzz_decode([&] { kf_audio_sample_info(bytes); });
    KfAudioPredictor predictor{};
    std::array<s16, 28> pcm{};
    fuzz_decode([&] { kf_audio_decode_block(bytes, predictor, pcm); });
    KfMusicInfo music{};
    std::vector<KfMusicEvent> notes;
    fuzz_decode([&] { kf_music_decode(bytes, notes, music); });
    return 0;
}
