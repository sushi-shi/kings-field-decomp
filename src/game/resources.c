#include <kf/address.h>
#include <kf/game_resources.h>
#include <kf/resources.h>
#include <kf/game_equipment.h>
#include <kf/game_map.h>
#include <kf/game_player.h>
#include <kf/game_render.h>
#include <kf/psyq.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

DATA(0x80057b48, 0x8)
char map_mix_tim_filename[8] = "MIX.TIM";

DATA(0x80065be8, 0xcc0)
KfCellWindow render_cell_windows[16];

RODATA(0x80012178, 0x3c)

/*
 * Advances a chunked stream to its next chunk: each chunk is a byte length
 * followed by the payload. The macro assigns so callers can pass
 * STREAM_NEXT(stream) + 4 (the new payload) as an argument.
 */
#define STREAM_NEXT(stream) ((stream) += *(u32 *)(stream) + 4)
/* Each map grid chunk holds 100 x 100 bytes, copied as 0x9c4 words. */
#define MAP_GRID_WORDS 0x9c4

ADDRESS(0x8001b100, 0x80)
void tim_upload_images(u_long *tim_data)
{
    TIM_IMAGE image;

    OpenTIM(tim_data);
    while (ReadTIM(&image) != 0) {
        if (image.caddr != 0) {
            LoadImage(image.crect, image.caddr);
            DrawSync(0);
        }
        if (image.paddr != 0) {
            LoadImage(image.prect, image.paddr);
            DrawSync(0);
        }
    }
}

ADDRESS(0x8001b180, 0x210)
void common_resources_load(void)
{
    u_long *images;
    u8 *stream;
    u8 *block;

    cd_file_load_allocated((void **)&images, "COM\\MIX.TIM");
    tim_upload_images(images);
    memory_release_last();
    cd_file_load_allocated((void **)&stream, "COM\\COM.DAT");
    asset_registry_set(0x15, stream + 4);
    block = STREAM_NEXT(stream);
    memcpy(render_cell_windows, block + 4, sizeof render_cell_windows);
    weapon_records_load_and_mirror_angles(
        (const KfWeaponRecord *)(STREAM_NEXT(stream) + 4));
    armor_records_load(
        (const KfArmorRecord *)(STREAM_NEXT(stream) + 4));
    magic_load_records(STREAM_NEXT(stream) + 4);
    map_object_definitions_load(
        (KfMapObjectDefinition *)(STREAM_NEXT(stream) + 4));
    memcpy(
        player_level_growth_table,
        (KfPlayerLevelGrowth *)(STREAM_NEXT(stream) + 4),
        sizeof player_level_growth_table);
    memory_release_last();
    memory_arena_cursor = block + 16;
}

ADDRESS(0x8001b390, 0x14)
void map_resource_path_set_floor(s32 floor)
{
    map_resource_path[1] = floor + '0';
}

ADDRESS(0x8001b3a4, 0x40)
void *map_resource_load_file(const char *filename)
{
    void *data;

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
    map_resource_path[6] = player_state.map_variant + '0';
    cd_file_load_into(*asset_buffer, map_resource_path);
    asset_registry_load_tmd_archive(0, *asset_buffer);
}

ADDRESS(0x8001b49c, 0xbc)
void audio_play_current_map_sequence(void)
{
    s32 sequence_id = 0;

    switch (player_state.progress_state.current_floor) {
    case 1:
        if (player_state.progress_state.level >= 15) {
            sequence_id = 1;
        }
        break;
    case 2:
        if (player_state.progress_state.level >= 25) {
            sequence_id = 1;
        }
        break;
    case 5:
        if (player_state.map_variant == 3) {
            sequence_id = 1;
        }
        break;
    }
    audio_play_map_sequence(sequence_id);
}

ADDRESS(0x8001b558, 0x258)
void map_resources_load(s32 floor, s32 use_variant)
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
    audio_load_vab(stream + 4, STREAM_NEXT(stream) + 4);
    block = stream;
    STREAM_NEXT(stream);
    audio_play_current_map_sequence();
    source = map_resource_copy_words(
        (u32 *)map_cell_attribute_grid,
        (u32 *)(stream + 4),
        MAP_GRID_WORDS);
    source = map_resource_copy_words(
        (u32 *)map_floor_height_grid, source, MAP_GRID_WORDS);
    source = map_resource_copy_words(
        (u32 *)map_cell_orientation_grid, source, MAP_GRID_WORDS);
    source = map_resource_copy_words(
        (u32 *)map_collision_flag_grid, source, MAP_GRID_WORDS);
    map_resource_copy_words(
        (u32 *)map_collision_grid, source, MAP_GRID_WORDS);
    item_load_floor_placements(
        (KfFloorItemPlacement *)(STREAM_NEXT(stream) + 4));
    map_object_pool_load(
        (KfMapObjectPlacement *)(STREAM_NEXT(stream) + 4));
    actor_pool_load_placements(
        (KfActorPlacement *)(STREAM_NEXT(stream) + 4));
    actor_definitions_load(
        (KfActorDefinition *)(STREAM_NEXT(stream) + 4));
    map_event_pool_load(
        (KfMapEventDefinition *)(STREAM_NEXT(stream) + 4));
    memory_release_last();
    memory_arena_cursor = block + 16;
    stream = map_resource_load_file("MIXB.DAT");
    tmd_register(1, stream + 4);
    tmd_register(0, STREAM_NEXT(stream) + 4);
    asset_registry_load_tmd_archive(10, STREAM_NEXT(stream) + 4);
    asset_registry_load_tmd_archive(30, STREAM_NEXT(stream) + 4);
    STREAM_NEXT(stream);
    if (use_variant == 0) {
        asset_registry_load_tmd_archive(0, stream + 4);
    } else {
        map_variant_asset_buffer = memory_allocate(0x5a000);
        map_variant_assets_load();
    }
    player_sync_position_to_map();
    memory_set_allocation_mode(2);
}
