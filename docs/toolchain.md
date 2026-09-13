# SDK and source-to-EXE toolchain

The repository has one active historical SDK: the complete hash-pinned Psy-Q
Release 2.5 floppy tree. It is staged without files from another SDK, compiler
disk, or assembler archive. This is the baseline used to identify retail
library objects; it is not a claim that Release 2.5 was the exact retail
revision.

The medium contains CCPSX 1.02, PSYLINK 1.17, PSYLIB 1.04, ASMPSX 1.07, its
compiler directory, all 12 general `.LIB`/`.OBJ` inputs, all four H2000 inputs,
and the demo CARD object. The two CARD objects are byte-identical. H2000
`LIBAPI.LIB` differs from the general archive, so both retain their original
paths inside the same SDK tree.

The medium itself contains an extensionless GCC 2.4.1 frontend pair and a
`.EXE` GCC 2.6.0 pair in `compiler/`. Preserving both is part of preserving
this one distribution; neither pair is staged as a second SDK.

| Environment variable | Release 2.5 path |
| --- | --- |
| `PSYQ_SDK` | Root of the complete media tree |
| `PSYQ_BIN` | `isa board/PSXBIN/BIN` |
| `PSYQ_INCLUDE` | `isa board/PSXLIB/INCLUDE` |
| `PSYQ_LIB` | `isa board/PSXLIB/LIB` |
| `PSYQ_H2000_LIB` | `H2000/LIB2000` |
| `PSYQ_COMPILER` | `compiler` |

Both assembler executables on the medium stop with `Software Data Key not
present` under DOSBox. The executable chain uses a separately hash-pinned,
working ASPSX 1.07 (`PSYQ_ASPSX`) alongside the native GCC probe selected by
each unit's profile. PSYLINK, CPE2X, headers, libraries, and overlay startup
come from the preserved Release 2.5 tree. Build reports distinguish those
provenances; none of this proves the exact historical tool versions.

The native builder applies one guarded correction to its copy of
`LIBETC.LIB/INTR.OBJ`: return to the BIOS after each interrupt-dispatch pass.
The original SDK loop can starve controller and audio updates on modal screens.
Original and corrected archive hashes are recorded separately in build reports;
the staged SDK and matching reference inputs stay unchanged. See the
[runtime evidence and regression control](patterns/sdk-interrupt-return.md).

The [classic SDK header audit](sdk-header-audit.md) records which original
headers are used and why the remaining compatibility declarations are needed.

Classic additionally uses the original GNU GCC 2.5.7 `stdarg.h` and `va-mips.h`,
packaged separately from the Sony SDK. `PSYQ_C_INCLUDE` points to their Nix
store directory. `compile_classic` supplies `__GNUC__=2`, `__GNUC_MINOR__=5`,
`__mips__` and `__MIPSEL__`, which the standalone preprocessor does not define.
The ordinary matching compiler path keeps its existing preprocessing contract.

There is one source compilation path: CPPPSX -> CC1PSX -> ASPSX -> native
Psy-Q OBJ. `kf build` passes those objects and the SDK libraries to PSYLINK,
then runs CPE2X without rewriting its output. `kf analyze` and `kf try` use
the same compiler and assembler, and read the resulting native objects into
ELF views for objdiff. No maspsx or GNU assembler processes game source.
Debug metadata is enabled in the compiler and assembler so private symbols
remain inspectable. It is part of the shared probe flags, not a second
comparison build configuration.

GNU MIPS binutils, psy-k and objdiff are inspection tools. They neither lay
out nor emit the candidate executables. The compiler scheduling evidence is
in [`patterns/gcc257-epilogue-and-scheduling.md`](patterns/gcc257-epilogue-and-scheduling.md).
Earlier GNU-container calibration in
[`patterns/assembler-section-extents.md`](patterns/assembler-section-extents.md)
is historical; current section extents and COMMON requests are read directly
from native ASPSX objects.

The Ghidra extension's later Psy-Q 2.60 signatures are also an analysis
corpus. Vendored-function inventory code may use them to propose a name after
the Release 2.5 object search. Those JSON signatures cannot override exact
Release 2.5 object evidence and are not SDK inputs.

`tests/psylink_order_smoke.py` can run the Release 2.5 PSYLINK directly because
that tool does not require the assembler's software key. It tests linker order
using preserved SDK objects.

The initializer verifies and stages the SDK directly:

1. `flake.nix` fetches the one Release 2.5 medium by immutable SHA-256.
2. `scripts/create-toolchain.py` verifies its tools and all 17 `.LIB`/`.OBJ`
   paths, then copies the complete extracted tree under `release-2.5/`.
3. A deterministic manifest records every staged file.

No generated SDK binary, retail game image, or executable belongs in Git.

## Analysis tools

`nix develop` provides the single historical SDK and the separate analysis
programs above, plus Ghidra/PyGhidra, DOSBox, little-endian MIPS GNU
binutils, psy-k, disc-image utilities, objdiff, and the normal
C/C++/Python build tools. Ghidra plugin packaging is documented separately in
[`ghidra.md`](ghidra.md).

Splat, Rabbitizer, and spimdisasm use the separately locked Python environment
because the latter two are not available in the pinned Nixpkgs revision. The
shell wrappers keep both setup and execution on that project lock:

```sh
kf-python-sync
splat --help
```

The first command is the one-time `uv sync --frozen --no-install-project`
step; `splat` delegates to `uv run --frozen` thereafter.
