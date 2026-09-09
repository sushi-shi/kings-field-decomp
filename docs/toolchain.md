# Toolchain environment and attribution boundary

The exact compiler, assembler, linker version, and optimization profile used
to build King's Field are not yet proven. The reproducible environment must not
turn a strong family attribution into a false exact claim.

What is established:

- King's Field's three linked programs (`PSX.EXE`, `GAME.EXE`, and `OPEN.EXE`)
  contain substantial relocation-aware matches to contemporary Sony Psy-Q
  libraries.
- The leading host-tool family is S.N. Systems Psy-Q: CCPSX, ASPSX, PSYLINK,
  PSYLIB, and CPE2X.
- The 1994-12-29 Release 2.5 floppy collection contains CCPSX 1.02, PSYLINK
  1.17, PSYLIB 1.04, and ASMPSX 1.07, plus two compiler frontend pairs whose
  embedded GNU versions are 2.4.1 and 2.6.0.
- A separate GNU C Compiler 2.60 disk supplies another GCC 2.6.0 pair. It is
  not byte-identical to the pair in Release 2.5, so both remain distinct.
- The Release 2.5 copies of LIBCD, LIBSPU, LIBSND, LIBGTE, LIBGPU, and LIBSN
  are exact inputs used by the relocation-aware library matcher.

The Nix flake therefore exposes a **candidate matrix**:

| Environment variable | Contents |
| --- | --- |
| `PSYQ_DIR` | Root of the staged candidate bundle |
| `PSYQ_BIN` | Release 2.5 DOS host tools |
| `PSYQ_ASPSX` | Pinned working ASPSX 1.07 DOS executable for direct EXE builds |
| `PSYQ_INCLUDE` | Release 2.5 headers |
| `PSYQ_LIB` | Release 2.5 Psy-Q libraries |
| `PSYQ_GCC241_DIR` | Release 2.5 extensionless GCC 2.4.1 frontends |
| `PSYQ_GCC260_RELEASE25_DIR` | Release 2.5 DOS/COFF GCC 2.6.0 frontends |
| `PSYQ_GCC260_DISK_DIR` | Independent GNU C 2.60 disk frontends |
| `KF_GCC260_NATIVE` | Decompals old-gcc 0.17 native GCC 2.6.0 PSX rebuild |

The old host tools are DOS executables. The Release 2.5 media copy of ASPSX
stops with a software-key/network-manager error. A distinct preserved 1.07
copy from the assembler project's test archive runs successfully under
DOSBox-X and is now pinned as `PSYQ_ASPSX`. `kf link` uses it directly, then
PSYLINK 1.17 and CPE2X with the original SDK libraries. There are no ELF
adapters or retail output-byte inputs in executable generation. See
[the executable build](executable-linking.md).

maspsx and GNU MIPS binutils remain the separate ELF-based objdiff analysis
route. Those generated ELF objects do not feed the executable link.

The practical assembler path uses `-no-pad-sections` to retain explicit assembly
extents without GNU-as automatic end padding. Explicit zero bytes, alignment
directives, COMMON allocations and ELF alignment requirements are preserved.
This contract is recorded in object metadata and tested against both native
compilers; it is not historical ASPSX attribution. See
[the section-extent controls](patterns/assembler-section-extents.md).

`tests/psylink_order_smoke.py` runs the original pinned PSYLINK 1.17 under
headless DOSBox against preserved Release 2.5 objects. It verifies direct input
ordering separately from lazy archive extraction and is part of
`nix flake check`; see `vendored-functions.md` for the conclusions and limits.

The shell also exposes `cc1psx-260`/`cpppsx-260` and `cc1psx-257`/`cpppsx-257`.
These are native Linux rebuilds of the GCC 2.6.0 and GCC 2.5.7 PSX targets
from Decompals old-gcc 0.17, pinned by archive SHA-256 (`KF_GCC260_NATIVE`,
`KF_GCC257_NATIVE`). They enable the live C -> assembly -> maspsx -> ELF
matching loop. They do not collapse the two historical 2.6.0 distributions
into one, prove host-binary identity, or prove which GCC built the retail
game. A controlled probe on 2026-09-01 ran both historical DOS `CC1PSX` 2.6.0
binaries under headless DOSBox and obtained assembly identical to the native
2.6.0 rebuild, while the 2.5.7 rebuild reproduces retail's framed epilogue
and load hoisting; see
[`patterns/gcc257-epilogue-and-scheduling.md`](patterns/gcc257-epilogue-and-scheduling.md).
The Release 2.5 GCC 2.4.1 frontend is a raw DJGPP v1 COFF image that needs a
`GO32` extender the media does not include, so it has not been executed.

The initializer verifies and stages the original media directly:

1. `flake.nix` fetches original media by immutable SHA-256 and supplies all
   extraction tools.
2. `scripts/create-toolchain.py` extracts, verifies, stages, manifests, and
   normalizes the result.
3. `flake.nix` invokes the same Python staging path as a Nix derivation, so
   the first `nix develop` initializes the historical toolchain and later loads
   reuse Nix's cached result.

The two historical source files total 6.3 MiB and the staged output is 7.7 MiB.
The optional native GCC 2.6.0 probe adds a 3.18 MiB compressed fixed-output
archive.

No generated SDK binary, retail game image, or executable belongs in Git.

## Analysis tools

`nix develop` provides the historical compiler candidates and practical PSX
build path above, plus Ghidra/PyGhidra, DOSBox, little-endian MIPS GNU
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
