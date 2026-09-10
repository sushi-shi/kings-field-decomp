# One SDK and the analysis environment

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
present` under DOSBox. The repository no longer substitutes a working ASPSX
from another archive. A coherent `kf link` is unavailable until the pinned
SDK's compiler and assembler can run; see [the executable-linking
status](executable-linking.md).

The native `cc1psx` rebuilds, maspsx, GNU MIPS binutils, psy-k and objdiff are
analysis programs. They compile comparison objects for the reconstruction loop
but are not members of the SDK and do not support a historical executable-build
claim. Their versions remain pinned so matching results are reproducible. The
GCC 2.5.7 and 2.6.0 comparison behavior is documented in
[`patterns/gcc257-epilogue-and-scheduling.md`](patterns/gcc257-epilogue-and-scheduling.md),
and the ELF section model is documented in
[`patterns/assembler-section-extents.md`](patterns/assembler-section-extents.md).

The Ghidra extension's later Psy-Q 2.60 signatures are also an analysis
corpus. Vendored-function inventory code may use them to propose a name after
the Release 2.5 object search. Those JSON signatures cannot override exact
Release 2.5 object evidence and are not SDK inputs.

`tests/psylink_order_smoke.py` can run the Release 2.5 PSYLINK directly because
that tool does not require the assembler's software key. It tests linker order
using preserved SDK objects and does not create a mixed executable toolchain.

The initializer verifies and stages the SDK directly:

1. `flake.nix` fetches the one Release 2.5 medium by immutable SHA-256.
2. `scripts/create-toolchain.py` verifies its tools and all 17 `.LIB`/`.OBJ`
   paths, then copies the complete extracted tree under `release-2.5/`.
3. A deterministic manifest records every staged file.

No generated SDK binary, retail game image, or executable belongs in Git.

## Analysis tools

`nix develop` provides the single historical SDK and the separate analysis
programs above, plus Ghidra/PyGhidra, DOSBox, little-endian MIPS GNU
binutils, maspsx, psy-k, disc-image utilities, objdiff, and the normal
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
