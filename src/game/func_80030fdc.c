#include <kf/semantic_types.h>

extern KfMapObjectDefinition map_object_definitions[160];


void func_80030fdc(const KfMapObjectDefinition *definitions)
{
    const u32 *source = (const u32 *)definitions;
    u32 *destination = (u32 *)map_object_definitions;
    s32 count = 0x140;

    do {
        *destination++ = *source++;
    } while (--count != 0);
}
