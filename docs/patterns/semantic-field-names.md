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
| Effect +0x22/+0x32 | `rotation_pad`, `direction_pad` | Copied fourth lanes of the constructor's SVECTOR arguments and between rotation/direction vectors; no invented operational meaning for SDK padding. |
| Effect +0x38 | `control.frames_remaining` | Halfword countdown in kinds 4/6/9/10; kind 4 decrement/store at 80038d54/80038d68, kind 10 at 80038eb8/80038ec4. |
| Effect +0x38 | `control.orbit_angle` | Kind 17 initializes to zero; 2D projectile helper feeds it to sin/cos and advances by 64 modulo 4096. |
| Effect +0x38 | `control.parent_effect_index` | Kind 19 constructor receives parent pointer difference; dispatcher 800393b4 reads its low byte and indexes the effect pool. |
| Effect +0x38 | `control.target_mode` | Kind 20 uses 0xff for random motion, 0xfe for player homing, and any other value for actor cone search. The value is not used as an actor array index. |
| Effect +0x3a | `propagation.generations_remaining`, `propagation.branch` | Kind 10 decrements and passes it to its child (80038ed4..80038f70); kind 6 uses 0/1/2/0xff to choose propagation direction and lifetime. |

The kind-36 constructor initializes the trailing two slots to 0xff although its
handler does not consume them. Their union names describe the established uses
in the other kinds; no new behavior is inferred from those stores.

## Still requiring evidence

The patch leaves the two unused animation-file header fields, OPEN reset-only
control words, floor-item +3, and map-event +0x0c/+0x0d open. Each needs its actual serialized or binary
consumer checked before a final semantic name. Renaming them to generic
`reserved`/`data` merely to remove the search hits would not meet the goal.

The shipped-resource audit provides constraints, not semantic names:

- All 214 animation clip headers have 20 at +2. This is not zero padding.
- All 827 morph objects have zero in the initial word. This alone does not
  distinguish a flag, index, exporter field, or reserved storage.
- Floor-item +3 varies across the five floors (8, 28, 32, 64); GAME and OPEN
  copy it from placement records, but their current renderers do not read it.
- Active event definitions have zero in both +0x0b and +0x0c in all five
  floors. Runtime +0x0d is persisted and restored without a known behavioral
  consumer.
- OPEN's three trailing control words are zeroed individually and by the
  complete state clear. The validated direct-reference census has no reads.

The resource census uses the existing length-prefixed chunk and animation
oracles. It covers 169 shipped assets, including 70 animated assets. The
actor-definition loader copies 12 records per floor; trailing chunk bytes are
not automatically additional live actor definitions.

## Verification

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
