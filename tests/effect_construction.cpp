#include "../src/game/effect_pool.cpp"

#include <cassert>
#include <type_traits>

KfPlayerState player_state{};
static unsigned sounds;

KfAudioPlayback audio_playback()
{
    static KfAudioState state{};
    static s32 slot;
    return {state, slot, true};
}
void sound_ref_play(KfAudioPlayback, const SoundRef *, s16) { ++sounds; }
KfAudioPlaybackResult audio_play_spatial_default_range(const SoundRef *, const VECTOR *, s16)
{
    ++sounds;
    return KF_AUDIO_PLAYED;
}
KfAudioPlaybackResult audio_play_spatial_range(const SoundRef *, const VECTOR *, s16, s32, s32)
{
    ++sounds;
    return KF_AUDIO_PLAYED;
}

static_assert(!std::is_invocable_v<decltype(effect_spawn_light_needle),
    u8, KfEffectType, const VECTOR &, const SVECTOR &, std::nullptr_t, KfEffectSoundRequest>);

int main()
{
    for (auto &effect : effect_state.records)
        effect.type = KF_EFFECT_SLOT_FREE;
    const VECTOR position{200, -800, 600};
    const SVECTOR direction{300, -900, 500}, rotation{100, 200, 300};
    constexpr auto type = KF_EFFECT_CLASS_20 | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER;
    for (auto variant : {KfEffectVariant::Normal, KfEffectVariant::Alternate}) {
        sounds = 0;
        auto *effect = effect_spawn_lightning_bolt(17, type, position, direction,
            variant, 37, KF_EFFECT_SOUND_SILENT);
        assert(effect && effect->id == 17 && effect->type == type);
        assert(effect->position.vx == position.vx && effect->direction.vector.vy == direction.vy);
        assert(effect->kind == KF_MAGIC_LIGHTNING_BOLT && effect->control.frames_remaining == 37);
        assert(effect->base_render_id.billboard == (variant == KfEffectVariant::Normal
            ? KF_EFFECT_BILLBOARD_LIGHTNING_BOLT : KF_EFFECT_BILLBOARD_LIGHTNING_BOLT_ALTERNATE));
        assert(sounds == 0);

        effect = effect_spawn_homing_projectile(17, type, position, direction,
            variant, rotation, KF_EFFECT_HOMING_PLAYER, KF_EFFECT_SOUND_PLAY);
        assert(effect && effect->kind == KF_EFFECT_KIND_HOMING_PROJECTILE);
        assert(effect->base_render_id.model == (variant == KfEffectVariant::Normal
            ? KF_EFFECT_MODEL_HOMING_PROJECTILE : KF_EFFECT_MODEL_HOMING_PROJECTILE_ALTERNATE));
        assert(effect->control.target_mode == KF_EFFECT_HOMING_PLAYER);
        assert(effect->rotation.vector.vx == -rotation.vx && effect->rotation.vector.vy == rotation.vy);
        assert(effect->direction.vector.vx == -rotation.vx && effect->direction.vector.vy == rotation.vy);
        assert(effect->scale_x == KF_FIXED12_ONE / 2 && effect->scale_y == KF_FIXED12_ONE / 2);
        assert(sounds == 1);
    }

    auto *scatter = effect_spawn_scatter_projectile(17, type, position, direction, 3, 37, 2100);
    assert(scatter && scatter->propagation.generations_remaining == 3);
    assert(scatter->control.frames_remaining == 37 && scatter->scale_x == 2100);
    sounds = 0;
    auto *root = effect_spawn_fire_wall(17, type, position, direction, KF_EFFECT_GROUND_BRANCH_ROOT);
    auto *leaf = effect_spawn_fire_wall(17, type, position, direction, KF_EFFECT_GROUND_BRANCH_LEAF);
    assert(root && leaf && sounds == 1);
    assert(root->propagation.branch == KF_EFFECT_GROUND_BRANCH_ROOT);
    assert(leaf->propagation.branch == KF_EFFECT_GROUND_BRANCH_LEAF);

    for (auto &effect : effect_state.records)
        effect.type = type;
    sounds = 0;
    assert(!effect_spawn_fire_ball(17, type, position, direction));
    assert(!effect_spawn_light_needle(17, type, position, direction, rotation, KF_EFFECT_SOUND_PLAY));
    assert(!effect_spawn_scatter_projectile(17, type, position, direction, 3, 37, 2100));
    assert(sounds == 0);
}
