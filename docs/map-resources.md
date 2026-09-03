# Map and common-resource loading

This GAME campaign follows the CD-file wrappers, TIM upload loop, mutable map
path, and the MIX/CHR archive call chain. All eleven reviewed functions are
strict 100% C matches under `probe-gcc257-o2-g0`. The compiler/profile remains
a productive probe rather than historical attribution.

## Exact resource ownership

| Address | Identity | Operation |
| --- | --- | --- |
| `0x8001acf0` | `cd_file_load_allocated` | Builds a versioned disc path and loads the file into a fresh arena allocation. |
| `0x8001af9c` | `cd_file_load_into` | Loads a versioned disc path into caller-owned storage. |
| `0x8001b100` | `tim_upload_images` | Iterates `TIM_IMAGE` records and uploads each present CLUT and pixel image through the Release 2.5 LIBGPU interface. |
| `0x8001b180` | `common_resources_load` | Loads and distributes the common texture and data archives. |
| `0x8001b390` | `map_resource_path_set_floor` | Writes the ASCII floor digit at byte one of the mutable `B0\\` path. |
| `0x8001b3a4` | `map_resource_load_file` | Appends a filename at byte three and returns the CD loader's allocated buffer. |
| `0x8001b3e4` | `map_resource_copy_words` | Copies aligned words and returns the advanced source cursor used by five chained grid loads. |
| `0x8001b414` | `map_variant_assets_load` | Builds `CHR0.MIM`, patches its variant digit, loads it into the persistent buffer, and registers its TMD archive. |
| `0x8001b49c` | `audio_play_current_map_sequence` | Selects sequence zero or one from floor, level, and map variant through a sparse switch. |
| `0x8001b558` | `map_resources_load` | Orchestrates the current floor's texture, audio, grid, entity, event, and TMD archives. |
| `0x800204c0` | `asset_registry_load_tmd_archive` | Traverses a length-prefixed TMD archive and registers consecutive asset IDs. |

The TIM wrapper uses the authentic `TIM_IMAGE`, `OpenTIM`, `ReadTIM`,
`LoadImage`, and `DrawSync` declarations from Psy-Q Release 2.5. Those linked
services remain vendored; only the game-owned iteration policy counts as
decompilation progress. `map_resource_copy_words` is not labelled `memcpy`:
its only five calls form one archive cursor chain and its return value is the
advanced source pointer.

The contiguous `0x8001b100–0x8001b7b0` run is one `game.resources` unit. Its
eight functions share resource formats and calls, compile under one profile,
and own one contiguous 60-byte literal contribution at
`0x80012178–0x800121b4`. The preceding `game.cd_file` unit remains separate:
its three functions share CD retry state and versioned-path data, while the
following error-screen function begins a distinct display subsystem.

## Data and structure ownership

| Address | Identity | Type | Evidence |
| --- | --- | --- | --- |
| `0x80012178` | `game.resources` literals | `.rodata[0x3c]` | `COM\\MIX.TIM`, `COM\\COM.DAT`, `CHR0.MIM`, `MIXA.DAT`, and `MIXB.DAT` compile in retail order and match as one contribution. |
| `0x80055af0` | `map_resource_path` | `char[12]` | Raw prefix `B0\\`; floor digit at `+1`; nine-byte filename including NUL at `+3`. |
| `0x80057b48` | `map_mix_tim_filename` | `char[8]` | Writable `MIX.TIM` filename owned by `game.resources`. |
| `0x8009ddac` | `map_variant_asset_buffer` | `u8 *` | Receives the `0x5a000` allocation and is reloaded for CHR file input and TMD registration. |
| `0x800a078c` | `KfPlayerState.player_map_variant` | `u8` at `+0x0c` | Transition setters, floor-five tests, and the `CHR0.MIM` filename digit all use byte accesses. |

The mutable path is an array, not a structure. The five equal `0x2710` grid
loads also remain separate typed arrays: common extent and load order do not
prove an enclosing aggregate. Conversely, `player_map_variant` is a field of
the already proved complete `0xe0`-byte `KfPlayerState`, so its exact offset,
extent, and datatype are checked in both structure TSVs and the C header.

`CHR0.MIM` is a literal inside the unit's `RODATA` claim, not a standalone
global. Per-function evidence and ownership conclusions are recorded in
`config/evidence/game_semantic_map_resources.tsv`.
