#include "../src/lib/resources.cpp"

#include <cassert>
#include <string_view>
#include <vector>

namespace kf {
[[noreturn]] void host_fail(const char *message)
{
    std::fprintf(stderr, "%s\n", message);
    std::exit(77);
}
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    const std::string_view scenario = argv[1];
    constexpr std::size_t record_bytes = 8 + KF_CELL_WINDOW_CELL_CAPACITY;
    std::vector<u8> bytes(record_bytes * KF_CELL_WINDOW_YAW_COUNT);
    for (std::size_t i = 0; i < bytes.size(); i += record_bytes) {
        bytes[i] = 14;
        bytes[i + 2] = 14;
        bytes[i + 4] = 6;
        bytes[i + 6] = 7;
        bytes[i + record_bytes - 1] = 2;
    }
    if (scenario == "truncated") bytes.pop_back();
    if (scenario == "zero-width") bytes[0] = 0;
    if (scenario == "zero-height") bytes[2] = 0;
    if (scenario == "oversized") bytes[0] = 15;
    if (scenario == "wide") { bytes[0] = 0; bytes[1] = 1; bytes[2] = 1; }
    if (scenario == "origin") bytes[4] = 14;
    if (scenario == "visibility") bytes.back() = 3;
    const auto size = bytes.size();
    if (scenario == "unaligned") bytes.insert(bytes.begin(), 0);
    std::array<KfCellWindow, KF_CELL_WINDOW_YAW_COUNT> windows{};
    cell_windows_load({bytes.data() + (scenario == "unaligned"), size}, windows);
    for (const auto &window : windows) {
        assert(window.width == 14 && window.height == 14);
        assert(window.origin_x == 6 && window.origin_z == 7);
        assert(window.cells[0] == KF_CELL_WINDOW_HIDDEN);
        assert(window.cells[KF_CELL_WINDOW_CELL_CAPACITY - 1] == KF_CELL_WINDOW_NEAR);
    }
}
