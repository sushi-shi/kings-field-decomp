# GAME spell identities and retained casting literals

## Function Match Plan

Start at `00e992a` on master with hash-verified Japanese retail. Use the project
matcher workflow and explicit GAME semantic queries. Six-view dossiers, source
history, adjacent inventory and decoded labels are under
`build/constant-names/spell-identities/`. All selected units use
`probe-gcc257-o2-g0`; this remains a probe, not compiler attribution.

Decode all nine spell rows and propagate names through selection, learning,
casting, weapon magic, and the matching effect kinds. Distinguish the 24 runtime
records from the nine displayed spells. Preserve the existing ground-branch
effect identity while identifying player spell 6 as Fire Wall. Replace exact
instant-spell keep-masks with the existing status bits, without changing which
bits survive. Document authored thresholds and geometry; do not invent tuning
rationale. Complete a retained-literal ledger for magic.c and refresh the
equipment and map-script ledgers affected by named spell IDs.

These game-state/menu/spell policies are absent from the vendored roster.
Psy-Q matrix, trigonometry, audio and input functions remain external SDK
boundaries; no library bodies are reconstructed. Before commit, force affected
compilation, compare all 112 object sections/all 484 scores/full report, check
exact controls against raw retail words and ordered referents, run modern
compilation, inventory validation, ruff, tests, whitespace and full kf build.
No new size assertions, per-spell tests or banking claims.

## Evidence snapshots

| GAME address / bytes | Function | Starting strict % | Constraint |
| --- | --- | --- | --- |
| `0x80020cfc / 1500` | `item_load_database` | 99.746666 | Control: STAT.DAT spell rows immediately follow eighty 20-byte item rows; nine rows at offset 0x1318, names and runtime records share menu indices. |
| `0x8002317c / 1328` | `menu_magic_panel` | 95.89759 | Instant IDs 0..3; learned byte must equal 1. Preserve MP debit, HP halfword stores/cap and exact keep-masks 3/12; panel result also represents cancellation. |
| `0x80023e9c / 1136` | `menu_spell_select` | 98.06338 | Ranged IDs 4..8, ten-halfword name stride; selected ID remains byte storage, none=255, selection result remains signed. |
| `0x8002718c / 2104` | `menu_draw_name_list` | 100.0 | Control: selected byte indexes the nine-row spell bank, excluding none=255. |
| `0x800151cc / 740` | `game_state_initialize` | 100.0 | Starting selected spell ID8 is Light Needle; keep all stock and state initialization order. |
| `0x80015714 / 2068` | `player_recalculate_combat_stats` | 100.0 | Dispoison learns at base magic37 with Healing known; Fire Wall at70, Lightning Bolt at75. Keep learned-byte conditions and notification calls. |
| `0x800167e4 / 100` | `player_select_magic` | 100.0 | Control: low-byte O32 input stored at player+5c; none clears pointer; other IDs index 20-byte magic records. |
| `0x80018880 / 6684` | `player_update` | 96.94554 | Wind Cutter receives bracelet MP-cost halving; weapon magic uses Fire Ball/Light Needle records and corresponding effect kinds. Preserve unrelated kinds20/36 and record18. |
| `0x8003425c / 136` | `map_ambient_script_floor3` | 100.0 | Restoration region grants Resist Fire and Bless together; keep X15..17/Z64 and existing learned-byte checks. |
| `0x80034610 / 144` | `map_action_script_floor3` | 100.0 | Bracelet possession grants Wind Cutter; stage3 dialogue grants Fire Ball; preserve notification and progress referents. |
| `0x80034a80 / 724` | `map_event_interact` | 100.0 | Mirror of Truth exchange grants Healing; unrelated character, item, page, stage and link identities unchanged. |
| `0x8003a274 / 44` | `magic_load_records` | 100.0 | Control: copies all24 records, distinct from the nine displayed player spell names. |
| `0x8003a2a0 / 1216` | `magic_cast` | 98.89145 | Selected spells4..8 pass their IDs unchanged to effect kind. Name branches and document Lightning Bolt lifetime/aim, Wind Cutter speed, Light Needle rotation pointer and Fire Wall ground spawning. |
| `0x80036f44 / 2092` | `effect_pool_construct` | 100.0 | Kinds4/5/7/8 correspond directly to named spells. Preserve kind23 normalization to4, render IDs, variadic slots, and Lightning Bolt record sound reads. |
| `0x8003781c / 52` | `effect_pool_set_current` | 100.0 | Control: runtime effect kind indexes the broader record table; do not narrow this to nine spells. |
| `0x80038a38 / 6156` | `effect_update_dispatch` | 96.93957 | Name only kinds4/5/7/8 and record4 reads. Preserve all phase/render IDs and damage components; ground branch6 remains its established behavioral name. |

## Retail labels and domain boundaries

The loader copies nine 20-byte name rows from `COM/STAT.DAT` at
`0x1318 + 20*i`, immediately after the eighty item rows. File length is 5708
bytes and SHA-256 is
`3f51069ac6291bffdfeb981b14963a22564b40fa5d9034f226797f84247b97f4`.
The glyph atlas comes from `COM/MIX.TIM`, SHA-256
`9e1031c32ea9efc2d124ae97dd6f2a0291bc88153b74b8c1d57f99abefd55959`.
The [equipment-label audit](game-item-equipment-identities.md) records the
TIM offsets, palette, nibble order, glyph geometry and diacritic overlays.
This campaign's `labels.py`, `labels.json` and inspected `labels.png` preserve
the nine-row decoding under the ignored evidence directory.

| ID | Retail label | Source constant | Consumer |
| --- | --- | --- | --- |
| 0 | ヒーリング | `KF_MAGIC_HEALING` | Instant HP recovery; Mirror of Truth exchange grants learning. |
| 1 | ディスポイゾン | `KF_MAGIC_DISPOISON` | Instant status keep-mask3. |
| 2 | レジストファイア | `KF_MAGIC_RESIST_FIRE` | Instant fire-defense boost. |
| 3 | ブレス | `KF_MAGIC_BLESS` | Instant status keep-mask12 and triple-magic HP recovery. |
| 4 | ライトニングボルト | `KF_MAGIC_LIGHTNING_BOLT` | Aimed projectile with lifetime and ground impact effects. |
| 5 | ファイアーボール | `KF_MAGIC_FIRE_BALL` | Projectile; Flame Sword uses this spell record and effect. |
| 6 | ファイアーウォール | `KF_MAGIC_FIRE_WALL` | Ground-branch effect rooted at the target or projected floor position. |
| 7 | ウィンドカッター | `KF_MAGIC_WIND_CUTTER` | Projectile; bracelet grants learning and halves its MP cost. |
| 8 | ライトニードル | `KF_MAGIC_LIGHT_NEEDLE` | Rotation-oriented projectile; starting selection and Colichemarde weapon magic. |

Names transcribe/translate the Japanese labels rather than asserting an
official English localization. `KF_MAGIC_PLAYER_COUNT` names the nine-row
extent in the declaration, definition and ranged-selection bound. The menu
visits Healing through Bless as instant spells and Lightning Bolt through
Light Needle as ranged spells. It requires learned byte exactly 1; other
nonzero bytes are not silently accepted.

The runtime table still contains 24 records. `magic_cast` passes selected
IDs 4..8 unchanged as the constructor's kind, and the constructor indexes
20-byte records using that kind. Thus four new effect-kind constants refer
to the corresponding spell constants. The existing behavioral identity
`KF_EFFECT_KIND_GROUND_BRANCH` now explicitly equals `KF_MAGIC_FIRE_WALL`.
This does not turn other effect kinds, render IDs, animation clips or sound
indices into player spell IDs. In particular, record 18 and record 20 keep
their existing effect identities, and kind 23 still normalizes to the
Lightning Bolt runtime kind while selecting a different render base.

Player spell storage and the selector parameter remain bytes; generic menu
selection/results remain signed integers because cancellation and pending
values share those paths. These names do not claim that enum storage/type
propagation through every spell/effect boundary is finished.

## Exact instant-spell policy

The panel debits the selected record's MP cost before applying its effect;
insufficient MP returns the selected ID without casting. Healing adds the
current magic stat to HP. Bless adds three times that stat. Both preserve
the retail halfword store before the maximum-HP comparison: the change does
not introduce a wider saturating addition. The original multiplier choice
for Bless is unknown.

Dispoison's raw `andi 3` at GAME `0x800235cc` becomes
`KF_PLAYER_STATUS_CURSE | KF_PLAYER_STATUS_DARKNESS`. Bless's `andi 12`
at `0x80023630` becomes `KF_PLAYER_STATUS_POISON | KF_PLAYER_STATUS_SLOWED`.
These are masks of bits to keep, not just names for bits to clear. Both clear
the fire-defense boost bit and every higher bit. Their timer fields remain
untouched. Resist Fire sets the existing boost bit and calls the existing
boost helper. No mask is broadened to fit the spell's apparent intention.

The threshold-learning code independently establishes Dispoison at base
magic ≥37 if Healing is known, Fire Wall at ≥70, and Lightning Bolt at ≥75.
Each path preserves its prior learned-byte condition and notification.
These are authored progression thresholds; the original choices are unknown.
The floor 3 restoration region grants Resist Fire and Bless together. Its
bracelet-possession path grants Wind Cutter, while dialogue stage 3 grants
Fire Ball. Mirror of Truth grants Healing through the existing exchange.

The Wind Blade Bracelet's equip check now reads as a Wind Cutter cost
discount, using the same right shift by one and floor rounding. Flame Sword
requires Fire Ball learned and uses that record/effect pair. Colichemarde
uses Light Needle's pair after its existing stat checks. Moonlight Sword's
separate kind 36/record 18 pair and Triple Fang's kind 20/record 20 pair remain.

## Casting geometry and retained literals

The shared launch offset is (-200,200,400) world units before camera rotation.
Target search uses a 20000-world-unit range and the existing named aim
tolerance. Fire Ball/Light Needle displacement scales are 600; Lightning Bolt
and Wind Cutter use 800 world units per active movement update, subject to
the existing fixed-point direction rounding. These are observed speeds,
not explanations for why the original author chose them.

Untargeted Lightning Bolt uses pitch -128 angle units (-11.25°) and a
twenty-update countdown. Targeted casting replaces the scratch distance with
integer `distance / 800` before passing it as the countdown. The constructor
stores a halfword; the update decrements and tests the narrowed result for
zero, with collision able to trigger impact earlier. Do not replace this
with ceiling division or clamp a zero countdown: zero can wrap through the
existing unsigned decrement. The target-height lookup subtracts one from
the attribute index and chooses +3000 or +5000 Y aim adjustment at the
signed -4999 threshold. The level-specific tuning remains unresolved.

Light Needle passes a rotation pointer in the variadic slot where Lightning
Bolt passes its countdown; Fire Ball/Wind Cutter ignore that slot. Fire Wall
uses the target position when available, otherwise a point 6000 world units
forward (three map tiles before integer trigonometric rounding), with Y
sampled from the floor. These argument meanings and SDK pointer boundaries
are preserved.

This batch replaces **68 numeric source uses across nine C files** with
spell/effect names or existing status bits. The complete
[spell/menu ledger](game-spell-literal-ledger.md) explains **127 remaining
occurrences in 92 expression groups** across `magic.c` and `menu_panels.c`.
The refreshed [equipment ledger](game-equipment-literal-ledger.md) contains
143 occurrences in 99 groups, down from 146; the
[floor-script ledger](game-map-script-literal-ledger.md) contains 296 in 175
groups, down from 305. These are scoped reviews; other source literals and
unresolved identities remain work for the broader campaign.

## Verification and verdicts

All nine affected units were forcibly compiled. The full comparison retains
every section of all 112 objects, all 484 strict scores and the complete
objdiff report from `00e992a`; even debug-line sections are unchanged.
Every function in the sixteen-row evidence table retains its starting
percentage: ten exact controls remain exact, six partial functions remain
partial. Independent raw comparison covers 2051 complete retail words for
the exact controls, including delay slots and ordered call/address referents.
No new function is banked.

Modern compilation remains 64/112 passing and 48 failing, with exactly the
same multiset of 320 diagnostics. Inventory validation, `ruff check scripts
tests` and `git diff --check` pass. The repository suite ran 656 tests in
84.550 seconds: 655 pass; the existing untracked save/load-hub test
`test_complete_hub_words_referents_and_state_update_slots` still fails its
retail-word comparison. No test was added or modified for this batch.

Full `kf build` ran and remains unsuccessful on existing data/ownership
divergences: source data 8/61, config contributions 4/4, target relinks PSX 1/1,
GAME 75/77 and OPEN 34/38. The two GAME/four OPEN section-base conflicts remain,
with zero artifact failures. The spell naming introduces no new matching,
compiler or test regression.
