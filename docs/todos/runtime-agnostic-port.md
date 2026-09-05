# Runtime-agnostic game port

## Problem

The reconstructed sources target the original PlayStation execution model:
32-bit MIPS/Psy-Q ABI, fixed RAM addresses, BIOS events, controller polling,
synchronous CD reads, memory-card files, GTE fixed-point operations, GPU packet
lists, and LIBSND/SPU audio. They are compiled as reconstruction objects for
comparison with the retail programs, not linked as a complete host program.

Linux and WebAssembly should not require a reimplementation of the entire
Psy-Q API or a virtual PlayStation. Instead, the game should be adapted into a
runtime-neutral core with small host interfaces and separate Linux and browser
backends.

Some platform boundary is unavoidable: a game must obtain input, time, assets,
persistent storage, graphics output, and audio output from its host. The goal
is a narrow interface expressed in game concepts, not a compatibility layer
that preserves Sony SDK entry points and hardware register behavior.

## Relationship to the matching project

The retail executables remain the behavioral and binary authority. The
reconstructed source is an evidence-backed working model, and exact functions
must not be regressed to make the port convenient.

Keep the matching and porting concerns separate:

```text
src/psx, src/game, src/open     retail reconstruction and object matching
port/core                       runtime-neutral game adaptation
port/platform/linux             Linux process, window, input, and storage glue
port/platform/wasm              browser loop, input, and persistent-storage glue
port/render                     shared host renderer
port/audio                      shared decoding, sequencing, and mixing
```

Code may be shared with `src/` when its natural source form is both portable
and compatible with the retail reconstruction. Otherwise, the portable
implementation should remain separate and name the retail image/function from
which its behavior was derived. Do not fill matching sources with pervasive
`KF_PORTABLE` branches or change proven source structure solely to share code.

Every porting change that also touches reconstruction inputs must still pass
the normal focused matches and full `kf build` regression gate.

## Intended architecture

The host owns the process and event loop. The core owns game state and advances
by one fixed simulation tick when called:

```c
typedef enum KfMode {
    KF_MODE_OPENING,
    KF_MODE_GAME,
    KF_MODE_QUIT
} KfMode;

void kf_core_initialize(KfMode initial_mode);
KfMode kf_core_tick(const KfInputState *input);
void kf_core_render(KfRenderList *output);
void kf_core_shutdown(void);
```

The exact interface is unresolved, but it must have these properties:

- no infinite loop, busy wait, interrupt dependency, or blocking event poll in
  the core;
- a retail-derived fixed simulation cadence rather than variable-delta game
  logic;
- input supplied as a per-tick snapshot;
- asset and save operations represented as explicit requests/results;
- rendering expressed through pointer-free commands or durable handles; and
- audio expressed as sounds, voices, sequences, spatial parameters, and mixer
  state rather than LIBSND calls.

Linux and WASM should run the same core, render implementation, asset decoders,
and audio logic. Their backends should differ only where the host APIs require
it.

## Portability work

### ABI and data layout

- Replace host-facing uses of `long` with explicit `int32_t`/`uint32_t` types.
- Do not cast host pointers into 32-bit integers. Use indices, offsets, or
  handles where the retail format stores an address-sized value.
- Separate serialized/CD asset layouts from in-memory structures. Parse
  little-endian fields explicitly and apply packing only to file-format types.
- Replace type-punning loads and stores with helpers that have defined aliasing
  and alignment behavior.
- Retain fixed-point widths, signed overflow expectations, truncation, and
  shift behavior where they affect gameplay.
- Define all portable runtime storage independently of retail virtual
  addresses. Address-derived names can remain until semantics support better
  names, but their numeric address must not be their storage mechanism.

`wasm32` happens to have 32-bit pointers and `long`, but the core must not rely
on that coincidence. The same code must work correctly on a normal 64-bit
Linux ABI.

### Main loop and lifecycle

- Split initialization, one-frame update, presentation, and shutdown.
- Replace the VSync callback counter and critical-section busy wait with host
  scheduling around a fixed tick.
- Convert multi-frame fades, input press/release waits, CD polling, card event
  polling, and similar blocking loops into explicit state machines.
- Model `OPEN.EXE` and `GAME.EXE` as modes in one program. Preserve the small
  result/state handoff performed by `PSX.EXE`; do not preserve dynamic
  executable loading.
- Permit an initial development mode that starts `GAME.EXE` behavior directly
  while the opening program remains incomplete.

### Input

- Translate keyboard and controller events into one neutral `KfInputState`.
- Preserve the retail active-low button meanings at the translation boundary
  or normalize them once and document the chosen representation.
- Make edge detection deterministic from consecutive tick snapshots rather
  than host event timing.
- Keep controller mapping and rebinding outside gameplay code.

### Assets and filesystem

- Require user-supplied retail data or an extraction result; do not commit or
  distribute game assets.
- Map CD paths such as `\\KF\\...;1` to logical asset identifiers without
  exposing ISO9660 spelling rules to game code.
- Decide whether TIM, TMD, VAB, and SEQ are decoded at installation time or at
  load time. Both platforms should consume the same decoded representation.
- Make browser asset access non-blocking at the host boundary. Prefer loading
  a mode/map bundle before entering core code that expects the data to exist.
- Preserve bounds, field widths, and sentinel behavior from the retail loaders
  even if the host filesystem is more permissive.

### Rendering

This is the largest workstream. Current game renderers construct Psy-Q
primitives, place them in reverse ordering tables, and depend on GTE projection,
lighting, clipping, and depth-cue behavior.

The runtime-neutral renderer should:

- accept explicit triangles, quads, sprites, texture/CLUT references, vertex
  colors, transparency mode, and ordering depth;
- use stable handles rather than GPU packet pointers or 24-bit linked-list
  addresses;
- implement the required fixed-point transform, projection, lighting, clipping,
  fog, and depth rules as ordinary portable math, without emulating GTE
  registers;
- preserve PS1 texture-page, CLUT, semi-transparency, draw-order, and optional
  dithering behavior to the fidelity level selected below; and
- use one renderer on Linux and WASM where practical, with only surface/context
  creation supplied by the platform backend.

Do not convert gameplay coordinates and collision math wholesale to floating
point. A renderer may convert final values for the host GPU after deterministic
game and projection calculations have completed.

### Audio

- Decode VAB/VAG sample data into a neutral sample bank.
- Decide whether SEQ playback is implemented as a portable sequencer or
  converted to another representation during asset import.
- Retain voice allocation, key-on/key-off policy, volume ramps, positional
  panning, and music transitions in shared code.
- Put only device creation, buffer submission, and browser resume/unlock rules
  in the platform backend.
- Treat exact SPU interpolation, reverb, and timing as an optional fidelity
  tier rather than a prerequisite for the first playable build.

### Saves

- Preserve the game payload semantics independently of the PlayStation
  memory-card container.
- Store saves as ordinary files on Linux and persistent browser storage on
  WASM.
- Decide separately whether importing/exporting original memory-card saves is
  required.
- Make writes atomic where the host permits it and preserve the retail
  temporary-save recovery behavior at the game-policy level.

## Suggested milestones

### 0. Scope and fidelity decision

- Choose the first target: main gameplay without `OPEN.EXE` is recommended.
- Decide whether first release aims for gameplay equivalence, close visual
  equivalence, or pixel/audio accuracy.
- Choose the portable build system and supported Linux architectures.
- Define the asset-extraction and distribution policy.

Exit condition: the renderer/audio fidelity target and legal asset boundary
are written down, because both materially change the schedule.

### 1. Host-linkable headless core

- Establish a modern C compile with warnings and sanitizers.
- Introduce portable ABI and endian helpers.
- Provide owned storage for the selected game-state subset.
- Link pure math, map, collision, player, actor, item, and script logic without
  Psy-Q libraries.
- Add deterministic unit and tick tests.

Exit condition: a test can initialize a known state and advance repeatable
ticks without graphics, audio, or host input APIs.

### 2. Non-blocking game lifecycle

- Extract initialization and one-tick progression from the retail-shaped main
  loop.
- Convert the minimum required waits/fades to states.
- Add recorded input playback and per-tick state checksums.

Exit condition: a recorded scenario produces identical checksums across Linux
native and `wasm32` test builds.

### 3. Assets, input, and saves

- Load one extracted map/resource bundle.
- Add neutral input translation.
- Implement host save storage and the game's save payload policy.

Exit condition: the headless game can enter one map, move the player, and round
trip a save.

### 4. First rendered map

- Decode TIM/TMD data.
- Implement fixed-point transforms and the neutral render list.
- Render map geometry, sprites, and the minimum UI on Linux.

Exit condition: a known camera position renders a stable frame and can be
compared with a retail emulator capture.

### 5. Gameplay-complete Linux build

- Port remaining render paths, menus, transitions, and effects.
- Add sound effects and music.
- Exercise every map transition, death/restart path, item/menu operation, and
  save/load path.

Exit condition: the main game is completable without invoking PS1 code or
Psy-Q libraries.

### 6. WASM frontend

- Drive the fixed tick from the browser event loop.
- Add browser input, canvas/context lifecycle, audio activation, asset preload,
  and persistent saves.
- Test memory growth, tab suspension, focus loss, and controller reconnects.

Exit condition: the same game data and deterministic scenarios behave the same
as the Linux build.

### 7. Opening and full lifecycle

- Complete enough `OPEN.EXE` semantics to implement the title/opening flow.
- Reproduce its state handoff to and from main gameplay.
- Replace the `PSX.EXE` loader loop with ordinary in-process mode transitions.

Exit condition: startup, title/opening, new game, return flow, and repeated
mode transitions work without stale state.

### 8. Fidelity and release work

- Compare representative gameplay traces and captures against retail.
- Resolve deterministic divergences before masking them with presentation
  changes.
- Add packaging, user-facing configuration, crash diagnostics, and save
  migration/versioning.
- Document required retail inputs and reproducible extraction.

## Verification strategy

The port needs behavioral evidence in addition to compilation:

- unit tests for fixed-point helpers, format parsing, collision, state
  transitions, and serialization;
- deterministic replay tests using recorded input and state checksums;
- function- or subsystem-level comparisons against reconstructed semantics;
- emulator traces for important state transitions and SDK-facing results;
- image comparisons for representative maps, sprites, menus, fog, and effects;
- audio event traces before waveform-level comparison is attempted; and
- native builds with undefined-behavior and address sanitizers.

Retail reconstruction tests remain separate and authoritative for byte-match
claims. A passing portable test never promotes a reconstruction identity or
match result.

## Planning estimate

Very rough effort for one experienced C/PS1 developer, assuming reusable host
graphics/audio infrastructure and no requirement for pixel-perfect output in
the first milestone:

| Deliverable | Estimated effort |
| --- | ---: |
| Headless portable core | 1-2 person-months |
| Playable Linux main game | 4-8 person-months total |
| WASM frontend after the core works | 1-2 additional person-months |
| Opening, broad parity, fidelity, and release polish | 9-18 person-months total |

The largest schedule variables are rendering fidelity, SEQ/SPU audio fidelity,
the amount of unresolved global/data ownership encountered during full linking,
and whether `OPEN.EXE` reconstruction proceeds in parallel.

## Open decisions

- Is close visual behavior sufficient, or is pixel-accurate PS1 rasterization
  required?
- Must audio reproduce SPU interpolation and reverb, or is musically equivalent
  playback sufficient?
- Should original memory-card images be importable/exportable?
- Are assets decoded at installation time or at runtime?
- Does the first playable build skip `OPEN.EXE`?
- Which deterministic state and frame traces form the initial parity suite?
- Which reconstructed modules are safe to share directly without compromising
  the matching source model?
