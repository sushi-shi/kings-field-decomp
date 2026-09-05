# OPEN formatter, display adjustment, and small SDK census review

This campaign covers the user's two ordered OPEN-only lists: fourteen starts
of 8–200 bytes, followed by five starts of 224–812 bytes. The per-function
pre-edit snapshots and verdicts are in `open_semantic_format.tsv`,
`open_semantic_display_adjust.tsv`, and `open_small_sdk_review.tsv` under
`config/evidence/`. Every address below is in OPEN unless qualified otherwise.

## Formatter: four exact C functions

| OPEN start | Bytes | Identity | GAME homolog |
| --- | ---: | --- | --- |
| `8001a3fc` | 224 | `format_int_dec` | `8003a81c` |
| `8001a4dc` | 140 | `format_int_hex` | `8003a8fc` |
| `8001a568` | 108 | `format_pad_left` | `8003a988` |
| `8001a5d4` | 576 | `format_vsprintf` | `8003a9f4` |

The independent overlays have identical instruction shapes throughout these
four bodies; the padding helper is also byte-identical. The complete call
graph, scratch sharing, matching source shape, and gapless run support the WIP
`open.format` module. They do not establish an original filename. The following
24-byte no-op diagnostic sink remains in its already verified unit, and the
preceding managed-voice function has a distinct object/API family.

Decimal conversion uses a signed 32-bit input and divisor, while hex conversion
uses unsigned division and uppercase digits. Both suppress leading zeroes with
byte counters and put the NUL store in the return delay slot. Padding measures
and subtracts with byte-wide wraparound, then writes before the supplied string.
The formatter accepts `d/D`, `x/X`, `s/S`, widths `1` through `8`, and numeric
zero padding. Newline becomes carriage return. It counts the final NUL in its
return value. Unsupported-format behavior and signed-minimum negation are
preserved, not silently replaced by standard `sprintf` behavior. These bodies
are game-local diagnostic policy, not reconstructed Sony `printf` providers.

The digit anchor is `80037978`. The widest supported padding of a one-digit
number reaches seven bytes earlier. A sign, ten decimal digits, and NUL need
twelve bytes starting at the digit anchor. The source therefore models the
minimum accessed scratch span as `format_number_storage[19]` at `80037971`;
all four HI16/LO16 references retain the **+7 addend**. This is a candidate
storage boundary, not a recovered complete allocation. Its original outer
bounds remain unresolved. There are no overlapping interior globals or
out-of-array prepends in this OPEN model.

With the repository's current `probe-gcc257-o2-g0` profile, all four functions
are strict objdiff **100%**. All four scratch pairs and sixteen control-flow
references have been reviewed. Exact bytes under this productive probe do not
prove the historical compiler or original source spelling.

## Display adjustment: complete C, non-exact

`display_adjust_vram_view` at `8001a82c` is 584 retail bytes. It clears both
DRAWENV background flags, toggles the buffer byte, waits for GPU/frame sync,
submits the selected DISPENV, disables selected DRAWENV drawing-on-display,
and submits that DRAWENV. It saves the selected eight-byte `RECT`, waits for
`PADh` release, then pans its VRAM coordinates by four per directional input.
The loop masks x to 1023 and y to 511 before submitting the display environment.
After another press/release of `PADh`, it restores the saved rectangle and sets
the selected `dfe` and both `isbg` flags to one. There is no final `PutDispEnv`.

Authentic pinned SDK types matter: DRAWENV is 92 bytes, DISPENV is 20 bytes,
RECT is eight bytes with two-byte alignment, and `dfe` is offset 23, not the
adjacent `dtd` at 22. The RECT assignment explains the retail `lwl/lwr` and
`swl/swr` copies without a fabricated word-aligned replacement. The old
`LIBETC.H` spelling for mask `0800` is `PADh`, not `PADstart`.

The ordered call set is DrawSync, VSync, PutDispEnv, PutDrawEnv, PadRead,
PadRead, VSync, PutDispEnv, PadRead. Ten address pairs and the internal jump
are reviewed, including newly admitted base constructions at
`8001a874/878` (DISPENV) and `8001a8d8/8dc` (DRAWENV). Their exact carry-adjusted
targets and existing aggregate owners are preserved.

The first C reconstruction is **95.678085% strict objdiff**. `kf try`'s separate
text similarity is 92.3%; it is not the matching criterion. Compared from the
first divergence, retail saves ra at sp+32 and only s0/s1, whereas the probe
saves ra at sp+36 and also s2 in the same 40-byte frame. Retail starts with
`lui at; sb zero,+24(at)` for the first background flag and later reconstructs
the DRAWENV base in v0. The probe retains that first field address in s1, then
subtracts 24 before PutDrawEnv, using s2 for the DISPENV base. The loop, RECT
save/restore, masks, directional stores, and final flag stores otherwise agree.
The compiled function is 588 bytes including the return delay slot, versus
584 in retail; section alignment adds a separate trailing nop.

This is an **unattributed address-base reuse / saved-register residue** after
referent, call, CFG, width, constant, and delay-slot review. No source facts
justify separate overlapping DRAWENV globals, fake locals, volatile accesses,
or forced assembly. The function is not banked as exact. The exact historical
compiler/profile and source organization remain open.

## Twelve SDK providers, not twelve game matches

| OPEN start | Retail bytes | Provider | Pinned member offset |
| --- | ---: | --- | --- |
| `80022c7c` | 176 | `SpuVmSelectToneAndVag` | VMANAGER `ad0` |
| `80022d2c` | 380 | `SpuVmDoAllocate` | VMANAGER `b80` |
| `80022ebc` | 16 | `SpuVmDamperOff` | VMANAGER `d54` |
| `80022ecc` | 812 | `vmNoiseOn` | VMANAGER `d64` |
| `800231f8` | 200 | `vmNoiseOn2` | VMANAGER `10c0` |
| `8002c308` | 84 | `LoadAverage12` | MSC `68` |
| `8002c35c` | 84 | `LoadAverage0` | MSC `b8` |
| `8002c3b0` | 140 | `LoadAverageShort12` | MSC `108` |
| `8002c43c` | 140 | `LoadAverageShort0` | MSC `190` |
| `8002c4c8` | 96 | `LoadAverageByte` | MSC `218` |
| `8002c528` | 120 | `LoadAverageCol` | MSC `274` |
| `8002c7b4` | 148 | `SquareRoot12` | MSC `4fc` |

These mappings use actual Release 2.5 `LIBSND.LIB/VMANAGER.OBJ` and
`LIBGTE.LIB/MSC.OBJ`, their symbols/debug extents, authentic SDK headers,
retail calls/GTE operations, and independently reviewed GAME homologs. Extraction
and symbol evidence are reproducible with `psyk extract` and `psyk list --code`.
The later PSX-loader 2.60 signature catalog is only corroboration for symbol
presence, not proof of a matching archive revision or member offset.

The five VMANAGER functions plus the already classified intervening
`SpuVmDamperOn` form one constant-delta six-function overlay run. All 401
instruction shapes agree, with 271 byte-identical words. Real archive versions
differ: `SpuVmDoAllocate` is 448 bytes in the pinned archive and lacks the same
retail loop/mask choices; `vmNoiseOn` and `vmNoiseOn2` are 860 and 248 archive
bytes, respectively. Their provenance is `sdk-lineage-supported`, not an exact
Release 2.5 byte claim.

The six LoadAverage functions are completely byte-identical across GAME and
OPEN (166 words). Against MSC, each retail body has exactly one duplicated
four-byte operation: `swc2` at +40 hex for the VECTOR helpers, `mfc2` at +60
for the SVECTOR helpers, +40 for Byte, and +50 for Col. Removing that one
adjacent identical instruction leaves an unmasked exact archive comparison.
Tests verify both the duplicate and all remaining bytes, with a corrupt-byte
negative control. This supports SDK ownership while retaining version skew.

SquareRoot12 is exact to the pinned archive under its real relocation mask:
only four bytes belonging to the two table-address immediates are masked.
The former starts `8002c834` (12 bytes) and `8002c840` (8 bytes) are internal
right-shift and zero-result tails. Branches at `8002c820` and `8002c7c8`, with
live internal registers, prove they belong to the preceding body. The census
now has one 148-byte function with two labels, not three unrelated functions.

The GAME/OPEN SquareRoot12 pair has 35 exact words and 37 matching instruction
shapes. Its wider LoadAverage neighborhood is not treated as one original TU
merely from address proximity. `overlay_lineage.tsv` uses three separately
checked groups for this campaign.

The pinned MSC object SHA-256 is
`a7f13f0c9d824310fdaf4ccd939aab41496fe364fc16c3c99b6d8d482133c937`;
LIBGTE archive SHA-256 is
`3e0fbd64b24c6b12133708f032a45c4a3888dc04bd6ff433ea9421e0cce2d790`.
Neither archive bytes nor generated signature reports are committed.

## Accounting and controls

The first fourteen starts resolve to two exact C functions, ten SDK providers,
and two false starts. The second five resolve to two more exact C functions,
two SDK providers, and the non-exact display routine. OPEN moves from
87 exact / 101 started / 122 eligible to **91 exact / 106 started / 108 eligible**.
The denominator reduction is twelve SDK providers plus two false starts, not
fourteen reconstructed game functions. The two remaining unstarted eligible
functions are `80014e28` (1896 bytes) and `8001764c` (3320 bytes), outside this
user-selected batch.

`tests/test_open_small_sdk.py` verifies provider scope, corrected boundaries,
overlay bytes, real MSC operations/masks, and VMANAGER private symbols/extents.
`tests/test_open_format_display.py` checks function claims, image-qualified
identities, bounded scratch and its interior addend, ordered reviewed calls
and references, retail masks/copies/delay slots, and actual SDK layouts. The
layout fixture rejects `dfe` at the wrong neighboring byte. These are static
binary/compiler checks; the game is not executed.

Verification on the `3d0adb4` base passes the full `kf build`, OPEN's strict
historical-baseline check, all 367 repository tests with the existing offline
Rust driver built (no skips), `ruff check scripts tests`, and `git diff --check`.
The staged `nix flake check -L` also passes; its isolated test run skips the
29 controls requiring unavailable local retail, generated objects, or tools.
All 349 previously exact game functions remain exact; four new formatters bring
the three-image exact count to 353. All 13 vendor-source controls and all 61
data-owning units match. Only the four exact `open.format` rows are banked.

The all-image `kf check --strict` retains four pre-existing GAME historical-best
discrepancies; this campaign edits neither their source nor their ledger rows:

| GAME start | Current strict score | Historical best |
| --- | ---: | ---: |
| `8001d730` | 60.490950 | 60.764706 |
| `8001de18` | 17.954199 | 18.305344 |
| `8001f218` | 91.250000 | 92.960230 |
| `8001fafc` | 97.452515 | 98.296090 |
