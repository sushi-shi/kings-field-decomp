# OPEN ending-scroll controller and data owners

## Function Match Plan: initial entity-position assignment (`a229537`)

OPEN `80014e28`/1944 bytes starts at strict 97.129630% under
`probe-gcc257-o2-g0`. The six semantic queries, complete retail
CFG including the detached epilogue, full sole caller, adjacent scene/fade,
entity-finder and camera-start interfaces, resource loaders, source history,
data owners and current focused diff were inspected. The contract remains
40 direct calls, eight validated internal jumps, 31 validated address pairs,
no strings/candidate references, a 264-byte frame and ten-register epilogue.
The SDK header declarations and PRIM/REG/GEO/SCSMVOL provider evidence still
exclude library ownership of this game-specific controller.

At setup, retail reloads entity 26 from sp+168 twice: before the position-Y
read at `800150ec`, then again before its store at `80015104`. The current
compound `-=` assignment retains one pointer across both accesses and omits
the second reload. Test an ordinary explicit `entity->position.vy =
entity->position.vy - 1500` assignment, which expresses separate source and
destination evaluations without adding a pointer carrier, volatile access,
padding, call or semantic operation. Keep all other updates, state-machine
branches, objects, types and compiler options fixed. Inspect the earliest
camera-argument ordering difference as well as the two pointer evaluations;
reject this spelling if it does not explain the decoded reload.

The explicit assignment emits exactly the same candidate as `-=`: 1944
bytes, 167 unequal aligned words, the same forty numeric calls and 31 ordered
data targets. It does not recover the omitted reload or earlier camera-path
argument setup, so it is reverted.

All five adjacent state initializations assign zero to actual signed-short
selectors/blends. Retail clears them in the existing source order (lighting,
background, sequence, scrolling, scroll phase), but the camera argument
precedes that group rather than following it. Test a single chained zero
assignment, retaining that right-to-left write order and the same five
objects. This distinguishes a shared initialization value from five separate
assignments without changing declaration order or adding carriers. It is a
bounded source-dependency test, not a permutation of independent states;
retain it only if actual instruction differences are recovered.

The shared zero assignment also emits the original 1944-byte candidate,
including all 167 unequal words, numeric calls and address targets. It is
reverted. Neither setup spelling explains the first camera-argument/order
divergence, omitted pointer reload or the existing lighting-store join.
The original source is retained at strict 97.129630%; no exact or banking
claim is made. Further setup syntax trials need new evidence rather than
another equivalent assignment spelling.

The restored unit was recompiled and the strict result rechecked. Full
`kf build` preserves 95/108 OPEN exact functions and all 13 SDK source
controls, with the same OPEN TMD switch-addend and GAME switch/historical-best
failures. Ruff and `git diff --check` pass. The 401-test run takes 19.969
seconds: 400 pass and the existing 883-versus-882 pointer-cast floor fails.
No source, inventory, test, compiler-profile or bank change is retained.

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

## Lighting-join follow-up plan

Pre-edit snapshot: OPEN `80014e28`, 1944 bytes, `void(void)`, remains
97.129630% strict under `probe-gcc257-o2-g0`. The six semantic queries, sole
caller, adjacent boundaries, source history and SDK declarations were checked
again. The 264-byte frame, detached ten-register epilogue, 40 calls, eight
retail internal jumps and 31 ordered address pairs remain the baseline;
there are no candidate relocations, strings or indirect calls. The game-local
resource/entity policy and external SDK interfaces still exclude SDK ownership.

Retail's signed blend tests at `800151e4` and `8001520c` both compare against
4097. Their failure edges converge at `80015230`: store the next short phase
(1 or 2), then clear the blend at `80015234`. The passing paths call the
existing matrix wrapper with matrix pairs 1/3 and 3/4 and increment the blend
by 64 and 3 in their jump delay slots. The current source emits a separate
phase-1 store/jump before the case-1 block. Its earliest divergence is still
camera-argument setup at `800150c0`, not a relocation error.

The focused hypothesis is a shared phase-advance/reset source path for the
two completed interpolation phases. Test ordinary counter progression first;
do not add a temporary solely to force both constants into one register.
Preserve the call set, types, data owners and all other state-machine behavior.
Compare the first divergence and the transition subgraph after each focused
compile, then retain only a supported, humane source form. A score change does
not establish historical compiler attribution or an exact match.

The counter-progression probe did share the completion block, but changed
the observed literal halfword stores to a saved-register increment and moved
several stack slots (94.545265%). It was rejected on those instruction and
storage facts. A literal-assignment form with an explicit common reset label
still emitted two phase-store blocks and changed saved-register assignment
(96.800415%); it was also rejected. Since the two-test retail dispatch has no
table and does not prove a C `switch`, the final focused comparison uses an
`if`/`else if` dispatch with the original literal assignments and reset bodies.

The `if`/`else if` form inverted the first dispatch edge, moved the phase-1
test after the phase-0 body, and still retained the extra phase-store block
(95.802470%). It was rejected for the newly divergent dispatch topology.
The original source is restored: **97.129630%, no exact-count movement**.
None of these experiments supplies a source correction that explains both
the observed constant-store join and the retail dispatch. This narrows the
remaining residue without attributing it to a compiler mechanism. Further
syntax variations without new evidence are not part of this follow-up.

The added bounded instruction-pattern control verifies both signed blend
tests, matrix arguments, phase constants, interpolation increments and their
delay slots. It follows either the retail shared store/reset or the compiled
extra jump with its store delay slot, and verifies that successful
interpolation skips the reset. Wrong-phase and wrong-reset-register controls
fail as intended. This is not an instruction interpreter, a full behavioral
proof, or a relaxation of the exact-match criterion.

After restoring and really rebuilding the source, all five focused tests,
Ruff, all 383 repository tests (no local skips), full `kf build`, OPEN strict
baseline checking and `git diff --check` pass. OPEN remains **91 exact / 108
started / 108 eligible**; all images retain 353 exact functions, 13 exact
vendor-source controls and 63 exact data-owning units. No C, retail model or
banked baseline change is retained by this follow-up.
Staged `nix flake check -L` also passes; its isolated 383-test run skips the
43 controls that require local artifacts.
