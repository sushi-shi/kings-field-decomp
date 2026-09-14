#include <kf/lib/null.h>
#include <kf/lib/address.h>
#include <kf/lib/map_data.h>
#include <kf/lib/memory.h>
#include <kf/lib/debug.h>
#include <kf/open/render.h>
#include <kf/lib/tmd.h>
#include <kf/lib/graphics.h>

/*
 * OPEN.EXE display, TMD, and projection bodies following render initialization.
 * Shared instruction shapes with the GAME renderer support common source
 * lineage; OPEN-specific state layouts and call paths remain explicit.
 */

#include "../lib/display_frame.inc"

RODATA(0x80012138, 0x87)

#include "../lib/tmd.inc"

ADDRESS(0x8001738c, 0xcc)
void tmd_project_vertices(s32 count)
{
    KfScreenVertex *out;
    SVECTOR *vertex;
    long perspective;
    long flag;
    long depth;
    long unused_depth;

    if (count >= KF_PROJECTED_VERTEX_CAPACITY) {
        debug_printf_sink("POINT OVER !!!!!!\n");
        return;
    }
    out = open_graphics_runtime.tmd_projected_vertices;
    vertex = open_graphics_runtime.current_tmd_vertices;
    for (count--; count != -1; count--) {
        RotTransPers(vertex, &out->sxy.word, &perspective, &flag);
        out->p2 = perspective << open_graphics_runtime.tmd_projection_shift;
        ReadSZ2(&depth, &unused_depth);
        out->sz = depth;
        out++;
        vertex++;
    }
}

#include "../lib/tmd_perspective_right.inc"

#include "../lib/tmd_transform.inc"
