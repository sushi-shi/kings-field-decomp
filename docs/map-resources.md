# Map and common-resource loading

This GAME campaign follows the CD-file wrappers, TIM upload loop, mutable map
path, and the MIX/CHR archive call chain. Eleven functions now have reviewed
identities; six bounded helpers are strict 100% C matches under
`probe-gcc257-o2-g0`. The compiler/profile remains a productive probe rather
than historical attribution.

## Exact helpers

| Address | Identity | Operation |
| --- | --- | --- |
| `0x8001b100` | `tim_upload_images` | Iterates `TIM_IMAGE` records and uploads each present CLUT and pixel image through the Release 2.5 LIBGPU interface. |
| `0x8001b390` | `map_resource_path_set_floor` | Writes the ASCII floor digit at byte one of the mutable `B0\\` path. |
| `0x8001b3a4` | `map_resource_load_file` | Appends a filename at byte three and returns the CD loader's allocated buffer. |
| `0x8001b3e4` | `map_resource_copy_words` | Copies aligned words and returns the advanced source cursor used by five chained grid loads. |
| `0x8001b414` | `map_variant_assets_load` | Builds `CHR0.MIM`, patches its variant digit, loads it into the persistent buffer, and registers its TMD archive. |
| `0x8001b49c` | `audio_play_current_map_sequence` | Selects sequence zero or one from floor, level, and map variant through a sparse switch. |

The TIM wrapper uses the authentic `TIM_IMAGE`, `OpenTIM`, `ReadTIM`,
`LoadImage`, and `DrawSync` declarations from Psy-Q Release 2.5. Those linked
services remain vendored; only the game-owned iteration policy counts as
decompilation progress. `map_resource_copy_words` is not labelled `memcpy`:
its only five calls form one archive cursor chain and its return value is the
advanced source pointer.

## Inventory-only orchestration

The larger `cd_file_load_allocated`, `cd_file_load_into`,
`common_resources_load`, and `map_resources_load` bodies retain reviewed
names, complete signatures, call/data evidence, and CFG dossiers but are not
claimed as reconstructed. `asset_registry_load_tmd_archive` is similarly
labelled from its counted length-prefixed loop, consecutive asset IDs, and
calls to `asset_registry_select` and `tmd_prepare_primitive_indices`.

## Data and structure ownership

| Address | Identity | Type | Evidence |
| --- | --- | --- | --- |
| `0x80055af0` | `map_resource_path` | `char[12]` | Raw prefix `B0\\`; floor digit at `+1`; nine-byte filename including NUL at `+3`. |
| `0x8009ddac` | `map_variant_asset_buffer` | `u8 *` | Receives the `0x5a000` allocation and is reloaded for CHR file input and TMD registration. |
| `0x800a078c` | `KfPlayerState.player_map_variant` | `u8` at `+0x0c` | Transition setters, floor-five tests, and the `CHR0.MIM` filename digit all use byte accesses. |

The mutable path is an array, not a structure. The five equal `0x2710` grid
loads also remain separate typed arrays: common extent and load order do not
prove an enclosing aggregate. Conversely, `player_map_variant` is a field of
the already proved complete `0xe0`-byte `KfPlayerState`, so its exact offset,
extent, and datatype are checked in both structure TSVs and the C header.

Per-function evidence and the remaining source/TU limits are recorded in
`config/evidence/game_semantic_map_resources.tsv`.
