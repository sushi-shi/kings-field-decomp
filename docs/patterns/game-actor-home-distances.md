# GAME actor dispatcher: home-distance dependency control

## Function Match Plan

Target: `GAME.EXE 0x8002fa88`, `actor_update_current_action(void)`, body
`0xd90` (3472 bytes), in `game.actor_behavior`. The fresh unchanged probe is
3480 bytes and native objdiff is 98.938940%. The unit has nine exact functions;
`actor_update_awareness` is the other partial and is not part of this edit.

The six semantic views were refreshed against hash-validated retail bytes.
All 868 instruction words agree with the earlier complete dispatcher dossier.
The sole direct caller, `actor_pool_update` at `0x8003087c`, supplies no
arguments and ignores the result, after binding an occupied, active actor.
The preceding boss-death routine ends at `0x8002fa88`; the following pool
updater starts at `0x80030818`. Source history includes the byte-preserving
actor-unit combination `6d05a06` and subsequent shared type/name work.

This is game actor policy, not an SDK wrapper: it reads the bound actor and
definition, handles action/progress states, invokes game movement, attack,
animation and collision functions, and adjusts cell occupancy. There is no
vendored attribution for this body. The `rand` callee is separately attributed
to Psy-Q `LIBAPI.LIB` member `C47`; that library body is not reconstructed here.

### Control flow, calls and referents

- Retail has 69 direct calls, 86 conditional branches, 43 direct jumps, two
  indirect jumps, and one `jr ra`. Every transfer retains its delay slot.
- The 80-byte frame saves `ra`, `s3`, `s2`, `s1`, and `s0` at offsets
  72, 68, 64, 60, and 56. The actor and definition are `s0` and `s1`.
- Five jumps go directly to the epilogue at `0x800307fc`: `0x8002fd18`,
  `0x8002ff40`, `0x8002ff6c`, `0x8002ff88`, and `0x8002ff90`. They preserve
  the boss-death, transformation, and three post-death lifecycle exits,
  bypassing the normal occupancy-add call at `0x800307f4`.
- Fourteen ordered HI16/LO16 pairs refer to the actor context/player position,
  player floor state, map attribute grid/height table, and two dispatch tables.
  No string references occur. No relocation or identity change is proposed.
- The unit's `RODATA(0x800124d4, 0x264)` contains the preceding effect table
  (20 rows), action table at `0x80012524` (128 rows), and vertical table at
  `0x80012724` (five rows). All rows point inside their respective functions.
  Action 33 targets `0x800303cc`. Candidate table references remain candidates;
  the navigator's incomplete indirect CFG is not evidence that these arms
  are unreachable.

### First source hypothesis

The first raw difference at `+0x44` is a branch displacement caused by the
eight-byte excess. The first non-target instruction difference is at
`0x80030414`: the tile/product temporaries use different registers. Existing
multiply-spelling experiments in `source-shapes-gcc257.md` are not repeated.

There is a separate directly observed dependency fact: retail loads both
position components (`lw` at `0x80030454` and `0x80030458`) before the first
near-home branch. The Z subtraction at `0x8003046c` is that branch's delay
slot, so it executes on both outcomes. The unchanged source instead places
the Z expression behind short-circuit `&&`; its probe loads Z later and
contains two extra load-delay nops in this region.

Introduce meaningful block-scoped `s32 home_dx, home_dz`, assigned after the
existing home-coordinate calculations and before either bounds test. Tile
coordinates remain unsigned bytes, local offsets signed halfwords, current
positions signed words, and both tests remain strictly between -200 and 200
(`addiu 199; sltiu 399` in retail). Keep `home_x/home_z` for the heading call.
Do not initialize their observed read-before-assignment on the progress-zero
path, change animation slots, or move either calculation into another arm.

Verification: fresh focused compile, first-divergence comparison, raw linked
instructions/ordered targets and switch rows, native objdiff, all-unit score
comparison, then full build/lint/tests before any commit. Exactness remains
strict native 100%, not `kf try`'s listing similarity.

## Outcome

The first focused experiment produces an exact instruction/relocation listing:
10 of the unit's 11 functions now agree, with only the untouched awareness
routine partial. Both extra nops disappear and the tile/product register
sequence also agrees; no multiply rewrite or compiler-profile change is needed.

A separate fresh compile resolves every relocation to its numeric retail
referent, without masks. The dispatcher is exactly 3472 bytes, all 868 words
match, all 69 call targets and 14 ordered HI16/LO16 referents match, and all
201 control transfers (including the five early-exit jumps) and their delay
slots agree. The linked-byte SHA-256 is
`ac036226c3167ecb900baf7ad50a46a602ec2b6c58fa4f82e984167bc94cdd12`.
The other ten functions are byte/reference-identical to the pre-edit objects;
the nine exact neighbors remain raw exact. Resolving table destinations through
their containing functions' curated retail addresses, both objects' complete
612-byte RODATA sections reproduce all 153 retail pointer rows, with no extra
tail bytes. This destination audit does not claim whole-unit data closure:
the untouched awareness routine is four bytes short, shifting section-relative
table addends, and the compiler section's eight-byte alignment does not admit
the claimed RODATA base. The data gate still reports both defects.

The productive probe remains GCC 2.5.7, `-O2 -G0 -mcpu=r2000`, maspsx
`--expand-div`, ASPSX profile 1.07. This result closes a source dependency
difference; it does not prove historical compiler attribution or an optimizer
mechanism for the old register symptom.

Native objdiff confirms **100%** (previously 98.938940%). Across all 484 scored
functions, this is the only percentage change. Eligible game-source totals
move from 401/471 to **402/471**: GAME 303/362, OPEN 98/108, PSX 1/1. The 13
vendored source controls are excluded from these progress totals.

All 657 repository tests pass (91.603 seconds); Ruff and `git diff --check`
pass. The full build freshly recompiles the affected unit and reports the same
new exact result. It remains non-green on existing data/placement and
known-reference coverage gates: GAME data 9/42, OPEN 2/19, PSX 0/1, and target
relink 75/77, 34/38, 1/1 respectively. The new GAME data-owning unit comes from
concurrent entity-render ownership work, not this dispatcher edit. Config-owned
SDK data remains 4/4 verified.

Banking was initially deferred while unrelated render/type/inventory build
inputs were unstaged; no `--dirty` override was used. After that work was
committed as `fef7f30`, the full rebuilt corpus still had this function at
strict 100%, with all 484 scores unchanged. The single function was then
banked with `kf bank --function game:0x8002fa88`; no other ledger row changed.

The intervening CFG-tool correction (`d378713`) now marks the actor dispatcher's
159 untraced blocks as `reachability-unknown` and reports both unresolved
dispatches. It does not manufacture switch edges or change the independently
verified five early-exit jumps. All 665 repository tests, Ruff, and
`nix flake check -L` pass; the full-build data/placement limitations above remain.
