#include <kf/address.h>
#include <kf/audio.h>
#include <kf/cd_file.h>
#include <kf/item.h>
#include <kf/map_data.h>
#include <kf/memory.h>
#include <kf/open_render.h>
#include <kf/open_resources.h>
#include <kf/psyq.h>
#include <kf/psyq_libc.h>
#include <kf/resources.h>

enum {
    OPEN_CD_READ_ATTEMPTS = 100,
    OPENING_ENTITY_SCENE_BASE_Y = -10000
};

/*
 * OPEN CD loading and opening-resource transitions form one contiguous code
 * run. Their initialized data is contiguous as well: the CD suffix is
 * immediately followed by the first opening-resource path.
 */
DATA(0x800372dc, 0x5)
char cd_path_prefix[5] = "\\KF\\";
DATA(0x800372e4, 0x3)
char cd_version_suffix[3] = ";1";
DATA(0x800372e8, 0x8)
char opening_ending_sequence_path[8] = "B0\\END.";

DATA(0x800375d8, 0x4)
static CdlLOC cd_read_location;

DATA(0x800375e0, 0x4)
static u8 *opening_scene1_arena_cursor;

DATA(0x800375e8, 0x4)
static u8 *opening_ending_arena_cursor;

RODATA(0x8001205c, 0xb4)

/* A resource chunk stores its payload byte length before the payload. */
#define STREAM_NEXT(stream) \
    ((stream) += *(u32 *)(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES)
#define MAP_GRID_WORDS (sizeof map_cell_attribute_grid / sizeof(u32))

/* Loads \KF\<relative_path>;1 into a fresh arena allocation. */
ADDRESS(0x80016014, 0x148)
KfResourceLoadResult cd_file_load_allocated(
    void **destination, const char *relative_path)
{
    char *path = cd_path_buffer;
    s32 attempt;

    memcpy(path, cd_path_prefix, sizeof cd_path_prefix);
    strcat(path, relative_path);
    strcat(path, cd_version_suffix);
    if (CdSearchFile(&cd_search_file, path) == 0) {
        return KF_RESOURCE_LOAD_FAILED;
    }
    if ((cd_search_file.size & (KF_CD_SECTOR_BYTES - 1)) != 0) {
        cd_search_file.size =
            ((cd_search_file.size >> KF_CD_SECTOR_SHIFT) + 1) << KF_CD_SECTOR_SHIFT;
    }
    *destination = memory_allocate(cd_search_file.size);
    cd_read_location.minute = cd_search_file.pos.minute;
    cd_read_location.second = cd_search_file.pos.second;
    cd_read_location.sector = cd_search_file.pos.sector;
    for (attempt = 0; attempt < OPEN_CD_READ_ATTEMPTS; attempt++) {
        s32 result;

        CdControl(CdlSetloc, (u_char *)&cd_read_location, 0);
        CdRead(
            cd_search_file.size >> KF_CD_SECTOR_SHIFT,
            (u_long *)*destination,
            CdlModeSpeed);
        while ((result = CdReadSync(KF_CD_READ_POLL, 0)) > 0) {
        }
        if (result == 0) {
            attempt = KF_CD_READ_STOP_ATTEMPT;
        }
    }
    return KF_RESOURCE_LOADED;
}

/* Loads \KF\<relative_path>;1 into caller-owned storage. */
ADDRESS(0x8001615c, 0x13c)
KfResourceLoadResult cd_file_load_into(
    void *destination, const char *relative_path)
{
    char *path = cd_path_buffer;
    s32 attempt;

    memcpy(path, cd_path_prefix, sizeof cd_path_prefix);
    strcat(path, relative_path);
    strcat(path, cd_version_suffix);
    if (CdSearchFile(&cd_search_file, path) == 0) {
        return KF_RESOURCE_LOAD_FAILED;
    }
    if ((cd_search_file.size & (KF_CD_SECTOR_BYTES - 1)) != 0) {
        cd_search_file.size =
            ((cd_search_file.size >> KF_CD_SECTOR_SHIFT) + 1) << KF_CD_SECTOR_SHIFT;
    }
    cd_read_location.minute = cd_search_file.pos.minute;
    cd_read_location.second = cd_search_file.pos.second;
    cd_read_location.sector = cd_search_file.pos.sector;
    for (attempt = 0; attempt < OPEN_CD_READ_ATTEMPTS; attempt++) {
        s32 result;

        CdControl(CdlSetloc, (u_char *)&cd_read_location, 0);
        CdRead(
            cd_search_file.size >> KF_CD_SECTOR_SHIFT,
            (u_long *)destination,
            CdlModeSpeed);
        while ((result = CdReadSync(KF_CD_READ_POLL, 0)) > 0) {
        }
        if (result == 0) {
            attempt = KF_CD_READ_STOP_ATTEMPT;
        }
    }
    return KF_RESOURCE_LOADED;
}

/* Uploads every CLUT and pixel image in a Psy-Q TIM stream. */
ADDRESS(0x80016298, 0x80)
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

/* Copies WORD_COUNT words and returns the first unread source word. */
ADDRESS(0x80016318, 0x30)
const u32 *resource_stream_copy_words(
    u32 *destination, const u32 *source, s32 word_count)
{
    while (word_count-- != 0) {
        *destination++ = *source++;
    }
    return source;
}

ADDRESS(0x80016348, 0x1c8)
void opening_resources_load_scene0(void)
{
    KfResourcePointer stream;
    u8 *vab_chunk;
    const u32 *source;

    audio_stop_sequence(KF_AUDIO_STOP_FADE);
    audio_close_vab();
    memory_allocation_reset();
    cd_file_load_allocated(&stream.storage, "B0\\MIXA0.");
    audio_load_vab(stream.bytes + KF_RESOURCE_CHUNK_HEADER_BYTES,
        (vab_chunk = STREAM_NEXT(stream.bytes)) + KF_RESOURCE_CHUNK_HEADER_BYTES);
    STREAM_NEXT(stream.bytes);
    source = resource_stream_copy_words(
        map_cell_attribute_grid.words,
        (const u32 *)(stream.bytes + KF_RESOURCE_CHUNK_HEADER_BYTES),
        MAP_GRID_WORDS);
    source = resource_stream_copy_words(
        map_floor_height_grid.words, source, MAP_GRID_WORDS);
    source = resource_stream_copy_words(
        map_cell_orientation_grid.words, source, MAP_GRID_WORDS);
    source = resource_stream_copy_words(
        map_collision_flag_grid.words, source, MAP_GRID_WORDS);
    resource_stream_copy_words(
        map_collision_grid.words, source, MAP_GRID_WORDS);
    item_load_floor_placements(
        (KfFloorItemPlacement *)(STREAM_NEXT(stream.bytes) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    opening_entity_pool_load_placements(
        (const KfMapObjectPlacement *)(STREAM_NEXT(stream.bytes) + KF_RESOURCE_CHUNK_HEADER_BYTES),
        KF_OPENING_ENTITY_FLOOR_HEIGHT);
    STREAM_NEXT(stream.bytes);
    memory_release_last();
    memory_arena.allocation.cursor = vab_chunk + KF_RESOURCE_REUSE_PREFIX_BYTES;
    audio_play_sequence_file("B0\\OPEN0.");
    cd_file_load_allocated(&stream.storage, "B0\\MIXB0.");
    tmd_register(KF_TMD_SLOT_ENTITIES,
        (KfTmdHeader *)(stream.bytes + KF_RESOURCE_CHUNK_HEADER_BYTES));
    tmd_register(KF_TMD_SLOT_MAP,
        (KfTmdHeader *)(STREAM_NEXT(stream.bytes) + KF_RESOURCE_CHUNK_HEADER_BYTES));
    STREAM_NEXT(stream.bytes);
    memory_set_allocation_mode(KF_MEMORY_USE_HEAP);
}

ADDRESS(0x80016510, 0xb4)
void opening_resources_load_scene1(void)
{
    KfResourcePointer stream;
    u8 *vab_chunk;

    audio_stop_sequence(KF_AUDIO_STOP_FADE);
    audio_close_vab();
    memory_allocation_reset();
    cd_file_load_allocated(&stream.storage, "B0\\MIXA1.");
    audio_load_vab(stream.bytes + KF_RESOURCE_CHUNK_HEADER_BYTES,
        (vab_chunk = STREAM_NEXT(stream.bytes)) + KF_RESOURCE_CHUNK_HEADER_BYTES);
    STREAM_NEXT(stream.bytes);
    memory_release_last();
    opening_scene1_arena_cursor = vab_chunk + KF_RESOURCE_REUSE_PREFIX_BYTES;
    memory_arena.allocation.cursor = opening_scene1_arena_cursor;
    memory_set_allocation_mode(KF_MEMORY_USE_HEAP);
    audio_play_sequence_file("B0\\OPEN1.");
}

ADDRESS(0x800165c4, 0xf0)
void opening_resources_load_scene3(void)
{
    KfResourcePointer tim_stream;
    KfResourcePointer stream;

    memory_allocation_reset();
    memory_arena.allocation.cursor = opening_scene1_arena_cursor;
    audio_play_sequence_file("B0\\OPEN3.");
    cd_file_load_allocated(&tim_stream.storage, "B0\\MIX3.");
    tim_upload_images(tim_stream.tim_data);
    memory_release_last();
    cd_file_load_allocated(&stream.storage, "B0\\MIXA3.");
    opening_entity_pool_load_placements(
        (const KfMapObjectPlacement *)(stream.bytes + KF_RESOURCE_CHUNK_HEADER_BYTES),
        OPENING_ENTITY_SCENE_BASE_Y);
    STREAM_NEXT(stream.bytes);
    memory_release_last();
    cd_file_load_allocated(&stream.storage, "B0\\MIXB3.");
    tmd_register(KF_TMD_SLOT_ENTITIES,
        (KfTmdHeader *)(stream.bytes + KF_RESOURCE_CHUNK_HEADER_BYTES));
    STREAM_NEXT(stream.bytes);
    memory_release_last();
    memory_set_allocation_mode(KF_MEMORY_USE_HEAP);
}

ADDRESS(0x800166b4, 0x134)
void opening_resources_load_ending(void)
{
    KfResourcePointer tim_stream;
    KfResourcePointer stream;
    u8 *vab_chunk;
    u8 **arena_cursor = &memory_arena.allocation.cursor;

    memory_allocation_reset();
    cd_file_load_allocated(&tim_stream.storage, "B0\\MIX9.");
    tim_upload_images(tim_stream.tim_data);
    memory_release_last();
    cd_file_load_allocated(&stream.storage, "B0\\MIXAE.");
    audio_load_vab(stream.bytes + KF_RESOURCE_CHUNK_HEADER_BYTES,
        (vab_chunk = STREAM_NEXT(stream.bytes)) + KF_RESOURCE_CHUNK_HEADER_BYTES);
    opening_entity_pool_load_placements(
        (const KfMapObjectPlacement *)(STREAM_NEXT(stream.bytes) + KF_RESOURCE_CHUNK_HEADER_BYTES),
        OPENING_ENTITY_SCENE_BASE_Y);
    STREAM_NEXT(stream.bytes);
    memory_release_last();
    *arena_cursor = vab_chunk + KF_RESOURCE_REUSE_PREFIX_BYTES;
    audio_play_sequence_file(opening_ending_sequence_path);
    cd_file_load_allocated(&stream.storage, "B0\\MIXBE.");
    tmd_register(KF_TMD_SLOT_ENTITIES,
        (KfTmdHeader *)(stream.bytes + KF_RESOURCE_CHUNK_HEADER_BYTES));
    STREAM_NEXT(stream.bytes);
    opening_ending_arena_cursor = *arena_cursor;
    memory_set_allocation_mode(KF_MEMORY_USE_HEAP);
}

ADDRESS(0x800167e8, 0x58)
void opening_resources_load_ending_entities(void)
{
    KfResourcePointer stream;

    cd_file_load_allocated(&stream.storage, "B0\\MIXAF.");
    opening_entity_pool_load_placements(
        (const KfMapObjectPlacement *)(stream.bytes + KF_RESOURCE_CHUNK_HEADER_BYTES),
        OPENING_ENTITY_SCENE_BASE_Y);
    STREAM_NEXT(stream.bytes);
    memory_release_last();
}

ADDRESS(0x80016840, 0x9c)
void opening_resources_load_ending_sequence(void)
{
    KfResourcePointer stream;
    u8 *vab_chunk;
    u8 **arena_cursor = &memory_arena.allocation.cursor;

    audio_stop_sequence(KF_AUDIO_STOP_IMMEDIATE);
    audio_close_vab();
    memory_allocation_reset();
    *arena_cursor = opening_ending_arena_cursor;
    cd_file_load_allocated(&stream.storage, "B0\\MIXAG.");
    audio_load_vab(stream.bytes + KF_RESOURCE_CHUNK_HEADER_BYTES,
        (vab_chunk = STREAM_NEXT(stream.bytes)) + KF_RESOURCE_CHUNK_HEADER_BYTES);
    memory_release_last();
    *arena_cursor = vab_chunk + KF_RESOURCE_REUSE_PREFIX_BYTES;
    audio_play_sequence_file("B0\\ENDG.");
}
