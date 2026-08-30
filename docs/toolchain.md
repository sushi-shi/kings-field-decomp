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
| `PSYQ_INCLUDE` | Release 2.5 headers |
| `PSYQ_LIB` | Release 2.5 Psy-Q libraries |
| `PSYQ_GCC241_DIR` | Release 2.5 extensionless GCC 2.4.1 frontends |
| `PSYQ_GCC260_RELEASE25_DIR` | Release 2.5 DOS/COFF GCC 2.6.0 frontends |
| `PSYQ_GCC260_DISK_DIR` | Independent GNU C 2.60 disk frontends |

The old host tools are DOS executables. DOSBox is included for controlled
execution, but ASPSX is key-protected. maspsx and GNU MIPS binutils provide the
practical, scriptable assembly route while we compare their output against the
retail programs and the original tools.

The initializer keeps the useful verification half of the Gruntz project
pattern, without its derived release archive:

1. `flake.nix` fetches original media by immutable SHA-256 and supplies all
   extraction tools.
2. `scripts/create-toolchain.py` extracts, verifies, stages, manifests, and
   normalizes the result.
3. `flake.nix` invokes the same Python staging path as a Nix derivation, so
   the first `nix develop` initializes the historical toolchain and later loads
   reuse Nix's cached result.

The two source files total 6.3 MiB and the staged output is 7.7 MiB. Gruntz's
separate release artifact avoided repeatedly fetching much larger Visual
Studio, service-pack, and DirectX media; that indirection has no useful payoff
at this size.

No generated SDK binary, retail game image, or executable belongs in Git.
