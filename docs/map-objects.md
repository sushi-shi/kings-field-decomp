# Map objects and copy regions

This GAME inventory pass covers the setup block at
`0x800308c0..0x80031450`. The names describe behavior established by retail
MIPS, callers, data references, and loop bounds; they do not claim original
FromSoftware spellings.

## Storage layout

| Address | Extent | Identity | Evidence |
| --- | ---: | --- | --- |
| `0x800561b0` | `0x18` | `map_copy_regions[4]` | four six-byte source/destination rectangle descriptors |
| `0x8006e8e0` | `0x500` | `map_object_metadata` | destination of one `0x140`-word resource copy and indexed by object ID |
| `0x8006ede0` | `0x20a8` | `map_object_pool[190]` | clear/load loops use 190 records at a `0x2c` stride |

The four `KfMapCopyRegion` records have byte fields for source x/z,
destination x/z, width, and height. `map_apply_copy_region` copies the selected
rectangle across five parallel map layers, each addressed with a 100-column
stride. Region ID `0xff` disables the operation. Bytes inside this table had
previously produced short false-positive strings; the exact table boundary and
six-byte indexing supersede them.

`map_object_metadata` is intentionally typed as `u8[0x500]`. Consumers expose
several indexed fields and object-type cases, but the internal record size has
not yet been proved. Its linkage remains `unknown`. `map_object_pool` is shared
by initialization, collision, and runtime update code and is recorded as a
global array. Its `190 * 0x2c` extent ends exactly at `0x80070e88`.

Serialized actor and map-object placement records use proven `0x10` and `0x14`
strides. They remain opaque `KfActorPlacement` and `KfMapObjectPlacement`
types until individual fields are corroborated by the resource producer or
additional consumers. The partial live `KfMapObject` layout records only the
object ID, cell and world coordinates, yaw, and state fields used by this
reviewed block; unknown bytes remain explicit.

## Reviewed operations

- `actor_pool_load_placements` consumes up to 128 actor placement records and
  initializes the existing `KfActor[128]` pool.
- `actor_definitions_load` copies exactly `0x720` bytes into the twelve-record
  actor-definition array.
- `map_apply_copy_region` applies one initialized rectangle descriptor to five
  map layers.
- `map_object_mark_collision_edge` marks the current or neighboring collision
  cell according to object metadata and cardinal yaw.
- `map_object_probe_forward` probes the cardinal cell in front of an object.
- `map_object_pool_clear`, `map_object_metadata_load`, and
  `map_object_pool_load` initialize the 190 live objects and their metadata.

`map_object_pool_load` reads `0x14`-byte placement records, writes `0x2c`-byte
live records, handles special object types, and marks collision edges. Three
adjacent halfword counters reset by `map_object_pool_clear` remain unnamed;
their ownership and individual meanings need runtime-family evidence.

Per-function CFG, call, signature, initialized-byte, Ghidra, and
vendor-negative evidence is recorded in
`config/evidence/game_semantic_map_objects.tsv`.

## Next questions

- Recover the metadata record structure instead of projecting isolated fields
  into separate globals.
- Name remaining live-object fields from movement, interaction, and render
  consumers.
- Resolve the three counters following the pool and the special-object switch
  table near `0x80012738`.
- Recover translation-unit boundaries before deciding whether the map-copy
  table and metadata blob had external or file-local linkage.
