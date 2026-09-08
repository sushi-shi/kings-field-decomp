# Objdiff local branch comparison

The pinned objdiff 3.7.3 revision can report a false 100% function match when
a function moves in its section and a zero-sized local label aliases its
start. Equal section-relative jump encodings do not establish equal
function-relative destinations.

## Reproduction and mechanism

The synthetic MIPS control in `tests/objdiff_mips_smoke.py` is a 24-byte
function: `j`/`nop`, `jr ra`/return 1, `jr ra`/return 2. Retail selects the
second return pair at function +16. A wrong candidate selects +8. Moving
that candidate by eight bytes makes its encoded `R_MIPS_26 .text` jump
address equal to the target's. Adding the compiler-style zero-sized local
`LM7` label at its start also lets objdiff's relocation-address fallback
accept the referent. The instruction comparison then skips argument
comparison because both raw instruction bytes and relocations compare equal.

Before the fix, the wrong candidate with both the prefix and local label
scores 100%; the unshifted wrong candidate scores below 100%. Omitting the
local alias also rejects the shifted wrong candidate. Correct destinations
score 100% in all four placement/alias combinations. The regression check
asserts the selected function's score rather than accepting a 100% value
anywhere in the JSON report.

`patches/objdiff-branch-destination.patch` also compares the aligned
instruction-row indices of branch destinations before using the fast path.
A mismatch triggers the existing argument comparison, whose `BranchDest`
rule already compares those indices. Comparing raw section addresses would
reintroduce the defect; comparing function offsets alone would ignore
instruction alignment across inserted/deleted rows.

## OPEN campaign evidence

In `opening_ending_scroll_run`, the jump at +3ac targets +418 in retail and
+414 in the kept candidate. A diagnostic CSE-option change grows the
preceding function by four bytes without changing any function-relative
instructions in the ending. Its displayed score nevertheless rises from
99.917694% to 99.927986%, including with `functionRelocDiffs=all`, because the
wrong jump's encoded section address now coincides with retail. This is a
comparison artifact, not matching progress. The compiler options are not
adopted; the preceding sibling also regresses.

The patch changes comparison tooling only. Synthetic instructions in the
negative control are not reconstructed game source.

## Upstream snapshot audit

The general fast-path correction also changes one existing PowerPC fixture.
In `LoadData__20CDamageVulnerabilityFR12CInputStreami`, aligned row 19 has
identical PC-relative `b +24` encodings. Retail's branch at section 1424
lands on aligned row 28 (`cmpw r27, r28`); the candidate's at 1412 lands on
row 25 (`lwz r0, @67@sda21`), before three additional loads/setup instructions.
The old fast path hid this destination mismatch. The function score changes
from 72.6% to 72.54737%, and the aggregate `.text` snapshot changes from
58.466667% to 58.460133%. The patch updates only that numeric expectation;
all other upstream snapshots remain unchanged. Expected-output regeneration
was confined to a diagnostic derivation used to inspect this difference.

## Verification

The normal patched derivation passes its upstream tests with fixed snapshots,
and all eight MIPS destination controls plus the existing leaf/relocation
controls pass. The original OPEN kept and shifted candidates now both score
99.917694%, with the disputed jump marked as an argument mismatch.

A full rebuild preserves all 427 game exacts and thirteen vendored exacts,
including OPEN 106/108. Eight already non-exact GAME functions receive lower
fuzzy scores; comparing both binaries against the same objects confirms newly
reported branch-destination mismatches, with no loss of an exact function.
Ruff, 703 repository tests (nine skips), `git diff --check` and
`nix flake check -L` pass. Both affected OPEN units were recompiled. Full
`kf build --reconfigure` still fails the existing data/ownership/relink checks;
the data-match and target-relink counts for all three images are unchanged.
The patch does not close either remaining OPEN function or bank new results.
