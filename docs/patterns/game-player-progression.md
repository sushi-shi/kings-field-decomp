# Player progression limits and growth records

## Function Match Plan

At `2f1f04a`, name the shared power, vital, experience and level caps, the
training threshold, the runtime growth-table count, and the three base-magic
learning thresholds. Propagate the training threshold into Verdite's existing
addition. Use the runtime count in the table declaration, the extrapolation
boundary and the final/penultimate row expressions. Keep the exclusive clamp
tests as inclusive maximum plus one; do not change comparison operators,
integer widths, store/reload order, callbacks, or delay slots.
Reuse the existing `KF_AUDIO_MAX_VOLUME` for the reviewed level-up cue's
127 argument, retaining its call-delay-slot value and sound-reference row.

The complete six-view GAME snapshots, neighboring bodies, history, 112 objects
and 484 strict scores are captured under
`build/constant-names/player-progression/`. GAME is selected explicitly and
`kf init` validates the retail images. These functions implement player state,
equipment, progression and resource policy, not a vendored library; SDK calls
remain external. The confirmed actor-damage callers distinguish qualifying
magic/physical training from the signed-halfword experience reward on death.
Equipment changes and player updates also invoke stat recalculation.

| GAME address | Function | Bytes | Strict match (before = after) | Evidence and hypothesis |
| --- | --- | --- | --- | --- |
| `80015714` | `player_recalculate_combat_stats` | 2068 | 100% | Halfword base/derived powers; 37/70/75 base-magic tests precede learned-state stores; cap tests 1000 and stores 999. Preserve all weapon/armor/accessory operations. |
| `80015f28` | `player_increment_physical_power_training` | 152 | 100% | Halfword increment/store/reload, compare 100, raise base power, clear training in cap-branch delay slot; notification only below overflow boundary. |
| `80015fc0` | `player_increment_magic_training` | 152 | 100% | Same counter/cap structure for magic; called from actor damage and Verdite use. |
| `80016058` | `player_add_experience` | 548 | 88.824814% | Signed s16 reward and signed word total; byte level capped at 255; runtime rows 39/38 supply extrapolation; halfword growth stores precede cap checks. |
| `80018054` | `player_use_item` | 1116 | 100% | Verdite adds 100 at 80018464; store at 8001846c is the training-call delay slot. Preserve all switch arms, probes and callbacks. |
| `800151cc` | `game_state_initialize` | 740 | 100% | Header control: seeds initial HP/MP, powers and experience threshold from runtime growth row zero. |
| `8001b180` | `common_resources_load` | 528 | 100% | Header control: source begins at chunk +4 and endpoint is chunk +484; copies exactly 480 bytes, not the complete 600-byte resource chunk. |

The shipped `KF/COM/COM.DAT` contains fifty growth records, but the loader
copies only forty. The new count therefore describes the runtime table, not
the file's complete record count. Keep the remaining records outside the
loaded owner. Check the terminal rows directly before choosing their names.

Force the three affected units, inspect the first comparison divergence,
compare all object sections and scores, and independently resolve each reviewed
function's words/calls/addresses. Exact rows must still reproduce retail.
Run modern checking, inventory validation, Ruff, the existing tests, whitespace
checks and full `kf build`. Document every retained literal in the four
progression/stat functions and refresh the existing item-use ledger. Do not
add tests or size assertions, or claim to close the partial experience routine.

## What the values mean

The named maxima identify the encoded upper-bound checks: 999 for base and
derived physical power/magic, 9999 for maximum HP/MP, 99999 for accumulated
experience and 255 for the level byte's wrap-prevention guard. These are
different domains. They do not turn every arithmetic update into a saturating
operation: retail stores halfword results before reloading and comparing them,
and experience additions retain their signed s16 input and signed total.
`maximum + 1` expresses each original exclusive comparison boundary.

Each training helper increments its own unsigned halfword, tests the reloaded
value against 100, raises the corresponding base stat once, and resets training
to zero. Excess points are discarded. If the reloaded stat reaches the 1000
boundary it is clamped to 999 without the increase notification; both paths
then recalculate combat stats. The reset remains the cap-branch delay-slot
store at `80015f88` / `80016020`. Verdite first adds a full threshold and stores
it in the helper-call delay slot; the helper still performs its own increment.
No carry-preserving or repeated-gain loop replaces that behavior.

The three named learning thresholds test **base magic**, not equipment-adjusted
magic. Dispoison requires 37 and already learned Healing. Fire Wall requires
70 and Lightning Bolt 75. Each target must still be unlearned, and each newly
set learning byte queues the existing notification independently. Accessory
bonuses do not substitute for those base-magic requirements. The original
balance rationale for these magnitudes remains unknown.

The 7380-byte `KF/COM/COM.DAT` has SHA-256
`8fea86564bf5927f00156edf3163f3d008f86289103dd076746fad7a8922e329`.
Its final chunk header at `0x1a78` gives a 600-byte payload: fifty twelve-byte
records starting at `0x1a7c`. The runtime copy uses only 480 bytes at GAME
`800650b0..80065290`, supported both by its raw copy endpoint and the existing
data inventory. This independently confirms the distinction already noted in
the [resource-parser coverage](../game-resource-parser-coverage.md).

| Zero-based resource row | HP | MP | Physical-power step | Magic step | Next-level experience threshold |
| --- | --- | --- | --- | --- | --- |
| 0 | 30 | 20 | 20 | 20 | 50 |
| 38 | 749 | 490 | 4 | 4 | 69404 |
| 39 | 777 | 508 | 4 | 4 | 72904 |
| 40, not loaded | 805 | 526 | 4 | 4 | 76404 |

The new runtime count is forty. Count minus one and count minus two select
loaded rows 39 and 38. From a previous level of forty onward, retail adds
their HP/MP differences (28/18) and experience-threshold difference (3500),
but uses the final row's power increments (4/4) directly. It never begins
indexing the extra resource rows; rows 41..49 even contain experience thresholds
of 32767, which the observed progression path does not load. No rationale for
the unused records is inferred. Level 255 names a storage guard, not a claim
that the normal experience progression can reach that level.

The [complete four-function ledger](game-player-progression-literal-ledger.md)
records the remaining zeros, last-row index arithmetic, clamp-boundary ones,
weapon component positions, sound-reference position and accessory tuning.
The named destination or owning item explains each retained tuning literal;
equal-valued bonuses do not become a shared rule. All armor slots retain the
twice-added cutting-defense component and its intermediate narrowing stores.

## Final verdict

All seven snapshot functions have the final verdict **unchanged**. Independently
resolved candidate instructions preserve 1321 words, 43 ordered calls and 245
address references. The six exact functions reproduce all 1189 retail words
and the independently resolved target objects. `player_add_experience` remains
88.824814%, with 132 candidate words versus 137 retail words. Its first retail
divergence remains `8001605c`: `sw ra,28(sp)` instead of `sw ra,24(sp)`.
The existing growth-address and scheduling differences are not newly attributed
to a compiler mechanism, nor repaired by altering the progression semantics.

An isolated `2f1f04a` build and a second build with only these three source/header
edits agree in every section of all 112 objects except `game.player_death`'s
debug line table. The live build's runtime sections, symbols and relocations
agree with the isolated named result outside the independently edited actor
unit. That frozen actor source also compiles identically with both header
versions. Among all 484 captured scores, the sole change belongs to the
concurrent phase-predicate reconstruction subsequently committed as `258f6ad`;
none belongs to this naming campaign.

All three affected units were forcibly compiled. Inventory validation, Ruff,
all 678 existing tests (79.126 seconds), and whitespace checks pass. Modern
checking retains exactly the preceding 320 error diagnostics and 64/112 passing
source/image variants. Full `kf build` was run after the final source changes;
it retains the existing data/placement failures: data PSX 0/1, GAME 9/42,
OPEN 2/19; target relinks 1/1, 75/77 and 34/38; six conflicting section bases,
zero artifact failures. No new tests, size assertions or banked matches are
part of this change.

The four-function census drops from 63 to 45 retained numeric occurrences;
the [item-use ledger](game-item-use-literal-ledger.md) drops from 80 to 79 after
the shared Verdite threshold. The startup ledger's source locations are
refreshed without changing its 98 retained startup occurrences or their reasons.
Overall this replaces 35 inline C numeric uses with named expressions, while
retaining sixteen explicit ones/twos that derive exclusive limits and terminal
indices: a net reduction of nineteen inline occurrences. The runtime table's
header declaration also uses the named count. Wider player and source audits
remain open.
