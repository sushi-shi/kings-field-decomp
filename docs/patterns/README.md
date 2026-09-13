# Compiler and MIPS pattern notes

Reusable, evidence-backed observations about the retail code generation and
about which source shapes reproduce it under the current probes. Each note
records the retail evidence first and the probe behaviour second; none of them
promotes a probe to a proven historical toolchain.

- [semantic-consistency-review.md](semantic-consistency-review.md): consumer
  review of shared domains and retained helpers, cylinder/overlay naming,
  sprite traversal distinctions and the full-floor predicate boundary.
- [common-code-review.md](common-code-review.md): completed 522-entry source
  reading and application of all 90 candidate verdicts; 39 retained helpers,
  rejected trials, and unchanged bytes across all 101 comparison objects.
- [sdk-data-ownership.md](sdk-data-ownership.md): complete SDK tables, startup
  boundaries and reviewed ownership of the remaining data-field candidates.
- [data-ownership-inventory-cleanup.md](data-ownership-inventory-cleanup.md):
  redundant game RODATA identities and complete LIBSPU dispatch-table owners.
- [effect-constructor-varargs.md](effect-constructor-varargs.md): kind-specific
  optional argument domains, native `va_arg` controls, and the remaining
  manual argument walker omitted from the cleanup checklist.
- [startup-address-origins.md](startup-address-origins.md): startup literal
  owners and xrefs, shared RAM/stack policy, SDK section-end expressions,
  compiler controls and unresolved original definition mechanisms.
- [startup-sdk-provenance.md](startup-sdk-provenance.md): SDK/BIOS ownership
  audit of startup and memory code, exact InitHeap trampoline controls,
  all-source signature screening and limits of negative provenance evidence.
- [game-upstream-ast-campaign.md](game-upstream-ast-campaign.md): upstream AST
  search of the twelve remaining GAME functions, the exact entity renderer
  through a consumed matrix pointer, reviewed spawn/player improvements and
  rejected semantic false leads.
- [game-filename-permuter.md](game-filename-permuter.md): two strict-exact GAME
  filename builders, upstream finite-combination search, complete-TU sibling
  preservation and the remaining thirteen-function campaign.
- [compiler-warning-triage.md](compiler-warning-triage.md): all eighteen GCC
  uninitialized-local warnings and three missing returns, with caller contracts
  and retail-inherited reads distinguished.

- [cast-owner-reduction.md](cast-owner-reduction.md): fifteen further cast
  removals, 31 source controls, registry reader/writer bounds, and unchanged
  production objects; the merged tree has 533 casts, including 430 pointer casts.
- [cast-floor-review.md](cast-floor-review.md): historical 542-site decision
  pass, fifteen reproduced controls and complete shipped-resource bounds
  census; superseded by the owner and storage reduction.
- [cast-reduction-review.md](cast-reduction-review.md): complete 729-site
  review, 187 further cast removals, and unchanged instructions and scores.
- [type-cleanup.md](type-cleanup.md): production assertion removal, five
  union removals, shared resource interfaces, rejected cast substitutions,
  and measured matching impact.
- [remaining-data-owner-exhaustion.md](remaining-data-owner-exhaustion.md): all
  22 failed owners audited, renderer contribution closures, private CD/audio
  workspaces, native small initializers and per-owner evidence limits.
- [data-module-continuation.md](data-module-continuation.md): private menu
  packet storage, native initialized/small-BSS sections, and consolidation
  preserving all 35 function bodies.
- [data-repair-campaign.md](data-repair-campaign.md): complete pool/allocator
  extents, native small-BSS allocation, private reservations, and remaining
  initialized-data, sound-selector and jump-table boundaries.
- [data-owner-recovery.md](data-owner-recovery.md): native ASPSX alignment,
  literal contribution boundaries, initialized small-data ownership and the
  complete GAME/OPEN strict data-owner verdict ledger.
- [executable-byte-provenance.md](executable-byte-provenance.md): native PSX
  initialized bytes, CPE2X uninitialized-memory disclosure, retail header/tail
  consumers, complete-converter and DOS controls, and the retail date window.
- [boolean-modeling.md](boolean-modeling.md): whole-source libclang value-flow
  census, Boolean storage types, reviewed conversions and numeric exclusions.
- [typed-storage-cast-campaign.md](typed-storage-cast-campaign.md): shared object
  models, complete cast accounting, and verified packet/colour conversions.
- [type-assertion-and-union-audit.md](type-assertion-and-union-audit.md):
  assertion-removal compiler controls and a complete review of the 43 unions,
  distinguishing representation conveniences from supported payload variants.
- [typed-menu-records.md](typed-menu-records.md): shared screen points, complete
  glyph rows and list titles through all menu consumers and by-value labels.
- [typed-effect-records.md](typed-effect-records.md): one shared effect pool
  record for construction, rendering, rotation and direction mutation.
- [typed-world-transforms.md](typed-world-transforms.md): complete map vectors,
  shared actor/effect rotations and compatible angle helper interfaces.

- [typed-map-links.md](typed-map-links.md): shared byte, word, gold, spawn and
  bounded item-array views for complete map link records.
- [typed-weapon-render.md](typed-weapon-render.md): typed weapon projection,
  translation and SDK rotation fields, with all compared bodies unchanged.
- [typed-position-interfaces.md](typed-position-interfaces.md): shared SDK
  vectors across actor, effect, player, map and targeting interfaces.
- [typed-screen-coordinates.md](typed-screen-coordinates.md): shared packed
  word and SDK halfword views across GAME and OPEN projection consumers.

- [game-era-source-experiment.md](game-era-source-experiment.md): all 21
  non-exact GAME functions, 133 period-style C attempts, two new exact matches,
  a reproduced statistics-header match, and per-function negative results.
- [game-second-source-pass.md](game-second-source-pass.md): seventeen remaining
  GAME functions, forty-four complete-operation source attempts, raw-byte
  negative controls and evidence-limited verdicts without new exact matches.
- [game-json-hypothesis-campaign.md](game-json-hypothesis-campaign.md): JSON
  source controls, per-function verdicts and exact actor-awareness wait join.
- [game-branch-integration.md](game-branch-integration.md): integration verdicts,
  preserved newer source models and current validation of four partial functions.
- [game-low-trial-campaign.md](game-low-trial-campaign.md): fixed campaign of
  eight non-exact functions with fewer than six recorded JSON trials at selection.
- [gcc257-instrumentation.md](gcc257-instrumentation.md): source-pinned debug
  compiler, observational trace contract, controlled probes and OPEN residues.
- [gcc257-address-quantities.md](gcc257-address-quantities.md): observed CSE
  address inputs and quantity constants, independent debugger controls, and
  production assembler-profile parity.
- [open-dispatch-constant-lifetimes.md](open-dispatch-constant-lifetimes.md):
  exact OPEN constant-sharing control, local versus global allocation, and
  the rejected scroll-enable equality's new reload requirement.
- [gcc257-stack-object-alignment.md](gcc257-stack-object-alignment.md): OPEN
  automatic aggregate spacing, used extents and limits on inferring capacities
  or compact local groupings from stack gaps.
- [open-address-arithmetic-review.md](open-address-arithmetic-review.md): two
  rejected direct vertex lookups and traced unused stack slots; cleanup remains open.
- [reconstruction-debt-review.md](reconstruction-debt-review.md): owner
  recovery replaced by CSE-related constant addressing, the stale-pseudo stack
  slot behind the OPEN vertex arithmetic, load-then-advance stdarg macros,
  aligned formatter scratch, and typed projection/morph arrays.
- [game-save-ui-matches.md](game-save-ui-matches.md): status-result and dialogue
  pathname trials, plus delay-slot and unseen-prologue trace controls.
- [game-status-result-traces.md](game-status-result-traces.md): exact status
  message mapper through its incomplete C return contract; all callers discard
  the result, with raw-byte and forwarding/fallthrough controls.
- [game-dialogue-path-traces.md](game-dialogue-path-traces.md): rejected
  independent directory-byte and advancing-cursor controls, with three-compiler
  parity and preservation of all 23 sibling bodies.
- [game-experience-traces.md](game-experience-traces.md): rejected scalar-field
  pointer control for the level loop; twelve exact siblings remain unchanged.
- [game-world-count-traces.md](game-world-count-traces.md): byte-count and
  explicit action-join controls emit the unchanged serialized world writer.
- [game-entity-cull-traces.md](game-entity-cull-traces.md): shared wrapped
  coordinates and texture-page lifetime recover the retail body size and
  ordered references; small controls reproduce both changes.
- [game-ui-trace-matches.md](game-ui-trace-matches.md): exact configuration and
  buy/sell panels through shared control flow and the real selection-result
  channel; rejected notification-tail and sell-navigation trials.
- [game-selection-trace-matches.md](game-selection-trace-matches.md): exact
  drop-item and active-spell selectors through the confirmation-result channel
  and direct equipment-field references; all six unit functions verified.
- [game-root-item-use-traces.md](game-root-item-use-traces.md): exact root and
  item-use panels through the magic-result channel and HP-first recovery;
  all three unit functions and the seven switch pointers match retail.
- [game-map-door-sounds.md](game-map-door-sounds.md): direct conditional
  sound calls close the map-object updater; sixteen JSON states and full raw
  sibling/reference controls.
- [game-map-drop-initialization.md](game-map-drop-initialization.md): dropped
  object velocity initialization through the common action-selection exit.
- [game-actor-awareness.md](game-actor-awareness.md): exact actor lifecycle
  update through the shared suppression store, with CSE traversal and small
  constant-sharing controls.
- [game-effect-dispatch-traces.md](game-effect-dispatch-traces.md): effect
  collision/radius ownership, halfword scale assignments and scalar updates;
  all instructions align structurally, with 42 register-word differences.

- [game-horizontal-movement-bearing.md](game-horizontal-movement-bearing.md):
  corrected collision-bearing Z sign, SDK short-vector stack layout,
  and rejected cell-pointer/angle-update controls.
- [semantic-field-names.md](semantic-field-names.md): consumer-backed actor,
  player, map, render, and effect field identities, reused effect storage,
  and the remaining serialized-field questions.
- [source-constant-names.md](source-constant-names.md): domain-specific constant
  names and the reasons for retaining individual classes of inline literals.
- [source-literal-coverage.md](source-literal-coverage.md): completed literal
  naming/accounting scope, current counts and links to all source ledgers.
- [header-literal-ledger.md](header-literal-ledger.md): all 53 headers, retained
  layout witnesses and opaque byte extents, including zero-token headers.
- [effect-homing-mode-domain.md](effect-homing-mode-domain.md): byte homing mode
  through construction and steering, with explicit mixed-payload boundaries.
- [ground-effect-branch-domain.md](ground-effect-branch-domain.md): semantic child
  helper names and branch-role propagation through halfword storage and word calls.
- [save-slot-id-domain.md](save-slot-id-domain.md): logical slot IDs through
  signed-halfword APIs and byte directory tags, separate from physical indices.
- [menu-sound-cue-domain.md](menu-sound-cue-domain.md): typed feedback cues across
  90 callers, the shared stereo volume and corrected retail call accounting.
- [shop-id-domain.md](shop-id-domain.md): shop identity through the optional-byte
  reader, buy/sell panels and price drawing, preserving legacy word parameters.
- [save-slot-overlay-domain.md](save-slot-overlay-domain.md): signed overlay
  selection through save/load drawing and confirmation, retaining range behavior.
- [item-id-domain.md](item-id-domain.md): inventory IDs through equipment fields,
  item lists and player/menu APIs, with explicit integer and resource boundaries.
- [floor-sprite-base-index.md](floor-sprite-base-index.md): correct the floor
  animation selector name and keep sprite indices separate from inventory IDs.
- [item-stock-banks.md](item-stock-banks.md): player/shop row identities and
  the price-column origin, backed by startup and purchase consumers.
- [effect-kind-domain.md](effect-kind-domain.md): effect-kind byte storage,
  constructor/dispatch propagation, and spell/packed-code/index boundaries.
- [map-copy-region-domain.md](map-copy-region-domain.md): five authored map-copy
  regions, a byte selector domain, and all direct/dynamic call boundaries.
- [progress-controlled-map-links.md](progress-controlled-map-links.md): shared
  weapon-door and boss-emitter link names backed by shipped placements.
- [floor3-dialogue-door-link.md](floor3-dialogue-door-link.md): shipped placement
  and retail consumer evidence for character 12 unlocking the floor-3 door pair.
- [map-character-domain.md](map-character-domain.md): character IDs through
  map records and dialogue, with three exchange roles and explicit encodings.
- [map-event-animation-turn-domains.md](map-event-animation-turn-domains.md):
  byte enum fields for base/interaction clips and pending collision turns.
- [floor-item-billboard-selector.md](floor-item-billboard-selector.md): shared
  name for the packed-facing zero selector in GAME and OPEN.
- [map-image-groups.md](map-image-groups.md): typed signboard/inscription image
  groups, the shop sequence index and current map-script ledger reconciliation.
- [game-render-setup-literal-ledger.md](game-render-setup-literal-ledger.md):
  complete retained-literal accounting for GAME renderer setup and frame resets.
- [game-effect-update-constants.md](game-effect-update-constants.md): effect
  motion, collision/audio units, typed helper boundaries and the complete
  [105-occurrence remainder ledger](game-effect-update-literal-ledger.md).
- [game-effect-dispatch-constants.md](game-effect-dispatch-constants.md): full
  dispatcher tuning/phase audit, parent-child timing, random-call order and
  the [133-occurrence remainder ledger](game-effect-dispatch-literal-ledger.md).
- [game-magic-collision-constants.md](game-magic-collision-constants.md): magic
  launch/targeting units, shared collision-rectangle capacity and the complete
  [52-occurrence remainder ledger](game-magic-collision-literal-ledger.md).
- [overlay-mode-domains.md](overlay-mode-domains.md): separate GAME exit status
  and OPEN mode, typed global/API flow through the raw Exec boundary, and the
  [154-occurrence OPEN initializer ledger](open-render-init-literal-ledger.md).
- [open-input-action-domain.md](open-input-action-domain.md): typed opening
  input actions through the shared state and controller; verification deferred.
- [visibility-screen-domains.md](visibility-screen-domains.md): shared byte
  visibility classes through GAME/OPEN and typed system-screen selection;
  [complete map-cell remainder ledger](map-cell-literal-ledger.md).
- [sprite-cue-tmd-slots.md](sprite-cue-tmd-slots.md): typed sprite depth-cue
  modes and TMD registry slots; [OPEN sprite literals](open-sprite-literal-ledger.md).
- [quad-color-types.md](quad-color-types.md): SDK RGB fields through simple
  quad APIs and fade callers; [retained literals](fade-quad-literal-ledger.md).
- [quad-rectangle-types.md](quad-rectangle-types.md): unsigned screen rectangle
  fields through quad builders, fades and scrolling scene callers.
- [ending-scroll-states.md](ending-scroll-states.md): typed scrolling enable
  state and the four-tick starfield/panel update cadence.
- [renderer-literal-review.md](renderer-literal-review.md): named HUD layer and
  complete [packet](renderer-packet-literal-ledger.md) and
  [entity/HUD](entity-render-literal-ledger.md) retained-literal ledgers.
- [menu-optional-argument.md](menu-optional-argument.md): named item/shop inputs
  with promoted variadic access for modern compilation.
- [floor-item-random-frame.md](floor-item-random-frame.md): the shared random
  frame scale; [small-module ledger](small-module-literal-ledger.md).
- [palette-domains.md](palette-domains.md): distinct GAME/OPEN palette enums,
  typed selector APIs and explicit table-index boundaries.
- [game-lighting-literal-ledger.md](game-lighting-literal-ledger.md): complete
  117-occurrence lighting/fade accounting, seven retail color matrices, signed
  fog rounding and nine exact function reviews.
- [save-result-domains.md](save-result-domains.md): distinct internal/menu/cleanup
  result channels, typed formatting confirmation and shared union storage;
  20 named values with unchanged 32-function verification.
- [menu-confirm-state.md](menu-confirm-state.md): shared idle/requested domain
  across menu input and drawing, 67 named values, unchanged 38-function check,
  and [complete root/list/save ledger](menu-root-list-save-literal-ledger.md).
- [collision-ledger-reconciliation.md](collision-ledger-reconciliation.md): current
  four-file, 322-occurrence coverage; actor jump's biased height-table consumer
  and unchanged nine-function verification.
- [game-world-collision-literals.md](game-world-collision-literals.md): world
  query/grid remainder audit, all 255 height-table entries and the complete
  [270-occurrence ledger](game-world-collision-literal-ledger.md).
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
- [game-render-hud-constants.md](game-render-hud-constants.md): atlas-backed HUD
  slots, lighting consumers, culling and depth units; complete
  [223-occurrence remainder ledger](game-render-hud-literal-ledger.md).
- [open-render-constants.md](open-render-constants.md): shared scene model IDs,
  render-time yaw, depth and gradient placement; complete
  [110-occurrence renderer ledger](open-render-literal-ledger.md).
- [game-notification-identities.md](game-notification-identities.md): all 32
  retail message labels, typed queue/link fields and variadic API, separate
  phase enum, and the [retained-literal ledger](game-notification-literal-ledger.md).
- [game-map-dialogue-state.md](game-map-dialogue-state.md): GAME dialogue
  stages/pages, animation phase, byte enum domains and corrected call evidence.
- [game-map-event-motion-constants.md](game-map-event-motion-constants.md):
  wandering units, cone ranking, audio distances and restored random positions;
  complete 129-occurrence event/query/load ledger.
- [game-map-event-literals.md](game-map-event-literals.md): map-event movement,
  queries, saved fields and the complete three-module retained-literal ledger.
- [game-map-script-state.md](game-map-script-state.md): typed saved floor-script
  fields, area-trigger stages, revival, map-piece reveals and boss progression.
- [game-map-script-literals.md](game-map-script-literals.md): floor-script
  literal ledger, cutscene units and phases, interaction bounds and menu modes.
- [game-map-script-motion-constants.md](game-map-script-motion-constants.md):
  fade/rotation rates, transform heights, camera interval and interaction
  probes; complete 246-occurrence floor-script ledger.
- [game-map-interaction-loops.md](game-map-interaction-loops.md): container
  traversal ownership, modular pitch samples, partner-search joins and
  controlled prescan/CSE evidence; dispatcher retained at 99.211440%.
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
  shared preview rotation, draw-rate units, Q12 lighting and retained literal ledger;
  pickup corner ordering recovers the complete tile/enqueue suffix at 99.570850%.
- [game-menu-preview-source-controls.md](game-menu-preview-source-controls.md):
  audited name-row ordering residue and recovered sell/buy price-selection CFG.
- [game-shop-menu-controls.md](game-shop-menu-controls.md): separate typed shop
  actions and panel phases, named list results and the nine-row viewport.
- [game-menu-confirm-choice.md](game-menu-confirm-choice.md): typed accept/decline
  choices across input and rendering, with distinct outcome domains.
- [game-menu-confirm-literal-ledger.md](game-menu-confirm-literal-ledger.md):
  current per-occurrence confirmation and item-module literal accounting.
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
  and the complete graphics owner reproduce the entire actor body.
- [compiler-attribution.md](compiler-attribution.md): a dedicated
  cross-version pass over the residue corpus. How GCC 2.4.1 CC1PSX was made
  runnable (a go32-stub frankenbinary under DOSBox) and the finding that it
  reproduces retail no better than 2.5.7 `-O2` (byte-identical on most residue
  functions, worse where it diverges); the residue ceiling is an unattributed
  scheduling/regalloc residue, and `-mcpu=r2000` is a per-TU discriminator, not
  a global truth.
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
- [objdiff-local-branch-comparison.md](objdiff-local-branch-comparison.md):
  reject a false exact MIPS jump hidden by a function shift and local label;
  compare aligned destinations before accepting equal instruction bytes.
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
  an inline dequeue boundary restores the initial tail load and closes the unit.
- [game-warp-mode-domain.md](game-warp-mode-domain.md): checked shimmer modes,
  constructor kind identity and the renderer's Y-scale/yaw consumers.
- [game-warp-literal-ledger.md](game-warp-literal-ledger.md): complete current
  warp and lifecycle literal accounting, including authored motion values.
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
  unconditional view-matrix rebuild; the production migration is documented below.
- [game-enqueue-stack-order.md](game-enqueue-stack-order.md): controlled GCC
  declaration/pseudo/stack evidence closes both remaining textured polygon
  enqueuers without changing expressions or compiler options.
- [game-graphics-runtime-owner.md](game-graphics-runtime-owner.md): production
  graphics owner closes floor placement, sprite enqueue and actor rendering;
  all 173 affected-unit bodies checked, with data placement limits retained.
- [game-display-environment-data.md](game-display-environment-data.md): complete
  SDK-typed environment arrays, reviewed interior references and the screen
  fade's unconditional RGB stores; the later complete owner retains these checks.
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
- [game-actor-radial-falloff.md](game-actor-radial-falloff.md): exact radial
  damage through unconditional loop-entry deficit calculation, with controlled
  constant sharing and loop motion.
- [game-actor-motion-limits.md](game-actor-motion-limits.md): drift acceleration,
  per-axis and yaw speed limits, exclusive home tolerance, and progress-byte roles.
- [game-actor-constants.md](game-actor-constants.md): actor phase limits,
  sound/parameter slots, collision policies, damage units and packed flags;
  consumer explanations for all retained literals in the three actor modules.
- [game-actor-behavior-constants.md](game-actor-behavior-constants.md): actor
  movement and jump units, animation contacts, selection cutoffs, projectile
  parameters and boss-death timing; complete 173-occurrence behavior ledger.
- [game-actor-core-constants.md](game-actor-core-constants.md): combat weighting,
  selection bands and odds, shared contact ranges and sound-table extent;
  complete 174-occurrence core ledger and unchanged 40-function verification.
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
- [game-effect-collision-traces.md](game-effect-collision-traces.md): coordinate
  operand order recovers the rectangle conversion and selected-value registers;
  bounds-return sharing and final query/default scheduling remain non-exact.
- [game-world-collision-source.md](game-world-collision-source.md): early
  typed definition pointers, shared hit value and separate query mask recover
  world-query instructions.
- [game-world-collision-traces.md](game-world-collision-traces.md): a local
  unsigned halfword query mask recovers the incoming word load/copy and closes
  all 321 world-query instructions at strict 100%.
- [game-magic-cast-source.md](game-magic-cast-source.md): case-local objects,
  distinct distance outputs and live projectile speed, followed by exact
  pitch initialization and branch-local height intermediates; all four unit
  functions and five switch targets match retail.
- [game-map-object-clearing.md](game-map-object-clearing.md): corrected
  type-0..8 link predicate and aligned whole-link reset close both clearing
  functions; the earlier memset control emitted an unsupported call.
- [game-map-object-constants.md](game-map-object-constants.md): interaction
  types, running actions, switch phases and transient drop groups; the linked
  motion follow-up maintains the complete remaining-literal ledger.
- [game-map-effect-identities.md](game-map-effect-identities.md): shared
  map-switch effect identity across construction and loading; drop action-band
  limits propagated into saved-floor orientation restoration.
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
- [game-audio-initialize-fill.md](game-audio-initialize-fill.md): the consumed
  signed-halfword voice reset value shared with OPEN restores GAME's setup
  order and the complete initializer body.
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
- [game-stats-header-index.md](game-stats-header-index.md): one consumed glyph
  index across class and status phases recovers eighteen class-calculation
  words; row-spacing materialization and the frame remain non-exact.
- [game-status-render-traces.md](game-status-render-traces.md): a shared
  attack/defense subtotal recovers sixteen rating instructions; the renderer
  remains non-exact, with early coordinate and row-spacing differences.
- [game-display-owner-traces.md](game-display-owner-traces.md): consuming the
  published fog distance leaves both display-owner candidates unchanged; CSE
  still drops the retail DTD base before the fog store.
- [game-status-rating-arithmetic.md](game-status-rating-arithmetic.md): signed
  attack/defense rating divisions and reviewed class-selection jumps restore
  the detailed status renderer's arithmetic branches and ordered references.
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
- [game-menu-window-rows.md](game-menu-window-rows.md): exact window renderer
  from composed background-call branches and direct typed row addresses.
- [game-menu-sprite-blit.md](game-menu-sprite-blit.md): thirteen-caller audit
  and shared signed point/dimension recovery; the genuine SDK rectangle macro
  closes both blitters while preserving every narrow load and ordered store.
- [game-preview-traces.md](game-preview-traces.md): conditional price values
  recover twenty detail-renderer words; both name-copy loops remain partial.
- [game-item-database-traces.md](game-item-database-traces.md): three decimal
  filename controls emit the same non-exact loader; no source change retained.
- [game-item-menu-dimensions.md](game-item-menu-dimensions.md): shared item,
  stock and glyph-row extents, complete-object startup clears, CD sector units,
  and the [database/startup literal ledger](game-item-database-literal-ledger.md).
- [game-player-progression.md](game-player-progression.md): training and stat
  limits, runtime growth-table extent and extrapolation, spell-learning
  requirements, and a complete progression/stat literal ledger.
- [game-experience-loop-addresses.md](game-experience-loop-addresses.md): bounded
  loop and typed-player-view controls do not recover the simultaneous player
  and growth-field address lifetimes; the experience body remains non-exact.
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
  cancellation-condition control leaves the remaining comparison unchanged;
  the subsequent root/item trace campaign closes the function.
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
  and traversal order recover the inventory back-edge delay slot; the later
  selection-trace campaign closes the remaining register and address differences.
- [game-status-panel-flow.md](game-status-panel-flow.md): reviewed frame-head
  jump, shared settling increment and natural corner order recover the
  status-panel CFG; descriptor addressing and frame extent remain non-exact.
- [menu-backdrop-stack-frame.md](menu-backdrop-stack-frame.md): isolates the
  shared 64-byte menu-tile frame residue and rejects debug, GCC 2.6.0, and
  materialized typed-aggregate explanations.
- [game-list-confirmation-flow.md](game-list-confirmation-flow.md): current-pad
  lifetime, positive edge order, explicit toggle and first-frame/post-input
  redraw entries close the shared list-confirmation loop.
- [game-item-use-flow.md](game-item-use-flow.md): restores empty-list input,
  confirmation reset and long-list upward wrap; direct name-row indexing and
  loop shape recover all transfers; the subsequent root/item trace campaign
  closes the seven-word healing residue.
- [game-shop-panel-flow.md](game-shop-panel-flow.md): restores buy/sell
  confirmation reset and long-list upward wrap; subsequent GCC trace work
  closes both panels with all 689 retail instruction words matching.
- [game-magic-panel-flow.md](game-magic-panel-flow.md): direct spell-name
  indexing, cancellation-first handling, cursor branches and a shared normal
  return close the magic panel while preserving its distinct failure exits.
- [game-equipment-spell-selector-flow.md](game-equipment-spell-selector-flow.md):
  pre-switch stock-bank acquisition closes equipment selection and its two
  tables; indexed spell names restore cursor order, and the later
  selection-trace campaign closes the active-spell panel.
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
- [open-era-source-experiment.md](open-era-source-experiment.md): 36 period-style
  C alternatives; actual SDK RGB macros and direct DRAWENV arrays close display
  initialization, while ending scroll retains its seven-word residue.
- [open-sentinel-sharing.md](open-sentinel-sharing.md): retained color-argument
  lifetime, seven-word working residue, and shared-sentinel source controls.
- [open-inline-struct-floating.md](open-inline-struct-floating.md): inline
  helper boundaries, local lighting/geometry structs, and bounded float/double
  arithmetic controls for the two remaining OPEN functions.
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
- [game-world-state-address.md](game-world-state-address.md): reader/writer
  floor-address components and link countdown recover the writer's first
  356 bytes; strict 97.528730%, with unchanged serialized state and exact siblings.
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

- [GAME menu-runtime constants](game-menu-runtime-constants.md): backdrop resource
  geometry, sprite offsets, primitive brightness and distinct list defaults;
  [complete retained-literal ledger](game-menu-runtime-literal-ledger.md).

- [GAME item-detail and dialog constants](game-menu-detail-constants.md):
  save-slot overlays, summary geometry and shared numeric spacing;
  [complete retained-literal ledger](game-menu-detail-literal-ledger.md).

- [GAME list, status-panel and map geometry](game-menu-panel-geometry.md):
  shared backdrop dimensions, list row/quantity offsets and map crop/resources;
  [complete panel ledger](game-menu-panel-literal-ledger.md).

- [Grouped retained-literal reconciliation](game-retained-ledger-reconciliation.md):
  complete current accounting for player death/vitals, item use and notifications.

- [GAME item-use range and effect settings](game-item-use-constants.md):
  shared interaction policy, mirror targeting, staff reload and harp recipes.

- [GAME floor enum domain](game-floor-enum-domain.md): one floor identity across
  byte/word storage and APIs, explicit encoding boundaries and compiler rejection controls.

- [GAME object-model domain](game-object-model-domain.md): resource-backed model
  identities and enum propagation through runtime objects, actor drops and rendering.

- [GAME map-object state domains](game-map-object-state-domains.md): distinct
  behavior/action types, typed door mapping and retained behavior-3 evidence;
  [current occurrence ledger](game-map-object-literal-ledger.md) covers both modules.

- [OPEN model enum domain](open-model-domain.md): separate model IDs through
  entity storage, lookup and rendering, with resource-backed geometry names;
  [entity-pool ledger](open-entity-pool-literal-ledger.md) covers its retained literals.

- [OPEN scene control domains](open-scene-domains.md): transition, lighting
  and sequence enum propagation; [complete scene ledger](open-scene-literal-ledger.md)
  accounts for all 592 retained occurrences, including initialized data.

- [GAME statistics text layout](game-status-text-layout.md): view-specific row
  spacing, shared decimal widths and digit advance; the
  [current status ledger](game-menu-status-literal-ledger.md) includes corrected rating arithmetic.

- [game-save-status-domain.md](game-save-status-domain.md): typed internal card/file statuses,
  preserved menu result mappings, signed-halfword message dispatch and the
  [complete save literal ledger](game-save-system-literal-ledger.md).

- [Player sound-table slots](game-player-sound-slots.md): caller-proved weapon-attack,
  death and level-up selectors and the [complete current core ledger](game-player-core-literal-ledger.md).

- [Player option domain](game-player-option-domain.md): saved configuration bytes,
  typed menu copies, exact-one versus nonzero consumers and the
  [complete configuration ledger](game-menu-config-literal-ledger.md).

- [HUD and effect-sprite states](game-hud-sprite-state.md): separate byte enum
  domains, preserved traversal rules and explicit compass-option conversion.

- [Inventory and shop preview layout](game-menu-preview-layout.md): shared model placement, text origins, row pitch and numeric widths; [complete inventory-preview ledger](game-menu-inventory-preview-literal-ledger.md).

- [Animation-cache and camera coverage](game-animation-cache-camera-coverage.md): serialized reversal evidence, GAME/OPEN point origins and a [complete forty-occurrence ledger](game-animation-cache-camera-literal-ledger.md).

- [Audio sequence domains](audio-sequence-domains.md): separate playback state and
  stop-mode enums, typed call sites and compiler rejection controls; the
  [complete audio/resource ledger](audio-sequence-literal-ledger.md) covers four modules.

- [Resource loading and remaining OPEN audio coverage](resource-audio-coverage.md):
  CD completion semantics, path offsets and voice dispatch, with a
  [complete five-module ledger](resource-audio-literal-ledger.md).

- [Equipment confirmation and name-column layout](game-equipment-confirm-layout.md):
  typed pending requests, fixed category rows and the
  [complete equipment ledger](game-equipment-literal-ledger.md).

- [Matrix/vector/color coverage](math-literal-coverage.md): explicit component
  arithmetic, corrected RGB555 signature and caller limits, with the
  [complete nine-file ledger](math-literal-ledger.md).

- [Formatter state domains](formatter-state-domains.md): separate digit, parser
  and padding enums, preserved custom grammar and clarified scratch ownership,
  with the [complete three-file ledger](formatter-literal-ledger.md).

- [Allocation mode and startup coverage](allocation-mode-domain.md): typed
  shared allocator API and all nine mode calls, with a
  [complete four-file ledger](allocator-startup-literal-ledger.md).

- [Actor culling and placement stream state](actor-culling-domain.md): recover
  actor byte +2 as a visibility policy, type it through both writers and its
  renderer, and complete the [actor-pool ledger](actor-pool-literal-ledger.md).

- [Typed player and dialogue packed state](typed-packed-state.md): shared byte,
  halfword and word views preserve packed comparisons and physical owners.

- [Typed resource allocation boundaries](typed-resource-boundaries.md): generic
  allocation outputs and stream variables share the actual void-pointer contract.

- [Typed equipment and magic tables](typed-record-tables.md): full word-copy
  spans and typed record arrays share the proven complete table extents.

- [Typed matrix elements](typed-matrix-elements.md): GAME adopts the SDK cell
  pointers of its exact OPEN twin; all 12 functions retain their retail words.

- [Typed animation vertices](typed-animation-vertices.md): shared SDK/word
  vertex views, a typed cache-record result and the VDF header blend; all 51
  compared functions preserve their linked instructions.

- [Typed map grids and definition tables](typed-map-tables.md): complete grid
  byte/cell/word views and actor/map-definition table inputs; all 177 compared
  functions retain their instructions and referents.

- [Typed selected TMD headers](typed-tmd-headers.md): registration and selected
  assets use the 12-byte header type; serialized byte offsets keep explicit
  decoding boundaries, and all 76 compared functions retain their words.

- [Projected addresses and sprite packets](typed-projected-addresses.md): exact OPEN sprite color view and controlled byte-pointer, owner and packed-XY residue comparisons.

- [Shared allocator state](typed-memory-owner.md): one arena owner per image, shared cursor/stack fields, exact instruction controls and explicit OPEN BSS placement residue.

- [Redundant casts after type propagation](typed-redundant-casts.md): seven identity conversions removed with all 27 compared bodies unchanged.

- [Typed menu ordering-table indexing](typed-ordering-index.md): two byte-pointer casts removed, with all sixteen compared bodies unchanged.

- [Allocated save workspace](typed-save-workspace.md): complete header/payload allocation with independent public buffer views and exact null-path behavior.

- [Final typed-storage cast audit](typed-cast-audit.md): complete 451-site classification, whole-campaign byte preservation and explicit remaining evidence limits.

- [GCC 2.5.7 typedef comparisons](gcc257-typedef-comparisons.md): a real-function
  control separates the s32 typedef from bare long, with limits on its
  relevance to the two remaining OPEN mismatches.

- [GAME effect-slot sequence helper boundary](game-map-object-sequence-inline.md):
  two acquire/publish inline boundaries preserve the spawn residue.

- [Strict enum domain audit](strict-enum-audit.md): initial field/API campaign,
  shared progress and animation domains, strict Clang compatibility, and
  unchanged retail objects across all 484 functions.

- [Complete Clang field review](enum-field-review.md): 1,046 current declarations,
  a per-field evidence ledger, and all nine findings implemented as shared enums,
  typed flags and component views.

- [Array enum review](array-enum-review.md): every array declaration and explicit
  aggregate initializer, typed overlay argument slots and fixed floor-sprite
  selectors, with unchanged compiled objects.

- [Sony SDK header helpers](sony-header-helpers.md): Release 2.5 macro/inline
  census, candidate source families, retail GTE negative controls and the
  initial sprite/camera pilot.
- [Complete Sony helper campaign](sony-header-helper-campaign.md): all 147
  candidates reused, signed direction views, exact GAME display initialization,
  corrected switch-table addends and per-function verdicts.
- [Sony helper near misses](sony-helper-near-misses.md): reordered triples,
  chained sprite corners and scalar controls; three more player helpers remove
  the remaining operation-order differences, leaving an eight-byte frame residue.
- [Remaining Sony helper investigation](sony-remaining-functions.md): all eighteen
  non-exact functions reviewed, thirty compiled controls, and three exact menu
  functions from reflected SDK UV rectangles and byte-domain texture dimensions.
- [SDK expressions and the final fifteen](sony-final-fifteen.md): refreshed
  evidence for every remaining function, twenty-two additional source controls,
  and native-parity traces of the successful UV expressions' stack allocations.
- [July 1994 Sony SDK header control](sony-sdk-july-1994.md): earlier original
  headers and samples, unchanged helper expressions, and complete-object
  equality for all fifteen functions under the older GTE declaration boundary.
- [Remaining game source follow-up](game-final-fifteen-followup.md): sequence
  expression, complete filename-owner and nine-function rendering-context
  controls, with preserved exact siblings and no additional exact function.
- [Shared constant review](shared-constant-review.md): evaluated enum comparison,
  semantic decisions for equal values, and shared definitions across TUs.
- [Complete enum equality review](enum-equality-review.md): decisions for all
  179 remaining groups, exact membership coverage, and consumer encoding aliases.

- [Complete enum reuse review](enum-reuse-review.md): all 302 starting enum blocks
  reviewed, direct shared domains and constants, complete member/value coverage,
  and preserved bytes and strict results for every reconstructed function.

- [Native COMMON symbol names](native-common-symbol-names.md): a controlled global
  rename changes native COMMON placement and 221 GAME executable bytes while
  preserving compiled sections, ordered referents, and strict objdiff results.

- [Unknown-field review](unknown-field-review.md): all 79 header declarations
  accounted for, explicit raw-reference coverage limits, resource negative
  controls, and Rust codec fields reconciled with proven C meanings.
- [Unknown-field format provenance](unknown-field-format-provenance.md): all
  33 owners checked against pinned SDK types and Sony format manuals, with
  embedded-type boundaries and documentation-version discrepancies recorded.
- [Unknown-field alias audit](unknown-field-alias-audit.md): scoped floor-item
  and event pointer paths, wide-copy lanes, dialogue-index bounds, persistence
  and animation-cache owner indirection checked against retail instructions.
