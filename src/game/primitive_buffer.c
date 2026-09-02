#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfDisplayState display_state;

struct PolyFT4_8002accc {
    u32 tag;
    u8 r0;
    u8 g0;
    u8 b0;
    u8 code;
    u8 remainder[32];
};

extern struct PolyFT4_8002accc *current_poly_ft4;
extern void SetPolyFT4(struct PolyFT4_8002accc *primitive);


ADDRESS(0x8002accc, 0x50)
void primitive_buffer_begin_poly_ft4(void)
{
    SetPolyFT4(current_poly_ft4);
    current_poly_ft4->r0 = 0x60;
    current_poly_ft4->g0 = 0x60;
    current_poly_ft4->b0 = 0x60;
}

ADDRESS(0x8002ad1c, 0x50)
void primitive_buffer_commit_poly_ft4(s32 depth)
{
    depth <<= 2;
    AddPrim((u32 *)((u8 *)display_state.ordering_table + depth), current_poly_ft4);
    current_poly_ft4++;
    display_state.primitive_buffer->cursor = (u8 *)current_poly_ft4;
}
