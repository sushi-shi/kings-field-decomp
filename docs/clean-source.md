# Generated source branch

`kf clean` derives a standalone C project and optional Rust resource library.
The generator and its tests live here; the generated branch contains only
source, build/run support, licensing and a fresh README.

```sh
nix develop -c kf clean --out build/clean-source --verify \
  --publish source --worktree build/source
```

Generation reads committed `HEAD`, including that revision's templates, unit
membership and compiler settings. `--ref REVISION` selects another committed
input. `--working-tree` previews tracked working files, including staged new
files, but cannot publish. Untracked work is never an export input.

The output is deterministic. Only marked output directories can be replaced;
an output inside this checkout must be below `build/`. Publication creates a
local generated branch and a persistent worktree. It refuses dirty destination
worktrees, unrelated existing branches, and collisions with ignored files.
Identical regeneration from the same commit is a no-op. Later generations retain
the previous source tip and the new reconstruction commit as ancestry.
Provenance stays in commit messages. No remote push occurs.

The allowlist retains the C files used by the executable builder, their project
and SDK wrapper headers, linker boundaries, build support, and the Rust codec
library. Vendored verification bodies are excluded because the executable
builder uses the SDK libraries. Codec oracle executables and all tests are
excluded. The source flake has no checks or test dependencies.

The lexer removes claims and comments without altering strings or joining
tokens. License notices remain. Type-checking conditionals select the C view;
enum macros become typedefs, integer constants and explicit casts, with O32
integer promotions. Boolean storage widths remain unchanged. Per-image
conditionals, runtime macros, initialization and linker boundaries remain.
Unknown cleanup constructs fail generation rather than silently surviving.

The compiler/assembler and linker implementations under `scripts/psxbuild/`
are shared with the reconstruction commands. The generated manifest contains
ordinary source membership and build options, without claims or inventories.
Nix tool definitions are shared, while the generated flake selects only the
tools required for building and running the game.

`--verify` builds the standalone flake, runs the current reconstruction build,
and requires byte-identical native CPE linker outputs. It compares every EXE
byte, accepting and explicitly reporting differences only in the reserved
header words at offsets `0x08..0x0f`, which the pinned CPE2X writer leaves
uninitialized. All other header bytes and the full executable payload must
agree. Full-file equality is reported separately; no bytes are patched and
this check does not bank or declare a retail match. The original-writer control
is `tests/test_cpe2x_header.py`. It also builds
the exported Rust library. When verifying committed HEAD, commit relevant
working changes first so that both builds have the same inputs.

From the generated worktree:

```sh
nix build
nix run . -- --disc "/path/to/King's Field (Japan).cue"
```

The launcher validates the local disc and reads file extents from ISO9660.
It replaces all three executables in a cached copy and recomputes sector EDC
and ECC. It does not need local reconstruction configuration or extracted
retail files. Executables larger than their existing extents are rejected.
Game resources stay outside Git and the Nix store. Emulator boot/launch is the
runtime acceptance check; gameplay validation is separate.
