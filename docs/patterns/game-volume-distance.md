# GAME player and actor volume-distance queries

## Function Match Plan and shared evidence

Campaign starts at clean `39492de`, GAME **274/362 strict-exact**. Retail
hashes are validated by `kf init`. Both targets implement inclusive X/Z
range rejection, an optional vertical interval test, and eighth-scale
horizontal squared distance through the same SDK square root. This shared
algorithm, call set and arithmetic establish the relationship. Their source
owners and public signatures remain separate.

All six required semantic views were read for each target using
`--image game`, including unfiltered references and current strict match
state. The complete retail bodies, raw focused differences, caller sites,
shared declarations and source history were inspected before editing.

Both retail bodies have ten CFG blocks, a 24-byte frame, saved `$s0/$ra`,
one proven call to `SquareRoot0`, and a common epilogue. Each rejection
uses `bnez` with `li v0,-1` in the delay slot. The X arithmetic shift by
three is in the optional-Y guard's delay slot. Signed `mult` squares both
scaled horizontal deltas; the sum is formed in the SDK call delay slot.
The result is shifted left three and bounded before return. The epilogue
restores both saved registers and releases the frame in the `jr` delay
slot. There are no strings, indirect transfers or candidate outgoing refs.

Vendor negative controls: neither target appears in the SDK/FID inventories.
Their entity-specific position and volume policy is game-owned. The sole
callee is the already-vendored Psy-Q Release 2.5 `LIBGTE.LIB:MSC+0x478`
`SquareRoot0` (132-byte signature, 1024 fixed instruction bits); authentic
`LIBGTE.H` declares `long SquareRoot0(long)`. Its raw body, including the
trailing zero-result return, was inspected. The navigator's fragmented-CFG
limitation for this SDK body is not bypassed or promoted to game ownership.

## GAME 80017108: player distance

`player_distance_to_point`, **244 bytes / 61 words**, `game.player_core`,
starts at **95.327866%**. Its five signed word inputs are X/Y/Z, maximum
distance and point height; the fifth is loaded from incoming stack+16.
Y `0xffff` disables the interval test. Player height is fixed at 1700:
half point height and 850 define the summed half-height and relative center.
Validated HI16/LO16 pairs read `player_state+0xa4/+0xac/+0xb4` (camera X/Z
and floor height), followed by the call relocation: **seven ordered ELF
relocations**. The load instructions and addends are already correct.

All four proven callers were inspected in source and retail:

- `80016718`, radial damage: three origin words, radius in `$a3` and
  stack+16; the argument move occupies the call slot. The caller consumes
  -1 or the word distance for falloff.
- `80017078`, cone query: point X/Z, Y sentinel, bound and zero height;
  bound moves into `$a3` in the slot. It retains the distance for return.
- `8001a784`, world collision: point X/Y/Z, radius+800 in the slot and
  height on stack; tests the -1 result.
- `800321d4`, map-object update: object X/Z, Y sentinel, zero height and
  30000 maximum distance in the slot; tests -1. This direct call does not
  close the caller's unresolved action-table inventory.

The preceding cone helper was read completely, as was the following
movement entry and its first collision path. Original source history includes
`4577e2a:src/game/player_80017040.c` and later owner consolidation.

The current C's last vertical rejection introduces an inverted branch and
extra internal jump, unlike the retail common rejection tail. It also puts
the final distance in `$a0` rather than `$v1`. First hypothesis: use a shared
`out_of_range` return for all four rejection guards, as independently
corroborated by the actor helper's matching rejection topology. Keep every
predicate, arithmetic expression, width, call, constant and data reference.
Require all 61 words and seven relocations, not only a higher score.

## GAME 8002d97c: actor distance

`actor_distance_to_point`, **240 bytes / 60 words**, `game.actor`, starts
at **96%**. It accepts an actor pointer and six signed word scalars: point
X/Y/Z, maximum distance, actor height and point height. The last three come
from caller stack+16/+20/+24. Word positions at actor+`0x1c/0x20/0x24`
and arithmetic shifts establish full-width coordinates and signed heights.
Its only relocation is the proven SDK call; no global address pair occurs.

All nine proven sites were inspected in source and retail:

- `8002d5a4`, radial damage: origin XYZ, radius, definition `lhu` height
  and radius again; the actor pointer moves in the call slot.
- `8002d70c`, attack: player X/Z, player Y+1500, unsigned-halfword maximum
  distance, definition height and 1700; actor height is stored in the slot.
- `8002d8a0`, cone selector: origin X/Z, Y sentinel in the slot, maximum
  distance and zero heights.
- `8002db40`, overlap: input XYZ, definition radius plus extra radius,
  definition height and input point height; height is stored in the slot.
- `8002e73c/e8cc/e914`, awareness: player X/Z, Y sentinel in each slot,
  bounds 28000/32000/32000 and zero heights.
- `8002f52c/80030658`, action updates: player X/Z, Y sentinel in each slot,
  bound 32000 and zero heights. Both forward the distance to action choice.

Both neighbors (cone selector and overlap scan) were read completely.
Source history `642f170` already has the shared rejection tail and the
current separately computed `top` expression. Those branches and the SDK
call agree with retail. The first mismatch is the actor-height/negative-bound
register exchange (`$t2/$t1` versus `$t1/$t2`), followed by actor Y being
loaded after rather than before the point-center subtraction.

First hypothesis: express the vertical center delta as the single real
calculation `(actor->position.vy - actor_height) - (point_y - point_height)`
after halving both heights, then form their sum in `point_height`. Remove
the now-unnecessary separate `top`; preserve the subtraction association,
signed widths, sentinel, predicates, shared exit and call. This tests the
retail center calculation directly, with no fake load carrier, volatile or
forced register. Require all 60 words and the one call relocation for closure.

## Final verdicts

The player's shared rejection exit is **100% strict objdiff**, up from
95.327866%. Raw comparison confirms all **61 encoded words** and **seven
ordered relocations** exactly. Both the vertical branch and final distance
register now agree. `game.player_core` moves from 12/14 to **13/14 exact**.

The actor's direct center expression improves from 96% to **99.333336%**.
It restores the retail Y-load position without changing the 60-word extent,
branches, call, constants or arithmetic. Seven encoded words still exchange
`$t1/$t2` roles between actor height and the negative horizontal bound:
function-relative +`0xc`, +`0x20`, +`0x24`, +`0x48`, +`0x6c`, +`0x7c`,
+`0x80`. The SDK call remains `R_MIPS_26 SquareRoot0` at +`0xc4`.
This is retained as a simpler, evidence-supported center calculation, but
is **not exact or banked**. No compiler mechanism or forced register fix is
claimed. The actor unit stays 23/29 exact.

The two targets are the only changed comparisons across all 484 report rows,
including 13 vendored verification controls. The other 482 are unchanged.
GAME moves **274/362 to 275/362 exact**; OPEN stays 97/108 and PSX stays
1/1. No banked function regresses.

Verification: **551 repository tests pass** (47.806 s), Ruff passes and
`git diff --check` passes. Full `kf build` still exits 1 on existing
non-code closure gates: source data 11/59, config SDK data 2/2 and target
relink 108/114, plus incomplete known-reference data ownership. No source
owner, SDK attribution, curated relocation or verification gate was changed.

## Horizontal-parameter control at `8bb4c8b`

Function Match Plan: GAME `8002d97c`, 240 bytes, strict 99.333336%, unchanged
GCC 2.5.7 probe. Refresh all six image-qualified views, the complete sixty
retail instructions, nine caller setups and both adjacent actor scans. The
seven word arguments, signed heights, half-height arithmetic, Y sentinel,
24-byte frame, ten blocks, eight branches, sole SDK call and common -1 exit
remain as documented. SquareRoot0 retains its independent LIBGTE attribution;
this coordinate/volume policy has no vendored identity. There are no data
references, strings or unresolved transfers in the selected game function.

Retail overwrites the incoming X/Z registers with actor-relative differences
and then eighth-scale differences; the original absolute X/Z values are not
used again. Test expressing those two evolving values through `point_x` and
`point_z` themselves, removing only the separate `delta_x`/`delta_z` locals.
Keep the Y calculation, signed comparisons, their operand order, the two
shift locations and the shared rejection tail unchanged. This tests the
observed input lifetime; it does not add a cached negative bound or permute
height locals to choose t1/t2. Require complete raw comparison and unchanged
sibling bodies before retaining it; strict 100% remains the closure criterion.

### Exact actor-distance result

Using the two horizontal parameters as evolving coordinates reaches strict
**100%**, up from 99.333336%. All sixty retail words agree, including the
eight rejection/sentinel branches, the SquareRoot0 call at +0xc4 and the
restoring return delay slot. The only changes from the previous body are
the seven register operands already identified above: height now uses t1 and
the negative horizontal bound uses t2. No constant, type, signature, source
owner, relocation or SDK interface changes. Both player and actor distance
helpers are now exact; no optimizer-mechanism attribution is implied.

A fresh pinned compilation compares all 27 exact actor functions directly
against hash-verified GAME bytes and their ordered calls/address pairs. The
distance body's SHA-256 is
`db54e4f032fba574067312f16346237ef9700aa11b699a4421fe316c26b843a7`.
The other 28 actor bodies retain their previous bytes and all non-debug
relocation identities/addends; the two remaining partial actor functions
are unchanged after their rejected controls. A committed-source/header
control isolates this function from concurrent player-damage naming in the
same actor file and shared player header.

Across all 484 scored rows, only actor distance changes. GAME advances
309 -> 310/362 exact, OPEN stays 98/108 and PSX 1/1, totaling 409/471.
Ruff, `git diff --check` and all 678 repository tests pass (79.059 seconds).
The source was freshly rebuilt and the full build run; existing failures
remain data/ownership/placement: PSX data 0/1, GAME 9/42, OPEN 2/19;
target relinks 1/1, 75/77 and 34/38, six conflicting-section cases and
zero artifact failures. No tooling, flake or verification gate changes.

Selected banking after staging only this campaign refuses the concurrent
player-damage naming inputs, including the unrelated multiplier call in the
same actor source. Preserve that unstaged hunk and do not use a dirty override.
Commit the isolated exact distance reconstruction; its one baseline update
remains pending those independent inputs. The preceding phase predicate's
deferred bank was completed separately in `fb0297b` while inputs were clean.
