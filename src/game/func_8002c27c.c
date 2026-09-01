#include <kf/semantic_types.h>

extern KfSaveHeader *save_header_buffer;
extern KfSavePayload *save_payload_buffer;
extern void *func_8001ac0c(s32 size);
/* Psy-Q Release 2.5 MEMORY.H declares memset without a prototype. */
extern void *memset();

s32 func_8002c27c(void)
{
    save_header_buffer = func_8001ac0c(0x2800);
    if (save_header_buffer == 0) {
        return -1;
    }
    save_payload_buffer = (KfSavePayload *)(save_header_buffer + 1);
    memset(save_header_buffer, 0, sizeof(KfSaveHeader));
    memset(save_payload_buffer, 0, sizeof(KfSavePayload));
    return 0;
}
