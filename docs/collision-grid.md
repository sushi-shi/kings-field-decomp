# Floor height and collision occupancy grids

This GAME campaign follows two direct calls and four shared 100-by-100 map
arrays. The three adjacent helpers form the `game.collision_grid`
reconstruction unit: the direct call chain, shared grid layer, common collision
policy, and common compiler profile support that ownership model. Their address
order alone would not establish an original translation unit.

## Exact functions

| Address | Identity | Observed operation |
| --- | --- | --- |
| `0x8001a29c` | `map_floor_height_for_cell_position` | Converts the cell's height byte to negative world y and adds 300 on the half selected by orientation values 1 through 4 when collision kind is 6. |
| `0x8001a44c` | `map_floor_height_at_position` | Divides Psy-Q `VECTOR.vx/vz` by 2000, forms the 100-wide flat index, and returns the first helper's result. |
| `0x8001a4e8` | `collision_adjust_cell_occupancy` | Adds `+1` or `-1` to the low-five-bit occupancy value over the bounded 5x5 neighborhood while preserving bits `0xe0`. |

All three C functions are strict 100% object matches under
`probe-gcc257-o2-g0`. This remains a productive compiler probe rather than
historical compiler/profile attribution.

The preceding player-update routine and following collision-query dispatcher
remain separate units. In particular, `collision_query_world` is a larger,
entity-oriented dispatcher: it coordinates several actor, object, and event
overlap tests and owns a separate cell-height table. That is a semantic
boundary from these map-grid lookup and occupancy primitives.

Both collision units expose their shared result layout, data, and operations
through `include/kf/game_collision.h`; map-owned layouts and operations remain
in `include/kf/game_map.h`.

The wrapper's initial reconstruction incorrectly truncated the flat cell
index to `s16`. Retail passes the two division results directly, and removing
that unsupported conversion produces the exact instruction stream. The
occupancy call sites load both cell coordinates with `lbu` or `lhu`; the
arguments are therefore `u16`, not `s16`. Its retail loop keeps a persistent
next-row pointer and copies it to the current-byte cursor at the loop head.
Writing that same source lifetime closes the object without fake locals,
volatile state, or assembly.

## Data identities

| Address | Identity | Type | Evidence |
| --- | --- | --- | --- |
| `0x800668e8` | `map_collision_flag_grid` | `u8[100][100]` | The updater changes its low five bits modulo 32 and preserves `0xe0`; `collision_query_world` tests the low-five value for zero and also masks `0xf0` against query flags. |
| `0x80069018` | `map_cell_orientation_grid` | `u8[100][100]` | The map loader copies exactly `0x9c4` words (`0x2710` bytes); floor and render paths interpret values 1 through 4 as orientations. |
| `0x80095900` | `map_floor_height_grid` | `u8[100][100]` | Each byte is multiplied by `-100` to produce the base world floor height. |
| `0x80098018` | `map_collision_grid` | `u8[100][100]` | Kind 6 enables the orientation-dependent 300-unit edge adjustment. |

These arrays are typed data objects, not structures. Their equal extents and
loading sequence do not prove an enclosing C aggregate, so no speculative map
structure is introduced. The only structure touched here is the existing
Psy-Q `VECTOR` (`0x10` bytes: `vx` at `+0x00`, `vz` at `+0x08`) used by the
position wrapper.

All thirteen direct occupancy call instructions are reviewed relocation rows,
including three originally discovered by the instruction-word scan. Their
decoded contexts establish real calls with unsigned coordinate loads and
`+1`/`-1` lifecycle updates. Per-function evidence and remaining naming limits
are recorded in `config/evidence/game_semantic_collision_grid.tsv`.
