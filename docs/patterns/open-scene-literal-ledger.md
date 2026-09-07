# OPEN scene retained-literal ledger

Companion to [the scene-domain evidence](open-scene-domains.md). All **591**
current numeric/character occurrences in `src/open/opening_scenes.c` have
individual rows, including duplicate tokens within one expression. Comments,
strings, identifier digits, named enum/macro definitions and address claims
are excluded; unary minus remains part of the expression. Initialized owners
are identified by their DATA names. This documents authored data and observed
uses without claiming a semantic identity for unconsumed bytes or counters.

The camera helper checks the named X terminator before consuming a segment.
The sprite helpers consume four rectangle halfwords and only the even bytes
of each UV descriptor. These contracts distinguish coordinates and lengths
from ignored bytes; ignored storage is not automatically named padding.


The [audio sequence-domain review](audio-sequence-domains.md) names all three stop requests.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 80 | `0` | `(u32)&((KfOpeningEntity *)0)->position == 0x08 ? 1 : -1];` | Null base used only to calculate the measured field offset. |
| `initializers` | 80 | `0x08` | `(u32)&((KfOpeningEntity *)0)->position == 0x08 ? 1 : -1];` | Measured position offset within the opening entity; this is an offset check, not a size assertion. |
| `initializers` | 80 | `1` | `(u32)&((KfOpeningEntity *)0)->position == 0x08 ? 1 : -1];` | Positive array extent when the offset agrees. |
| `initializers` | 80 | `1` | `(u32)&((KfOpeningEntity *)0)->position == 0x08 ? 1 : -1];` | Negative array extent makes an offset mismatch a compile error. |
| `opening_scene0_camera_path` | 83 | `17` | `KfCameraPathPoint opening_scene0_camera_path[17] = {` | Exact authored camera-path record count, including the terminating row; not a runtime pool capacity. |
| `opening_scene0_camera_path` | 84 | `101000` | `{{101000, -13000, 101000, 0}, {256, 0, 200, 0}, 0, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 84 | `13000` | `{{101000, -13000, 101000, 0}, {256, 0, 200, 0}, 0, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 84 | `101000` | `{{101000, -13000, 101000, 0}, {256, 0, 200, 0}, 0, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 84 | `0` | `{{101000, -13000, 101000, 0}, {256, 0, 200, 0}, 0, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 84 | `256` | `{{101000, -13000, 101000, 0}, {256, 0, 200, 0}, 0, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 84 | `0` | `{{101000, -13000, 101000, 0}, {256, 0, 200, 0}, 0, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 84 | `200` | `{{101000, -13000, 101000, 0}, {256, 0, 200, 0}, 0, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 84 | `0` | `{{101000, -13000, 101000, 0}, {256, 0, 200, 0}, 0, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 84 | `0` | `{{101000, -13000, 101000, 0}, {256, 0, 200, 0}, 0, 0},` | Initial camera row has no incoming segment; its authored zero speed is not used as a divisor. |
| `opening_scene0_camera_path` | 84 | `0` | `{{101000, -13000, 101000, 0}, {256, 0, 200, 0}, 0, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene0_camera_path` | 85 | `101000` | `{{101000, -12000, 117000, 0}, {0, 128, -200, 0}, 400, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 85 | `12000` | `{{101000, -12000, 117000, 0}, {0, 128, -200, 0}, 400, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 85 | `117000` | `{{101000, -12000, 117000, 0}, {0, 128, -200, 0}, 400, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 85 | `0` | `{{101000, -12000, 117000, 0}, {0, 128, -200, 0}, 400, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 85 | `0` | `{{101000, -12000, 117000, 0}, {0, 128, -200, 0}, 400, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 85 | `128` | `{{101000, -12000, 117000, 0}, {0, 128, -200, 0}, 400, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 85 | `200` | `{{101000, -12000, 117000, 0}, {0, 128, -200, 0}, 400, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 85 | `0` | `{{101000, -12000, 117000, 0}, {0, 128, -200, 0}, 400, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 85 | `400` | `{{101000, -12000, 117000, 0}, {0, 128, -200, 0}, 400, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_scene0_camera_path` | 85 | `0` | `{{101000, -12000, 117000, 0}, {0, 128, -200, 0}, 400, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene0_camera_path` | 86 | `101000` | `{{101000, -11700, 127000, 0}, {0, 1024, 100, 0}, 400, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 86 | `11700` | `{{101000, -11700, 127000, 0}, {0, 1024, 100, 0}, 400, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 86 | `127000` | `{{101000, -11700, 127000, 0}, {0, 1024, 100, 0}, 400, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 86 | `0` | `{{101000, -11700, 127000, 0}, {0, 1024, 100, 0}, 400, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 86 | `0` | `{{101000, -11700, 127000, 0}, {0, 1024, 100, 0}, 400, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 86 | `1024` | `{{101000, -11700, 127000, 0}, {0, 1024, 100, 0}, 400, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 86 | `100` | `{{101000, -11700, 127000, 0}, {0, 1024, 100, 0}, 400, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 86 | `0` | `{{101000, -11700, 127000, 0}, {0, 1024, 100, 0}, 400, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 86 | `400` | `{{101000, -11700, 127000, 0}, {0, 1024, 100, 0}, 400, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_scene0_camera_path` | 86 | `0` | `{{101000, -11700, 127000, 0}, {0, 1024, 100, 0}, 400, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene0_camera_path` | 87 | `95000` | `{{95000, -11500, 127000, 0}, {0, 0, 300, 0}, 400, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 87 | `11500` | `{{95000, -11500, 127000, 0}, {0, 0, 300, 0}, 400, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 87 | `127000` | `{{95000, -11500, 127000, 0}, {0, 0, 300, 0}, 400, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 87 | `0` | `{{95000, -11500, 127000, 0}, {0, 0, 300, 0}, 400, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 87 | `0` | `{{95000, -11500, 127000, 0}, {0, 0, 300, 0}, 400, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 87 | `0` | `{{95000, -11500, 127000, 0}, {0, 0, 300, 0}, 400, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 87 | `300` | `{{95000, -11500, 127000, 0}, {0, 0, 300, 0}, 400, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 87 | `0` | `{{95000, -11500, 127000, 0}, {0, 0, 300, 0}, 400, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 87 | `400` | `{{95000, -11500, 127000, 0}, {0, 0, 300, 0}, 400, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_scene0_camera_path` | 87 | `0` | `{{95000, -11500, 127000, 0}, {0, 0, 300, 0}, 400, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene0_camera_path` | 88 | `95000` | `{{95000, -11200, 151000, 0}, {-256, 0, 0, 0}, 400, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 88 | `11200` | `{{95000, -11200, 151000, 0}, {-256, 0, 0, 0}, 400, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 88 | `151000` | `{{95000, -11200, 151000, 0}, {-256, 0, 0, 0}, 400, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 88 | `0` | `{{95000, -11200, 151000, 0}, {-256, 0, 0, 0}, 400, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 88 | `256` | `{{95000, -11200, 151000, 0}, {-256, 0, 0, 0}, 400, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 88 | `0` | `{{95000, -11200, 151000, 0}, {-256, 0, 0, 0}, 400, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 88 | `0` | `{{95000, -11200, 151000, 0}, {-256, 0, 0, 0}, 400, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 88 | `0` | `{{95000, -11200, 151000, 0}, {-256, 0, 0, 0}, 400, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 88 | `400` | `{{95000, -11200, 151000, 0}, {-256, 0, 0, 0}, 400, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_scene0_camera_path` | 88 | `0` | `{{95000, -11200, 151000, 0}, {-256, 0, 0, 0}, 400, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene0_camera_path` | 89 | `95000` | `{{95000, -11800, 165000, 0}, {0, -800, 0, 0}, 400, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 89 | `11800` | `{{95000, -11800, 165000, 0}, {0, -800, 0, 0}, 400, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 89 | `165000` | `{{95000, -11800, 165000, 0}, {0, -800, 0, 0}, 400, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 89 | `0` | `{{95000, -11800, 165000, 0}, {0, -800, 0, 0}, 400, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 89 | `0` | `{{95000, -11800, 165000, 0}, {0, -800, 0, 0}, 400, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 89 | `800` | `{{95000, -11800, 165000, 0}, {0, -800, 0, 0}, 400, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 89 | `0` | `{{95000, -11800, 165000, 0}, {0, -800, 0, 0}, 400, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 89 | `0` | `{{95000, -11800, 165000, 0}, {0, -800, 0, 0}, 400, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 89 | `400` | `{{95000, -11800, 165000, 0}, {0, -800, 0, 0}, 400, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_scene0_camera_path` | 89 | `0` | `{{95000, -11800, 165000, 0}, {0, -800, 0, 0}, 400, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene0_camera_path` | 90 | `97000` | `{{97000, -12200, 167000, 0}, {0, -1024, -200, 0}, 400, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 90 | `12200` | `{{97000, -12200, 167000, 0}, {0, -1024, -200, 0}, 400, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 90 | `167000` | `{{97000, -12200, 167000, 0}, {0, -1024, -200, 0}, 400, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 90 | `0` | `{{97000, -12200, 167000, 0}, {0, -1024, -200, 0}, 400, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 90 | `0` | `{{97000, -12200, 167000, 0}, {0, -1024, -200, 0}, 400, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 90 | `1024` | `{{97000, -12200, 167000, 0}, {0, -1024, -200, 0}, 400, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 90 | `200` | `{{97000, -12200, 167000, 0}, {0, -1024, -200, 0}, 400, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 90 | `0` | `{{97000, -12200, 167000, 0}, {0, -1024, -200, 0}, 400, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 90 | `400` | `{{97000, -12200, 167000, 0}, {0, -1024, -200, 0}, 400, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_scene0_camera_path` | 90 | `0` | `{{97000, -12200, 167000, 0}, {0, -1024, -200, 0}, 400, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene0_camera_path` | 91 | `113000` | `{{113000, -11900, 167000, 0}, {0, -1024, -100, 0}, 400, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 91 | `11900` | `{{113000, -11900, 167000, 0}, {0, -1024, -100, 0}, 400, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 91 | `167000` | `{{113000, -11900, 167000, 0}, {0, -1024, -100, 0}, 400, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 91 | `0` | `{{113000, -11900, 167000, 0}, {0, -1024, -100, 0}, 400, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 91 | `0` | `{{113000, -11900, 167000, 0}, {0, -1024, -100, 0}, 400, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 91 | `1024` | `{{113000, -11900, 167000, 0}, {0, -1024, -100, 0}, 400, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 91 | `100` | `{{113000, -11900, 167000, 0}, {0, -1024, -100, 0}, 400, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 91 | `0` | `{{113000, -11900, 167000, 0}, {0, -1024, -100, 0}, 400, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 91 | `400` | `{{113000, -11900, 167000, 0}, {0, -1024, -100, 0}, 400, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_scene0_camera_path` | 91 | `0` | `{{113000, -11900, 167000, 0}, {0, -1024, -100, 0}, 400, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene0_camera_path` | 92 | `127000` | `{{127000, -11800, 167000, 0}, {0, -1024, 0, 0}, 400, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 92 | `11800` | `{{127000, -11800, 167000, 0}, {0, -1024, 0, 0}, 400, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 92 | `167000` | `{{127000, -11800, 167000, 0}, {0, -1024, 0, 0}, 400, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 92 | `0` | `{{127000, -11800, 167000, 0}, {0, -1024, 0, 0}, 400, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 92 | `0` | `{{127000, -11800, 167000, 0}, {0, -1024, 0, 0}, 400, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 92 | `1024` | `{{127000, -11800, 167000, 0}, {0, -1024, 0, 0}, 400, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 92 | `0` | `{{127000, -11800, 167000, 0}, {0, -1024, 0, 0}, 400, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 92 | `0` | `{{127000, -11800, 167000, 0}, {0, -1024, 0, 0}, 400, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 92 | `400` | `{{127000, -11800, 167000, 0}, {0, -1024, 0, 0}, 400, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_scene0_camera_path` | 92 | `0` | `{{127000, -11800, 167000, 0}, {0, -1024, 0, 0}, 400, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene0_camera_path` | 93 | `131000` | `{{131000, -12200, 167000, 0}, {100, -800, -100, 0}, 400, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 93 | `12200` | `{{131000, -12200, 167000, 0}, {100, -800, -100, 0}, 400, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 93 | `167000` | `{{131000, -12200, 167000, 0}, {100, -800, -100, 0}, 400, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 93 | `0` | `{{131000, -12200, 167000, 0}, {100, -800, -100, 0}, 400, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 93 | `100` | `{{131000, -12200, 167000, 0}, {100, -800, -100, 0}, 400, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 93 | `800` | `{{131000, -12200, 167000, 0}, {100, -800, -100, 0}, 400, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 93 | `100` | `{{131000, -12200, 167000, 0}, {100, -800, -100, 0}, 400, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 93 | `0` | `{{131000, -12200, 167000, 0}, {100, -800, -100, 0}, 400, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 93 | `400` | `{{131000, -12200, 167000, 0}, {100, -800, -100, 0}, 400, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_scene0_camera_path` | 93 | `0` | `{{131000, -12200, 167000, 0}, {100, -800, -100, 0}, 400, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene0_camera_path` | 94 | `139000` | `{{139000, -12000, 171000, 0}, {100, -512, -100, 0}, 400, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 94 | `12000` | `{{139000, -12000, 171000, 0}, {100, -512, -100, 0}, 400, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 94 | `171000` | `{{139000, -12000, 171000, 0}, {100, -512, -100, 0}, 400, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 94 | `0` | `{{139000, -12000, 171000, 0}, {100, -512, -100, 0}, 400, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 94 | `100` | `{{139000, -12000, 171000, 0}, {100, -512, -100, 0}, 400, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 94 | `512` | `{{139000, -12000, 171000, 0}, {100, -512, -100, 0}, 400, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 94 | `100` | `{{139000, -12000, 171000, 0}, {100, -512, -100, 0}, 400, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 94 | `0` | `{{139000, -12000, 171000, 0}, {100, -512, -100, 0}, 400, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 94 | `400` | `{{139000, -12000, 171000, 0}, {100, -512, -100, 0}, 400, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_scene0_camera_path` | 94 | `0` | `{{139000, -12000, 171000, 0}, {100, -512, -100, 0}, 400, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene0_camera_path` | 95 | `149000` | `{{149000, -11900, 181000, 0}, {100, -750, -100, 0}, 400, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 95 | `11900` | `{{149000, -11900, 181000, 0}, {100, -750, -100, 0}, 400, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 95 | `181000` | `{{149000, -11900, 181000, 0}, {100, -750, -100, 0}, 400, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 95 | `0` | `{{149000, -11900, 181000, 0}, {100, -750, -100, 0}, 400, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 95 | `100` | `{{149000, -11900, 181000, 0}, {100, -750, -100, 0}, 400, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 95 | `750` | `{{149000, -11900, 181000, 0}, {100, -750, -100, 0}, 400, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 95 | `100` | `{{149000, -11900, 181000, 0}, {100, -750, -100, 0}, 400, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 95 | `0` | `{{149000, -11900, 181000, 0}, {100, -750, -100, 0}, 400, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 95 | `400` | `{{149000, -11900, 181000, 0}, {100, -750, -100, 0}, 400, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_scene0_camera_path` | 95 | `0` | `{{149000, -11900, 181000, 0}, {100, -750, -100, 0}, 400, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene0_camera_path` | 96 | `161000` | `{{161000, -11900, 189000, 0}, {100, -1024, -100, 0}, 400, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 96 | `11900` | `{{161000, -11900, 189000, 0}, {100, -1024, -100, 0}, 400, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 96 | `189000` | `{{161000, -11900, 189000, 0}, {100, -1024, -100, 0}, 400, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 96 | `0` | `{{161000, -11900, 189000, 0}, {100, -1024, -100, 0}, 400, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 96 | `100` | `{{161000, -11900, 189000, 0}, {100, -1024, -100, 0}, 400, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 96 | `1024` | `{{161000, -11900, 189000, 0}, {100, -1024, -100, 0}, 400, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 96 | `100` | `{{161000, -11900, 189000, 0}, {100, -1024, -100, 0}, 400, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 96 | `0` | `{{161000, -11900, 189000, 0}, {100, -1024, -100, 0}, 400, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 96 | `400` | `{{161000, -11900, 189000, 0}, {100, -1024, -100, 0}, 400, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_scene0_camera_path` | 96 | `0` | `{{161000, -11900, 189000, 0}, {100, -1024, -100, 0}, 400, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene0_camera_path` | 97 | `163000` | `{{163000, -11900, 187000, 0}, {100, -1536, -100, 0}, 300, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 97 | `11900` | `{{163000, -11900, 187000, 0}, {100, -1536, -100, 0}, 300, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 97 | `187000` | `{{163000, -11900, 187000, 0}, {100, -1536, -100, 0}, 300, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 97 | `0` | `{{163000, -11900, 187000, 0}, {100, -1536, -100, 0}, 300, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 97 | `100` | `{{163000, -11900, 187000, 0}, {100, -1536, -100, 0}, 300, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 97 | `1536` | `{{163000, -11900, 187000, 0}, {100, -1536, -100, 0}, 300, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 97 | `100` | `{{163000, -11900, 187000, 0}, {100, -1536, -100, 0}, 300, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 97 | `0` | `{{163000, -11900, 187000, 0}, {100, -1536, -100, 0}, 300, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 97 | `300` | `{{163000, -11900, 187000, 0}, {100, -1536, -100, 0}, 300, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_scene0_camera_path` | 97 | `0` | `{{163000, -11900, 187000, 0}, {100, -1536, -100, 0}, 300, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene0_camera_path` | 98 | `163000` | `{{163000, -11900, 163000, 0}, {100, -512, -100, 0}, 200, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 98 | `11900` | `{{163000, -11900, 163000, 0}, {100, -512, -100, 0}, 200, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 98 | `163000` | `{{163000, -11900, 163000, 0}, {100, -512, -100, 0}, 200, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 98 | `0` | `{{163000, -11900, 163000, 0}, {100, -512, -100, 0}, 200, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 98 | `100` | `{{163000, -11900, 163000, 0}, {100, -512, -100, 0}, 200, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 98 | `512` | `{{163000, -11900, 163000, 0}, {100, -512, -100, 0}, 200, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 98 | `100` | `{{163000, -11900, 163000, 0}, {100, -512, -100, 0}, 200, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 98 | `0` | `{{163000, -11900, 163000, 0}, {100, -512, -100, 0}, 200, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 98 | `200` | `{{163000, -11900, 163000, 0}, {100, -512, -100, 0}, 200, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_scene0_camera_path` | 98 | `0` | `{{163000, -11900, 163000, 0}, {100, -512, -100, 0}, 200, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene0_camera_path` | 99 | `163000` | `{{163000, -12100, 161000, 0}, {100, -256, -100, 0}, 100, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 99 | `12100` | `{{163000, -12100, 161000, 0}, {100, -256, -100, 0}, 100, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 99 | `161000` | `{{163000, -12100, 161000, 0}, {100, -256, -100, 0}, 100, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_scene0_camera_path` | 99 | `0` | `{{163000, -12100, 161000, 0}, {100, -256, -100, 0}, 100, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 99 | `100` | `{{163000, -12100, 161000, 0}, {100, -256, -100, 0}, 100, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 99 | `256` | `{{163000, -12100, 161000, 0}, {100, -256, -100, 0}, 100, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 99 | `100` | `{{163000, -12100, 161000, 0}, {100, -256, -100, 0}, 100, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene0_camera_path` | 99 | `0` | `{{163000, -12100, 161000, 0}, {100, -256, -100, 0}, 100, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 99 | `100` | `{{163000, -12100, 161000, 0}, {100, -256, -100, 0}, 100, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_scene0_camera_path` | 99 | `0` | `{{163000, -12100, 161000, 0}, {100, -256, -100, 0}, 100, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene0_camera_path` | 100 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row position Y fill; the X sentinel prevents this field from defining a segment. |
| `opening_scene0_camera_path` | 100 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row position Z fill; the X sentinel prevents this field from defining a segment. |
| `opening_scene0_camera_path` | 100 | `0` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 100 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row rotation X fill; the X sentinel prevents this field from defining a segment. |
| `opening_scene0_camera_path` | 100 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row rotation Y fill; the X sentinel prevents this field from defining a segment. |
| `opening_scene0_camera_path` | 100 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row rotation Z fill; the X sentinel prevents this field from defining a segment. |
| `opening_scene0_camera_path` | 100 | `0` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene0_camera_path` | 100 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row segment speed fill; the X sentinel prevents this field from defining a segment. |
| `opening_scene0_camera_path` | 100 | `0` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene3_camera_path` | 104 | `3` | `KfCameraPathPoint opening_scene3_camera_path[3] = {` | Exact authored camera-path record count, including the terminating row; not a runtime pool capacity. |
| `opening_scene3_camera_path` | 105 | `101000` | `{{101000, -11500, 115200, 0}, {0, 0x800, 0, 0}, 0, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_scene3_camera_path` | 105 | `11500` | `{{101000, -11500, 115200, 0}, {0, 0x800, 0, 0}, 0, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_scene3_camera_path` | 105 | `115200` | `{{101000, -11500, 115200, 0}, {0, 0x800, 0, 0}, 0, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_scene3_camera_path` | 105 | `0` | `{{101000, -11500, 115200, 0}, {0, 0x800, 0, 0}, 0, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene3_camera_path` | 105 | `0` | `{{101000, -11500, 115200, 0}, {0, 0x800, 0, 0}, 0, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene3_camera_path` | 105 | `0x800` | `{{101000, -11500, 115200, 0}, {0, 0x800, 0, 0}, 0, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene3_camera_path` | 105 | `0` | `{{101000, -11500, 115200, 0}, {0, 0x800, 0, 0}, 0, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene3_camera_path` | 105 | `0` | `{{101000, -11500, 115200, 0}, {0, 0x800, 0, 0}, 0, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene3_camera_path` | 105 | `0` | `{{101000, -11500, 115200, 0}, {0, 0x800, 0, 0}, 0, 0},` | Initial camera row has no incoming segment; its authored zero speed is not used as a divisor. |
| `opening_scene3_camera_path` | 105 | `0` | `{{101000, -11500, 115200, 0}, {0, 0x800, 0, 0}, 0, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene3_camera_path` | 106 | `101000` | `{{101000, -11500, 101000, 0}, {0, 0x800, 0, 0}, 0x10, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_scene3_camera_path` | 106 | `11500` | `{{101000, -11500, 101000, 0}, {0, 0x800, 0, 0}, 0x10, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_scene3_camera_path` | 106 | `101000` | `{{101000, -11500, 101000, 0}, {0, 0x800, 0, 0}, 0x10, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_scene3_camera_path` | 106 | `0` | `{{101000, -11500, 101000, 0}, {0, 0x800, 0, 0}, 0x10, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene3_camera_path` | 106 | `0` | `{{101000, -11500, 101000, 0}, {0, 0x800, 0, 0}, 0x10, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene3_camera_path` | 106 | `0x800` | `{{101000, -11500, 101000, 0}, {0, 0x800, 0, 0}, 0x10, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene3_camera_path` | 106 | `0` | `{{101000, -11500, 101000, 0}, {0, 0x800, 0, 0}, 0x10, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_scene3_camera_path` | 106 | `0` | `{{101000, -11500, 101000, 0}, {0, 0x800, 0, 0}, 0x10, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene3_camera_path` | 106 | `0x10` | `{{101000, -11500, 101000, 0}, {0, 0x800, 0, 0}, 0x10, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_scene3_camera_path` | 106 | `0` | `{{101000, -11500, 101000, 0}, {0, 0x800, 0, 0}, 0x10, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene3_camera_path` | 107 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row position Y fill; the X sentinel prevents this field from defining a segment. |
| `opening_scene3_camera_path` | 107 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row position Z fill; the X sentinel prevents this field from defining a segment. |
| `opening_scene3_camera_path` | 107 | `0` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene3_camera_path` | 107 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row rotation X fill; the X sentinel prevents this field from defining a segment. |
| `opening_scene3_camera_path` | 107 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row rotation Y fill; the X sentinel prevents this field from defining a segment. |
| `opening_scene3_camera_path` | 107 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row rotation Z fill; the X sentinel prevents this field from defining a segment. |
| `opening_scene3_camera_path` | 107 | `0` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_scene3_camera_path` | 107 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row segment speed fill; the X sentinel prevents this field from defining a segment. |
| `opening_scene3_camera_path` | 107 | `0` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_ending_camera_path` | 111 | `9` | `KfCameraPathPoint opening_ending_camera_path[9] = {` | Exact authored camera-path record count, including the terminating row; not a runtime pool capacity. |
| `opening_ending_camera_path` | 112 | `101000` | `{{101000, -11500, 101000, 0}, {0, 0, 0, 0}, 0, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 112 | `11500` | `{{101000, -11500, 101000, 0}, {0, 0, 0, 0}, 0, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 112 | `101000` | `{{101000, -11500, 101000, 0}, {0, 0, 0, 0}, 0, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 112 | `0` | `{{101000, -11500, 101000, 0}, {0, 0, 0, 0}, 0, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_camera_path` | 112 | `0` | `{{101000, -11500, 101000, 0}, {0, 0, 0, 0}, 0, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 112 | `0` | `{{101000, -11500, 101000, 0}, {0, 0, 0, 0}, 0, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 112 | `0` | `{{101000, -11500, 101000, 0}, {0, 0, 0, 0}, 0, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 112 | `0` | `{{101000, -11500, 101000, 0}, {0, 0, 0, 0}, 0, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_camera_path` | 112 | `0` | `{{101000, -11500, 101000, 0}, {0, 0, 0, 0}, 0, 0},` | Initial camera row has no incoming segment; its authored zero speed is not used as a divisor. |
| `opening_ending_camera_path` | 112 | `0` | `{{101000, -11500, 101000, 0}, {0, 0, 0, 0}, 0, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_ending_camera_path` | 113 | `101000` | `{{101000, -11500, 108600, 0}, {0, 0, 0, 0}, 35, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 113 | `11500` | `{{101000, -11500, 108600, 0}, {0, 0, 0, 0}, 35, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 113 | `108600` | `{{101000, -11500, 108600, 0}, {0, 0, 0, 0}, 35, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 113 | `0` | `{{101000, -11500, 108600, 0}, {0, 0, 0, 0}, 35, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_camera_path` | 113 | `0` | `{{101000, -11500, 108600, 0}, {0, 0, 0, 0}, 35, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 113 | `0` | `{{101000, -11500, 108600, 0}, {0, 0, 0, 0}, 35, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 113 | `0` | `{{101000, -11500, 108600, 0}, {0, 0, 0, 0}, 35, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 113 | `0` | `{{101000, -11500, 108600, 0}, {0, 0, 0, 0}, 35, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_camera_path` | 113 | `35` | `{{101000, -11500, 108600, 0}, {0, 0, 0, 0}, 35, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_ending_camera_path` | 113 | `0` | `{{101000, -11500, 108600, 0}, {0, 0, 0, 0}, 35, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_ending_camera_path` | 114 | `101000` | `{{101000, -11500, 108800, 0}, {0, 0, 0, 0}, 20, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 114 | `11500` | `{{101000, -11500, 108800, 0}, {0, 0, 0, 0}, 20, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 114 | `108800` | `{{101000, -11500, 108800, 0}, {0, 0, 0, 0}, 20, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 114 | `0` | `{{101000, -11500, 108800, 0}, {0, 0, 0, 0}, 20, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_camera_path` | 114 | `0` | `{{101000, -11500, 108800, 0}, {0, 0, 0, 0}, 20, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 114 | `0` | `{{101000, -11500, 108800, 0}, {0, 0, 0, 0}, 20, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 114 | `0` | `{{101000, -11500, 108800, 0}, {0, 0, 0, 0}, 20, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 114 | `0` | `{{101000, -11500, 108800, 0}, {0, 0, 0, 0}, 20, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_camera_path` | 114 | `20` | `{{101000, -11500, 108800, 0}, {0, 0, 0, 0}, 20, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_ending_camera_path` | 114 | `0` | `{{101000, -11500, 108800, 0}, {0, 0, 0, 0}, 20, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_ending_camera_path` | 115 | `101000` | `{{101000, -11500, 109000, 0}, {0, 0, 0, 0}, 10, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 115 | `11500` | `{{101000, -11500, 109000, 0}, {0, 0, 0, 0}, 10, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 115 | `109000` | `{{101000, -11500, 109000, 0}, {0, 0, 0, 0}, 10, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 115 | `0` | `{{101000, -11500, 109000, 0}, {0, 0, 0, 0}, 10, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_camera_path` | 115 | `0` | `{{101000, -11500, 109000, 0}, {0, 0, 0, 0}, 10, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 115 | `0` | `{{101000, -11500, 109000, 0}, {0, 0, 0, 0}, 10, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 115 | `0` | `{{101000, -11500, 109000, 0}, {0, 0, 0, 0}, 10, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 115 | `0` | `{{101000, -11500, 109000, 0}, {0, 0, 0, 0}, 10, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_camera_path` | 115 | `10` | `{{101000, -11500, 109000, 0}, {0, 0, 0, 0}, 10, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_ending_camera_path` | 115 | `0` | `{{101000, -11500, 109000, 0}, {0, 0, 0, 0}, 10, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_ending_camera_path` | 116 | `101000` | `{{101000, -11500, 109050, 0}, {0, 0, 0, 0}, 1, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 116 | `11500` | `{{101000, -11500, 109050, 0}, {0, 0, 0, 0}, 1, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 116 | `109050` | `{{101000, -11500, 109050, 0}, {0, 0, 0, 0}, 1, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 116 | `0` | `{{101000, -11500, 109050, 0}, {0, 0, 0, 0}, 1, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_camera_path` | 116 | `0` | `{{101000, -11500, 109050, 0}, {0, 0, 0, 0}, 1, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 116 | `0` | `{{101000, -11500, 109050, 0}, {0, 0, 0, 0}, 1, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 116 | `0` | `{{101000, -11500, 109050, 0}, {0, 0, 0, 0}, 1, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 116 | `0` | `{{101000, -11500, 109050, 0}, {0, 0, 0, 0}, 1, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_camera_path` | 116 | `1` | `{{101000, -11500, 109050, 0}, {0, 0, 0, 0}, 1, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_ending_camera_path` | 116 | `0` | `{{101000, -11500, 109050, 0}, {0, 0, 0, 0}, 1, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_ending_camera_path` | 117 | `101000` | `{{101000, -11500, 109400, 0}, {0, 0, 0, 0}, 30, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 117 | `11500` | `{{101000, -11500, 109400, 0}, {0, 0, 0, 0}, 30, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 117 | `109400` | `{{101000, -11500, 109400, 0}, {0, 0, 0, 0}, 30, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 117 | `0` | `{{101000, -11500, 109400, 0}, {0, 0, 0, 0}, 30, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_camera_path` | 117 | `0` | `{{101000, -11500, 109400, 0}, {0, 0, 0, 0}, 30, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 117 | `0` | `{{101000, -11500, 109400, 0}, {0, 0, 0, 0}, 30, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 117 | `0` | `{{101000, -11500, 109400, 0}, {0, 0, 0, 0}, 30, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 117 | `0` | `{{101000, -11500, 109400, 0}, {0, 0, 0, 0}, 30, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_camera_path` | 117 | `30` | `{{101000, -11500, 109400, 0}, {0, 0, 0, 0}, 30, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_ending_camera_path` | 117 | `0` | `{{101000, -11500, 109400, 0}, {0, 0, 0, 0}, 30, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_ending_camera_path` | 118 | `101000` | `{{101000, -11500, 109800, 0}, {0, 0, 0, 0}, 60, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 118 | `11500` | `{{101000, -11500, 109800, 0}, {0, 0, 0, 0}, 60, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 118 | `109800` | `{{101000, -11500, 109800, 0}, {0, 0, 0, 0}, 60, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 118 | `0` | `{{101000, -11500, 109800, 0}, {0, 0, 0, 0}, 60, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_camera_path` | 118 | `0` | `{{101000, -11500, 109800, 0}, {0, 0, 0, 0}, 60, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 118 | `0` | `{{101000, -11500, 109800, 0}, {0, 0, 0, 0}, 60, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 118 | `0` | `{{101000, -11500, 109800, 0}, {0, 0, 0, 0}, 60, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 118 | `0` | `{{101000, -11500, 109800, 0}, {0, 0, 0, 0}, 60, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_camera_path` | 118 | `60` | `{{101000, -11500, 109800, 0}, {0, 0, 0, 0}, 60, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_ending_camera_path` | 118 | `0` | `{{101000, -11500, 109800, 0}, {0, 0, 0, 0}, 60, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_ending_camera_path` | 119 | `101000` | `{{101000, -11500, 117000, 0}, {0, 0, 0, 0}, 100, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 119 | `11500` | `{{101000, -11500, 117000, 0}, {0, 0, 0, 0}, 100, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 119 | `117000` | `{{101000, -11500, 117000, 0}, {0, 0, 0, 0}, 100, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_camera_path` | 119 | `0` | `{{101000, -11500, 117000, 0}, {0, 0, 0, 0}, 100, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_camera_path` | 119 | `0` | `{{101000, -11500, 117000, 0}, {0, 0, 0, 0}, 100, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 119 | `0` | `{{101000, -11500, 117000, 0}, {0, 0, 0, 0}, 100, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 119 | `0` | `{{101000, -11500, 117000, 0}, {0, 0, 0, 0}, 100, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_camera_path` | 119 | `0` | `{{101000, -11500, 117000, 0}, {0, 0, 0, 0}, 100, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_camera_path` | 119 | `100` | `{{101000, -11500, 117000, 0}, {0, 0, 0, 0}, 100, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_ending_camera_path` | 119 | `0` | `{{101000, -11500, 117000, 0}, {0, 0, 0, 0}, 100, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_ending_camera_path` | 120 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row position Y fill; the X sentinel prevents this field from defining a segment. |
| `opening_ending_camera_path` | 120 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row position Z fill; the X sentinel prevents this field from defining a segment. |
| `opening_ending_camera_path` | 120 | `0` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_camera_path` | 120 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row rotation X fill; the X sentinel prevents this field from defining a segment. |
| `opening_ending_camera_path` | 120 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row rotation Y fill; the X sentinel prevents this field from defining a segment. |
| `opening_ending_camera_path` | 120 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row rotation Z fill; the X sentinel prevents this field from defining a segment. |
| `opening_ending_camera_path` | 120 | `0` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_camera_path` | 120 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row segment speed fill; the X sentinel prevents this field from defining a segment. |
| `opening_ending_camera_path` | 120 | `0` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_ending_scroll_camera_path` | 124 | `3` | `static KfCameraPathPoint opening_ending_scroll_camera_path[3] = {` | Exact authored camera-path record count, including the terminating row; not a runtime pool capacity. |
| `opening_ending_scroll_camera_path` | 125 | `101000` | `{{101000, -8500, 89000, 0}, {0, 0, 0, 0}, 0, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_scroll_camera_path` | 125 | `8500` | `{{101000, -8500, 89000, 0}, {0, 0, 0, 0}, 0, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_scroll_camera_path` | 125 | `89000` | `{{101000, -8500, 89000, 0}, {0, 0, 0, 0}, 0, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_scroll_camera_path` | 125 | `0` | `{{101000, -8500, 89000, 0}, {0, 0, 0, 0}, 0, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_scroll_camera_path` | 125 | `0` | `{{101000, -8500, 89000, 0}, {0, 0, 0, 0}, 0, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_scroll_camera_path` | 125 | `0` | `{{101000, -8500, 89000, 0}, {0, 0, 0, 0}, 0, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_scroll_camera_path` | 125 | `0` | `{{101000, -8500, 89000, 0}, {0, 0, 0, 0}, 0, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_scroll_camera_path` | 125 | `0` | `{{101000, -8500, 89000, 0}, {0, 0, 0, 0}, 0, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_scroll_camera_path` | 125 | `0` | `{{101000, -8500, 89000, 0}, {0, 0, 0, 0}, 0, 0},` | Initial camera row has no incoming segment; its authored zero speed is not used as a divisor. |
| `opening_ending_scroll_camera_path` | 125 | `0` | `{{101000, -8500, 89000, 0}, {0, 0, 0, 0}, 0, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_ending_scroll_camera_path` | 126 | `101000` | `{{101000, -12000, 89000, 0}, {0, 0, 0, 0}, 2, 0},` | Authored world-space position X sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_scroll_camera_path` | 126 | `12000` | `{{101000, -12000, 89000, 0}, {0, 0, 0, 0}, 2, 0},` | Authored world-space position Y sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_scroll_camera_path` | 126 | `89000` | `{{101000, -12000, 89000, 0}, {0, 0, 0, 0}, 2, 0},` | Authored world-space position Z sample in this trajectory; preserve the numeric path shape. |
| `opening_ending_scroll_camera_path` | 126 | `0` | `{{101000, -12000, 89000, 0}, {0, 0, 0, 0}, 2, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_scroll_camera_path` | 126 | `0` | `{{101000, -12000, 89000, 0}, {0, 0, 0, 0}, 2, 0},` | Authored rotation X sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_scroll_camera_path` | 126 | `0` | `{{101000, -12000, 89000, 0}, {0, 0, 0, 0}, 2, 0},` | Authored rotation Y sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_scroll_camera_path` | 126 | `0` | `{{101000, -12000, 89000, 0}, {0, 0, 0, 0}, 2, 0},` | Authored rotation Z sample in 4096-unit turns; preserve the camera orientation trajectory. |
| `opening_ending_scroll_camera_path` | 126 | `0` | `{{101000, -12000, 89000, 0}, {0, 0, 0, 0}, 2, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_scroll_camera_path` | 126 | `2` | `{{101000, -12000, 89000, 0}, {0, 0, 0, 0}, 2, 0},` | Authored segment speed used with displacement length to form camera deltas and frame count. |
| `opening_ending_scroll_camera_path` | 126 | `0` | `{{101000, -12000, 89000, 0}, {0, 0, 0, 0}, 2, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_ending_scroll_camera_path` | 127 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row position Y fill; the X sentinel prevents this field from defining a segment. |
| `opening_ending_scroll_camera_path` | 127 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row position Z fill; the X sentinel prevents this field from defining a segment. |
| `opening_ending_scroll_camera_path` | 127 | `0` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Zero VECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_scroll_camera_path` | 127 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row rotation X fill; the X sentinel prevents this field from defining a segment. |
| `opening_ending_scroll_camera_path` | 127 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row rotation Y fill; the X sentinel prevents this field from defining a segment. |
| `opening_ending_scroll_camera_path` | 127 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row rotation Z fill; the X sentinel prevents this field from defining a segment. |
| `opening_ending_scroll_camera_path` | 127 | `0` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Zero SVECTOR fourth lane; preserve the SDK vector representation. |
| `opening_ending_scroll_camera_path` | 127 | `1` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Terminator-row segment speed fill; the X sentinel prevents this field from defining a segment. |
| `opening_ending_scroll_camera_path` | 127 | `0` | `{{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},` | Zero record halfword +0x1a; the reviewed camera consumer does not establish its purpose. |
| `opening_scene0_sound` | 131 | `9` | `SoundRef opening_scene0_sound = {9, 0, 0x43};` | Authored sound program byte; no independently supported sound-asset name. |
| `opening_scene0_sound` | 131 | `0` | `SoundRef opening_scene0_sound = {9, 0, 0x43};` | Authored sound tone byte; zero selects a tone, not silence. |
| `opening_scene0_sound` | 131 | `0x43` | `SoundRef opening_scene0_sound = {9, 0, 0x43};` | Authored sound note byte; no independently supported sound-asset name. |
| `opening_scene3_overlay_rects` | 134 | `4` | `u16 opening_scene3_overlay_rects[KF_OPENING_SCENE3_OVERLAY_COUNT][4] = {` | Four halfwords per rectangle: X, Y, width, height. |
| `opening_scene3_overlay_rects` | 135 | `32` | `{32, 256, 255, 254},` | Authored rectangle X origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_scene3_overlay_rects` | 135 | `256` | `{32, 256, 255, 254},` | Authored rectangle Y origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_scene3_overlay_rects` | 135 | `255` | `{32, 256, 255, 254},` | Authored rectangle width in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_scene3_overlay_rects` | 135 | `254` | `{32, 256, 255, 254},` | Authored rectangle height in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_scene3_overlay_rects` | 136 | `32` | `{32, 512, 255, 254},` | Authored rectangle X origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_scene3_overlay_rects` | 136 | `512` | `{32, 512, 255, 254},` | Authored rectangle Y origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_scene3_overlay_rects` | 136 | `255` | `{32, 512, 255, 254},` | Authored rectangle width in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_scene3_overlay_rects` | 136 | `254` | `{32, 512, 255, 254},` | Authored rectangle height in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 140 | `4` | `static u16 opening_ending_scroll_panels[ENDING_PANEL_COUNT][4] = {` | Four halfwords per rectangle: X, Y, width, height. |
| `opening_ending_scroll_panels` | 141 | `32` | `{32, 256, 255, 254},` | Authored rectangle X origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 141 | `256` | `{32, 256, 255, 254},` | Authored rectangle Y origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 141 | `255` | `{32, 256, 255, 254},` | Authored rectangle width in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 141 | `254` | `{32, 256, 255, 254},` | Authored rectangle height in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 142 | `32` | `{32, 512, 255, 254},` | Authored rectangle X origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 142 | `512` | `{32, 512, 255, 254},` | Authored rectangle Y origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 142 | `255` | `{32, 512, 255, 254},` | Authored rectangle width in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 142 | `254` | `{32, 512, 255, 254},` | Authored rectangle height in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 143 | `32` | `{32, 768, 255, 254},` | Authored rectangle X origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 143 | `768` | `{32, 768, 255, 254},` | Authored rectangle Y origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 143 | `255` | `{32, 768, 255, 254},` | Authored rectangle width in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 143 | `254` | `{32, 768, 255, 254},` | Authored rectangle height in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 144 | `32` | `{32, 1024, 255, 254},` | Authored rectangle X origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 144 | `1024` | `{32, 1024, 255, 254},` | Authored rectangle Y origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 144 | `255` | `{32, 1024, 255, 254},` | Authored rectangle width in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 144 | `254` | `{32, 1024, 255, 254},` | Authored rectangle height in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 145 | `32` | `{32, 1261, 255, 254},` | Authored rectangle X origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 145 | `1261` | `{32, 1261, 255, 254},` | Authored rectangle Y origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 145 | `255` | `{32, 1261, 255, 254},` | Authored rectangle width in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 145 | `254` | `{32, 1261, 255, 254},` | Authored rectangle height in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 146 | `32` | `{32, 1490, 255, 254},` | Authored rectangle X origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 146 | `1490` | `{32, 1490, 255, 254},` | Authored rectangle Y origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 146 | `255` | `{32, 1490, 255, 254},` | Authored rectangle width in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 146 | `254` | `{32, 1490, 255, 254},` | Authored rectangle height in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 147 | `32` | `{32, 1746, 255, 254},` | Authored rectangle X origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 147 | `1746` | `{32, 1746, 255, 254},` | Authored rectangle Y origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 147 | `255` | `{32, 1746, 255, 254},` | Authored rectangle width in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 147 | `254` | `{32, 1746, 255, 254},` | Authored rectangle height in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 148 | `32` | `{32, 2002, 255, 254},` | Authored rectangle X origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 148 | `2002` | `{32, 2002, 255, 254},` | Authored rectangle Y origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 148 | `255` | `{32, 2002, 255, 254},` | Authored rectangle width in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 148 | `254` | `{32, 2002, 255, 254},` | Authored rectangle height in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 149 | `32` | `{32, 2258, 255, 254},` | Authored rectangle X origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 149 | `2258` | `{32, 2258, 255, 254},` | Authored rectangle Y origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 149 | `255` | `{32, 2258, 255, 254},` | Authored rectangle width in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_panels` | 149 | `254` | `{32, 2258, 255, 254},` | Authored rectangle height in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_scene3_overlay_uv` | 153 | `8` | `u8 opening_scene3_overlay_uv[8] = {0, 0, 0, 0, 255, 0, 254, 0};` | Eight authored descriptor bytes; the FT4 helper reads offsets 0, 2, 4 and 6. |
| `opening_scene3_overlay_uv` | 153 | `0` | `u8 opening_scene3_overlay_uv[8] = {0, 0, 0, 0, 255, 0, 254, 0};` | Authored texture U origin read by the FT4 helper; retain its numeric crop. |
| `opening_scene3_overlay_uv` | 153 | `0` | `u8 opening_scene3_overlay_uv[8] = {0, 0, 0, 0, 255, 0, 254, 0};` | Authored UV descriptor byte +1; no read in the reviewed FT4 helper establishes a separate meaning. |
| `opening_scene3_overlay_uv` | 153 | `0` | `u8 opening_scene3_overlay_uv[8] = {0, 0, 0, 0, 255, 0, 254, 0};` | Authored texture V origin read by the FT4 helper; retain its numeric crop. |
| `opening_scene3_overlay_uv` | 153 | `0` | `u8 opening_scene3_overlay_uv[8] = {0, 0, 0, 0, 255, 0, 254, 0};` | Authored UV descriptor byte +3; no read in the reviewed FT4 helper establishes a separate meaning. |
| `opening_scene3_overlay_uv` | 153 | `255` | `u8 opening_scene3_overlay_uv[8] = {0, 0, 0, 0, 255, 0, 254, 0};` | Authored texture U span read by the FT4 helper; retain its numeric crop. |
| `opening_scene3_overlay_uv` | 153 | `0` | `u8 opening_scene3_overlay_uv[8] = {0, 0, 0, 0, 255, 0, 254, 0};` | Authored UV descriptor byte +5; no read in the reviewed FT4 helper establishes a separate meaning. |
| `opening_scene3_overlay_uv` | 153 | `254` | `u8 opening_scene3_overlay_uv[8] = {0, 0, 0, 0, 255, 0, 254, 0};` | Authored texture V span read by the FT4 helper; retain its numeric crop. |
| `opening_scene3_overlay_uv` | 153 | `0` | `u8 opening_scene3_overlay_uv[8] = {0, 0, 0, 0, 255, 0, 254, 0};` | Authored UV descriptor byte +7; no read in the reviewed FT4 helper establishes a separate meaning. |
| `opening_scene3_overlay_color` | 156 | `200` | `CVECTOR opening_scene3_overlay_color = {200, 200, 200, 0};` | Authored red color component; preserve the gradient endpoint or material modulation. |
| `opening_scene3_overlay_color` | 156 | `200` | `CVECTOR opening_scene3_overlay_color = {200, 200, 200, 0};` | Authored green color component; preserve the gradient endpoint or material modulation. |
| `opening_scene3_overlay_color` | 156 | `200` | `CVECTOR opening_scene3_overlay_color = {200, 200, 200, 0};` | Authored blue color component; preserve the gradient endpoint or material modulation. |
| `opening_scene3_overlay_color` | 156 | `0` | `CVECTOR opening_scene3_overlay_color = {200, 200, 200, 0};` | Authored zero fourth color byte; the RGB consumer does not use it as opacity. |
| `opening_ending_scroll_backgrounds` | 159 | `2` | `static u16 opening_ending_scroll_backgrounds[2][4] = {` | Two explicitly initialized background rectangles. |
| `opening_ending_scroll_backgrounds` | 159 | `4` | `static u16 opening_ending_scroll_backgrounds[2][4] = {` | Four halfwords per rectangle: X, Y, width, height. |
| `opening_ending_scroll_backgrounds` | 160 | `0` | `{0, 0, KF_DISPLAY_WIDTH, 160}, {0, 160, KF_DISPLAY_WIDTH, 160},` | Authored rectangle X origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_backgrounds` | 160 | `0` | `{0, 0, KF_DISPLAY_WIDTH, 160}, {0, 160, KF_DISPLAY_WIDTH, 160},` | Authored rectangle Y origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_backgrounds` | 160 | `160` | `{0, 0, KF_DISPLAY_WIDTH, 160}, {0, 160, KF_DISPLAY_WIDTH, 160},` | Authored rectangle height in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_backgrounds` | 160 | `0` | `{0, 0, KF_DISPLAY_WIDTH, 160}, {0, 160, KF_DISPLAY_WIDTH, 160},` | Authored rectangle X origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_backgrounds` | 160 | `160` | `{0, 0, KF_DISPLAY_WIDTH, 160}, {0, 160, KF_DISPLAY_WIDTH, 160},` | Authored rectangle Y origin in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_backgrounds` | 160 | `160` | `{0, 0, KF_DISPLAY_WIDTH, 160}, {0, 160, KF_DISPLAY_WIDTH, 160},` | Authored rectangle height in pixels; preserves panel layout, cropping and nonuniform scroll spacing. |
| `opening_ending_scroll_top_start` | 164 | `32` | `static CVECTOR opening_ending_scroll_top_start = {32, 0, 32, 0};` | Authored red color component; preserve the gradient endpoint or material modulation. |
| `opening_ending_scroll_top_start` | 164 | `0` | `static CVECTOR opening_ending_scroll_top_start = {32, 0, 32, 0};` | Authored green color component; preserve the gradient endpoint or material modulation. |
| `opening_ending_scroll_top_start` | 164 | `32` | `static CVECTOR opening_ending_scroll_top_start = {32, 0, 32, 0};` | Authored blue color component; preserve the gradient endpoint or material modulation. |
| `opening_ending_scroll_top_start` | 164 | `0` | `static CVECTOR opening_ending_scroll_top_start = {32, 0, 32, 0};` | Authored zero fourth color byte; the RGB consumer does not use it as opacity. |
| `opening_ending_scroll_bottom_start` | 167 | `255` | `static CVECTOR opening_ending_scroll_bottom_start = {255, 0, 0, 0};` | Authored red color component; preserve the gradient endpoint or material modulation. |
| `opening_ending_scroll_bottom_start` | 167 | `0` | `static CVECTOR opening_ending_scroll_bottom_start = {255, 0, 0, 0};` | Authored green color component; preserve the gradient endpoint or material modulation. |
| `opening_ending_scroll_bottom_start` | 167 | `0` | `static CVECTOR opening_ending_scroll_bottom_start = {255, 0, 0, 0};` | Authored blue color component; preserve the gradient endpoint or material modulation. |
| `opening_ending_scroll_bottom_start` | 167 | `0` | `static CVECTOR opening_ending_scroll_bottom_start = {255, 0, 0, 0};` | Authored zero fourth color byte; the RGB consumer does not use it as opacity. |
| `opening_ending_scroll_top_end` | 170 | `0` | `static CVECTOR opening_ending_scroll_top_end = {0, 0, 32, 0};` | Authored red color component; preserve the gradient endpoint or material modulation. |
| `opening_ending_scroll_top_end` | 170 | `0` | `static CVECTOR opening_ending_scroll_top_end = {0, 0, 32, 0};` | Authored green color component; preserve the gradient endpoint or material modulation. |
| `opening_ending_scroll_top_end` | 170 | `32` | `static CVECTOR opening_ending_scroll_top_end = {0, 0, 32, 0};` | Authored blue color component; preserve the gradient endpoint or material modulation. |
| `opening_ending_scroll_top_end` | 170 | `0` | `static CVECTOR opening_ending_scroll_top_end = {0, 0, 32, 0};` | Authored zero fourth color byte; the RGB consumer does not use it as opacity. |
| `opening_ending_scroll_bottom_end` | 173 | `0` | `static CVECTOR opening_ending_scroll_bottom_end = {0, 0, 32, 0};` | Authored red color component; preserve the gradient endpoint or material modulation. |
| `opening_ending_scroll_bottom_end` | 173 | `0` | `static CVECTOR opening_ending_scroll_bottom_end = {0, 0, 32, 0};` | Authored green color component; preserve the gradient endpoint or material modulation. |
| `opening_ending_scroll_bottom_end` | 173 | `32` | `static CVECTOR opening_ending_scroll_bottom_end = {0, 0, 32, 0};` | Authored blue color component; preserve the gradient endpoint or material modulation. |
| `opening_ending_scroll_bottom_end` | 173 | `0` | `static CVECTOR opening_ending_scroll_bottom_end = {0, 0, 32, 0};` | Authored zero fourth color byte; the RGB consumer does not use it as opacity. |
| `opening_ending_scroll_panel_color` | 176 | `200` | `static CVECTOR opening_ending_scroll_panel_color = {200, 200, 200, 0};` | Authored red color component; preserve the gradient endpoint or material modulation. |
| `opening_ending_scroll_panel_color` | 176 | `200` | `static CVECTOR opening_ending_scroll_panel_color = {200, 200, 200, 0};` | Authored green color component; preserve the gradient endpoint or material modulation. |
| `opening_ending_scroll_panel_color` | 176 | `200` | `static CVECTOR opening_ending_scroll_panel_color = {200, 200, 200, 0};` | Authored blue color component; preserve the gradient endpoint or material modulation. |
| `opening_ending_scroll_panel_color` | 176 | `0` | `static CVECTOR opening_ending_scroll_panel_color = {200, 200, 200, 0};` | Authored zero fourth color byte; the RGB consumer does not use it as opacity. |
| `opening_ending_scroll_background_color` | 179 | `0` | `static CVECTOR opening_ending_scroll_background_color = {0, 0, 0, 0};` | Authored red color component; preserve the gradient endpoint or material modulation. |
| `opening_ending_scroll_background_color` | 179 | `0` | `static CVECTOR opening_ending_scroll_background_color = {0, 0, 0, 0};` | Authored green color component; preserve the gradient endpoint or material modulation. |
| `opening_ending_scroll_background_color` | 179 | `0` | `static CVECTOR opening_ending_scroll_background_color = {0, 0, 0, 0};` | Authored blue color component; preserve the gradient endpoint or material modulation. |
| `opening_ending_scroll_background_color` | 179 | `0` | `static CVECTOR opening_ending_scroll_background_color = {0, 0, 0, 0};` | Authored zero fourth color byte; the RGB consumer does not use it as opacity. |
| `opening_ending_scroll_uv` | 182 | `8` | `static u8 opening_ending_scroll_uv[8] = {0, 0, 1, 0, 255, 0, 254, 0};` | Eight authored descriptor bytes; the FT4 helper reads offsets 0, 2, 4 and 6. |
| `opening_ending_scroll_uv` | 182 | `0` | `static u8 opening_ending_scroll_uv[8] = {0, 0, 1, 0, 255, 0, 254, 0};` | Authored texture U origin read by the FT4 helper; retain its numeric crop. |
| `opening_ending_scroll_uv` | 182 | `0` | `static u8 opening_ending_scroll_uv[8] = {0, 0, 1, 0, 255, 0, 254, 0};` | Authored UV descriptor byte +1; no read in the reviewed FT4 helper establishes a separate meaning. |
| `opening_ending_scroll_uv` | 182 | `1` | `static u8 opening_ending_scroll_uv[8] = {0, 0, 1, 0, 255, 0, 254, 0};` | Authored texture V origin read by the FT4 helper; retain its numeric crop. |
| `opening_ending_scroll_uv` | 182 | `0` | `static u8 opening_ending_scroll_uv[8] = {0, 0, 1, 0, 255, 0, 254, 0};` | Authored UV descriptor byte +3; no read in the reviewed FT4 helper establishes a separate meaning. |
| `opening_ending_scroll_uv` | 182 | `255` | `static u8 opening_ending_scroll_uv[8] = {0, 0, 1, 0, 255, 0, 254, 0};` | Authored texture U span read by the FT4 helper; retain its numeric crop. |
| `opening_ending_scroll_uv` | 182 | `0` | `static u8 opening_ending_scroll_uv[8] = {0, 0, 1, 0, 255, 0, 254, 0};` | Authored UV descriptor byte +5; no read in the reviewed FT4 helper establishes a separate meaning. |
| `opening_ending_scroll_uv` | 182 | `254` | `static u8 opening_ending_scroll_uv[8] = {0, 0, 1, 0, 255, 0, 254, 0};` | Authored texture V span read by the FT4 helper; retain its numeric crop. |
| `opening_ending_scroll_uv` | 182 | `0` | `static u8 opening_ending_scroll_uv[8] = {0, 0, 1, 0, 255, 0, 254, 0};` | Authored UV descriptor byte +7; no read in the reviewed FT4 helper establishes a separate meaning. |
| `opening_scene0_run` | 194 | `0` | `blend = 0;` | Zero blend accumulator at the start of this interpolation or intermediate counter. |
| `opening_scene0_run` | 206 | `0` | `opening_camera_path_step(0);` | No added world-space Y displacement for this camera step. |
| `opening_scene0_run` | 223 | `0` | `if (blend < 0) {` | Signed blend falling below zero ends the scene fade-out. |
| `opening_scene1_draw_fade` | 277 | `0` | `left->x0 = 0;` | Zero x0 coordinate anchors this panel vertex or texture sample at its origin. |
| `opening_scene1_draw_fade` | 278 | `0` | `left->y0 = 0;` | Zero y0 coordinate anchors this panel vertex or texture sample at its origin. |
| `opening_scene1_draw_fade` | 280 | `0` | `left->y1 = 0;` | Zero y1 coordinate anchors this panel vertex or texture sample at its origin. |
| `opening_scene1_draw_fade` | 281 | `0` | `left->x2 = 0;` | Zero x2 coordinate anchors this panel vertex or texture sample at its origin. |
| `opening_scene1_draw_fade` | 287 | `0` | `right->y0 = 0;` | Zero y0 coordinate anchors this panel vertex or texture sample at its origin. |
| `opening_scene1_draw_fade` | 289 | `0` | `right->y1 = 0;` | Zero y1 coordinate anchors this panel vertex or texture sample at its origin. |
| `opening_scene1_draw_fade` | 295 | `0` | `left->u0 = 0;` | Zero u0 coordinate anchors this panel vertex or texture sample at its origin. |
| `opening_scene1_draw_fade` | 296 | `0` | `left->v0 = 0;` | Zero v0 coordinate anchors this panel vertex or texture sample at its origin. |
| `opening_scene1_draw_fade` | 298 | `0` | `left->v1 = 0;` | Zero v1 coordinate anchors this panel vertex or texture sample at its origin. |
| `opening_scene1_draw_fade` | 299 | `0` | `left->u2 = 0;` | Zero u2 coordinate anchors this panel vertex or texture sample at its origin. |
| `opening_scene1_draw_fade` | 304 | `0` | `right->u0 = 0;` | Zero u0 coordinate anchors this panel vertex or texture sample at its origin. |
| `opening_scene1_draw_fade` | 305 | `0` | `right->v0 = 0;` | Zero v0 coordinate anchors this panel vertex or texture sample at its origin. |
| `opening_scene1_draw_fade` | 307 | `0` | `right->v1 = 0;` | Zero v1 coordinate anchors this panel vertex or texture sample at its origin. |
| `opening_scene1_draw_fade` | 308 | `0` | `right->u2 = 0;` | Zero u2 coordinate anchors this panel vertex or texture sample at its origin. |
| `opening_scene1_run` | 333 | `0` | `shade = 0;` | Black starting shade for the panel fade. |
| `opening_scene1_run` | 338 | `1` | `} while (shade < KF_TEXTURE_BASE_BRIGHTNESS + 1);` | One-past bound preserves inclusion of the base-brightness endpoint. |
| `opening_scene1_run` | 340 | `0` | `frame = 0;` | Frame countdown/index origin. |
| `opening_scene1_run` | 345 | `0` | `VSync(0);` | SDK wait-for-next-VBlank mode. |
| `opening_scene1_run` | 358 | `0` | `} while (shade >= 0);` | Zero is the black endpoint of the signed fade countdown. |
| `opening_entity_transition` | 373 | `0` | `initial_scale_y = 0;` | Zero-height cylinder before the grow transition. |
| `opening_entity_transition` | 386 | `1` | `entity_index = TRANSITION_ENTITY_COUNT - 1;` | Last entity index for the decrement-to-minus-one loop. |
| `opening_entity_transition` | 394 | `0` | `entity->rotation.z = 0;` | Zero initial Euler angle before the scripted rotation. |
| `opening_entity_transition` | 395 | `0` | `entity->rotation.y = 0;` | Zero initial Euler angle before the scripted rotation. |
| `opening_entity_transition` | 396 | `0` | `entity->rotation.x = 0;` | Zero initial Euler angle before the scripted rotation. |
| `opening_entity_transition` | 402 | `1` | `} while (entity_index != -1);` | Minus one marks exhaustion after the final index zero. |
| `opening_entity_transition` | 408 | `0` | `frame = 0;` | Frame countdown/index origin. |
| `opening_entity_transition` | 411 | `0` | `entity_index = 0;` | First transition-entity index. |
| `opening_entity_transition` | 416 | `1` | `if (scale_y < TRANSITION_TALL_SCALE_Y + 1) {` | One-past bound includes the tall scale endpoint before adding the signed step. |
| `opening_entity_transition` | 425 | `0` | `opening_render_frame(0, 0);` | Null position asks the transform helper to retain the current view position. |
| `opening_entity_transition` | 425 | `0` | `opening_render_frame(0, 0);` | Null rotation asks the transform helper to retain the current view rotation. |
| `opening_entity_transition` | 426 | `0` | `VSync(0);` | SDK wait-for-next-VBlank mode. |
| `opening_entity_transition` | 436 | `1` | `entity_index = TRANSITION_ENTITY_COUNT - 1;` | Last entity index for the decrement-to-minus-one loop. |
| `opening_entity_transition` | 441 | `1` | `} while (entity_index != -1);` | Minus one marks exhaustion after the final index zero. |
| `opening_scene3_run` | 458 | `0` | `wave_angle = 0;` | Zero phase of the camera sine wave. |
| `opening_scene3_run` | 460 | `0` | `texture_pages[0] = (u16)GetTPage(` | Ordinal of this unrolled panel texture-page entry. |
| `opening_scene3_run` | 463 | `0` | `cluts[0] = (u16)GetClut(0, PANEL_CLUT_FIRST_Y);` | Ordinal of this unrolled panel CLUT entry. |
| `opening_scene3_run` | 463 | `0` | `cluts[0] = (u16)GetClut(0, PANEL_CLUT_FIRST_Y);` | VRAM X origin of the panel CLUT. |
| `opening_scene3_run` | 464 | `1` | `texture_pages[1] = (u16)GetTPage(` | Ordinal of this unrolled panel texture-page entry. |
| `opening_scene3_run` | 467 | `1` | `cluts[1] = (u16)GetClut(0, PANEL_CLUT_FIRST_Y + 1);` | Ordinal of this unrolled panel CLUT entry. |
| `opening_scene3_run` | 467 | `0` | `cluts[1] = (u16)GetClut(0, PANEL_CLUT_FIRST_Y + 1);` | VRAM X origin of the panel CLUT. |
| `opening_scene3_run` | 467 | `1` | `cluts[1] = (u16)GetClut(0, PANEL_CLUT_FIRST_Y + 1);` | Panel ordinal added to the first CLUT row; one row per panel. |
| `opening_scene3_run` | 473 | `0` | `entity_14->rotation.y = 0;` | Zero initial Euler angle before the scripted rotation. |
| `opening_scene3_run` | 474 | `0` | `entity_13->rotation.y = 0;` | Zero initial Euler angle before the scripted rotation. |
| `opening_scene3_run` | 477 | `0` | `blend = 0;` | Zero blend accumulator at the start of this interpolation or intermediate counter. |
| `opening_scene3_run` | 487 | `1` | `if (blend >= KF_FIXED12_ONE + 1) {` | One-past bound keeps the full Q12 endpoint in the fade. |
| `opening_scene3_run` | 519 | `0` | `overlay_index = 0;` | First overlay index. |
| `opening_scene3_run` | 520 | `0` | `overlay_rect = opening_scene3_overlay_rects[0];` | First rectangle begins the contiguous overlay traversal. |
| `opening_scene3_run` | 521 | `1` | `overlay_y = (s16 *)&overlay_rect[1];` | Y component of the four-halfword rectangle, viewed signed while scrolling. |
| `opening_scene3_run` | 534 | `4` | `overlay_y += 4;` | Advance one four-halfword rectangle while retaining the Y-component offset. |
| `opening_scene3_run` | 535 | `4` | `overlay_rect += 4;` | Advance one four-halfword X/Y/width/height rectangle. |
| `opening_scene3_run` | 548 | `0` | `blend = 0;` | Zero blend accumulator at the start of this interpolation or intermediate counter. |
| `opening_scene3_run` | 557 | `1` | `} while (blend < KF_FIXED12_ONE + 1);` | One-past bound keeps the full Q12 endpoint in the fade. |
| `opening_ending_scene_run` | 574 | `0` | `wave_angle = 0;` | Zero phase of the camera sine wave. |
| `opening_ending_scene_run` | 580 | `0` | `entity_14->rotation.y = 0;` | Zero initial Euler angle before the scripted rotation. |
| `opening_ending_scene_run` | 581 | `0` | `entity_13->rotation.y = 0;` | Zero initial Euler angle before the scripted rotation. |
| `opening_ending_scene_run` | 587 | `1` | `SetDispMask(1);` | Boolean display enable passed to the SDK. |
| `opening_ending_scene_run` | 588 | `0` | `blend = 0;` | Zero blend accumulator at the start of this interpolation or intermediate counter. |
| `opening_ending_scene_run` | 599 | `1` | `} while (blend < KF_FIXED12_ONE + 1);` | One-past bound keeps the full Q12 endpoint in the fade. |
| `opening_ending_scene_run` | 602 | `0` | `brightness = 0;` | Black starting value or lower clamp of the brightness ramp. |
| `opening_ending_scene_run` | 603 | `0` | `blend = 0;` | Zero blend accumulator at the start of this interpolation or intermediate counter. |
| `opening_ending_scene_run` | 622 | `0` | `&open_graphics_runtime.display_draw_environments[0],` | Fixed double-buffer draw-environment index; both buffers receive the same brightness. |
| `opening_ending_scene_run` | 627 | `1` | `&open_graphics_runtime.display_draw_environments[1],` | Fixed double-buffer draw-environment index; both buffers receive the same brightness. |
| `opening_ending_scene_run` | 636 | `0xfff` | `if (blend < 0xfff) {` | Guard of an intermediate counter with no value consumer before reset; its original purpose remains unresolved. |
| `opening_ending_scene_run` | 637 | `0x40` | `blend += 0x40;` | Measured increment of the otherwise unconsumed intermediate counter; not proven to drive visible brightness. |
| `opening_ending_scene_run` | 639 | `0xfff` | `blend = 0xfff;` | Measured clamp of the otherwise unconsumed intermediate counter; not an angle mask. |
| `opening_ending_scene_run` | 651 | `0` | `blend = 0;` | Zero blend accumulator at the start of this interpolation or intermediate counter. |
| `opening_ending_scene_run` | 653 | `0` | `if (brightness < 0) {` | Lower-bound test against black before the signed darkening ramp is displayed. |
| `opening_ending_scene_run` | 654 | `0` | `brightness = 0;` | Black starting value or lower clamp of the brightness ramp. |
| `opening_ending_scene_run` | 659 | `0` | `&open_graphics_runtime.display_draw_environments[0],` | Fixed double-buffer draw-environment index; both buffers receive the same brightness. |
| `opening_ending_scene_run` | 662 | `1` | `&open_graphics_runtime.display_draw_environments[1],` | Fixed double-buffer draw-environment index; both buffers receive the same brightness. |
| `opening_ending_scene_run` | 672 | `1` | `} while (blend < KF_FIXED12_ONE + 1);` | One-past bound keeps the full Q12 endpoint in the fade. |
| `opening_ending_scroll_run` | 680 | `0` | `{{0, 0, -4095}, {4095, 0, -2048}, {-4095, 0, -2048}}, {0, 0, 0}` | Authored Q12 light-matrix coefficient row 0, column 0; preserve 4095 rather than normalize it to 4096. |
| `opening_ending_scroll_run` | 680 | `0` | `{{0, 0, -4095}, {4095, 0, -2048}, {-4095, 0, -2048}}, {0, 0, 0}` | Authored Q12 light-matrix coefficient row 0, column 1; preserve 4095 rather than normalize it to 4096. |
| `opening_ending_scroll_run` | 680 | `4095` | `{{0, 0, -4095}, {4095, 0, -2048}, {-4095, 0, -2048}}, {0, 0, 0}` | Authored Q12 light-matrix coefficient row 0, column 2; preserve 4095 rather than normalize it to 4096. |
| `opening_ending_scroll_run` | 680 | `4095` | `{{0, 0, -4095}, {4095, 0, -2048}, {-4095, 0, -2048}}, {0, 0, 0}` | Authored Q12 light-matrix coefficient row 1, column 0; preserve 4095 rather than normalize it to 4096. |
| `opening_ending_scroll_run` | 680 | `0` | `{{0, 0, -4095}, {4095, 0, -2048}, {-4095, 0, -2048}}, {0, 0, 0}` | Authored Q12 light-matrix coefficient row 1, column 1; preserve 4095 rather than normalize it to 4096. |
| `opening_ending_scroll_run` | 680 | `2048` | `{{0, 0, -4095}, {4095, 0, -2048}, {-4095, 0, -2048}}, {0, 0, 0}` | Authored Q12 light-matrix coefficient row 1, column 2; preserve 4095 rather than normalize it to 4096. |
| `opening_ending_scroll_run` | 680 | `4095` | `{{0, 0, -4095}, {4095, 0, -2048}, {-4095, 0, -2048}}, {0, 0, 0}` | Authored Q12 light-matrix coefficient row 2, column 0; preserve 4095 rather than normalize it to 4096. |
| `opening_ending_scroll_run` | 680 | `0` | `{{0, 0, -4095}, {4095, 0, -2048}, {-4095, 0, -2048}}, {0, 0, 0}` | Authored Q12 light-matrix coefficient row 2, column 1; preserve 4095 rather than normalize it to 4096. |
| `opening_ending_scroll_run` | 680 | `2048` | `{{0, 0, -4095}, {4095, 0, -2048}, {-4095, 0, -2048}}, {0, 0, 0}` | Authored Q12 light-matrix coefficient row 2, column 2; preserve 4095 rather than normalize it to 4096. |
| `opening_ending_scroll_run` | 680 | `0` | `{{0, 0, -4095}, {4095, 0, -2048}, {-4095, 0, -2048}}, {0, 0, 0}` | Zero light-matrix translation X. |
| `opening_ending_scroll_run` | 680 | `0` | `{{0, 0, -4095}, {4095, 0, -2048}, {-4095, 0, -2048}}, {0, 0, 0}` | Zero light-matrix translation Y. |
| `opening_ending_scroll_run` | 680 | `0` | `{{0, 0, -4095}, {4095, 0, -2048}, {-4095, 0, -2048}}, {0, 0, 0}` | Zero light-matrix translation Z. |
| `opening_ending_scroll_run` | 700 | `0` | `lighting_blend = 0;` | Zero starts the current lighting interpolation. |
| `opening_ending_scroll_run` | 702 | `0` | `texture_pages[0] = GetTPage(` | Ordinal of this unrolled panel texture-page entry. |
| `opening_ending_scroll_run` | 705 | `0` | `cluts[0] = GetClut(0, PANEL_CLUT_FIRST_Y);` | Ordinal of this unrolled panel CLUT entry. |
| `opening_ending_scroll_run` | 705 | `0` | `cluts[0] = GetClut(0, PANEL_CLUT_FIRST_Y);` | VRAM X origin of the panel CLUT. |
| `opening_ending_scroll_run` | 706 | `1` | `texture_pages[1] = GetTPage(` | Ordinal of this unrolled panel texture-page entry. |
| `opening_ending_scroll_run` | 709 | `1` | `cluts[1] = GetClut(0, PANEL_CLUT_FIRST_Y + 1);` | Ordinal of this unrolled panel CLUT entry. |
| `opening_ending_scroll_run` | 709 | `0` | `cluts[1] = GetClut(0, PANEL_CLUT_FIRST_Y + 1);` | VRAM X origin of the panel CLUT. |
| `opening_ending_scroll_run` | 709 | `1` | `cluts[1] = GetClut(0, PANEL_CLUT_FIRST_Y + 1);` | Panel ordinal added to the first CLUT row; one row per panel. |
| `opening_ending_scroll_run` | 710 | `2` | `texture_pages[2] = GetTPage(` | Ordinal of this unrolled panel texture-page entry. |
| `opening_ending_scroll_run` | 712 | `2` | `PANEL_TPAGE_FIRST_X + 2 * PANEL_TPAGE_X_STRIDE, KF_TEXTURE_LOWER_PAGE_Y);` | Panel ordinal multiplied by the named texture-page X stride. |
| `opening_ending_scroll_run` | 713 | `2` | `cluts[2] = GetClut(0, PANEL_CLUT_FIRST_Y + 2);` | Ordinal of this unrolled panel CLUT entry. |
| `opening_ending_scroll_run` | 713 | `0` | `cluts[2] = GetClut(0, PANEL_CLUT_FIRST_Y + 2);` | VRAM X origin of the panel CLUT. |
| `opening_ending_scroll_run` | 713 | `2` | `cluts[2] = GetClut(0, PANEL_CLUT_FIRST_Y + 2);` | Panel ordinal added to the first CLUT row; one row per panel. |
| `opening_ending_scroll_run` | 714 | `3` | `texture_pages[3] = GetTPage(` | Ordinal of this unrolled panel texture-page entry. |
| `opening_ending_scroll_run` | 716 | `3` | `PANEL_TPAGE_FIRST_X + 3 * PANEL_TPAGE_X_STRIDE, KF_TEXTURE_LOWER_PAGE_Y);` | Panel ordinal multiplied by the named texture-page X stride. |
| `opening_ending_scroll_run` | 717 | `3` | `cluts[3] = GetClut(0, PANEL_CLUT_FIRST_Y + 3);` | Ordinal of this unrolled panel CLUT entry. |
| `opening_ending_scroll_run` | 717 | `0` | `cluts[3] = GetClut(0, PANEL_CLUT_FIRST_Y + 3);` | VRAM X origin of the panel CLUT. |
| `opening_ending_scroll_run` | 717 | `3` | `cluts[3] = GetClut(0, PANEL_CLUT_FIRST_Y + 3);` | Panel ordinal added to the first CLUT row; one row per panel. |
| `opening_ending_scroll_run` | 718 | `4` | `texture_pages[4] = GetTPage(` | Ordinal of this unrolled panel texture-page entry. |
| `opening_ending_scroll_run` | 720 | `4` | `PANEL_TPAGE_FIRST_X + 4 * PANEL_TPAGE_X_STRIDE, KF_TEXTURE_LOWER_PAGE_Y);` | Panel ordinal multiplied by the named texture-page X stride. |
| `opening_ending_scroll_run` | 721 | `4` | `cluts[4] = GetClut(0, PANEL_CLUT_FIRST_Y + 4);` | Ordinal of this unrolled panel CLUT entry. |
| `opening_ending_scroll_run` | 721 | `0` | `cluts[4] = GetClut(0, PANEL_CLUT_FIRST_Y + 4);` | VRAM X origin of the panel CLUT. |
| `opening_ending_scroll_run` | 721 | `4` | `cluts[4] = GetClut(0, PANEL_CLUT_FIRST_Y + 4);` | Panel ordinal added to the first CLUT row; one row per panel. |
| `opening_ending_scroll_run` | 722 | `5` | `texture_pages[5] = GetTPage(` | Ordinal of this unrolled panel texture-page entry. |
| `opening_ending_scroll_run` | 724 | `5` | `PANEL_TPAGE_FIRST_X + 5 * PANEL_TPAGE_X_STRIDE, KF_TEXTURE_LOWER_PAGE_Y);` | Panel ordinal multiplied by the named texture-page X stride. |
| `opening_ending_scroll_run` | 725 | `5` | `cluts[5] = GetClut(0, PANEL_CLUT_FIRST_Y + 5);` | Ordinal of this unrolled panel CLUT entry. |
| `opening_ending_scroll_run` | 725 | `0` | `cluts[5] = GetClut(0, PANEL_CLUT_FIRST_Y + 5);` | VRAM X origin of the panel CLUT. |
| `opening_ending_scroll_run` | 725 | `5` | `cluts[5] = GetClut(0, PANEL_CLUT_FIRST_Y + 5);` | Panel ordinal added to the first CLUT row; one row per panel. |
| `opening_ending_scroll_run` | 726 | `6` | `texture_pages[6] = GetTPage(` | Ordinal of this unrolled panel texture-page entry. |
| `opening_ending_scroll_run` | 728 | `6` | `PANEL_TPAGE_FIRST_X + 6 * PANEL_TPAGE_X_STRIDE, KF_TEXTURE_LOWER_PAGE_Y);` | Panel ordinal multiplied by the named texture-page X stride. |
| `opening_ending_scroll_run` | 729 | `6` | `cluts[6] = GetClut(0, PANEL_CLUT_FIRST_Y + 6);` | Ordinal of this unrolled panel CLUT entry. |
| `opening_ending_scroll_run` | 729 | `0` | `cluts[6] = GetClut(0, PANEL_CLUT_FIRST_Y + 6);` | VRAM X origin of the panel CLUT. |
| `opening_ending_scroll_run` | 729 | `6` | `cluts[6] = GetClut(0, PANEL_CLUT_FIRST_Y + 6);` | Panel ordinal added to the first CLUT row; one row per panel. |
| `opening_ending_scroll_run` | 730 | `7` | `texture_pages[7] = GetTPage(` | Ordinal of this unrolled panel texture-page entry. |
| `opening_ending_scroll_run` | 732 | `7` | `PANEL_TPAGE_FIRST_X + 7 * PANEL_TPAGE_X_STRIDE, KF_TEXTURE_LOWER_PAGE_Y);` | Panel ordinal multiplied by the named texture-page X stride. |
| `opening_ending_scroll_run` | 733 | `7` | `cluts[7] = GetClut(0, PANEL_CLUT_FIRST_Y + 7);` | Ordinal of this unrolled panel CLUT entry. |
| `opening_ending_scroll_run` | 733 | `0` | `cluts[7] = GetClut(0, PANEL_CLUT_FIRST_Y + 7);` | VRAM X origin of the panel CLUT. |
| `opening_ending_scroll_run` | 733 | `7` | `cluts[7] = GetClut(0, PANEL_CLUT_FIRST_Y + 7);` | Panel ordinal added to the first CLUT row; one row per panel. |
| `opening_ending_scroll_run` | 734 | `8` | `texture_pages[8] = GetTPage(` | Ordinal of this unrolled panel texture-page entry. |
| `opening_ending_scroll_run` | 736 | `8` | `PANEL_TPAGE_FIRST_X + 8 * PANEL_TPAGE_X_STRIDE, KF_TEXTURE_LOWER_PAGE_Y);` | Panel ordinal multiplied by the named texture-page X stride. |
| `opening_ending_scroll_run` | 737 | `8` | `cluts[8] = GetClut(0, PANEL_CLUT_FIRST_Y + 8);` | Ordinal of this unrolled panel CLUT entry. |
| `opening_ending_scroll_run` | 737 | `0` | `cluts[8] = GetClut(0, PANEL_CLUT_FIRST_Y + 8);` | VRAM X origin of the panel CLUT. |
| `opening_ending_scroll_run` | 737 | `8` | `cluts[8] = GetClut(0, PANEL_CLUT_FIRST_Y + 8);` | Panel ordinal added to the first CLUT row; one row per panel. |
| `opening_ending_scroll_run` | 747 | `0` | `background_blend = 0;` | Zero starts the background gradient interpolation. |
| `opening_ending_scroll_run` | 749 | `0` | `scrolling = 0;` | False scroll-enable flag before the disk reaches its final Y. |
| `opening_ending_scroll_run` | 750 | `0` | `scroll_phase = 0;` | Start position of the numeric 0,3,2,1 cycle. |
| `opening_ending_scroll_run` | 752 | `0` | `SetBackColor(0, 0, 0);` | Black red component passed to the SDK. |
| `opening_ending_scroll_run` | 752 | `0` | `SetBackColor(0, 0, 0);` | Black green component passed to the SDK. |
| `opening_ending_scroll_run` | 752 | `0` | `SetBackColor(0, 0, 0);` | Black blue component passed to the SDK. |
| `opening_ending_scroll_run` | 753 | `0` | `SetFarColor(0, 0, 0);` | Black red component passed to the SDK. |
| `opening_ending_scroll_run` | 753 | `0` | `SetFarColor(0, 0, 0);` | Black green component passed to the SDK. |
| `opening_ending_scroll_run` | 753 | `0` | `SetFarColor(0, 0, 0);` | Black blue component passed to the SDK. |
| `opening_ending_scroll_run` | 759 | `0` | `open_graphics_runtime.floor_item_state.material.color.r = 0;` | Black initial material color component before the scheduled brightening. |
| `opening_ending_scroll_run` | 760 | `0` | `open_graphics_runtime.floor_item_state.material.color.g = 0;` | Black initial material color component before the scheduled brightening. |
| `opening_ending_scroll_run` | 761 | `0` | `open_graphics_runtime.floor_item_state.material.color.b = 0;` | Black initial material color component before the scheduled brightening. |
| `opening_ending_scroll_run` | 775 | `0` | `lighting_blend = 0;` | Zero starts the current lighting interpolation. |
| `opening_ending_scroll_run` | 786 | `0` | `lighting_blend = 0;` | Zero starts the current lighting interpolation. |
| `opening_ending_scroll_run` | 792 | `1` | `if (--sequence_delay == -1) {` | Countdown expires at minus one: the initial 20 produces 21 pre-decrements. |
| `opening_ending_scroll_run` | 801 | `0` | `if (sequence_volume == 0) {` | Mute endpoint triggers replacement of the sequence. |
| `opening_ending_scroll_run` | 808 | `0` | `opening_camera_path_step(0);` | No added world-space Y displacement for this camera step. |
| `opening_ending_scroll_run` | 812 | `0` | `} else if (scrolling == 0) {` | False scroll-enable flag prevents repeating the disk/starfield visibility swap. |
| `opening_ending_scroll_run` | 814 | `1` | `scrolling = 1;` | True scroll-enable flag after the disk reaches its final Y. |
| `opening_ending_scroll_run` | 831 | `0` | `sprite_add_g4(opening_ending_scroll_backgrounds[0],` | First authored background rectangle receives the vertical gradient. |
| `opening_ending_scroll_run` | 833 | `1` | `sprite_add_f4(opening_ending_scroll_backgrounds[1],` | Second authored background rectangle receives the uniform color. |
| `opening_ending_scroll_run` | 836 | `0` | `if (scroll_phase == 0) {` | Start position of the 0,3,2,1 four-step cycle; starfield roll/color update once per cycle. |
| `opening_ending_scroll_run` | 837 | `1` | `entity_27->rotation.z = (entity_27->rotation.z - 1) & KF_ANGLE_WRAP_MASK;` | Unit decrement is one of 4096 angle units per selected update (0.087890625 degrees); the cycle selects one update in four. |
| `opening_ending_scroll_run` | 847 | `0` | `if (scrolling > 0) {` | Positive boolean scroll-enable value. |
| `opening_ending_scroll_run` | 848 | `0` | `panel_index = 0;` | First ending-panel index. |
| `opening_ending_scroll_run` | 849 | `0` | `panel = opening_ending_scroll_panels[0];` | First rectangle begins the contiguous ending-panel traversal. |
| `opening_ending_scroll_run` | 851 | `0` | `if (scroll_phase == 0 \|\| scroll_phase == 2) {` | Cycle start selects the first of two panel updates per four-step cycle. |
| `opening_ending_scroll_run` | 851 | `2` | `if (scroll_phase == 0 \|\| scroll_phase == 2) {` | Cycle midpoint selects the second of two panel updates per four-step cycle. |
| `opening_ending_scroll_run` | 852 | `1` | `if ((s16)opening_ending_scroll_panels[ENDING_PANEL_COUNT - 1][1] >` | Last panel index selects the stopping reference for the entire scroll. |
| `opening_ending_scroll_run` | 852 | `1` | `if ((s16)opening_ending_scroll_panels[ENDING_PANEL_COUNT - 1][1] >` | Y component of the last rectangle, compared as signed screen coordinates. |
| `opening_ending_scroll_run` | 854 | `1` | `--panel[1];` | Unit decrement of the rectangle Y component moves the panel up one pixel. |
| `opening_ending_scroll_run` | 860 | `1` | `if ((u16)(panel[1] + PANEL_CLIP_Y_BIAS) < PANEL_CLIP_SPAN) {` | Rectangle Y component in the existing biased unsigned clipping predicate. |
| `opening_ending_scroll_run` | 866 | `4` | `panel += 4;` | Advance one four-halfword X/Y/width/height rectangle. |
| `opening_ending_scroll_run` | 868 | `1` | `if (--scroll_phase == -1) {` | Minus one triggers wrap after cycle position zero. |
| `opening_ending_scroll_run` | 869 | `1` | `scroll_phase = ENDING_SCROLL_PHASE_COUNT - 1;` | Last cycle position restores the 0,3,2,1 countdown. |
