#include <kf/address.h>
#include <kf/semantic_types.h>

extern void lighting_set_active_color_matrix(s32 index);
extern void tmd_select(u16 slot);
extern void tmd_select_object_vertices(u16 object_index);
extern KfTmdObject *tmd_get_object(u16 object_index);
extern void func_8001c60c(s32 arg0);
extern void func_8001c7f8(u16 arg0, s16 arg1);

ADDRESS(0x8001ed38, 0x58)
void func_8001ed38(void)
{
    lighting_set_active_color_matrix(0);
    SetGeomScreen(0xc8);
    tmd_select(4);
    tmd_select_object_vertices(0);
    func_8001c60c(tmd_get_object(0)->vertex_count);
    func_8001c7f8(0, 0x3e8);
}
