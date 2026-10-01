#ifndef KF_GAME_AUDIO_H
#define KF_GAME_AUDIO_H

struct PlayerContext;
struct WorldState;
#include <kf/platform/frame_task.hpp>
#include <kf/lib/audio.h>

extern KfAudioState audio_state;
extern s32 audio_voice_slot_index;
KfAudioPlayback audio_playback(PlayerContext &player);

// Bind only around synchronous simulation entry points, never across a yield.
struct PartyAudioScope {
    WorldState *previous;
    explicit PartyAudioScope(WorldState &world);
    ~PartyAudioScope();
    PartyAudioScope(const PartyAudioScope &) = delete;
    PartyAudioScope &operator=(const PartyAudioScope &) = delete;
};
void party_audio_receive(WorldState &world, u32 &last_sequence, bool baseline);
void party_audio_update_listener(WorldState &world);

extern void audio_initialize(void);
extern kf::FrameTask<void> audio_load_vab(KfAudioBankResource resource);
extern kf::FrameTask<void> audio_play_map_sequence(PlayerContext &player, u8 sequence_index);
extern kf::FrameTask<void> audio_play_current_map_sequence(PlayerContext &player);
extern KfAudioPlaybackResult audio_play_spatial(PlayerContext &player,
    const SoundRef *sound, const VECTOR *position, s16 volume, s32 max_distance,
    s32 attenuation_distance);
extern KfAudioPlaybackResult audio_play_spatial_default_range(PlayerContext &player,
    const SoundRef *sound, const VECTOR *position, s16 volume);
extern KfAudioPlaybackResult audio_play_spatial_range(PlayerContext &player,
    const SoundRef *sound, const VECTOR *position, s16 volume,
    s32 max_distance, s32 attenuation_distance);
extern kf::FrameTask<void> audio_stop_sequence_fade(void);
extern kf::FrameTask<void> audio_stop_sequence_master_fade(s32 fade_step);

#endif // KF_GAME_AUDIO_H
