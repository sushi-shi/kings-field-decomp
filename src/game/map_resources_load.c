#include <kf/address.h>
#include <kf/semantic_types.h>

extern void audio_stop_sequence_fade(void);
extern void effect_pool_reset(void);
extern void memory_allocation_reset(void);
extern void map_resource_path_set_floor(s32 floor);
extern void *map_resource_load_file(const char *filename);
extern void tim_upload_images(u_long *tim_data);
extern void memory_release_last(void);
extern void audio_load_vab(u8 *vab_header, u8 *vab_body);
extern void audio_play_current_map_sequence(void);
extern const u32 *map_resource_copy_words(
    u32 *destination, const u32 *source, u32 word_count);
extern void func_80020b4c(s16 *block);
extern void map_object_pool_load(const KfMapObjectPlacement *placements);
extern void actor_pool_load_placements(const KfActorPlacement *placements);
extern void actor_definitions_load(const KfActorDefinition *definitions);
extern void map_event_pool_load(const KfMapEventDefinition *definitions);
extern void tmd_register(u16 index, void *asset);
extern void asset_registry_load_tmd_archive(u16 first_asset_id, u8 *archive);
extern void *memory_allocate(s32 size);
extern void map_variant_assets_load(void);
extern void player_sync_position_to_map(void);
extern void memory_set_allocation_mode(s32 mode);

extern u8 map_cell_attribute_grid[100][100];
extern u8 map_floor_height_grid[100][100];
extern u8 map_cell_orientation_grid[100][100];
extern u8 map_collision_flag_grid[100][100];
extern u8 map_collision_grid[100][100];
extern u8 *map_variant_asset_buffer;
extern u8 *memory_arena_cursor;

DATA(0x80057b48, 0x8)
char map_mix_tim_filename[8] = "MIX.TIM";

RODATA(0x8001219c, 0x18)

/*
 * Advances a chunked stream to its next chunk: each chunk is a byte length
 * followed by the payload.  The macro assigns so callers can pass
 * STREAM_NEXT(stream) + 4 (the new payload) as an argument.
 */
#define STREAM_NEXT(stream) ((stream) += *(u32 *)(stream) + 4)
/* Each map grid chunk holds 100 x 100 bytes, copied as 0x9c4 words. */
#define MAP_GRID_WORDS 0x9c4

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
    source = map_resource_copy_words((u32 *)map_cell_attribute_grid, (u32 *)(stream + 4), MAP_GRID_WORDS);
    source = map_resource_copy_words((u32 *)map_floor_height_grid, source, MAP_GRID_WORDS);
    source = map_resource_copy_words((u32 *)map_cell_orientation_grid, source, MAP_GRID_WORDS);
    source = map_resource_copy_words((u32 *)map_collision_flag_grid, source, MAP_GRID_WORDS);
    map_resource_copy_words((u32 *)map_collision_grid, source, MAP_GRID_WORDS);
    func_80020b4c((s16 *)(STREAM_NEXT(stream) + 4));
    map_object_pool_load((KfMapObjectPlacement *)(STREAM_NEXT(stream) + 4));
    actor_pool_load_placements((KfActorPlacement *)(STREAM_NEXT(stream) + 4));
    actor_definitions_load((KfActorDefinition *)(STREAM_NEXT(stream) + 4));
    map_event_pool_load((KfMapEventDefinition *)(STREAM_NEXT(stream) + 4));
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
