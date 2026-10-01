#include "../src/lib/tmd.cpp"

#include <cassert>
#include <string_view>

namespace kf {
[[noreturn]] void host_fail(const char *message)
{
    std::fprintf(stderr, "%s\n", message);
    std::exit(77);
}
}
void memory_release_last(KfMemoryArena &) {}

int main(int argc, char **argv)
{
    assert(argc == 2);
    const std::string_view scenario = argv[1];
    struct Model {
        KfTmdHeader header;
        KfTmdObject object;
        std::array<SVECTOR, 2> vertices;
    } model{{0, 0, 1}, {sizeof(KfTmdObject), 2, 0, 0, 0, 0, 0}, {}};
    std::array<KfTmdResource, 5> slots{};
    KfTmdResource current{};
    std::span<const SVECTOR> vertices;
    std::array<KfScreenVertex, KF_PROJECTED_VERTEX_CAPACITY> projected{};
    KfTmdContext context{slots, current, vertices, projected};
    auto register_model = [&] {
        tmd_register(context, KF_TMD_SLOT_MAP, reinterpret_cast<u8 *>(&model), sizeof model);
    };
    register_model();
    tmd_select_object_vertices(context, 0);
    assert(tmd_vertices(context, 2).data() == model.vertices.data());
    assert(tmd_vertices(context, 2).size() == 2);
    if (scenario == "short-source") tmd_vertices(context, 3);
    if (scenario == "negative") tmd_vertices(context, -1);
    if (scenario == "capacity") tmd_vertices(context, KF_PROJECTED_VERTEX_CAPACITY + 1);
    if (scenario == "select") tmd_select(context, KF_TMD_SLOT_MAP);
    if (scenario == "register") register_model();
    if (scenario == "release") {
        KfMemoryArena arena{};
        tmd_release_last_allocation(context, arena, KF_TMD_SLOT_MAP);
        assert(!current.data && !slots[0].data);
    }
    tmd_vertices(context, 2);
}
