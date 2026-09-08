# PSX executable closure campaign

## Link Match Plan

Keep the existing exact C body and its SDK/API types unchanged. PSX `main` is
208 bytes at `80010028`, with fifteen text relocations and two pointer-table
relocations. The complete source unit owns both 20-byte path strings and the
eight-byte pointer table. The seven SDK text members and SNDEF data are
independently identified supplied library inputs, not game progress.

1. Preserve the native compiler's explicit section directives in the ELF
   bridge, checking larger alignment requests, typed word alignment, extents,
   symbols and actual linked relocation targets with synthetic inputs.
2. Link complete SDK objects in the independently recovered PSX order.
3. Account for container metadata and final-sector padding separately from
   initialized source data and BSS. Use full-file equality without masks.
4. Rebuild and strictly match PSX, run the repository checks and full build,
   and report the exact boundaries of any retained compatibility model.

## Compiler-directed sections

The real GCC 2.5.7 output places `.align 2` before the path strings, pointer
table and `main`. GNU ELF assigns ordinary `.text` and `.data` a sixteen-byte
minimum in addition to those directives. Reassembling the same expanded input
using neutral section names and then restoring their names produces four-byte
constraints, with all 208 text, eight pointer-table and forty literal bytes
unchanged. This uses no retail address to select an alignment.

The opt-in `section_alignment = "directives"` profile implements that bridge
for PSX. Explicit `.align 3` remains eight-byte alignment, `.balign 32` remains
32, and typed `.word` data still causes GAS's ordinary word alignment. Symbols
and relocations survive a real GNU link at a four-mod-sixteen text address.
The bridge does not rewrite `sh_addralign` fields after assembly.

COMMON allocation remains a separate contract. maspsx's expansion can rely on
implicit BSS alignment, so populated ordinary BSS retains the existing GNU
constraint. An unused default BSS section is removed; it has no reservation
and must not round the SDK's BSS end. PSX has no source COMMON allocation.
The other compiler profiles retain their existing behavior.

The exact compiler/assembler attribution remains open, as for the existing
probe. This is a source-assembly-to-ELF contract, not evidence that the rebuilt
host programs are byte-identical to the historical tools.

## Whole-object link and container boundary

`kf link --image psx` now passes the complete SDK objects in the recovered
order: `SNMAIN, A36, C113, C57, C66, C67, C114, SNDEF`. The
[object-order evidence](../object-link-order.md) predates the executable
comparison. GNU ld still honors each input's section constraints and applies
its relocations normally; the linker does not assign individual SDK function
addresses. All eight original SDK members are independently verified after
linking: **304 provided bytes and 18 native patch expressions**.

The resulting initialized image is 560 bytes:

| Range | Provider | Bytes |
| --- | --- | ---: |
| `80010000–80010028` | Compiled C path literals | 40 |
| `80010028–800100f8` | Compiled C `main` | 208 |
| `800100f8–80010224` | Seven complete SDK text contributions | 300 |
| `80010224–8001022c` | Compiled C pointer table | 8 |
| `8001022c–80010230` | SDK `SNDEF` stack-size word | 4 |

There is no PSX inventory payload or retail-delinked startup input. The entry
is `80010100`; SDK `.sbss` is four uninitialized bytes at `80010230`, ending
at `80010234`. All code, data, constants, referents and linked source symbol
placements agree with retail before container padding is considered.

The 2048-byte header remains an explicit retail template. It contains region
metadata and apparent uninitialized converter state, including a CPE prefix
at header offset `88`; entry and load extent are regenerated from the link.
The final sector contains another CPE v1 prefix immediately after initialized
data, as independently observed in all three retail images. The packer models
that prefix as magic/version, select-unit-zero and the start of a PC register
record, followed by zero padding to the sector boundary. It uses the **actual
linked load end**, without reading retail payload bytes or fixing a retail
address, and truncates the prefix if fewer than nine padding bytes remain.
It neither extends a source section nor initializes BSS.

This is an **inferred container compatibility rule**. The report calls its
mechanism `candidate` and leaves `historical_converter_reproduced` false.
It does not prove the historical converter or complete source reconstruction.
The zero-padding serializer remains available as the comparison control; on
PSX it differs in exactly seven nonzero prefix bytes.

## Native converter negative controls

Two distinct supplied binaries both identify themselves as CPE2X 1.3:

| Candidate | SHA-256 |
| --- | --- |
| Release 2.5 tool archive | `8ee3df02d30d9269bba8c570d69f3c9d2b59aff98af0fbf796367526bc02ef20` |
| Runtime 2.6 CD | `641d95ebe8131c3503407518cb6110ed311cb5f87943d866296660ab98938af2` |

The flake pins the [preserved Runtime 2.6 CD](https://archive.org/download/ps1_sdks/Programmer%20Tool%20-%20Runtime%20Library%20Version%202.6%20%28Japan%29%20%28En%2CJa%29_DTL-S2170_redump.zip)
and verifies each extracted tool hash. `PSYQ_RUNTIME26_BIN` exposes its
assembler/converter candidates separately from the original archive. Both
ASPSX candidates remain software-key protected; the second candidate's visible
version label does not establish its directive behavior.

`tests/test_executable.py` runs both unmodified converters through the pinned
DOSBox-X on synthetic CPE records. Both produce correct load bytes, entry and
sector size, but **zero tail padding**. The PSX-sized investigative control
also produces zeros with each converter. The compatibility rule is therefore
not attributed to either native binary. The runtime is never executed.

## Result and reproduction

The normal command emits a 4096-byte `PSX.EXE` with **zero differing bytes**,
including header and padding. Linked and retail SHA-256 are both:

```text
670f0ca702570fdb814a73ffa840b97a6584d2753072ea3b55c35541a7cec276
```

Run in `nix develop` after `kf init`:

```sh
kf try --unit psx.main
kf match --unit psx.main
kf link --image psx
python3 -m unittest tests.test_asm_sections tests.test_executable tests.test_workflow
```

Inspect `build/link/psx/comparison.json` for the full-file comparison, section
extents, source placements, whole-SDK verification and container provenance.
No game C change or new function match is needed: `main` remains strict 100%.

All **733 local repository tests**, Ruff, whitespace checks and
`nix flake check -L` pass. The flake test run has 147 expected skips for local
retail/artifact-dependent checks; the local run has none. A fresh focused
compile and repeated production link preserve the exact file hash. Only the
PSX `main` ledger fingerprint is re-banked for its new profile; its score and
the exact function count are unchanged, with no banked exact regression.

The full `kf build` remains red on existing reconstruction gates. Source data
now passes **15/61** units (PSX 1/1, GAME 11/41, OPEN 3/19), SDK data passes
4/4 contributions, and target relinking passes 110/116 units. PSX's remaining
known-reference gate records four unresolved indirect BIOS controls; its data
and target placement checks both pass. There are no comparison artifact
failures. All three executable links succeed, but GAME and OPEN remain
non-identical. No game execution is used as validation.
