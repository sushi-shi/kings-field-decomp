#include <kf/semantic_types.h>

extern KfActorDefinition actor_definitions[12];


void func_80030a6c(const KfActorDefinition *definitions)
{
    const u32 *source = (const u32 *)definitions;
    u32 *destination = (u32 *)actor_definitions;
    s32 count = 0x1c8;

    do {
        *destination++ = *source++;
    } while (--count != 0);
}
