# Codec type consistency

The Rust resource codecs had accumulated separate vector declarations and
record fields that lagged behind the established C models. This audit covers
the library modules, their host-driver consumers and integration tests.
It reconciles known field types and grouping while preserving every encoded
bit. It does not identify the remaining unknown C fields or prove additional
retail function matches.

## Shared values and storage boundaries

[`math`](../../tools/kf-codec/src/math.rs) owns the shared vector values:

| Type | Meaning and encoded extent |
| --- | --- |
| `Vec3s` | Game `KfVec3s`: three signed halfwords, six bytes, no SDK pad. |
| `Vec3i` | Three signed world coordinates; excludes the SDK VECTOR pad word. |
| `EulerAngles` | Three signed game angle components, with 4096 units per turn. |
| `SVector` | SDK SVECTOR: three signed halfwords plus the preserved pad halfword, eight bytes. |

Weapon rotations and animation vertices use the same `SVector` representation;
`animation::Vertex` names that type for animation consumers. Weapon translation
and actor attachment offsets share `Vec3s`. Explicit little-endian conversion
methods define encoded vectors; Rust field layout and `size_of` do not define
the file format.

The three-byte game `SoundRef` belongs to
[`audio`](../../tools/kf-codec/src/audio.rs), shared by magic and actor
definitions. Named `program`, `tone` and `note` bytes replace anonymous sound
byte sequences without narrowing their accepted values.

## Known record models

| Record family | Established source model used by the codec |
| --- | --- |
| Weapon, armor, magic and level growth | [Equipment](../../include/kf/game/equipment.h), [magic](../../include/kf/game/magic.h) and [player-stat](../../include/kf/game/player.h) field widths and names; unknown ranges remain encoded verbatim. |
| Actor definitions | [KfActorDefinition](../../include/kf/game/actor.h): action parameters, sound references, signed attachment offsets and special-attack values, animation tables, combat values and the unknown +0x38 range. The former opaque whole-record array becomes an explicit decoded record. |
| Map events | [KfMapEventDefinition](../../include/kf/lib/map.h): character/model identifiers, dialogue-page limits and stage limit replace generic kind/variant/tag/image-limit names. |
| Menu text | [MenuPoint, MenuGlyphRow and MenuGlyphString](../../include/kf/game/menu.h): signed screen coordinates and shared glyph-row grouping. |
| Animation assets and cache | [KfAssetHeader](../../include/kf/game/asset.h) and [KfPoolRecord](../../include/kf/game/pool.h): clip counts, clip indices, cached vertices and owner-slot identity. |

Decoded resource identifiers and flags retain their full encoded integer
domain. A known enum vocabulary does not justify rejecting other bit patterns
in a lossless codec. Likewise, encoded PlayStation pointers remain 32-bit
addresses, not host references that could be dereferenced by Rust.
Indexed collections such as combat components, animation slots, glyph codes
and dialogue-page limits remain fixed arrays within their owning types.

## Deliberately retained byte interfaces

Borrowed archive, image, audio and save views retain resource slices. Runtime
loaders and save/world-state transformations accept caller-owned byte buffers
because retail writes selected fields and preserves the rest. Replacing those
destinations with newly initialized Rust structs would discard the seeded
bytes used to verify partial writes. This differs from exposing an entirely
opaque decoded actor definition after its constituent fields are known.

The library remains `no_std`, allocation-free and `forbid(unsafe_code)`.
The host driver exchanges explicit length-prefixed byte blocks with Python;
Rust field/module names are not part of that transport. This workspace has
no other Rust crate consumers and is not published. Named-field API changes
are reflected in the in-tree driver and tests.

## Module verdicts

All eighteen exported library modules were reviewed, including the newly
added `math` module. `lib.rs` additionally exports that module; the host-driver
consumers and tests follow the changed names.

| Module | Verdict |
| --- | --- |
| `math` | Added shared vector values and explicit six/eight-byte conversions. |
| `animation` | Unified SDK vectors; reconciled cache/clip fields; documented encoded PSX pointer types. |
| `asset_archive` | Renamed signed animation count; borrowed archive bounds and offset types retained. |
| `audio` | Added shared SoundRef; inspected existing VAB/SEQ signedness and little/big-endian boundaries. |
| `placements` | Shared vectors and grouped dialogue limits; exact partial runtime writes retained. |
| `records` | Typed actor/action/sound records, shared vectors and weapon mirroring; reconciled equipment/growth fields. |
| `stat` | Signed grouped menu points and glyph rows, shop-indexed prices; corrected CD-file layout documentation. |
| `chunked` | Byte containers and host slice-size bounds retained. |
| `common` | Loader orchestration and caller-owned destination buffers retained. |
| `game_data` | Named COM/MIX/grid borrowed views and raw transport boundaries retained. |
| `map_resources` | Typed load events/payloads and raw grid destinations retained. |
| `registry` | Encoded u32 PSX addresses and partial pointer-table writes retained. |
| `save` | Explicit byte-region views, preserved ranges and encoded scalar domains retained. |
| `world_persist` | Variable-length byte persistence, tail preservation and overflow checks retained. |
| `world_state` | Count/offset domains and partial writes to caller-owned destinations retained. |
| `audio_vab_state` | Typed service calls, u32 PSX addresses and signed SDK bank IDs retained; private Sony regions remain bytes. |
| `tim` | Signed SDK rectangle values, u32 encoded pointers and borrowed pixel data retained. |
| `tmd` | Variable GPU-packet traversal, retail low-halfword count/index operations and preserved packet bytes retained. |

## Isolated resource comparison setup

The full comparison exposed a stale harness setting: native COMMON globals
in the equipment, map-object and save modules require explicit object binding.
The resource, outer-map and save-writing oracles select the existing
`bind_data_objects` mode for their candidate fixtures.
Curated object identities determine fixture addresses;
the shared linker retains its validation and strict default. COMMON bindings
do not certify equality between compiler and curated allocation extents.
This verifies isolated parser behavior and makes no assertion about native
section placement or executable relink closure. A local integration control
covers all seventeen authored/synthetic fixed-record loader comparisons,
including the native weapon COMMON reference that previously failed to link.
The STAT fixture also follows the current `menu_assets` identity, with an
unaligned-source integration control covering its banks, file records and
ordered directory-search paths.
Both outer loaders address the cursor through its current `memory_arena`
owner at +8. The outer-map integration control exercises floor five, including
native COMMON grids, sparse inputs and the external map-variant path.
The map fixture also requires identical retail and candidate path initializers
and uses them directly, preserving the guard against overwriting linked patches.

## Verification

The Rust tests pass (102 passed; five optional integration tests ignored).
All ten comparison suites were run directly with local retail inputs and
fresh candidate objects, retaining their documented SDK/service boundaries:

| Suite | Passing coverage |
| --- | --- |
| TMD | 250 payloads; complete mutated bytes. |
| Resources | 461 cases across fixed records, TIMs, placements, archives, STAT and the COM outer loader. |
| Map resources | All five floor outer walks. |
| Animation | 1,136 record, cache, vertex, lifecycle and static-pointer cases. |
| Audio | Five VAB banks and nine SEQ files, including 15,880 events and 5,154 running-status events. |
| VAB state | 16 success/failure/fade controls. |
| Save parsing | 15 deterministic cases. |
| Save writing | 14 deterministic cases, including short writes and failures. |
| World state | 15 deterministic cases. |
| World persistence | Five deterministic cases. |

The full `kf build`, 827 local Python tests (no skips), `ruff check scripts
tests`, Rust formatting and all-target checks, release driver build,
`git diff --check` and `nix flake check -L` pass. Flake sandbox tests skip
local-retail-dependent controls; the local run above exercises them.
