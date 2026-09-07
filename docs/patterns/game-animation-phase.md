# GAME animation phase predicate

## Function Match Plan and evidence snapshot

Target: `GAME.EXE:0x8002dccc actor_animation_crossed_phase`, 48 bytes,
`src/game/actor.c` / `game.actor`, initially **97.5%** strict objdiff.
GAME starts at 271/362 exact; this actor unit at 23/29.
The retail hashes were validated during the preceding actor-selector campaign.
All six required semantic views were read with `--image game`, without hiding
candidate references. Source history back to the original actor reconstruction
and both adjacent functions were inspected.

- A three-block leaf with no frame, calls, strings, relocation entries,
  absolute globals or indirect transfers. It reads the shared `KfActor`
  unsigned phase at +`0x12` once using `lhu`, then uses that value in both
  comparisons. The first comparison uses `sltu` after narrowing the input to
  16 bits. The input narrowing fills the phase load-delay slot.
- The first failure branches directly to the common return, initializing the
  false result in its delay slot. The success path reads signed step +`0x3c`
  with `lh`; a load-delay `nop` precedes full-width subtraction of the step
  from the cached phase. The lower-bound comparison is signed, with no
  halfword wrap of the subtraction. The result is inverted with `xori 1`.
  The final `jr` delay slot moves that boolean result into `$v0`.
- The adjacent exact `actor_advance_animation_clamped` writes both fields,
  and the exact `actor_play_sound_at_phase` consumes this predicate. Their
  retail bodies and the shared field declarations were read; no object
  layout change is justified.
- All **11 proven call sites** pass the current actor pointer in `$a0`, a
  halfword phase or bounded constant in `$a1`, and test `$v0` for zero/nonzero:

| Caller | Site(s) | Phase evidence |
| --- | --- | --- |
| `actor_play_sound_at_phase` | `8002dd1c` | incoming phase masked with `0xffff` |
| `actor_update_effect_action` | `8002f4d4` | `lhu` from definition `action_animation_phases[action+8]` |
| `actor_update_current_action` | `8002fd24`, `800300f4`, `800304cc` | `0x800`, `0xd48`, `0x4b0` |
| `actor_update_current_action` | `80030574`, `80030588`, `8003059c`, `800305b0` | `0x8c0`, `0xa80`, `0xc80`, `0xe00` |
| `actor_update_current_action` | `800305d4`, `800305f8` | `0xdac`, `0xed8` |

The dispatcher has unresolved indirect switch control in its own inventory;
the direct calls above are decoded/proven independently. This campaign does
not promote its indirect paths or change the dispatcher.

Vendor negative control: no target row appears in the vendored or supplied
Psy-Q FID census; supplied SDK headers have no matching actor/phase API. The
field pair and action/effect/audio caller policy identify a game-owned actor
operation, not a libc/GTE wrapper. The census still independently identifies
downstream `SquareRoot0` and `catan` in LIBGTE. No library body is reconstructed.

The existing source repeats the phase field expression, and the compiled
body swaps the cached-phase/result roles (`a2`/`v1`) relative to retail while
preserving the instructions and CFG. The first source hypothesis is an
explicit `u16 current_phase = actor->animation_phase` local before the false
result initializer, reused by both comparisons. This exposes the single
observed field read without fake carriers or changed arithmetic. Keep the
word return and `u16 phase` interface, predicates, loads and delay slots;
require all 12 words and strict 100% before closure. If unchanged, record
the residue instead of permuting unrelated locals or forcing registers.

## Verdict

The explicit `u16` phase local emitted exactly the same body as the starting
source: the six operand differences remain the cached-phase/result register
swap. No call, referent, branch, signedness, immediate or delay-slot change
appeared. The trial was reverted; the original source and interface are kept.
This function remains **97.5%**, not exact and not newly banked. No compiler
mechanism is attributed. A future attempt needs an independent source fact,
not another spelling of the same cache or a register-forcing carrier.

The restored actor unit was rebuilt during the following effect-owner
campaign. Canonical strict reports still give this predicate 97.5%, and all
23 exact actor functions remain exact. Full build/check and the existing 551
tests, lint and flake checks ran with that restored source. The full build's
remaining failures concern the documented data/ownership/relink work, not a
new actor-function regression.

## Return-form control at `2f1f04a`

Refresh all six GAME-qualified views, the complete twelve retail words, both
adjacent boundaries, and the argument/result windows at all eleven callers.
The current strict score remains 97.5%; all 25 exact actor siblings are
controls. Retail loads phase into v1 and initializes the false result in a2
in the first branch's delay slot. Canonical source exchanges those two roles
at six instructions. There are three blocks, one branch, one return, and no
calls, strings, relocations or unresolved control in this game-owned leaf.
The shared unsigned phase and signed step fields remain at +0x12 and +0x3c;
callers still pass a halfword threshold and consume a word zero/nonzero result.

Function Match Plan: express the decoded failure edge directly as
`if (phase >= actor->animation_phase) return 0;`, followed by the signed
lower-bound comparison as the return expression. This removes the explicit
result initializer and assignment without changing either predicate, the
short-circuit step load, or the interface. It tests a return-form hypothesis,
not a register-forcing local or declaration permutation. Compare the first
raw divergence and all twelve words; revert if it does not explain retail.
Keep the already rejected explicit-phase-cache control out of this experiment.

The direct early-return control recovers retail's v1 phase operand, but emits
60 bytes, a fourth block and a new internal jump; its branch delay slot is
empty instead of initializing the shared boolean result. Reject it. Retail's
single false-result definition and conditional lower-bound evaluation instead
support a short-circuit boolean return expression. Test the conjunction of
the same two comparisons: it must retain the first guard's conditional step
load and produce the common result/return without the extra exit block.

### Exact short-circuit result

The conjunction emits all twelve retail words and reaches strict **100%**,
up from 97.5%. It retains the same three blocks, conditional signed-step load,
one return and both delay slots, with no relocations or extra jump. The
false-result initialization, phase operand and returned boolean now use the
retail registers naturally. The retained source changes only the function's
return expression; shared types, input widths and compiler profile are unchanged.
This establishes the successful source form, not a historical optimizer cause.

A separate fresh compilation compares all 26 exact actor bodies directly
against the hash-verified GAME bytes, including the new predicate's 48 bytes
with SHA-256 `f4c3f9cd9ea09ec8503c78b1b8c43f931201524d1ab7fe8ce88c850149d7a1ac`.
Each target object first relinks back to retail, and every ordered call and
address pair agrees. A second fresh compilation substitutes the committed
`2f1f04a` player header for the concurrent progression-naming edit: the complete
actor text and all non-debug relocation identities/addends remain identical.
The exact result does not depend on that unrelated header change.

Only this predicate changes among all 484 scored rows. GAME advances
308 -> 309/362 exact, OPEN remains 98/108 and PSX 1/1; total 408/471.
The full build retains the existing data/placement/ownership failures:
source data PSX 0/1, GAME 9/42, OPEN 2/19; target relinks 1/1, 75/77 and
34/38 respectively, with six conflicting-section cases and zero artifact
failures. No other function score or data claim is changed by this campaign.

Ruff, `git diff --check`, and all 678 repository tests pass with the final
short-circuit source (78.336 seconds). No tooling or flake change is involved.

After staging only the matching campaign, selected banking of `GAME:8002dccc`
correctly refuses three independently modified player-naming build inputs.
Keep the verified reconstruction without a dirty override; its one exact
ledger update remains pending that campaign's completion. The four earlier
menu results were separately banked while inputs were clean in `0f00593`.
