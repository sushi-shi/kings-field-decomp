#include <kf/address.h>
#include <kf/semantic_types.h>

extern MATRIX color_matrix_table[7];

ADDRESS(0x8001bab8, 0x2c)
void lighting_set_active_color_matrix(s32 index)
{
    SetColorMatrix(&color_matrix_table[index]);
}
