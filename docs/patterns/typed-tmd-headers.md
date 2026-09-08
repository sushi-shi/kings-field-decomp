# Typed selected TMD headers

## Function Match Plan

Baseline `bd747d7`. Both overlays select pointers to the same 12-byte TMD
header: preparation reads object_count at +8, object lookup addresses the
28-byte object table at +12, and packet/vector offsets are relative to that
header boundary. The GAME registry selects the same payload through the
asset's serialized TMD offset. Slot stores, selection and all file/menu
callers establish the shared pointer type. Replace byte/void slot and current
asset declarations with `KfTmdHeader *`, and propagate that parameter through
the game registration API.

Typed header reads need no casts. Object lookup advances one complete header
before decoding the object-table record type. Keep explicit byte views for
serialized packet/vector offsets, and explicit header conversions at the
resource, menu-allocation and asset-offset boundaries. These are actual
heterogeneous storage boundaries. Do not hide them in pointer unions or
weaken the registration signature to reduce a cast total.

The header remains three on-disk words; add a 12-byte static size check. No
variable payload extent or object capacity is invented. The slot selectors,
unsigned count narrowing, prepared eight-byte offsets, switches, calls,
return forms, delay slots and ordered references stay unchanged.

Image-specific six-view snapshots, pre-edit objects and source history are
reviewed under `build/cast-model/typed-tmd-headers/`, alongside the existing
TMD/resource/graphics dossiers and adjacent functions. These are game TMD
policies around separately vendored allocation/SDK APIs; no SDK body changes.
The unchanged packet and normal readers are included in the raw unit controls.

| Image / function | VA / bytes | Strict before | Blocks/JAL/returns | Final verdict |
| --- | --- | ---: | --- | --- |
| GAME.EXE `common_resources_load` | `8001b180 / 210` | 100 | 7/10/1 | 100%; raw unchanged |
| GAME.EXE `map_resources_load` | `8001b558 / 258` | 100 | 4/31/1 | 100%; raw unchanged |
| GAME.EXE `tmd_select` | `8001c0e8 / 2c` | 100 | 1/0/1 | 100%; raw unchanged |
| GAME.EXE `tmd_get_object` | `8001c114 / 24` | 100 | 1/0/1 | 100%; raw unchanged |
| GAME.EXE `tmd_prepare_primitive_indices` | `8001c2b0 / 300` | 100 | 18/0/1 | 100%; raw unchanged |
| GAME.EXE `tmd_register` | `8001c5b0 / 3c` | 100 | 1/1/1 | 100%; raw unchanged |
| GAME.EXE `asset_registry_select` | `8002059c / 38` | 100 | 1/0/1 | 100%; raw unchanged |
| GAME.EXE `menu_load_item_model` | `8002aea4 / 68` | 100 | 5/3/1 | 100%; raw unchanged |
| OPEN.EXE `opening_resources_load_scene0` | `80016348 / 1c8` | 100 | 1/18/1 | 100%; raw unchanged |
| OPEN.EXE `opening_resources_load_scene3` | `800165c4 / f0` | 100 | 1/12/1 | 100%; raw unchanged |
| OPEN.EXE `opening_resources_load_ending` | `800166b4 / 134` | 100 | 1/12/1 | 100%; raw unchanged |
| OPEN.EXE `tmd_select` | `80016e68 / 2c` | 100 | 1/0/1 | 100%; raw unchanged |
| OPEN.EXE `tmd_get_object` | `80016e94 / 24` | 100 | 1/0/1 | 100%; raw unchanged |
| OPEN.EXE `tmd_prepare_primitive_indices` | `80017030 / 300` | 100 | 18/0/1 | 100%; raw unchanged |
| OPEN.EXE `tmd_register` | `80017330 / 3c` | 100 | 1/1/1 | 100%; raw unchanged |

Rebuild the affected units and compare all linked words, calls and physical
referents to baseline and retail. Preserve every exact result. Update shared
signatures, field inventories and existing assertions, then run target-C
census, full build, repository tests, Ruff and diff checks. Bank only the
directly edited exact consumers after independent raw verification.


## Result

All 76 compared functions retain every linked word, call and physical
referent. Seventy-two remain exact and four existing partials are unchanged.
All fifteen reviewed functions are 100%; twelve have directly edited bodies
or object-table expressions and are eligible for banking. The selected and
registered pointers now carry the 12-byte header type throughout both images.

Four casts disappear from header/object access, and eight explicit conversions
now mark genuine resource, allocation and serialized-offset boundaries. The
census therefore rises from 714 to 718 written casts; 42 remain in headers,
and C pointer casts total 456 (350 fewer than the original 806). This increase
records the actual decoding boundaries while eliminating the untyped stored
asset and registration interface. No helper or pointer union hides a cast.
All 112 target-C variants parse without errors.

All 713 repository tests pass (nine skips), Ruff and diff checks pass, and
the full build retains 439/471 exact functions, the existing closure gates
and zero artifact failures. No SDK type, table extent or relocation changes.
