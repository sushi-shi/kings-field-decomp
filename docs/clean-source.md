# Generated source and classic branches

`kf clean` derives a standalone C++20 project and optional Rust resource library.
`kf clean --classic` derives the pinned C build without codecs.
The generator and its tests live here; the generated branch contains only
source, build/run support, licensing and a fresh README.

```text
              master (you are here)
                 |
     +-----------+-----------+
     |                       |
     v                       v
  source                  classic
     |
     v
   port
```

| Branch | Purpose |
| --- | --- |
| `master` | Reconstruction and matching |
| `source` | C++ PS1 build, codecs, and base for porting |
| `classic` | C PS1 build |
| `port` | Crossplatform port |

Both exports build and run on PS1. `source` is the base for the crossplatform port;
`port` owns platform changes and is not overwritten by regeneration.

```sh
nix develop -c kf clean --out build/clean-source --verify \
  --publish source --worktree build/source
nix develop -c kf clean --classic --out build/clean-classic --verify \
  --publish classic --worktree build/classic
```

Generation reads committed `HEAD`, including that revision's templates, unit
membership and compiler settings. `--ref REVISION` selects another committed
input. `--working-tree` previews tracked working files, including staged new
files, but cannot publish. Untracked work is never an export input.

The output is deterministic. Only marked output directories can be replaced;
an output inside this checkout must be below `build/`. Publication creates a
local generated branch and a persistent worktree. It refuses dirty destination
worktrees, unrelated existing branches, and collisions with ignored files.
Each generated branch always contains exactly one root commit. Regeneration
replaces that snapshot; identical regeneration from the same commit is a no-op.
Older exports with ancestry are collapsed automatically. Provenance stays in
commit messages. Publication is local; updating GitHub requires a push with an
explicit expected-tip `--force-with-lease` for each generated branch.

The allowlist retains the C files used by the executable builder, shared `.inc`
implementation fragments, their project
and SDK wrapper headers, linker boundaries, build support, and the Rust codec
library. Vendored verification bodies are excluded because the executable
builder uses the SDK libraries. Codec oracle executables and all tests are
excluded. The source flake has no checks or test dependencies.

The lexer removes claims and comments without altering strings or joining
tokens. License notices remain. Source selects the C++ view with real scoped
enums, typed storage and conversions. Classic selects typedefs, integer constants
and explicit casts, with O32 integer promotions. Boolean storage widths remain unchanged. Per-image
conditionals, runtime macros, initialization and linker boundaries remain.
Unknown cleanup constructs fail generation rather than silently surviving.

The compiler/assembler and linker implementations under `scripts/psxbuild/`
are shared with the reconstruction commands. The generated manifest contains
ordinary source membership and build options, without claims or inventories.
Nix tool definitions are shared, while the generated flake selects only the
tools required for building and running the game.

`--verify` builds the standalone flake and, on source, its Rust library.
The C++ build uses Clang, combines ELF objects with the MIPS linker and converts
their relocations to a native Psy-Q linker input. SDK library bodies remain
ordinary archive inputs, with the documented
[interrupt-return correction](patterns/sdk-interrupt-return.md) applied by the
shared builder. Typed overloads have real implementations; the effect
constructor uses named arguments instead of relying on old compiler stack slots.
Counted export transformations leave the reconstruction source untouched.

Classic gets GCC 2.5.7's original `stdarg.h` and `va-mips.h` from Nix;
the exported tree has no project copy of `stdarg.h`. Its compiler invocation
supplies the GCC version and little-endian MIPS definitions normally supplied
by the driver. The matching build on master retains its existing varargs header.

For classic, verification independently builds the unstripped master sources
with the same original compiler headers, under `build/clean-reference/`,
and requires byte-identical native CPE linker outputs. It compares every EXE
byte, accepting and explicitly reporting differences only in the reserved
header words at offsets `0x08..0x0f`, which the pinned CPE2X writer leaves
uninitialized. All other header bytes and the full executable payload must
agree. Full-file equality is reported separately; no bytes are patched and
this check does not bank or declare a retail match. The original-writer control
is `tests/test_cpe2x_header.py`. The original varargs implementation can change
generated instructions relative to master's matching header; that comparison
is not a cleanup identity check. `tests/test_classic_varargs.py` executes the
original headers across O32 register, stack, promotion and alignment boundaries.
C++ output is not expected to match the classic
compiler's bytes. When verifying committed HEAD, commit relevant
working changes first so that both builds have the same inputs.

From the generated worktree:

```sh
nix build
nix run . -- --disc "/path/to/King's Field (Japan).cue"
nix run . -- --retail --disc "/path/to/King's Field (Japan).cue"
```

Both generated branches also retain master's retail-run command:

```sh
export KF_RETAIL_DISC="/path/to/King's Field (Japan).cue"
nix develop -c kf-run-retail
nix run .#retail  # equivalent, without the build environment
```

Retail mode validates and runs the original image without a game build or
replacement executables.

The launcher validates the local disc and reads file extents from ISO9660.
It replaces all three executables in a cached copy and recomputes sector EDC
and ECC. It does not need local reconstruction configuration or extracted
retail files. Larger executables move to appended sectors; the directory and
volume descriptors are updated while existing asset extents stay intact.
Game resources stay outside Git and the Nix store. Emulator boot/launch is the
runtime acceptance check; gameplay validation is separate.
