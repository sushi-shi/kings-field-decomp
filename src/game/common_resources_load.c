#include <kf/address.h>
#include <kf/semantic_types.h>
#include <MEMORY.H>
#include <kf/game.h>

extern s32 cd_file_load_allocated(void **destination, char *relative_path);
extern void player_weapon_load_records_and_mirror_angles(const KfWeaponRecord *source);
extern void magic_load_records(void *block);

extern u8 DAT_80065be8[3264];

RODATA(0x80012178, 0x18)

/*
 * Advances a chunked stream to its next chunk: each chunk is a byte length
 * followed by the payload.  The macro assigns so callers can pass
 * STREAM_NEXT(stream) + 4 (the new payload) as an argument.
 */
#define STREAM_NEXT(stream) ((stream) += *(u32 *)(stream) + 4)

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
    memcpy(DAT_80065be8, block + 4, sizeof DAT_80065be8);
    player_weapon_load_records_and_mirror_angles((KfWeaponRecord *)(STREAM_NEXT(stream) + 4));
    asset_aux_block_load((u32 *)(STREAM_NEXT(stream) + 4));
    magic_load_records(STREAM_NEXT(stream) + 4);
    map_object_definitions_load((KfMapObjectDefinition *)(STREAM_NEXT(stream) + 4));
    memcpy(
        player_level_growth_table,
        (KfPlayerLevelGrowth *)(STREAM_NEXT(stream) + 4),
        sizeof player_level_growth_table);
    memory_release_last();
    memory_arena_cursor = block + 16;
}
