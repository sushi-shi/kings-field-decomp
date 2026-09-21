# Authored-style source cleanup

The port keeps the existing game logic, but does not need to retain the source
shapes used for matching PS1 instructions. Read whole functions and their callers
before extracting a helper: the goal is a recognizable operation with a clear
owner, not a shorter spelling for every expression. These are port design
choices, not claims about how the original developers spelled their source.

Use ordinary structures, functions and scoped enums. Pure predicates and small
constant operations can be `constexpr`; stateful operations belong in their
owning translation unit. No inheritance, generic gameplay framework, replacement
state machine or blanket macro-to-function rewrite is needed.

## This cleanup

- Replace domain macros with typed operations: button edges, actor bearing,
  player floor position, equipped-item membership, full-floor classification,
  yaw probes, three decimal path digits, transition color advancement, sequence
  release and menu vital fractions. Remove the two now-empty macro headers.
- Remove runtime word-view unions around player motion, map coordinates and
  dialogue. Compare warp coordinates and dialogue fields directly instead of
  packing them into integers. Keep serialized save fields and their order.
- Use live local references for player motion/cells and named map queries for
  cell attributes and base floor heights. The base-height query is deliberately
  distinct from the step-aware `map_floor_height_at_position` operation.
- Express script rectangle membership as a predicate over cell coordinates.
- Replace seven fixed-pool traversal loops with forward range loops: actor
  update/placement loading/target selection, event loading/target selection,
  object placement loading and opening entity placement loading.
- Extract the armor regeneration/drain rule once, retaining five explicit,
  guarded calls in head/body/shield/arm/leg order. Weapon effects stay separate.

## Behavior-sensitive boundaries

- Movement accepts Z before testing X. The second test must see the updated Z
  cell. Local references remain live across movement, menus and warps; cached
  values must not replace reads that previously occurred after those calls.
- Runtime map coordinates retain their `z, x` field order; named warp endpoints
  use `x, z`. Dialogue-start checks ignore `stage_limit`, as the old mask did.
  Neither runtime type is a new disk or save format.
- A pool traversal visits live slots in ascending order, not a snapshot of active
  entries. Later slots activated during an update can still run that tick.
  Preserve free-slot guards, actor binding (including the final null binding),
  search tie behavior and output-distance defaults.
- Placement sentinels stop resource-cursor advancement, not destination cleanup:
  every remaining destination slot still has its free-state fields set.
- Armor regeneration precedes drain within each slot. Keep each HP adjustment;
  clamping and death side effects make combining the deltas incorrect. Increment
  the equipment timer and read the current map attribute afterward, as before.
- Yaw probes subtract the already-shifted sine product. Negating before the
  signed shift changes rounding. Vector helpers only write the original fields;
  the floor-position operation does not initialize vector padding.
- Transition colors retain byte addition and wrap behavior, not a new saturating
  clamp. Path formatting writes exactly three characters, without a terminator.
  Sequence release also clears the sequence pointer and active state. Vital
  fraction drawing preserves glyph mutation, cursor movement and draw order.
- Replacing a macro must account for argument evaluation and aliasing. The
  converted call sites use ordinary values/references; this is not permission
  to apply the same conversion mechanically to other macros.

## Follow-up queue

These are concrete cleanup candidates from the whole-function survey, not new
gameplay features or prerequisites for using the current Linux client.

### Stacked pass: pools and object copies

Actor/effect free-slot searches, resets, death selection and effect/event/object
updates now use forward array traversal. Index-returning actor searches retain
explicit indices. Opening lookup is a conventional sentinel-terminated loop,
not a fixed-capacity scan. Current-object bindings and live iteration are unchanged.

Five definition/equipment table loaders now assign their complete typed objects;
weapon yaw mirroring remains a separate ordered pass. Object links are explicitly
cleared over their eight-byte object extent. This removes word-pointer aliasing
without changing resource decoding or resetting other fields in a reused slot.

### Stacked pass: player update operations

`player_update` now coordinates named, module-local movement, view, darkness,
damage-reaction, equipment, floor-hazard and status-effect operations. Loaded-game
restoration has its own operation. The extracted bodies retain their original
widths, comparisons and side-effect order; movement remains forward then strafe,
and view bob remains before button-driven turning. Death/menu early returns,
occupancy changes and the common post-menu tail remain in the coordinator.

### Remaining container candidates

Review the remaining actor/event/effect searches and sweeps for range loops,
local references and first-match helpers. Keep current-object binding and live
iteration explicit. Do not convert sentinel-delimited searches to fixed-capacity
loops or change `effect_pool_acquire`'s zero-count behavior without auditing its
callers. Inspect initialization/copy loops by their actual object ownership;
prefer object assignment only when it preserves the intended fields.

### 2. Model collision results at their boundary

After the overlapping switch/signedness work, tackle
`map_object_probe_forward` and its four remaining jumps together with the result
representation. Inventory producers and consumers first: high-word kind/detail,
shifted rejection flags, all-ones no-hit and literal-one results are not one
uniform bitset. Some existing actor checks compare a high word to a detail
constant; retain that behavior until its meaning is established. Do not conceal
it behind a confidently named but inaccurate predicate.

### 3. Replace generic vector macros by audited operation families

Audit `setVector`, `copyVector`, `addVector` and `limitRange` at every caller.
Track sequential writes, aliasing, narrow destinations, and random calls before
choosing overloads or explicit statements. A normal function eagerly evaluates
arguments; a macro may interleave evaluation with destination writes. Avoid a
large template abstraction merely to reproduce that behavior.

### 4. Shorten rendering around shared visibility decisions

Review the repeated cell-window checks in the entity-render passes. A shared
predicate may help, but must preserve narrowing to `u16`, the separate square
culling mode for nearby actors, and the ordered rendering passes. Keep face
ordering, lighting, scratch-buffer ownership and rendering calculations intact.
Do not combine the passes into a new scene/object hierarchy.

### 5. Make resource traversal explicit and bounded

Replace `RESOURCE_STREAM_NEXT` as part of a loader-boundary change carrying
resource ends and preserving arena/rewind lifetimes. A typed wrapper around an
unchecked cursor is not a sufficient cleanup. OPEN's TMD packet macros still
have callers; separate file-format decoding from runtime draw construction when
addressing them, rather than deleting them as obsolete SDK leftovers.

### 6. Extract smaller camera/menu operations where behavior agrees

Consider segment setup helpers shared by GAME/OPEN camera paths, but retain their
different fetch/increment order and initial poses. In menus and scripts, prefer
local formatting, selection and interaction helpers over a common controller:
input contexts, waits, retained frames and resource lifetimes differ. Continue
shortening repetitive nested access using references and meaningful operations,
not generic accessors for every field.

## Verification scope

Build and link Linux and WASM, inspect compiler diagnostics against the branch
baseline, and independently review the complete diff and relevant callers.
No new unit/parity campaign or automated gameplay input is part of this cleanup.
Static review and successful builds do not close the browser/ending runtime
verification gaps tracked in [port-status.md](port-status.md).

For this change, fresh Linux/WASM builds linked successfully. Both retain the
baseline's 57 compiler warnings, with no added or removed diagnostics. A separate
clean-context review covered all 41 changed source/header files and the relevant
callers/callees, with no actionable findings. No gameplay run was performed.
