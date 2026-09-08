# GAME actor awareness: shared suppression exit

`actor_update_awareness` at GAME `8002e6a8` now matches all 684 retail bytes
under the unchanged `probe-gcc257-o2-g0` profile. The near-player and overlap
rejections jump to the failed-roll branch's existing wait-for-range-exit
store. This expresses the retail shared block directly and lets the probe
reuse the lifecycle comparison's constant in the later slot-policy test.

## Function Match Plan and evidence snapshot

The campaign started on master `d746e9c` with strict 97.736840%, 680 emitted
bytes, and ten exact siblings in `game.actor_behavior`. All six GAME semantic
views, the sole caller, game callees, adjacent functions, and source history
were reviewed before the isolated edit. The plan and captures remain under
`build/gcc257/game-actor-awareness/`.

`actor_pool_update` binds the current actor, calls awareness without arguments,
and ignores its return. Awareness captures the slot byte before the first
distance query and promotes it afterwards. Preserve this two-stage local:
the earlier enum-width experiment documented in
[modern-type-checking.md](modern-type-checking.md) already disproved narrowing
both locals. Lifecycle and spawn chance are unsigned bytes, local coordinates
are signed halfwords, collision radius is an unsigned halfword, and distance
is signed. The distance queries pass seven arguments; overlap passes five.

All eleven ordered calls and nine ordered address pairs agreed at baseline.
The curated target independently resolved to literal retail words. There are
no strings or candidate references. The local actor/definition/player data
and lifecycle operations establish game ownership; `rand` at GAME `8005049c`
is separately identified as the Psy-Q `LIBAPI.LIB` C47 signature and excluded
from game progress. No SDK body, type, compiler flag, or inventory was changed.

Retail branches from the near-player gate (`8002e808`), failed slot-policy
test (`8002e830`), and overlap rejection (`8002e888`) all reach the same block
at `8002e8a0`. Its jump owns the lifecycle byte store in its delay slot. The
common action-selection call remains at `8002e8e4`; the terminal return owns
the stack restore at `8002e950`. Range limits 28000/26000/32000, height sentinel
65535, query miss -1, tile size 2000, spawn-chance shift 7, animation sentinel
255, action 32, and all lifecycle/slot values remain unchanged.

The first trial replaced both early suppression store/return pairs with
`goto wait_for_range_exit` and labeled the existing final suppression store.
This is one shared-exit hypothesis supported by the retail destinations.
It reached strict 100% on the first focused build.

## Measured CSE behavior

The old first raw divergence was the saved `$ra` offset, 56 instead of 60.
Both frames were 64 bytes, but retail saved one additional register. Retail
loads `1` into `$s5` for the lifecycle switch and reuses it after two calls
for `spawn_policy == KF_ACTOR_SLOT_PERSISTENT`; the old source materialized
two separate constants. This difference was already present after CSE1.

| Observation | Separate early returns | Shared suppression store |
| --- | --- | --- |
| Near-gate continuation label | UID196, two uses | UID191, one use |
| CSE traversal from function entry through UID180 | Stops before later random call | Follows branch with status `AROUND` |
| Constant-1 pseudos after CSE1 and CSE2 | 153 and 118 | 151 only |
| Shared constant global allocation | None | `$s5`, refs/calls/lifetime 3/2/68 |
| Existing -1 pseudo81 allocation | `$s5` | `$s6` |
| Strict score / emitted bytes | 97.736840% / 680 | 100% / 684 |

The pinned compiler's `cse_end_of_basic_block` only follows a conditional
target with `LABEL_NUSES == 1`; its branch-around path also rejects intervening
labels and invalidates values modified in the skipped block. Independent GDB
entry/return observations confirm the table in both CSE passes. The shared
label changes the second near-gate branch to the common rejection target,
leaving only one use of the continuation label. CSE can then carry the entry
constant through to the later comparison. No source constant carrier was
introduced.

A small three-state awareness control repeats the same rejection structure
with a minimal actor record and external distance, random, overlap, and
activation calls. Its separate-return version stops at a two-use label and
retains two constant-1 pseudos; the shared-store version follows `AROUND`,
keeps one pseudo, and allocates it to `$s4` across two calls. Both real sources
and both small controls emit identical whole ELF objects under native,
instrumented, and debugger compilers. This supports the observed probe
mechanism, without establishing the historical compiler identity.

## Final verdicts and verification

| Function | GAME address / bytes | Verdict |
| --- | --- | --- |
| actor_select_next_action | 8002e2e8 / 960 | Unchanged exact |
| actor_update_awareness | 8002e6a8 / 684 | Newly exact |
| actor_move_xz_with_collision | 8002e954 / 940 | Unchanged exact |
| actor_move_along_heading | 8002ed00 / 212 | Unchanged exact |
| actor_spawn_action_effect | 8002edd4 / 1108 | Unchanged exact |
| actor_prepare_charge_toward_player | 8002f228 / 244 | Unchanged exact |
| actor_apply_horizontal_movement | 8002f31c / 332 | Unchanged exact |
| actor_update_effect_action | 8002f468 / 240 | Unchanged exact |
| actor_apply_random_movement | 8002f558 / 884 | Unchanged exact |
| actor_update_boss_death_sequence | 8002f8cc / 444 | Unchanged exact |
| actor_update_current_action | 8002fa88 / 3472 | Unchanged exact |

The normal focused build recompiled `game.actor_behavior`. Independent
relocation resolution checks every instruction, direct call, and ordered
referent against retail for all eleven claims; all score strict 100%.
Modern type checking, Ruff, and all 713 repository tests pass. The full
`kf build` reports no banked-function regressions and retains the known data
ownership, extent, addend, and placement failures in all three images.
GAME advances from 330/362 to 331/362 exact; OPEN remains 106/108 and PSX 1/1.
The complete OPEN/GAME function objective remains open.
