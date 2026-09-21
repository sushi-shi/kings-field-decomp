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

## First pass: domain helpers and runtime fields

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
  signed shift changes rounding. Vector value assignment initializes unused
  layout padding; partial updates still change only their intended components.
- Transition colors retain byte addition and wrap behavior, not a new saturating
  clamp. Path formatting writes exactly three characters, without a terminator.
  Sequence release also clears the sequence pointer and active state. Vital
  fraction drawing preserves glyph mutation, cursor movement and draw order.
- Replacing a macro must account for argument evaluation and aliasing. The
  converted call sites use ordinary values/references; this is not permission
  to apply the same conversion mechanically to other macros.

## Stacked follow-up passes

Each pass targets the preceding branch, beginning with PR #18 into `port`.
Review and merge in that order; these are source cleanup, not gameplay features.

### Pools and object copies

Actor/effect free-slot searches, resets, death selection and effect/event/object
updates now use forward array traversal. Index-returning actor searches retain
explicit indices. Opening lookup is a conventional sentinel-terminated loop,
not a fixed-capacity scan. Current-object bindings and live iteration are unchanged.

Five definition/equipment table loaders now assign their complete typed objects;
weapon yaw mirroring remains a separate ordered pass. Object links are explicitly
cleared over their eight-byte object extent. This removes word-pointer aliasing
without changing resource decoding or resetting other fields in a reused slot.

### Player update operations

`player_update` now coordinates named, module-local movement, view, darkness,
damage-reaction, equipment, floor-hazard and status-effect operations. Loaded-game
restoration has its own operation. The extracted bodies retain their original
widths, comparisons and side-effect order; movement remains forward then strafe,
and view bob remains before button-driven turning. Death/menu early returns,
occupancy changes and the common post-menu tail remain in the coordinator.

### Rendering visibility

GAME's five entity passes now share a cell-window predicate, with a separate
actor square-culling predicate. Cell subtraction still narrows to `u16` before
bounds checks. The active window is consulted at each check while the frame's
origin stays fixed. Object/actor/floor-item/effect/event ordering, material and
lighting setup, and visible-only floor-item animation updates are unchanged.
GAME/OPEN entity arrays use forward iteration; floor items use the loader-validated
active count. No renderer backend or face sorting changes.

### Vector value types

`setVector`, `copyVector`, `addVector` and the interim set/copy/add wrappers have
been removed. `VECTOR`/`SVECTOR` are small value types: initialization and copies
use direct assignment, addition uses `+=`, and `narrowed()`/`widened()` make
16/32-bit conversion explicit. Narrowing retains truncation, not saturation.
Constructors initialize the unused fourth field; copying copies it. The fourth
field has no gameplay, rendering or save consumer in the audited source. Packed
sizes/offsets, standard layout, trivial copying and trivial default construction
are checked in the type definitions. Union-member assignments name the owner.
Whole-vector rewrites are limited to independent component expressions; partial
updates, dependent assignments and RNG ordering remain explicit. Addition still
reads and writes components in XYZ order. No inheritance or generic vector
framework is introduced.
The sole `limitRange` caller uses `std::clamp<s32>` after the original charge
increment/narrowing, preserving that operation's value range and ordering.

### Explicit state access

Map/effect aliases now name their owning state objects directly. Floor-script
storage has a typed reference-returning accessor; member writes still name the
owning script union. No duplicate globals or copied snapshots were introduced.
The floor-item loader uses a small view of the live count and fixed array.
Projected-vertex lookup is
a typed OPEN helper. Word-count/emitter constants are `constexpr`; boolean
aliases are removed in favor of `true`/`false`. Preset selection lives at each
palette owner with its actual enum type, without a conditional type alias.
Include guards and schema/resource macros remain distinct from runtime state
aliases; the later ownership pass removes compile-time module selection.

### Menu navigation and dialogue exchanges

List movement and its shared input/sound operation live in `src/game/menu_list.cpp`.
Seven callers retain their empty-list and cancel branches, Up-before-Down
priority, preview reload failures and independent menu loops. Dialogue exchanges
share eligibility and response/advance operations, but retain each exchange's
grant/consume/notification order. The speaking event and the event whose dialogue
is advanced remain separate identities, including across the blocking dialogue.

### Shared camera-path operations

Segment calculations and fixed-point pose publication/advancement now live in
`src/lib/camera_path.cpp`, compiled once by the later ownership pass. GAME still fetches
then increments its point cursor and starts at the player's camera; OPEN still
increments then fetches and starts at path point zero. Sentinel handling,
frame decrement/transition order, arithmetic widths and offsets are unchanged.
Scene pacing and camera-path data are not changed.

### One-program shared-code ownership

`KF_OPEN` and all its function-presence guards are removed. `build.json` lists
common sources once, separately from opening/gameplay scene policy. Common
camera, math, file, memory, formatting, display, TMD and audio operations have
one compiled implementation. Shared types are declared outside phase namespaces;
the menu-specific enum encoder explicitly extends the common overload set.

Stateful calls supply their actual owner. TMD contexts borrow the selected
phase's slots, current resource/vertices and fixed projected array; opening still
has two slots, gameplay eight. Contexts are call-local views, not persistent
references that survive re-entry. Render state and display types are shared;
gameplay-only lighting matrices remain in gameplay storage. Five unused packet
counters with no readers or increments are removed. Frame waits, buffer-index
changes and presentation order are unchanged.

Each phase keeps its own arena, including its backing allocation. Resource
loads and rewinds take that arena explicitly; re-entry and exit destroy the same
owner as before. Floor-item loading receives its own count/array and height grid;
drawing receives the view, sprites and phase-specific sprite producer. Animation
still advances only after a visible item is submitted.

Audio resource decoding, sequence/bank release and voice-slot operations are
shared. A call-local playback view supplies the voice owner and current effects
option (always enabled for opening). Phase-owned panning, sequence fades, bank
settling, palette tables and initialization values remain distinct. Formatting
scratch storage is shared and reset at each phase entry; no caller retains its
results across a phase transition. No global active-phase selector replaces the
removed preprocessor flag.

Linux and WASM link after this pass. Linked-symbol inspection confirms single
definitions of the inspected shared operations. Independent review covered the
full ownership diff, its callers and reset chains; its header self-include
finding is fixed. No gameplay or timing comparison was performed for this pass.

## Remaining cleanup boundaries

These need their own behavior/type or loader-boundary work, not cosmetic wrappers.
They are not prerequisites for using the current Linux client.

### Collision result modeling

After the overlapping switch/signedness work, tackle
`map_object_probe_forward` and its four remaining jumps together with the result
representation. Inventory producers and consumers first: high-word kind/detail,
shifted rejection flags, all-ones no-hit and literal-one results are not one
uniform bitset. Some existing actor checks compare a high word to a detail
constant; retain that behavior until its meaning is established. Do not conceal
it behind a confidently named but inaccurate predicate.

### Bounded resource traversal

Replace `RESOURCE_STREAM_NEXT` as part of a loader-boundary change carrying
resource ends and preserving arena/rewind lifetimes. A typed wrapper around an
unchecked cursor is not a sufficient cleanup. OPEN's TMD packet macros still
have callers; separate file-format decoding from runtime draw construction when
addressing them, rather than deleting them as obsolete SDK leftovers.

### Further local readability work

Continue reviewing whole functions when touching remaining actor/script/menu
code. Keep unknown semantics explicit; do not add a helper for every field or
merge differing state machines merely because they look similar. Retain sentinel
searches and live pool updates where those express the actual operation. This
stack addresses the identified repeatable helper/macro families, not a claim that
every long function or raw expression in the codebase has been eliminated.

## Review follow-ups and ownership boundaries

PR #23 also names the floor-item sprite-range check and the trained-spell unlock
operation. The range check keeps zero-frame records valid, bounds the first
sprite before subtraction, and leaves error handling and allocation order alone.
Spell checks retain the Healing prerequisite for Dispoison, base-magic thresholds,
Dispoison/Fire Wall/Lightning Bolt order, and one notification per new spell.

The following work belongs above the current stack rather than expanding #23's
state-alias review into an application/rendering rewrite:

1. **One-program shared-code ownership:** implemented above, with explicit
   phase state and no executable-mode selector.
2. **Projected-face operations.** Group the repeated projected-vertex lookup,
   winding check and face-coordinate construction in the opening renderers.
   Preserve the fourth-vertex lookup after triangle winding rejection, flat
   versus Gouraud fog inputs (including existing vertex-zero uses), depth
   arithmetic, packet traversal and submission order. A helper should express
   a face operation, not just rename three pointer assignments.
3. **Arithmetic operations.** Replace straightforward bounds with `std::min`,
   `std::max` or `std::clamp` only when narrowing, side effects and comparison
   semantics agree. Existing repeated distance calculations are candidates for
   explicit fixed-point length operations: `fixed_vector2_length` and
   `fixed_vector3_length` shift components before squaring, while movement code
   also has deliberately unshifted square-root inputs. The shared
   `kf::length_square_root` is a quantized mantissa lookup, not host `std::sqrt`;
   preserve that numeric contract. Angle wrapping/approach and damage attenuation
   also have rounding/width rules that ordinary clamp or floating math do not
   express. Keep useful fixed-width aliases; remove aliases that only hide an
   identical native type as those callers are converted (for example face shading).

The rendering/math entries remain an implementation queue, not a claim that all
boilerplate has already been removed.

## Verification scope

Build and link Linux and WASM, inspect compiler diagnostics against the branch
baseline, and independently review the complete diff and relevant callers.
No new unit/parity campaign or automated gameplay input is part of this cleanup.
Static review and successful builds do not close the browser/ending runtime
verification gaps tracked in [port-status.md](port-status.md).

The first pass and complete follow-up stack both passed clean Linux/WASM builds.
Each target retains the baseline's 57 compiler warnings; comparing diagnostics
with worktree paths normalized found none added or removed. Independent
clean-context reviews cover each pass and its relevant callers. No gameplay run
was performed, and build/static-review success is not runtime verification.
