#ifndef KF_PLATFORM_AVATAR_DECODE_H
#define KF_PLATFORM_AVATAR_DECODE_H

#include <kf/lib/avatar.h>
#include <kf/lib/byte_reader.h>
#include <vector>

namespace kf::avatar {
using codec::Bytes;
using codec::Reader;
using codec::slice;
using codec::require;
inline u16 u16_at(Bytes bytes, std::size_t at) { return Reader(slice(bytes, at, 2)).u16_le(); }
inline u32 u32_at(Bytes bytes, std::size_t at) { return Reader(slice(bytes, at, 4)).u32_le(); }
inline void put16(std::vector<u8> &bytes, u16 value) { bytes.push_back(value); bytes.push_back(value >> 8); }
inline void put32(std::vector<u8> &bytes, u32 value) { put16(bytes, value); put16(bytes, value >> 16); }
std::vector<u8> convert(Bytes mo, Bytes rtim, Bytes fdat, Bytes mof);
}
#endif // KF_PLATFORM_AVATAR_DECODE_H
