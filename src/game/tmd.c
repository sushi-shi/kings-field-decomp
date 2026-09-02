#include <kf/address.h>
#include <kf/semantic_types.h>

extern u8 *tmd_slots[8];
extern void *current_asset_data;
extern struct KfVec4s *current_tmd_vertices;

/* Object-table records follow the 12-byte TMD header of the selected asset. */
#define TMD_OBJECTS(asset) ((KfTmdObject *)((u8 *)(asset) + 12))

ADDRESS(0x8001c0e8, 0x2c)
void tmd_select(u16 index)
{
    current_asset_data = tmd_slots[index];
}

ADDRESS(0x8001c114, 0x24)
KfTmdObject *tmd_get_object(u16 index)
{
    return TMD_OBJECTS(current_asset_data) + index;
}

ADDRESS(0x8001c138, 0x10)
void tmd_set_current_vertices(struct KfVec4s *vertices)
{
    current_tmd_vertices = vertices;
}

ADDRESS(0x8001c148, 0x3c)
void tmd_select_object_vertices(u16 index)
{
    current_tmd_vertices =
        (struct KfVec4s *)((u8 *)current_asset_data + 12 + tmd_get_object(index)->vertex_offset);
}
