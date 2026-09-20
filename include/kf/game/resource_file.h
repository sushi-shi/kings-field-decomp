#ifndef KF_GAME_RESOURCE_FILE_H
#define KF_GAME_RESOURCE_FILE_H

#include <kf/lib/resource_file.h>
#include <kf/lib/item.h>

struct KfItemModelFile {
    char path[32];
    std::size_t size;
};

extern KfItemModelFile item_model_files[KF_ITEM_COUNT];
void resource_file_index_item_models();
KfResourceLoadResult resource_file_load_item_model(u8 **destination, s32 index, std::size_t *loaded_size);

#endif
