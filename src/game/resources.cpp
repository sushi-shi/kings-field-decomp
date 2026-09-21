#include <kf/lib/null.h>

#include <kf/game/resources.h>
#include <kf/lib/resources.h>
#include <kf/game/equipment.h>
#include <kf/lib/map.h>
#include <kf/game/player.h>
#include <kf/game/render.h>
#include <kf/lib/geometry_types.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <kf/game/game.h>

enum {
    MAP_VARIANT_ASSET_BUFFER_BYTES = 0x5a000,
    MAP_SEQUENCE_DEFAULT = 0,
    MAP_SEQUENCE_ALTERNATE = 1,
    MAP_FLOOR1_ALTERNATE_MUSIC_PROGRESS = 15,
    MAP_FLOOR2_ALTERNATE_MUSIC_PROGRESS = 25
};

char map_resource_path[KF_MAP_RESOURCE_PATH_BYTES] = "B0/";

char map_mix_tim_filename[8] = "MIX.TIM";

KfCellWindow render_cell_windows[KF_CELL_WINDOW_YAW_COUNT];

KfMapGrid map_collision_flag_grid;

KfMapOrientationGrid map_cell_orientation_grid;

KfMapGrid map_floor_height_grid;

KfMapCollisionGrid map_collision_grid;

KfMapAttributeGrid map_cell_attribute_grid;

static constexpr auto map_grid_word_count = sizeof map_cell_attribute_grid / sizeof(u32);

void common_resources_load(void)
{
    u8 *images;
    std::size_t image_size;
    u8 *stream;
    u8 *block;

    resource_file_load_allocated(&images, "COM/MIX.TIM", &image_size);
    tim_upload_images(images, image_size);
    memory_release_last();
    std::size_t resource_size;
    resource_file_load_allocated(&stream, "COM/COM.DAT", &resource_size);
    const u8 *resource_end = stream + resource_size;
    const auto effect_asset = resource_chunk_view(stream, resource_end);
    asset_registry_set(
        KF_ASSET_EFFECT_SPRITES, stream + KF_RESOURCE_CHUNK_HEADER_BYTES, effect_asset.size);
    block = stream = resource_stream_next(stream, resource_end);
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

void map_resource_path_set_floor(KfFloorId floor)
{
    map_resource_path[1] = kf_enum_encode<s32>(floor) + '0';
}

u8 *map_resource_load_file(const char *filename, std::size_t *loaded_size)
{
    u8 *data;

    strcpy(&map_resource_path[3], filename);
    resource_file_load_allocated(&data, map_resource_path, loaded_size);
    return data;
}

void map_variant_assets_load(void)
{

    u8 **asset_buffer = &map_runtime_state.variant_asset_buffer;

    memcpy((void *)(&map_resource_path[3]), (const void *)("CHR0.MIM"), sizeof "CHR0.MIM");
    map_resource_path[6] = kf_enum_encode<u8>(player_state.map_variant) + '0';
    std::size_t loaded_size;
    if (resource_file_load_into(*asset_buffer, MAP_VARIANT_ASSET_BUFFER_BYTES, map_resource_path, &loaded_size) != KF_RESOURCE_LOADED)
        exit(1);
    asset_registry_load_tmd_archive(KF_ASSET_ACTOR_FIRST, *asset_buffer, loaded_size);
}

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

void map_resources_load(KfFloorId floor, KfMapVariant map_variant)
{
    u8 *stream;
    u8 *block;
    const u32 *source;

    audio_stop_sequence_fade();
    effect_pool_reset();
    memory_allocation_reset();
    map_resource_path_set_floor(floor);
    std::size_t image_size;
    u8 *images = map_resource_load_file(map_mix_tim_filename, &image_size);
    tim_upload_images(images, image_size);
    memory_release_last();
    std::size_t resource_size;
    stream = map_resource_load_file("MIXA.DAT", &resource_size);
    const u8 *resource_end = stream + resource_size;
    audio_load_vab_resource(stream, resource_size);
    block = stream;
    stream = resource_stream_next(stream, resource_end);
    block = stream;
    stream = resource_stream_next(stream, resource_end);
    audio_play_current_map_sequence();
    const auto map_grids = resource_chunk_view(stream, resource_end);
    if (map_grids.size < 5 * sizeof map_cell_attribute_grid)
        kf::host_fail("Truncated map grids");
    source = map_resource_copy_words(
        map_cell_attribute_grid.words,
        (u32 *)(stream + KF_RESOURCE_CHUNK_HEADER_BYTES),
        map_grid_word_count);
    source = map_resource_copy_words(
        map_floor_height_grid.words, source, map_grid_word_count);
    source = map_resource_copy_words(
        map_cell_orientation_grid.words, source, map_grid_word_count);
    source = map_resource_copy_words(
        map_collision_flag_grid.words, source, map_grid_word_count);
    map_resource_copy_words(
        map_collision_grid.words, source, map_grid_word_count);
    stream = resource_stream_next(stream, resource_end);
    const auto floor_items = resource_chunk_view(stream, resource_end);
    item_load_floor_placements(floor_items.data, floor_items.size);
    stream = resource_stream_next(stream, resource_end);
    resource_chunk_view(stream, resource_end);
    map_object_pool_load(
        (KfMapObjectPlacement *)(stream + KF_RESOURCE_CHUNK_HEADER_BYTES));
    stream = resource_stream_next(stream, resource_end);
    resource_chunk_view(stream, resource_end);
    actor_pool_load_placements(
        (KfActorPlacement *)(stream + KF_RESOURCE_CHUNK_HEADER_BYTES));
    stream = resource_stream_next(stream, resource_end);
    resource_chunk_view(stream, resource_end);
    actor_definitions_load(
        (const KfActorDefinitionTable *)(stream + KF_RESOURCE_CHUNK_HEADER_BYTES));
    stream = resource_stream_next(stream, resource_end);
    resource_chunk_view(stream, resource_end);
    map_event_pool_load(
        (KfMapEventDefinition *)(stream + KF_RESOURCE_CHUNK_HEADER_BYTES));
    memory_release_last();
    memory_arena.allocation.cursor = block + KF_RESOURCE_REUSE_PREFIX_BYTES;
    stream = map_resource_load_file("MIXB.DAT", &resource_size);
    resource_end = stream + resource_size;
    const auto entity_tmd = resource_chunk_view(stream, resource_end);
    tmd_register(KF_TMD_SLOT_ENTITIES,
        stream + KF_RESOURCE_CHUNK_HEADER_BYTES, entity_tmd.size);
    stream = resource_stream_next(stream, resource_end);
    const auto map_tmd = resource_chunk_view(stream, resource_end);
    tmd_register(KF_TMD_SLOT_MAP,
        stream + KF_RESOURCE_CHUNK_HEADER_BYTES, map_tmd.size);
    stream = resource_stream_next(stream, resource_end);
    const auto event_models = resource_chunk_view(stream, resource_end);
    asset_registry_load_tmd_archive(KF_ASSET_MAP_EVENT_FIRST,
        stream + KF_RESOURCE_CHUNK_HEADER_BYTES, event_models.size);
    stream = resource_stream_next(stream, resource_end);
    const auto effect_models = resource_chunk_view(stream, resource_end);
    asset_registry_load_tmd_archive(KF_ASSET_EFFECT_FIRST,
        stream + KF_RESOURCE_CHUNK_HEADER_BYTES, effect_models.size);
    stream = resource_stream_next(stream, resource_end);
    if (map_variant == KF_MAP_VARIANT_DEFAULT) {
        const auto actor_models = resource_chunk_view(stream, resource_end);
        asset_registry_load_tmd_archive(KF_ASSET_ACTOR_FIRST,
            stream + KF_RESOURCE_CHUNK_HEADER_BYTES, actor_models.size);
    } else {
        map_runtime_state.variant_asset_buffer = (u8 *)memory_allocate(MAP_VARIANT_ASSET_BUFFER_BYTES);
        map_variant_assets_load();
    }
    player_sync_position_to_map();
    memory_set_allocation_mode(KF_MEMORY_USE_HEAP);
}

void resources_reset_module_state(void)
{
    kf::restore_initial_value<map_resource_path>();
    kf::restore_initial_value<map_mix_tim_filename>();
    kf::restore_initial_value<render_cell_windows>();
    kf::restore_initial_value<map_collision_flag_grid>();
    kf::restore_initial_value<map_cell_orientation_grid>();
    kf::restore_initial_value<map_floor_height_grid>();
    kf::restore_initial_value<map_collision_grid>();
    kf::restore_initial_value<map_cell_attribute_grid>();
}
