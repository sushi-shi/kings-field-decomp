#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfActorDefinition actor_definitions[12];

ADDRESS(0x80030a6c, 0x2c)
void actor_definitions_load(const KfActorDefinition *definitions)
{
    const u32 *source = (const u32 *)definitions;
    u32 *destination = (u32 *)actor_definitions;
    s32 count = 0x1c8;

    do {
        *destination++ = *source++;
    } while (--count != 0);
}
