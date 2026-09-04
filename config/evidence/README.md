# Reviewed evidence

This directory preserves the small, reviewable inputs behind
the curated retail inventories; it does not contain Sony binaries.

`game_match_campaign_30.tsv` records the pre-edit MIPS evidence and final
matching verdicts for the first GAME reconstruction campaign.
`game_semantic_math_lifecycle.tsv` records the assembly, CFG, xref, SDK-header,
and vendored-negative-control evidence behind the first lifecycle/frame-pacer
and fixed-point helper names. These campaign tables support identities in
`function_identities.tsv` and `data_identities.tsv`; they are not recovered
original symbols.

`game_semantic_audio_control.tsv` records the per-function retail, caller,
string, shared-state, and Psy-Q 2.5 header evidence for GAME audio
initialization, VAB loading, and sequence lifecycle policy. API-name candidates
without an exact FID remain explicitly qualified in that ledger.

`game_semantic_audio_spatial.tsv` records positional sound, listener snapshots,
and the ten-slot voice manager. The companion `vmanager-key-utilities` rows in
`overlay_lineage.tsv` prove that the called `SsUtKeyOn`/`SsUtKeyOff` bodies are
shared SDK code in GAME and OPEN even though their version-skewed bytes evade
the exact Release 2.5 FIDs.

`game_semantic_camera_events.tsv` records the camera-path interpolator and the
adjacent map-event family. It includes the stack-owned path-state layout, the
eight-record event-pool boundary, five shared global/static candidates, the
ambiguous short FID on the current-event setter, and six rejected false
control-flow xrefs. The two families share an investigation band, not proven
translation-unit ownership.

`game_semantic_player_death.tsv` records player HP/MP adjustment, broad game
state initialization, restart policy, and the two-phase death transition. It
also supports four global/static identities, including the previously missing
saved-color-matrix BSS extent and its three carry-adjusted relocation pairs.
The fourth `lui`/`ori` construction is retained as address evidence only.

`game_semantic_player_stats.tsv` records player experience and level growth,
base/effective physical-power and magic state, POWER/MAGIC/training counters,
combat-stat recalculation, and magic/weapon/equipment selection. It also
records the two compiler switch tables, the runtime-loaded 40-record level
growth table, packed sound references, and mutable weapon-image path.

`game_semantic_player_motion_attack.tsv` records the weapon swing/charge path,
map-position synchronization, grounded view bob, and vertical floor-following
state machine. It supports complete `KfPlayerMotionState`, `KfMapCell`, and
partially decoded `KfWeaponRecord` layouts plus three 100-by-100 map grids.

`game_semantic_player_interactions.tsv` records horizontal collision movement,
floor-entry warping, player transform snapshots, actor/person image display,
item dispatch, and the shared world-collision query. It also corrects the
weapon load destination to `KfWeaponRecord[16]`, separates the following
`KfCollisionTarget`, and reviews both item switch tables and the campaign's
decoded relocations.

`game_semantic_player_update.tsv` records the large player-update identity,
three adjacent lighting-preset wrappers, and status-effect helper 4. It ties
the two recovered signed timers to `KfPlayerState` offsets `0x50` and `0x52`
and records the relocation-only causes behind three initially non-exact helper
objects.

`game_semantic_menu_interaction_api.tsv` records the player-facing map-action
dispatcher and modal-menu boundary. It proves the read-only position input,
mutable rotation output, and the variadic O32 home-slot contract shared by the
player-update and map-interaction units.

`game_semantic_item_menu_data.tsv` records the four consecutive banks loaded
from `COM\\STAT.DAT`: 80 item-name rows, 9 magic-name rows, and the two 80-by-2
buy/sell price tables. It also records every affected consumer and proves that
the former interior address identities are aggregate row addends.

`game_semantic_render_cell_windows.tsv` records the 16 runtime-loaded
`KfCellWindow` records, the per-frame active-window pointer, and the loader,
selector, and entity-culling functions that prove their extents and fields.

`game_semantic_effect5_texture_tables.tsv` records the two three-entry texture
page and CLUT tables prepared for floor mode five and their paired actor-render
consumer. It proves that six former halfword identities were two indexed banks.

`game_semantic_display_tmd.tsv` records eleven related GAME/OPEN function
pairs for frame submission, view transforms, TMD registration/access, and
primitive-index preparation. It supports complete `KfPrimitiveBuffer` and
`KfTmdObject` layouts, the image-qualified display/TMD state, and the explicit
negative control that moves four version-skewed `LIBGPU/VSYNC` functions to the
vendored inventory. `overlay_lineage.tsv` independently checks the shared
instruction shapes and both overlay deltas.

`game_tu_menu_runtime.tsv` records the nine-function tail from the menu frame
helpers through numbered TIM loading. Together with
`game_tu_menu_presentation.tsv`, it supports the one 16-function WIP module
ending at the explicitly unresolved save-system boundary.

`game_tu_menu_presentation.tsv` records the seven gapless GAME functions from
the two-option dispatcher through the window-backdrop renderer. It supports
one shared glyph-string interface, typed font/sprite descriptors and border
quads, and the presentation half of the combined WIP menu-runtime module. The
neighboring list renderer remains a separate unresolved boundary.

`game_open_semantic_memory_allocator.tsv` records the seven-function LIFO
allocator duplicated in GAME and OPEN, including its signed interfaces, two
allocation modes, 16-entry state array, all direct calls, and the intentional
overlay-specific arena spans. `overlay_lineage.tsv` independently checks the
constant address delta and all 144 instruction shapes. The adjacent state is
kept as separate globals because no enclosing C object has yet been proved.

`psyq_release_25_text_sections.tsv` is the `.text` subset of the relocation-
aware object match report produced during the original King's Field
investigation. Each address was found by masking only link-editable bits in a
Psy-Q object and comparing every remaining bit against a hash-verified retail
PS-X EXE. Duplicate `S_R`/`S_W` rows intentionally retain an archive ambiguity.

`psyq_release_25_complete_objects.tsv` records eight reviewed 16-byte `LIBAPI`
members: six in `PSX.EXE` plus the identical `C51` `malloc` members in GAME
and OPEN. These were below the original matcher's 32-byte cutoff, so their
complete-byte comparisons were reviewed separately.

`kf-vendored-seed` does not blindly trust either table. It extracts the named
members from the hash-pinned Release 2.5 archive with `psy-k`, reconstructs
interleaved `.text` records, applies the retained MIPS relocation masks, checks
sizes/bit counts/relocation counts, and verifies each recorded occurrence in
the clean retail executables. It also checks XDEF offsets before using a symbol
as a function name.

The external SDK archive is supplied by the Nix environment and remains
hash-pinned there. The executable inputs are user-supplied; their SHA-256,
header, load address, and load size must match `scripts/kf/retail.py` before
the evidence or the secondary Psy-Q 2.60 signature lane is accepted.
