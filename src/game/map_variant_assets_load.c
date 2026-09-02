#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;

/* Psy-Q Release 2.5 MEMORY.H declares memcpy without a prototype. */
extern void *memcpy();

extern char map_resource_path[12];
extern const char map_variant_filename_template[9];
extern u8 *map_variant_asset_buffer;

extern s32 cd_file_load_into(void *destination, const char *relative_path);
extern void asset_registry_load_tmd_archive(u16 first_asset_id, u8 *archive);

ADDRESS(0x8001b414, 0x88)
void map_variant_assets_load(void)
{
    /* Both retail calls reload the buffer through one saved slot address. */
    u8 **asset_buffer = &map_variant_asset_buffer;

    memcpy(
        &map_resource_path[3],
        map_variant_filename_template,
        sizeof(map_variant_filename_template));
    map_resource_path[6] = player_state.map_variant + '0';
    cd_file_load_into(*asset_buffer, map_resource_path);
    asset_registry_load_tmd_archive(0, *asset_buffer);
}
