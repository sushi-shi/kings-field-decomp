#include <kf/lib/gpu_packets.h>
#include <kf/open/render.h>
#include <psyq/libc.h>

SVECTOR render_sprite_light_normal = {0, 0, KF_FIXED12_ONE, 0};

void render_enqueue_sprite(
    KfSpriteQuad *sprite, s16 depth_bias, KfSpriteDepthCueMode depth_cue_mode)
{
    SVECTOR corners[4];
    SVECTOR anchor;
    long anchor_sxy;
    long depth_cue;
    long clip_flag;
    long sxy0;
    long sxy1;
    long sxy2;
    long sxy3;
    KfGpuFT4 *prim;
    s32 otz;

    corners[0].vx = corners[2].vx = sprite->x;
    corners[1].vx = corners[3].vx = sprite->x + sprite->w;
    corners[0].vy = corners[1].vy = sprite->y;
    corners[2].vy = corners[3].vy = sprite->y + sprite->h;
    corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;
    anchor.vx = anchor.vy = anchor.vz = 0;
    otz = RotTransPers(&anchor, &anchor_sxy, &depth_cue, &clip_flag);
    RotTransPers4(&corners[0], &corners[1], &corners[2], &corners[3],
                  &sxy0, &sxy1, &sxy2, &sxy3, &depth_cue, &clip_flag);

    prim = (KfGpuFT4 *)primitive_buffer_allocate(sizeof(POLY_FT4));
    SetPolyFT4(&prim->sdk);
    prim->sdk.clut = open_graphics_runtime.floor_item_state.material.clut;
    prim->sdk.tpage = open_graphics_runtime.floor_item_state.material.tpage;

    memcpy((void *)&prim->sdk.x0, (const void *)&sxy0, sizeof sxy0);
    memcpy((void *)&prim->sdk.x1, (const void *)&sxy1, sizeof sxy1);
    memcpy((void *)&prim->sdk.x2, (const void *)&sxy2, sizeof sxy2);
    memcpy((void *)&prim->sdk.x3, (const void *)&sxy3, sizeof sxy3);
    prim->sdk.u0 = prim->sdk.u2 = sprite->u;
    prim->sdk.u1 = prim->sdk.u3 = sprite->u + sprite->u_span;
    prim->sdk.v0 = prim->sdk.v1 = sprite->v;
    prim->sdk.v2 = prim->sdk.v3 = sprite->v + sprite->v_span;
    open_graphics_runtime.floor_item_state.material.color.cd = prim->sdk.code;
    if (depth_cue_mode == KF_SPRITE_DEPTH_CUE_BOOSTED) {
        depth_cue += depth_cue >> 1;
    }
    NormalColorDpq(&render_sprite_light_normal, &open_graphics_runtime.floor_item_state.material.color,
                   depth_cue, &prim->packed.color0);
    if (otz + depth_bias >= KF_SCENE_MIN_OT_DEPTH) {
        AddPrim(
            (void *)(&open_graphics_runtime.ordering_table[(otz + depth_bias) & KF_ORDERING_TABLE_INDEX_MASK]),
            (void *)&prim->sdk);
    }
}
