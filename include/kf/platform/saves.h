#ifndef KF_PLATFORM_SAVES_H
#define KF_PLATFORM_SAVES_H

#include <kf/lib/types.h>

#include <cstddef>
#include <array>

namespace kf {
enum class SaveSlot : u8 { First = 1, Second = 2, Third = 3 };
enum class SaveFileResult : int { Ok = 0, Missing = 1, Invalid = 2, IoError = 3, NoSpace = 4, Unavailable = 5, Pending = 6 };
inline constexpr std::size_t save_file_capacity = 16384;
inline constexpr std::size_t campaign_file_capacity = 132 * 1024;
bool save_storage_start(const char *directory = nullptr);
void save_storage_shutdown();
// Create once, then retain across sessions. Private credential, never a world field.
bool online_profile_load(std::array<u8, 32> &credential);
SaveFileResult save_file_read(SaveSlot slot, u8 *data, std::size_t capacity, std::size_t *size);
// Success means the replacement has completed its durable commit, not just a write request.
// An I/O error after native rename may leave the new file visible without proving durability.
SaveFileResult save_file_write(SaveSlot slot, const u8 *data, std::size_t size);
SaveFileResult campaign_file_read(SaveSlot slot, u8 *data, std::size_t capacity, std::size_t *size);
bool campaign_file_write_begin(SaveSlot slot, const u8 *data, std::size_t size);
SaveFileResult campaign_file_write_poll();
}

#endif // KF_PLATFORM_SAVES_H
