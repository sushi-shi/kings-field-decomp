#include <kf/open_render.h>
#include <kf/gpu_packets.h>

#define OFFSET(type, field) ((unsigned long)&((type *)0)->field)
#define CHECK(name, condition) typedef char name[(condition) ? 1 : -1]

#ifndef EXPECTED_COLLISION_GRID_OFFSET
#define EXPECTED_COLLISION_GRID_OFFSET 0xcf0
#endif

CHECK(cd_sector_words, KF_CD_SECTOR_WORDS * sizeof(u32) == KF_CD_SECTOR_BYTES);
CHECK(rtbl_payload, sizeof(((KfOpeningSceneCells *)0)->windows) == 0xcc0);
CHECK(rtbl_transfer, sizeof(((KfOpeningCellStorage *)0)->rtbl_sectors) == 0x1000);
CHECK(cell_storage, sizeof(KfOpeningCellStorage) == 0x3400);
CHECK(cell_storage_alignment, __alignof__(KfOpeningCellStorage) == 4);
CHECK(scene_origin, OFFSET(KfOpeningCellStorage, scene) == 0);
CHECK(grid_offset, OFFSET(KfOpeningCellStorage, scene.collision_flags)
    == EXPECTED_COLLISION_GRID_OFFSET);
CHECK(grid_end, OFFSET(KfOpeningCellStorage, scene.collision_flags)
    + sizeof(KfMapGrid) == sizeof(KfOpeningCellStorage));
CHECK(startup_overlap, OFFSET(KfOpeningCellStorage, scene.collision_flags)
    < sizeof(((KfOpeningCellStorage *)0)->rtbl_sectors));
CHECK(gt3_allocation, sizeof(KfGpuGT3) == sizeof(POLY_GT3)
    && sizeof(KfGpuGT3) == 40);
CHECK(gt4_allocation, sizeof(KfGpuGT4) == sizeof(POLY_GT4)
    && sizeof(KfGpuGT4) == 52);
