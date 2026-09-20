#include <kf/game/resource_file.h>
#include "../lib/resource_file.inc"

KfItemModelFile item_model_files[KF_ITEM_COUNT];

void resource_file_index_item_models()
{
    for (s32 i = 0; i < KF_ITEM_COUNT; ++i) {
        auto *entry = &item_model_files[i];
        snprintf(entry->path, sizeof entry->path, "ITEM%d/I%03d.TMD", i / 30 + 1, i + 1);
        entry->size = 0;
        kf::DataFile file {};
        const auto result = resource_file_open(&file, entry->path);
        if (result == kf::FileResult::Ok)
            entry->size = file.size;
        else if (result != kf::FileResult::NotFound)
            kf::data_file_report_error(result, entry->path);
        kf::data_file_close(&file);
    }
}

KfResourceLoadResult resource_file_load_item_model(u8 **destination, s32 index, std::size_t *loaded_size)
{
    *destination = NULL;
    *loaded_size = 0;
    if (index < 0 || index >= KF_ITEM_COUNT || !item_model_files[index].size)
        return KF_RESOURCE_LOAD_FAILED;
    return resource_file_try_load_allocated(destination, item_model_files[index].path, loaded_size);
}


void resource_file_reset_module_state(void)
{
    kf::restore_initial_value<item_model_files>();
}
