#include <kf/address.h>
#include <kf/game_types.h>
#include <kf/game.h>

/* Psy-Q Release 2.5 MEMORY.H declares strcpy without a prototype. */
extern char *strcpy();

extern s32 cd_file_load_allocated(void **data_out, const char *relative_path);

ADDRESS(0x8001b3a4, 0x40)
void *map_resource_load_file(const char *filename)
{
    void *data;

    strcpy(&map_resource_path[3], filename);
    cd_file_load_allocated(&data, map_resource_path);
    return data;
}
