# Function, global, and static identities

The structural census says where linked code and data exist. The identity
layer records the current source-level interpretation without changing delink
symbols or pretending that WIP names are original symbols:

- `config/retail/function_identities.tsv` has exactly one row for each
  carveable non-vendored function: 1 PSX, 495 GAME, and 244 OPEN rows;
- `config/retail/data_identities.tsv` tracks initialized objects and referenced
  BSS objects that may become C globals, file-local statics, or
  function-local statics; and
- address-derived `func_` and `DAT_` names are stable unresolved identities.

Current semantic coverage is 165 of 740 functions: 162 GAME, one PSX, and two
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
`game_semantic_player_death.tsv`, and `game_semantic_player_combat.tsv` files
under `config/evidence/`. The recovered subsystems and remaining unknowns are
described in [`save-system.md`](save-system.md),
[`screen-images.md`](screen-images.md), [`actor-system.md`](actor-system.md),
[`map-objects.md`](map-objects.md), [`audio-system.md`](audio-system.md),
[`camera-and-map-events.md`](camera-and-map-events.md),
[`event-queries-and-matrix-effects.md`](event-queries-and-matrix-effects.md),
[`player-vitals-and-death.md`](player-vitals-and-death.md), and
[`player-combat.md`](player-combat.md).

The current first pass contains 3,622 data identities, including 838
non-overlapping BSS extents. Eighty-nine data identities have semantic review:
27 in loaded storage and 62 in BSS. Eight loaded identities are candidate or
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
`current_map_event` pointer and four-byte `map_progress_state`, and aggregates
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
directly observed status resistance, and four signed status timers. All eleven
objects retain unknown source linkage, and their adjacency is not treated as
proof of one original structure. Component and effect lore remains numbered.

Functions owned by an object family use `owner_action`, with the same parts in
the `owner` and `action` columns. Signatures use semicolon-separated C
declarations so argument names and widths can be refined independently. Data
rows carry an exact admitted extent, load/BSS storage, tentative linkage scope,
datatype, and owner.

Shared inventory-only layout names such as `KfVecXZs`, `KfVec3s`, `KfVec3i`,
`KfVec4s`, `KfVec4i`, `KfPitchYaw`, `KfEulerAngles`, `KfActorDefinition`,
`KfActorActionProfile`, `SoundRef`, `KfAudioVoiceSlots`, `KfActor`,
`KfActorPlacement`,
`KfMapCopyRegion`, `KfMapObjectPlacement`, `KfMapObjectDefinition`,
`KfMapObject`, `KfCameraPathPoint`, `KfCameraPathState`,
`KfMapEventDefinition`, `KfMapEvent`, `KfMapProgressState`, `KfPlayerVitals`,
`KfSaveSlotSummary`, `KfSaveDirectory`, `KfSaveHeader`, and
`KfSavePayload` live in `include/kf/semantic_types.h` with compile-time size
checks; established reconstruction types such as `KfMatrix` remain in
`include/kf/game_types.h`. A type name records only fields and extents
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
  instruction-shape counts for all 740 functions;
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
