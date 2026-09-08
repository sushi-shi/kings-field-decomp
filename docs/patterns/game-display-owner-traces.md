# GAME display-owner traces

## Function Match Plan

Continue from `494484d` with hash-verified GAME.EXE and the unchanged pinned
GCC 2.5.7 profile. Refresh the six semantic views for display initialization,
frame setup and presentation; read all five caller windows, adjacent boundaries,
current source/history, complete SDK object declarations and prior owner controls.
The first two frame helpers are exact controls for the initializer's shared state.

| GAME function | Retail bytes; strict baseline | Preserved evidence |
| --- | --- | --- |
| `8001bb94 display_initialize` | 332; 94.253010% | No arguments; 32-byte frame, thirteen direct calls and fourteen validated address pairs. Four five-argument SDK setup calls use 320x240 and opposite framebuffer halves. Retail retains first DRAWENV.dtd through PutDispEnv at +162 and the fog word store at +18538. |
| `8001bfb8 display_begin_frame` | 152; 100% | No arguments; 24-byte frame, one call and seven validated pairs. Preserve byte toggle/reloads, 12-byte primitive-buffer and 64 KiB ordering-table strides, 16384-entry clear, cursor publication and three word resets. |
| `8001c050 display_present_frame` | 152; 100% | No arguments; 24-byte frame, five calls and four validated pairs. Preserve synchronization, repeated byte-index reads, SDK strides 92/20 and the ordering-table last-entry address. |

All three have one straight-line block, one return with frame restoration in
its delay slot, no strings, candidate references or unresolved transfers. Their
startup/frame policy is game code. The supplied Release 2.5 LIBGPU EXT/SYS and
LIBGTE GEO/REG services retain their existing vendor evidence and real SDK types.
SetFogNear takes two signed words, matching the stored signed fog distance.

The existing complete-owner fixture preserves both exact frame helpers but
leaves initialization at 336 bytes, first diverging at +a4 in the dtd base
register. Its registry/projection/morph byte spans remain explicitly unresolved.
Do not replace them with inferred capacities or migrate production storage here.

Capture current native/traced canonical and owner objects. Test one real data-flow
axis: after publishing fog distance 11000, pass that stored field to SetFogNear
instead of repeating the constant. No intervening call can change it; the API
receives the same value. Test this in both existing declarations, preserving all
other expressions and source order. Inspect CSE and allocation at the first
changed instruction; reject an unchanged or worse result as an explanation of
retail's retained member address. Compare complete linked bodies and ordered
targets, including every unchanged unit sibling and a wrong-root control.

Only strict objdiff 100% closes a function. A temporary raw result or trace
explanation does not bank this provisional owner. Any retained production edit
requires fresh focused matching, full build and repository verification.

## Verdict

The stored-field argument produces identical complete function words, calls and
address targets in both declarations. All target code/data sections also remain
identical; source-directory metadata differs. Native and instrumented compilation
of each individual source produces identical whole ELFs. No production edit is
retained from this trial.

In the complete-owner trace, first CSE derives both the DISPENV argument at
DTD+162 and the fog store at DTD+18538 from pseudo 75. Second CSE substitutes
the fog destination back to the absolute owner+149672 expression. The DTD
pseudo consequently has three references, crosses zero calls, has live length
22 and receives a0. The canonical separate-owner form has no remaining
allocation references to that pseudo. In both forms, pseudo 71 retains height
240 in s0 across the three setup calls; fog value 11000 is in v0.

The trial creates pseudo 91 for the additional fog-field address at expansion.
First CSE folds its load back to constant 11000, and it has zero allocation
references. Both DTD and height lifetimes are unchanged. This records an
observed probe decision, not historical compiler attribution or a source remedy.

| Function | Final verdict |
| --- | --- |
| `display_initialize` | Canonical strict 94.253010%, 340 bytes; owner diagnostic 336 bytes versus retail 332. First difference remains +a4: canonical uses at and owner uses a0, where retail uses s0. Stored-field argument changes neither body. Still open. |
| `display_begin_frame` | All 152 retail bytes remain exact with either declaration and argument form. |
| `display_present_frame` | All 152 retail bytes remain exact with either declaration and argument form. |

All eighteen functions in each compiled TU are checked: the other seventeen
functions reproduce every retail word. The delinked target independently
reproduces retail, all ordered direct calls agree, and shifting the candidate
owner by four changes each selected body while preserving its calls. The owner
initializer's additional absolute fog address remains the documented fifteenth
materialization; canonical source has sixteen, versus retail's fourteen.

Generated dossiers, compiler traces, complete raw comparisons and allocation
snapshots live under `build/gcc257/display-owner/`. The current source/config,
bank rows and compiler profile are unchanged, so the verified `494484d` full
build and 703-test checkpoint still apply. This refresh does not add three
previously unreviewed consumers to the older owner audit. GAME remains 324/362
and OPEN 106/108 exact; all forty remaining functions still require strict 100%.

## Post-adoption typed-owner control

After the complete owner was adopted, a fresh six-view retail audit and
focused compile confirm `display_initialize` at 98.421684%. The remaining
raw difference starts at +a4: the dtd anchor uses a0 rather than s0, and
fog uses an additional absolute address pair. The existing 13 calls and
post-setup store order remain the controls.

Function Match Plan: compare direct owner members with a typed
`KfGraphicsRuntimeGame *` scoped from the dtd assignments through SetFogNear.
Use only the established complete owner and real members; preserve SDK types,
constants and order. The two-state JSON result is 98.421684% for direct
members and 79.421684% for the typed pointer. Raw inspection shows the pointer
retains owner+0x20000 in s0, combines the byte stores under that base, forms
PutDispEnv separately and expands the frame to 40 bytes. Retail retains
owner+0x2003e and independent byte-address pairs. Reject the pointer trial;
no canonical source or owner change is retained.

Results: `build/hypotheses/20260908-182634-game-render-display_initialize`.
Raw listings: `build/display-owner-objects`. This is a negative source-lifetime
control, not a compiler-mechanism attribution.

## Byte-assignment expression controls

Function Match Plan: with the same six-view GAME 8001bb94/332-byte evidence
and adopted complete owner, cross three two-state assignment axes in JSON:
chained versus separate dtd stores, separate versus chained isbg stores, and
separate versus chained RGB stores. Preserve the observed byte-store order
(dtd 1 then 0, isbg 0 then 1, RGB r/g/b for each environment), real SDK field
types, thirteen calls, constants and referents. These are assignment-expression
controls; no new pointer, owner or padding is introduced.

All eight states compile. The four chained-dtd states remain 98.421684%;
the four separate-dtd states score 93.518074%. Independent recompilation and
complete raw listings, including relocation annotations, prove each group
byte-identical internally: neither isbg nor RGB chaining affects instructions.
Splitting dtd changes the first post-setup instruction at +0x98, materializing
owner+0x2009a in a0 before loading 1. The first environment dtd then uses an
absolute store, and PutDispEnv derives its argument with +70 instead of +162.
The extra absolute fog-store address remains. Retail instead loads 1 first
and retains owner+0x2003e in s0. Reject the separate-dtd form; retain canonical
source unchanged and leave the function open at 98.421684%.

Results: `build/hypotheses/20260908-183009-game-render-display_initialize`.
Manifest: `build/display-assign-hypotheses.json`; independently compiled raw
listings: `build/display-assign-objects`. This batch yields no exact result.


## Environment initialization inline boundaries

Function Match Plan at `36859de`: hash-validate retail and refresh all six
GAME views for `8001bb94`/332 bytes. Read the complete 83-word body, startup
caller, adjacent texture/render initializers, source history, adopted owner
layout and prior pointer/assignment controls. Baseline is 336 bytes and
98.421684%, first differing at +a4: a0 rather than s0 holds the first dtd
address. Preserve the 32-byte frame, thirteen proven calls, fourteen retail
address pairs, one return and all delay slots. There are no branches, strings
or unresolved transfers. SDK services retain LIBGPU/LIBGTE archive/header
attribution; their implementations are not campaign source.

Test three cohesive inline scopes: dtd/isbg/RGB initialization, those stores
plus PutDispEnv, and the four default-environment calls plus those stores and
activation. For each, use global fields, typed SDK array parameters, or the
existing complete graphics owner. All nine hypotheses preserve store order:
dtd 1 then 0, isbg 0 then 1, and RGB r/g/b for each environment. The baseline
makes ten JSON states. No artificial local or field is introduced.

All states compile and inline with the thirteen original machine calls.
Independent fresh compilation and linked-word resolution give three groups:

| States | Bytes / strict score | Raw result |
| --- | --- | --- |
| Baseline and all three global helpers | 336 / 98.421684% | All 84 words and fifteen address pairs identical. First difference remains +a4. |
| Flags arrays/owner, activation arrays/owner, full setup owner | 340 / 94.253010% | Identical within the group. At +a4 the dtd store becomes absolute; PutDispEnv gains its own pair and a nop delay slot. Sixteen pairs; the fog pair remains. |
| Full setup SDK arrays | 332 / 89.168680% | First difference +0: frame 40, extra s1 save. The first draw-environment base stays in s1 across setup calls; twelve address pairs. Matching extent does not imply matching source. |

Reject every helper. All seventeen sibling functions preserve every retail
word and ordered target. No canonical source, identity or bank change is kept.
The JSON report is
`build/hypotheses/20260908-205726-game-render-display_initialize`;
the dossier and independent objects are `build/display-inline-evidence.txt`
and `build/display-inline-objects`.

A follow-up Function Match Plan returns the initialized graphics owner from
the full environment helper, consuming it in the actual later fog-field
store. Compare a global helper returning the owner with a helper accepting
and returning that owner. Both preserve all dynamic operations and SDK types.
The global-return form grows to 348 bytes / 94.686745%: after SetFarColor it
materializes the full owner and adds 0x20000 before storing at +0x48a8, rather
than reusing the retail dtd anchor. It retains fifteen address pairs and first
difference +a4. The provided-owner form is raw-identical to the earlier
340-byte/sixteen-pair group (94.253010%). All siblings remain retail-exact.
Neither result is retained. The three-state report is
`build/hypotheses/20260908-205844-game-render-display_initialize`, with
independent controls under `build/display-result-inline-objects`.

Both JSON baselines have the canonical source SHA; all thirteen states have
no compile errors. Ruff, all 713 tests (103.393 seconds) and
`git diff --check` pass. Full `kf build` retains GAME 337/362 exact and
99.182% aggregate similarity. Data matches remain GAME 11/41, OPEN 3/19,
PSX 0/1; target relink remains 75/77, 34/38, 1/1. Artifact failures are zero.
Existing incomplete closure checks keep the full build nonzero. Generated
manifests, sources and comparisons remain uncommitted under `build/`.
