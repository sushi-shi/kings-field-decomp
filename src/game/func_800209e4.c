#include <kf/game_types.h>

extern void free(void *allocation);

void func_800209e4(struct KfPoolRecord *record)
{
    record->state = 0;
    *record->backlink = 0;
    if (record->allocation != 0) {
        free(record->allocation);
        record->allocation = 0;
    }
}
