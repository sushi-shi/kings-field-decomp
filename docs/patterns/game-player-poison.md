# GAME player poison status

## Function Match Plan

Name the player and equipment poison fields from the retail menu's `毒`
glyph and the complete damage/timer/display chain. GAME status bit 2 is mask
0x4. The existing shared structures and aggregate ownership remain unchanged:
`KfPlayerState` at 800a0780 owns resistance +0x42 and timer +0x4c;
`KfArmorRecord` owns its resistance contribution at +0x08. The resistance
fields are unsigned halfwords, and the timer is a signed halfword with -1
inactive. Replace `status_effect2_resistance` with `poison_resistance` and
`status_effect2_timer` with `poison_timer` in shared declarations, source and
the curated field inventory.

The campaign starts from 3d7761c. Refreshed image-qualified disassembly/CFG,
incoming/outgoing references, strings, match state and source-history evidence
are captured for each affected function. Earlier player-combat, update and
menu dossiers supply the surrounding functions, caller ABI and SDK boundary
checks. The game status object, equipment records and menu glyphs establish
game ownership; the `rand` implementation remains a vendored boundary.

| GAME address | Function | Starting strict score | Evidence in this campaign |
| --- | --- | --- | --- |
| 800151cc | game_state_initialize | 100% | Seeds resistance 5 and timer -1; preserves the existing initialization order. |
| 80015714 | player_recalculate_combat_stats | 100% | Rebuilds resistance from zero using the head, body, arm, leg and shield armor records. |
| 80016324 | player_apply_damage | 100% | Tests incoming mask 4, compares unsigned resistance with the random bucket, then writes timer 600 and sets the same bit. |
| 80018880 | player_update | 96.945540% | Signed timer countdown, mask clearing, modulo-20 damage and two-update color flash. |
| 8001fde4 | render_frame | 100% | The copied status flags' mask 4 enables the poison HUD sprite in the existing priority chain. |
| 80025f38 | menu_draw_stats_header | 97.686110% | Mask 4 inserts glyph 0x88 in the status row. |
| 800264d8 | menu_draw_status_details | 95.110700% | The same status glyph; a separate `毒` label aligns with the resistance value. |

Keep all constants' values, widths, CFGs, delay slots, data identities and
ordered references. Introduce names for the mask, inactive timer, initial
duration, damage interval and flash width. Rebuild affected objects and
require all non-debug sections and all 484 strict scores to remain unchanged.
If type-name metadata changes, normalize only the two documented field-name
substitutions when comparing decoded DWARF. No new result is banked.

## Direct evidence and limits

The [retail menu atlas](game-menu-glyph-render.md#retail-glyph-format-and-asset-evidence)
identifies text glyph 0x88 as `毒`, poison. Both status renderers select that
glyph for mask 4. The detailed panel also prints `poison_resistance` in the
matching defense row. This supplies the semantic witness missing from the
original numbered field names.

At 8001640c..80016458, retail reads `player_state + 0x42` with `lhu`, calls
`rand`, multiplies its result by 100, arithmetically shifts by 15 and uses
strict `slt`. A successful comparison stores 600 to +0x4c and sets mask 4
at +0x2a. This is `resistance < ((rand() * 100) >> 15)`, not an inclusive
comparison or a direct percentage-success promise. The record contribution
and the accumulated player value therefore retain unsigned 16-bit widths.

The pinned SDK's `RAND.H` defines `RAND_MAX` as 32767. Under that API
contract, the scale/shift yields buckets 0..99; resistance 0 still rejects
bucket 0, while resistance 99 or higher rejects every bucket. Retail `rand`
at 8005049c is only a three-instruction BIOS dispatch stub: it loads 0xa0,
jumps through that register, and sets selector 0x2f in the delay slot. The
RNG implementation and bucket probabilities are not reconstructed here.

At 8001a07c..8001a13c, retail loads the timer with `lh`, skips -1, tests
mask 4, decrements and stores the halfword, explicitly sign-extends it, and
checks for -1 before the signed remainder by 20. It narrows the remainder
to a halfword, uses `sltiu ...,2` for the color flash and calls
`player_adjust_hp(-1)` only at remainder zero. The HP argument occupies the
call's delay slot. Clearing the status externally resets the timer to -1;
normal expiry clears mask 4 with 0xfffb while retaining the other flag bits.

From initial timer 600, uninterrupted processing flashes on updates 19/20,
39/40, and so on, and subtracts one HP on updates 20,40,...,600. There are
30 damage ticks. The following update reaches -1 and clears the flag.
These are executions of the status-processing block, not a claim about
seconds or presentation frame rate. Reapplying poison can reset the timer,
and curing it can end the sequence early. The original tuning rationale for
600, 20 and the two-update flash is not recovered.

The values retained literally in the poison paths have these roles:

| Value | Reason |
| --- | --- |
| Initial resistance 5 | Existing new-game seed, separate from combat-stat recalculation, which rebuilds the value from zero. Its tuning rationale is unknown. |
| Recalculation seed 0 | Start the sum before the five equipped contributions. |
| Random scale 100 and shift 15 | Convert the SDK random result to the existing integer bucket scale; preserve the strict comparison and its boundary behavior. |
| Remainder comparison 0 | Select the damage update within the periodic flash. |
| HP delta -1 | Lose exactly one HP per damage tick. |
| UI aggregate divisor 5 | The aggregate-defense display weights poison resistance by one fifth before combining it with the five defense components. This display rule does not alter resistance or incoming damage; its design rationale is unknown. |
| Text glyph 0x88 | Authored character index for `毒`, independently decoded from the retail texture. |

Composite cure masks retain their exact existing values. In particular,
the item-use mask 0xb retains bits 0,1,3 and clears every other bit; replacing
it with the complement of the poison bit would change higher-bit behavior.
The mask 3 also clears poison but has a different retained set. Those item
and spell policies are not rewritten by this field-naming campaign.

## Final verdict

All seven function scores in the plan remain unchanged, as do the other
477 rows and the complete strict report. Every non-debug section of all
112 objects agrees with 3d7761c; only `player_update`'s debug line table
differs. No DWARF field-name exception was needed. Normalizing the two field
spellings and five constants preserves all statements, types, widths and
values in the five changed C sources and two shared headers. The curated
inventory changes exactly three names and their evidence, with every type,
offset, extent and row order preserved.

Direct retail controls retain all 185 initialization words, 517 combat-stat
words, 228 damage-application words and 326 frame-render words. Their ordered
call/data-reference counts are respectively 3/77, 3/130, 6/24 and 20/64;
all agree with the delinked target and physical retail referents. The partial
player-update and menu bodies preserve every source instruction and relocation.

Ruff and diff checks pass. The existing inventory test expects the new field
spellings while retaining its original type/offset checks. All 651 repository
tests pass in 87.439 seconds with nine skips. Full `kf build` still fails on
the existing ownership/placement findings: source-data matches 6/60 units,
target relink verifies PSX 1/1, GAME 75/77 and OPEN 34/38, and artifact failures
remain zero. The two GAME and four OPEN conflicting-section-base findings are
unchanged. Post-test raw-word, section, source-value and inventory checks pass.
No result is newly banked, and no tooling implementation or flake changed.

The subsequent [remaining-status campaign](game-player-statuses.md) shares
poison's former `KF_POISON_TIMER_INACTIVE` spelling as
`KF_PLAYER_STATUS_TIMER_INACTIVE`. The value stays -1 and every poison
countdown/zero-tick instruction remains unchanged.
