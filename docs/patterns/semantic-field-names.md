# Semantic field naming

The requested scope is every `unknown_` occurrence in `src/`, including the
shared header declarations. This first patch covers the fields with established
consumers; the remaining serialized and reset-only fields still need analysis.
The entire objective remains open.

## Function Match Plan

Preserve every operation, literal, signedness, structure extent, and member
offset. Split byte arrays only where their individual consumers establish
different meanings. Use equal-width union members for effect slots reused by
different kinds. Keep the existing dispatcher edits verbatim except for member
spellings. After applying, rebuild all affected units, compare all 112 object
hashes against the fresh baseline, inspect strict objdiff per function, and run
the full build. No new exactness or compiler-attribution claim is intended.

Hash-identical retail images were initialized with `kf init`. Baseline
`kf build compare` passed. `snapshots.json` and the per-unit directories record
35 functions' image, VA, extent, CFG/disassembly, calls, strings, references,
current strict matches, and source history. This is game structure naming;
the SDK and runtime callees remain vendored and their bodies are untouched.

## Field evidence

All addresses in this table refer to GAME.EXE.

| Owner / offset | Source name | Consumer evidence |
| --- | --- | --- |
| Player +0x0d | `allow_near_actor_spawn` | `player_sync_position_to_map` sets it at 80016fb8; awareness at 8002e6a8 rejects dormant actors within 26000 when clear; main loop clears after the first actor/render sweep. |
| Player +0x58 | `equipment_effect_ticks` | Cleared by position sync; player update divides the word by equipment HP/MP regeneration and drain intervals, then increments it at 80019f40. |
| Player +0x78 | `weapon_magic_shots_remaining` | 8001923c onward computes the number of weapon-magic shots; emission decrements it, uses parity for attachment, and charges MP at the final shot. |
| Player +0x79 | `weapon_magic_delay` | Weapon magic emits at 1, writes per-weapon delays of 1/2/3, and decrements other nonzero values. |
| Player +0x98 | `hud_gauges_enabled` | Config row 2, initialized to 1 at 80016e9c; render_frame controls HP/MP/charge/status sprites with it. |
| Player +0x99 | `compass_enabled` | Config row 3, initialized to 1 at 80016ea4; enables the two HUD components whose needle rotation is the negated view yaw. |
| Render +0x80 | `effect_color_matrix` | Passed to SetColorMatrix by render_effect_sprites at 8001f8d4; initialized from color table 3 and interpolated during death at 800184e4/80018700. |
| Actor definition +0 | `pursuit_distance_scale` | 8002e364 loads the byte; selection shifts it by 8 and selects player-pursuit action 2 inside the threshold, with existing hysteresis. |
| Actor definition +1 | `model_and_texture` | render_actor takes low nibble as asset and high nibble as texture-page/CLUT variant. |
| Actor definition +2 | `melee_attack_chance` | 8002e4b0 supplies chance for action 4, whose animation triggers actor_try_attack_player. |
| Actor definition +0x34/+0x36 | `special_attack_chance`, `special_attack_range` | Signed loads at 8002e3b0/8002e3e0/8002e3e4 feed selection of actions 16/17; 80030108/800301ec pass the same range to the attack. |
| Actor definition +0x82 | `effect_owner_id` | Halfword loads at 8002f10c/8002f164/8002f19c/8002f1c4 supply the constructor's owning-ID argument. |
| Actor definition +0x96 | `gold_drop_limit` | Actor death at 8002fd3c computes `rand() * limit >> 15`. The callee at 800319c8 places object 39 and stores the resulting amount in the two link bytes. COM.DAT defines object 39 as behavior 0x41; `map_interaction_dispatch` reconstructs that amount, notifies it, credits `player_state.gold`, and frees the object. The bound is exclusive. |
| Placement +5 / Actor +7 | `spawn_chance` | Copied by actor_pool_load_placements; awareness at 8002e760 and 8002e810 compares the byte shifted by 7 against rand. |
| Placement +6 / Actor +9 | `death_drop_object_id` | Copied by placement loader; 8002fdd4 passes it as object ID to map_object_spawn_effect during death, except sentinel 0x63. |
| Object link +6/+7 | `linked_notification`, `default_notification` | map_interaction_dispatch passes the first to notify_enqueue for behavior 8 with a live link; the second is its default notification. Reset clears both. |
| Event definition +0x0d / Event +0x0e | `behavior` | Pool load copies the byte; update dispatches 1 to wander and 2 to spinner; interaction also switches on it. |
| Event +0x0f | `animation_clip` | Third argument to render_bind_animated_instance; interaction selects 0/1 while advancing animation phase. |
| Event +0x10 | `collision_turn_pending` | Wander clears after successful movement, sets after collision chooses a new yaw, and avoids another random yaw until turning completes. |
| Event +0x34/+0x38 | `rotation_x`, `rotation_z` | Loader clears the outer lanes of the eight-byte SVECTOR consumed by RotMatrix and collision; +0x36 is the existing yaw field. |
| Effect +2/+3 | `base_render_id`, `render_id` | Constructor initializes both; 80038e98..80038eac alternates current ID from base, and 8003957c onward cycles it. Rendering selects a sprite or model from current ID. |
| Effect +4 | `animation_clip` | Renderer uses 0xff for billboard mode; otherwise supplies this byte as clip index to render_bind_animated_instance. |
| Effect +5 | `sound_played` | 80039040 checks the latch; 8003906c stores the spatial-audio result. audio_play_spatial returns 0 outside range and 1 after issuing sound; helpers reset the latch for another pass. |
| Effect +7 | `phase` | Entry byte snapshot at 80038a7c; kind-specific lifecycle/age transitions and map-object triggers. |
| Effect +8 | `visual.animation_phase`, `visual.pulse_base_scale` | Model animation consumes this halfword; kind 10 instead stores its base scale at 80038efc and computes sin/cos scale oscillation at 80038fb4 onward. |
| Effect +0x22/+0x32 | `rotation.pad`, `direction.words.pad` | Copied fourth lanes of the constructor's SVECTOR arguments and between rotation/direction vectors; no invented operational meaning for SDK padding. |
| Effect +0x38 | `control.frames_remaining` | Halfword countdown in kinds 4/6/9/10; kind 4 decrement/store at 80038d54/80038d68, kind 10 at 80038eb8/80038ec4. |
| Effect +0x38 | `control.orbit_angle` | Kind 17 initializes to zero; 2D projectile helper feeds it to sin/cos and advances by 64 modulo 4096. |
| Effect +0x38 | `control.parent_effect_index` | Kind 19 constructor receives parent pointer difference; dispatcher 800393b4 reads its low byte and indexes the effect pool. |
| Effect +0x38 | `control.target_mode` | Kind 20 uses 0xff for random motion, 0xfe for player homing, and any other value for actor cone search. The value is not used as an actor array index. |
| Effect +0x3a | `propagation.generations_remaining`, `propagation.branch` | Kind 10 decrements and passes it to its child (80038ed4..80038f70); kind 6 uses 0/1/2/0xff to choose propagation direction and lifetime. |

The kind-36 constructor initializes the two bytes at +0x38/+0x39 to 0xff,
although its handler does not consume them. The byte view retains that
unresolved purpose; other kinds establish the union's countdown, orbit,
parent-index and homing-selector meanings. The incoming constructor audit
below corrects the former two-halfword interpretation of these stores.

## Still requiring evidence

The patch leaves the unused animation-clip header field, OPEN reset-only
control words, floor-item +3, and map-event +0x0c/+0x0d open. Each needs its actual serialized or binary
consumer checked before a final semantic name. Renaming them to generic
`reserved`/`data` merely to remove the search hits would not meet the goal.

The shipped-resource audit provides constraints, not semantic names:

- All 214 animation clip headers have 20 at +2. This is not zero padding.
- All 827 morph objects have zero in the initial word. This observation alone
  was inconclusive; the VDF format follow-up below identifies the field.
- Floor-item +3 varies across the five floors (8, 28, 32, 64); GAME and OPEN
  copy it from placement records, but their current renderers do not read it.
- Active event definitions have zero in both +0x0b and +0x0c in all five
  floors. Runtime +0x0d is persisted and restored without a known behavioral
  consumer.
- OPEN's three trailing control words are zeroed individually and by the
  complete state clear. The validated direct-reference census has no reads.

The OPEN controls now have a specific cross-image candidate: GAME's three
map-object effect allocation sequences. GAME clears 0x25b8 bytes from
8006e8e0, while its definitions and live records occupy 0x25a8; its sequence
words at 80070e92/94/96 therefore lie 10, 12, and 14 bytes after the records.
OPEN clears 0x510 bytes from 80049538, has 0x500 bytes of live records, and
clears words at 80049a42/44/46 at the same relative tail offsets. Both pool
reset routines clear the three words in descending address order. This is
evidence of a shared or copied pool tail, but assigning GAME's allocation
semantics to OPEN remains a candidate: OPEN has no corresponding allocator
consumer. The field spellings are retained until that distinction is resolved.

The resource census uses the existing length-prefixed chunk and animation
oracles. It covers 169 shipped assets, including 70 animated assets. The
actor-definition loader copies 12 records per floor; trailing chunk bytes are
not automatically additional live actor definitions.

## Morph-record format follow-up

The GAME `render_bind_animated_instance` dossier at 800205d4 and the shipped
asset census now have external format evidence. Sony's
[Data Conversion Utilities, figure 2-7](https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/Dataconv.pdf#page=100)
defines each VDF object record as three 32-bit words (TMD object number,
first affected vertex, affected vertex count), followed by eight-byte vertex
deltas. This matches the complete `KfMorphObject` layout. Sony's
[MIMe support answer](https://psx.arthus.net/sdk/Psy-Q/DOCS/BBS/scej_bbs.pdf#page=43)
explains that the object number selects a TMD object and is zero for a
single-object model.

Audit the full shipped record corpus against that layout and its TMD object
bounds, then name +0 `tmd_object_index`. This is a supported format identity,
not proof of the particular converter/version used by FromSoftware. Retail
ignores this word and explicitly selects TMD object zero; keep that behavior.
The clip's +2 word belongs to the game's surrounding animation grammar and
is not explained by the VDF diagram. Rebuild the affected unit and verify its
sections and strict match state before keeping the rename.

The audit passes for all 827 records in 70 animated assets. Each selected TMD
object exists, each vertex range fits that object, and all 757 within-asset
record boundaries equal the next table offset after exactly twelve header
bytes and eight bytes per vertex. Every object selector is zero. The complete
record correspondence plus the documented format supports the rename; it
does not rely on zero values alone. The surrounding clip grammar is still
unresolved.

The rebuilt pool has unchanged non-debug sections and retains its strict
98.79829% binder result; its six lifecycle functions remain exact. Lint and
all 635 tests pass (nine local prerequisites skipped). Full `kf build` retains
the existing data-placement failures. The source now has ten `unknown_`
lines; no unmatched function was banked.

## Initial field-naming verification

Changes are isolated on `codex/semantic-names`. After rebuilding the affected
units, all 112 compiled objects are SHA-256 identical to the fresh baseline at
`23a4356`; the entire strict objdiff report is also identical. There is no first
instruction or relocation divergence for any function in this naming phase.
The inventory now describes 93 layouts and 767 fields, with 674 named fields.

`ruff check scripts tests`, all 634 repository tests (nine local prerequisites
skipped), `git diff --check`, and `nix flake check -L` pass. Full `kf build` was
run and still fails existing data-placement/ownership gates, including GAME's
37 divergent data-owning units. Identical compiled objects establish that this
phase did not change those failures. This campaign does not claim completion
of the remaining field identities or constant audit.

## Save-summary field recovery

Function Match Plan: replace the opaque six-word `KfSaveSlotSummary` array
with six named `u32` members. The writer GAME 8002b73c, catalog copier
8002b078, reader/consistency check 8002beb0, dialog renderer 80027ee4 and
load panel 8002552c are covered by the complete image-qualified dossiers.
Their current source, raw member accesses, callers, callees, string/data
references, adjacent bodies and histories were reviewed. Preserve the six
individual assignments/comparisons, signed display predicate, all widths,
24-byte stride, partial catalog clear and relocation referents. Add measured
layout assertions and reconcile the curated structure inventory. Require all
112 non-debug objects and all 484 strict scores to agree with 1a2cbce.

| Summary offset | Member | Writer evidence in GAME |
| --- | --- | --- |
| 00 | `experience` | 8002bb18 loads the player's experience word; 8002bb20 stores it at directory summary +00. |
| 04 | `current_floor` | 8002bb28 loads the unsigned floor byte at player +0a; 8002bb30 stores the widened word. |
| 08 | `current_hp` | 8002bb38 loads the unsigned halfword at player +12; 8002bb40 stores it. |
| 0c | `maximum_hp` | 8002bb48 loads the unsigned halfword at player +10; 8002bb50 stores it. |
| 10 | `current_mp` | 8002bb58 loads the unsigned halfword at player +16; 8002bb60 stores it. |
| 14 | `maximum_mp` | 8002bb68 loads the unsigned halfword at player +14; 8002bb70 stores it in the call delay slot. |

The dialog independently loads these six words at 80028170, 800281d0,
80028228, 8002827c, 800282d4 and 80028328. Its signed positive-HP predicate
at 80028120/28 gates a displayed row; the load panel also checks current HP
before confirming a slot. Retain that signed display cast even though the
serialized fields are `u32`. The catalog still clears only 0x24 bytes of
three 0x18-byte summaries; naming does not repair that observed behavior.

Verification: the measured size/offset assertions compile under the pinned
toolchain. The inventory now has 772 fields, 680 named, reflecting six named
members in place of one opaque array. All 112 non-debug objects and all 484
strict scores remain identical to 1a2cbce. All 644 repository tests pass
(nine skips), flake checks pass (644 tests, 131 sandbox skips), and lint and
whitespace checks pass. The full build retains the existing ownership and
placement failures, with five of 60 source data owners matching, target
relink PSX 1/1, GAME 75/77 and OPEN 34/38, and no artifact failures.

## PlayStation save-icon header

Function Match Plan: refine GAME `save_file_initialize_buffers` (8002c304,
0x20c bytes, currently strict 100%) from byte offsets into the supported
PlayStation card-header layout. Its complete dossier covers the raw CFG,
sole caller `save_file_write_slot`, SDK boundaries, strings, data references,
relocations, adjacent functions and history. The King's Field title and icon
paths establish game ownership; the routine is not a vendored library body.
Preserve all four byte stores, the 53-byte title copy, first-frame palette
selection, three 128-byte image copies, calls and delay-slot operations.
Add measured layout checks and update the curated inventory. Require every
non-debug section of all 112 objects and all 484 strict scores to agree with
the immutable 1a2cbce baseline before committing.

Sony's [Run-Time Library Overview, tables 5-6 and 5-7](https://gamingdoc.org/wp-content/uploads/2020/07/Run-Time-Library-Ovewview-LIBOVR46.pdf#page=63)
documents the `SC` signature, icon-type byte, block count, 64-byte Shift-JIS
title, 28 zero bytes, sixteen 16-bit CLUT entries and three 128-byte frames
for type 0x13. This later documentation supports the file format, not an
attribution of the game's compiler or SDK release. The supplied Release 2.5
headers provide no corresponding card-header C type. `KfPsxSaveHeader`
models the three-frame variant used here, with the game directory following
at +0x200.

| Header offset | Member | GAME initializer evidence |
| --- | --- | --- |
| 00 | `magic[2]` | 8002c334 and 8002c344 store `S` and `C` as bytes. |
| 02 | `icon_type` | 8002c354 stores 0x13, selecting three animation frames. |
| 03 | `block_count` | 8002c364 stores five, also used in the file-create request. |
| 04 | `title[64]` | 8002c374 selects the destination; the loop and tail copy 53 bytes including the terminator. |
| 44 | `zero_pad[28]` | The initial 0x280-byte clear supplies the format's required zeros; later writes skip this span. |
| 60 | `clut[16]` | 8002c3d0..8002c40c copy 32 bytes from ICO1's palette. |
| 80 | `icon_frames[3][128]` | Destination calculations at 8002c41c, 8002c46c and 8002c4bc select successive frames. |

The three shipped `TIM/ICO1.TIM`..`ICO3.TIM` files each have 192 bytes:
4-bit TIM data, a 16-by-1 palette at byte 20 and 16-by-16 pixels at byte 64.
Thus each image contributes 16 * 16 / 2 = 128 packed pixel bytes. The local
2048-byte buffer serves a sector-rounded CD read; it does not describe an
icon's encoded length. Source offset names refer specifically to these three
assets, since general TIM pixel offsets depend on the palette block size.
Retail-derived decoded images and measurements remain ignored build outputs.

Verification: the seven offset checks and 0x200-byte extent compile under the
pinned toolchain. The inventory now has 94 structures and 779 fields, 687
named. All 112 objects retain every non-debug section and all 484 strict
scores from 1a2cbce; the initializer remains strict 100%. All 644 repository
tests pass (nine skips), flake checks pass (644 tests, 131 sandbox skips),
and lint/whitespace checks pass. The full build retains the existing
ownership and placement failures: five of 60 source data owners match,
target relink is PSX 1/1, GAME 75/77 and OPEN 34/38, and there are no artifact
failures. No new function is banked by this field recovery.

## Incoming effect constructor and field widths

Function Match Plan: integrate committed master 282cbe7 into the naming
worktree. Its GAME constructor dossier and independent raw-word/table
controls establish full SVECTOR copies, exact optional-argument publication,
the decoded case joins and corrected relocation targets. Preserve that exact
constructor and the semantic fields established by the dispatcher and
projectile helpers. The selected family is `game.effect_pool`,
`game.effect_dispatch`, `game.effect_update` and the warp-shimmer consumer;
their image-qualified dossiers, current sources, histories and incoming
evidence cover all affected bodies.

Refine `control.parent_effect_index` and `control.target_mode` to byte members:
the constructor's byte stores agree with their byte-only dispatcher reads.
Retain halfword countdown/orbit members in the same shared union. Kind 36
writes +0x39 and +0x38 as bytes, not the two halfwords +0x3a and +0x38;
preserve a low/high byte view without inventing their unused semantic roles.
Use the incoming complete SDK rotation and unsigned direction union, and
retain the established animation, audio, lifecycle and propagation names.

Verification: independently compile the four literal incoming source units
with their incoming effect header and compare every non-debug section with
the reconciled sources. Require the other 108 objects and 483 unaffected
strict function scores to remain unchanged from 1a2cbce; only the constructor
may improve to 100%. Run the incoming fresh retail-word/switch-table controls,
the full repository and flake checks, and full `kf build`. Keep the incoming
banking record without creating another banked result.

The reconciled constructor is byte-identical to the entire independently
compiled incoming object. Its three companion units match every non-debug
section of the incoming controls, and the other 111 objects retain every
non-debug section from 1a2cbce. Exactly one of 484 strict scores changes:
GAME 80036f44 improves from 52.130020% to 100%, as established by the incoming
source work. The four direct retail controls reproduce all 523 constructor
words, ordered calls/referents, its 45 table rows and the three exact siblings.

The combined inventory retains recursive named-bound support and adds the
incoming standalone-union coverage. It has 98 types and 784 fields, 689
named. Parent/target selectors are `u8`; countdown/orbit and propagation
remain `u16`. The kind-36 low/high bytes remain explicitly opaque. All 651
repository tests pass (nine skips), all flake checks pass (651 tests,
135 sandbox skips), and lint/whitespace checks pass. Full `kf build` retains
the existing ownership/placement failures, with the incoming effect-pool
result improving source-data matches from five to six of 60. Target relink
remains PSX 1/1, GAME 75/77 and OPEN 34/38, with no artifact failures.

## Player poison witness

The retail menu atlas now supplies the missing semantic witness for
`KfPlayerState.poison_resistance`, `KfPlayerState.poison_timer` and
`KfArmorRecord.poison_resistance`. The `毒` glyph is selected by status mask
4 and labels the same resistance value whose timer causes periodic HP loss.
The [poison dossier](game-player-poison.md) records the per-function plan,
raw instruction evidence, preserved layouts and limits of the timing names.
