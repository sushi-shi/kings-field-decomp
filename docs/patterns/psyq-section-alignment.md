# LNK alignment tags are not ELF byte alignments

## Campaign and evidence boundary

Baseline `bf051cf`: strict source data is 6/60, independent SDK data 4/4,
and target relinking 110/116. A read-only placement audit finds 47 invalid
source-section alignments. Nineteen source units have matching complete data
contents/layout and fail only on those alignments. Two more units have matching
data contents but conflicting section bases; changing alignment cannot fix
their ownership. These observations do not establish an original assembler.

Plan: inspect the compiler/maspsx/GAS boundary, then calibrate original Psy-Q
linker placement independently. Keep source C, function claims, profiles,
DATA/RODATA extents, ordered referents and every function score unchanged.
Correct a format conversion only if independent inputs demonstrate the rule;
do not derive ELF constraints from convenient retail addresses.

## Original-linker controls

The supplied **PSYLINK 1.17** executable has SHA-256
`b98e7180fc33b4c94adf41dcf9cc73f469864d23aabb3f3fcedf5e9f2c1ca040`.
`tests/psylink_alignment_smoke.py` constructs minimal, synthetic LNK v2
data-only objects: processor 7, a section-definition record, section switch,
complete byte record, offset-zero XDEF and EOF. The independent `psyk` listing
checks each input's version, processor, raw tag, extent and export. Neither
retail bytes nor the production importer supplies an expected output.

Original PSYLINK runs 88 cases under the flake's DOSBox-X. Each case links two
inputs in one `.data` section. The first payload has 1, 3, 9 or 17 bytes; the
second is the complete three-byte sequence `a5 00 5a`. Aligned and misaligned
`/o` origins, equal and mixed input tags, and unsupported-bit controls distinguish
per-contribution alignment, initial-origin rounding and payload extent.

| Raw LNK v2 tag shown as “alignment” by `psyk` | Observed input byte alignment |
| --- | ---: |
| 2 | 1 |
| 4 | 2 |
| 8 | 4 |
| 16 | 16 |

For a nine-byte first payload at `80010000`, a second input tagged 8 starts at
`8001000c`, **not** `80010010`. Tag 4 starts it at `8001000a`. A 17-byte first
payload distinguishes tag 16's sixteen-byte requirement (`80010020`) from an
eight-byte hypothesis (`80010018`). The first origin is separately rounded to
four bytes before applying the first input's requirement: an origin ending in
`03` yields `04` for tag 2/4/8, but `10` for tag 16.

Tags 1, 32 and 64 do not impose the extrapolated alignment: the second input
can start at `80010009`. They remain unsupported by the importer. No generalized
division, shift or meaning for unknown flag combinations is inferred.

Every case checks both exported addresses and **the complete CPE byte stream**:
header, select-unit record, each load record's address and exact payload, and
EOF. Alignment holes are absent load ranges, not emitted zero payloads. Explicit
zero bytes inside a payload remain present. There is no game execution.

## Kept SDK conversion

`config_data.parse_sdk_section` now requires one LNK v2 object and maps only the
four calibrated tags to ELF byte alignment. `SdkSection.lnk_alignment` and
comparison evidence retain the raw value separately from `alignment` and
`sdk_alignment`. Unknown versions/tags fail closed; no SDK patch, reservation,
symbol or unsupported record is discarded.

Both `LIBGTE.LIB/GEO.OBJ:.data` and `LIBSND.LIB/VMANAGER.OBJ:.data` have raw tag
8. Their four GAME/OPEN contributions therefore change from the incorrectly
copied ELF constraint 8 to the calibrated constraint **4**. Member hashes,
complete payloads (2048 and 386 bytes), exports/private linkage, curated
addresses/extents and source-versus-target independence are preserved.
The TSV alignment column remains a byte count, not a raw-format tag.

An ELF integration control parses raw tag 8 and places a full nine-byte
provider at a four-mod-eight address. It requires both genuine GNU relinks,
all nine bytes, the four-byte ELF constraint and raw tag 8 in evidence. Under
the old conversion it fails. Other controls reject wrong version, unknown
tags, mismatched provider alignment, changed linkage, truncated payloads and
stale native reports. Both SDK sides still pass the same strict comparator.

## Game compiler alignment remains unresolved

The pinned maspsx changes `.data` to `.section .data` and `.rdata` to
`.section .rodata`; it forwards explicit `.align` directives. The alignment
TODO in its small-data **prepass** is not evidence that the emitted directives
are discarded. GNU `as` still applies its sixteen-byte ordinary `.data`/BSS
minimum; `-no-pad-sections` removes automatic tails, not that constraint.
Compiler-emitted `.align 3` before switch tables still requires eight bytes.

The original ASPSX remains key-protected. SDK LNK metadata plus a linker test
does not establish how that assembler translated every compiler directive or
what the original game TU boundaries were. Consequently this campaign does
**not** rewrite game ELF alignment, compiler assembly, COMMON allocation,
RODATA claims, or placement checks. A four-byte-aligned delinked target is a
curated model, not a recovered section header from the PS-X EXE. The old map
panel note claiming that such metadata proved `.align 2` is corrected.

## Reproduction

Inside `nix develop`, use `PSYLINK_DOSBOX` pointing to the DOSBox-X supplied
by the flake's `psylink-order` check, then run:

```sh
python3 tests/psylink_alignment_smoke.py
python3 -m unittest tests.test_config_data tests.test_psyq_pitch_table tests.test_psyq_rsin_table
kf build
nix flake check -L
```

The flake now runs the original order and alignment controls together. No
generated objects, CPE/SYM files, reports or local retail paths are committed.

## Verification

All 88 original-linker controls and 612 local repository tests pass, with no
local skips, along with Ruff and diff checks. The required full build regenerates
all four SDK providers/targets and their native reports. Their eight ELF data
sections now require four-byte alignment; each complete 386/2048-byte payload
keeps its prior SHA-256. All four native SDK comparisons and both sides' GNU
relinks still pass, covering 4868 bytes across GAME and OPEN.

`nix flake check -L` also passes, including the 88 original-linker cases,
existing archive-order checks, native compiler/objdiff controls and 612
sandbox tests with 112 expected local-artifact/tool skips.

All 112 source objects, all 112 source-owned target objects, and every one of
484 function-score rows are unchanged. The 360/471 exact game functions and
thirteen exact vendor controls remain exact. No C, claim, identity, function
baseline or game compiler alignment changes; nothing is newly banked.

Strict source data stays 6/60 and target relinking 110/116, with the same six
conflicting bases. Reachability retains 620 unmatched reached config ranges.
The full default build remains red on these data/ownership/placement failures,
without compilation or delinking errors. This fixes SDK format fidelity; it
does not close additional source data units or prove linked-image equality.
