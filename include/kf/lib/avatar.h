#pragma once
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
/* IDs 0..42 retain MO archive slots; 43 is the MOF 545 throne model. */
enum { KF_AVATAR_MAX_BYTES = 32 * 1024 * 1024, KF_AVATAR_SLOTS = 44 };
typedef struct KfAvatarPack KfAvatarPack;
typedef struct KfAvatarImport KfAvatarImport;
typedef struct KfAvatarRead { uint32_t offset, length; } KfAvatarRead;
typedef struct KfAvatarVertex {
    int16_t x, y, z;
    int16_t nx, ny, nz;
    uint16_t u, v;
    uint8_t r, g, b, unlit;
} KfAvatarVertex;
typedef struct KfAvatarMesh {
    uint32_t triangles;
    uint16_t slot, height;
} KfAvatarMesh;
/* Views remain valid until close. Packs are local resources, never peer data. */
KfAvatarPack *kf_avatar_open(const uint8_t *bytes, size_t length);
void kf_avatar_close(KfAvatarPack *pack);
uint32_t kf_avatar_count(const KfAvatarPack *pack);
int kf_avatar_mesh(const KfAvatarPack *pack, uint32_t index, KfAvatarMesh *mesh);
const KfAvatarVertex *kf_avatar_vertices(const KfAvatarPack *pack, uint32_t index);
const uint8_t *kf_avatar_rgba(const KfAvatarPack *pack, uint32_t index);
/* Pull-based local disc import: request returns 1/read, 0/complete, -1/failed.
 * Supply exactly the requested byte range. Result storage lives until close. */
KfAvatarImport *kf_avatar_import_open(uint32_t disc_size);
void kf_avatar_import_close(KfAvatarImport *importer);
int kf_avatar_import_request(const KfAvatarImport *importer, KfAvatarRead *read);
int kf_avatar_import_supply(KfAvatarImport *importer, const uint8_t *bytes, size_t length);
const uint8_t *kf_avatar_import_result(const KfAvatarImport *importer, uint32_t *length);
void kf_avatar_import_error(const KfAvatarImport *importer, uint8_t *message, size_t capacity);
#ifdef __cplusplus
}
#endif
