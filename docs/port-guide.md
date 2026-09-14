# Linux and WebAssembly port guide

## Direction and status

Build the reconstructed King's Field sources with a modern toolchain and a
portability layer supplying the PlayStation services they use. Plan for Linux
and browser WebAssembly from the beginning; Linux is the first complete
playable frontend.

This guide records the architecture and proposed implementation choices, not an
implemented port. It supersedes the earlier proposal to replace the game's
rendering and audio interfaces wholesale. Preserve game-facing Psy-Q interfaces
where practical and implement the required service surface behind them.

Gameplay, resource loading, rendering, menus, and presentation have reconstructed
source. The existing build compares MIPS objects; it does not link a complete
native game. See the [current match status](../README.md#match-status). Its
curated census excludes identified vendored code and does not measure port
completion. Complete byte matching is not required before starting; high fuzzy
similarity does not establish native behavioral correctness.

## Three programs, one application

| Image | Retail responsibility | Native responsibility |
| --- | --- | --- |
| `PSX.EXE` | Alternately load and execute OPEN and GAME | Application lifecycle and mode transitions |
| `OPEN.EXE` | Opening/title presentation and ending sequences | Opening mode and its state |
| `GAME.EXE` | Playable game | Gameplay mode and its state |

The executables reuse physical PlayStation RAM. Loading another executable does
not inherently clear all RAM. An address is meaningful only with its image;
complete isolation between programs has not been established.

The [bootstrap](../src/psx/main.c) keeps a two-word argument/result block on its
stack. It copies the previous result into the request word before running OPEN,
then runs GAME. [GAME](../src/game/main.c) writes the next presentation result;
[OPEN](../src/open/main.c) reads the request. Preserve this protocol as ordinary
application state, replacing executable loading with in-process mode transitions.

Current source evidence supports separate gameplay initialization:
[game_main_loop](../src/game/game.c) clears major gameplay state and initializes
its audio, display, and resources. OPEN does not appear to initialize GAME's
main gameplay data. This does not exclude other surviving dependencies.
[opening_run](../src/open/opening_controller.c) finishes with a graphics reset
that keeps the display. Audit RAM, VRAM, and device state across transitions
before deciding what to reset or retain.

Classify observed state as image-local, explicitly handed off, or
platform-persistent. Initially enter GAME directly for development, but compare
that path with OPEN-to-GAME startup before claiming equivalent initialization.

## Source organization and build

Proposed layout:

```text
src/game/                    retail reconstruction, reused where compatible
src/open/                    retail reconstruction, reused where compatible
src/psx/                     retail bootstrap reference
port/include/                portable SDK and ABI interfaces
port/runtime/                shared service implementations
port/render/                 packet interpretation and graphics backend
port/audio/                  decoding, sequencing, and mixing
port/images/game/            GAME startup, storage, required adaptations
port/images/open/            OPEN startup, storage, required adaptations
port/platform/linux/         native process and host integration
port/platform/wasm/          browser lifecycle, imports, persistent storage
port/tests/                  differential, replay, presentation controls
```

Compile GAME and OPEN as separate C targets. They have overlapping external
function and global names; separate static libraries alone do not resolve those
collisions. Rename entry points and namespace image-owned symbols before linking
both into one executable. Generated per-image prefix mappings are one candidate;
apply them consistently to declarations, definitions, and function-pointer
references. Keep SDK service symbols common. Share game implementations only
after verifying compatible behavior and state ownership.

The proposed build uses CMake + Ninja, modern Clang/GCC for Linux, and Emscripten
for browsers, with dependencies/toolchains pinned through Nix. Reuse source
membership and image defines from `config/units.toml` where practical. Native
compiler settings remain separate from retail probe profiles. Maintain explicit
source selection and reviewed replacements for adapted implementations. The
native runtime must not depend on generated MIPS objects or proprietary SDK
libraries.

Supply portable headers through target-specific include configuration. Existing
`psyq/*.h` wrappers are integration points. Every translation unit must see
consistent signatures and structures; arbitrary header shadowing must not
create incompatible views. `ADDRESS`, `DATA`, and `RODATA` already expand to
nothing and do not force native storage addresses.

Prefer direct source reuse and localized changes. If native adaptation would
distort reconstruction, keep an explicit implementation under `port/`, recording
its retail image/function provenance. Avoid pervasive conditional branches and
an untracked duplicate source tree. Ordinary per-image globals are acceptable;
there is no initial requirement to pass a giant context through every function.

Changes touching reconstruction inputs retain focused matches and full
`kf build` regression gates. Native tests do not promote byte-match status.

## Proposed libraries

| Responsibility | Default proposal |
| --- | --- |
| Window, input, controllers, timing | SDL3 |
| Shared renderer | OpenGL ES 3-compatible path; WebGL 2 through Emscripten |
| GTE operations | Portable CPU fixed-point implementation |
| Audio output | SDL3 audio streams |
| Audio behavior | Shared sample decoder, SEQ sequencer, voice mixer |
| Assets and saves | File interfaces behind CD/card-compatible services |
| Build | CMake + Ninja; Clang/GCC and Emscripten; Nix pinning |

SDL documents OpenGL ES 3 contexts through WebGL 2 on compatible browsers.
Use a common supported feature set and test shaders on both targets. SDL supplies
host services, not PS1 packet semantics, GTE behavior, or a complete sound engine.
[SDL browser support](https://wiki.libsdl.org/SDL3/README-emscripten),
[SDL audio](https://wiki.libsdl.org/SDL3/CategoryAudio).

SDL GPU/Vulkan was an initial Linux-only recommendation. Browser compatibility
was not established for that choice. The documented GLES/WebGL path is the
baseline here; revisit SDL GPU or WebGPU only with a working dual-target
prototype. [SDL GPU API](https://wiki.libsdl.org/SDL3/CategoryGPU).

### Evaluate PsyCross first

[PsyCross](https://github.com/OpenDriver2/PsyCross) offers Psy-Q-style headers,
software GTE, polygon/ordering-list handling, OpenGL rendering with VRAM handling,
and OpenAL-based ADPCM playback. Its documented dependencies are SDL2 and OpenAL
Soft, and it advertises Emscripten support. Its README also lists missing GTE
functions and incomplete ADSR support.

Prototype one King's Field scene using a pinned revision on Linux and in a
browser. Check the actual API inventory, widths, packet layouts, GTE outputs,
rendering, and audio services needed by this game. Success in other games and
advertised compatibility percentages do not establish King's Field conformance.

If adopted, initially retain its SDL2/OpenGL/OpenAL stack instead of combining
integration with an SDL3 migration. If unsuitable, implement the required
surface with the default stack above. Record prototype findings before choosing.

## ABI, storage, and dependencies

Separate serialized/hardware records from runtime objects. The former need
explicit widths, byte order, and verified offsets; the latter can use native
pointers and owned allocation. Current `s32`/`u32` use `long`, which is eight
bytes on typical 64-bit Linux. Redefining them does not automatically repair
pointer-bearing records or SDK uses of `long`.

- Decode little-endian fields safely and audit alignment/type-punning accesses.
- Use offsets or handles for file offsets and packet links. Never truncate
  native pointers into 32-bit words or 24-bit GPU links.
- Preserve fixed-point arithmetic, narrowing, rounding, and saturation. Express
  required wrapping and shifts with defined operations rather than relying on
  modern compiler treatment of undefined behavior.
- Match callback signatures exactly, including indirect calls in WASM.
- Define every required global and initialize actual owners. Replace clears
  that span adjacent retail objects with explicit native resets.
- Replace fixed RAM heap endpoints and pointer/integer allocation-stack entries
  in [memory.c](../src/lib/memory.c) with owned storage and bookkeeping.
- Retain persistence only where the transition audit establishes it.

Maintain a native dependency inventory: each external symbol resolves to game
code, owned data, or an explicit service implementation. Test doubles may fail
loudly for unsupported operations. Silent-success stubs cannot count as an
implemented playable path. WASM's 32-bit pointers must not hide LP64 problems.

## Main loop, input, and browser lifecycle

Expose initialization, bounded advancement, and shutdown. The exact API remains
open; a possible shape is:

```c
void kf_app_initialize(const KfLaunch *launch);
KfStepResult kf_app_tick(const KfInputState *input);
void kf_app_shutdown(void);
```

A tick advances the active mode: gameplay, menu, fade, or loading. Preserve
observed update/render ordering; splitting rendering out is optional until its
state mutations are understood. Use retail-derived fixed simulation ticks,
independent of monitor refresh or browser animation callbacks. Supply input
snapshots and derive edges from consecutive ticks. Preserve or explicitly
translate active-low buttons; keep mapping outside gameplay.

Linux alone could retain blocking loops with event processing inside waits.
With browsers planned, establish yielding boundaries now: nested menus, fades,
input-release waits, CD/card polling, and the outer loop must all let the host
regain control. Emscripten supports iteration callbacks. Asyncify can suspend
existing blocking control flow, at size/performance cost, and is a possible
migration aid; bounded updates are the long-term interface.
[Emscripten execution model](https://emscripten.org/docs/porting/emscripten-runtime-environment.html),
[Asyncify](https://emscripten.org/docs/porting/asyncify.html).

Start without requiring threads. Browser pthread builds impose hosting
requirements including COOP/COEP headers. Define pause/resume behavior for tab
suspension and focus loss; avoid unbounded simulation catch-up. Handle audio
activation after user interaction, controller reconnects, and graphics context
loss at the host boundary.
[SDL browser constraints](https://wiki.libsdl.org/SDL3/README-emscripten).

## Assets and saves

Require user-supplied retail data or an extraction/import result. Do not commit
retail assets or bundle them into a publicly distributed WASM preload. Translate
CD paths and sector behavior at the service boundary.

Browser support does not require making every loader asynchronous. Import or
preload a mode/floor bundle before entering code that expects it. Existing
synchronous reads can then use an in-memory virtual filesystem. Budget decoded
assets and retained bundles rather than preloading the whole disc by default.
Preserve bounds, widths, and sentinel semantics.

Emscripten's default memory filesystem is ephemeral. Browser saves require a
persistent backend such as IDBFS and completed synchronization. Distinguish a
successful memory write from durable storage; do not rely on page shutdown to
flush saves. Linux can use ordinary files and atomic replacement where
appropriate. Preserve payload and temporary-save recovery semantics. Original
memory-card import/export is a separate decision.
[Emscripten filesystem behavior](https://emscripten.org/docs/porting/emscripten-runtime-environment.html).

The [Rust codecs](../tools/README.md) and [parser comparison census](game-resource-parser-coverage.md)
provide reusable format knowledge and tests. Reuse through a deliberate build/FFI
boundary where beneficial; they are not automatically integrated with native C.
Choose import-time versus runtime decoding per format, consistently across hosts.

## Rendering and audio

Preserve the existing rendering frontend initially:

```text
game rendering code
  -> GTE-compatible transforms and lighting
  -> primitive and ordering-table construction
  -> portability-layer packet interpretation
  -> host buffers, textures, shaders, ordered draws
```

Implement required projection, clipping, lighting, depth-cue, flags, saturation,
and transform behavior in portable fixed-point code. Resolve packet links safely
on both pointer widths and preserve submission order. Implement texture-page and
CLUT lookup, transparency, texture windows, draw areas, and VRAM transfers as
required by the observed API/packet inventory.

Establish baseline behavior before adding perspective correction, depth-buffer
ordering changes, higher-resolution enhancements, or other presentation options.
Do not convert gameplay/collision math wholesale to floating point. Define the
visual fidelity target before accepting screenshot differences.

Preserve game-level audio calls and policy. Supply sample decoding, sequencing,
voice allocation, pitch, volume, envelopes, and mixing as required. SDL/OpenAL
output alone does not implement LIBSND. Share audio logic between hosts; isolate
device submission and browser activation. Exact interpolation, reverb, and
waveform timing are separate fidelity decisions. Document approximations.

## Correctness strategy

Retail is the behavioral reference. Test game code and the portability layer
separately before testing their integration.

| Level | Comparison | Main failures exposed |
| --- | --- | --- |
| Game functions | Retail MIPS, reconstructed MIPS, native C, WASM; Rust where available | Logic, widths, arithmetic, layout, pointer conversion |
| Layer conformance | Identical SDK inputs against an explicit reference | GTE rounding/flags, packets, audio, I/O |
| Recorded gameplay | Identical state, RNG, input, service completion schedule | Timing, integration, lifecycle, state drift |
| Presentation | Ordered packets/audio events, then frames/waveforms | Rasterization, palettes, transparency, sequencing, mixing |

Extend existing isolated oracles with native C and WASM paths. Begin with
resource loading, saves, movement, and collision. Compare meaningful fields and
ordered side effects; normalize pointers to object identities/offsets. Exclude
padding and native addresses from hashes, and retain field-level mismatch reports.

Declare each reference and its scope: actual retail instructions, emulator or
hardware capture, or independently checked model. Existing Python oracles are
bounded function harnesses with explicit services, not full-game or cycle-accurate
emulation. A shared mocked service cannot validate its own native replacement.
See the [oracle boundaries](../tools/README.md).

Test used primitive families, palettes, blending and ordering; GTE boundary values,
flags and saturation; audio sequence/voice events; short reads and failed writes;
controller edges and disconnections. Apply the same gates to PsyCross as to a
local implementation.

Record per-tick player, actor, inventory, map-event, RNG, and mode state. Control
service completion timing and locate the first differing tick, then subsystem.
Native/WASM agreement alone is insufficient: both can share a mistake relative
to retail.

Compare normalized ordered primitives before screenshots. Matching packets with
different pixels implicate rasterization; differing packets point to transforms
or game state. Compare audio commands/voice state before waveforms. Record asset
hashes, image identity, input trace, and reference configuration with captures.

Use native address/undefined-behavior sanitizers and warning checks. Browser tests
must cover rendering, audio activation, persistent-save reload, suspension, and
focus. Exercise `OPEN -> GAME -> OPEN -> GAME`, intro skipping, death/restart,
ending, and repeated mode entry. Probe residual RAM/VRAM/device state explicitly.

## Milestones and acceptance gates

Every core milestone builds and runs applicable tests on Linux and WASM. Add a
browser rendering smoke test with the first primitive. Linux receives the full
playable frontend first; WASM integration starts early.

| Milestone | Exit condition |
| --- | --- |
| 0. Backend/fidelity prototype | PsyCross evaluated on both targets; stack, initial fidelity, asset strategy recorded |
| 1. Linking and headless execution | Selected gameplay subset links with owned storage and explicit services; deterministic tests pass on Linux/WASM |
| 2. GAME lifecycle/assets | Full GAME target links; initialization and one floor load work; movement/collision replay agrees with retail checkpoints |
| 3. Rendered floor | Map, sprites, minimum UI; packets and captures meet chosen fidelity on both targets |
| 4. Playable Linux game | Actors, combat, menus, transitions, audio, saves; main game completable without executing MIPS code |
| 5. Full lifecycle/browser frontend | OPEN handoff and repeated transitions verified; browser imports, persistence, activation, suspension, input work |
| 6. Parity/release | Broad replay/playthrough coverage, documented fidelity differences, packaging, reproducible imports |

The first meaningful gameplay acceptance gate is one loaded floor, deterministic
movement/collision agreement, and representative frame agreement. Compilation
or a title screen is only an intermediate result.

## Open decisions and estimates

- Adopt PsyCross or implement the required layer after evaluating the prototype?
- Which symbol-namespacing and source-adaptation mechanisms should the build use?
- Which visual/audio differences are acceptable for the first playable release?
- Which formats are decoded during import versus load time?
- Should original card images be importable/exportable?
- Which retail scenarios/captures form the initial parity suite?
- Which Linux architectures and browser configurations are release targets?
- What additional transition persistence does the RAM/VRAM audit establish?

The superseded proposal estimated 1-2 person-months for a headless core, 4-8 total
for playable Linux, and 9-18 total for broad parity/polish. Those were rough
assumptions, not measured commitments, and should not be used as the schedule
for this revised layer-based design. Re-estimate after the backend prototype
and first linked gameplay slice using measured API, storage, lifecycle, and
fidelity gaps.
