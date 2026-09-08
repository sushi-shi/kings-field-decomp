# GAME item-database filename trace

## Function Match Plan

Continue from `bbddfd5`, following the item-name and shop-price owners used
by the active preview campaign. GAME `80020cfc item_load_database`, 1500
bytes in `game.item`, starts at strict 99.746666% with the unchanged pinned
GCC 2.5.7 profile. Hash verification, all six semantic views, the complete
retail body/CFG, caller and adjacent windows, source history and the earlier
parser-verification/closure dossiers precede this trial. Evidence is under
`build/gcc257/item-database/`; native and traced whole ELF bytes agree.

The sole direct caller is game initialization at `80014768`, passing no
arguments and ignoring any return. Preserve the void interface, 80-byte
frame (s0/s1/s2 and ra), forty-byte filename buffer, loaded resource pointer,
six fixed copies and fatal load-error path. The five static calls are memset,
the allocated resource loader, exit, memory release and CdSearchFile. There
are ten address pairs, six internal jumps, two literal paths and no indirect
transfers. This is game file/resource policy; SDK CD/BIOS boundaries retain
their existing archive/header lineage evidence.

The contiguous predecessor ends after `80020cf8`; the loader returns with
frame restoration at `800212d4`, before the exact item-menu root. All four
banked menu siblings are controls, and the unrelated floor-placement partial
must remain byte-identical to its baseline. The unit's RODATA contains the
literal `\\KF\\ITEM0\\I000.TMD;1` and `COM\\STAT.DAT` strings. Preserve every
copy size/owner, both literal strings, all eighty file searches, one-based
file numbers, thirty-file directory groups and conditional sector rounding.

All 375 words except sixteen register fields agree. The first difference is
the divisor-30 materialization at `800211b8`: retail uses a2 and the candidate
v1. Retail then forms `i + 1` in v0 and its quotient by 100 in v1; candidate
uses a2 for both. The final remainder/quotient by ten agrees. Signed divisions,
overflow/zero checks, their delay slots, ASCII additions and filename-byte
positions are unchanged. Previous movement of `n = i + 1` and direct digit
expressions had no effect and will not be repeated.

Test consuming the actual file-number working value after writing its
hundreds digit: `n %= 100`, then use n for the final two digits, removing the
separate remainder variable. This is decimal decomposition with the same
signed word arithmetic, not a constant/register carrier. Trace CSE's merged
divide/modulo outputs and allocation of the real number before comparing
the first raw difference. Do not change division signedness or remove the
hundreds-digit computation merely because the observed range is below 100.

Only strict 100% permits banking this function. Rebuild any retained source,
verify every word and ordered target plus exact siblings, then run Ruff,
repository tests, whitespace checks and the full build. Bank only exact
selected rows from the staged campaign; generated traces remain in `build/`.

The remaining-digits trial leaves strict objdiff at 99.746666%; CSE still
merges the shared divide/modulo outputs. No source change is retained from
it. Next test a named word-sized, one-based directory number (`i / 30 + 1`)
and encode that number with ASCII zero for filename byte eight. This makes
the actual ITEM1/ITEM2/ITEM3 numeric value explicit before the character
conversion, preserving the final +49 operation, signed division and all
filename results. Inspect the resulting division outputs and retained
directory-value lifetime rather than changing declarations to force a register.

The named-directory trial also remains at 99.746666%. One further arithmetic
ordering axis uses the existing remainder variable: compute `rem = n % 100`
before encoding the hundreds quotient. Retail produces both division outputs
before either corresponding filename store; expressing that decomposition
before the character writes preserves their order and all values, while
testing which divide/modulo expression establishes the compiler's shared
output pair. No local or operation is added by this trial.

## Verdict

All three trials emit the exact same 375 linked words as the starting
candidate, not merely the same score. Each still differs from retail in the
same sixteen register fields, beginning at `800211b8`. Five static call
targets, ten address materializations, the division checks and delay slots,
copy lengths, paths and sector-rounding flow are unchanged. All five sibling
functions preserve their complete baseline word arrays and ordered targets.
The four menu siblings remain strict 100%; the floor-placement loader remains
98.888885%. No source change or new bank entry results from this campaign.

The selected n/rem lifetimes and frame features do not produce a distinct
state in the remainder-first trial. Baseline RTL shows CSE combining each
hundreds/tens divide-and-modulo pair; the directory quotient remains v1,
where retail uses a2. The real file number remains a2, where retail uses v0,
and its hundreds quotient stays a2 rather than v1. This is a bounded negative
result for these source forms, not a proved optimizer limitation. The loader
remains open at strict 99.746666%; only a 100% match can close it.

## Remainder-width Function Match Plan at `34890aa`

Refresh all six GAME views, all 375 retail and candidate words, the caller,
adjacent boundaries, resource-loader interfaces, literal bytes, shared types
and source history. The baseline still has native/traced whole-ELF parity,
six internal jumps, ten ordered address pairs and five calls. Its sixteen
register-field differences start at `800211b8`; no referent, CFG, call or
immediate difference precedes them. All five siblings now reproduce retail
exactly, including the previously partial floor-placement loader. Preserve
the SDK CD/BIOS interfaces and their recorded library-lineage attribution.

Test only the remainder working value's width. Signed `n % 100` lies in
[-99, 99] for every signed-word n, so a signed-byte remainder represents
every possible result without changing either subsequent decimal operation.
The actual loop uses n = 1..80. This bound permits a source hypothesis; it
does not establish the original declaration. Keep n, the loop index and all
divisions signed-word, preserving retail's divide checks, byte positions,
40-byte name buffer, six copies and conditional sector rounding.

First compare reduced word/byte remainder controls with native/debug/traced
parity. Inspect whether the narrowing survives as instructions absent from
retail before applying this axis to an isolated copy of the complete unit.
Do not enumerate alternative widths or retain an ineffective type change.
Require raw words and exact siblings before any strict-100% banking claim.

The reduced controls emit identical 220-byte instruction streams. The byte
control initially expands its promoted remainder into a left/right shift
pair. Those shifts survive both CSE passes and disappear in COMBINE; the
division by ten then consumes the original signed remainder. The complete
item unit reproduces that same change at UIDs 320/321 and 324. This agrees
with the pinned `combine.c` sign-bit analysis of signed remainder results;
it does not establish the historical declaration or compiler.

The isolated byte-remainder trial emits all 375 baseline words and the same
ten ordered referents, retaining the sixteen register-field differences and
strict 99.746666%. All five exact siblings reproduce retail. Baseline, trial
and both reduced controls have native/debug/traced whole-ELF parity. Reject
the ineffective width change; no production C or bank entry changes.
Generated evidence is under `build/gcc257/game-item-remainder-traces/`.

## ASCII digit-width controls

Function Match Plan: freshly inspect GAME 80020cfc/1500 bytes, the full
375-word retail body/CFG, all six semantic views and sole initialization
caller at 80014768. The current 99.746666% body still differs only in the
sixteen recorded register fields. Preserve the 80-byte frame, six resource
copies, two paths, five calls, ten address pairs, six internal jumps, signed
divisions and filename-store order. Prior numeric decomposition and inline
helper trials remain negative controls; no vendor code is reconstructed.

Test explicit actual ASCII digits with signed-word, signed-byte or unsigned-byte
locals, alongside the direct-expression baseline. Every generated digit fits
all three representations. The first four-state manifest computes rem before
the directory digit; it reorders the /100 division ahead of /30 and therefore
cannot isolate width. Its scores are baseline 99.746666%, word 99.114670%,
and both bytes 98.045334%. Retain this as an ordering control only.

Correct the declarations' computation order to directory, hundreds, remainder,
tens and ones, and rerun four states. The word form is completely raw-identical
to baseline at 99.746666%. Signed and unsigned byte forms are raw-identical to
each other at 98.770670%; both recover a2 for the directory quotient and v1
for the hundreds quotient, but use v1 rather than retail v0 for i+1 and move
ASCII additions ahead of the first/third stores. All remain 1500 bytes.

A further four-state matrix separates directory-digit width from the three
filename digits. Directory-only narrowing remains raw-identical to baseline
(99.746666%); filename-only narrowing gives 98.741330%, and narrowing both
reproduces 98.770670%. Filename narrowing causes the register change but also
changes the store-adjacent arithmetic schedule; it does not explain retail.
Reject all trials and keep canonical source unchanged. No function is banked.

JSON reports: `build/hypotheses/20260908-184005-game-item-item_load_database`,
`build/hypotheses/20260908-184051-game-item-item_load_database`, and
`build/hypotheses/20260908-184138-game-item-item_load_database`. Independent
raw listings are under `build/item-digit-objects`, `build/item-digit-ordered-objects`
and `build/item-digit-components-objects`; the fresh full dossier is
`build/item-digit-evidence.txt`, and the baseline diff is
`build/item-current-compare.txt`.

## Numeric-digit and division-input widths

Function Match Plan at `663d13f`: hash-validate retail and refresh all six
GAME views for `80020cfc`/1500 bytes. Inspect the complete 375-word body,
existing caller/adjacency evidence, shared file-table boundary and prior
ASCII-width controls. Preserve the 80-byte frame, five calls, ten address
pairs, six internal jumps, six fixed copies, both paths, eighty searches,
signed division checks and byte-store order. The baseline remains strict
99.746666%, first differing at the divisor-30 register at `800211b8`.

First isolate numeric digits before ASCII conversion, rather than narrowing
the completed characters as in the previous experiment. A five-state JSON
matrix compares canonical expressions with explicit s32/s16/s8/u8 directory,
hundreds, tens and ones values. They are real consumed digit values, and all
fit each tested width. Every variant emits exactly the same complete raw
function listing and relocation annotations as baseline: 1500 bytes,
99.746666%. Numeric-digit width therefore does not explain this residue.

Next isolate the actual division inputs: the one-based file number and its
remainder modulo 100, both ranging from 1 through 80 in this loop. A 25-state
matrix independently tests s32/s16/u16/s8/u8 for each. No state reaches exact.
With a word file number, signed-halfword and signed-byte remainders are
raw-identical to baseline. Narrow signed file numbers instead introduce a
shifted induction value, another saved register and an 88-byte frame; the
signed-remainder forms become 1520 bytes and 98.106670%. Unsigned file-number
or remainder forms introduce masks and unsigned division, removing signed
overflow checks present in retail; their 1456..1500-byte results are not a
source explanation even when an individual register assignment agrees.

Reject all controls and leave canonical source unchanged. This rules out
these numeric-width hypotheses under the current pinned profile; it does
not establish the original local types or a compiler limitation. No new
function is banked. The complete fresh dossier is
`build/item-numeric-evidence.txt`. Five-state results are under
`build/hypotheses/20260908-190954-game-item-item_load_database`, and the
25-state results under
`build/hypotheses/20260908-191049-game-item-item_load_database`.
Separate compiled/raw inspections are `build/item-numeric-objects` and
`build/item-number-width-objects`.


## Per-file lookup inline boundaries

Function Match Plan at `2df69b2`: refresh retail hashes and all six GAME
semantic views for `80020cfc`/1500 bytes, read the complete 375-word body,
sole initialization caller, contiguous source neighbors, shared CD entry
layout and source history. The exact floor-placement loader is now an
additional control alongside the four exact menu siblings. Preserve the
80-byte frame, 40-byte path, six resource copies, five static calls, ten
address pairs, six internal jumps, both strings, eighty searches and all
signed division checks. Prior formatter-only helpers and numeric-width
controls do not explain the first difference at `800211b8` (+4bc).
`CdSearchFile` retains LIBCD ISO9660 archive/header lineage; `memset` and
`exit` retain LIBAPI signature evidence. No library body is reconstructed.

Test the cohesive per-file formatting/search/conditional-size-rounding
operation as a static inline helper. The global-index interface takes the
real path and item index; the selected-entry interface additionally receives
the proven 20-byte `KfCdFileEntry` pointer. Preserve the existing SDK cast
only at the search call. These are boundary hypotheses, not claims that
retail used helpers.

All three JSON states compile. Global-index lookup emits all 375 linked
words, five call targets and ten address pairs identically to baseline at
99.746666%. Selected-entry lookup yields 1496 bytes and 98.381330%, first
differing at +4ac: it removes the independent table-size cursor and its
address pair, then retains the chosen entry in s0 across the search and
accesses size at +4. Nine address pairs remain; the quotient residue is not
resolved. All five siblings retain complete retail instruction equality.
Reject both helpers and keep canonical source unchanged.

A second Function Match Plan narrows the boundary to formatting plus search,
leaving conditional rounding in the caller. Test returning the authentic
`CdlFILE *` search result or its consumed word-sized nonzero status. Both
interfaces inline to the exact same 375 baseline words and ordered targets,
with 99.746666%, first difference +4bc and all siblings unchanged. Neither
helper is retained; no source, identity or bank changes result.

Generated reports are
`build/hypotheses/20260908-205115-game-item-item_load_database` and
`build/hypotheses/20260908-205204-game-item-item_load_database`.
The refreshed dossier is `build/item-lookup-inline-evidence.txt`;
independent compile/raw controls are under `build/item-lookup-inline-objects`
and `build/item-find-inline-objects`. Each JSON matrix includes the unchanged
canonical baseline and has no compile errors. Generated files are not committed.

Ruff, all 713 tests (113.746 seconds) and `git diff --check` pass.
Full `kf build` retains GAME 337/362 exact, 99.182% aggregate similarity,
source-data matches 11/41 GAME, 3/19 OPEN and 0/1 PSX, and target relink
75/77, 34/38 and 1/1 respectively. No artifact failures occur; the full
build remains nonzero on existing incomplete closure checks.

## Loop-index width control

Function Match Plan at `0e6fe05`: refresh hashes, all six GAME semantic views
and all 375 retail words for `80020cfc` / 1500 bytes / 99.746666%. Review the
sole initialization caller, source neighbors/history, six resource copies,
actual CD entry layout and SDK attribution. The frame remains 80 bytes,
with five static calls, ten data pairs, six internal jumps, both literal
paths and eighty file searches. The source copies already match retail;
only sixteen filename-calculation register fields differ, starting at +4bc.

Earlier width trials changed numeric digits or the one-based number and
remainder, not the loop index itself. Test the existing s32 counter against
s16/u16/s8/u8 in a five-state JSON matrix. All can represent the full 0..80
induction range; this permits the experiment but does not prove a narrow
original declaration. Keep the same loop, digit expressions, word n/rem,
file-table indexing, stores and sector rounding. Require retail's signed
division checks and table traversal, not merely equivalent filenames.

| Counter | Bytes | Strict % | Data pairs |
| --- | ---: | ---: | ---: |
| s32 | 1500 | 99.746666 | 10 |
| s16 | 1540 | 94.146670 | 11 |
| u16 | 1496 | 93.149330 | 11 |
| s8 | 1540 | 94.146670 | 11 |
| u8 | 1496 | 93.149330 | 11 |

All four narrow variants first differ at +4: the saved-ra offset changes
and the s2 save disappears, while frame extent stays 80 bytes. They lose
the independent file-size cursor, form additional table-size references,
and insert extensions or masks around the induction value. The unsigned
variants also replace the directory's signed division with DIVU and remove
its signed-overflow guard. The signed variants retain signed arithmetic
but grow the body and alter the later digit computation/traversal. Equal
scores for paired widths do not imply identical instructions: the extension
shift or mask still encodes the chosen width.

Reject all four; retain canonical s32 source, all literals and identities.
Independent compilation resolves every trial's words and relocations,
confirms five ordered call targets and verifies all five sibling functions
against complete raw retail bodies and ordered referents. No function is
banked. The manifest is `build/item-loop-width-hypotheses.json`, the fresh
dossier is `build/item-loop-width-evidence.txt`, and the report is
`build/hypotheses/20260908-214031-game-item-item_load_database`.
Separate objects and linked-word comparisons are in
`build/item-loop-width-objects`.

Full `kf build` preserves GAME 337/362 exact / 99.428% aggregate. Existing
source-data matches remain GAME 11/41, OPEN 3/19 and PSX 0/1; target relinks
remain 75/77, 34/38 and 1/1, with zero artifact failures. The existing closure
gates still return nonzero. This follow-up changes documentation only; the
preceding successful 713-test/Ruff run covers the unchanged production code,
configuration and tests. Whitespace checks pass again.
