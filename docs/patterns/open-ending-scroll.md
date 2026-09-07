# OPEN ending-scroll controller and data owners

## Dispatch/value-allocation controls at 106/108

The two-condition `if` spelling preserves retail's five long-lived scene-state
registers and emits 49 CFG blocks, but lays each phase body directly after its
test and is eight bytes shorter than retail. Explicit forward labels without a
next-phase value compile byte-identically to the canonical 51-block switch,
so label syntax alone does not recover the shared phase store. A per-frame
typed snapshot of the phase is likewise byte-identical to the canonical form.

Combining explicit retail-shaped dispatch labels with an unsigned-halfword
next-phase join emits the same 49-block, four-byte-short carrier-family object
already seen with the typed and full-word joins: lighting blend moves to `s8`,
and sequence, scroll tick, entity 27 and background blend rotate through
`s4`-`s7`. The unsigned width does not change that result. All controls are
removed; the canonical switch remains the highest strict match.

## Post-106 lighting-tail controls

With OPEN at 106/108, the canonical candidate still has 51 blocks against
retail's 49. A short-lived next-phase value again recovers the complete retail
successor graph and branch count, but rotates the five persistent scene-state
registers and shortens the body by four bytes. Moving its declaration after
all persistent locals does not change the object. Narrowing it to `u8` adds an
unsupported mask before the halfword store and remains worse. Both are removed.

Spelling each phase as `if (blend > 4096) transition; else interpolate` is
semantically equivalent and introduces no carrier. It reaches 50 blocks, but
duplicates the transition jumps and moves the remainder of the loop farther
from retail. The retained `<=` form remains the best supported source. These
results strengthen the classification as an unattributed cross-branch-tail
residue; they do not justify a register hint, fake local, or compiler change.

The remaining supplied GCC 2.5.7 scheduling control was also run: disabling
the instruction scheduler leaves the scroll controller at 51 blocks and drops
its listing similarity to 74.9%. It additionally regresses four exact sibling
functions in the consolidated unit. Together with the prior plain and R3000
controls, no available scheduling profile recovers this tail; the attributed
R2000 profile is restored.

## Camera-call/local-initialization order (`073ada0`)

OPEN `80014e28` began this pass at 1944 bytes and strict 97.129630%. The six
image-qualified semantic views, complete 49-block retail CFG, sole caller,
adjacent exact controllers, SDK/provider boundaries, data owners and source
history were refreshed before editing. Retail forms the ending camera-path
argument before five local selector/blend initializations, performs the entity
handoff, and only then calls `opening_camera_path_begin`; the probe formerly
formed the argument immediately before that call.

Moving the five ordinary local initializations to immediately after the camera
call is behavior-preserving because none is observable by the callee and none
is read before the loop. Under the pinned GCC 2.5.7 probe, those initializations
schedule upward between argument formation and the entity stores. The camera
path HI16/LO16 pair consequently moves to the exact retail position and the
strict score rises to **97.952675%**. All forty calls and 31 resolved address
pairs still agree, and the four exact functions in `open.opening_scenes` remain
byte-identical. The source order is retained as a supported correction.

The next real structural residue remains the lighting selector: retail has 49
CFG blocks while the candidate has 51 because the first completed phase keeps
an extra store/jump block. Earlier counter, shared-label, `if` dispatch and
case-local-exit trials remain rejected. The remaining register and stack
pointer schedule is not attributed to a compiler mechanism, and the function
is not exact or banked.
## Edge-scoped lighting-phase join control

A next-phase short live only on the two failed interpolation edges, with the
successful paths and completed-state default bypassing one shared store/reset,
recovers retail's 49-block CFG and branch count. Moving that temporary between
loop and function scope is byte-identical. Both forms rotate every long-lived
scene-state register and score below the canonical source, reproducing the
earlier carrier-family residue. They are removed; narrower C scope does not
explain the retail join under this probe.

## Function Match Plan: scene-3 CLUT work extent

OPEN `opening_scene3_run` at `80014804` was 816 bytes and strict
99.931370%. Its retail and candidate streams had the same 19-block CFG, 27
direct calls, 19 ordered address pairs, constants, delay slots and body words;
all fourteen unequal words were the 112-byte retail frame versus the 96-byte
candidate frame and their saved-register offsets. The six image-qualified
semantic views, callers, callees, adjacent functions, resources, data owners,
relocations and source history were refreshed before editing. There are no
string references. The exact scene 0, scene 1 and ending-scene functions in
the same unit serve as compiler-profile controls.

Retail places the two texture-page values at sp+40/sp+44 and the two populated
CLUT values at sp+48/sp+52, then saves s0 at sp+88. Expanding both arrays to
four words recovers the 112-byte frame but incorrectly moves all three CLUT
accesses eight bytes higher. Keeping the texture-page extent at two words and
recovering the existing CLUT work array as six words preserves every accessed
offset while accounting for the otherwise unused sixteen-byte tail before the
saved registers. The scene initializes and consumes only the first two CLUT
slots, but the compiler reserves the complete source object.

The named six-slot extent emits an identical retail listing for
`opening_scene3_run`; all four exact sibling listings remain identical. This
is a recovered stack-object extent, not a separate padding local. Strict
objdiff confirms 100.000000%, and the full build raises OPEN to 101/108 exact
functions without a code-match regression. The result is banked; the build
still stops only at the repository's existing section-placement, data-owner
and relocation-addend gates.

## Retail GCC 2.4.1 availability control

The two frame-only residues prompted a TU-wide compiler discriminator after
refreshing both functions' six OPEN semantic views. The pinned toolchain does
contain Sony's GCC 2.4.1 `CPPPSX` and `CC1PSX`, but they are legacy Intel COFF
executables rather than native probes. They are not executable directly, and
Wine cannot launch them even from executable `.exe` copies. The experiment
therefore never reached preprocessing or compilation and says nothing about
the 56/48-byte transition or 112/96-byte scene-3 frames. All temporary
compiler-registry, manifest, profile, and TU-assignment wiring is removed.
The available native GCC 2.5.7 result remains authoritative.

## R3000 and combined-live-range controls

Compiling the complete opening-scenes unit with GCC 2.5.7 `-mcpu=r3000` is
byte-identical to the configured r2000 probe. The exact scene 0/1 and ending
scene functions stay exact; `opening_entity_transition` and
`opening_scene3_run` retain their eight- and sixteen-byte frame deficits, and
the ending scroll retains its 51-block residue. The temporary profile is
removed.

The independently observed ending-scroll facts were also tested together: a
typed camera-path pointer initialized after the second entity lookup, plus an
`s16` next-lighting-phase value joining both completed interpolation cases.
The result has retail's 49-block CFG, but the camera address is still delayed,
every long-lived scene-state register rotates, and the body is four bytes
shorter. This is the same substantive divergence as the earlier carrier
control, not a complementary live-range fix. Both locals are removed.

## Complete camera-array and signed carrier controls (`82320b9` follow-up)

A pointer to the complete three-element ending camera-path array, assigned at
retail's early address-materialization point and dereferenced at the unchanged
call, is folded to the canonical late argument setup. It does not alter one
instruction. A signed-halfword next-lighting-phase carrier does recover the
retail 49-block CFG and shared phase-store/reset tail, but rotates all long-lived
state registers and reduces the body by four bytes, just like the wider carrier
family. Both are reverted. Owner extent and selector signedness alone do not
explain the early address schedule or the shared join.

## Opening-scenes scheduler control (`b46dd11` follow-up)

The consolidated `open.opening_scenes` unit was rebuilt under
`probe-gcc257-o2-plain` as a TU-wide control. It does not recover the retail
lighting join in `opening_ending_scroll_run`: the probe still emits 51 blocks
against retail's 49 and retains the broad instruction-order differences.
It also introduces new scheduling differences in the otherwise frame-only
`opening_entity_transition` and `opening_scene3_run`, and regresses exact
`opening_ending_scene_run`. The pinned `probe-gcc257-o2-g0` profile is restored.

This rejects a plain-versus-`g0` scheduler explanation for the scroll residue
without attributing a historical compiler or optimizer mechanism. The exact
sibling is the negative control; no source, profile, baseline, or bank change
is retained.

### Full-word phase-carrier and camera-pointer controls

An ordinary shared `s32` next-phase value recovers retail's 49-block CFG and
single literal phase store. Function and loop block scope compile identically,
however: the extra pseudo rotates every long-lived lighting, background,
sequence, scroll and entity register, and the body becomes four bytes shorter.
Changing the carrier to `u16` is also identical. The structurally improved but
globally divergent form is reverted rather than kept for its block count.

Retail materializes the ending camera-path address after the second entity
lookup and before the initial state stores. An explicit typed local assigned at
that point is optimized back to the existing late call-argument setup, so it
does not recover the schedule or any raw word. No register hint, volatile use,
or artificial access is introduced to force the address lifetime.

## Function Match Plan: interpolation-path case exits (`4647acc`)

OPEN `80014e28` remains 1944 bytes, strict 97.129630%, with 167 unequal
aligned words. Retail hashes, six semantic views, complete CFG/detached
epilogue, sole dispatcher caller, adjacent ending/fade controllers, shared
camera/entity/matrix interfaces, resources, SDK signatures/providers and
source history were refreshed. The 264-byte frame, forty calls, eight retail
internal jumps, 31 validated address pairs, no strings/candidates, short
selectors/blends, nine panel descriptors and numeric targets are unchanged.
The game-owned state machine remains separate from its SDK services.

The first divergence is still camera-argument setup at +0x298. The concrete
CFG discrepancy is the extra phase-1 store/jump block: retail's successful
interpolation paths both jump directly out of the lighting switch, while
their completed paths share the literal phase store and blend reset. Test
case-local early `break` after each successful interpolation, leaving each
literal phase assignment/reset as the remaining case body. Preserve dispatch
order, both signed tests, calls, 64/3 increments and 1/2 phase constants.
Unlike the earlier increment/common-reset-label/if-dispatch trials, this
retains the switch and introduces no shared carrier, label, incremented phase
or reversed condition. Compare the full raw stream, not the subgraph alone.
No scene-3 frame padding is justified by its independently refreshed evidence.

The early case exits emit the identical 1944-byte candidate and ordered
relocations, including all 167 unequal aligned words. Restore the original
conditional bodies; the exact 532-byte dispatcher and scene-3's fourteen
frame-only differences are unchanged.

### Lighting-selector signedness

The refreshed retail dispatch loads the phase with `lhu` at `800151b8`,
stores it with `sh`, and tests only equality with 0/1; it assigns only 0/1/2.
Unlike the blend and scrolling flag, this selector has no signed comparison
or arithmetic consumer. Its halfword width is established, but signedness
is not. With the original control flow restored, test `u16 lighting_phase`
as an unsigned state selector, leaving all other state/blend types unchanged.
This tests the decoded load/compare contract, not a temporary introduced to
force literal values into a register. Compare dispatch, phase-store join,
stack accesses and the entire body; reject additional narrowing instructions.

The unsigned selector removes retail's post-load value copy and still leaves
two separate phase-store paths. It emits 1940 bytes with 262 unequal aligned
words, while all forty calls and 31 ordered numeric targets remain unchanged.
Restore the signed-short model: the unsigned hypothesis does not explain the
observed dispatch/store sequence. Both trials are removed; no new exact
function or source change is retained from this follow-up.

The restored source's real rebuild and strict match confirm 97.129630%.
Full `kf build` preserves 97/108 OPEN exact functions and all 13 SDK source
controls, with only the existing OPEN switch-table mismatch and GAME data/
four historical-best failures. Ruff, all 401 tests and `git diff --check`
pass. Only these evidence notes change; source, types, profiles, data owners,
baseline rows and GAME remain untouched.

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

## Ending-data linkage control

The focused listing renders references to the ten ending-scroll-only initialized
objects as `.data+offset` in the candidate and as curated object identities in
the delinked target. Removing internal linkage from the camera path, panel and
background rectangles, four gradient colors, panel/background colors, and UV
bytes makes the relocations retain those names and raises listing similarity
from 82.8% to 90.0%. Strict resolved objdiff remains exactly **97.129630%**:
all physical targets and code bytes are unchanged. Restore `static`, which
matches their supported single-TU ownership. Symbol presentation is not the
remaining camera-setup, phase-join, or register-lifetime source fact.
