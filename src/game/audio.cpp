#include <kf/game/system.h>
#include <kf/game/player.h>
#include <kf/game/world.h>
#include <algorithm>
#include <kf/game/resources.h>
#include <kf/lib/null.h>

#include <kf/lib/math.h>
#include <kf/game/audio.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <kf/game/game.h>

enum {
    GAME_SEQUENCE_BUFFER_BYTES = 0x3000,
    GAME_SEQUENCE_VOLUME = 0x4b,
    GAME_REVERB_DEPTH = 0x10,
    GAME_SOUND_PAN_NARROWING_GAIN_BOOST = 36,
    GAME_SOUND_PAN_NARROW_THRESHOLD = 64,
    GAME_SOUND_PAN_DIVISOR = 3000
};

s32 audio_voice_slot_index = 9;

KfAudioState audio_state;

static WorldState *audio_world;

PartyAudioScope::PartyAudioScope(WorldState &world) : previous(audio_world)
{
    audio_world = &world;
    if (world.party.enabled && !world.prediction) party_audio_update_listener(world);
}
PartyAudioScope::~PartyAudioScope() { audio_world = previous; }

static bool party_audio_record(const SoundRef &sound, const VECTOR &position, s16 volume,
    s32 max_distance, s32 attenuation_distance)
{
    if (!audio_world || !audio_world->party.enabled || audio_world->prediction ||
        audio_world->sound_sequence == UINT32_MAX) return false;
    if ((!sound.program && !sound.tone_and_flags && !sound.note) || sound.program > 127 ||
        sound.tone_and_flags > 15 || sound.note > 127 || volume <= 0 ||
        max_distance <= 0 || max_distance > 60000 || attenuation_distance < max_distance || attenuation_distance > 60000 ||
        std::abs(std::int64_t(position.vx)) > 1000000 || std::abs(std::int64_t(position.vy)) > 1000000 ||
        std::abs(std::int64_t(position.vz)) > 1000000) return false;
    const u32 sequence = ++audio_world->sound_sequence;
    audio_world->sounds[sequence % KF_WORLD_SOUNDS] = {sequence, position.vx, position.vy, position.vz,
        sound.program, sound.tone_and_flags, sound.note, static_cast<u8>(std::min<int>(volume, 127)),
        max_distance, attenuation_distance};
    return true;
}

void audio_initialize(void)
{
    kf::sound_reset(kf::ReverbPreset::Studio, GAME_REVERB_DEPTH, GAME_REVERB_DEPTH);
    kf::sound_master_volume(KF_AUDIO_MAX_VOLUME, KF_AUDIO_MAX_VOLUME);
    audio_state.bank = nullptr;
    audio_state.sequence = nullptr;
    audio_voice_slot_index = KF_AUDIO_VOICE_SLOTS - 1;
    audio_state.sequence_buffer = (u8 *)memory_allocate(memory_arena, GAME_SEQUENCE_BUFFER_BYTES);
    audio_state.sequence_active = KF_AUDIO_SEQUENCE_INACTIVE;
    audio_reset_voice_slots(audio_state);
}

kf::FrameTask<void> audio_load_vab(KfAudioBankResource resource)
{
    (co_await audio_stop_sequence_fade());
    audio_close_vab(audio_state);
    audio_state.bank = kf::sound_bank_load(resource.header, resource.header_size, resource.body, resource.body_size);
    if (!audio_state.bank)
        kf::host_fail("Cannot decode sound bank");
}

static constexpr unsigned sequence_path_capacity = 20;
static constexpr unsigned sequence_number_offset = 6, sequence_floor_offset = 1;

kf::FrameTask<void> audio_play_map_sequence(PlayerContext &player, u8 sequence_index)
{
    char path[sequence_path_capacity] = "B0/SND0.SEQ";
    std::size_t sequence_size;

    (co_await audio_stop_sequence_fade());
    if (player.state.audio_music_enabled != KF_PLAYER_OPTION_OFF) {
        path[sequence_number_offset] = sequence_index + '0';
        path[sequence_floor_offset] = kf_enum_encode<u8>(player.state.progress_state.current_floor) + '0';
        if (resource_file_load_into(audio_state.sequence_buffer, GAME_SEQUENCE_BUFFER_BYTES, path, &sequence_size) == KF_RESOURCE_LOADED) {
            audio_state.sequence = kf::sound_sequence_load(audio_state.sequence_buffer, sequence_size, audio_state.bank);
            if (!audio_state.sequence)
                kf::host_fail("Cannot decode music sequence");
            kf::sound_sequence_volume(audio_state.sequence, GAME_SEQUENCE_VOLUME, GAME_SEQUENCE_VOLUME);
            kf::sound_sequence_play(audio_state.sequence);
            audio_state.sequence_active = KF_AUDIO_SEQUENCE_ACTIVE;
        }
    }
}

kf::FrameTask<void> audio_stop_sequence_fade(void)
{
    s32 volume;

    if (audio_state.sequence_active == KF_AUDIO_SEQUENCE_ACTIVE) {
        volume = GAME_SEQUENCE_VOLUME;
        do {
            (co_await game_wait_frame());
            kf::sound_sequence_volume(audio_state.sequence, volume, volume);
        } while (--volume >= 0);
        audio_release_sequence(audio_state);
    }
}

kf::FrameTask<void> audio_stop_sequence_master_fade(s32 fade_step)
{
    s32 volume;

    if (audio_state.sequence_active == KF_AUDIO_SEQUENCE_ACTIVE) {
        volume = GAME_SEQUENCE_VOLUME << KF_FIXED8_BITS;
        do {
            (co_await game_wait_frame());
            kf::sound_master_volume(volume >> KF_FIXED8_BITS, volume >> KF_FIXED8_BITS);
            volume -= fade_step;
        } while (volume > 0);
        kf::sound_master_volume(0, 0);
        kf::sound_sequence_volume(audio_state.sequence, 0, 0);
        audio_release_sequence(audio_state);
    }
}

static KfAudioPlaybackResult audio_play_spatial_local(KfAudioPlayback playback,
    const SoundRef *sound,
    const VECTOR *position,
    s16 volume,
    s32 max_distance,
    s32 attenuation_distance)
{
    const auto dx = std::int64_t(position->vx) - audio_state.listener_position.vx;
    const auto dy = std::int64_t(position->vy) - audio_state.listener_position.vy;
    const auto dz = std::int64_t(position->vz) - audio_state.listener_position.vz;
    if (max_distance <= 0 || attenuation_distance <= 0 || std::abs(dx) >= max_distance ||
        std::abs(dy) >= max_distance || std::abs(dz) >= max_distance) return KF_AUDIO_NOT_PLAYED;
    const s32 delta_x = dx, delta_y = dy, delta_z = dz;
    s32 distance_gain_q7;
    s32 attenuated_volume;
    s32 angle;
    s32 left;
    s32 right;

    const s32 listener_distance = fixed_vector3_length(delta_x, delta_y, delta_z);
    if (listener_distance >= max_distance) {
        return KF_AUDIO_NOT_PLAYED;
    }
    distance_gain_q7 = ((attenuation_distance - listener_distance) << KF_FIXED7_BITS) / attenuation_distance;
    attenuated_volume = (distance_gain_q7 * volume) >> KF_FIXED7_BITS;
    attenuated_volume = std::clamp<s32>(attenuated_volume, 0, KF_AUDIO_MAX_VOLUME);
    angle = vector_xz_to_angle(
        position->vx - audio_state.listener_position.vx,
        audio_state.listener_position.vz - position->vz);
    angle = (angle - audio_state.listener_rotation.vy + KF_ANGLE_QUARTER_TURN)
        & KF_ANGLE_WRAP_MASK;
    if (angle >= KF_ANGLE_HALF_TURN) {
        angle = KF_ANGLE_FULL_TURN - angle;
    }
    angle >>= 1;
    // Retail compares the masked high bit to 1, so this branch never runs.
    // Preserve that quirk; changing it to a nonzero test changes panning.
    if ((sound->tone_and_flags & KF_SOUND_PAN_NARROWING_FLAG) == 1) {
        distance_gain_q7 += GAME_SOUND_PAN_NARROWING_GAIN_BOOST;
        distance_gain_q7 = std::min<s32>(distance_gain_q7, KF_AUDIO_MAX_VOLUME);
    }
    if (distance_gain_q7 >= GAME_SOUND_PAN_NARROW_THRESHOLD) {
        angle = (((angle - KF_ANGLE_EIGHTH_TURN)
            * (KF_FIXED7_ONE * 2 - distance_gain_q7 * 2)) >> KF_FIXED7_BITS)
            + KF_ANGLE_EIGHTH_TURN;
    }
    left = (attenuated_volume * kf::angle_sine(angle)) / GAME_SOUND_PAN_DIVISOR;
    left = std::min<s32>(left, KF_AUDIO_MAX_VOLUME);
    right = (attenuated_volume * kf::angle_cosine(angle)) / GAME_SOUND_PAN_DIVISOR;
    right = std::min<s32>(right, KF_AUDIO_MAX_VOLUME);
    audio_play_voice(playback,
        audio_state.bank,
        sound->program,
        sound->tone_and_flags & KF_SOUND_TONE_INDEX_MASK,
        sound->note,
        left,
        right);
    return KF_AUDIO_PLAYED;
}

KfAudioPlaybackResult audio_play_spatial(PlayerContext &player, const SoundRef *sound,
    const VECTOR *position, s16 volume, s32 max_distance, s32 attenuation_distance)
{
    const SoundRef selected {sound->program, static_cast<u8>(sound->tone_and_flags & KF_SOUND_TONE_INDEX_MASK), sound->note};
    party_audio_record(selected, *position, volume, max_distance, attenuation_distance);
    const auto result = audio_play_spatial_local(audio_playback(player), sound, position, volume, max_distance, attenuation_distance);
    // A shared effect emits once, even if the host cannot hear its position.
    return audio_world && audio_world->party.enabled ? KF_AUDIO_PLAYED : result;
}

static bool party_audio_direct(void *context, const SoundRef &sound, s16 volume)
{
    auto &player = *static_cast<PlayerContext *>(context);
    if (!audio_world || !audio_world->party.enabled || audio_world->prediction) return false;
    if (sound.tone_and_flags > KF_SOUND_TONE_INDEX_MASK) return false;
    party_audio_record(sound, player.state.camera_position, volume,
        KF_AUDIO_DEFAULT_MAX_DISTANCE, KF_AUDIO_DEFAULT_ATTENUATION_DISTANCE);
    if (player.local_view) return false;
    audio_play_spatial_local(audio_playback(player), &sound, &player.state.camera_position, volume,
        KF_AUDIO_DEFAULT_MAX_DISTANCE, KF_AUDIO_DEFAULT_ATTENUATION_DISTANCE);
    return true;
}

void party_audio_update_listener(WorldState &world)
{
    VECTOR position;
    SVECTOR rotation;
    player_update_transform_snapshot(party_view_player(world), &position, &rotation);
    audio_set_listener_transform(audio_state, &position, &rotation);
}

void party_audio_receive(WorldState &world, u32 &last_sequence, bool baseline)
{
    if (baseline) { last_sequence = world.sound_sequence; return; }
    if (last_sequence >= world.sound_sequence) return;
    auto &player = world.party.members[world.party.local_slot].player;
    party_audio_update_listener(world);
    const KfAudioPlayback playback {audio_state, audio_voice_slot_index,
        player.state.audio_effects_enabled != KF_PLAYER_OPTION_OFF};
    const u32 count = std::min<u32>(world.sound_sequence - last_sequence, KF_WORLD_SOUNDS);
    for (u32 age = count; age > 0; --age) {
        const u32 sequence = world.sound_sequence - age + 1;
        const auto &event = world.sounds[sequence % KF_WORLD_SOUNDS];
        if (event.sequence != sequence) continue;
        const SoundRef sound {event.program, event.tone, event.note};
        const VECTOR source {event.x, event.y, event.z};
        audio_play_spatial_local(playback, &sound, &source, event.volume, event.max_distance, event.attenuation_distance);
    }
    last_sequence = world.sound_sequence;
}

void audio_reset_module_state(void)
{
    kf::restore_initial_value<audio_voice_slot_index>();
    kf::restore_initial_value<audio_state>();
    audio_world = nullptr;
}

KfAudioPlaybackResult audio_play_spatial_default_range(PlayerContext &player,
    const SoundRef *sound,
    const VECTOR *position,
    s16 volume)
{
    return audio_play_spatial(player, sound, position, volume,
        KF_AUDIO_DEFAULT_MAX_DISTANCE, KF_AUDIO_DEFAULT_ATTENUATION_DISTANCE);
}

KfAudioPlaybackResult audio_play_spatial_range(PlayerContext &player,
    const SoundRef *sound,
    const VECTOR *position,
    s16 volume,
    s32 max_distance,
    s32 attenuation_distance)
{
    return audio_play_spatial(player,
        sound,
        position,
        volume,
        max_distance,
        attenuation_distance);
}

KfAudioPlayback audio_playback(PlayerContext &player)
{
    auto *listener = &player;
    if (audio_world && audio_world->party.enabled)
        listener = &audio_world->party.members[audio_world->party.local_slot].player;
    const bool prediction = audio_world ? audio_world->prediction : player.prediction;
    return {audio_state, audio_voice_slot_index,
        !prediction && listener->state.audio_effects_enabled != KF_PLAYER_OPTION_OFF,
        party_audio_direct, &player};
}
