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
- [game-effect-update-constants.md](game-effect-update-constants.md): effect
  motion, collision/audio units, typed helper boundaries and the complete
  [105-occurrence remainder ledger](game-effect-update-literal-ledger.md).
- [modern-type-checking.md](modern-type-checking.md): modern compiler adoption,
  enum storage requirements, and consumer-backed vector pointer corrections.
- [game-selected-magic-types.md](game-selected-magic-types.md): selected-spell
  byte enum propagated through the player field, API, local and menu array;
  compiler rejection evidence and explicit resource/serialization boundaries.
- [game-magic-learning-state.md](game-magic-learning-state.md): byte learning
  enum propagated through runtime records and saves, exact-one versus nonzero
  predicates, and the retail resource's initial learning state.
- [game-entity-sprite-tables.md](game-entity-sprite-tables.md): typed floor-item
  and effect descriptor tables, packed facing/count constants, depth-bias
  names, and the retail initial-frame bounds limitation.
- [game-render-material-identities.md](game-render-material-identities.md): HUD,
  notification and floor-item material names; SDK palette records and their
  ownership evidence.
- [game-notification-identities.md](game-notification-identities.md): all 32
  retail message labels, typed queue/link fields and variadic API, separate
  phase enum, and the [retained-literal ledger](game-notification-literal-ledger.md).
- [game-map-dialogue-state.md](game-map-dialogue-state.md): GAME dialogue
  stages/pages, animation phase, byte enum domains and corrected call evidence.
- [game-map-event-literals.md](game-map-event-literals.md): map-event movement,
  queries, saved fields and the complete three-module retained-literal ledger.
- [game-map-script-state.md](game-map-script-state.md): typed saved floor-script
  fields, area-trigger stages, revival, map-piece reveals and boss progression.
- [game-map-script-literals.md](game-map-script-literals.md): floor-script
  literal ledger, cutscene units and phases, interaction bounds and menu modes.
- [game-asset-animation-layout.md](game-asset-animation-layout.md): resource-backed
  clip counts, typed asset ownership/cache states and the pool/registry literal audit.
- [game-item-pickup-outcomes.md](game-item-pickup-outcomes.md): corrected pickup
  identity, typed outcomes, positioned labels and retained pickup/modal literals.
- [game-spell-identities.md](game-spell-identities.md): all nine retail spell
  names through learning, casting and effect kinds, exact status keep-masks,
  and the [casting/menu literal ledger](game-spell-literal-ledger.md).
- [game-item-equipment-identities.md](game-item-equipment-identities.md): 38
  retail item labels, corrected head/body/shield identities, special equipment
  behavior and unchanged object verification.
- [game-item-special-identities.md](game-item-special-identities.md): nineteen
  retail special-item labels, illusion staff timer propagation, harp countdown
  parameters and the complete retained item-use literal ledger.
- [game-item-consumables-accessories.md](game-item-consumables-accessories.md):
  ten retail item names across healing, revival, accessory bonuses and shop stock;
  exact cure-mask policy and explanations of retained balance amounts.
- [game-shop-price-domains.md](game-shop-price-domains.md): typed buy/sell prices,
  shop-window rows, retail Gold Cross/key identities and the shop literal ledger.
- [game-menu-preview-transform.md](game-menu-preview-transform.md): complete
  shared preview rotation, draw-rate units, Q12 lighting and retained literal ledger.
- [game-menu-confirmation-protocol.md](game-menu-confirmation-protocol.md): typed
  footer kinds, previews and results; retail Japanese labels and literal ledger.
- [game-equipment-domains.md](game-equipment-domains.md): separate player-slot
  and menu-category types, empty selections, record extents and literal ledger.

- [gcc257-epilogue-and-scheduling.md](gcc257-epilogue-and-scheduling.md):
  the framed-epilogue delay-slot form, the checked `div` expansion, and the
  prologue/load scheduling that separate the GCC 2.5.7 probe from the 2.6.0
  probe, with the corpus numbers behind the profile change.
- [game-menu-list-control-flow.md](game-menu-list-control-flow.md): exact menu
  list renderer; shared glyph workspace, unsigned decimal split, pointer walk,
  row-derived displacement and guarded loops resolve the former residue.
- [source-shapes-gcc257.md](source-shapes-gcc257.md): source shapes that
  decided exact matches under the 2.5.7 probe (loop counters, return-value
  joins, struct copies, two-statement accumulations).
- [game-model-emitter-lifetimes.md](game-model-emitter-lifetimes.md): retail
  case-body order closes map-object rendering; updating the byte texture index
  and the temporary complete graphics owner reproduce the entire actor body.
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
- [game-entity-traversal.md](game-entity-traversal.md): wrapped cell origins and
  shared actor/effect continuations recover retail loop tails; floor-item
  ownership and remaining instruction differences stay explicit.
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
- [game-actor-animation-slots.md](game-actor-animation-slots.md): separate
  action codes, definition-table slots and resource animation IDs; preserve
  the dispatcher mapping and remove the misleading action-minus-two macro.
- [game-actor-states.md](game-actor-states.md): name actor actions, lifecycle,
  vertical/collision states and allocation policies; distinguish progress
  markers, jump phases and counters across updates, rendering and save data.
- [game-actor-home-distances.md](game-actor-home-distances.md): dispatcher
  home-distance dependency control and explicit shared-epilogue exit audit.
- [game-actor-damage-exits.md](game-actor-damage-exits.md): boss-only health
  guard, unconditional credit mask, fatal-first arm and rounded damage lifetime.
- [game-actor-radial-falloff.md](game-actor-radial-falloff.md): calculated-first
  falloff and short scale restore the loop; entry setup remains non-exact.
- [game-actor-motion-limits.md](game-actor-motion-limits.md): drift acceleration,
  per-axis and yaw speed limits, exclusive home tolerance, and progress-byte roles.
- [game-actor-constants.md](game-actor-constants.md): actor phase limits,
  sound/parameter slots, collision policies, damage units and packed flags;
  consumer explanations for all retained literals in the three actor modules.
- [game-animation-phase.md](game-animation-phase.md): exact short-circuit
  phase predicate; eleven-caller audit and rejected cache/early-return controls.
- [game-effect-state.md](game-effect-state.md): complete startup-cleared effect
  owner and member-relative addressing closing the current-record selector.
- [game-effect-protocol.md](game-effect-protocol.md): behavioral effect kinds,
  collision/power bits, render and homing selectors, propagation roles and
  floor phases; distinguish actor damage unity from player Q12 damage scale.
- [game-effect-literals.md](game-effect-literals.md): projectile and hazard
  phases, corrected swing-probe offsets, and explanations for all 764 retained
  literals in the five effect/magic modules.
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
- [game-map-object-constants.md](game-map-object-constants.md): interaction
  types, running actions, switch phases and transient drop groups; all 267
  retained literals in the two map-object modules have consumer-specific reasons.
- [game-map-forward-probe.md](game-map-forward-probe.md): shared collision
  block and result join; partial reconstruction improved, not exact.
- [game-map-collision-edge.md](game-map-collision-edge.md): chained
  opposite-row assignments recover the shared grid base and close the marker.
- [game-volume-distance.md](game-volume-distance.md): exact player and actor
  volume distances through a shared rejection tail and evolving horizontal
  coordinate parameters; signed height and SDK square-root contracts retained.
- [game-spatial-audio-lifetime.md](game-spatial-audio-lifetime.md): one evolving
  distance/attenuation value restores the GAME spatial-audio body, with the
  exact OPEN counterpart as an independent source-shape witness.
- [game-effect-scatter.md](game-effect-scatter.md): unsigned caller evidence
  and signed in-place accumulation close the three-halfword scatter helper.
- [game-floor-deformation.md](game-floor-deformation.md): mutable absolute
  step, byte coordinates and direct predecrement countdown close the helper.
- [game-menu-list-format.md](game-menu-list-format.md): typed menu-row
  access closes list initialization; index-driven termination and a word-sized
  padding temporary close decimal glyph formatting.
- [game-menu-glyph-render.md](game-menu-glyph-render.md): index-derived
  fixed pitch closes number rendering; signed shared dimensions close the
  string renderer without altering its positive decoration constants.
  Retail atlas offsets prove the kana marks, blanks and slash.
- [game-menu-class-titles.md](game-menu-class-titles.md): decoded SOLDIER/MAGE
  title grid, shared base-stat thresholds and atlas strides, and the complete
  [status-text literal ledger](game-menu-status-literal-ledger.md).
- [game-menu-list-results.md](game-menu-list-results.md): list pending and
  no-selection controls, item HP recovery amounts, Bless's healing multiplier,
  and a complete [item-use/discard ledger](game-menu-item-list-literal-ledger.md).
- [game-menu-choice-state.md](game-menu-choice-state.md): typed root choices,
  equipment dispatch and panel phases, configuration row bindings, and a
  complete [four-function literal ledger](game-menu-choice-literal-ledger.md).
- [game-menu-save-results.md](game-menu-save-results.md): typed save/load
  confirmation results, distinct root controls, window rows, decoded message
  textures, and the complete four-function literal ledger.
- [game-menu-window-kinds.md](game-menu-window-kinds.md): decoded window layouts,
  enum propagation through drawing/prompt/list APIs, and retained initializer
  literals with their measured units.
- [game-menu-drawing-layers.md](game-menu-drawing-layers.md): shared ordering
  buckets, passive-panel input-release timing, map scale and texture placements,
  with a complete [panel/map/list ledger](game-menu-panel-literal-ledger.md).
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
  and shared signed point/dimension recovery; the genuine SDK rectangle macro
  closes both blitters while preserving every narrow load and ordered store.
- [game-item-menu-dimensions.md](game-item-menu-dimensions.md): shared item,
  stock and glyph-row extents, complete-object startup clears, CD sector units,
  and the [database/startup literal ledger](game-item-database-literal-ledger.md).
- [game-player-progression.md](game-player-progression.md): training and stat
  limits, runtime growth-table extent and extrapolation, spell-learning
  requirements, and a complete progression/stat literal ledger.
- [game-player-damage-units.md](game-player-damage-units.md): damage and
  multiplier scales, Q12 falloff, poison-roll conversion and restart override;
  completes literal-ledger coverage of the player death/combat module.
- [game-player-vertical-state.md](game-player-vertical-state.md): typed player
  vertical states, terrain step/fall parameters, walking bob, shared camera
  height and bottomless-pit identity; includes the motion literal ledger.
- [game-player-core-constants.md](game-player-core-constants.md): weapon charge
  and phase units, hit windows, collision geometry and shared HUD dimensions;
  completes the literal ledger for all fourteen player-core functions.
- [game-player-update-constants.md](game-player-update-constants.md): input
  acceleration and pitch limits, weapon-magic timing and projectile geometry,
  Moonlight projectile and map hazard identities; includes the complete
  [player-update literal ledger](game-player-update-literal-ledger.md).
- [game-effect-pool-constants.md](game-effect-pool-constants.md): lightning
  impact/blast and ground-branch visual identities, constructor scale/audio
  units, emergence-depth and orbit-center relationships; includes the complete
  [effect-pool literal ledger](game-effect-pool-literal-ledger.md).
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
- [game-config-panel-abi.md](game-config-panel-abi.md): two by-value labels,
  input policy and first-frame/loop-tail draw structure recover the options
  panel's CFG; its draw helper stays exact and nine register-role words remain.
- [game-save-load-hub-flow.md](game-save-load-hub-flow.md): exit-row highlight,
  pending-action reset, post-loop return and case dispatch close the hub;
  the earlier basic-block-layout limitation was not established.
- [game-save-load-panels-flow.md](game-save-load-panels-flow.md): confirmation
  resets, shared returns, empty-slot arm order and the explicit format-row
  guard recover both panels without register or constant carriers.
- [game-drop-item-flow.md](game-drop-item-flow.md): observed initialization
  and traversal order recover the inventory back-edge delay slot; the
  remaining three register roles are an unattributed, non-exact residue.
- [game-status-panel-flow.md](game-status-panel-flow.md): reviewed frame-head
  jump, shared settling increment and natural corner order recover the
  status-panel CFG; descriptor addressing and frame extent remain non-exact.
- [game-list-confirmation-flow.md](game-list-confirmation-flow.md): current-pad
  lifetime, positive edge order, explicit toggle and first-frame/post-input
  redraw entries close the shared list-confirmation loop.
- [game-item-use-flow.md](game-item-use-flow.md): restores empty-list input,
  confirmation reset and long-list upward wrap; direct name-row indexing and
  loop shape recover all transfers, leaving one seven-word healing residue.
- [game-shop-panel-flow.md](game-shop-panel-flow.md): restores buy/sell
  confirmation reset and long-list upward wrap, with matching transfer
  destinations and ordered referents; 17/25 register-operand words remain.
- [game-magic-panel-flow.md](game-magic-panel-flow.md): direct spell-name
  indexing, cancellation-first handling, cursor branches and a shared normal
  return close the magic panel while preserving its distinct failure exits.
- [game-equipment-spell-selector-flow.md](game-equipment-spell-selector-flow.md):
  pre-switch stock-bank acquisition closes equipment selection and its two
  tables; indexed spell names remove a separate cursor-order mismatch.
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
