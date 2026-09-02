#include <kf/address.h>
#include <kf/semantic_types.h>

extern struct KfMatrix color_matrix_table[7];
extern void SetColorMatrix(struct KfMatrix *matrix);

ADDRESS(0x8001bab8, 0x2c)
void func_8001bab8(s32 index)
{
    SetColorMatrix(&color_matrix_table[index]);
}
