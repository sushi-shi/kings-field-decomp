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
#define STREAM_NEXT(stream) ((stream) += *(u32 *)(stream) + 4)
#define MAP_GRID_WORDS 0x9c4
#define CD_SECTOR_SIZE 0x800
#define CD_SECTOR_SHIFT 11

/* Loads \KF\<relative_path>;1 into a fresh arena allocation. */
ADDRESS(0x80016014, 0x148)
s32 cd_file_load_allocated(
    void **destination, const char *relative_path)
{
    char *path = cd_path_buffer;
    s32 attempt;

    memcpy(path, cd_path_prefix, sizeof cd_path_prefix);
    strcat(path, relative_path);
    strcat(path, cd_version_suffix);
    if (CdSearchFile(&cd_search_file, path) == 0) {
        return 1;
    }
    if ((cd_search_file.size & (CD_SECTOR_SIZE - 1)) != 0) {
        cd_search_file.size =
            ((cd_search_file.size >> CD_SECTOR_SHIFT) + 1) << CD_SECTOR_SHIFT;
    }
    *destination = memory_allocate(cd_search_file.size);
    cd_read_location.minute = cd_search_file.pos.minute;
    cd_read_location.second = cd_search_file.pos.second;
    cd_read_location.sector = cd_search_file.pos.sector;
    for (attempt = 0; attempt < 100; attempt++) {
        s32 result;

        CdControl(CdlSetloc, (u_char *)&cd_read_location, 0);
        CdRead(
            cd_search_file.size >> CD_SECTOR_SHIFT,
            *destination,
            CdlModeSpeed);
        while ((result = CdReadSync(1, 0)) > 0) {
        }
        if (result == 0) {
            attempt = 100;
        }
    }
    return 0;
}

/* Loads \KF\<relative_path>;1 into caller-owned storage. */
ADDRESS(0x8001615c, 0x13c)
s32 cd_file_load_into(
    void *destination, const char *relative_path)
{
    char *path = cd_path_buffer;
    s32 attempt;

    memcpy(path, cd_path_prefix, sizeof cd_path_prefix);
    strcat(path, relative_path);
    strcat(path, cd_version_suffix);
    if (CdSearchFile(&cd_search_file, path) == 0) {
        return 1;
    }
    if ((cd_search_file.size & (CD_SECTOR_SIZE - 1)) != 0) {
        cd_search_file.size =
            ((cd_search_file.size >> CD_SECTOR_SHIFT) + 1) << CD_SECTOR_SHIFT;
    }
    cd_read_location.minute = cd_search_file.pos.minute;
    cd_read_location.second = cd_search_file.pos.second;
    cd_read_location.sector = cd_search_file.pos.sector;
    for (attempt = 0; attempt < 100; attempt++) {
        s32 result;

        CdControl(CdlSetloc, (u_char *)&cd_read_location, 0);
        CdRead(
            cd_search_file.size >> CD_SECTOR_SHIFT,
            destination,
            CdlModeSpeed);
        while ((result = CdReadSync(1, 0)) > 0) {
        }
        if (result == 0) {
            attempt = 100;
        }
    }
    return 0;
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
    u8 *stream;
    u8 *vab_chunk;
    const u32 *source;

    audio_stop_sequence(1);
    audio_close_vab();
    memory_allocation_reset();
    cd_file_load_allocated((void **)&stream, "B0\\MIXA0.");
    audio_load_vab(stream + 4, (vab_chunk = STREAM_NEXT(stream)) + 4);
    STREAM_NEXT(stream);
    source = resource_stream_copy_words(
        (u32 *)map_cell_attribute_grid,
        (const u32 *)(stream + 4),
        MAP_GRID_WORDS);
    source = resource_stream_copy_words(
        (u32 *)map_floor_height_grid, source, MAP_GRID_WORDS);
    source = resource_stream_copy_words(
        (u32 *)map_cell_orientation_grid, source, MAP_GRID_WORDS);
    source = resource_stream_copy_words(
        (u32 *)map_collision_flag_grid, source, MAP_GRID_WORDS);
    resource_stream_copy_words(
        (u32 *)map_collision_grid, source, MAP_GRID_WORDS);
    item_load_floor_placements(
        (KfFloorItemPlacement *)(STREAM_NEXT(stream) + 4));
    opening_entity_pool_load_placements(
        (const KfMapObjectPlacement *)(STREAM_NEXT(stream) + 4), 0);
    STREAM_NEXT(stream);
    memory_release_last();
    memory_arena_cursor = vab_chunk + 16;
    audio_play_sequence_file("B0\\OPEN0.");
    cd_file_load_allocated((void **)&stream, "B0\\MIXB0.");
    tmd_register(1, stream + 4);
    tmd_register(0, STREAM_NEXT(stream) + 4);
    STREAM_NEXT(stream);
    memory_set_allocation_mode(KF_MEMORY_USE_HEAP);
}

ADDRESS(0x80016510, 0xb4)
void opening_resources_load_scene1(void)
{
    u8 *stream;
    u8 *vab_chunk;

    audio_stop_sequence(1);
    audio_close_vab();
    memory_allocation_reset();
    cd_file_load_allocated((void **)&stream, "B0\\MIXA1.");
    audio_load_vab(stream + 4, (vab_chunk = STREAM_NEXT(stream)) + 4);
    STREAM_NEXT(stream);
    memory_release_last();
    opening_scene1_arena_cursor = vab_chunk + 16;
    memory_arena_cursor = opening_scene1_arena_cursor;
    memory_set_allocation_mode(KF_MEMORY_USE_HEAP);
    audio_play_sequence_file("B0\\OPEN1.");
}

ADDRESS(0x800165c4, 0xf0)
void opening_resources_load_scene3(void)
{
    u_long *tim_stream;
    u8 *stream;

    memory_allocation_reset();
    memory_arena_cursor = opening_scene1_arena_cursor;
    audio_play_sequence_file("B0\\OPEN3.");
    cd_file_load_allocated((void **)&tim_stream, "B0\\MIX3.");
    tim_upload_images(tim_stream);
    memory_release_last();
    cd_file_load_allocated((void **)&stream, "B0\\MIXA3.");
    opening_entity_pool_load_placements(
        (const KfMapObjectPlacement *)(stream + 4), -10000);
    STREAM_NEXT(stream);
    memory_release_last();
    cd_file_load_allocated((void **)&stream, "B0\\MIXB3.");
    tmd_register(1, stream + 4);
    STREAM_NEXT(stream);
    memory_release_last();
    memory_set_allocation_mode(KF_MEMORY_USE_HEAP);
}

ADDRESS(0x800166b4, 0x134)
void opening_resources_load_ending(void)
{
    u_long *tim_stream;
    u8 *stream;
    u8 *vab_chunk;
    u8 **arena_cursor = &memory_arena_cursor;

    memory_allocation_reset();
    cd_file_load_allocated((void **)&tim_stream, "B0\\MIX9.");
    tim_upload_images(tim_stream);
    memory_release_last();
    cd_file_load_allocated((void **)&stream, "B0\\MIXAE.");
    audio_load_vab(stream + 4, (vab_chunk = STREAM_NEXT(stream)) + 4);
    opening_entity_pool_load_placements(
        (const KfMapObjectPlacement *)(STREAM_NEXT(stream) + 4), -10000);
    STREAM_NEXT(stream);
    memory_release_last();
    *arena_cursor = vab_chunk + 16;
    audio_play_sequence_file(opening_ending_sequence_path);
    cd_file_load_allocated((void **)&stream, "B0\\MIXBE.");
    tmd_register(1, stream + 4);
    STREAM_NEXT(stream);
    opening_ending_arena_cursor = *arena_cursor;
    memory_set_allocation_mode(KF_MEMORY_USE_HEAP);
}

ADDRESS(0x800167e8, 0x58)
void opening_resources_load_ending_entities(void)
{
    u8 *stream;

    cd_file_load_allocated((void **)&stream, "B0\\MIXAF.");
    opening_entity_pool_load_placements(
        (const KfMapObjectPlacement *)(stream + 4), -10000);
    STREAM_NEXT(stream);
    memory_release_last();
}

ADDRESS(0x80016840, 0x9c)
void opening_resources_load_ending_sequence(void)
{
    u8 *stream;
    u8 *vab_chunk;
    u8 **arena_cursor = &memory_arena_cursor;

    audio_stop_sequence(0);
    audio_close_vab();
    memory_allocation_reset();
    *arena_cursor = opening_ending_arena_cursor;
    cd_file_load_allocated((void **)&stream, "B0\\MIXAG.");
    audio_load_vab(stream + 4, (vab_chunk = STREAM_NEXT(stream)) + 4);
    memory_release_last();
    *arena_cursor = vab_chunk + 16;
    audio_play_sequence_file("B0\\ENDG.");
}
