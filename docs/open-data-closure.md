# OPEN data closure campaign

## Function Match Plan

Baseline: `b9d490c0`. The requested endpoint is all twenty OPEN source data
owners passing the existing strict gate, preserving the 107 exact functions
and the remaining ending-scroll residue. Owner counts and placement checks
are not substitutes for source recovery.

Fresh objects reproduce twelve passing owners. Six failing owners involve
exported BSS requests: opening helpers, camera path, resources, render init,
audio and PAD. Opening scenes has two initialized-data intervals represented
as one section. Format has an unresolved enclosing allocation. Each change
requires an evidence snapshot of its consumers and a final function verdict.
Do not flatten exported allocation requests, invent padding, or force symbols
to retail addresses to obtain closure.

The now-retired composed executable comparison exposed a build defect: its
OPEN link refused the resources module's real `.sdata` contribution. The
reconstructed startup labelled its own empty contribution and assumed it was
first. Its actual contract is the output section base: retail
OPEN `8001aa7c` loads `gp=80037284` and tail-jumps to main. This is vendored
startup work, excluded from game progress.

That experiment replaced the input-order assumption with a native section-base
reference and calibrated the available Psy-Q tools with empty and nonempty
small-data contributions. The resulting controls required the decoded startup
value to equal the MAP section base, including a signed low-half carry case.
They are retained as historical evidence; the composed source-to-EXE path is
no longer available.

The Psy-Q development manual documents SECT as a linked section-base
expression ([chapter 9, page 9-8](https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/sdevtc.pdf#page=112)).
Applicability to the pinned ASPSX/PSYLINK versions must be established by the
native controls, rather than presumed from the later manual.

## Retired composed executable experiment

PSYLINK 1.17 rejects the proposed command-file `SECT` expression. ASPSX 1.07
accepts `.sdata` as an external name but PSYLINK rejects that reference to a
section; this is not a usable section-base expression either. Neither probe
was retained in that experiment.

The experiment's startup anchor was a separate, empty native object included
before all C/library inputs. Its standard section declarations have the same
order as ASPSX's C objects. It adds no instruction, initialized payload or
BSS reservation. The original 24-byte startup family remains after game code
and refers to the exported start label. Original ASPSX/PSYLINK/CPE2X controls
verify empty and nonempty small data, two distinct C contributions, the signed
low-half carry case, native CPE payloads and the unchanged conversion output.
All nine executable tests passed at the time. The mixed linker, its startup
shim and those controls have since been removed.

Those native builds replaced the previously stale executable snapshot:

| Image | Same-offset file bytes | Island-aligned load bytes | Nonzero load bytes |
| --- | --- | --- | --- |
| PSX | 98.39% → 98.39% | 99.66% → 99.66% | 99.06% → 99.06% |
| GAME | 13.68% → 13.61% | 73.73% → 74.22% | 70.65% → 71.08% |
| OPEN | 14.31% → 14.19% | 72.22% → 72.35% | 69.58% → 69.70% |

These comparisons include all linked source changes since the old snapshots;
the modest gains are not attributed solely to the startup correction. The
heuristic island scores do not establish executable equality.

## Formatter allocation boundary evidence

The original LIBCD BIOS object's private `.bss` contribution reserves 64 bytes
for `Result`. Its matched `CD_sync` uses `Result+0x28` and `Result+0x10`.
OPEN retail pairs at `8001b8d0/8001b8d4` construct `800379b0`, and pairs at
`8001b8f8/8001b8fc` construct `80037998`. Both imply `Result=80037988`.
The matched `CD_ready` independently constructs that base at
`8001ba50/8001ba54`. No relocation masks enter these address calculations.

GAME `CD_sync` constructs `800598e0` and `800598c8`, and `CD_ready` constructs
`800598b8`, corroborating the same section-relative references. The private
audio workspace ends at OPEN `80037960` / GAME `80059890`. Thus both overlays
have a forty-byte interval before the first identified LIBCD private BSS
contribution, with the numeric digit anchor 24 bytes into the interval and
sixteen bytes before its upper bound.

This is stronger neighboring-allocation evidence, but not proof that a single
forty-byte formatter array occupied that interval. It does not distinguish
an enclosing buffer from separate preceding storage or prove its original C
array bound. The nineteen-byte accessed-span claim remains explicitly partial;
no speculative padding or larger buffer is committed to make it pass.

## Verification status

Ruff and 776 repository tests pass (nine skips). `nix flake check -L` also
passes, including 776 sandbox tests with 140 local-artifact skips and the
original-linker controls. The full build freshly
recompiles all source units and preserves PSX 1/1, GAME 348/362 and OPEN
107/108 exact game functions, plus thirteen exact vendor source controls.
The full build remains red on the established data, reference-coverage and
placement failures. Source data remains PSX 1/1, GAME 29/41, OPEN 12/20;
the four independent SDK contributions pass. No function is newly banked.
The twenty-owner OPEN goal is still incomplete.

## Exported allocation provenance plan

The native source audit finds thirteen COMMON definitions in eight game
owners. Besides the already failing owners, `memory`, `render_map_cells` and
`opening_entity_pool` currently pass because the analysis adapter flattens
their single exported request into fixed BSS. Native requests are respectively
88, 3264 and 1296 bytes. None is a fixed input-section contribution.

Preserve each compiler-emitted COMMON reservation in a nonallocated analysis
note, independently of DATA claims and C sizeof. Keep allocated bytes, symbols,
relocations and all function scores unchanged. The shared placement checker
must reject these still-unallocated requests even when two flattened objects
agree, until the complete native allocation can be established. Add compiled
positive/negative controls before modifying the checker. A smaller honest
count is preferable to counting unproved storage as complete.

The analysis compiler now records every emitted `.comm` request in
`.note.kf.allocations`, without consulting DATA claims. This nonallocated
note preserves the reservation extent separately from the C object size;
`.lcomm` remains a fixed private contribution. The shared placement checker
rejects these requests as `unplaced-exported-allocation`. Malformed metadata
also fails the gate. This does not yet model their native final placement.

A compiled private/exported pair reproduces the old false pass using identical
flattened source and target objects. The exported case now fails, while the
private case passes. The control checks allocated section contents, flags,
extents, named allocations and ordered relocations against an object compiled
without the note; all remain identical. The menu-bank integration control
still verifies all six layouts and its 37-byte literal contribution, but now
requires the exported-allocation failure. In particular, its 180-byte magic
name array retains the compiler's distinct 184-byte reservation request.

The fresh full build preserves all 484 function sizes and scores, including
OPEN 107/108 exact functions and thirteen exact vendor source controls. The
corrected source-data counts are PSX 1/1, GAME 22/41 and OPEN 9/20. The
three newly failing OPEN owners listed above were false positives, not source
regressions. The full build remains red on data, coverage and placement.
Native executable inputs and the previously refreshed executable scores are
unchanged by this analysis-only correction. No function is newly banked.

Ruff, `git diff --check` and all 778 local tests pass (nine skips).
`nix flake check -L` passes, including 778 sandbox tests with 140 expected
local-artifact skips. This verification supersedes the earlier counts above;
the twenty-owner OPEN objective remains incomplete.
