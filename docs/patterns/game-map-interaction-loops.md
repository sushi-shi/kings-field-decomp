# GAME interaction-loop reconstruction

## Function Match Plan

At master `382cca6`, GAME `map_interaction_dispatch` (`0x80034de4`, 2308
retail bytes) starts at strict **96.490470%**, with 2288 compiled bytes on
`probe-gcc257-o2-g0`. Its owner remains the WIP contiguous `game.map_scripts`
unit. The complete six-view dossier, retail and compiled instructions/CFG,
sole caller, helper interfaces and bodies, adjacent boundaries, shared object
layouts, data references, and source history `8d49a84`/`d591e9c` were reviewed
before the isolated source controls.

Preserve `void(const VECTOR *, SVECTOR *)`. The call in `player_update` at
`0x80018a3c` supplies the player camera position at `0x800a0824` and its rotation
at position +20 in the delay slot; no result is consumed. The 72-byte frame
saves nine s-registers and ra. The function has 54 ordered direct calls,
13 ordered address pairs, and two bounded table dispatches: 84 behavior rows
and five one-based floor rows. The 89 existing pointer candidates remain
candidates. The navigator's indirect edges are not promoted by this work.

The custom interaction, container, door, notification, image and floor policies
are game code. `rsin`/`rcos` remain separately attributed Release 2.5 LIBGTE GEO
providers; SDK bodies and interfaces are unchanged. There are no direct string
references in the dispatcher. Preserve the private map-image path and the
partial adjacent formatter. Fourteen other unit functions are exact controls.

The first raw difference is the position pointer in s8 instead of retail s7
at +0x2c. The outer minus-one constant later occupies the opposite saved
register. The first substantive CFG difference is the hinged-container
prescan, followed by its opening loop. Calls and ordered physical referents
already agree. Recover the observed counter/cursor ownership, widths, loop
conditions and exits before drawing conclusions about register allocation.

The initial hypothesis gives the hinged and unhinged container cases their
own signed-halfword counter and byte cursor. Retail uses s0/s1 for the first
pair and s1/s0 for the second; the starting source uses one function-scope
pair for both. Keep the initializers at their original operation points for
that first control. Subsequent controls must isolate independently observed
pitch values, preparation order or branch joins. Every candidate must retain
all other fifteen functions and pass native/traced parity and raw referent
checks. Strict 100% is the only banking criterion.

## Retained source facts

The final source reaches **99.211440%**, with 2296 compiled bytes. It retains
six related corrections:

- Each container case owns its counter and cursor. The first case shares its
  counter between the prescan and later pickup pass. Both emitted traversal
  register pairs now agree with retail.
- The opening loop keeps the consumed original pitch sample separately from
  its `u16` displacement from the interval start. Compound subtraction narrows
  that displacement before its unsigned comparison. This recovers the
  `65345` add, halfword mask, span `1858`, and reuse of the original sample
  for the conditional `+16` store. The SDK `SVECTOR` declaration is preserved.
- The pickup cursor is selected before its countdown is initialized. The
  compiler now forms object +33 in both opening-guard delay slots and starts
  the countdown after the opening loop, matching retail.
- A failed partner search reaches its parameter-none store after the loop.
  Successful pairing skips that store and joins the original-door action.
  This recovers the decoded miss/success join at `0x80035504/08`.
- The existing gold amount snapshot becomes the total after the notification
  and supplies the store. This recovers the two retail sum/store instructions
  at `0x8003556c/70`; the notification still receives the original amount.
- The prescan samples its existing parameter byte inside each iteration,
  tests the nonempty exit, then performs the narrowed pre-decrement test.
  Its top condition and unconditional back edge now agree structurally with
  retail. The compiler still hoists the byte read. No advancing cursor is
  invented for this authored repeated-byte scan.

## Compiler observations and controls

The normal, instrumented and debugger compilers emit identical whole ELFs for
the real baseline, gold intermediate, neighbor-selection control and retained
source. Nine small control sources also have three-way parity. The compiler
source hash is `222b6cc36272847ffde09ad4b7f2db81632fe7fd282bfc6df1f155939b63cb12`;
these observations describe this pinned probe, not historical attribution.

The scope control changes one named counter/cursor pseudo pair into two.
The small two-case control reproduces that identity change: counter 73 becomes
73/83, and cursor 74 becomes 74/84. This supports the real disjoint source
ownership; it does not assert that unrelated small functions must select the
same hard registers as the dispatcher.

The pitch controls expose the numeric difference at EXPAND. The direct
expression adds signed `-191`; the consumed `u16` displacement materializes
`65345`, which LOOP hoists and COMBINE retains with its unsigned test. The
small lid-opening pair reproduces this change. An independent exhaustive
check of all 65,536 pitch bit patterns verifies both the interval decision
and stored pitch value. Both real earlier variants already have a duplicated
initial opening guard at JUMP1; its eventual retention is not attributed to
a guessed loop-complexity threshold.

`stmt.c`'s `expand_end_loop` scans for an opening conditional exit to rotate
to the tail, stopping at block-scope notes, labels or calls. The real consumed
per-iteration parameter sample places its scope before the guard; EXPAND keeps
the top test instead of rotating it. A small outer object scan with the same
byte/count prescan reproduces both forms, and LOOP hoists the sampled load.
This control also reproduces the remaining sharing of the outer minus-one
constant. The scope belongs to an actual consumed byte value; there is no
unused declaration or volatile barrier.

The neighbor control separates selection from validation: admission breaks
the search loop, and link validation and the action follow it. It preserves
all three behavior loads through CSE1/CSE2, where the retained source keeps
only the first. In the real control, CSE1 stops at loop-end note 1154 before
starting validation at 1157; its later link branch follows a separate AROUND
path. The small pair reproduces one versus three loads, with its corresponding
loop-end stop at 69 and validation starting at 72. This agrees with CSE's
explicit loop-end boundary rule. The control also changes branch layout and
is not retained. Changing its small selection loop from `for` to `while`
produces identical complete `.text`, so that spelling is not retried in GAME.

## Isolated verdicts

Parents are stated where a control did not use the preceding retained state.
The source/call/referent audit preserves all fifteen siblings for every row.

| Control | Strict % | Bytes | Verdict |
| --- | ---: | ---: | --- |
| Baseline | 96.490470 | 2288 | Starting evidence |
| Case-owned traversal | 96.585785 | 2288 | Retain independent source ownership |
| Consumed `u16` displacement | 96.506065 | 2320 | Retain numeric intermediate despite lower score |
| Original pitch sample | 97.372620 | 2312 | Retain sample reuse |
| Explicit for-body prescan | 96.454070 | 2316 | Reject; still rotates the guard |
| Labeled prescan, from pitch sample | 97.230500 | 2300 | Reject; wrong count/cursor allocation and shared minus-one |
| Cursor-before-count, from pitch sample | 98.006935 | 2308 | Retain observed preparation order |
| Failed-neighbor join | 98.535530 | 2304 | Retain decoded store/skip join |
| Grouped behavior switch | 98.093590 | 2312 | Reject; adds signed guard, still one behavior load |
| Gold accumulation, from failed-neighbor join | 98.552860 | 2304 | Retain actual sum destination |
| Selection before validation | 98.301560 | 2316 | Isolated positive load control; wrong branch layout |
| Compound prescan condition, from gold | 97.866554 | 2300 | Reject; adds post-loop narrowed counter test |
| Per-iteration prescan sample, from gold | 99.211440 | 2296 | Retained source |
| Sample plus selection-before-validation | 98.960140 | 2308 | Isolated composition; exact size is insufficient |

## Remaining differences

The function remains partial and is not banked. The prescan's local
`li a0,-1` at `0x800351fc` is still a nop, its exhausted test uses the outer
minus-one register, and its unconditional jump returns to that nop instead of
the following compare. The trace retains the local constant at CSE1 and
combines it into the outer constant during LOOP. Position/minus-one retain
their opposite s7/s8 roles.

The paired-door path still reads its definition byte once into a1, reusing
it for admission, linked-door rejection and the action argument. Retail reads
it three times, retains the definition base in v1, and occupies twelve more
bytes. No intervening store or call justifies adding volatile qualification.
Ending selection before validation restores the loads but moves the increment
and back edge ahead of validation and reverses the definition-address add
operands. That lower-scoring state remains an experiment, not a source fact
established by its load count.

No relocation, data identity, SDK type, production compiler/profile or bank
entry changes. The remaining raw differences stay open without a compiler-wall
label or a forced source carrier.

## Verification and unit verdicts

The affected production object was rebuilt by the normal focused match.
Its resolved words agree with the retained isolated object; all delinked
targets reconstruct literal retail words. The 54 call targets and 13 ordered
physical references agree throughout the dispatcher. The four recovered
pitch/guard/preparation regions are checked as raw words, and all fourteen
exact siblings remain literal-retail exact across 924 words. The private DATA
bytes are unchanged. The existing literal ledger keeps all 211 occurrences,
expressions and reasons; only this function's line references are updated.

| Function | Final strict % | Verdict |
| --- | ---: | --- |
| `actor_pool_find_at_tile` | 100 | Unchanged, raw exact |
| `map_ambient_script_floor1` | 100 | Unchanged, raw exact |
| `map_ambient_script_floor2` | 100 | Unchanged, raw exact |
| `map_ambient_script_floor3` | 100 | Unchanged, raw exact |
| `map_ambient_script_floor4` | 100 | Unchanged, raw exact |
| `map_ambient_script_floor5` | 100 | Unchanged, raw exact |
| `map_action_script_floor1` | 100 | Unchanged, raw exact |
| `map_reveal_fade` | 100 | Unchanged, raw exact |
| `map_action_script_floor2` | 100 | Unchanged, raw exact |
| `map_action_script_floor3` | 100 | Unchanged, raw exact |
| `map_action_script_floor4` | 100 | Unchanged, raw exact |
| `map_floor5_transition_cutscene` | 100 | Unchanged, raw exact |
| `map_action_script_floor5` | 100 | Unchanged, raw exact |
| `map_event_interact` | 100 | Unchanged, raw exact |
| `map_show_screen_image` | 88.888885 | Unchanged partial, 148 compiled bytes |
| `map_interaction_dispatch` | 99.211440 | Improved partial, 2296 compiled bytes |

Ruff, modern type checking and all 725 repository tests pass (121.457 seconds).
The subsequent full build retains its existing data extent/ownership/addend/
placement and conflicting-base failures, with no exact-function regression.
This campaign adds no exact function: current counts are GAME 336/362,
OPEN 106/108 and PSX 1/1. The independently committed TMD enqueue closure is
not credited to these interaction changes. No tooling changes require an
additional flake check.

Generated plans, six-view dossiers, isolated objects, traces, debugger paths,
nine small controls, raw audits and verification logs remain under
`build/gcc257/game-interaction-revisit/`. The retained source SHA-256 is
`9088396cd2af5ceb38f581a27ca2b7f2ac8e2cfa921ce793aa8edc939b2a1c28`.
