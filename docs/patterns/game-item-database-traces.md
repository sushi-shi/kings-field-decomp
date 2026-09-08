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
