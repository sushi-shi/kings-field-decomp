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
