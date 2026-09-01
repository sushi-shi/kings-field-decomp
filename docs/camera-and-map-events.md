# Camera paths and map events

This pass identifies two GAME.EXE families in the investigation band
`0x800332e4..0x80033ae4`. They are grouped for review convenience only.
Address adjacency does not prove that the camera-path and map-event functions
belonged to one original translation unit.

## Camera-path controller

`camera_path_begin`, `camera_path_compute_segment`, and `camera_path_step`
operate on a caller-owned `0x64`-byte `KfCameraPathState`. The controlling
caller at `0x800346a8` allocates that state on its stack, begins a path, steps
it while rendering, and copies the final transform back to the live camera.

The state retains a pointer to `0x1c`-byte `KfCameraPathPoint` records. A point
whose x coordinate is `-1` terminates the path. Segment setup computes a
three-axis distance, derives a frame count from the point speed, and prepares
Q4 position and shortest-path 12-bit rotation deltas. The point pointer used by
the reviewed caller is `0x800561d0`, but the surrounding initialized bytes do
not yet prove a safe table boundary or sentinel sequence. It therefore remains
unnamed.

## Map-event pool

`map_event_pool_load` proves eight `0x44`-byte runtime records at
`0x8009db88`. It consumes `0x18`-byte serialized definitions, expands tile
coordinates into 2000-unit world coordinates, initializes render/progress
fields, and writes `0xff` to unused slot states after the source sentinel.
The following word at `0x8009dda8` is a pointer to the currently processed
record, not an interior field of the pool.

`map_event_refresh_image_for_progress` raises a record's image index according
to both its own limit and the highest map reached. `map_event_set_current`
stores the current-record pointer. `map_event_advance_rotation_blocking`
increments the halfword rotation phase at offset `0x12`, renders, and waits for
the frame pacer until the requested target is reached.

The 16-byte setter has four byte-identical Psy-Q FID candidates, including
`GsSetWorkBase`. That short match is ambiguous: the stored word is consumed as
a `KfMapEvent *` by two game update routines and immediately follows the
eight-record event pool. It is retained as game code unless stronger archive
ownership evidence appears.

## Globals and statics

The pass replaces field-sized candidates with five reviewed objects:

| Address | Identity | Type | Evidence |
| --- | --- | --- | --- |
| `0x8009db88` | `map_event_pool` | `KfMapEvent[8]` | fixed eight-record loader and repeated `0x44` strides |
| `0x8009dda8` | `current_map_event` | `KfMapEvent *` | setter plus two typed consumers |
| `0x800a0788` | `map_progress_state` | `KfMapProgressState` | initialization, transition update, and event image clamp |
| `0x800a0824` | `camera_position` | `KfVec4i` | render calls and camera-path copies |
| `0x800a0838` | `camera_rotation` | `KfVec4s` | render calls and camera-path copies |

All five retain `scope=unknown`. Cross-function use proves that each is shared
state, but linked bytes do not distinguish an externally visible global from a
file-local static used by several functions. The separate word at
`0x800a0834` is not absorbed into the camera position or rotation because its
ownership is still unresolved.

## Rejected control-flow candidates

The relocation scan had interpreted words at `0x80035070`, `0x800350a4`,
`0x800350d0`, `0x800350f4`, and `0x80035110` as calls to
`map_event_advance_rotation_blocking`, and the word at `0x800359a4` as a call
to `map_event_set_current`. Raw disassembly shows ordinary non-control-flow
instructions at all six sites. Their relocation rows are preserved with
`status=rejected` so later scans cannot silently promote the same false xrefs.

The per-function evidence and final signature verdicts are in
`config/evidence/game_semantic_camera_events.tsv`.
