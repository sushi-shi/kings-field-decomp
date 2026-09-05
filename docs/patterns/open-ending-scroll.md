# OPEN ending-scroll controller and data owners

`opening_ending_scroll_run`, OPEN `80014e28`, is complete reconstructed C at
**97.129630% strict objdiff**, not an exact or banked match. The sole caller,
`opening_run` at OPEN `800156bc`, remains **100%** after the identity change.
The per-function pre-edit snapshots and final verdicts are recorded in
`config/evidence/open_semantic_ending_scroll.tsv`.

## Function Match Plan and evidence

Before editing, the campaign inspected all six image-qualified semantic
queries, the sole call at `80015890`, the preceding ending scene and following
fade, source history, resource loaders, data bytes and xrefs, and authentic SDK
headers. The plan was to recover the infinite frame loop, lighting and audio
states, entity handoff, camera path and nine scrolling panels; preserve the
MATRIX/CVECTOR interfaces; correct the complete function extent; then compare
ordered referents, calls, CFG and widths before code-generation symptoms.

This is game-local scene policy, not a Sony provider: it selects object IDs
26/27, loads the ending resources and sequence, and controls the shared camera,
material and display state. GetTPage/GetClut, matrix, GPU and sound APIs remain
external SDK calls. The pinned headers declare GetTPage/GetClut as `u_short`
results with `int` parameters, and SsSetMVol with two `short` parameters.

The old 1896-byte extent stopped at the infinite loop's back-edge delay slot.
The following 48 bytes restore all ten registers saved on entry, from exactly
the same offsets in the 264-byte frame, and finish with `jr ra` and
`addiu sp,sp,264`. They belong to this function, giving **1944 bytes (`0x798`)**.
There is no reachable exit from the loop; the C does not invent one to explain
the emitted unreachable epilogue. This corrects code/data accounting without
adding or removing a function.

## Objects and semantics

The unit owns the 32-byte local MATRIX initializer at `80012000`, one RODATA
claim rather than eight scalar globals. It owns ten initialized data objects:

| OPEN VA | Size | Owner |
| --- | ---: | --- |
| `80035820` | 84 | Three `KfCameraPathPoint` records |
| `80035888` | 72 | Nine `u16[4]` panel rectangles |
| `80037290` | 16 | Two background rectangles |
| `800372a0`–`800372b4` | 6 × 4 | Six CVECTOR colors |
| `800372b8` | 8 | Panel UV coordinates |

The camera sentinel, all panel dimensions and irregular starting Y values,
and the nonzero second UV byte pair are retained literally. Former interior
panel labels at `3588a`, `35892`, and `358ca` are array addends, not overlapping
globals. File-static linkage is a sole-consumer working model, not proof of
historical linkage. Every initialized data byte and the RODATA initializer
match retail.

The initial light matrix is copied to the existing render-state owner. Two
matrices are interpolated in successive phases, with increments 64 and 3.
Entity 26 rises to Y -8000, then becomes inactive while entity 27 activates.
The background blend is clamped at 4096; the active entity rotates around Z
and the floor-item material brightens. Panels move on scroll phases 0/2 until
the last panel's signed Y reaches 50. Visibility is the exact unsigned
halfword test `(u16)(y + 255) < 495`. The sequence transition waits 20 frames,
then decrements volume from 381, passing signed `volume / 3` to both channels.
There are no indirect calls, strings or switch tables.

Retail explicitly stores a stack-owned position VECTOR at sp+24, although the
loop never consumes it. The source retains this observed snapshot, not an
invented padding variable. The nine-entry texture-page and CLUT arrays occupy
the observed stack locations; no fake tenth entry is needed.

## Focused comparisons and remaining differences

The first complete reconstruction scored 96.224280%. Three separately built
width corrections were retained from their raw instruction evidence:

1. Making the scrolling flag signed short reproduced retail's `lhu`, promoted
   copy and `blez`, reaching 96.471190%.
2. Applying the same short promotion model to the spilled lighting selector
   reproduced its copy after `lhu`, reaching 96.615230%.
3. Making the sequence selector short reproduced the register copy preceding
   the case-1/2 tests and recovered the retail blend/sequence saved-register
   assignment, reaching 97.129630%.

These bounded state selectors do not prove a unique original declaration.
The observations use the existing `probe-gcc257-o2-g0` profile, not a newly
attributed historical compiler. No flags, assembly, volatile carriers, fake
locals or dead source statements were introduced to improve the percentage.

The first real divergence is at retail `800150c0` (+`298`): retail materializes
the camera-path argument before clearing several initial states; the probe
does so after those stores. It also uses a different temporary for some spill
reloads and reloads the entity pointer one fewer time during setup. In the
lighting transition the probe has an extra unconditional branch/store block,
where retail converges on a shared phase-store/reset join. Thus byte-level CFG
identity is **not** claimed, although the state transitions agree. The frame,
stack object locations, complete extent and ten-register epilogue agree.
These remain unattributed instruction-order/temporary and branch-join
differences; no optimizer mechanism or compiler wall is asserted.

All 40 direct calls, eight internal jumps and 31 HI16/LO16 pairs are reviewed,
as is the caller relocation. Compiled references to file-static data use
`.data + offset`, while the delinked target has curated datum symbols. The
ordered-reference test resolves those offsets through the compiled symbols
and source DATA owners: all 31 addresses agree, including interior addends.
Nominal symbol spelling in the text diff is not a different data referent.

`tests/test_open_ending_scroll.py` checks the corrected boundary, nonoverlapping
owners, ordered calls and raw relocations, resolved compiled data targets,
signed panel/flag operations, and the full unreachable epilogue, with an
incorrect restore-slot negative control. The campaign adds one started
function: OPEN is **91 exact / 107 started / 108 eligible**. The last unstarted
function is the 3320-byte renderer at `8001764c`.

The full `kf build`, OPEN strict baseline check, Ruff, all 371 repository tests
(no local skips), and `git diff --check` pass. Staged `nix flake check -L` also
passes; its isolated run skips 32 local-artifact controls. All 353 previously
exact functions, 13 vendor-source controls, and 62 data-owning units remain
exact. The all-image strict historical-best check still reports the four
pre-existing GAME deficits documented in `open-format-display-sdk.md`; this
campaign does not edit those functions or their baseline rows. Only the
unchanged exact OPEN caller's baseline is refreshed; the ending scroll is not
banked.
