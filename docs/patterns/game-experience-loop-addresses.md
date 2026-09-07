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
