# GAME map-copy descriptor extent and first-load lifetime

## Function Match Plan

Baseline `11c81c4`, clean `master`; GAME retail freshly hash-validated.
`game.map_object_pool` uses `probe-gcc257-o2-g0` (`-O2 -G0 -mcpu=r2000`,
ASPSX 1.07 compatibility and checked division expansion). This is still a
probe, not historical compiler attribution. All six semantic views, the
complete body, nine argument-constraining call windows, neighboring functions,
data bytes and source history were read before editing.

GAME `80030a98 map_apply_copy_region` owns 484 bytes and scores
94.256195% strictly. Its eight blocks implement the `region_id == 0xff`
exit, zero-height exit, zero-width row skip, two byte countdown loops and one
shared return. The 40-byte leaf frame saves s0..s8; `jr ra` owns the final
stack release. There are no calls, indirect transfers or strings. All eight
address pairs are validated, with no outgoing candidate reference: descriptor
base `800561b0`, collision `80098018`, orientation `80069018`, floor height
`80095900`, attributes `8009a748`, collision flags `800668e8`, then repeated
attribute and collision bases. These are sixteen physical HI16/LO16 entries;
no relocation is changed in this campaign.

The entry masks a0 to u8. Six unsigned descriptor fields occupy offsets 0..5;
the table index is `region_id * 6`, without subtracting one. Width and height
use byte postdecrement tests; promoted coordinates advance without byte
narrowing. All five grids retain their shared `u8[100][100]` declarations and
the source/destination order seen in retail. There is no observable result.

Nine decoded incoming calls constrain this signature. The map-object update
call at `80032720` supplies its action-parameter byte, guarded against 255.
Calls at `800327f8/80032800` supply 2/3; the floor-1 action call at `80034410`
supplies 1; restore calls at `800362d8/800363c0/800363c8` supply 1/2/3.
Crucially, the floor-5 ambient and restore calls at `800343c8/80036438` both
load **4** in their call delay slots (`34040004`). None consumes a result.
The restore function's outer indirect dispatch is not promoted by these
direct-call observations.

The former four-record source has an out-of-bounds access on both index-4
calls. Retail bytes at `800561c8` are `00 00 24 04 07 01`, a fifth descriptor
`{0, 0, 36, 4, 7, 1}`. Extend the table to five six-byte records / 30 bytes.
The next two bytes at `800561ce` are `01 01`, not inferred padding. Preserve
them as unclassified and preserve the independently reviewed camera path
starting at `800561d0`. No sixth record is supported; it would overlap that
camera owner. The calls prove the fifth record, not safety for arbitrary
non-255 indices. Source linkage remains unresolved.

This is game map policy, absent from the vendored/FID inventory, with five
fixed game grids and floor-specific descriptors rather than a library copy
API. Existing SDK memcpy providers are separately attributed at `8004a52c`
and `80052938`. The previous `actor_definitions_load` (`80030a6c`, 44 bytes)
and next `map_object_mark_collision_edge` (`80030c7c`, 572 bytes) retain their
own extents. Original source commit `7c7c843` already had the four-record
declaration and current nested-loop form; later data ownership and module
consolidation did not establish a new table boundary.

First rebuild with only the corrected data extent. Then test a local holding
the actual first attribute byte at the inner-loop entry: retail `80030ba4`
loads it before materializing four destination addresses and stores it at
`80030be0`. The baseline candidate computes the addresses before that load;
its first differing word is at +0xcc (source-x register), followed by changed
source/destination pointer setup and inner-loop registers. Both bodies already
have 121 words. The proposed local is the byte being copied, not a dummy
carrier. Preserve all guards, loop bounds, grid identities, widths and writes.

## Verification plan

Compare each focused build from the first raw divergence, including all
sixteen relocations. Require raw equality for the six existing exact siblings,
all 84 owned switch-table entries and all 30 descriptor bytes. Extend the
existing adjacent-data census control rather than introducing a new harness.
Compare all 484 function scores against the baseline; run the full build,
Ruff, repository tests and diff checks before commit. Bank only a new strict
100% result. No compiler profile, ownership workaround or scoring relaxation
is part of the plan.

## Focused experiments

The data-only rebuild preserves 94.256195% and the original first difference
at +0xcc. Fresh compilation verifies all 30 initialized descriptor bytes,
the 84 relocated switch entries and all six previously exact siblings.

An inner-block `u8 attribute = ...` before the first store does not reproduce
the early retail load. The body grows to 524 bytes with a 56-byte frame;
the first attribute source address is recomputed each iteration while its
destination is held in an advancing pointer. This contradicts retail's
five advancing source pointers and 40-byte frame, so it is not a kept fix.
Next isolate lexical scope by declaring that same real byte with the existing
function locals, leaving the load/store statements unchanged. No width,
condition, store order or source/destination coordinate changes in this trial.

The function-scope byte control emits the same 524-byte body, ruling out
declaration scope as the discriminator. Test the zero-extended loaded value
as a word-sized local next; retail `lbu` produces a full zero-extended register
and `sb` consumes its low byte without an intervening mask. This is a width
control, not proof of the original local's declared type.

The word-local control also emits the same changed body. All explicit
attribute-local trials are rejected and the ordinary assignment is restored.
The retail pointer updates give another independent ordering clue: the five
source pointers advance before the destination collision-flag pointer, which
advances in the inner backedge delay slot (`80030c24`). The old C instead
writes `destination_x++; source_x++;`. Test `source_x++; destination_x++;`
with the original five assignments. These are the real column advances;
neither affects the other's value or any copy. The analogous source-row then
destination-row increments already follow retail at the outer-loop tail.

The source-before-destination column advances reproduce all normalized
instruction and relocation lines of the 484-byte retail body, including the
early attribute load. The original five assignments, all existing types and
both countdown loops remain. There is no explicit attribute local. Retain
this ordinary increment order subject to strict objdiff, raw relink and full
regression checks; do not infer unique historical source syntax from it.

## Final function verdict

Strict native objdiff confirms **94.256195 -> 100%** for
`map_apply_copy_region`. A separate fresh source compile produces the same
sections as the production object. Resolving every reference to the actual
GAME address reproduces all **121 retail words / 484 bytes**, including
the frame, unsigned guards/countdowns, branch targets, complete delay slots,
five load/store pairs and shared epilogue. The sixteen physical relocation
sites/kinds and eight ordered numeric address targets agree. Shifting the
descriptor base by one record is a negative control and fails raw equality.
No candidate relocation or vendored identity was promoted.

All six exact siblings remain raw-word exact. The complete `.data` is
**30 bytes**, with no invented tail padding; all 84 relocated `.rodata`
switch entries still agree with retail. The unit is now **7/8 exact**;
`map_object_probe_forward` remains unchanged and non-exact at 93.755104%.
The missing fifth descriptor was a real source extent error independently
of the text mismatch. Extending it alone did not move the function score;
the final column-advance order supplies the text closure.

All 484 function-score rows were compared with `11c81c4`: only this function
changes. GAME exact count rises **294/362 -> 295/362**; OPEN stays **98/108**,
PSX stays **1/1**, and the thirteen vendored controls stay exact. Overall
game-code exact count is **394/471**, not whole-image closure.

Ruff and `git diff --check` pass; all **649 tests pass** (84.463 seconds).
The existing data-partition control now covers all 348 neighboring bytes,
checks the fifth descriptor and both immediate-index-4 callers, and preserves
the unclassified `01 01` pair. No new general CFG or test framework is added.
The full `kf build` was run after the focused rebuild. Its existing strict
data/ownership/placement failures remain: **7/60** source-data units exact,
**4/4** independent SDK data contributions and **110/116** target units
relinked, with six conflicting-section-base units and zero artifact failures.
These gates are not relaxed to bank the individually verified function.
Only `GAME.EXE:80030a98` is newly banked; no other ledger row is refreshed.

Generated evidence is local under `build/map-copy-*`; the reused fresh-source
raw probe is `build/map-pool-clear-probe.py`. Neither is committed input.
