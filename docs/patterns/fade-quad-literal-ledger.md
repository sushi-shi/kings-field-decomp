# Fade and simple-quad retained literals

Companion to [the quad color review](quad-color-types.md). All 97 remaining
numeric occurrences in these four modules are listed, including duplicate
indices and initialized data. Claims and named definitions are excluded.

## `src/game/display_play_transition.c`

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 8 | `4` | `u16 v[4];` | Four unsigned-halfword rectangle entries encode X, Y, width and height. |
| `initializers` | 11 | `8` | `u8 v[8];` | Eight-byte texture descriptor with U, V and spans in even bytes; preserve the unused odd bytes. |
| `initializers` | 18 | `0` | `KF_TRANSITION_RECT_X, 0, KF_TRANSITION_RECT_WIDTH, KF_TRANSITION_RECT_HEIGHT` | Authored top-of-screen rectangle Y origin. |
| `initializers` | 22 | `0` | `0, 0, 0, 0, KF_TRANSITION_RECT_WIDTH, 0, KF_TRANSITION_RECT_HEIGHT, 0` | Texture U origin is zero. |
| `initializers` | 22 | `0` | `0, 0, 0, 0, KF_TRANSITION_RECT_WIDTH, 0, KF_TRANSITION_RECT_HEIGHT, 0` | Unused descriptor byte one retains its authored zero. |
| `initializers` | 22 | `0` | `0, 0, 0, 0, KF_TRANSITION_RECT_WIDTH, 0, KF_TRANSITION_RECT_HEIGHT, 0` | Texture V origin is zero. |
| `initializers` | 22 | `0` | `0, 0, 0, 0, KF_TRANSITION_RECT_WIDTH, 0, KF_TRANSITION_RECT_HEIGHT, 0` | Unused descriptor byte three retains its authored zero. |
| `initializers` | 22 | `0` | `0, 0, 0, 0, KF_TRANSITION_RECT_WIDTH, 0, KF_TRANSITION_RECT_HEIGHT, 0` | Unused descriptor byte five retains its authored zero. |
| `initializers` | 22 | `0` | `0, 0, 0, 0, KF_TRANSITION_RECT_WIDTH, 0, KF_TRANSITION_RECT_HEIGHT, 0` | Unused descriptor byte seven retains its authored zero. |
| `initializers` | 25 | `0` | `CVECTOR fade_screen_color = {0, 0, 0, 0};` | Initial red modulation is black. |
| `initializers` | 25 | `0` | `CVECTOR fade_screen_color = {0, 0, 0, 0};` | Initial green modulation is black. |
| `initializers` | 25 | `0` | `CVECTOR fade_screen_color = {0, 0, 0, 0};` | Initial blue modulation is black. |
| `initializers` | 25 | `0` | `CVECTOR fade_screen_color = {0, 0, 0, 0};` | Authored zero in the SDK command byte; these helpers only read RGB. |
| `initializers` | 27 | `7` | `char fade_screen_path[7] = "B0\\L0.";` | Authored path storage includes its terminating NUL; retain the explicit seven-byte data extent. |
| `display_play_transition` | 41 | `0` | `DrawSync(0);` | Authentic SDK blocking synchronization mode; wait for queued GPU work. |
| `display_play_transition` | 42 | `0` | `PutDrawEnv(&display_draw_environments[0]);` | Index of one of the two display-environment records; preserve their explicit submission order. |
| `display_play_transition` | 43 | `1` | `PutDrawEnv(&display_draw_environments[1]);` | Index of one of the two display-environment records; preserve their explicit submission order. |
| `display_play_transition` | 44 | `0` | `DrawSync(0);` | Authentic SDK blocking synchronization mode; wait for queued GPU work. |
| `display_play_transition` | 54 | `0` | `clut = GetClut(0, KF_TRANSITION_CLUT_Y);` | Palette starts at VRAM X zero; the Y row has a separate named contract. |
| `display_play_transition` | 56 | `0` | `for (i = 0; i < KF_TRANSITION_FADE_FRAMES; i++) {` | Start the frame count at zero; the named frame bound determines the fade duration. |
| `display_play_transition` | 68 | `0` | `DrawSync(0);` | Authentic SDK blocking synchronization mode; wait for queued GPU work. |

## `src/open/opening_fade.c`

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 5 | `4` | `u16 opening_fade_rect[4] = {` | Four unsigned-halfword rectangle entries encode X, Y, width and height. |
| `initializers` | 6 | `0` | `KF_TRANSITION_RECT_X, 0, KF_TRANSITION_RECT_WIDTH, KF_TRANSITION_RECT_HEIGHT` | Authored top-of-screen rectangle Y origin. |
| `initializers` | 9 | `8` | `u8 opening_fade_uv[8] = {` | Eight-byte texture descriptor with U, V and spans in even bytes; preserve the unused odd bytes. |
| `initializers` | 10 | `0` | `0, 0, 0, 0, KF_TRANSITION_RECT_WIDTH, 0, KF_TRANSITION_RECT_HEIGHT, 0` | Texture U origin is zero. |
| `initializers` | 10 | `0` | `0, 0, 0, 0, KF_TRANSITION_RECT_WIDTH, 0, KF_TRANSITION_RECT_HEIGHT, 0` | Unused descriptor byte one retains its authored zero. |
| `initializers` | 10 | `0` | `0, 0, 0, 0, KF_TRANSITION_RECT_WIDTH, 0, KF_TRANSITION_RECT_HEIGHT, 0` | Texture V origin is zero. |
| `initializers` | 10 | `0` | `0, 0, 0, 0, KF_TRANSITION_RECT_WIDTH, 0, KF_TRANSITION_RECT_HEIGHT, 0` | Unused descriptor byte three retains its authored zero. |
| `initializers` | 10 | `0` | `0, 0, 0, 0, KF_TRANSITION_RECT_WIDTH, 0, KF_TRANSITION_RECT_HEIGHT, 0` | Unused descriptor byte five retains its authored zero. |
| `initializers` | 10 | `0` | `0, 0, 0, 0, KF_TRANSITION_RECT_WIDTH, 0, KF_TRANSITION_RECT_HEIGHT, 0` | Unused descriptor byte seven retains its authored zero. |
| `initializers` | 13 | `0` | `CVECTOR opening_fade_color = {0, 0, 0, 0};` | Initial red modulation is black. |
| `initializers` | 13 | `0` | `CVECTOR opening_fade_color = {0, 0, 0, 0};` | Initial green modulation is black. |
| `initializers` | 13 | `0` | `CVECTOR opening_fade_color = {0, 0, 0, 0};` | Initial blue modulation is black. |
| `initializers` | 13 | `0` | `CVECTOR opening_fade_color = {0, 0, 0, 0};` | Authored zero in the SDK command byte; these helpers only read RGB. |
| `opening_fade_in` | 26 | `0` | `clut = GetClut(0, KF_TRANSITION_CLUT_Y);` | Palette starts at VRAM X zero; the Y row has a separate named contract. |
| `opening_fade_in` | 27 | `0` | `frame = 0;` | Start the frame count at zero; the named frame bound determines the fade duration. |
| `opening_fade_in` | 42 | `0` | `DrawSync(0);` | Authentic SDK blocking synchronization mode; wait for queued GPU work. |

## `src/game/sprite_add_ft4.c`

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `sprite_add_ft4` | 14 | `0` | `prim->x0 = position[0];` | Rectangle X origin slot; both left vertices use it and right vertices add the width. |
| `sprite_add_ft4` | 15 | `1` | `prim->y0 = position[1];` | Rectangle Y origin slot; both top vertices use it and bottom vertices add the height. |
| `sprite_add_ft4` | 16 | `0` | `prim->x1 = position[0] + position[2];` | Rectangle X origin slot; both left vertices use it and right vertices add the width. |
| `sprite_add_ft4` | 16 | `2` | `prim->x1 = position[0] + position[2];` | Rectangle width slot, added to X to form the right edge. |
| `sprite_add_ft4` | 17 | `1` | `prim->y1 = position[1];` | Rectangle Y origin slot; both top vertices use it and bottom vertices add the height. |
| `sprite_add_ft4` | 18 | `0` | `prim->x2 = position[0];` | Rectangle X origin slot; both left vertices use it and right vertices add the width. |
| `sprite_add_ft4` | 19 | `1` | `prim->y2 = position[1] + position[3];` | Rectangle Y origin slot; both top vertices use it and bottom vertices add the height. |
| `sprite_add_ft4` | 19 | `3` | `prim->y2 = position[1] + position[3];` | Rectangle height slot, added to Y to form the bottom edge. |
| `sprite_add_ft4` | 20 | `0` | `prim->x3 = position[0] + position[2];` | Rectangle X origin slot; both left vertices use it and right vertices add the width. |
| `sprite_add_ft4` | 20 | `2` | `prim->x3 = position[0] + position[2];` | Rectangle width slot, added to X to form the right edge. |
| `sprite_add_ft4` | 21 | `1` | `prim->y3 = position[1] + position[3];` | Rectangle Y origin slot; both top vertices use it and bottom vertices add the height. |
| `sprite_add_ft4` | 21 | `3` | `prim->y3 = position[1] + position[3];` | Rectangle height slot, added to Y to form the bottom edge. |
| `sprite_add_ft4` | 22 | `0` | `prim->u0 = texcoords[0];` | Byte offset of the texture U origin in the existing even-byte descriptor. |
| `sprite_add_ft4` | 23 | `2` | `prim->v0 = texcoords[2];` | Byte offset of the texture V origin in the existing even-byte descriptor. |
| `sprite_add_ft4` | 24 | `0` | `prim->u1 = texcoords[0] + texcoords[4];` | Byte offset of the texture U origin in the existing even-byte descriptor. |
| `sprite_add_ft4` | 24 | `4` | `prim->u1 = texcoords[0] + texcoords[4];` | Byte offset of the texture U span, added to the U origin. |
| `sprite_add_ft4` | 25 | `2` | `prim->v1 = texcoords[2];` | Byte offset of the texture V origin in the existing even-byte descriptor. |
| `sprite_add_ft4` | 26 | `0` | `prim->u2 = texcoords[0];` | Byte offset of the texture U origin in the existing even-byte descriptor. |
| `sprite_add_ft4` | 27 | `2` | `prim->v2 = texcoords[2] + texcoords[6];` | Byte offset of the texture V origin in the existing even-byte descriptor. |
| `sprite_add_ft4` | 27 | `6` | `prim->v2 = texcoords[2] + texcoords[6];` | Byte offset of the texture V span, added to the V origin. |
| `sprite_add_ft4` | 28 | `0` | `prim->u3 = texcoords[0] + texcoords[4];` | Byte offset of the texture U origin in the existing even-byte descriptor. |
| `sprite_add_ft4` | 28 | `4` | `prim->u3 = texcoords[0] + texcoords[4];` | Byte offset of the texture U span, added to the U origin. |
| `sprite_add_ft4` | 29 | `2` | `prim->v3 = texcoords[2] + texcoords[6];` | Byte offset of the texture V origin in the existing even-byte descriptor. |
| `sprite_add_ft4` | 29 | `6` | `prim->v3 = texcoords[2] + texcoords[6];` | Byte offset of the texture V span, added to the V origin. |

## `src/open/sprite_add_ft4.c`

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `sprite_add_ft4` | 14 | `0` | `prim->x0 = position[0];` | Rectangle X origin slot; both left vertices use it and right vertices add the width. |
| `sprite_add_ft4` | 15 | `1` | `prim->y0 = position[1];` | Rectangle Y origin slot; both top vertices use it and bottom vertices add the height. |
| `sprite_add_ft4` | 16 | `0` | `prim->x1 = position[0] + position[2];` | Rectangle X origin slot; both left vertices use it and right vertices add the width. |
| `sprite_add_ft4` | 16 | `2` | `prim->x1 = position[0] + position[2];` | Rectangle width slot, added to X to form the right edge. |
| `sprite_add_ft4` | 17 | `1` | `prim->y1 = position[1];` | Rectangle Y origin slot; both top vertices use it and bottom vertices add the height. |
| `sprite_add_ft4` | 18 | `0` | `prim->x2 = position[0];` | Rectangle X origin slot; both left vertices use it and right vertices add the width. |
| `sprite_add_ft4` | 19 | `1` | `prim->y2 = position[1] + position[3];` | Rectangle Y origin slot; both top vertices use it and bottom vertices add the height. |
| `sprite_add_ft4` | 19 | `3` | `prim->y2 = position[1] + position[3];` | Rectangle height slot, added to Y to form the bottom edge. |
| `sprite_add_ft4` | 20 | `0` | `prim->x3 = position[0] + position[2];` | Rectangle X origin slot; both left vertices use it and right vertices add the width. |
| `sprite_add_ft4` | 20 | `2` | `prim->x3 = position[0] + position[2];` | Rectangle width slot, added to X to form the right edge. |
| `sprite_add_ft4` | 21 | `1` | `prim->y3 = position[1] + position[3];` | Rectangle Y origin slot; both top vertices use it and bottom vertices add the height. |
| `sprite_add_ft4` | 21 | `3` | `prim->y3 = position[1] + position[3];` | Rectangle height slot, added to Y to form the bottom edge. |
| `sprite_add_ft4` | 22 | `0` | `prim->u0 = texcoords[0];` | Byte offset of the texture U origin in the existing even-byte descriptor. |
| `sprite_add_ft4` | 23 | `2` | `prim->v0 = texcoords[2];` | Byte offset of the texture V origin in the existing even-byte descriptor. |
| `sprite_add_ft4` | 24 | `0` | `prim->u1 = texcoords[0] + texcoords[4];` | Byte offset of the texture U origin in the existing even-byte descriptor. |
| `sprite_add_ft4` | 24 | `4` | `prim->u1 = texcoords[0] + texcoords[4];` | Byte offset of the texture U span, added to the U origin. |
| `sprite_add_ft4` | 25 | `2` | `prim->v1 = texcoords[2];` | Byte offset of the texture V origin in the existing even-byte descriptor. |
| `sprite_add_ft4` | 26 | `0` | `prim->u2 = texcoords[0];` | Byte offset of the texture U origin in the existing even-byte descriptor. |
| `sprite_add_ft4` | 27 | `2` | `prim->v2 = texcoords[2] + texcoords[6];` | Byte offset of the texture V origin in the existing even-byte descriptor. |
| `sprite_add_ft4` | 27 | `6` | `prim->v2 = texcoords[2] + texcoords[6];` | Byte offset of the texture V span, added to the V origin. |
| `sprite_add_ft4` | 28 | `0` | `prim->u3 = texcoords[0] + texcoords[4];` | Byte offset of the texture U origin in the existing even-byte descriptor. |
| `sprite_add_ft4` | 28 | `4` | `prim->u3 = texcoords[0] + texcoords[4];` | Byte offset of the texture U span, added to the U origin. |
| `sprite_add_ft4` | 29 | `2` | `prim->v3 = texcoords[2] + texcoords[6];` | Byte offset of the texture V origin in the existing even-byte descriptor. |
| `sprite_add_ft4` | 29 | `6` | `prim->v3 = texcoords[2] + texcoords[6];` | Byte offset of the texture V span, added to the V origin. |
| `sprite_add_f4` | 46 | `0` | `prim->x0 = position[0];` | Rectangle X origin slot; both left vertices use it and right vertices add the width. |
| `sprite_add_f4` | 47 | `1` | `prim->y0 = position[1];` | Rectangle Y origin slot; both top vertices use it and bottom vertices add the height. |
| `sprite_add_f4` | 48 | `0` | `prim->x1 = position[0] + position[2];` | Rectangle X origin slot; both left vertices use it and right vertices add the width. |
| `sprite_add_f4` | 48 | `2` | `prim->x1 = position[0] + position[2];` | Rectangle width slot, added to X to form the right edge. |
| `sprite_add_f4` | 49 | `1` | `prim->y1 = position[1];` | Rectangle Y origin slot; both top vertices use it and bottom vertices add the height. |
| `sprite_add_f4` | 50 | `0` | `prim->x2 = position[0];` | Rectangle X origin slot; both left vertices use it and right vertices add the width. |
| `sprite_add_f4` | 51 | `1` | `prim->y2 = position[1] + position[3];` | Rectangle Y origin slot; both top vertices use it and bottom vertices add the height. |
| `sprite_add_f4` | 51 | `3` | `prim->y2 = position[1] + position[3];` | Rectangle height slot, added to Y to form the bottom edge. |
| `sprite_add_f4` | 52 | `0` | `prim->x3 = position[0] + position[2];` | Rectangle X origin slot; both left vertices use it and right vertices add the width. |
| `sprite_add_f4` | 52 | `2` | `prim->x3 = position[0] + position[2];` | Rectangle width slot, added to X to form the right edge. |
| `sprite_add_f4` | 53 | `1` | `prim->y3 = position[1] + position[3];` | Rectangle Y origin slot; both top vertices use it and bottom vertices add the height. |
| `sprite_add_f4` | 53 | `3` | `prim->y3 = position[1] + position[3];` | Rectangle height slot, added to Y to form the bottom edge. |
