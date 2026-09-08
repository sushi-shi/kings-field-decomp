# OPEN comparison constants before allocation

These observations use `probe-gcc257-o2-g0` and the instrumented compiler
source fingerprint
`ea9c07d5f5a67e28a6e3bfd21c5c53607543832a9be6561329591d0433f521fa`.
The source baseline is `695d1ae`. They explain this probe's decisions, not the
historical compiler identity. OPEN remains 106/108 exact; neither remaining
function is closed by these controls.

## Retail evidence and exact control

OPEN `opening_ending_scroll_run`, `80014e28`/1944 bytes, copies its short
lighting selector into `v0` and compares it with one in `t0`. The two phase
completion paths also use `t0`, enabling the shared phase-store/reset tail.
The best isolated top-color-pointer source has seven unequal linked words:
the two dispatches' comparison registers and the default edge's reuse of one.
Its frame, calls, referents and all other words agree; it is still non-exact.

A scan of all 665 curated OPEN function bodies found one other game-owned
function with `li t0..t7,1` immediately after a zero comparison branch:
`render_map_cell`, `80018bbc`/464 bytes. Its unchanged source and its 320-byte
traversal sibling were rebuilt with native and traced GCC. Both remain strict
100%, with complete resolved retail-word equality and whole-object parity.
The other scan hit outside the ending was the vendored
`SpuSetReverbModeParam`; it was not counted as game progress.

The exact map-cell source compares both visibility and orientation against
one, without an intervening call. Expansion creates separate SI constants
at UIDs 82 and 169. CSE1 substitutes the first pseudo, p99, into the second
comparison and removes the second definition. P99 spans blocks, receives no
local allocation, and gets `t1` during global allocation. Its trace records
three references, zero crossed calls and a live length of 66.

## The ending's local/global distinction

In the top-color-pointer ending trial, comparison ones p156/p166 receive
`v0` in local allocation. Selectors p154/p164 remain unallocated at that
stage and receive `v1` globally. This is not a local quantity-priority tie.

`flow.c` marks a pseudo global when its occurrences span different basic
blocks. `local-alloc.c` admits a pseudo only when it belongs to one block,
dies once and meets the register-class condition. The selector occurs on
both sides of the first conditional branch; each one constant is confined
to a later block. Local allocation runs first. Declaration reordering cannot
by itself change this block-membership distinction.

The exact map-cell control demonstrates CSE sharing under the same compiler.
It does not establish that the ending's constant was shared by CSE, rather
than introduced through another source lifetime. The ending's common case
exit interrupts the corresponding CSE path.

## Function Match Plan: scroll-enable equality

The ending's six-view dossier, sole caller, complete CFG, adjacent controls,
SDK providers, source history and owners were reviewed. It has forty proven
calls, 31 validated address pairs, eight internal jumps, no strings or
candidate/indirect outgoing references, and six exact siblings. Canonical
strict similarity is 97.952675%, with the correct 264-byte frame.

The signed-short scroll-enable local has exactly two producers: initialization
to zero and the entity-handoff assignment to one. Its address does not escape.
Retail later loads it from sp+184, copies the promoted value and uses `blez`.
Test its active predicate as `scroll_state == ENDING_SCROLL_ACTIVE` instead
of `scroll_state > ENDING_SCROLL_WAIT_DISK`, preserving all producers and the
stored width. These predicates agree over every produced state. The signed
retail branch remains a required byte-level check, not a relaxed criterion.

The MIPS backend materializes a nonzero equality operand as an SI pseudo.
Loop optimization precedes instruction combination, so this third real
comparison tests whether the additional one enters the invariant pool before
any later range simplification. No constant carrier, new state, artificial
addressability, compiler option or production-source change is introduced.

## Verdict

The trial reaches 97.921814%, 1940 bytes. Native and traced complete objects
agree; all forty calls, 31 ordered referents and six raw-exact siblings remain.
The first raw divergence is +2b4, with 236 unequal aligned words.

The predicted invariant decision does change: the three ones combine into a
pool with life 3/savings 3 and move outside the loop. Moving that pool reduces
the movement threshold; the three minus-ones are consequently not hoisted.
However, the active predicate survives combination as a comparison between
the short enable state and the pooled one. Its selector copy disappears and
its branch differs from retail. Reload now needs two simultaneous registers
at UID 888 for the spilled flag and rematerialized one. The lighting tails
still use different spill registers and remain unmerged.

Retain this candidate as an exploratory base. Invariant movement alone is
insufficient for banking, but its new reload requirement must be investigated
separately before judging whether the useful constant sharing can survive a
further source correction. The earlier blanket rejection and prohibition on
composition were premature. Retail's signed branch remains a required final
check. Generated
native objects, complete traces, compact snapshots and raw audits are under
`build/gcc257/open-one-sharing-control` and
`build/gcc257/open-scroll-active-equality`; they are not committed inputs.

## Continuing on top of the shared constant

The user's correction was borne out: the new reload requirement can be
removed while retaining the hoisted comparison-one pool. This is a verified
compiler-state improvement, not an exact function match.

The original equality candidate's state p76 has seven weighted references,
19 crossed calls and live length 249. The pooled one p155 has seven weighted
references, 19 crossed calls and live length 498. Both fail the global
register search: call-clobbered registers cannot hold these lifetimes, and
the remaining integer registers conflict with allocated values. Reload
substitutes the constant equivalence for p155. UID888 consequently needs
both the spilled halfword state and the literal one at the same instruction.

Four isolated controls were compiled after recording their match plans:

| Change on the retained equality base | Shared one retained | Reload registers | Lighting tails shared | Strict objdiff | Bytes |
| --- | --- | --- | --- | --- | --- |
| Explicit unsigned-halfword conversion at the active gate | Yes | 2 | No | 97.921814% | 1940 |
| Frame-entry pointer for the actual top-color arguments | No | 1 | Yes | 99.660490% | 1944 |
| Advance the guarded scroll state from zero to one | Yes | 1 | Yes | 95.921814% | 1936 |
| Same advancement, exchange two independent zero initializations | Yes | 1 | Yes | 98.251030% | 1960 |

The conversion control produces every resolved word of the original equality
candidate. The pointer composition moves its real pointer first, lowering the
loop movement threshold from 26 to 23. The three-one pool then fails its
movement test, as predicted. Its single reload register does not demonstrate
preserved sharing; the final ten unequal words include the earlier dispatch
differences and the changed active predicate.

The advancement control preserves the actual guarded transition:
`scroll_state = KF_ENUM_DECODE(KfEndingScrollState,
KF_ENUM_ENCODE(s16, scroll_state) + 1)`. The guard admits only zero. CSE does
not fold this short arithmetic using its knowledge of the distinct promoted
guard expression. Loop optimization still moves the three-one pool, now at
UID1082, with 231 real instructions and the same life3/savings3. Combine
retains the gate's signed promotion at UIDs886/887 rather than absorbing the
short state into the equality branch. The promoted selector gets `v0`; the
constant reload uses `t0`. There is no two-register reload requirement, both
lighting completion constants use `t0`, and jump optimization shares the tail.

This advancement also raises the state's weighted references from seven to
nine. It wins `s8` over the background blend: both have nine references, but
their live lengths are 253 and 255. `global.c:allocno_compare` computes
priorities 1067 and 1058 respectively. This is a separate allocation effect.

Exchanging only their two independent zero initializations reverses those
live lengths and priorities. The background blend returns to `s8`, and the
scroll state is spilled again. Crucially, the pooled one and the single reload
register both survive. The state reload feeds the promoted selector before
the branch; the constant reload can reuse `t0` afterwards. This second control
separates the reload fix from permanently allocating the state to a register.

The last candidate still has 164 unequal aligned words, starting at +2a4,
and is sixteen bytes longer than retail. Its signed extensions, state update,
initialization order and equality gate still need byte-level correction.
Keep both the original sharing candidate and the successful reload compositions
as exploratory bases. Their lower scores do not undo the observed mechanisms,
and none of these objects qualifies for banking.

All four controls preserve the forty calls, 31 ordered referents, 264-byte
frame with matching save offsets, and six raw-exact siblings. Native and
traced complete objects agree. The advancement and initialization controls
also agree byte-for-byte when rebuilt without the observational loop/global
dumps. The production source hash, profile, configuration and baselines are
unchanged; OPEN remains 106/108 exact. No full production rebuild was needed
for these ignored-only source controls and documentation changes.

Generated plans, source candidates, complete traces and verification are under
`build/gcc257/open-shared-one-{reload,top,advance,init-order}`. The reproducible
`open-shared-one-reload/audit.py` checks source hashes, complete native/traced
parity, every call and referent, exact siblings, sharing, allocations and reload
requirements; `summary.json` and each `reload-verification.txt` retain results.

## Gate widening and direct range observations

Continuing from the retained initialization-order composition produced the
following controls. Each had a plan before its source edit and preserved the
shared one, single `t0` reload register, merged lighting tails, background in
`s8`, spilled short scroll state and matching 264-byte frame.

| Further change | Strict objdiff | Bytes | Observed result |
| --- | --- | --- | --- |
| Unsigned widening at the handoff's zero guard | 98.251030% | 1960 | Every resolved word equals the preceding composition |
| Set the enable bit at the guarded activation | 98.364200% | 1960 | OR replaces addition; the extra signed shifts remain |
| Unsigned widening at the later ACTIVE gate | 98.446500% | 1948 | Reload folds widening into a direct halfword load |
| Initialize background after `SetFogNear` | 98.436210% | 1948 | Retains its register but does not recover retail initialization order |

The third control is distinct from the earlier unsigned-gate trial: its base
has the retained increment, whose range GCC does not infer through the loop.
Expansion and combination keep `ZERO_EXTEND(p76:HI)` at UID886. During reload,
that operation accepts the stack operand directly and emits `lhu v0,184(sp)`.
It replaces the previous state reload, load-delay gap and signed shifts,
removing twelve bytes while the equality comparison still uses the pooled one.
Native, traced and debugger-driven default-profile objects agree completely.

The compiler now emits every retail word from function offsets +390 through
+4e8, including the lighting and sequence dispatches. Before +4ec, only the
two exchanged zero initializations at +2a4/+2ac differ. This is a verified
region of a non-exact function, not an additional exact-function count.
The handoff still shifts the short for its zero test and increments/reloads
the stored state instead of storing literal one. The later gate uses the
direct load and equality branch instead of retail's load/copy/`blez`.
These changes shift later instruction addresses; 164 aligned words differ,
and the candidate remains four bytes longer than retail.

GDB observations of the unchanged debug compiler establish the range facts
directly. Immediately after combine's initial analysis, the direct-assignment
state has `reg_nonzero_bits[p76] = 0x1`, with fifteen sign-bit copies. Both the
increment and bit-setting recurrences produce `0xffffffff`, with one sign-bit
copy. `combine.c` explicitly disables use of the global range arrays while
computing them in a single forward scan. The source-level proof that these
guarded recurrences only produce zero and one is therefore stronger than the
compiler's recorded result. OR does not recover the desired literal store or
promotions in this compiler; preserve the increment composition separately.

A small independent control uses a short flag, an external poll, the same
zero-guarded activation, an external frame call and a signed positive gate.
Literal assignment, increment and OR have identical reachable flag values.
Their observed range masks are respectively `0x1`, `0xffffffff` and
`0xffffffff`, matching the real ending. All three controls produce identical
complete objects with native, traced and debugger-driven compilers.

At the ending's active comparison, direct function-entry observations show
the second operand is still register p155, not `CONST_INT(1)`. The logical
predecessor links reach the local promotion but not the hoisted definition.
`combine.c` says those links do not cross basic blocks, and its constant-based
comparison simplifications require a literal operand. This explains why a
range fact alone does not transform the pooled-one equality into the required
retail branch. The zero in p155's global range-array entry is not a claim
that its value is zero: that initial analysis only populates entries for
registers assigned more than once.

All four new real-source controls retain forty ordered calls, 31 ordered
referents and six raw-exact siblings, with complete native/traced parity.
The authoritative state refresh still reports OPEN106/108, GAME329/362 and
PSX1/1, with no state failures. Production source/configuration/baselines and
the unrelated dirty documents remain unchanged; nothing was banked.

Generated evidence is in `open-shared-one-handoff-conversion`,
`open-shared-one-activate-bit`, `open-shared-one-gate-widen` and
`open-shared-one-background-init` under `build/gcc257/`. The reproducible
debugger wrapper, actual-function observations and small controls are in
`open-shared-one-range-trace/`; its `audit.py` and the expanded
`open-shared-one-reload/audit.py` pass all recorded assertions. An initial
line breakpoint did not observe the gate; the verified captures use the
comparison routine's entry and record its actual operands.

## Computing the next phase before its dispatch

A further controlled source question starts from the retained top-color
pointer candidate. Its seven unequal words remain confined to the dispatch
registers and one jump. The actual lighting transitions are 0 to 1 and 1 to 2.
Computing their promoted next value once before the switch, then using it on
the two existing failed interpolation edges, distinguishes calculation before
path facts from the earlier calculation inside each selected case. The two
blend resets remain separate. This introduces no new guard, call or unused
value, and signed-halfword promotion plus one cannot overflow the word result.

The result is non-exact: 99.135800%, 1952 bytes versus retail 1944, with the
matching 264-byte frame and all saved-register homes. It preserves the single
`t0` reload register and shared phase-store/reset tail. The first divergence
is +398: the short recurrence now needs signed extension, and an addition
computes the future phase in `a0`. The shared store consumes that value
instead of retail's literal 1/2 producers in `t0`. There are 249 unequal
aligned words and two extra words. The earlier 99.917694% pointer candidate
and the independently retained shared-one compositions remain intact.

Debugger observations at the two stores establish the CSE distinction in
both passes. Each selected path gives the current promoted phase a known
quantity of 0 or 1. The already computed next value's quantity has no constant;
the later path fact does not propagate backward through its earlier addition.
In the earlier inside-case control, CSE1 instead folds the assignments to
literal 1 and 2, and CSE2 receives those literals. Captures also retain rescans
where the current value is unknown; those are distinct from the selected-path
observations.

Two small functions with a short phase input and calls consuming the next
value reproduce the same behavior. Calculation before the switch leaves
register call arguments after both CSE passes. Calculation inside each case
gives literal arguments after CSE1. The two real sources and both small
controls have complete native/traced/debugger object parity. The actual
ending retains forty ordered calls, 31 ordered referents and six raw-exact
siblings. Production source, profile, configuration and baselines are
unchanged; no function was banked.

Generated plans, candidates, debugger captures, small controls and the passing
`audit.py` are under `build/gcc257/open-next-phase-before-dispatch/`.
This result explains why the earlier next-value calculation survives; its
extra arithmetic does not satisfy the retail instruction contract.
