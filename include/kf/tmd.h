#ifndef KF_TMD_H
#define KF_TMD_H

/* Shared PlayStation TMD payload, primitive packet, and projection layouts. */

#include <kf/game_types.h>
#include <kf/psyq.h>

/* The game reads the 32-bit object count as two explicit halfwords. */
typedef struct KfTmdHeader {
    u32 id;
    u32 flags;
    u16 object_count;
    u16 object_count_high;
} KfTmdHeader;

/* Standard 0x1c-byte object-table record in an unlinked TMD payload. */
typedef struct KfTmdObject {
    u32 vertex_offset;
    u32 vertex_count;
    u32 normal_offset;
    u32 normal_count;
    u32 primitive_offset;
    u16 primitive_count;
    u16 primitive_count_high;
    s32 scale;
} KfTmdObject;

/*
 * Primitive bodies follow the four-byte olen/ilen/flag/mode header. n0..n3
 * are normal indices, v0..v3 are vertex indices, and tu/tv are texel pairs.
 */
typedef struct KfTmdF3 {
    u8 r;
    u8 g;
    u8 b;
    u8 mode;
    u16 n0;
    u16 v0;
    u16 v1;
    u16 v2;
} KfTmdF3;

typedef struct KfTmdG3 {
    u8 r;
    u8 g;
    u8 b;
    u8 mode;
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
    u8 mode;
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
    u8 mode;
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

/* One GTE-projected vertex consumed by the polygon enqueue paths. */
typedef struct KfScreenVertex {
    DVECTOR sxy;
    s16 sz;
    s16 p2;
} KfScreenVertex;

#endif
