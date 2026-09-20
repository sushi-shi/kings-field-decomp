#ifndef KF_GAME_ASSET_H
#define KF_GAME_ASSET_H

#include <kf/lib/types.h>

enum {
    KF_ASSET_ARCHIVE_HEADER_BYTES = 4,
    KF_ASSET_ACTOR_FIRST = 0,
    KF_ASSET_MAP_EVENT_FIRST = 10,
    KF_ASSET_WEAPON = 20,
    KF_ASSET_EFFECT_SPRITES = 21,
    KF_ASSET_EFFECT_FIRST = 30,
    KF_ASSET_REGISTRY_KNOWN_ENTRIES = 48,
    KF_WEAPON_ASSET_BUFFER_BYTES = 49152
};

typedef struct KfAssetHeader {
    u32 byte_size;
    s32 animation_clip_count;
    u32 tmd_data_offset;
    u32 object_table_offset;
    u32 clip_table_offset;
} KfAssetHeader;

extern void asset_registry_load_tmd_archive(
    u16 first_asset_id, u8 *archive, std::size_t size);
extern void asset_registry_select(u16 index);
extern void asset_registry_set(u16 index, void *data, std::size_t size);

#endif
