#include <kf/address.h>
#include <kf/semantic_types.h>

/*
 * Vertex projection primitives that feed the polygon emitters.  Each walks the
 * current TMD vertex array through the GTE and records one eight-byte screen
 * entry per vertex into the shared projection scratch buffer.  The perspective
 * variants read the depth straight from the SZ FIFO with ReadSZ2 so the
 * emitters get the full sixteen-bit Z rather than RotTransPers' truncated
 * return value.
 *
 * WIP: the scratch buffer at DAT_800911b0 is modelled locally as KfScreenVertex
 * until its polygon-emitter consumers are reconstructed and the type can move
 * into the shared header with a structures.tsv entry.
 *
 * Codegen residue: tmd_project_vertices and tmd_project_vertices_shift are
 * structurally exact but swap two callee-saved registers versus retail -- the
 * loop counter and the &out->sz induction pointer trade $s0/$s1.  The pointer
 * giv carries two stores plus its own bump, so the gcc-2.5.7 probe ranks it
 * above the counter, whereas retail gives the counter the lower register.  The
 * borderline two-store giv is the only difference; tmd_transform_vertices,
 * whose giv carries three stores and clearly outranks the counter, matches
 * exactly.  See docs/patterns/source-shapes-gcc257.md.
 */

extern SVECTOR *current_tmd_vertices;

/* Projection scratch buffer: one entry per transformed TMD vertex. */
typedef struct KfScreenVertex {
    DVECTOR sxy; /* +0: packed screen X/Y from RotTransPers/RotTrans */
    s16 sz;      /* +4: depth (SZ, or view-space Z) */
    s16 p2;      /* +6: doubled perspective term (or a second copy of Z) */
} KfScreenVertex;

extern KfScreenVertex DAT_800911b0[];

/* Writes GTE data register 19 (SZ3) to *sz0; the second slot follows the call
 * convention but is ignored by the linked library body. */
extern void ReadSZ2(long *sz0, long *sz1);

ADDRESS(0x8001c60c, 0x9c)
void tmd_project_vertices(s32 count)
{
    KfScreenVertex *out;
    SVECTOR *vtx;
    long p;
    long flag;
    long sz0;
    long sz1;
    s32 i;

    out = DAT_800911b0;
    vtx = current_tmd_vertices;
    for (i = count - 1; i != -1; i--) {
        RotTransPers(vtx, (long *)&out->sxy, &p, &flag);
        out->p2 = (u16)p << 1;
        ReadSZ2(&sz0, &sz1);
        out->sz = (u16)sz0;
        out++;
        vtx++;
    }
}

ADDRESS(0x8001c6a8, 0xac)
void tmd_project_vertices_shift(s32 count, u8 shift)
{
    KfScreenVertex *out;
    SVECTOR *vtx;
    long p;
    long flag;
    long sz0;
    long sz1;
    s32 i;

    out = DAT_800911b0;
    vtx = current_tmd_vertices;
    for (i = count - 1; i != -1; i--) {
        RotTransPers(vtx, (long *)&out->sxy, &p, &flag);
        out->p2 = (u16)p << 1;
        ReadSZ2(&sz0, &sz1);
        out->sz = sz0 >> shift;
        out++;
        vtx++;
    }
}

ADDRESS(0x8001c754, 0xa4)
void tmd_transform_vertices(s32 count)
{
    KfScreenVertex *out;
    SVECTOR *vtx;
    VECTOR pos;
    long flag;
    s32 i;

    out = DAT_800911b0;
    vtx = current_tmd_vertices;
    for (i = count - 1; i != -1; i--) {
        RotTrans(vtx, &pos, &flag);
        out->sxy.vx = pos.vx;
        out->sxy.vy = pos.vy;
        out->p2 = pos.vz;
        out->sz = pos.vz;
        out++;
        vtx++;
    }
}
