# Map-event queries and matrix effects

This pass reviews the GAME.EXE band `0x80033ae4..0x80033ee4` and the matrix
helpers that its call chain resolves. The map-event query and player-effect
families are adjacent only as an investigation band; that is not evidence that
they shared an original translation unit.

## Map-event queries

`map_event_distance_to_point` computes horizontal distance from the event's
reference coordinates, returning `-1` outside a caller-supplied bound.
`map_event_pool_find_target_in_cone` applies it to all eight active records and
selects the smallest wrapped facing error inside the requested cone. Its sole
caller first tries the analogous actor selector and then passes camera
position, camera yaw, a 6000-unit range, and a `0x155` angular tolerance to the
event selector.

`map_event_pool_find_overlap` returns the first active record index whose
radius plus caller padding contains a point. This use, together with the event
loader's copy, names offset `0x14` in `KfMapEventDefinition` and offset `0x20`
in `KfMapEvent` as `radius`.

## Matrix interpolation and restoration effect

`matrix_interpolate` walks the nine signed halfwords of the 3x3 portion of a
PlayStation `MATRIX` and computes a Q12 blend. It leaves the padding and
translation lanes untouched. `lighting_set_color_matrix` and
`lighting_set_light_matrix` use that helper to build a stack matrix before
calling the corresponding Sony GTE API. Their previously opaque integer
parameters are therefore two matrix pointers and a blend.

`color_matrix_set_rgb` fills each 3x3 row with one supplied red, green, or blue
component. Its existing reconstruction remains exact after replacing the
temporary nine-halfword structure with the authentic project `KfMatrix` type.
`lighting_transition_color_matrix` installs five blends from zero through one,
rendering and waiting after each step.

`player_restore_vitals_with_color_cycle` saves the active color matrix,
transitions through green, cyan, and white, restores the saved matrix, restores
current HP and MP from their maxima, and clears the low four bits of an
otherwise unresolved player flag halfword. The caller's higher-level trigger
and those individual flag meanings are not yet named.

## Player vitals global/static

The former scalar candidates at `0x800a0790` and `0x800a0794` are one reviewed
eight-byte `KfPlayerVitals` object:

| Offset | Field | Evidence |
| --- | --- | --- |
| `0x0` | `maximum_hp` | initialization, HP clamp, HUD ratio, save summary |
| `0x2` | `current_hp` | adjustment helper and zero-HP death path |
| `0x4` | `maximum_mp` | initialization, MP clamp, HUD ratio, save summary |
| `0x6` | `current_mp` | adjustment helper and full-restoration effect |

The object retains `scope=unknown`. Its many consumers prove shared storage,
but linked bytes cannot distinguish external linkage from a file-local static
shared within an original translation unit. The neighboring flag word at
`0x800a07aa` stays address-derived until its individual bits are understood.

Per-function evidence and final signature verdicts are recorded in
`config/evidence/game_semantic_event_queries_matrix.tsv`.
