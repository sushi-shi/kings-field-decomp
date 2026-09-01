# Map objects and copy regions

These GAME inventory passes cover the setup and runtime helper blocks at
`0x800308c0..0x80031cc8`. The names describe behavior established by retail
MIPS, callers, data references, and loop bounds; they do not claim original
FromSoftware spellings.

## Storage layout

| Address | Extent | Identity | Evidence |
| --- | ---: | --- | --- |
| `0x800561b0` | `0x18` | `map_copy_regions[4]` | four six-byte source/destination rectangle descriptors |
| `0x8006e8e0` | `0x500` | `map_object_definitions[160]` | destination of one `0x140`-word resource copy; every runtime consumer indexes eight-byte records by object ID |
| `0x8006ede0` | `0x20a8` | `map_object_pool[190]` | clear/load loops use 190 records at a `0x2c` stride |
| `0x80070e92` | `0x02` | `map_object_effect_sequence_160` | wrapping allocation sequence for reserved slots 160..169 |
| `0x80070e94` | `0x02` | `map_object_effect_sequence_170` | wrapping allocation sequence for reserved slots 170..179 |
| `0x80070e96` | `0x02` | `map_object_effect_sequence_180` | wrapping allocation sequence for reserved slots 180..189 |

The four `KfMapCopyRegion` records have byte fields for source x/z,
destination x/z, width, and height. `map_apply_copy_region` copies the selected
rectangle across five parallel map layers, each addressed with a 100-column
stride. Region ID `0xff` disables the operation. Bytes inside this table had
previously produced short false-positive strings; the exact table boundary and
six-byte indexing supersede them.

`map_object_definitions` is now typed as 160 eight-byte records. Independent
consumers form `object_id * 8`, and the `0x500`-byte loader extent equals
`160 * 8`. The reviewed fields are a behavior type and unsigned collision and
interaction radii at offsets zero, two, and four. The last two bytes remain
unknown, as does the array's source linkage. `map_object_pool` is shared by
initialization, collision, and runtime update code and is recorded as a global
array. Its `190 * 0x2c` extent ends exactly at `0x80070e88`.

Serialized actor and map-object placement records use proven `0x10` and `0x14`
strides. They remain opaque `KfActorPlacement` and `KfMapObjectPlacement`
types until individual fields are corroborated by the resource producer or
additional consumers. The partial live `KfMapObject` layout records only the
object ID, cell and world coordinates, rotation, link byte, effect allocation
sequence, vertical velocity, action, and action timer used by the reviewed
blocks; unknown bytes remain explicit.

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
- `map_object_pool_clear`, `map_object_definitions_load`, and
  `map_object_pool_load` initialize the 190 live objects and their definitions.

`map_object_pool_load` reads `0x14`-byte placement records, writes `0x2c`-byte
live records, handles special object types, and marks collision edges.

The following runtime block adds point-distance and pool searches, idle action
initialization, reserved effect-slot allocation, actor-debris/effect spawning,
and linked-object trigger/clear operations. The three halfword sequences are
reset by `map_object_pool_clear`; spawners increment one sequence per ten-slot
band, and allocation chooses a free slot or the oldest wrapping sequence. Their
effects are supported, but their source linkage remains `unknown` rather than
guessing file-local `static` from the executable alone.

Per-function CFG, call, signature, initialized-byte, Ghidra, and
vendor-negative evidence is recorded in
`config/evidence/game_semantic_map_objects.tsv` and
`config/evidence/game_semantic_map_runtime.tsv`.

## Next questions

- Name the final two bytes of each map-object definition from additional
  consumers.
- Name remaining live-object fields from movement, interaction, and render
  consumers.
- Resolve action and behavior-type numeric values and the special-object switch
  table near `0x80012738`.
- Recover translation-unit boundaries before deciding whether the map-copy
  table, definition array, and effect sequences had external or file-local
  linkage.
