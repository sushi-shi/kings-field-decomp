#ifndef KF_GAME_ASSET_H
#define KF_GAME_ASSET_H

/* GAME.EXE model-asset registry layout and operations. */

#include <kf/game_types.h>

/* Header shared by static and animated model assets in the registry. */
typedef struct KfAssetHeader {
    u32 byte_size;
    s32 animation_data;
    u32 tmd_data_offset;
    u32 object_table_offset;
    u32 clip_table_offset;
} KfAssetHeader;

extern KfAssetHeader *asset_registry_entries[];

extern void asset_registry_load_tmd_archive(
    u16 first_asset_id, u8 *archive);
extern void asset_registry_select(u16 index);
extern void asset_registry_set(u16 index, void *asset);

#endif
