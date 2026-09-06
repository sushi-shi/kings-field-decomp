# GAME player damage labels

## Function Match Plan

Starting from 7f9e858, replace the numbered player attack/defense fields and
armor defense contributions with names supported by the retail menu pixels
and their numeric rows. Keep the shared KfPlayerState and KfArmorRecord
owners, every u16 field, all offsets, statement order, literals and referents.
No array layout, function signature, generic damage argument or status bit
changes in this pass. There is no new exact result to bank.

Fresh GAME-qualified six-view snapshots and source history cover all five
affected functions. Four bodies have exactly the same retail instruction
lines as the previously reviewed poison campaign; the weapon update adds
its complete CFG, caller, callees and O32 attack payload. The surrounding
player-combat, equipment and menu dossiers supply the shared ownership and
adjacent-function context. These game stat and UI consumers are not vendored
routines; the weapon update keeps RotMatrix and ApplyMatrix as SDK calls.

| GAME address | Function / extent | Starting strict score | Field evidence |
| --- | --- | --- | --- |
| 800151cc | game_state_initialize / 740 B | 100% | Seeds all five player defense fields to 5. |
| 80015714 | player_recalculate_combat_stats / 2068 B | 100% | Clears ten fields, accumulates five weapon components and five armor-slot contributions, then applies existing accessory/status bonuses. |
| 80016324 | player_apply_damage / 912 B | 100% | Loads the five unsigned defenses into the corresponding component calls. |
| 80016bc0 | player_update_weapon_attack / 612 B | 100% | Passes the five unsigned player attack fields to actor_apply_damage without reordering their component positions. |
| 800264d8 | menu_draw_status_details / 3252 B | 95.110700% | Prints decoded labels and their corresponding halfwords on matching Y coordinates. |

Rebuild the three affected units and require every non-debug section of all
112 objects, all 484 function scores and the complete strict report to remain
unchanged. Independently compare all words and ordered calls/data references
of the four exact bodies against retail. Normalize only the fifteen declared
field-name substitutions when checking source equivalence. Curated field
rows may change names and evidence, preserving their types, offsets, sizes
and ordering. Update the existing inventory expectations; run repository
checks and the full build before commit.

## Retail label and field correspondence

The [menu atlas evidence](game-menu-glyph-render.md#retail-glyph-format-and-asset-evidence)
records the hash-checked MIX.TIM page and palette, STAT.DAT descriptors,
glyph geometry and decoder. Applying that same decoder to the actual glyph
indices below gives the Japanese labels; these are not inferred from an
English guide or another release. The text renderer ignores the leading
blank glyph. Hexadecimal glyph indices are authored asset coordinates.

The player object starts at GAME 800a0780. All entries below are u16.

| Meaning | Glyph indices / decoded label | Player attack offset / name | Player defense offset / name | Armor contribution offset / name |
| --- | --- | --- | --- | --- |
| Cutting | d1 6a / 切る | +30 cutting_attack | +3c cutting_defense | +02 cutting_defense |
| Striking | d2 51 / 打つ | +32 striking_attack | +3e striking_defense | +04 striking_defense |
| Piercing | d3 4c / 刺す | +34 piercing_attack | +40 piercing_defense | +06 piercing_defense |
| Fourth component; labels differ | bf 58 78 79 / 聖の魔法 (attack); 78 58 78 79 / 魔の魔法 (defense) | +36 holy_attack | +44 magic_defense | +0a magic_defense |
| Fire magic | d4 58 78 79 / 炎の魔法 | +38 fire_attack | +46 fire_defense | +0c fire_defense |

Attack label/value rows both use Y=44,58,72,86,100. Defense rows both use
Y=130,144,158,186,200; the already identified poison resistance occupies
Y=172 between piercing and magic defense. This proves the field-to-label
mapping rather than merely recognizing a collection of words on the atlas.

The defense label's repeated magic glyph is present in retail, not a source
transcription fix: 80026d84 loads s0=0x78, with no subsequent write to s0
until the epilogue. At 80026ebc/80026ec4/80026ec8/80026ed0, the four label
halfwords become 78/58/78/79; 80026ed8 stores the -1 terminator. The draw call
at 80026ef0 owns the next Y advance in its delay slot. Do not silently replace
this label with the attack's 聖 or infer a dark/evil element from it.
`magic_defense` identifies this particular displayed component, not a combined
defense against every spell. Fire defense is a separate field.

At 80016d00..80016d48, the weapon update loads attack offsets 38,34,36,30,32
with lhu while assembling actor_apply_damage's argument order: actor index,
physical power, cutting, striking, piercing, holy, fire, charge, flags. The
piercing/holy/fire values occupy stack slots 16/20/24; holy's store is the
call delay slot. Thus the UI fields are actual damage components, not only
presentation totals. The update's 120-byte frame, five calls, three internal
jump relocations and final stack-release delay slot remain unchanged.

Combat-stat recalculation reads weapon attack_components[0..4] into those
five player fields. For shield, head, arm, leg and body armor it reads the
five documented armor offsets into the corresponding defenses. Each armor
slot adds cutting defense twice, with the same intervening halfword stores
and loads as retail. Preserve both additions and their narrowing behavior;
this naming evidence does not explain the original reason for that weighting.
The separate accessory switch continues to add its existing holy-attack,
magic-defense and fire-defense bonuses. Incoming player damage reads the
same defense fields in component order. The generic actor/player damage
payloads still require contextual evidence before giving their fourth
arguments one universal elemental name.

## Numeric limits

These names recover what the values measure, not why their magnitudes were
chosen. New-game defense 5, equipment/accessory bonus magnitudes and the
doubled armor cutting contribution retain their existing tuning; the original
design rationale is unknown. Zero accumulation seeds and empty-component
checks keep their arithmetic meaning. The damage formula's tenfold scale,
rounding +5 and division by 10 retain their existing tenths interpretation.
No new constant alias or designer-intent claim is introduced here.

## Final verdict

All five function scores in the plan remain unchanged, as do the other 479
rows and the complete strict report. All sections of all 112 objects agree
with 7f9e858, including debug line tables. The source-value control preserves
every statement, literal, type and width across the three C sources, two
shared headers and existing inventory test after only the fifteen field-name
substitutions. Exactly fifteen curated field rows change their names/evidence;
all offsets, sizes, types and row order are preserved.

Independent raw controls reproduce all 1,083 words of the four exact bodies:
weapon update 153, initialization 185, stat recalculation 517 and damage
application 228. Their ordered call/data-reference counts remain 5/28,
3/77, 3/130 and 6/24 respectively. The non-exact menu retains every source
instruction and relocation. Its fourth defense label remains the retail
78/58/78/79 sequence. The older source-shape note now correctly identifies
the repeated armor load as `lhu v1,2(a0)` at 800158b8/800158cc, not `lbu`.

Ruff and diff checks pass. All 651 repository tests pass in 85.294 seconds
with nine skips. Full `kf build` still fails on the existing data ownership
and placement findings: source-data matches 7/60 units, target relink is
PSX 1/1, GAME 75/77 and OPEN 34/38, with two GAME and four OPEN section-base
conflicts and zero artifact failures. Post-full-build section, score, raw,
source-value and field-inventory controls pass. No new result is banked;
no tooling implementation or flake changes.
