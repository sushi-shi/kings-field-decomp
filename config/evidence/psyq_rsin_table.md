# SDK sine-table ownership and biased relocation bases

## Function Match Plan

Recover the complete `rsin_tbl` allocations in GAME and OPEN from the pinned
Psy-Q archive. Review each sine/cosine address pair against retail instructions,
quadrant control flow and the SDK's original relocation expression. Preserve
the decoded address `S+A`, but make semantic navigation and reachability follow
the explicitly reviewed allocation `S`. Do not reconstruct vendor bodies as
game progress, change game source, or claim a default data comparison merely
because a config owner now exists.

Baseline: `d3717f1`. The preceding OPEN scene campaign found an apparent
`sin_1` reference into ending-camera data. The corresponding GAME base was
misidentified as `talk_image_path_template+4`. Address equality alone does not
establish the allocation read after the index is added.

## Independent SDK and retail evidence

The pinned Release 2.5 `LIBGTE.LIB` member `GEO.OBJ` exports global `rsin_tbl`
at `.data+0`; the section has alignment 8 and a single 2048-byte contribution.
It is a 1024-entry quarter-wave table, read with signed halfword loads.
`LIBGTE.H` declares `int rsin(int a)` and `int rcos(int a)`. The existing vendor
inventory already attributes both and their internal `sin_1` helper to GEO.

| Artifact | SHA-256 |
| --- | --- |
| `LIBGTE.LIB` | `3e0fbd64b24c6b12133708f032a45c4a3888dc04bd6ff433ea9421e0cce2d790` |
| `GEO.OBJ` | `7e62d2e92b7a30001549db9b60abaed0811db7db2ed213e3eca2e62d8a7db36e` |
| Complete `.data` payload | `74743e361fc4d78cbd41bd99b2acf5c75c9b5b0268ccd753ed282ec4394fb25a` |

| Image | Retail allocation (exclusive end) | File offset | SDK owner |
| --- | --- | --- | --- |
| GAME | `80057070..80057870` | `45870` | `GEO.OBJ:.data+0` |
| OPEN | `800367e8..80036fe8` | `24fe8` | `GEO.OBJ:.data+0` |

Both complete payloads equal the SDK bytes. Each old three-row census split
was a gap containing a 39-byte false ASCII hit and another gap. Replace those
fragments with the complete typed allocation and retain the five preceding
bytes as an unresolved gap. No bytes are dropped or materialized in game C.
The data identities are image-local, global load storage, `s16[1024]`, owned by
`libgte_geo`; the name and extent come from the SDK, not a guessed game meaning.

## Per-function evidence snapshot and verdict

SDK `.text` is based at `retail_rsin - 930`. Tests apply the original SDK
section-base patches before comparing every instruction, including delay
slots. This is an unmasked SDK/retail equality control, not a compiler-probe
objdiff score or a new banked game reconstruction.

| Function | GAME VA / size | OPEN VA / size | Evidence and final verdict |
| --- | --- | --- | --- |
| `rsin` | `8004f27c / 50` | `8002f050 / 50` | Signed angle, absolute value and modulo 4096; 24-byte frame; two conditional calls to `sin_1`; return with stack-pop slot. No table reference in this wrapper. Original SDK relocated body equals each complete retail body. Retain vendor classification. |
| `sin_1` | `8004f2cc / b8` | `8002f0a0 / b8` | Four unsigned quadrant tests, `lh` table reads, sign inversion in quadrants 2/3, common `jr`/nop tail; four table pairs and three internal jumps. Original SDK relocated body equals each complete retail body. Retain vendor classification. |
| `rcos` | `8004f384 / d4` | `8002f158 / d4` | Signed absolute angle and modulo 4096; direct/reversed quarter-table indexes, signed halfword results, four table pairs and three internal jumps; `jr`/nop tail. Original SDK relocated body equals each complete retail body. Retain vendor classification. |

Retail disassembly, direct callers/callees, string/data xrefs, neighboring SDK
functions, current vendor-only match state and inventory history were inspected.
The game callers remain unchanged. The functions are contiguous inside GEO's
text contribution; no new game TU or speculative source boundary is claimed.

## Original relocation expressions

The SDK uses patch types 82/84 for each HI16/LO16 pair, against `.data` section
`8cf`. Each LO site is four bytes after its HI site. Values below are byte
addends, not halfword indexes. Types 74 retain internal `.text` jump targets.

| SDK HI offset | GAME HI site | OPEN HI site | Addend to `rsin_tbl` |
| --- | --- | --- | --- |
| `98c` | `8004f2d8` | `8002f0ac` | `0` |
| `9bc` | `8004f308` | `8002f0dc` | `0` |
| `9e4` | `8004f330` | `8002f104` | `-1000` |
| `a18` | `8004f364` | `8002f138` | `0` |
| `a70` | `8004f3bc` | `8002f190` | `0` |
| `a98` | `8004f3e4` | `8002f1b8` | `-800` |
| `ac8` | `8004f414` | `8002f1e8` | `0` |
| `af4` | `8004f440` | `8002f214` | `-1800` |

For example, the third `sin_1` quadrant admits angles 2048..3071. After
`2*angle` is added to `rsin_tbl-1000`, the actual reads are table offsets
`0..7fe`, not the data/code containing the biased base. The original SDK
expression is `(sectbase(8cf)+$fffff000)`. The two biased cosine quadrants
similarly use `$fffff800` and `$ffffe800`. In OPEN the last expression even
falls numerically inside `sprintf`; it is not a function or callback pointer.

All sixteen curated rows now name `rsin_tbl`, with status `reviewed` and
`paired-reviewed` confidence. Their raw target VAs, sites, opcodes, registers
and pair ordering remain unchanged. Conservative byte validation makes the
reference tier `validated`, not `proven`: the explicit owner is supported by
the SDK and quadrant proof, not by the `--confirmed-only` display filter.

## Shared referent contract and controls

`named_data_referent` resolves an explicit image-local data identity separately
from `S+A`. An address-derived owner spelling resolves its exact start and
uses the curated symbol. Duplicate owner names fail rather than choosing an
arbitrary allocation. Unknown names are not promoted into known allocations.

For reviewed data-address/pointer rows, the delinker gives that owner priority
over coincident numeric code or unrelated RODATA targets. Unit-owned RODATA
keeps its section-relative representation based on the allocation's actual
section membership, including a biased address outside that section.
The semantic reference retains raw `target`
and adds `referent` plus signed `addend`; xrefs, disassembly, string/inventory
navigation and reachability use the real object endpoint. Competing explicit
owners remain separate candidate references. Rejected or byte-invalid rows
cannot redirect traversal; candidate owner spellings do not override code.

Taking a biased address or one-past pointer does not read the numerical base.
The reachability boundary control anchors such expressions to the allocation;
direct load/store lows still check their true accessed address and width and
report out-of-owner accesses. Genuine allocation/code overlaps remain errors.
This does not prove dynamic index bounds in arbitrary game functions.

`tests/test_data_referents.py` covers image isolation, exact-start aliases,
ambiguous/competing/rejected owners, code coincidences, pointer initializers,
signed addends, one-past pointers and retained direct-memory bounds failures.
`tests/test_psyq_rsin_table.py` checks full inventory extents/classification,
the SDK export/hash/patches, all six unmasked relocated bodies, both payloads,
all sixteen target address pairs and their exact delink/relink restoration.

Reproduction, inside `nix develop` after hash-validated `kf init`:

```sh
python -m unittest tests.test_data_referents tests.test_psyq_rsin_table
kf sema --image open xref rsin_tbl
kf sema --image game disasm sin_1
kf verify reachability --output build/rsin-reachability.json
```

## Remaining closure boundary

This is the boundary at this campaign's checkpoint. The subsequent
[config-data integration](config_data_contributions.md) supplies the default
object/comparison lane described below for these two complete SDK owners.

These are complete, independently identified SDK data owners, but config-only
data do not yet have a default standalone delink/comparison lane. The default
reachability gate must keep reporting them as config-only-not-compared. The
SDK equality controls do not close that integration gap. No game body, DATA
claim, source initializer, compiler profile or game/vendor denominator changes.
No function is banked and no linked-executable equality is claimed.

## Final campaign verification

All 112 source units were recompiled and every target redelinked. All compiled
objects and all 484 function score rows remain byte-identical to the baseline
comparison inputs/results, preserving 354 exact game functions. Of 1717 target
objects, only the four individual SDK `sin_1`/`rcos` objects change, carrying
the sixteen corrected address pairs. All 112 manifested module targets and
the other 1601 individual function targets remain byte-identical. Each changed
pair restores the original retail words when its signed addend is reapplied.

The graph replaces two arbitrary table-prefix gaps with the complete SDK
allocations. The same shared rule also exposes the already reviewed
`floor_entry_cells-2` reference in GAME, which previously followed the preceding
object. Config-only reached ranges therefore rise from 665 to 666: GAME
386 → 387, OPEN 279 unchanged. This is newly visible work, not matched progress.

All 502 local repository tests, Ruff and `git diff --check` pass. The flake
checks pass with 56 local-retail/oracle controls skipped in their sandbox.
The full default build remains red: strict data is 13/60, target relinking is
106/112 with the same six placement conflicts, and reachable-byte closure
remains incomplete. Neither SDK table is falsely counted as default-compared.
