#include <kf/address.h>
#include <kf/audio.h>
#include <kf/cd_file.h>
#include <kf/item.h>
#include <kf/map_data.h>
#include <kf/memory.h>
#include <kf/open_render.h>
#include <kf/open_resources.h>
#include <kf/psyq.h>
#include <kf/resources.h>

RODATA(0x8001205c, 0x24)

/* A resource chunk stores its payload byte length before the payload. */
#define STREAM_NEXT(stream) ((stream) += *(u32 *)(stream) + 4)
#define MAP_GRID_WORDS 0x9c4

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
    memory_set_allocation_mode(2);
}
