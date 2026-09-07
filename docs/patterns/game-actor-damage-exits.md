# GAME actor damage guards and shared exits

## Function Match Plan

Target: `GAME.EXE` `0x8002d120`, `actor_apply_damage`, 0x388 bytes,
owned by `game.actor`. Initial strict objdiff score: 89.986725%; the fresh
candidate has 908 bytes. The unit starts with 24/29 exact functions.

The image-qualified address, complete block disassembly, incoming/outgoing
xrefs, strings, and match views were inspected, together with the six direct
caller windows and their source, the preceding damage-component helper, the
following radial-damage caller, shared actor layouts, and source history.
The unconditional health guard dates to `12d4f21`'s first actor-core source.

Evidence snapshot:

- Nine `u16` arguments: register masks/spills and incoming stack halfword
  loads agree with weapon, radial-damage, and projectile/effect callers.
  Damage arithmetic and the final health difference use signed 32-bit values.
- Actor stride 0x48, definition stride 0x98, unsigned byte definition/action
  fields, health halfword at actor+0x14, defenses at definition+0x8c..0x94,
  experience at +0x84, animation slots at +0x11/+0x12.
- Ten proven calls: five damage-component calls, two training calls, one
  experience call, and two action changes. No unresolved indirect transfers
  or string references. No target/helper entry in the vendored inventory;
  the actor/definition, player progression, and floor-script accesses identify
  game logic rather than an SDK/runtime wrapper.
- Three validated HI16/LO16 pairs: `actor_state+0x720`, `player_state+0xa`,
  and `map_runtime_state+0x1cbe`. Two validated internal `R_MIPS_26` jumps
  go to +0x2cc and +0x358. No candidate reference is being promoted.
- 27 conditional branches, two internal jumps, ten calls, one `jr ra`,
  and four division traps. Control-transfer delay slots remain part of the
  body, including the final stack restore. Frame size is 88 bytes.
- Constants retained: floor 5, definition 7, dying phase threshold 1548,
  ten damage subunits per HP, rounding bias 5, scale denominator 5000,
  credit mask 0xf0/player credit 0x10, and existing typed action/slot values.

First hypothesis: restore the proven scope of the boss-only health guard.
At `0x8002d1b4` and `0x8002d1c0`, floor/definition mismatches branch to
`0x8002d1ec`, past both the boss flag and zero-health checks. The source must
not apply the zero-health early return to other actors. Shared epilogue
`0x8002d478` does not make every incoming guard unconditional.

Then inspect the first remaining divergence after each focused rebuild:
retail rounds the accumulator before division; masks hit flags in the
zero-damage branch's always-executed slot; and places the fatal arm before
the surviving arm. These are separate evidence-backed hypotheses, not a
license for spelling permutations or artificial register carriers.

Keep the corrected guard even if an intermediate score falls. Preserve the
other 28 functions, require strict 100% for banking, and audit raw words,
ordered call/data targets, control transfers and delay slots before closure.

## Results

The retained source reaches strict native objdiff **100%**, up from 89.986725%,
with `probe-gcc257-o2-g0`. This does not prove historical compiler attribution.

The focused sequence isolated the discrepancies:

1. Nesting both boss-flag and health guards inside the floor/definition case
   fixed the two wrong entry edges and the action-load delay-slot schedule.
2. Moving the existing credit mask before the zero-damage test filled retail's
   always-executed branch slot and removed the extra word (908 -> 904 bytes).
3. Testing `remaining <= 0` first restored the fatal-first arm, with the
   surviving arm second and the existing final health-store join.
4. Rounding the accumulator in place fixed `addiu s1,s1,5` and the dividend.
   A separate `damage /= 10` assignment still produced `mflo s1` instead of
   retail's transient `mflo v0`. Keeping that quotient inside the final
   scaling expression restored its lifetime without adding a local:

   ```c
   damage += ACTOR_DAMAGE_SUBUNITS_PER_HP / 2;
   damage = (damage / ACTOR_DAMAGE_SUBUNITS_PER_HP) * scale / KF_ACTOR_DAMAGE_SCALE_ONE;
   ```

Fresh compilation and relocation-aware raw comparison verified all 226 words
against the retail image, including every control transfer and its delay
slot, ten ordered calls, and three ordered data addresses. Both internal
jump targets agree. All other 28 functions' resolved words and references
are unchanged from the pre-campaign compiled baseline; 25/29 are now exact.
The remaining four actor-core functions are not claimed as closed.

All 484 scored rows were compared before/after: only this function's percentage
changed. Eligible exact counts are GAME 304/362, OPEN 98/108, PSX 1/1, or
403/471 overall; the 13 exact vendored source controls remain excluded.

The affected unit was rebuilt with `kf match --unit game.actor`, followed by
the full `kf build`. Its non-green result remains the existing data/placement
and known-reference closure work, with no artifact failures: GAME data 9/42,
OPEN 2/19, PSX 0/1; target relink 75/77, 34/38, and 1/1 respectively. The
function's exact result does not close the whole module or linked program.

Verification: all 665 repository tests passed (80.679 seconds),
`ruff check scripts tests` passed, and `git diff --check` passed. No tooling,
compiler/profile, data claims, or curated relocation inputs were changed.
`kf bank --function game:0x8002d120` banked only this verified exact row.
