#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfMapObjectState map_object_state;


ADDRESS(0x80030fdc, 0x2c)
void map_object_definitions_load(const KfMapObjectDefinition *definitions)
{
    const u32 *source = (const u32 *)definitions;
    u32 *destination = (u32 *)map_object_state.definitions;
    s32 count = 0x140;

    do {
        *destination++ = *source++;
    } while (--count != 0);
}
