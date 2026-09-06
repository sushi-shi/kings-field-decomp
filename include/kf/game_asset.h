#ifndef KF_GAME_ASSET_H
#define KF_GAME_ASSET_H

/* GAME.EXE model-asset registry layout and operations. */

#include <kf/game_types.h>

enum {
    KF_ASSET_ARCHIVE_HEADER_BYTES = 4,
    KF_ASSET_ACTOR_FIRST = 0,
    KF_ASSET_MAP_EVENT_FIRST = 10,
    KF_ASSET_WEAPON = 20,
    KF_ASSET_EFFECT_SPRITES = 21,
    KF_ASSET_EFFECT_FIRST = 30,
    KF_WEAPON_ASSET_BUFFER_BYTES = 49152
};

/* Header shared by static and animated model assets in the registry. */
typedef struct KfAssetHeader {
    u32 byte_size;
    s32 animation_clip_count;
    u32 tmd_data_offset;
    u32 object_table_offset;
    u32 clip_table_offset;
} KfAssetHeader;

extern KfAssetHeader *asset_registry_entries[];

extern void asset_registry_load_tmd_archive(
    u16 first_asset_id, u8 *archive);
extern void asset_registry_select(u16 index);
extern void asset_registry_set(u16 index, KfAssetHeader *asset);

#endif
