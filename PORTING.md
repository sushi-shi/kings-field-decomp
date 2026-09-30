# Port architecture and maintenance

See [README](README.md) for building and playing, [status](docs/port-status.md)
for remaining work, and [technical notes](docs/port-findings.md) for format and
retail evidence that constrains changes.

## Source ownership

The original code owns movement, AI, combat, animation, menus, scripts and
progression. Keep changes local to portability, verified defects and explicit
port policies. Use plain structs, functions and scoped enums; no inheritance,
RTTI or exceptions. Preserve calculation widths, rounding and random-call order.

| Location | Responsibility |
| --- | --- |
| `src/game`, `src/open` | Gameplay and opening/ending policy, separate phase state |
| `src/lib` | Shared operations compiled once, with explicit phase-owned inputs |
| `src/platform` | SDL host, local extraction, files, input, saves and lifecycle |
| `src/renderer`, `src/audio` | Native draw commands/shaders and software mixer |
| `include/kf` | C++ declarations and authoritative C codec interfaces |
| `codecs` | One Rust static library: safe audio/TIM parsers and a C FFI module |
| `web` | Browser launcher, resource cache and save persistence |
| `build.json`, `cmake` | Original translation-unit ownership and namespace wrappers |
| `tests` | Native regressions and isolated runtime scenarios |

One coordinator calls opening, game and ending. Each phase owns its arena and
initialized globals; reset both initialized storage and BSS on re-entry. Shared
functions receive call-local views of the correct owner. Do not retain those
views across phase transitions. Original nested loops yield through SDL/Asyncify.

## Types and casts

Correct declarations and runtime models before removing casts. Decode file bytes
with explicit widths, endianness and bounds; native runtime objects use native
pointers and sizes. Keep explicit numeric conversions where narrowing, wrapping
or signedness is intentional. Pointer reinterpretation requires proven alignment,
lifetime and extent. Preserve arena rewind boundaries and real loaded lengths.

The two codec headers, `include/kf/lib/codec.h` and `include/kf/audio/codec.h`, own
shared structs, enums and constants. `codecs/src/ffi/bindings.rs` is committed
bindgen output with compile-time layout checks. Ordinary builds consume it;
bindgen is needed only after changing those headers:

```sh
nix develop -c bash codecs/bindings.sh
nix develop -c bash codecs/bindings.sh --check
```

The flake pins bindgen and checks that regeneration produces the committed file.
The parsers forbid unsafe Rust; raw pointers and caller-owned output buffers are
confined to `codecs/src/ffi`. Buffers must be disjoint and remain valid for the call.

## Runtime boundaries

- **Resources:** extract the Japanese ISO or BIN/CUE locally, preserving file
  contents and paths. Load COM/MIX/TMD/MIM/TIM/SEQ/VAB through bounded reads.
  Keep original arena ownership; no replacement gameplay asset format.
- **Rendering:** original producers choose visibility, lighting, materials and
  depth. Submit copied vertices to the native GLES3/WebGL2 renderer. Sort whole
  faces, preserve equal-depth head insertion and split quads afterwards. Reused
  morph scratch pointers must not survive submission. Retained-frame expose or
  resize presents stored pixels without repeating blends.
- **Timing:** use the focus-aware shared clock and absolute deadlines. GAME world
  rendering owns one three-host-tick interval (about 20 updates/s), including
  scripts. OPEN scene0 uses 22 updates/s; other loops retain their own waits.
  These stable rates are port policy. Do not change per-step arithmetic or add a
  second complete gameplay interval after presentation.
- **Input:** original button logic consumes keyboard/controller actions. Mouse
  look updates view angles. Focus loss pauses and releases capture; browser
  callbacks collect input and service platform work without entering gameplay.
- **Audio:** preserve sound selection, scheduling, spatial and volume decisions.
  Decode banks/sequences into mixer-owned storage that outlives arena rewinds.
  Browser audio starts from a gesture; sample time controls playback timing.
- **Saves:** preserve save-point rules and original menus. Serialize versioned
  fixed-width fields, rebuild transient references, and report success only after
  native/IndexedDB persistence completes. Browser stores can still be evicted.
- **Errors:** required in-game resource and allocation failures use `host_fail`.
  Native failures release capture, pause audio, display the message and exit 1
  after dismissal. Optional file failures return to callers; startup CLI/import
  errors use console diagnostics.

## Behavior constraints when refactoring

- Movement updates Z before testing X; later checks and blocking calls must see
  live state. Runtime map coordinates use `z, x`; warp endpoints use `x, z`.
- Pools traverse live slots in ascending order. Preserve free-slot guards, tie
  behavior, final actor bindings and default output distances. Placement sentinels
  stop reading source records, but remaining destination slots still need reset.
- Armor regeneration precedes drain in head/body/shield/arm/leg order. Keep each
  HP adjustment because clamping and death handling make combined deltas differ.
- GAME camera paths fetch then increment; OPEN paths increment then fetch.
  Preserve sentinel, frame-decrement and transition order.
- Menu labels sometimes write only part of a row, reusing earlier glyphs. Preserve
  input priority, cancel paths and dialogue grant/consume/notification order.
- Signed shifts, byte wrapping, partial vector/matrix writes and aliasing are
  meaningful. `matrix_interpolate` updates rotation only, preserving translation.
  Module snapshots recursively copy C arrays, including nested arrays.
- A floor-local definition number is not a global actor identity. Keep unresolved
  resource meanings explicit rather than inventing names.

## Verification

Run in the pinned shell; do not commit retail assets or generated build products.
Build Linux and WASM after code changes and inspect the relevant callers and diff.
For tooling/build changes, also run `nix flake check -L`.

```sh
nix develop -c env ASAN_OPTIONS=detect_leaks=0 python3 -m unittest discover -s tests -v
nix develop -c cmake --preset sanitize
nix develop -c xvfb-run -a -s '-screen 0 1600x1200x24' \
  python3 tests/runtime_scenarios.py --data /path/to/extracted/disc
```

Disabling leak scanning is needed only where process inspection is blocked;
address and undefined-behavior checking stay enabled. Native regressions cover
COM copy spans and malformed bounds, scalar/array resets, matrix interpolation
and required/optional file errors. Cargo test targets are disabled; a successful
Cargo build is not test coverage.

The runtime driver builds isolated observers, enters floors 1–5 then 1, exercises
save/load and GAME/OPEN re-entry, and uses separate saves and display. `--movement`
adds fixed input sequences; `--source-root` selects a reference checkout and
`--baseline` compares its six saves byte for byte. This does not verify natural
stairs/warps, every combat/script path, a full ending, audio/pixel equivalence or
browser behavior. Use input scenarios only when authorized; otherwise keep
observations without input and leave the user's client and saves untouched.
