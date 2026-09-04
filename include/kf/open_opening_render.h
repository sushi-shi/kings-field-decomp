#ifndef KF_OPEN_OPENING_RENDER_H
#define KF_OPEN_OPENING_RENDER_H

#include <kf/semantic_types.h>

extern void opening_render_frame(
    const VECTOR *position, const SVECTOR *rotation);
extern void sprite_add_g4(
    const u16 *position, const u8 *color0, const u8 *color1,
    const u8 *color2, const u8 *color3);
extern void func_80019520(void);

#endif
