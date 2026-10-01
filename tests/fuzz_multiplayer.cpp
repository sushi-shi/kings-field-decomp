#include <kf/net/world.h>
#include <kf/lib/avatar.h>

#include <algorithm>
#include <array>
#include <memory>

// Compile with -fsanitize=fuzzer,address,undefined. Seed worlds by splitting the
// length-prefixed corpus emitted by coop-world-codec-test into individual files.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    KfNetHeader header{};
    KfNetInputBundle input{};
    KfNetCommand command{};
    KfNetInteraction interaction{};
    KfNetFragment fragment{};
    kf_net_header_decode(data, size, &header);
    kf_net_input_decode(data, size, &input);
    kf_net_command_decode(data, size, &command);
    kf_net_interaction_decode(data, size, &interaction);
    kf_net_fragment_decode(data, size, &fragment);
    KfNetTransfer transfer{};
    auto payload = std::make_unique<std::array<uint8_t, KF_NET_TRANSFER_LIMIT>>();
    kf_net_fragment_accept(&transfer, data, size, payload->data(), payload->size());

    KfNetWorldHeader world_header{};
    KfNetWorldSummary summary{};
    KfNetWorldLimits limits{};
    std::fill(std::begin(limits.asset_clips), std::end(limits.asset_clips), 256);
    auto world = std::make_unique<KfNetWorld>();
    kf_net_world_info(data, size, &world_header);
    kf_net_world_preflight(data, size, &world_header);
    kf_net_world_summary(data, size, &summary);
    kf_net_world_decode(data, size, &limits, world.get());

    auto *pack = kf_avatar_open(data, size);
    if (pack) {
        for (uint32_t index = 0; index < kf_avatar_count(pack); ++index) {
            KfAvatarMesh mesh{};
            kf_avatar_mesh(pack, index, &mesh);
            kf_avatar_vertices(pack, index);
            kf_avatar_rgba(pack, index);
        }
        kf_avatar_close(pack);
    }
    // Bound the disc size and supply only complete requested chunks. This
    // exercises importer states without filesystem access or retail assets.
    auto *importer = kf_avatar_import_open(32 * 1024 * 1024);
    size_t offset = 0;
    KfAvatarRead read{};
    for (unsigned step = 0; step < 8 && kf_avatar_import_request(importer, &read) == 1; ++step) {
        if (read.length > size - offset) break;
        if (!kf_avatar_import_supply(importer, data + offset, read.length)) break;
        offset += read.length;
    }
    kf_avatar_import_close(importer);
    return 0;
}
