# TMD and projected-sprite retained literals

Companion to [the renderer review](renderer-literal-review.md). All **153**
remaining numeric occurrences in the listed files have individual reasons.
Claims, comments, strings and named constant definitions are excluded.
Repeated tokens have separate rows in source order.

## `src/game/render_enqueuers.c`

94 retained occurrences.

| Function / data owner | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `tmd_textured_primitive_color` | 9 | `0` | `KF_TEXTURE_BASE_BRIGHTNESS, 0` | Initial SDK color command byte; the allocated primitive code replaces it before the lighting call. |
| `render_enqueue_tmd` | 34 | `0` | `while (remaining-- != 0) {` | Zero terminates the packet-count traversal; preserve the post-decrement and unsigned count behavior. |
| `render_enqueue_tmd` | 46 | `0` | `if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 64 | `3` | `(va->p2 + vb->p2 + vc->p2) / 3, (CVECTOR *)&prim->r0);` | Divide the three explicit vertex depth-cue factors by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_tmd` | 65 | `3` | `otz = (va->sz + vb->sz + vc->sz) / 3 >> KF_GTE_DEPTH_TO_OT_SHIFT;` | Divide the three explicit vertex depths by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_tmd` | 66 | `1` | `if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {` | Subtract one from the named inclusive minimum to preserve the existing strict-greater-than comparison. |
| `render_enqueue_tmd` | 80 | `0` | `if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 94 | `2` | `(va->p2 + vb->p2 + vc->p2 + vd->p2) >> 2, (CVECTOR *)&prim->r0);` | Average four explicit vertex terms by shifting two bits; retain signed right-shift rounding of the depth-cue sum. |
| `render_enqueue_tmd` | 95 | `2` | `otz = (va->sz + vb->sz + vc->sz + vd->sz) >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2);` | Average four explicit vertex terms by shifting two bits; combine with the separately named depth-to-OT shift in the original single shift. |
| `render_enqueue_tmd` | 96 | `1` | `if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {` | Subtract one from the named inclusive minimum to preserve the existing strict-greater-than comparison. |
| `render_enqueue_tmd` | 110 | `0` | `if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 124 | `3` | `otz = (va->sz + vb->sz + vc->sz) / 3 >> KF_GTE_DEPTH_TO_OT_SHIFT;` | Divide the three explicit vertex depths by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_tmd` | 125 | `1` | `if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {` | Subtract one from the named inclusive minimum to preserve the existing strict-greater-than comparison. |
| `render_enqueue_tmd` | 139 | `0` | `if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 157 | `2` | `otz = (va->sz + vb->sz + vc->sz + vd->sz) >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2);` | Average four explicit vertex terms by shifting two bits; combine with the separately named depth-to-OT shift in the original single shift. |
| `render_enqueue_tmd` | 158 | `1` | `if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {` | Subtract one from the named inclusive minimum to preserve the existing strict-greater-than comparison. |
| `render_enqueue_tmd` | 172 | `0` | `if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 192 | `3` | `otz = (va->sz + vb->sz + vc->sz) / 3 >> KF_GTE_DEPTH_TO_OT_SHIFT;` | Divide the three explicit vertex depths by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_tmd` | 193 | `1` | `if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {` | Subtract one from the named inclusive minimum to preserve the existing strict-greater-than comparison. |
| `render_enqueue_tmd` | 207 | `0` | `if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 232 | `2` | `otz = (va->sz + vb->sz + vc->sz + vd->sz) >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2);` | Average four explicit vertex terms by shifting two bits; combine with the separately named depth-to-OT shift in the original single shift. |
| `render_enqueue_tmd` | 233 | `1` | `if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {` | Subtract one from the named inclusive minimum to preserve the existing strict-greater-than comparison. |
| `render_enqueue_tmd` | 247 | `0` | `if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 255 | `1` | `SetSemiTrans(prim, 1);` | Boolean true enables the authentic SDK primitive semi-transparency flag; it is separate from the TMD mode bit. |
| `render_enqueue_tmd` | 262 | `3` | `otz = (va->sz + vb->sz + vc->sz) / 3 >> KF_GTE_DEPTH_TO_OT_SHIFT;` | Divide the three explicit vertex depths by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_tmd` | 263 | `1` | `if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {` | Subtract one from the named inclusive minimum to preserve the existing strict-greater-than comparison. |
| `render_enqueue_tmd` | 277 | `0` | `if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 298 | `2` | `(va->p2 + vb->p2 + vc->p2 + vd->p2) >> 2, (CVECTOR *)&prim->r0);` | Average four explicit vertex terms by shifting two bits; retain signed right-shift rounding of the depth-cue sum. |
| `render_enqueue_tmd` | 299 | `2` | `otz = (va->sz + vb->sz + vc->sz + vd->sz) >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2);` | Average four explicit vertex terms by shifting two bits; combine with the separately named depth-to-OT shift in the original single shift. |
| `render_enqueue_tmd` | 300 | `1` | `if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {` | Subtract one from the named inclusive minimum to preserve the existing strict-greater-than comparison. |
| `render_enqueue_tmd` | 314 | `0` | `if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 326 | `3` | `(va->p2 + vb->p2 + vc->p2) / 3, (CVECTOR *)&prim->r0);` | Divide the three explicit vertex depth-cue factors by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_tmd` | 327 | `3` | `otz = (va->sz + vb->sz + vc->sz) / 3 >> KF_GTE_DEPTH_TO_OT_SHIFT;` | Divide the three explicit vertex depths by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_tmd` | 328 | `1` | `if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {` | Subtract one from the named inclusive minimum to preserve the existing strict-greater-than comparison. |
| `render_enqueue_tmd` | 342 | `0` | `if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 351 | `1` | `SetSemiTrans(prim, 1);` | Boolean true enables the authentic SDK primitive semi-transparency flag; it is separate from the TMD mode bit. |
| `render_enqueue_tmd` | 360 | `2` | `otz = (va->sz + vb->sz + vc->sz + vd->sz) >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2);` | Average four explicit vertex terms by shifting two bits; combine with the separately named depth-to-OT shift in the original single shift. |
| `render_enqueue_tmd` | 361 | `1` | `if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {` | Subtract one from the named inclusive minimum to preserve the existing strict-greater-than comparison. |
| `render_enqueue_tmd` | 375 | `0` | `if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 383 | `1` | `SetSemiTrans(prim, 1);` | Boolean true enables the authentic SDK primitive semi-transparency flag; it is separate from the TMD mode bit. |
| `render_enqueue_tmd` | 388 | `3` | `(va->p2 + vb->p2 + vc->p2) / 3, (CVECTOR *)&prim->r0);` | Divide the three explicit vertex depth-cue factors by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_tmd` | 389 | `3` | `otz = (va->sz + vb->sz + vc->sz) / 3 >> KF_GTE_DEPTH_TO_OT_SHIFT;` | Divide the three explicit vertex depths by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_tmd` | 390 | `1` | `if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {` | Subtract one from the named inclusive minimum to preserve the existing strict-greater-than comparison. |
| `render_enqueue_tmd` | 404 | `0` | `if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 413 | `1` | `SetSemiTrans(prim, 1);` | Boolean true enables the authentic SDK primitive semi-transparency flag; it is separate from the TMD mode bit. |
| `render_enqueue_tmd` | 419 | `2` | `(va->p2 + vb->p2 + vc->p2 + vd->p2) >> 2, (CVECTOR *)&prim->r0);` | Average four explicit vertex terms by shifting two bits; retain signed right-shift rounding of the depth-cue sum. |
| `render_enqueue_tmd` | 420 | `2` | `otz = (va->sz + vb->sz + vc->sz + vd->sz) >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2);` | Average four explicit vertex terms by shifting two bits; combine with the separately named depth-to-OT shift in the original single shift. |
| `render_enqueue_tmd` | 421 | `1` | `if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {` | Subtract one from the named inclusive minimum to preserve the existing strict-greater-than comparison. |
| `model_textured_primitive_color` | 439 | `0` | `KF_TEXTURE_BASE_BRIGHTNESS, 0` | Initial SDK color command byte; the allocated primitive code replaces it before the lighting call. |
| `render_enqueue_model` | 464 | `0` | `while (remaining-- != 0) {` | Zero terminates the packet-count traversal; preserve the post-decrement and unsigned count behavior. |
| `render_enqueue_model` | 477 | `0` | `if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_model` | 497 | `3` | `otz = (va->sz + vb->sz + vc->sz) / 3 >> KF_GTE_DEPTH_TO_OT_SHIFT;` | Divide the three explicit vertex depths by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_model` | 498 | `1` | `if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {` | Subtract one from the named inclusive minimum to preserve the existing strict-greater-than comparison. |
| `render_enqueue_model` | 512 | `0` | `if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_model` | 537 | `2` | `otz = (va->sz + vb->sz + vc->sz + vd->sz) >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2);` | Average four explicit vertex terms by shifting two bits; combine with the separately named depth-to-OT shift in the original single shift. |
| `render_enqueue_model` | 538 | `1` | `if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {` | Subtract one from the named inclusive minimum to preserve the existing strict-greater-than comparison. |
| `render_enqueue_model` | 552 | `0` | `if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_model` | 570 | `3` | `(va->p2 + vb->p2 + vc->p2) / 3, (CVECTOR *)&prim->r0);` | Divide the three explicit vertex depth-cue factors by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_model` | 571 | `3` | `otz = (va->sz + vb->sz + vc->sz) / 3 >> KF_GTE_DEPTH_TO_OT_SHIFT;` | Divide the three explicit vertex depths by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_model` | 572 | `1` | `if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {` | Subtract one from the named inclusive minimum to preserve the existing strict-greater-than comparison. |
| `render_enqueue_model` | 586 | `0` | `if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_model` | 607 | `2` | `(va->p2 + vb->p2 + vc->p2 + vd->p2) >> 2, (CVECTOR *)&prim->r0);` | Average four explicit vertex terms by shifting two bits; retain signed right-shift rounding of the depth-cue sum. |
| `render_enqueue_model` | 608 | `2` | `otz = (va->sz + vb->sz + vc->sz + vd->sz) >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2);` | Average four explicit vertex terms by shifting two bits; combine with the separately named depth-to-OT shift in the original single shift. |
| `render_enqueue_model` | 609 | `1` | `if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {` | Subtract one from the named inclusive minimum to preserve the existing strict-greater-than comparison. |
| `map_textured_primitive_color` | 625 | `0` | `KF_TEXTURE_BASE_BRIGHTNESS, 0` | Initial SDK color command byte; the allocated primitive code replaces it before the lighting call. |
| `render_sprite_light_normal` | 629 | `0` | `SVECTOR render_sprite_light_normal = {0, 0, KF_FIXED12_ONE, 0};` | Zero X component of the positive-Z Q12 light normal. |
| `render_sprite_light_normal` | 629 | `0` | `SVECTOR render_sprite_light_normal = {0, 0, KF_FIXED12_ONE, 0};` | Zero Y component of the positive-Z Q12 light normal. |
| `render_sprite_light_normal` | 629 | `0` | `SVECTOR render_sprite_light_normal = {0, 0, KF_FIXED12_ONE, 0};` | Authored zero in the SDK vector padding halfword; it is not a normal component. |
| `render_enqueue_map` | 664 | `0` | `while (remaining-- != 0) {` | Zero terminates the packet-count traversal; preserve the post-decrement and unsigned count behavior. |
| `render_enqueue_map` | 677 | `0` | `if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_map` | 702 | `2` | `>> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + KF_MAP_OT_DEPTH_BIAS;` | Average four explicit vertex terms by shifting two bits; combine with the separately named depth-to-OT shift in the original single shift. |
| `render_enqueue_map` | 721 | `0` | `if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_map` | 742 | `3` | `otz = ((va->sz + vb->sz + vc->sz) / 3` | Divide the three explicit vertex depths by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_sprite` | 771 | `4` | `SVECTOR corners[4];` | Four vertices of the explicit rectangular sprite; this local extent follows the four-argument projection API. |
| `render_enqueue_sprite` | 783 | `0` | `corners[0].vx = corners[2].vx = sprite->x;` | The upper-left corner index in the explicit four-vertex rectangle. |
| `render_enqueue_sprite` | 783 | `2` | `corners[0].vx = corners[2].vx = sprite->x;` | The lower-left corner index in the explicit four-vertex rectangle. |
| `render_enqueue_sprite` | 784 | `1` | `corners[1].vx = corners[3].vx = sprite->x + sprite->w;` | The upper-right corner index in the explicit four-vertex rectangle. |
| `render_enqueue_sprite` | 784 | `3` | `corners[1].vx = corners[3].vx = sprite->x + sprite->w;` | The lower-right corner index in the explicit four-vertex rectangle. |
| `render_enqueue_sprite` | 785 | `0` | `corners[0].vy = corners[1].vy = sprite->y;` | The upper-left corner index in the explicit four-vertex rectangle. |
| `render_enqueue_sprite` | 785 | `1` | `corners[0].vy = corners[1].vy = sprite->y;` | The upper-right corner index in the explicit four-vertex rectangle. |
| `render_enqueue_sprite` | 786 | `2` | `corners[2].vy = corners[3].vy = sprite->y + sprite->h;` | The lower-left corner index in the explicit four-vertex rectangle. |
| `render_enqueue_sprite` | 786 | `3` | `corners[2].vy = corners[3].vy = sprite->y + sprite->h;` | The lower-right corner index in the explicit four-vertex rectangle. |
| `render_enqueue_sprite` | 787 | `0` | `corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;` | The upper-left corner index in the explicit four-vertex rectangle. |
| `render_enqueue_sprite` | 787 | `1` | `corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;` | The upper-right corner index in the explicit four-vertex rectangle. |
| `render_enqueue_sprite` | 787 | `2` | `corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;` | The lower-left corner index in the explicit four-vertex rectangle. |
| `render_enqueue_sprite` | 787 | `3` | `corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;` | The lower-right corner index in the explicit four-vertex rectangle. |
| `render_enqueue_sprite` | 787 | `0` | `corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;` | The sprite lies in its local XY plane, so every corner has zero local Z. |
| `render_enqueue_sprite` | 788 | `0` | `anchor.vx = anchor.vy = anchor.vz = 0;` | Local origin anchors the separate perspective/depth projection for sorting the sprite. |
| `render_enqueue_sprite` | 790 | `0` | `RotTransPers4(&corners[0], &corners[1], &corners[2], &corners[3],` | The upper-left corner is passed in the SDK four-vertex projection order. |
| `render_enqueue_sprite` | 790 | `1` | `RotTransPers4(&corners[0], &corners[1], &corners[2], &corners[3],` | The upper-right corner is passed in the SDK four-vertex projection order. |
| `render_enqueue_sprite` | 790 | `2` | `RotTransPers4(&corners[0], &corners[1], &corners[2], &corners[3],` | The lower-left corner is passed in the SDK four-vertex projection order. |
| `render_enqueue_sprite` | 790 | `3` | `RotTransPers4(&corners[0], &corners[1], &corners[2], &corners[3],` | The lower-right corner is passed in the SDK four-vertex projection order. |
| `render_enqueue_sprite` | 811 | `1` | `depth_cue += depth_cue >> 1;` | One-bit signed shift supplies half the depth-cue factor for the named boosted mode, preserving its rounding. |
| `render_enqueue_sprite` | 814 | `2` | `NormalColorDpq(&render_sprite_light_normal, (CVECTOR *)(&active_render_clut + 2), depth_cue,` | Two halfwords reach the measured four-byte-offset color span; enclosing GAME graphics ownership remains unresolved (render-material.md). |

## `src/open/render_tmd.c`

48 retained occurrences.

| Function / data owner | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `tmd_textured_primitive_color` | 8 | `0` | `KF_TEXTURE_BASE_BRIGHTNESS, 0` | Initial SDK color command byte; the allocated primitive code replaces it before the lighting call. |
| `render_enqueue_tmd` | 30 | `0` | `while (remaining-- != 0) {` | Zero terminates the packet-count traversal; preserve the post-decrement and unsigned count behavior. |
| `render_enqueue_tmd` | 49 | `0` | `if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 64 | `3` | `(vertex0->p2 + vertex1->p2 + vertex2->p2) / 3,` | Divide the three explicit vertex depth-cue factors by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_tmd` | 66 | `3` | `depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)` | Divide the three explicit vertex depths by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_tmd` | 70 | `0` | `vertices - (unsigned long)&((KfGraphicsRuntimeOpen *)0)->` | Null base is used only to form the projected-array member offset for recovery of its complete graphics owner. |
| `render_enqueue_tmd` | 85 | `0` | `if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 96 | `2` | `(vertex0->p2 + vertex1->p2 + vertex2->p2 + vertex3->p2) >> 2,` | Average four explicit vertex terms by shifting two bits; retain signed right-shift rounding of the depth-cue sum. |
| `render_enqueue_tmd` | 99 | `2` | `>> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + depth_bias;` | Average four explicit vertex terms by shifting two bits; combine with the separately named depth-to-OT shift in the original single shift. |
| `render_enqueue_tmd` | 102 | `0` | `vertices - (unsigned long)&((KfGraphicsRuntimeOpen *)0)->` | Null base is used only to form the projected-array member offset for recovery of its complete graphics owner. |
| `render_enqueue_tmd` | 117 | `0` | `if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 130 | `3` | `depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)` | Divide the three explicit vertex depths by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_tmd` | 134 | `0` | `vertices - (unsigned long)&((KfGraphicsRuntimeOpen *)0)->` | Null base is used only to form the projected-array member offset for recovery of its complete graphics owner. |
| `render_enqueue_tmd` | 149 | `0` | `if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 167 | `2` | `>> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + depth_bias;` | Average four explicit vertex terms by shifting two bits; combine with the separately named depth-to-OT shift in the original single shift. |
| `render_enqueue_tmd` | 170 | `0` | `vertices - (unsigned long)&((KfGraphicsRuntimeOpen *)0)->` | Null base is used only to form the projected-array member offset for recovery of its complete graphics owner. |
| `render_enqueue_tmd` | 185 | `0` | `if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 204 | `3` | `depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)` | Divide the three explicit vertex depths by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_tmd` | 208 | `0` | `vertices - (unsigned long)&((KfGraphicsRuntimeOpen *)0)->` | Null base is used only to form the projected-array member offset for recovery of its complete graphics owner. |
| `render_enqueue_tmd` | 223 | `0` | `if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 248 | `2` | `>> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + depth_bias;` | Average four explicit vertex terms by shifting two bits; combine with the separately named depth-to-OT shift in the original single shift. |
| `render_enqueue_tmd` | 251 | `0` | `vertices - (unsigned long)&((KfGraphicsRuntimeOpen *)0)->` | Null base is used only to form the projected-array member offset for recovery of its complete graphics owner. |
| `render_enqueue_tmd` | 266 | `0` | `if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 271 | `1` | `SetSemiTrans(&prim->sdk, 1);` | Boolean true enables the authentic SDK primitive semi-transparency flag; it is separate from the TMD mode bit. |
| `render_enqueue_tmd` | 280 | `3` | `depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)` | Divide the three explicit vertex depths by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_tmd` | 284 | `0` | `vertices - (unsigned long)&((KfGraphicsRuntimeOpen *)0)->` | Null base is used only to form the projected-array member offset for recovery of its complete graphics owner. |
| `render_enqueue_tmd` | 299 | `0` | `if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 317 | `2` | `(vertex0->p2 + vertex1->p2 + vertex2->p2 + vertex3->p2) >> 2,` | Average four explicit vertex terms by shifting two bits; retain signed right-shift rounding of the depth-cue sum. |
| `render_enqueue_tmd` | 320 | `2` | `>> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + depth_bias;` | Average four explicit vertex terms by shifting two bits; combine with the separately named depth-to-OT shift in the original single shift. |
| `render_enqueue_tmd` | 323 | `0` | `vertices - (unsigned long)&((KfGraphicsRuntimeOpen *)0)->` | Null base is used only to form the projected-array member offset for recovery of its complete graphics owner. |
| `render_enqueue_tmd` | 338 | `0` | `if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 347 | `3` | `(vertex0->p2 + vertex1->p2 + vertex2->p2) / 3,` | Divide the three explicit vertex depth-cue factors by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_tmd` | 349 | `3` | `depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)` | Divide the three explicit vertex depths by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_tmd` | 353 | `0` | `vertices - (unsigned long)&((KfGraphicsRuntimeOpen *)0)->` | Null base is used only to form the projected-array member offset for recovery of its complete graphics owner. |
| `render_enqueue_tmd` | 368 | `0` | `if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 374 | `1` | `SetSemiTrans(&prim->sdk, 1);` | Boolean true enables the authentic SDK primitive semi-transparency flag; it is separate from the TMD mode bit. |
| `render_enqueue_tmd` | 388 | `2` | `>> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + depth_bias;` | Average four explicit vertex terms by shifting two bits; combine with the separately named depth-to-OT shift in the original single shift. |
| `render_enqueue_tmd` | 391 | `0` | `vertices - (unsigned long)&((KfGraphicsRuntimeOpen *)0)->` | Null base is used only to form the projected-array member offset for recovery of its complete graphics owner. |
| `render_enqueue_tmd` | 406 | `0` | `if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 411 | `1` | `SetSemiTrans(&prim->sdk, 1);` | Boolean true enables the authentic SDK primitive semi-transparency flag; it is separate from the TMD mode bit. |
| `render_enqueue_tmd` | 416 | `3` | `(vertex0->p2 + vertex1->p2 + vertex2->p2) / 3,` | Divide the three explicit vertex depth-cue factors by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_tmd` | 418 | `3` | `depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)` | Divide the three explicit vertex depths by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_tmd` | 422 | `0` | `vertices - (unsigned long)&((KfGraphicsRuntimeOpen *)0)->` | Null base is used only to form the projected-array member offset for recovery of its complete graphics owner. |
| `render_enqueue_tmd` | 437 | `0` | `if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_tmd` | 443 | `1` | `SetSemiTrans(&prim->sdk, 1);` | Boolean true enables the authentic SDK primitive semi-transparency flag; it is separate from the TMD mode bit. |
| `render_enqueue_tmd` | 449 | `2` | `(vertex0->p2 + vertex1->p2 + vertex2->p2 + vertex3->p2) >> 2,` | Average four explicit vertex terms by shifting two bits; retain signed right-shift rounding of the depth-cue sum. |
| `render_enqueue_tmd` | 452 | `2` | `>> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + depth_bias;` | Average four explicit vertex terms by shifting two bits; combine with the separately named depth-to-OT shift in the original single shift. |
| `render_enqueue_tmd` | 455 | `0` | `vertices - (unsigned long)&((KfGraphicsRuntimeOpen *)0)->` | Null base is used only to form the projected-array member offset for recovery of its complete graphics owner. |

## `src/open/render_map.c`

6 retained occurrences.

| Function / data owner | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `map_textured_primitive_color` | 14 | `0` | `KF_TEXTURE_BASE_BRIGHTNESS, 0` | Initial SDK color command byte; the allocated primitive code replaces it before the lighting call. |
| `render_enqueue_map` | 36 | `0` | `while (remaining-- != 0) {` | Zero terminates the packet-count traversal; preserve the post-decrement and unsigned count behavior. |
| `render_enqueue_map` | 50 | `0` | `if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_map` | 74 | `2` | `>> (KF_GTE_DEPTH_TO_OT_SHIFT + 2))` | Average four explicit vertex terms by shifting two bits; combine with the separately named depth-to-OT shift in the original single shift. |
| `render_enqueue_map` | 90 | `0` | `if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_map` | 109 | `3` | `depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)` | Divide the three explicit vertex depths by three for their arithmetic mean; this is not a mesh-capacity selector. |

## `src/open/render_unlit.c`

5 retained occurrences.

| Function / data owner | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `render_enqueue_unlit_triangles` | 19 | `0` | `while (remaining-- != 0) {` | Zero terminates the packet-count traversal; preserve the post-decrement and unsigned count behavior. |
| `render_enqueue_unlit_triangles` | 40 | `0` | `vertex2->sxy) <= 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_unlit_triangles` | 67 | `0` | `vertex2->sxy) <= 0) {` | Signed projected-area zero separates accepted positive winding from rejected winding or a degenerate triangle. |
| `render_enqueue_unlit_triangles` | 84 | `3` | `depth = ((vertex0->sz + vertex1->sz + vertex2->sz) / 3) >> KF_GTE_DEPTH_TO_OT_SHIFT;` | Divide the three explicit vertex depths by three for their arithmetic mean; this is not a mesh-capacity selector. |
| `render_enqueue_unlit_triangles` | 89 | `0` | `vertices - (unsigned long)&((KfGraphicsRuntimeOpen *)0)->` | Null base is used only to form the projected-array member offset for recovery of its complete graphics owner. |
