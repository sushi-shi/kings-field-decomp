#ifndef KF_GPU_PACKETS_H
#define KF_GPU_PACKETS_H

#include <kf/psyq.h>

/* Retail uses packed GTE XY words and UV halfwords. Each union retains the
 * authentic SDK packet at API boundaries and an equivalent typed word view.
 * tests/fixtures/open_tmd_enqueue_layout.c checks every shared field offset.
 */

typedef union KfGpuF3 {
    POLY_F3 sdk;
    struct {
        u_long tag;
        CVECTOR color0;
        long xy0;
        long xy1;
        long xy2;
    } packed;
} KfGpuF3;

typedef union KfGpuF4 {
    POLY_F4 sdk;
    struct {
        u_long tag;
        CVECTOR color0;
        long xy0;
        long xy1;
        long xy2;
        long xy3;
    } packed;
} KfGpuF4;

typedef union KfGpuFT3 {
    POLY_FT3 sdk;
    struct {
        u_long tag;
        CVECTOR color0;
        long xy0;
        u_short uv0;
        u_short clut;
        long xy1;
        u_short uv1;
        u_short tpage;
        long xy2;
        u_short uv2;
        u_short pad1;
    } packed;
} KfGpuFT3;

typedef union KfGpuFT4 {
    POLY_FT4 sdk;
    struct {
        u_long tag;
        CVECTOR color0;
        long xy0;
        u_short uv0;
        u_short clut;
        long xy1;
        u_short uv1;
        u_short tpage;
        long xy2;
        u_short uv2;
        u_short pad1;
        long xy3;
        u_short uv3;
        u_short pad2;
    } packed;
} KfGpuFT4;

typedef union KfGpuG3 {
    POLY_G3 sdk;
    struct {
        u_long tag;
        CVECTOR color0;
        long xy0;
        CVECTOR color1;
        long xy1;
        CVECTOR color2;
        long xy2;
    } packed;
} KfGpuG3;

typedef union KfGpuG4 {
    POLY_G4 sdk;
    struct {
        u_long tag;
        CVECTOR color0;
        long xy0;
        CVECTOR color1;
        long xy1;
        CVECTOR color2;
        long xy2;
        CVECTOR color3;
        long xy3;
    } packed;
} KfGpuG4;

typedef union KfGpuGT3 {
    POLY_GT3 sdk;
    struct {
        u_long tag;
        CVECTOR color0;
        long xy0;
        u_short uv0;
        u_short clut;
        CVECTOR color1;
        long xy1;
        u_short uv1;
        u_short tpage;
        CVECTOR color2;
        long xy2;
        u_short uv2;
        u_short pad2;
    } packed;
} KfGpuGT3;

typedef union KfGpuGT4 {
    POLY_GT4 sdk;
    struct {
        u_long tag;
        CVECTOR color0;
        long xy0;
        u_short uv0;
        u_short clut;
        CVECTOR color1;
        long xy1;
        u_short uv1;
        u_short tpage;
        CVECTOR color2;
        long xy2;
        u_short uv2;
        u_short pad2;
        CVECTOR color3;
        long xy3;
        u_short uv3;
        u_short pad3;
    } packed;
} KfGpuGT4;

#endif
