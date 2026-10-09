# Actor attachment layout

`KfActorDefinition` (0x98-byte MIXA chunk-6 record) holds three effect
attachment triples at +0x28..+0x39, modelled as
`struct KfVec3s attachment_offsets[3]`.

## Retail evidence

- GAME `actor_spawn_action_effect` (`0x8002edd4`) forms
  `definition + 0x28 + 6 * effect_slot` at `0x8002ee9c..0x8002eea8` and reads
  the x/y/z halfwords at +0x28/+0x2a/+0x2c of that address
  (`0x8002eeac..0x8002eec4`). `KF_ACTOR_EFFECT_SLOT_THIRD` (2) therefore reads
  +0x34/+0x36/+0x38.
- The same +0x34/+0x36 lanes are read as signed halfwords for the special and
  jump attacks: `lh a2,52` and `lh a3,54` in `actor_select_next_action`
  (`0x8002e3b0`, `0x8002e3e0`, `0x8002e3e4`) and `lh a1,54` in
  `actor_update_current_action` (`0x80030108`, `0x800301ec`).
- Shipped data: B5 `MIXA.DAT` definition 7 dispatches effect kind 24 through
  slot 2 and stores `(0, -1000, -2200)` there; the former opaque +0x38 lane is
  that triple's z coordinate. The other 59 shipped records hold zero there.

## Source model

One array owner covers all three triples. The special-attack readers are
`actor_definition_special_attack_chance()` and
`actor_definition_special_attack_range()` in `include/kf/game/actor.h`, which
read the third triple's x and y lanes. The original declaration spelling
(array, union or separate members) remains unknown; the linked bytes prove
the offsets, widths and signedness only.
