#ifndef KF_TMD_H
#define KF_TMD_H

/* Shared TMD payload, primitive packet, projection layouts, and game APIs. */

#include <kf/game_types.h>
#include <kf/psyq.h>

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

/* GAME.EXE and OPEN.EXE implement this interface with separate state. */
extern KfTmdObject *tmd_get_object(u16 object_index);
extern void tmd_prepare_primitive_indices(void);
extern void tmd_project_vertices(s32 count);
extern void tmd_register(u16 slot, u8 *tmd);
extern void tmd_release_last_allocation(s32 slot);
extern void tmd_select(u16 slot);
extern void tmd_select_object_vertices(u16 object_index);
extern void tmd_set_current_vertices(SVECTOR *vertices);

#endif
