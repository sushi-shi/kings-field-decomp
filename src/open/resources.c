#include <kf/lib/null.h>

#include <kf/lib/audio.h>
#include <kf/lib/cd_file.h>
#include <kf/lib/item.h>
#include <kf/lib/map_data.h>
#include <kf/lib/memory.h>
#include <kf/open/render.h>
#include <kf/open/resources.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <kf/lib/resources.h>

enum {
    OPEN_CD_READ_ATTEMPTS = 100
};

char cd_path_prefix[5] = "\\KF\\";

char cd_version_suffix[3] = ";1";

char opening_ending_sequence_path[8] = "B0\\END.";

static CdlLOC cd_read_location;

static u8 *opening_scene1_arena_cursor;

static u8 *opening_ending_arena_cursor;

static CdlFILE cd_search_file;

static char cd_path_buffer[KF_CD_PATH_BYTES];

KfMapOrientationGrid map_cell_orientation_grid;

KfMapGrid map_floor_height_grid;

KfMapCollisionGrid map_collision_grid;

KfMapAttributeGrid map_cell_attribute_grid;

#define MAP_GRID_WORDS (sizeof map_cell_attribute_grid / sizeof(u32))

KfResourceLoadResult cd_file_load_allocated(
    u8 **destination, const char *relative_path)
{
    char *path = cd_path_buffer;
    s32 attempt;

    memcpy((void *)path, (const void *)cd_path_prefix, sizeof cd_path_prefix);
    strcat(path, relative_path);
    strcat(path, cd_version_suffix);
    if (CdSearchFile(&cd_search_file, path) == NULL) {
        return KF_RESOURCE_LOAD_FAILED;
    }
    if ((cd_search_file.size & (KF_CD_SECTOR_BYTES - 1)) != 0) {
        cd_search_file.size =
            ((cd_search_file.size >> KF_CD_SECTOR_SHIFT) + 1) << KF_CD_SECTOR_SHIFT;
    }
    *destination = (u8 *)memory_allocate(cd_search_file.size);
    CD_LOCATION_COPY(cd_read_location, cd_search_file.pos);
    for (attempt = 0; attempt < OPEN_CD_READ_ATTEMPTS; attempt++) {
        s32 result;

        CdControl(CdlSetloc, (u_char *)&cd_read_location, NULL);
        CdRead(
            cd_search_file.size >> KF_CD_SECTOR_SHIFT,
            (u_long *)*destination,
            CdlModeSpeed);
        while ((result = CdReadSync(KF_CD_READ_POLL, NULL)) > 0) {
        }
        if (result == 0) {
            attempt = KF_CD_READ_STOP_ATTEMPT;
        }
    }
    return KF_RESOURCE_LOADED;
}

KfResourceLoadResult cd_file_load_into(
    void *destination, const char *relative_path)
{
    char *path = cd_path_buffer;
    s32 attempt;

    memcpy((void *)path, (const void *)cd_path_prefix, sizeof cd_path_prefix);
    strcat(path, relative_path);
    strcat(path, cd_version_suffix);
    if (CdSearchFile(&cd_search_file, path) == NULL) {
        return KF_RESOURCE_LOAD_FAILED;
    }
    if ((cd_search_file.size & (KF_CD_SECTOR_BYTES - 1)) != 0) {
        cd_search_file.size =
            ((cd_search_file.size >> KF_CD_SECTOR_SHIFT) + 1) << KF_CD_SECTOR_SHIFT;
    }
    CD_LOCATION_COPY(cd_read_location, cd_search_file.pos);
    for (attempt = 0; attempt < OPEN_CD_READ_ATTEMPTS; attempt++) {
        s32 result;

        CdControl(CdlSetloc, (u_char *)&cd_read_location, NULL);
        CdRead(
            cd_search_file.size >> KF_CD_SECTOR_SHIFT,
            (u_long *)destination,
            CdlModeSpeed);
        while ((result = CdReadSync(KF_CD_READ_POLL, NULL)) > 0) {
        }
        if (result == 0) {
            attempt = KF_CD_READ_STOP_ATTEMPT;
        }
    }
    return KF_RESOURCE_LOADED;
}

#include "../lib/tim_upload_images.inc"

#include "../lib/resource_copy_words.inc"

void opening_resources_load_scene0(void)
{
    u8 *stream;
    u8 *vab_chunk;
    const u32 *source;

    audio_stop_sequence(KF_AUDIO_STOP_FADE);
    audio_close_vab();
    memory_allocation_reset();
    cd_file_load_allocated(&stream, "B0\\MIXA0.");
    audio_load_vab(stream + KF_RESOURCE_CHUNK_HEADER_BYTES,
        (vab_chunk = RESOURCE_STREAM_NEXT(stream)) + KF_RESOURCE_CHUNK_HEADER_BYTES);
    RESOURCE_STREAM_NEXT(stream);
    source = resource_stream_copy_words(
        map_cell_attribute_grid.words,
        (const u32 *)(stream + KF_RESOURCE_CHUNK_HEADER_BYTES),
        MAP_GRID_WORDS);
    source = resource_stream_copy_words(
        map_floor_height_grid.words, source, MAP_GRID_WORDS);
    source = resource_stream_copy_words(
        map_cell_orientation_grid.words, source, MAP_GRID_WORDS);
    source = resource_stream_copy_words(
        opening_cell_storage.scene.collision_flags.words, source, MAP_GRID_WORDS);
    resource_stream_copy_words(
        map_collision_grid.words, source, MAP_GRID_WORDS);
    item_load_floor_placements(
        (KfFloorItemPlacement *)(RESOURCE_STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    opening_entity_pool_load_placements(
        (const KfMapObjectPlacement *)(RESOURCE_STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES),
        KF_OPENING_ENTITY_FLOOR_HEIGHT);
    RESOURCE_STREAM_NEXT(stream);
    memory_release_last();
    memory_arena.allocation.cursor = vab_chunk + KF_RESOURCE_REUSE_PREFIX_BYTES;
    audio_play_sequence_file("B0\\OPEN0.");
    cd_file_load_allocated(&stream, "B0\\MIXB0.");
    tmd_register(KF_TMD_SLOT_ENTITIES,
        (KfTmdHeader *)(stream + KF_RESOURCE_CHUNK_HEADER_BYTES));
    tmd_register(KF_TMD_SLOT_MAP,
        (KfTmdHeader *)(RESOURCE_STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    RESOURCE_STREAM_NEXT(stream);
    memory_set_allocation_mode(KF_MEMORY_USE_HEAP);
}

void opening_resources_load_scene1(void)
{
    u8 *stream;
    u8 *vab_chunk;

    audio_stop_sequence(KF_AUDIO_STOP_FADE);
    audio_close_vab();
    memory_allocation_reset();
    cd_file_load_allocated(&stream, "B0\\MIXA1.");
    audio_load_vab(stream + KF_RESOURCE_CHUNK_HEADER_BYTES,
        (vab_chunk = RESOURCE_STREAM_NEXT(stream)) + KF_RESOURCE_CHUNK_HEADER_BYTES);
    RESOURCE_STREAM_NEXT(stream);
    memory_release_last();
    opening_scene1_arena_cursor = vab_chunk + KF_RESOURCE_REUSE_PREFIX_BYTES;
    memory_arena.allocation.cursor = opening_scene1_arena_cursor;
    memory_set_allocation_mode(KF_MEMORY_USE_HEAP);
    audio_play_sequence_file("B0\\OPEN1.");
}

void opening_resources_load_scene3(void)
{
    u8 *tim_stream;
    u8 *stream;

    memory_allocation_reset();
    memory_arena.allocation.cursor = opening_scene1_arena_cursor;
    audio_play_sequence_file("B0\\OPEN3.");
    cd_file_load_allocated(&tim_stream, "B0\\MIX3.");
    tim_upload_images(tim_stream);
    memory_release_last();
    cd_file_load_allocated(&stream, "B0\\MIXA3.");
    opening_entity_pool_load_placements(
        (const KfMapObjectPlacement *)(stream + KF_RESOURCE_CHUNK_HEADER_BYTES),
        KF_OPENING_SCENE_BASE_Y);
    RESOURCE_STREAM_NEXT(stream);
    memory_release_last();
    cd_file_load_allocated(&stream, "B0\\MIXB3.");
    tmd_register(KF_TMD_SLOT_ENTITIES,
        (KfTmdHeader *)(stream + KF_RESOURCE_CHUNK_HEADER_BYTES));
    RESOURCE_STREAM_NEXT(stream);
    memory_release_last();
    memory_set_allocation_mode(KF_MEMORY_USE_HEAP);
}

void opening_resources_load_ending(void)
{
    u8 *tim_stream;
    u8 *stream;
    u8 *vab_chunk;
    u8 **arena_cursor = &memory_arena.allocation.cursor;

    memory_allocation_reset();
    cd_file_load_allocated(&tim_stream, "B0\\MIX9.");
    tim_upload_images(tim_stream);
    memory_release_last();
    cd_file_load_allocated(&stream, "B0\\MIXAE.");
    audio_load_vab(stream + KF_RESOURCE_CHUNK_HEADER_BYTES,
        (vab_chunk = RESOURCE_STREAM_NEXT(stream)) + KF_RESOURCE_CHUNK_HEADER_BYTES);
    opening_entity_pool_load_placements(
        (const KfMapObjectPlacement *)(RESOURCE_STREAM_NEXT(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES),
        KF_OPENING_SCENE_BASE_Y);
    RESOURCE_STREAM_NEXT(stream);
    memory_release_last();
    *arena_cursor = vab_chunk + KF_RESOURCE_REUSE_PREFIX_BYTES;
    audio_play_sequence_file(opening_ending_sequence_path);
    cd_file_load_allocated(&stream, "B0\\MIXBE.");
    tmd_register(KF_TMD_SLOT_ENTITIES,
        (KfTmdHeader *)(stream + KF_RESOURCE_CHUNK_HEADER_BYTES));
    RESOURCE_STREAM_NEXT(stream);
    opening_ending_arena_cursor = *arena_cursor;
    memory_set_allocation_mode(KF_MEMORY_USE_HEAP);
}

void opening_resources_load_ending_entities(void)
{
    u8 *stream;

    cd_file_load_allocated(&stream, "B0\\MIXAF.");
    opening_entity_pool_load_placements(
        (const KfMapObjectPlacement *)(stream + KF_RESOURCE_CHUNK_HEADER_BYTES),
        KF_OPENING_SCENE_BASE_Y);
    RESOURCE_STREAM_NEXT(stream);
    memory_release_last();
}

void opening_resources_load_ending_sequence(void)
{
    u8 *stream;
    u8 *vab_chunk;
    u8 **arena_cursor = &memory_arena.allocation.cursor;

    audio_stop_sequence(KF_AUDIO_STOP_IMMEDIATE);
    audio_close_vab();
    memory_allocation_reset();
    *arena_cursor = opening_ending_arena_cursor;
    cd_file_load_allocated(&stream, "B0\\MIXAG.");
    audio_load_vab(stream + KF_RESOURCE_CHUNK_HEADER_BYTES,
        (vab_chunk = RESOURCE_STREAM_NEXT(stream)) + KF_RESOURCE_CHUNK_HEADER_BYTES);
    memory_release_last();
    *arena_cursor = vab_chunk + KF_RESOURCE_REUSE_PREFIX_BYTES;
    audio_play_sequence_file("B0\\ENDG.");
}
