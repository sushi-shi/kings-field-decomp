#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfActorState actor_state;

extern const SoundRef boss_death_phase_sounds[4];
extern const SoundRef boss_death_loop_sound;
extern u8 boss_defeat_complete;

/* Psy-Q LIBC: int rand(void). */
extern s32 rand(void);
extern void actor_play_sound_at_phase(const SoundRef *sound, u16 phase);
extern void map_object_pool_trigger_link(u8 link_id);
extern void actor_pool_begin_death_by_definition(u16 definition_id);
extern void sound_ref_play(const SoundRef *sound, s16 volume);
/* Effect spawner; this call site passes six arguments (the identity lists eight). */
extern u8 *func_80036f44(
    u8 kind, u8 arg1, u8 arg2, struct KfVec3i *position, void *output, u32 arg5);

/* Boss death: phase sounds, then random effects and a loop sound keyed on the death animation step. */
ADDRESS(0x8002f8cc, 0x1bc)
void actor_update_boss_death_sequence(void)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition = actor_state.current_definition;
    u32 effect_output[2];
    struct KfVec3i position;

    actor_play_sound_at_phase(&boss_death_phase_sounds[1], 500);
    actor_play_sound_at_phase(&boss_death_phase_sounds[2], 1000);
    actor_play_sound_at_phase(&boss_death_phase_sounds[3], 1500);
    if (actor->animation_phase >= 4095) {
        actor->animation_phase = 0xfff;
        actor->action_timer = 0;
        actor->lifecycle = 3;
        boss_defeat_complete = 1;
        map_object_pool_trigger_link(13);
        actor_pool_begin_death_by_definition(0);
        actor_pool_begin_death_by_definition(2);
        actor_pool_begin_death_by_definition(3);
        actor_pool_begin_death_by_definition(4);
    }
    if (actor->animation_phase % (definition->action_animation_steps[KF_ACTOR_ACTION_INDEX(6)] * 2) == 0) {
        position.x = actor->position.x + (rand() & 0x1fff) - 4096;
        position.z = actor->position.z + (rand() & 0x1fff) - 4096;
        position.y = actor->position.y - (rand() & 0xfff);
        func_80036f44(0, 0x13, 0x2c, &position, effect_output, 0);
        if (actor->animation_phase % (definition->action_animation_steps[KF_ACTOR_ACTION_INDEX(6)] * 4) == 0) {
            sound_ref_play(&boss_death_loop_sound, 100);
        }
    }
}
