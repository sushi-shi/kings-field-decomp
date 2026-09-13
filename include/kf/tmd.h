#ifndef KF_TMD_H
#define KF_TMD_H

/* Shared TMD payload, primitive packet, projection layouts, and game APIs. */

#include <kf/game_types.h>
#include <kf/enum.h>
#include <psyq/sdk.h>

KF_ENUM_BEGIN(KfTmdSlot, u16)
    KF_TMD_SLOT_MAP = 0,
    KF_TMD_SLOT_ENTITIES = 1,
    KF_TMD_SLOT_MENU_ITEM = 4
KF_ENUM_END(KfTmdSlot)

enum {
    KF_TMD_HEADER_BYTES = 12,
    KF_TMD_PACKET_HEADER_BYTES = 4,
    KF_TMD_WORD_BYTES = 4,
    KF_TMD_ILEN_BYTE = 1,
    KF_TMD_MODE_SHIFT = 24,
    KF_TMD_MODE_MASK = 0xfd,
    KF_TMD_ILEN_TO_BYTES_SHIFT = 6,
    KF_TMD_BODY_BYTES_MASK = 0x3fc,
    KF_TMD_VECTOR_OFFSET_SHIFT = 3,
    KF_TMD_DEFAULT_PERSPECTIVE_SHIFT = 1
};

/* Primitive mode bits retain the semitransparency modifier in packet storage. */
KF_ENUM_BEGIN(KfTmdMode, u8)
    KF_TMD_MODE_NONE = 0,
    KF_TMD_MODE_SEMITRANS = 0x02,
    KF_TMD_MODE_F3 = 0x20,
    KF_TMD_MODE_FT3 = 0x24,
    KF_TMD_MODE_F4 = 0x28,
    KF_TMD_MODE_FT4 = 0x2c,
    KF_TMD_MODE_G3 = 0x30,
    KF_TMD_MODE_GT3 = 0x34,
    KF_TMD_MODE_G4 = 0x38,
    KF_TMD_MODE_GT4 = 0x3c
KF_ENUM_END(KfTmdMode)
KF_ENUM_FLAGS(KfTmdMode, u8)

#if KF_MODERN_TYPES
constexpr KfTmdMode tmd_packet_mode(u32 word)
{
    return KF_ENUM_DECODE(KfTmdMode, word >> KF_TMD_MODE_SHIFT);
}
constexpr KfTmdMode tmd_packet_kind(u32 word)
{
    return KF_ENUM_DECODE(KfTmdMode, (word >> KF_TMD_MODE_SHIFT) & KF_TMD_MODE_MASK);
}
#else
#define tmd_packet_mode(word) ((word) >> KF_TMD_MODE_SHIFT)
#define tmd_packet_kind(word) (((word) >> KF_TMD_MODE_SHIFT) & KF_TMD_MODE_MASK)
#endif

/* On-disk counts are words; individual consumers may narrow them. */
typedef struct KfTmdHeader {
    u32 id;
    u32 flags;
    u32 object_count;
} KfTmdHeader;

/* Standard 0x1c-byte object-table record in an unlinked TMD payload. */
typedef struct KfTmdObject {
    u32 vertex_offset;
    u32 vertex_count;
    u32 normal_offset;
    u32 normal_count;
    u32 primitive_offset;
    u32 primitive_count;
    s32 scale;
} KfTmdObject;

/* Four-byte primitive header, viewed both as its packed word and disk bytes. */
typedef union KfTmdPacketHeader {
    u32 word;
    struct {
        u8 output_length;
        u8 input_length;
        u8 flag;
        KfTmdMode mode;
    } bytes;
} KfTmdPacketHeader;

/*
 * Primitive bodies follow the four-byte olen/ilen/flag/mode header. n0..n3
 * are normal indices, v0..v3 are vertex indices, and tu/tv are texel pairs.
 */
typedef struct KfTmdF3 {
    u8 r;
    u8 g;
    u8 b;
    KfTmdMode mode;
    u16 n0;
    u16 v0;
    u16 v1;
    u16 v2;
} KfTmdF3;

typedef struct KfTmdG3 {
    u8 r;
    u8 g;
    u8 b;
    KfTmdMode mode;
    u16 n0;
    u16 v0;
    u16 n1;
    u16 v1;
    u16 n2;
    u16 v2;
} KfTmdG3;

typedef struct KfTmdF4 {
    u8 r;
    u8 g;
    u8 b;
    KfTmdMode mode;
    u16 n0;
    u16 v0;
    u16 v1;
    u16 v2;
    u16 v3;
    u16 pad;
} KfTmdF4;

typedef struct KfTmdG4 {
    u8 r;
    u8 g;
    u8 b;
    KfTmdMode mode;
    u16 n0;
    u16 v0;
    u16 n1;
    u16 v1;
    u16 n2;
    u16 v2;
    u16 n3;
    u16 v3;
} KfTmdG4;

typedef struct KfTmdFt3 {
    u8 tu0;
    u8 tv0;
    u16 cba;
    u8 tu1;
    u8 tv1;
    u16 tsb;
    u8 tu2;
    u8 tv2;
    u16 pad;
    u16 n0;
    u16 v0;
    u16 v1;
    u16 v2;
} KfTmdFt3;

typedef struct KfTmdGt3 {
    u8 tu0;
    u8 tv0;
    u16 cba;
    u8 tu1;
    u8 tv1;
    u16 tsb;
    u8 tu2;
    u8 tv2;
    u16 pad;
    u16 n0;
    u16 v0;
    u16 n1;
    u16 v1;
    u16 n2;
    u16 v2;
} KfTmdGt3;

typedef struct KfTmdFt4 {
    u8 tu0;
    u8 tv0;
    u16 cba;
    u8 tu1;
    u8 tv1;
    u16 tsb;
    u8 tu2;
    u8 tv2;
    u16 pad0;
    u8 tu3;
    u8 tv3;
    u16 pad1;
    u16 n0;
    u16 v0;
    u16 v1;
    u16 v2;
    u16 v3;
    u16 pad2;
} KfTmdFt4;

typedef struct KfTmdGt4 {
    u8 tu0;
    u8 tv0;
    u16 cba;
    u8 tu1;
    u8 tv1;
    u16 tsb;
    u8 tu2;
    u8 tv2;
    u16 pad0;
    u8 tu3;
    u8 tv3;
    u16 pad1;
    u16 n0;
    u16 v0;
    u16 n1;
    u16 v1;
    u16 n2;
    u16 v2;
    u16 n3;
    u16 v3;
} KfTmdGt4;

/* Mode-discriminated prepared packet body. Texture modes share the packed
 * UV/CLUT/page prefix; three-vertex modes do not access the fourth UV pair.
 */
typedef union KfTmdPrimitive {
    KfTmdF3 f3;
    KfTmdF4 f4;
    KfTmdFt3 ft3;
    KfTmdFt4 ft4;
    KfTmdG3 g3;
    KfTmdG4 g4;
    KfTmdGt3 gt3;
    KfTmdGt4 gt4;
    CVECTOR color;
    struct {
        u16 uv0;
        u16 clut;
        u16 uv1;
        u16 tpage;
        u16 uv2;
        u16 pad0;
        u16 uv3;
        u16 pad1;
    } texture;
} KfTmdPrimitive;

/* SDK packed result and its two signed screen coordinates share four bytes. */
typedef union KfScreenXY {
    long word;
    DVECTOR vector;
} KfScreenXY;

/* One GTE-projected vertex consumed by the polygon enqueue paths. */
typedef struct KfScreenVertex {
    KfScreenXY sxy;
    s16 sz;
    s16 p2;
} KfScreenVertex;

/* GAME.EXE and OPEN.EXE implement this interface with separate state. */
extern KfTmdObject *tmd_get_object(u16 object_index);
extern void tmd_prepare_primitive_indices(void);
extern void tmd_project_vertices(s32 count);
extern void tmd_register(KfTmdSlot slot, KfTmdHeader *tmd);
extern void tmd_release_last_allocation(KF_ENUM_PARAM(KfTmdSlot, s32) slot);
extern void tmd_select(KfTmdSlot slot);
extern void tmd_select_object_vertices(u16 object_index);
extern void tmd_set_current_vertices(SVECTOR *vertices);

#endif
