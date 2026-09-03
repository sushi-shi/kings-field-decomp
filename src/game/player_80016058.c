#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;
extern KfPlayerLevelGrowth player_level_growth_table[40];
extern const SoundRef player_sound_refs[3];
extern void player_recalculate_combat_stats(void);
extern void notify_enqueue(s32 arg0);
extern void sound_ref_play(const SoundRef *sound, s16 volume);

ADDRESS(0x80016058, 0x224)
void player_add_experience(s16 amount)
{
    const KfPlayerLevelGrowth *growth;
    u8 level;

    player_state.experience += amount;
    if (player_state.experience > 99999) {
        player_state.experience = 99999;
    }
    while (player_state.experience >= player_state.next_level_experience) {
        level = player_state.progress_state.level;
        if (player_state.progress_state.level >= 255) {
            break;
        }
        player_state.progress_state.level = level + 1;
        if (level >= 40) {
            player_state.vitals.maximum_hp +=
                player_level_growth_table[39].maximum_hp
                - player_level_growth_table[38].maximum_hp;
            player_state.vitals.maximum_mp +=
                player_level_growth_table[39].maximum_mp
                - player_level_growth_table[38].maximum_mp;
            player_state.base_physical_power += player_level_growth_table[39].physical_power_step;
            player_state.base_magic += player_level_growth_table[39].magic_step;
            player_state.next_level_experience +=
                player_level_growth_table[39].experience_threshold
                - player_level_growth_table[38].experience_threshold;
        } else {
            growth = &player_level_growth_table[level];
            player_state.vitals.maximum_hp = growth->maximum_hp;
            player_state.vitals.maximum_mp = growth->maximum_mp;
            player_state.base_physical_power += growth->physical_power_step;
            player_state.base_magic += growth->magic_step;
            player_state.next_level_experience = growth->experience_threshold;
        }
        if (player_state.vitals.maximum_hp >= 10000) {
            player_state.vitals.maximum_hp = 9999;
        }
        if (player_state.vitals.maximum_mp >= 10000) {
            player_state.vitals.maximum_mp = 9999;
        }
        if (player_state.base_physical_power >= 1000) {
            player_state.base_physical_power = 999;
        }
        if (player_state.base_magic >= 1000) {
            player_state.base_magic = 999;
        }
        player_recalculate_combat_stats();
        notify_enqueue(0);
        sound_ref_play(&player_sound_refs[2], 0x7f);
    }
}
