#include <kf/address.h>
#include <kf/game_render.h>

/*
 * Vertex projection primitives that feed the polygon emitters.  Each walks the
 * current TMD vertex array through the GTE and records one eight-byte screen
 * entry per vertex into the shared projection scratch buffer.  The perspective
 * variants read the depth straight from the SZ FIFO with ReadSZ2 so the
 * emitters get the full sixteen-bit Z rather than RotTransPers' truncated
 * return value.
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
