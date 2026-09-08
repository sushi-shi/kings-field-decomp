# Compile and link executables with the Psy-Q tools

Inside `nix develop`, after `kf init`, run:

```sh
kf link --image psx
```

There is one executable build path:

```text
C source --GCC--> assembly --ASPSX--> Psy-Q objects
                                      + original SDK .LIB files
                                           |
                                        PSYLINK
                                           |
                                          CPE
                                           |
                                         CPE2X
                                           |
                                          EXE
```

GCC uses each unit's source, includes, defines and compiler flags from the
manifest. ASPSX 1.07 reads that compiler output directly, with DOS text line
endings. PSYLINK 1.17 consumes the resulting LNK objects and the original
Psy-Q library files. CPE2X writes the final PS-X EXE. The build does not change
any bytes in the object, linked CPE or converted EXE.

For PSX the original library inputs are `LIBSN.LIB` and `LIBAPI.LIB`.
The native linker selects their members and resolves their symbols. Its
ordinary command file specifies the program's load origin, input objects,
libraries and entry symbol. It does not place individual game functions or
rewrite their addresses.

The linker combines each object's code and data and resolves references
between objects. For example, a C call to `Load` refers to a library symbol;
the linker finds the implementation in the SDK and encodes the call target.

CPE2X supplies the executable header and rounds its load area to a 2048-byte
sector boundary. Space after the linked initialized data is **padding**.
PSX has 560 initialized bytes, so its load sector has 1488 padding bytes.
The separate 2048-byte header makes the whole executable 4096 bytes.

## Inputs, outputs and failures

`build/link/psx/` contains compiler output (`U0000.I`, `U0000.S`), the native
object (`U0000.OBJ`), `LINK.LNK`, native tool logs, `PSX.CPE`, `PSX.SYM`,
`PSX.MAP`, `PSX.EXE` and `comparison.json`. The report lists commands and
source, object, library and tool hashes. Every CPE load record is checked
against the converter's unchanged output before the retail comparison.

The verified retail EXE is used **only for comparison**. It does not supply
header bytes, missing initialized data, missing startup objects or tail bytes.
Missing source definitions fail the native link. A failed build removes an
older executable so it cannot be mistaken for the current result.

The custom ELF section rewrite, SDK-to-ELF link adapter, retail-header template,
inventory data fallback and inferred-padding mode have been removed. The
existing ELF-based `kf match` analysis remains separate from `kf link`; its
objects are not executable-build inputs.

`kf link --image game` and `kf link --image open` use the same direct path;
`kf link` attempts all three. A successful exit means native compilation,
assembly, linking and conversion succeeded. `comparison.file_equal` reports
unmasked complete-file equality. Build success alone is not an exactness claim.

GAME and OPEN currently reach PSYLINK but fail on unresolved symbols. Both
need their startup entry definition; examples of other missing definitions are
GAME's `player_state` and `game_exit_code`, and OPEN's `cd_path_buffer` and
`memory_system_heap_start`. The original archives also do not resolve every
curated SDK name, including GAME's `InitCARD2`. Their source and library
ownership must be completed before these programs can link.

## PSX result

Direct ASPSX and PSYLINK produce all **560 initialized bytes exactly**:
40 path-string bytes, 208 bytes of compiled `main`, 300 SDK code bytes, the
8-byte C pointer table and the SDK's 4-byte stack-size word. No game C change
was required for this link.

The tested native CPE2X output differs from retail in **66 file bytes**:
59 header bytes and seven padding bytes. Those differences are retained.
The historical converter/build environment has not been reproduced, so the
complete executable is not claimed exact. See the
[PSX evidence record](patterns/psx-exact-link.md).
