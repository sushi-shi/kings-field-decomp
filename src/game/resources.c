#include <kf/null.h>
#include <kf/address.h>
#include <kf/game_resources.h>
#include <kf/resources.h>
#include <kf/game_equipment.h>
#include <kf/game_map.h>
#include <kf/game_player.h>
#include <kf/game_render.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <kf/game.h>

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

/*
 * Advances a chunked stream to its next chunk: each chunk is a byte length
 * followed by the payload. The macro assigns so callers can pass
 * the next chunk's payload as an argument.
 */
#define STREAM_NEXT(stream) \
    ((stream) += *(u32 *)(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES)
/* Each map grid chunk holds 100 x 100 bytes, copied as 0x9c4 words. */
#define MAP_GRID_WORDS (sizeof map_cell_attribute_grid / sizeof(u32))

ADDRESS(0x8001b100, 0x80)
void tim_upload_images(void *tim_data)
{
    TIM_IMAGE image;

    OpenTIM(tim_data);
    while (ReadTIM(&image) != NULL) {
        if (image.caddr != NULL) {
            LoadImage(image.crect, image.caddr);
            DrawSync(0);
        }
        if (image.paddr != NULL) {
            LoadImage(image.prect, image.paddr);
            DrawSync(0);
        }
    }
}

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
    block = STREAM_NEXT(stream);
    memcpy(render_cell_windows, block + KF_RESOURCE_CHUNK_HEADER_BYTES,
        sizeof render_cell_windows);
    weapon_records_load_and_mirror_angles(
        (const KfWeaponTable *)(STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    armor_records_load(
        (const KfArmorTable *)(STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    magic_load_records(
        (const KfMagicTable *)(STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    map_object_definitions_load(
        (const KfMapObjectDefinitionTable *)(STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    memcpy(
        player_level_growth_table,
        (KfPlayerLevelGrowth *)(STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES),
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

ADDRESS(0x8001b3e4, 0x30)
const u32 *map_resource_copy_words(
    u32 *destination,
    const u32 *source,
    u32 word_count)
{
    while (word_count-- != 0) {
        *destination++ = *source++;
    }
    return source;
}

ADDRESS(0x8001b414, 0x88)
void map_variant_assets_load(void)
{
    /* Both retail calls reload the buffer through one saved slot address. */
    u8 **asset_buffer = &map_variant_asset_buffer;

    memcpy(&map_resource_path[3], "CHR0.MIM", sizeof "CHR0.MIM");
    map_resource_path[6] = KF_ENUM_ENCODE(u8, player_state.map_variant) + '0';
    cd_file_load_into(*asset_buffer, map_resource_path);
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
void map_resources_load(KfFloorId floor, KF_ENUM_PARAM(KfMapVariant, s32) use_variant)
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
        STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES);
    block = stream;
    STREAM_NEXT(stream);
    audio_play_current_map_sequence();
    source = map_resource_copy_words(
        (u32 *)&map_cell_attribute_grid,
        (u32 *)(stream + KF_RESOURCE_CHUNK_HEADER_BYTES),
        MAP_GRID_WORDS);
    source = map_resource_copy_words(
        (u32 *)&map_floor_height_grid, source, MAP_GRID_WORDS);
    source = map_resource_copy_words(
        (u32 *)&map_cell_orientation_grid, source, MAP_GRID_WORDS);
    source = map_resource_copy_words(
        (u32 *)&map_collision_flag_grid, source, MAP_GRID_WORDS);
    map_resource_copy_words(
        (u32 *)&map_collision_grid, source, MAP_GRID_WORDS);
    item_load_floor_placements(
        (KfFloorItemPlacement *)(STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    map_object_pool_load(
        (KfMapObjectPlacement *)(STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    actor_pool_load_placements(
        (KfActorPlacement *)(STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    actor_definitions_load(
        (const KfActorDefinitionTable *)(STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    map_event_pool_load(
        (KfMapEventDefinition *)(STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    memory_release_last();
    memory_arena.allocation.cursor = block + KF_RESOURCE_REUSE_PREFIX_BYTES;
    stream = map_resource_load_file("MIXB.DAT");
    tmd_register(KF_TMD_SLOT_ENTITIES,
        (KfTmdHeader *)(stream + KF_RESOURCE_CHUNK_HEADER_BYTES));
    tmd_register(KF_TMD_SLOT_MAP,
        (KfTmdHeader *)(STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    asset_registry_load_tmd_archive(KF_ASSET_MAP_EVENT_FIRST,
        STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES);
    asset_registry_load_tmd_archive(KF_ASSET_EFFECT_FIRST,
        STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES);
    STREAM_NEXT(stream);
    if (use_variant == KF_MAP_VARIANT_DEFAULT) {
        asset_registry_load_tmd_archive(KF_ASSET_ACTOR_FIRST,
            stream + KF_RESOURCE_CHUNK_HEADER_BYTES);
    } else {
        map_variant_asset_buffer = memory_allocate(MAP_VARIANT_ASSET_BUFFER_BYTES);
        map_variant_assets_load();
    }
    player_sync_position_to_map();
    memory_set_allocation_mode(KF_MEMORY_USE_HEAP);
}
