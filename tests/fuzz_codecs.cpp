#include <kf/audio/codec.h>
#include <memory>
extern "C" int LLVMFuzzerTestOneInput(const u8 *data, std::size_t size) {
    const std::span<const u8> bytes(data, size);
    KfTimInfo tim{};
    kf_tim_info(bytes, 0, tim);
    std::array<u8, 4096> rgba{};
    kf_tim_rgba(bytes, 0, 0, rgba);
    auto words = std::make_unique<std::array<u16, 1024 * 512>>();
    kf_tim_compose(bytes, *words);
    KfAssetInfo asset{};
    kf_asset_info(bytes, asset);
    KfAnimationData animation;
    kf_animation_decode(bytes, 4096, animation);
    KfPlacementLimits limits{100, 12, 2000};
    std::size_t count = 0;
    std::array<KfActorPlacementData, 16> actors{};
    std::array<KfObjectPlacementData, 16> objects{};
    std::array<KfEventPlacementData, 16> events{};
    kf_actor_placements_decode(bytes, limits, actors, count);
    kf_object_placements_decode(bytes, limits, objects, count);
    kf_event_placements_decode(bytes, limits, events, count);
    auto bank = std::make_unique<KfAudioBankData>();
    kf_audio_bank_decode(bytes, bytes, *bank);
    KfAudioSampleInfo sample{};
    kf_audio_sample_info(bytes, sample);
    KfAudioPredictor predictor{};
    std::array<s16, 28> pcm{};
    kf_audio_decode_block(bytes, predictor, pcm);
    KfMusicInfo music{};
    std::vector<KfMusicEvent> notes;
    kf_music_decode(bytes, notes, music);
    return 0;
}
