# GAME experience-loop address lifetimes

## Function Match Plan at `eb1c80f`

GAME `80016058 player_add_experience(s16 amount)` owns 548 retail bytes in
`game.player_death`; the current candidate is 528 bytes and strict 88.824814%.
Hash-identical retail, all six semantic views, the complete body, sole actor
damage call at `8002d434`, both adjacent exact functions, all three callees,
the shared player/growth records, resource copy and source history were
inspected. The actor caller loads the definition award with `lhu` at +132;
entry sign-extension establishes the signed-halfword boundary. The result
is unused. This is game progression policy, not an SDK/library body; the
notification and sound wrappers retain their independent game/SDK boundaries.

Retail has an issue-free eighteen-block graph, nine conditional branches,
three direct calls, one internal jump, one common return and twelve validated
data-address pairs. There are no strings, indirect transfers or candidate
outgoing references. The frame is 32 bytes, saving ra/s1/s0 at 24/20/16;
the final delay slot restores all 32 bytes. The initial experience cap is
signed 99999; the repeated threshold test reloads both word fields. Level is
a byte with a 255 guard; two separate level reads and the previous-level
comparison with forty must remain. The forty twelve-byte growth rows are
one real runtime owner, loaded as 480 bytes. Last/penultimate rows supply
the extrapolated vital and experience deltas; power increments use the last
row directly. All halfword growth stores precede the four cap checks, and
each successful increase calls stat recalculation, notification zero, and
sound-reference row two with volume 127.

The freshly compiled graph has the same known successors. Its first raw
difference is ra at sp+28 and an extra s2 save at +24. It keeps growth-table
+468 in s2 across the loop, replacing retail's eight independent constant
growth-field address pairs. Player anchors at +8 and +16 still agree. The
historical unstructured-loop trial lost those player anchors, so do not
repeat it alone or claim that equal branch counts prove the source shape.

First test an explicit nonempty guard followed by a post-tested `do` loop,
preserving the original level-255 break and every statement inside the loop.
Retail has that entry rejection and bottom reload/back-edge; they do not
uniquely prove a C loop spelling. This is a bounded structural hypothesis,
not permission for volatile accesses, detached growth globals, artificial
locals, compiler flags or declaration permutations. Compare actual addresses
before call/CFG and instruction details. Retain only source supported by the
complete raw result, preserving all twelve exact sibling bodies and their
referents; strict 100% and full verification are required for banking.

The guarded `do` loop emits the same instruction/relocation listing as the
original `while`: it does not explain the growth-address lifetime. Do not
retain that no-effect spelling. The remaining directly observed distinction
is between player subobjects whose addresses survive the three calls and
growth-field references formed anew inside each iteration.

Test explicit typed pointers to the existing progress and vitals subobjects,
initialized once before iteration and used for their actual fields. All other
player and growth accesses remain direct references to their original owners;
no arithmetic outside a subobject or new data alias is permitted. Compare
those views in the original loop and in an explicit shared repeat/exit form.
Refresh the historical label-loop-only negative control solely to separate
its effect from the typed views. This four-way comparison tests two concrete
address/control hypotheses, not declaration-order search. Keep the same
pre-loop threshold guard, repeated end-of-level reload, maximum-level exit,
calculation order and three calls. Reject any newly retained player or growth
address that disagrees with the raw owner/addend sequence.

The typed subobject views do not explain the retail address sharing: the
structured loop still retains a growth base and adds a third player-field
anchor; the label loop restores the independent growth accesses but adds
absolute references for player fields outside those two views. Before
rejecting explicit player ownership altogether, test one pointer to the
complete existing `KfPlayerState`, using it consistently for all player
fields in the label-loop control. This is a parent-object hypothesis, not
permission to reach outside a progress/vitals subobject or introduce shifted
aliases. Keep the growth table separate and unchanged. Judge the complete
address sequence and saved-register/frame words, not the recovered subset.

## Raw results

| Source control | Bytes | Address pairs | Verdict |
| --- | ---: | ---: | --- |
| Original `while` | 528 | 5 | Existing non-exact baseline |
| Guarded `do` | 528 | 5 | Complete baseline words/references unchanged |
| Typed subobjects, `while` | 536 | 5 | 40-byte frame and extra player anchor; rejected |
| Label loop only | 624 | 37 | Player anchors lost, absolute accesses proliferate; rejected |
| Label loop, typed subobjects | 600 | 25 | Independent player-field accesses remain; rejected |
| Label loop, complete player pointer | 532 | 12 | Ordered data targets recovered, but wrong player base/frame; rejected |

The complete-owner pointer restores all twelve ordered numeric addresses,
including the eight extrapolation fields, without altering the three calls.
It instead holds the player's root in s0, uses a 24-byte frame, and loses the
retail +8/+16 anchor lifetimes; field offsets, the normal growth-row index
schedule and entry/exit instructions therefore differ. Do not retain that
pointer merely because the address list agrees. All six independently
compiled variants preserve all twelve sibling bodies and their resolved
references, and each sibling remains raw-exact against retail. The target
experience body has SHA-256
`bc8708cf9656688de00641e8cc7281b1e64f4d85d1282db738fd2ff970280dd5`.

No experience source, owner, type, profile or banking change is kept. The
original strict 88.824814% source remains canonical. These controls narrow
the joint source-loop/address hypothesis without identifying an optimizer
mechanism or proving historical compiler attribution.

## Handoff verification

The subsequent full report comparison retains this function's strict
88.824814% result and all other progression-unit scores. Ruff, all 680
repository tests and `git diff --check` pass. Full `kf build` was run; its
existing data-placement/extent, target-relink and ownership failures remain.
The separate audio-initializer match is the only changed function score in
that 484-row comparison. No experience result is newly banked.

## Maximum-level exit control

A fresh six-view retail audit, complete body/CFG, actor call window, adjacent
bodies, callee source and current comparison reconfirm the same baseline.
Function Match Plan: replace only the level-255 loop break with an immediate
void return. The loop is the function's final statement, so all progression
work, widths, growth references, caps and the three calls remain unchanged.
This tests a source exit boundary without changing pointer views or owners.

Both JSON variants score 88.824814%. Independent recompilation produces
identical complete raw function listings, including relocation annotations.
The extra saved growth-table base and first frame-save difference remain.
No canonical source change is retained. Results are under
`build/hypotheses/20260908-182105-game-player_death-player_add_experience`,
with raw listings in `build/experience-exit-objects`.

## Growth-branch orientation control

The next two-state JSON trial compares the existing `level >= 40`
extrapolation-first branch with an equivalent `level < 40` table-row-first
branch. Both preserve all stores, arithmetic, caps, calls and owner identities.
The baseline remains 88.824814%; table-first scores 61.576640%. Independent
raw inspection shows the same extra s2 save and retained growth-table +468
address, plus the reversed branch layout and an index shift in its delay
slot instead of retail's nop. No source change is retained. Results are in
`build/hypotheses/20260908-182337-game-player_death-player_add_experience`;
raw listings are under `build/experience-growth-objects`.

## Inline growth boundaries

A refreshed six-view pass for GAME `80016058` retains the 548-byte target,
three calls, twelve address pairs, eighteen blocks and nine branches. The
current 528-byte candidate retains one growth-table base across the loop.
Test two inline boundaries without changing operations: extrapolation only,
and the complete extrapolation/table-row selection. A follow-up passes the
existing `KfPlayerState *` into each helper instead of accessing the global
directly. The outer experience loop, byte-level boundary, all caps, field
widths and three calls remain fixed; helper identities are hypotheses only.

| Boundary | Bytes | Strict score | Address pairs |
| --- | ---: | ---: | ---: |
| Canonical local growth | 528 | 88.824814 | 5 |
| Inline extrapolation | 592 | 85.846720 | 22 |
| Inline full growth | 608 | 83.613140 | 28 |
| Inline extrapolation with player argument | 592 | 85.846720 | 22 |
| Inline full growth with player argument | 608 | 83.613140 | 28 |

Independent compilation and numeric relocation resolution confirm the same
three call targets and all twelve exact sibling bodies in every state.
Each typed interface is identical to its direct-global counterpart across
all words and references. The helpers recover the retail frame saves and
initial experience-cap instructions, moving the first raw difference from
`+4` to the changed entry-exit displacement at `+0x58`. They then exchange
the progress/vitals anchor registers, introduce separate player-field address
pairs, and load the magic increment earlier in the extrapolation block.
Thus they recover neither the twelve-pair address sequence nor its field-load
order. No source is retained or banked; the boundaries do not explain retail.

## Inline growth with explicit player subobjects

Function Match Plan at `216ebf5`: refresh all six GAME views for `80016058`,
548 retail bytes / 88.824814%, and read its entire raw body, actor award
caller, adjacent training/damage functions, growth loader, shared types,
notification/audio/stat callees and previous loop/helper controls. The signed
halfword entry, byte level, nine conditional branches, common return, three
calls, twelve data pairs and internal jump retain the preceding evidence.
The semantic navigator counts that internal jump as a thirteenth validated
outgoing row; it is not another data address. The custom progression code
remains separate from SDK providers.

The earlier inline helpers recover the frame but lose player-address sharing;
the earlier typed views were tested in local or label loops. Test their
combination: extrapolation-only and complete-growth inline helpers, each with
an explicit progress view, vitals view, or both. Initialize each view from its
existing real subobject before the loop; pass the vitals view to the helper
when used there. Keep unrelated player fields as complete-owner accesses.
This adds no offset casts, overlapping globals or accesses outside a view.
Preserve all arithmetic, field widths, stores, cap tests and the three calls.

| JSON state | Bytes | Strict % | Data pairs | Frame |
| --- | ---: | ---: | ---: | ---: |
| Canonical | 528 | 88.824814 | 5 | 32 |
| Extrapolation / progress | 600 | 78.284670 | 21 | 32 |
| Extrapolation / vitals | 596 | 82.452550 | 17 | 40 |
| Extrapolation / both | 596 | 81.102190 | 17 | 40 |
| Complete growth / progress | 616 | 75.846720 | 27 | 32 |
| Complete growth / vitals | 604 | 81.686134 | 21 | 40 |
| Complete growth / both | 604 | 81.284670 | 21 | 40 |

Every state compiles and preserves all three ordered call targets. The
progress-only variants first differ at the saved-ra word (+4); vitals/both
variants first differ at frame allocation (+0). They introduce independent
player power/threshold references rather than recovering the twelve retail
data pairs. The normal-row path and cap region also use different player
anchors. No variant resolves the joint address/loop residue, so reject all
six and retain the original body at 88.824814%. No function is banked.

Separate disposable recompilation resolves all words and relocations. Across
all seven states, the twelve sibling functions match their raw retail bodies
and ordered calls/address pairs. The canonical source hash is unchanged.
The JSON report is
`build/hypotheses/20260908-213541-game-player_death-player_add_experience`;
the manifest is `build/experience-inline-views-hypotheses.json`, the refreshed
dossier is `build/experience-inline-views-evidence.txt`, and independent
objects/checks are under `build/experience-inline-views-objects`.

The full production build remains GAME 337/362 exact / 99.428% aggregate,
with source-data matches 11/41 and target relinks 75/77. OPEN/PSX retain
3/19 and 0/1 data matches, and 34/38 and 1/1 relinks. There are no artifact
failures; existing data/relink gates keep the build nonzero. This follow-up
changes documentation only. The preceding 713-test/Ruff run covered the same
production source, configuration and tests; whitespace checks pass again.
