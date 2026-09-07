# OPEN projected-sprite retained literals

Companion to [the sprite cue and TMD-slot review](sprite-cue-tmd-slots.md).
All 23 remaining numeric occurrences are listed, including repeated indices.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `render_sprite_light_normal` | 5 | `0` | `SVECTOR render_sprite_light_normal = {0, 0, KF_FIXED12_ONE, 0};` | Zero X component of the authored positive-Z unit normal. |
| `render_sprite_light_normal` | 5 | `0` | `SVECTOR render_sprite_light_normal = {0, 0, KF_FIXED12_ONE, 0};` | Zero Y component of the authored positive-Z unit normal. |
| `render_sprite_light_normal` | 5 | `0` | `SVECTOR render_sprite_light_normal = {0, 0, KF_FIXED12_ONE, 0};` | Explicit zero in the authentic SDK SVECTOR padding member. |
| `render_enqueue_sprite` | 11 | `4` | `SVECTOR corners[4];` | One temporary vector for each of the four quad vertices; local geometry extent. |
| `render_enqueue_sprite` | 23 | `0` | `corners[0].vx = corners[2].vx = sprite->x;` | Quad vertex index in SDK packet order; paired assignments construct the left/right or top/bottom rectangle edges. |
| `render_enqueue_sprite` | 23 | `2` | `corners[0].vx = corners[2].vx = sprite->x;` | Quad vertex index in SDK packet order; paired assignments construct the left/right or top/bottom rectangle edges. |
| `render_enqueue_sprite` | 24 | `1` | `corners[1].vx = corners[3].vx = sprite->x + sprite->w;` | Quad vertex index in SDK packet order; paired assignments construct the left/right or top/bottom rectangle edges. |
| `render_enqueue_sprite` | 24 | `3` | `corners[1].vx = corners[3].vx = sprite->x + sprite->w;` | Quad vertex index in SDK packet order; paired assignments construct the left/right or top/bottom rectangle edges. |
| `render_enqueue_sprite` | 25 | `0` | `corners[0].vy = corners[1].vy = sprite->y;` | Quad vertex index in SDK packet order; paired assignments construct the left/right or top/bottom rectangle edges. |
| `render_enqueue_sprite` | 25 | `1` | `corners[0].vy = corners[1].vy = sprite->y;` | Quad vertex index in SDK packet order; paired assignments construct the left/right or top/bottom rectangle edges. |
| `render_enqueue_sprite` | 26 | `2` | `corners[2].vy = corners[3].vy = sprite->y + sprite->h;` | Quad vertex index in SDK packet order; paired assignments construct the left/right or top/bottom rectangle edges. |
| `render_enqueue_sprite` | 26 | `3` | `corners[2].vy = corners[3].vy = sprite->y + sprite->h;` | Quad vertex index in SDK packet order; paired assignments construct the left/right or top/bottom rectangle edges. |
| `render_enqueue_sprite` | 27 | `0` | `corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;` | Quad vertex index in SDK packet order; the source retains the visible four-corner construction. |
| `render_enqueue_sprite` | 27 | `1` | `corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;` | Quad vertex index in SDK packet order; the source retains the visible four-corner construction. |
| `render_enqueue_sprite` | 27 | `2` | `corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;` | Quad vertex index in SDK packet order; the source retains the visible four-corner construction. |
| `render_enqueue_sprite` | 27 | `3` | `corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;` | Quad vertex index in SDK packet order; the source retains the visible four-corner construction. |
| `render_enqueue_sprite` | 27 | `0` | `corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;` | All corners lie in the local XY sprite plane, with zero Z. |
| `render_enqueue_sprite` | 28 | `0` | `anchor.vx = anchor.vy = anchor.vz = 0;` | Local origin supplies the anchor depth; all three coordinates are zero. |
| `render_enqueue_sprite` | 30 | `0` | `RotTransPers4(&corners[0], &corners[1], &corners[2], &corners[3],` | Pass each of the four corners in SDK vertex order so projected coordinate words map to the matching packet vertex. |
| `render_enqueue_sprite` | 30 | `1` | `RotTransPers4(&corners[0], &corners[1], &corners[2], &corners[3],` | Pass each of the four corners in SDK vertex order so projected coordinate words map to the matching packet vertex. |
| `render_enqueue_sprite` | 30 | `2` | `RotTransPers4(&corners[0], &corners[1], &corners[2], &corners[3],` | Pass each of the four corners in SDK vertex order so projected coordinate words map to the matching packet vertex. |
| `render_enqueue_sprite` | 30 | `3` | `RotTransPers4(&corners[0], &corners[1], &corners[2], &corners[3],` | Pass each of the four corners in SDK vertex order so projected coordinate words map to the matching packet vertex. |
| `render_enqueue_sprite` | 48 | `1` | `depth_cue += depth_cue >> 1;` | Arithmetic right shift contributes half the signed GTE cue factor; preserve its rounding instead of rewriting the operation as division or a decimal multiplier. |
