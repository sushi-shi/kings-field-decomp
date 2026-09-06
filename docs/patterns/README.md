# Compiler and MIPS pattern notes

Reusable, evidence-backed observations about the retail code generation and
about which source shapes reproduce it under the current probes. Each note
records the retail evidence first and the probe behaviour second; none of them
promotes a probe to a proven historical toolchain.

- [semantic-field-names.md](semantic-field-names.md): consumer-backed actor,
  player, map, render, and effect field identities, reused effect storage,
  and the remaining serialized-field questions.
- [source-constant-names.md](source-constant-names.md): domain-specific constant
  names and the reasons for retaining individual classes of inline literals.

- [gcc257-epilogue-and-scheduling.md](gcc257-epilogue-and-scheduling.md):
  the framed-epilogue delay-slot form, the checked `div` expansion, and the
  prologue/load scheduling that separate the GCC 2.5.7 probe from the 2.6.0
  probe, with the corpus numbers behind the profile change.
- [source-shapes-gcc257.md](source-shapes-gcc257.md): source shapes that
  decided exact matches under the 2.5.7 probe (loop counters, return-value
  joins, struct copies, two-statement accumulations).
- [compiler-attribution.md](compiler-attribution.md): a dedicated
  cross-version pass over the residue corpus. How GCC 2.4.1 CC1PSX was made
  runnable (a go32-stub frankenbinary under DOSBox) and the finding that it
  reproduces retail no better than 2.5.7 `-O2` (byte-identical on most residue
  functions, worse where it diverges); the residue ceiling is an unattributed
  scheduling/regalloc residue, and `-mcpu=r2000` is a per-TU discriminator, not
  a global truth.
- [psyq-cpe-tail-and-bss.md](psyq-cpe-tail-and-bss.md): the CPE v1 prefix
  embedded at GAME's and OPEN's initialized-data tails, and why referenced storage inside
  the final page can still be original BSS rather than initialized source data.
- [maspsx-private-bss.md](maspsx-private-bss.md): preserving compiler-private
  tentative storage in ELF, cross-TU linker controls, and the distinction
  between linkage correction and unresolved allocation topology.
- [assembler-section-extents.md](assembler-section-extents.md): distinguish
  automatic GNU-as section tails from explicit zeros, alignment directives and
  compiler COMMON allocations; retain actual ELF placement requirements.
- [psyq-section-alignment.md](psyq-section-alignment.md): original PSYLINK
  controls distinguish raw LNK v2 alignment tags from ELF byte alignment;
  SDK import correction without altering compiler-generated game objects.
- [common-allocation-fidelity.md](common-allocation-fidelity.md): reject a
  reproduced unclaimed-COMMON false pass; original PSYLINK controls distinguish
  fixed reservations, exported coalescing, size rounding and name-sensitive order.
- [compiler-owned-data-sizes.md](compiler-owned-data-sizes.md): independently
  measure C object sizes instead of copying retail DATA expectations into source
  symbols; reject a reproduced false BSS match hidden by COMMON rounding.
- [retail initialized packing](../../config/evidence/open_sound_data_extent.md):
  carve actual inter-object bytes, including nonzero controls, and separate
  OPEN's three-byte SoundRef from its following census byte.
- [bss-ownership-comparison.md](bss-ownership-comparison.md): per-section BSS
  ownership checks, swapped-identity and linkage false-positive controls, and
  inclusion of custom allocated NOBITS sections; matching data must also admit
  a retail base satisfying both objects' actual section alignment.
- [open-map-render-residue.md](open-map-render-residue.md): RTBL window
  ownership, historical matrix/setup controls and the exact map-cell closure.
- [sdk-vector-publication.md](sdk-vector-publication.md): natural XYZ source
  order closes OPEN map-cell and GAME camera-step despite their interleaved
  retail store schedules; separate-statement controls rule out macro uniqueness.
- [game-map-cell-source.md](game-map-cell-source.md): GAME's byte object-index
  lifetime, three-case door remapping and natural XYZ coordinates; the shared
  owner control narrows the remaining mismatch to eighteen stack operands.
- [game-map-copy-region-source.md](game-map-copy-region-source.md): direct
  index-4 callers recover the fifth six-byte rectangle descriptor;
  source-before-destination column advances recover the five-grid copy loop.
- [game-map-enqueue.md](game-map-enqueue.md): restore allocation-overflow
  returns and retail packet dispatch in the GAME map polygon emitter.
- [game-model-enqueue.md](game-model-enqueue.md): four-mode model packet
  traversal, material publication and shared depth tails.
- [game-tmd-enqueue.md](game-tmd-enqueue.md): twelve TMD mode entries,
  reviewed switch/internal jumps and shared lighting/depth tails.
- [game-sprite-enqueue.md](game-sprite-enqueue.md): projected sprite depth,
  authentic SDK outputs and texture publication order.
- [game-graphics-scratch.md](game-graphics-scratch.md): projection/morph owner
  controls and the animation allocation-retry target.
- [game-render-setup.md](game-render-setup.md): exact error-screen brightness
  lifetime and the unconditional TMD primitive countdown.
- [open-floor-item-render-residue.md](open-floor-item-render-residue.md):
  typed floor-item sprite descriptors and the extra facing-byte mask shared
  by the GAME and OPEN retail renderers.
- [open-sprite-render.md](open-sprite-render.md): typed sprite material,
  SDK packet ownership, and the depth/texture source forms behind the exact
  OPEN projected-sprite renderer.
- [open-ft4-descriptor.md](open-ft4-descriptor.md): the byte-identical SDK
  rectangle trial and why texture byte loads do not establish source field width.
- [open-camera-step.md](open-camera-step.md): grouped position/rotation
  publication closes the fixed-point camera step and its three-function unit.
- [open-tmd-countdown.md](open-tmd-countdown.md): unconditional primitive
  countdown initialization leaves only two frame instructions unmatched.
- [open-entity-traversal-residue.md](open-entity-traversal-residue.md): visible
  entity/item traversal, the wider floor-item state owner, and the remaining
  entry/material-base instruction differences.
- [notification-state.md](notification-state.md): retail payload/control
  base relationships, the variadic enqueue signature, and the distinction
  between the recovered notification fields and the wider graphics object.
- [game-notification-dequeue.md](game-notification-dequeue.md): typed dequeue
  control subobject and late pointer binding recover retail base/offset forms;
  the initial tail-load address materialization remains non-exact.
- [game-warp-shimmer-traversal.md](game-warp-shimmer-traversal.md): forward
  effect cursors, descending allocation/release counts and one active-effect
  pointer close the 568-byte warp-shimmer animator.
- [game-warp-trigger-results.md](game-warp-trigger-results.md): a missing
  completion result and explicit shared warp-call/destination paths close
  the 588-byte floor-trigger dispatcher.
- [game-warp-position-buffers.md](game-warp-position-buffers.md): complete
  VECTOR objects across all shimmer callers and full-word variant handoffs
  close the remaining change-floor and same-floor warp helpers.
- [game-projectile-helpers.md](game-projectile-helpers.md): byte lifetime,
  signed shared rotation fields and landing-store order close the 3D helper;
  the 2D helper's extra retail frame space remains unexplained.
- [effect-update-dispatch-map.md](effect-update-dispatch-map.md): reviewed
  GAME dispatcher relocations, corrected argument/state dataflow and observed
  handler order improve the partial match; matching call sets alone is insufficient.
- [render-material.md](render-material.md): supported texture/colour identities,
  unresolved aggregate boundaries, and the retail effect-pool skip behavior.
- [tmd-counts.md](tmd-counts.md): word-sized disk counts, explicit halfword
  narrowing in preparation, and the shared GAME/OPEN TMD interface.
- [open-unlit-triangles.md](open-unlit-triangles.md): the unlit F3/FT3 path,
  shared packed packet views, initial packet dependency and remaining OT-base
  and frame/register differences.
- [open-graphics-runtime-owner.md](open-graphics-runtime-owner.md): complete
  graphics owner, exact map traversal/VRAM adjustment, and preserved initializer controls.
- [game-graphics-owner-pilot.md](game-graphics-owner-pilot.md): complete-clear
  ownership pilot, preserved initializer/traversal controls, and the exact
  unconditional view-matrix rebuild; GAME data migration remains unresolved.
- [game-display-environment-data.md](game-display-environment-data.md): complete
  SDK-typed environment arrays, reviewed interior references and the screen
  fade's unconditional RGB stores; larger graphics ownership stays provisional.
- [open-map-enqueue.md](open-map-enqueue.md): lit map GT3/GT4 emission,
  the complete colour owner, and byte-identical shared packed-coordinate types.
- [open-emitter-source-controls.md](open-emitter-source-controls.md): negative
  combined-unit, packet-header and vertex-address spelling controls; no new exacts.
- [open-driver-default-control.md](open-driver-default-control.md): observed
  original CCPSX command defaults and the unchanged OPEN G0/G8 comparison.
- [tmd-projection.md](tmd-projection.md): projection/accessor ownership,
  the observed ReadSZ2 call contract, and unattributed register differences.
- [game-projection-countdown.md](game-projection-countdown.md): direct incoming
  count countdowns closing both GAME perspective projection helpers.
- [game-view-transform.md](game-view-transform.md): the copy-only nullable
  rotation branch and unconditional matrix updates closing the GAME view helper.
- [game-actor-free-countdown.md](game-actor-free-countdown.md): predecrement
  countdown and one result join closing the GAME actor free-slot search.
- [game-event-cone-search.md](game-event-cone-search.md): short angle locals
  closing the event selector without widening its existing facing parameter.
- [game-event-progress-image.md](game-event-progress-image.md): shared dirty
  tail and limit-first comparison close the event-image progression helper.
- [game-actor-cone-search.md](game-actor-cone-search.md): short angle locals
  and the six-caller short-facing contract closing the actor selector.
- [game-actor-action-selection.md](game-actor-action-selection.md): missing
  floor-grid relocation, shared rejection exit and typed indexing close
  ground selection; far-first profile selection improves its partial match.
- [game-animation-phase.md](game-animation-phase.md): eleven-caller audit and
  unchanged explicit phase-cache control; predicate remains non-exact.
- [game-effect-state.md](game-effect-state.md): complete startup-cleared effect
  owner and member-relative addressing closing the current-record selector.
- [game-effect-constructor.md](game-effect-constructor.md): reviewed switch
  references, full SDK-vector copies and corrected kind-dependent byte stores.
- [game-effect-map-collision.md](game-effect-map-collision.md): signed cell
  coordinates, direct neighbor indexing, selected rectangle and backward span
  join recover the collision helper; return/join differences remain non-exact.
- [game-world-collision-source.md](game-world-collision-source.md): early
  typed definition pointers, shared hit value and separate query mask recover
  world-query instructions; entry differences remain non-exact.
- [game-magic-cast-source.md](game-magic-cast-source.md): case-local objects,
  distinct distance outputs, projectile-speed lifetime and height-branch order
  improve magic casting; remaining stack/angle differences stay unattributed.
- [game-map-object-clearing.md](game-map-object-clearing.md): corrected
  type-0..8 link predicate and aligned whole-link reset close both clearing
  functions; the earlier memset control emitted an unsupported call.
- [game-map-forward-probe.md](game-map-forward-probe.md): shared collision
  block and result join; partial reconstruction improved, not exact.
- [game-map-collision-edge.md](game-map-collision-edge.md): chained
  opposite-row assignments recover the shared grid base and close the marker.
- [game-volume-distance.md](game-volume-distance.md): shared rejection
  tail closes player distance; direct actor-center expression improves its
  partial match with a remaining register-role exchange.
- [game-effect-scatter.md](game-effect-scatter.md): unsigned caller evidence
  and signed in-place accumulation close the three-halfword scatter helper.
- [game-floor-deformation.md](game-floor-deformation.md): mutable absolute
  step, byte coordinates and direct predecrement countdown close the helper.
- [game-menu-list-format.md](game-menu-list-format.md): typed menu-row
  access closes list initialization; index-driven termination and a word-sized
  padding temporary close decimal glyph formatting.
- [game-menu-glyph-render.md](game-menu-glyph-render.md): index-derived
  fixed pitch closes number rendering and removes the string-render preheader
  difference; retail atlas offsets prove the kana marks, blanks and slash.
  Four decoration immediates remain non-exact.
- [game-player-damage-labels.md](game-player-damage-labels.md): retail glyphs
  identify fifteen player/armor attack and defense fields, preserving the
  different fourth attack/defense labels and doubled cutting contribution.
- [game-player-statuses.md](game-player-statuses.md): curse, darkness, slowed
  movement and fire-defense boost; distinct timers, fade ratios and tuning
  limits preserved with exact symbolic and raw comparisons.
- [game-player-poison.md](game-player-poison.md): the retail poison glyph
  identifies the player/equipment resistance fields, status bit and countdown;
  timing and literal-value limits are documented from the damage/update chain.
- [game-menu-window-rows.md](game-menu-window-rows.md): signed title-test load
  correction and rejected direct-row-address control in the window renderer.
- [game-menu-sprite-blit.md](game-menu-sprite-blit.md): thirteen-caller audit
  and the rejected SDK rectangle-macro control; inset instructions remain
  non-exact without changing shared or SDK types.
- [game-screen-image-loop.md](game-screen-image-loop.md): three admitted
  DRAWENV address pairs and unconditional per-frame RGB stores close the
  screen-image/input helper.
- [game-image-path-components.md](game-image-path-components.md): directory
  digit cursors recover TALK/map path referents and improve both formatters;
  register roles and the map floor-load schedule remain non-exact.
- [game-option-menu-reset.md](game-option-menu-reset.md): clearing the pending
  selection before the exit guard closes the option-menu dispatch loop.
- [game-menu-root-results.md](game-menu-root-results.md): removing three
  unsupported result resets improves the main-menu loop; an equivalent
  cancellation-condition control leaves the remaining comparison unchanged.
- [game-dialog-summary-loop.md](game-dialog-summary-loop.md): indexing the
  shared save-summary records removes duplicate advancing pointers and closes
  the dialog-frame renderer.
- [animation-cache.md](animation-cache.md): animation-pool metadata, typed
  cache pointers, complete BSS extent and contiguous binder/lifecycle ownership.
- [mips-parser-instrumentation.md](mips-parser-instrumentation.md): isolated
  parser write guards, load-delay audit, and reduced Unicorn memory-hook
  control-flow defects.
- [game-parser-verification.md](game-parser-verification.md): parser failure
  controls, VAB partial state and cleanup quirks, and seven-function match review.
- [open-format-display-sdk.md](open-format-display-sdk.md): four exact custom
  formatters, the display-adjustment address-base residue, twelve SDK provider
  attributions, and the corrected SquareRoot12 boundary.
- [open-ending-scroll.md](open-ending-scroll.md): complete ending-scroll C,
  initialized panel/camera/color owners, the unreachable epilogue correction,
  and the remaining short-state and branch-join comparison.
- [open-tmd-enqueue.md](open-tmd-enqueue.md): all twelve general TMD polygon
  modes, SDK-preserving packed views, static compiled/retail call-path controls,
  and the remaining shared graphics-base and instruction differences.
- [open-transition-snapshot.md](open-transition-snapshot.md): the directly
  observed transition-position stack copy and the remaining eight-byte frame
  discrepancy, with an exact list of non-exact object differences.
- [open-display-init-pointers.md](open-display-init-pointers.md): observed
  DRAWENV pointer lifetimes and height initialization, with bounded retail/C
  argument/write traces and remaining address-base differences.
- [open-fog-interpolation.md](open-fog-interpolation.md): scaled-difference
  expression recovering the exact OPEN fog interpolation and its preserved start.
- [game-fog-interpolation.md](game-fog-interpolation.md): the independently
  checked GAME counterpart, its three callers and exact five-function matrix unit.
- [game-angle-tolerance.md](game-angle-tolerance.md): corrected GAME circular
  tolerance logic, twelve call sites and the exact sixteen-function math unit.
- [game-ft4-selectors.md](game-ft4-selectors.md): promoted GAME texture selector
  arguments, preserved SDK packet widths and exact transition caller.
- [open-item-cursor.md](open-item-cursor.md): saved stream start, postincrement
  scan and direct count-member accesses closing the OPEN placement loader.
- [game-parser-exact-closure.md](game-parser-exact-closure.md): six remaining
  parser match attempts, retained source improvements and instruction-level residues.
- [game-parser-inline-helpers.md](game-parser-inline-helpers.md): pinned-compiler
  inline support, six parser helper experiments and the retained typed vertex copy.
- [game-world-state-layout.md](game-world-state-layout.md): the complete map
  runtime owner, preserved interior referents and newly exact spinner consumer.
- [tmd-packet-header.md](tmd-packet-header.md): bounded packet-header inline
  experiments, their stack traffic and the item-count reset negative control.
- [mips-biased-data-referents.md](mips-biased-data-referents.md): original SDK
  negative address addends, allocation-relative traversal, RODATA ownership
  and bounds/relocation controls that preserve the decoded retail target.
- [config-owned-data-comparison.md](config-owned-data-comparison.md): independent
  SDK/retail data-only objects, native reports, whole-object ownership checks
  and two-sided relinking before config ranges count as compared.
