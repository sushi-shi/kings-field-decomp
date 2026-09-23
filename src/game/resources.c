#include <kf/lib/null.h>
#include <kf/lib/address.h>
#include <kf/game/resources.h>
#include <kf/lib/resources.h>
#include <kf/game/equipment.h>
#include <kf/lib/map.h>
#include <kf/game/player.h>
#include <kf/game/render.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <kf/game/game.h>

enum {
    MAP_VARIANT_ASSET_BUFFER_BYTES = 0x5a000,
    MAP_SEQUENCE_DEFAULT = 0,
    MAP_SEQUENCE_ALTERNATE = 1,
    MAP_FLOOR1_ALTERNATE_MUSIC_PROGRESS = 15,
    MAP_FLOOR2_ALTERNATE_MUSIC_PROGRESS = 25
};

DATA(0x80055af0, 0xc)
char map_resource_path[KF_MAP_RESOURCE_PATH_BYTES] = "B0\\";

DATA(0x80057b48, 0x8)
char map_mix_tim_filename[8] = "MIX.TIM";

DATA(0x80065be8, 0xcc0)
KfCellWindow render_cell_windows[KF_CELL_WINDOW_YAW_COUNT];

DATA(0x800668e8, 0x2710)
KfMapGrid map_collision_flag_grid;

DATA(0x80069018, 0x2710)
KfMapOrientationGrid map_cell_orientation_grid;

DATA(0x80095900, 0x2710)
KfMapGrid map_floor_height_grid;

DATA(0x80098018, 0x2710)
KfMapCollisionGrid map_collision_grid;

DATA(0x8009a748, 0x2710)
KfMapAttributeGrid map_cell_attribute_grid;

RODATA(0x80012178, 0x39)

/* Each map grid chunk holds 100 x 100 bytes, copied as 0x9c4 words. */
#define MAP_GRID_WORDS (sizeof map_cell_attribute_grid / sizeof(u32))

#include "../lib/tim_upload_images.inc"

ADDRESS(0x8001b180, 0x210)
void common_resources_load(void)
{
    u8 *images;
    u8 *stream;
    u8 *block;

    cd_file_load_allocated(&images, "COM\\MIX.TIM");
    tim_upload_images(images);
    memory_release_last();
    cd_file_load_allocated(&stream, "COM\\COM.DAT");
    asset_registry_set(
        KF_ASSET_EFFECT_SPRITES, (KfAssetHeader *)(stream + KF_RESOURCE_CHUNK_HEADER_BYTES));
    block = RESOURCE_STREAM_NEXT(stream);
    memcpy((void *)render_cell_windows, (const void *)(block + KF_RESOURCE_CHUNK_HEADER_BYTES),
        sizeof render_cell_windows);
    weapon_records_load_and_mirror_angles(
        (const KfWeaponTable *)(RESOURCE_STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    armor_records_load(
        (const KfArmorTable *)(RESOURCE_STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    magic_load_records(
        (const KfMagicTable *)(RESOURCE_STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    map_object_definitions_load(
        (const KfMapObjectDefinitionTable *)(RESOURCE_STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    memcpy(
        (void *)player_level_growth_table,
        (const void *)(const KfPlayerLevelGrowth *)(RESOURCE_STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES),
        sizeof player_level_growth_table);
    memory_release_last();
    memory_arena.allocation.cursor = block + KF_RESOURCE_REUSE_PREFIX_BYTES;
}

ADDRESS(0x8001b390, 0x14)
void map_resource_path_set_floor(KfFloorId floor)
{
    map_resource_path[1] = KF_ENUM_ENCODE(s32, floor) + '0';
}

ADDRESS(0x8001b3a4, 0x40)
u8 *map_resource_load_file(const char *filename)
{
    u8 *data;

    strcpy(&map_resource_path[3], filename);
    cd_file_load_allocated(&data, map_resource_path);
    return data;
}

#include "../lib/resource_copy_words.inc"

ADDRESS(0x8001b414, 0x88)
void map_variant_assets_load(void)
{
    /* Both retail calls reload the buffer through one saved slot address. */
    u8 **asset_buffer = &map_runtime_state.variant_asset_buffer;

    memcpy((void *)(&map_resource_path[3]), (const void *)("CHR0.MIM"), sizeof "CHR0.MIM");
    map_resource_path[6] = KF_ENUM_ENCODE(u8, player_state.map_variant) + '0';
    cd_file_load_into((void *)*asset_buffer, map_resource_path);
    asset_registry_load_tmd_archive(KF_ASSET_ACTOR_FIRST, *asset_buffer);
}

ADDRESS(0x8001b49c, 0xbc)
void audio_play_current_map_sequence(void)
{
    s32 sequence_id = MAP_SEQUENCE_DEFAULT;

    switch (player_state.progress_state.current_floor) {
    case KF_FLOOR_1:
        if (player_state.progress_state.level >= MAP_FLOOR1_ALTERNATE_MUSIC_PROGRESS) {
            sequence_id = MAP_SEQUENCE_ALTERNATE;
        }
        break;
    case KF_FLOOR_2:
        if (player_state.progress_state.level >= MAP_FLOOR2_ALTERNATE_MUSIC_PROGRESS) {
            sequence_id = MAP_SEQUENCE_ALTERNATE;
        }
        break;
    case KF_FLOOR_5:
        if (player_state.map_variant == KF_FLOOR5_ALTERNATE_MUSIC_VARIANT) {
            sequence_id = MAP_SEQUENCE_ALTERNATE;
        }
        break;
    }
    audio_play_map_sequence(sequence_id);
}

ADDRESS(0x8001b558, 0x258)
void map_resources_load(KfFloorId floor, KF_ENUM_PARAM(KfMapVariant, s32) map_variant)
{
    u8 *stream;
    u8 *block;
    const u32 *source;

    audio_stop_sequence_fade();
    effect_pool_reset();
    memory_allocation_reset();
    map_resource_path_set_floor(floor);
    tim_upload_images(map_resource_load_file(map_mix_tim_filename));
    memory_release_last();
    stream = map_resource_load_file("MIXA.DAT");
    audio_load_vab(stream + KF_RESOURCE_CHUNK_HEADER_BYTES,
        RESOURCE_STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES);
    block = stream;
    RESOURCE_STREAM_NEXT(stream);
    audio_play_current_map_sequence();
    source = map_resource_copy_words(
        map_cell_attribute_grid.words,
        (u32 *)(stream + KF_RESOURCE_CHUNK_HEADER_BYTES),
        MAP_GRID_WORDS);
    source = map_resource_copy_words(
        map_floor_height_grid.words, source, MAP_GRID_WORDS);
    source = map_resource_copy_words(
        map_cell_orientation_grid.words, source, MAP_GRID_WORDS);
    source = map_resource_copy_words(
        map_collision_flag_grid.words, source, MAP_GRID_WORDS);
    map_resource_copy_words(
        map_collision_grid.words, source, MAP_GRID_WORDS);
    item_load_floor_placements(
        (KfFloorItemPlacement *)(RESOURCE_STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    map_object_pool_load(
        (KfMapObjectPlacement *)(RESOURCE_STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    actor_pool_load_placements(
        (KfActorPlacement *)(RESOURCE_STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    actor_definitions_load(
        (const KfActorDefinitionTable *)(RESOURCE_STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    map_event_pool_load(
        (KfMapEventDefinition *)(RESOURCE_STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    memory_release_last();
    memory_arena.allocation.cursor = block + KF_RESOURCE_REUSE_PREFIX_BYTES;
    stream = map_resource_load_file("MIXB.DAT");
    tmd_register(KF_TMD_SLOT_ENTITIES,
        (KfTmdHeader *)(stream + KF_RESOURCE_CHUNK_HEADER_BYTES));
    tmd_register(KF_TMD_SLOT_MAP,
        (KfTmdHeader *)(RESOURCE_STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    asset_registry_load_tmd_archive(KF_ASSET_MAP_EVENT_FIRST,
        RESOURCE_STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES);
    asset_registry_load_tmd_archive(KF_ASSET_EFFECT_FIRST,
        RESOURCE_STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES);
    RESOURCE_STREAM_NEXT(stream);
    if (map_variant == KF_MAP_VARIANT_DEFAULT) {
        asset_registry_load_tmd_archive(KF_ASSET_ACTOR_FIRST,
            stream + KF_RESOURCE_CHUNK_HEADER_BYTES);
    } else {
        map_runtime_state.variant_asset_buffer = (u8 *)memory_allocate(MAP_VARIANT_ASSET_BUFFER_BYTES);
        map_variant_assets_load();
    }
    player_sync_position_to_map();
    memory_set_allocation_mode(KF_MEMORY_USE_HEAP);
}
