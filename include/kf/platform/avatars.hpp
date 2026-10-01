#pragma once
#include <kf/lib/avatar.h>
namespace kf {
struct AvatarMesh {
    KfAvatarMesh info;
    const KfAvatarVertex *vertices;
    const uint8_t *rgba;
};
bool avatars_load(const char *path);
bool avatars_import_disc(const char *path);
void avatars_release();
const AvatarMesh *avatar_mesh(unsigned slot);
unsigned avatar_texture(unsigned slot);
const char *avatars_hash();
}
