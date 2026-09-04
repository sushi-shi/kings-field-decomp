# Function, global, and static identities

The structural census says where linked code and data exist. The identity
layer records the current source-level interpretation without changing delink
symbols or pretending that WIP names are original symbols:

- `config/retail/function_identities.tsv` has exactly one row for each
  carveable non-vendored function: 1 PSX, 492 GAME, and 241 OPEN rows;
- `config/retail/data_identities.tsv` tracks initialized objects and referenced
  BSS objects that may become C globals, file-local statics, or
  function-local statics;
- `config/retail/structures.tsv` and `structure_fields.tsv` track complete
  target sizes and every field offset/extent, including opaque ranges; and
- address-derived `func_` and `DAT_` names are stable unresolved identities.

Current semantic coverage is 240 of 734 functions: 220 GAME, one PSX, and 19
OPEN identities. The reviewed GAME families now cover the linked lifecycle,
fixed-point math helpers, memory-card/save-file subsystem, full-screen/TALK
image path, and the actor core through targeting, animation, action selection,
awareness, placement loading, map-object runtime, audio control, camera paths,
map-event state, player vitals, and the death transition. Their
per-function evidence is in the
`game_semantic_math_lifecycle.tsv`, `game_semantic_save_system.tsv`,
`game_semantic_screen_talk.tsv`, `game_semantic_actor_core.tsv`,
`game_semantic_actor_ai.tsv`, `game_semantic_actor_actions.tsv`,
`game_semantic_map_objects.tsv`, `game_semantic_map_runtime.tsv`,
`game_semantic_audio_control.tsv`, `game_semantic_audio_spatial.tsv`,
`game_semantic_camera_events.tsv`, `game_semantic_event_queries_matrix.tsv`,
`game_semantic_player_death.tsv`, `game_semantic_player_combat.tsv`,
`game_semantic_player_stats.tsv`, `game_semantic_player_motion_attack.tsv`,
`game_semantic_player_interactions.tsv`, `game_semantic_player_update.tsv`,
`game_semantic_collision_grid.tsv`,
`game_semantic_map_resources.tsv`,
`game_semantic_display_tmd.tsv`, and `game_open_semantic_memory_allocator.tsv` files
under `config/evidence/`. The recovered subsystems and remaining unknowns are
described in [`save-system.md`](save-system.md),
[`screen-images.md`](screen-images.md), [`actor-system.md`](actor-system.md),
[`map-objects.md`](map-objects.md), [`audio-system.md`](audio-system.md),
[`camera-and-map-events.md`](camera-and-map-events.md),
[`event-queries-and-matrix-effects.md`](event-queries-and-matrix-effects.md),
[`player-vitals-and-death.md`](player-vitals-and-death.md),
[`player-combat.md`](player-combat.md),
[`player-stats-and-equipment.md`](player-stats-and-equipment.md),
[`player-motion-and-weapon-attack.md`](player-motion-and-weapon-attack.md),
[`player-interactions-and-collision.md`](player-interactions-and-collision.md),
[`player-update-and-lighting.md`](player-update-and-lighting.md),
[`collision-grid.md`](collision-grid.md),
[`map-resources.md`](map-resources.md),
[`display-and-tmd.md`](display-and-tmd.md),
[`memory-allocator.md`](memory-allocator.md), and
[`structure-layouts.md`](structure-layouts.md).

The current first pass contains 3,439 data identities. The lower row count is
progress: field-sized and pointer-sized seeds are merged when evidence proves
an owning table or structure. 185 data identities now have semantic review.
Eight loaded identities are candidate or
supported file-local statics: the six private GAME/OPEN `LIBGTE/MTX` matrix
stack objects plus the GAME frame pacer's vertical-sync counter and last-tick
state. The save pass adds typed shared pointers for the 0x280-byte header and
0x2580-byte payload workspaces, refines all four BIOS card event handles by
their `KERNEL.H` event specs, and names eight loaded string constants directly
in `data.tsv`. Their external versus file-local linkage remains unresolved
where the executable cannot decide it. The remaining direct-access extents
deliberately retain address-only names.

The actor pass replaces field-level BSS candidates with the owning arrays:
twelve `0x98`-byte `KfActorDefinition` records and 128 `0x48`-byte `KfActor`
records. It also names the actor player-transform context, bound-actor globals,
player snapshots, player-selected actor target, display selector/load buffer,
and the two-record Psy-Q `DRAWENV` array. The mutable TALK path is named
directly in `data.tsv`. The AI pass adds a typed action-profile table;
its identity and owner are supported, while its global-versus-static linkage
remains explicitly unresolved.

The action-handler pass corrects that table to 25 records using its raw-byte
boundary and adjacent direct references. It also types four packed boss-death
phase sounds and one loop sound as three-byte `SoundRef` objects, and names the
shared BSS progression flag set by the scripted death sequence. The initialized
objects retain `scope=unknown`; direct usage proves their role, not whether the
original declarations had internal linkage.

The map-object passes add four six-byte map-copy descriptors and replace 36
field-level BSS candidates with 160 eight-byte `KfMapObjectDefinition` records
and one 190-record `KfMapObject` pool. Placement records are loaded at a `0x14`
stride, while live objects have a proven `0x2c` stride. The runtime pass also
names the wrapping allocation sequences for reserved effect slots 160..169,
170..179, and 180..189. Its dispatcher pass collapses 99 false scalar pointer
identities into one compiler-emitted action jump table and types thirteen
adjacent packed gameplay sound references. The copy descriptors, definitions,
sound references, and counters retain `scope=unknown`; the object pool is
shared by initialization, collision, and runtime update families and is
recorded as global.

The audio passes name nine game-owned lifecycle, listener, spatialization, and
managed-voice functions. They correct the shared VAB/sequence state, settings,
listener position/rotation, loaded voice-ring index, and one complete
`KfAudioVoiceSlots` aggregate with five ten-element lanes. The control pass also
admits the previously missing 344-byte sequence table from its reviewed
HI16/LO16 pair and the Psy-Q 2.5 `SS_SEQ_TABSIZ` contract. These state objects
retain `scope=unknown`; adjacency and shared use do not prove external linkage.

The spatial pass also removes four false game candidates: GAME and OPEN each
contain the same instruction-shape-identical `SsUtKeyOn`/`SsUtKeyOff` VMANAGER
pair between exact Release 2.5 archive anchors. They are recorded as
`sdk-lineage-supported` vendored functions, so version-skewed SDK code is not
counted as game progress merely because its exact Release 2.5 FID missed.

The camera/event pass names three functions operating on a caller-owned
`KfCameraPathState` and four functions operating on map-event state. It replaces
41 field-sized BSS candidates with `map_event_pool`, names the following
`current_map_event` pointer and four-byte `player_progress_state`, and aggregates
the live camera transform into `camera_position` and `camera_rotation`. These
five objects retain `scope=unknown`: multiple xrefs prove shared state but not
external versus file-local linkage. Six instruction-word false positives are
preserved as rejected relocation rows rather than accepted as callers.

The following query/matrix pass names three map-event queries, the generic
3x3 matrix interpolator, two color-effect helpers, and the player restoration
effect. It also corrects both GTE matrix-wrapper signatures and merges two
scalar candidates into `player_vitals`, whose four halfwords are maximum and
current HP followed by maximum and current MP. Its linkage and the meanings of
the neighboring player flag bits remain unresolved.

The player-death pass names the signed HP/MP adjustment helpers, broad game
state initializer, restart policy, and the forward/reverse visual handlers. It
adds the missing `KfMatrix` BSS snapshot at `0x80058060` and names the saved fog
distance, camera-pitch step, and Q12 visual blend. These four identities retain
unknown source linkage. The player action-state byte remains inside its
unresolved four-byte owner, and nearby general camera/render fields remain
address-named rather than being falsely claimed as death-private storage.

The player-combat pass names the five-component damage formula, direct and
radial player-damage paths, and the bounded player-distance query. It also
names the status-effect flag word, five numbered component defenses, the one
directly observed status resistance, and four signed status timers. These
member identities retain unknown source linkage, while the later common-base
and complete-clear evidence supports their placement inside `KfPlayerState`.
Component and effect lore remains numbered.

The player stats/equipment pass names experience and level growth, hidden
physical-power and magic training, combat-stat recalculation, and magic,
weapon, and six-slot equipment selection. It aggregates both compiler switch
tables, the forty-record runtime level-growth table, POWER charge state, and
packed sound references. Component lore and unresolved record fields remain
numbered or opaque.

The player motion/weapon-attack pass names swing initialization and update,
map-position synchronization, view bob, vertical floor following, and the
distance-plus-facing query. It replaces five scalar motion seeds with one
complete `KfPlayerMotionState`, types two packed `KfMapCell` objects, decodes
three fields in the `0x2c`-byte `KfWeaponRecord`, and admits three exact
100-by-100 map grids. The weapon runtime fields remain separate identities:
adjacency does not prove an enclosing source structure.

The player interaction/collision pass names horizontal movement, the
floor-entry warp, transform snapshot, actor/person image display, item use,
and the common six-argument world-collision query. It aggregates the two item
switch tables and the five-entry floor-cell table. Exact copy bounds correct
the former `asset_block` into sixteen `KfWeaponRecord` objects and separate the
following `KfCollisionTarget`; its known transform and radius fields are typed
while the six-byte tail remains opaque.

The player-update pass names the `0x1a1c`-byte main player update, three
lighting-preset wrappers, and the leaf that installs status effect 4. It maps
two previously opaque signed halfwords to `KfPlayerState` timer fields at
`+0x50` and `+0x52`. The three repeated lighting wrappers share one
`game.lighting_presets` unit and the status mutator remains separate. All four
small functions are strict exact matches; the large update remains a
separately planned reconstruction.

The collision-grid pass names and exactly reconstructs the cell floor-height
query, its Psy-Q `VECTOR` position wrapper, and the 5x5 occupancy updater. It admits
the fourth 100-by-100 map grid at `0x80069018`, proves unsigned cell-coordinate
arguments from thirteen lifecycle call sites, and keeps all four grids as
separate arrays because their common extent does not prove an enclosing map
structure.

The shared GAME/OPEN display and TMD pass names 22 functions across eleven
cross-overlay pairs. It promotes two complete primitive-buffer records per
image to `KfPrimitiveBuffer[2]`, records both Psy-Q draw/display-environment
arrays, types the overlay-specific TMD pointer tables and current vertex
cursor, and aggregates the persistent view position and rotation vectors. The
standard 0x1c-byte `KfTmdObject` layout is checked against the lookup stride and
the primitive-preparation loop. The adjacent public `VSync` body and its
private worker are classified as version-skewed `LIBGPU/VSYNC` code and removed
from game progress.

Functions owned by an object family use `owner_action`, with the same parts in
the `owner` and `action` columns. Signatures use semicolon-separated C
declarations so argument names and widths can be refined independently. Data
rows carry an exact admitted extent, load/BSS storage, tentative linkage scope,
datatype, and owner.

The structure inventory currently covers 71 types and 635 fields. 521 fields
have supported semantic names and 114 exact ranges remain explicitly opaque.
`kf inventory check` derives the 32-bit layouts from the checked C headers and
rejects any TSV disagreement in size, offset, extent, name, or datatype.

`KfCdFileEntry` is owned by `include/kf/game_cd.h`; `KfSaveSlotSummary`,
`KfSaveDirectory`, `KfSaveHeader`, and `KfSavePayload` are owned by
`include/kf/game_save.h`; the shared TMD payload, primitive, and projection
layouts are owned by `include/kf/tmd.h`; and `SoundRef`, `KfAudioVoiceSlots`,
and `KfAudioState` are owned by `include/kf/audio.h`. The game-owned
`KfVecXZs`, `KfVec3s`, `KfVec3i`, `KfPitchYaw`, and `KfEulerAngles` layouts
are owned alongside their operations by `include/kf/game_math.h`; and the
actor layouts and state aggregate are owned by `include/kf/game_actor.h`.
The map cell, copy-region, map-object, map-event, and camera-path layouts are
owned by `include/kf/game_map.h`; and the serialized and runtime floor-item
layouts are owned with their loader interface by `include/kf/item.h`.
`KfMagicRecord` is owned with the spell loader and cast interface by
`include/kf/magic.h`; and the runtime and rendering views of the effect-pool
record are owned by `include/kf/game_effect.h`. `KfWeaponRecord` and
`KfArmorRecord` are owned with their consecutive resource loaders by
`include/kf/game_equipment.h`.
The remaining checked player, collision, and render-state layouts still live
in `include/kf/semantic_types.h`. Their sizes are
checked against `config/retail/structures.tsv` by `kf inventory check`;
inventory-only vectors such as `KfVec4s` and `KfVec4i` do not acquire C
declarations until a reconstructed interface needs them, and established
reconstruction types such as `KfMatrix` remain in `include/kf/game_types.h`.
A type name records only fields and extents
supported by the current MIPS access pattern; it does not claim the original
source spelling.

The confidence values are `address-only`, `candidate`, `supported`, and
`proven`. `proven` is reserved for a recovered source symbol/signature or an
equally direct witness; a plausible semantic reconstruction normally remains
`supported`.

## Review loop

```sh
kf inventory check
kf inventory propose
kf inventory ghidra --image game --image open
kf inventory propose-ghidra

kf sema --image game addr vector2s_scale_shift11
kf sema --image game xref pool_records --confirmed-only
kf sema --image game disasm vector2s_scale_shift11 --blocks
```

`kf inventory propose` writes two ignored review files:

- `build/function-inventory/evidence.tsv` combines MIPS live-in signature
  hints, callers, callees, indirect calls, strings, data references, and basic
  instruction-shape counts for all 734 functions;
- `build/function-inventory/data-evidence.tsv` lists confirmed/candidate users,
  read/write/address counts, and tentative scope/owner hints for every tracked
  datum;
- `build/function-inventory/relocs-bss.tsv` proposes decoded MIPS HI/LO pairs
  from non-vendored code to admitted BSS extents.

A datum with one user receives a `function-static` hint and that function as
an owner hint; the adjacent confidence column distinguishes confirmed from
candidate xrefs. This is prioritization evidence only: another xref may still
be missing, and local linkage cannot be reconstructed from one use alone.
Multiple users produce `shared`, not `global`: without reliable TU ownership,
xrefs cannot distinguish external linkage from a file-local static used by
several functions.

`kf inventory ghidra` keeps persistent, ignored projects under
`build/ghidra-inventory`. Its JSON contains the inferred prototype, structured
parameter storage, decompiled C, and non-flow memory references for each
non-vendored function. Ghidra results are proposals because analysis can merge
functions, infer the wrong width, or manufacture a misleading default name.
They must be checked against retail instructions, delay slots, callers,
callees, relocation referents, and the pinned headers before admission.
`kf inventory propose-ghidra` merges those inferred prototypes into an ignored
review TSV while preserving supported/proven signatures; it also never edits
the curated identity file. It also writes `ghidra-data-xrefs.tsv`, grouping
Ghidra's memory references by target and marking each as admitted data/text,
scratchpad, hardware, or unowned RAM. Unowned RAM and zero-admitted-site groups
are queues for missing global/static extents and relocation review.
Its companion `data-identities.ghidra.tsv` proposes non-overlapping BSS extents
only for RAM addresses directly read or written by an instruction. Mere scalar
constants, pointer-only parameters, heap initialization addresses, scratchpad,
and hardware references are not auto-admitted.

## Curation rules

- Never key an identity by VA alone; `image` plus `va` is the key.
- Preserve an address-derived name until evidence supports a semantic one.
- Record semantic argument names only after checking call sites and the body.
- Do not create an interior global when the address is a field or element of a
  larger owned object; refine the owner's datatype and extent instead.
- `scope=static` means file-local linkage and `scope=function-static` means a
  function-local static. Neither may be promoted solely from xref count.
- Keep generated reports under `build/`; only reviewed edits belong in the two
  identity TSVs.

The inventories are intentionally iterative. Missing functions, relocations,
data extents, and xrefs can invalidate earlier candidates without invalidating
the workflow.

The current scan has admitted 3,177 such pairs as candidate xrefs to 590 BSS
owners. Safe delinking still withholds an out-of-load pair until its relocation
row is individually reviewed; candidate rows also cannot suppress a stronger
reviewed pair at the same instruction.
