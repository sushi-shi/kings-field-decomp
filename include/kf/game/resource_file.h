#ifndef KF_GAME_RESOURCE_FILE_H
#define KF_GAME_RESOURCE_FILE_H

#include <array>
#include <kf/lib/resource_file.h>
#include <kf/lib/item.h>

inline constexpr unsigned item_model_path_capacity = 32;
inline constexpr unsigned menu_image_path_capacity = 16;
inline constexpr unsigned menu_image_number_offset = 5;

struct KfItemModelFile {
    std::array<char, item_model_path_capacity> path;
    std::size_t size;
};

extern std::array<KfItemModelFile, KF_ITEM_COUNT> item_model_files;
void resource_file_index_item_models();
KfResourceLoadResult resource_file_load_item_model(u8 **destination, s32 index, std::size_t *loaded_size);

#endif // KF_GAME_RESOURCE_FILE_H
