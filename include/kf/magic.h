#ifndef KF_MAGIC_H
#define KF_MAGIC_H

/*
 * game.magic (0x8003a244-0x8003a7dc): magic-record loading and cast dispatch.
 * Effect-pool lifecycle declarations live in game_effect.h.
 */

#include <kf/audio.h>
#include <kf/enum.h>

enum {
    KF_MAGIC_HEALING = 0,
    KF_MAGIC_DISPOISON = 1,
    KF_MAGIC_RESIST_FIRE = 2,
    KF_MAGIC_BLESS = 3,
    KF_MAGIC_PLAYER_COUNT = 9,
    KF_MAGIC_RECORD_COUNT = 24,
    KF_MAGIC_TABLE_WORD_COUNT = 120,
    KF_MAGIC_SOUND_COUNT = 2,
    KF_MAGIC_DAMAGE_COMPONENT_COUNT = 4
};

/* Only ranged spells and unequip are stored as the selected spell. */
KF_ENUM_BEGIN(KfSelectedMagicId, u8)
    KF_MAGIC_LIGHTNING_BOLT = 4,
    KF_MAGIC_FIRE_BALL = 5,
    KF_MAGIC_FIRE_WALL = 6,
    KF_MAGIC_WIND_CUTTER = 7,
    KF_MAGIC_LIGHT_NEEDLE = 8,
    KF_MAGIC_NONE = 0xff
KF_ENUM_END(KfSelectedMagicId)

/* Keep exact learned checks distinct from the nonzero gameplay checks. */
KF_ENUM_BEGIN(KfMagicLearningState, u8)
    KF_MAGIC_UNLEARNED = 0,
    KF_MAGIC_LEARNED = 1
KF_ENUM_END(KfMagicLearningState)

/* Runtime-loaded spell definition used by player and effect code. */
typedef struct KfMagicRecord {
    KfMagicLearningState learned;
    u8 charge_rate;
    SoundRef sounds[KF_MAGIC_SOUND_COUNT];
    u16 damage_components[KF_MAGIC_DAMAGE_COMPONENT_COUNT];
    u16 mp_cost;
    u8 unknown_12[0x02];
} KfMagicRecord;

/* Entry access and the complete resource-copy word span share one owner. */
typedef union KfMagicTable {
    KfMagicRecord entries[KF_MAGIC_RECORD_COUNT];
    u32 words[KF_MAGIC_TABLE_WORD_COUNT];
} KfMagicTable;

typedef char check_magic_table_size[sizeof(KfMagicTable) == 0x1e0 ? 1 : -1];

extern void magic_load_records(const KfMagicTable *table);
extern void magic_cast(void);

#endif
