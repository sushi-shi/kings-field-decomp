use kf_codec::audio::SoundRef;
use kf_codec::math::{SVector, Vec3s};
use kf_codec::records::*;

#[test]
fn fixed_loaders_copy_only_the_retail_extent() {
    for (size, loader) in [
        (
            RENDER_CELL_WINDOWS_SIZE,
            load_render_cell_windows as fn(&[u8], &mut [u8]) -> _,
        ),
        (ARMOR_RECORDS_SIZE, load_armor_records),
        (MAGIC_RECORDS_SIZE, load_magic_records),
        (MAP_OBJECT_DEFINITIONS_SIZE, load_map_object_definitions),
        (PLAYER_LEVEL_GROWTH_TABLE_SIZE, load_player_level_growth),
        (ACTOR_DEFINITIONS_SIZE, load_actor_definitions),
    ] {
        let source = vec![0x35; size + 7];
        let mut destination = vec![0xa7; size + 11];
        loader(&source, &mut destination).unwrap();
        assert!(destination[..size].iter().all(|&byte| byte == 0x35));
        assert!(destination[size..].iter().all(|&byte| byte == 0xa7));
    }
}

#[test]
fn weapon_angles_are_wrapping_negated_and_tail_is_untouched() {
    let mut source = vec![0x21; WEAPON_RECORDS_SIZE];
    let angles = [0u16, 1, 0x7fff, 0x8000, 0xffff];
    for index in 0..WEAPON_RECORD_COUNT {
        let at = index * WEAPON_RECORD_SIZE + 0x26;
        source[at..at + 2].copy_from_slice(&angles[index % angles.len()].to_le_bytes());
    }
    let mut destination = vec![0xcc; WEAPON_RECORDS_SIZE + 9];
    load_weapon_records(&source, &mut destination).unwrap();
    for index in 0..WEAPON_RECORD_COUNT {
        let at = index * WEAPON_RECORD_SIZE + 0x26;
        let actual = u16::from_le_bytes([destination[at], destination[at + 1]]);
        assert_eq!(actual, angles[index % angles.len()].wrapping_neg());
    }
    assert_eq!(&destination[WEAPON_RECORDS_SIZE..], &[0xcc; 9]);
}

#[test]
fn malformed_inputs_do_not_partially_write() {
    let mut destination = vec![0x5a; WEAPON_RECORDS_SIZE];
    assert!(matches!(
        load_weapon_records(&[0; WEAPON_RECORDS_SIZE - 1], &mut destination),
        Err(TransformError::SourceTooSmall { .. })
    ));
    assert!(destination.iter().all(|&byte| byte == 0x5a));

    let mut too_short = vec![0x69; MAGIC_RECORDS_SIZE - 1];
    assert!(matches!(
        load_magic_records(&[0; MAGIC_RECORDS_SIZE], &mut too_short),
        Err(TransformError::DestinationTooSmall { .. })
    ));
    assert!(too_short.iter().all(|&byte| byte == 0x69));
}

#[test]
fn typed_records_round_trip_every_byte() {
    fn pattern<const N: usize>() -> [u8; N] {
        let mut bytes = [0; N];
        for (index, byte) in bytes.iter_mut().enumerate() {
            *byte = (index as u8).wrapping_mul(37).wrapping_add(11);
        }
        bytes
    }

    let weapon = pattern::<WEAPON_RECORD_SIZE>();
    let mut encoded = [0; WEAPON_RECORD_SIZE];
    assert!(WeaponRecord::decode(&weapon).unwrap().encode(&mut encoded));
    assert_eq!(weapon, encoded);

    let armor = pattern::<ARMOR_RECORD_SIZE>();
    let mut encoded = [0; ARMOR_RECORD_SIZE];
    assert!(ArmorRecord::decode(&armor).unwrap().encode(&mut encoded));
    assert_eq!(armor, encoded);

    let magic = pattern::<MAGIC_RECORD_SIZE>();
    let mut encoded = [0; MAGIC_RECORD_SIZE];
    assert!(MagicRecord::decode(&magic).unwrap().encode(&mut encoded));
    assert_eq!(magic, encoded);

    let definition = pattern::<MAP_OBJECT_DEFINITION_SIZE>();
    let mut encoded = [0; MAP_OBJECT_DEFINITION_SIZE];
    assert!(MapObjectDefinition::decode(&definition)
        .unwrap()
        .encode(&mut encoded));
    assert_eq!(definition, encoded);

    let growth = pattern::<PLAYER_LEVEL_GROWTH_SIZE>();
    let mut encoded = [0; PLAYER_LEVEL_GROWTH_SIZE];
    assert!(PlayerLevelGrowth::decode(&growth)
        .unwrap()
        .encode(&mut encoded));
    assert_eq!(growth, encoded);

    let window = pattern::<204>();
    let mut encoded = [0; 204];
    assert!(RenderCellWindow::decode(&window)
        .unwrap()
        .encode(&mut encoded));
    assert_eq!(window, encoded);

    let actor = pattern::<ACTOR_DEFINITION_SIZE>();
    let mut encoded = [0; ACTOR_DEFINITION_SIZE];
    assert!(ActorDefinition::decode(&actor)
        .unwrap()
        .encode(&mut encoded));
    assert_eq!(actor, encoded);
}

#[test]
fn weapon_render_fields_follow_retail_offsets_and_preserve_sdk_pad() {
    let mut bytes = [0x5a; WEAPON_RECORD_SIZE];
    bytes[0x10..0x12].copy_from_slice(&0x8123u16.to_le_bytes());
    bytes[0x1c..0x2c].copy_from_slice(&[
        0xff, 0xff, 0x00, 0x80, 0xff, 0x7f, 0x34, 0x12, 0x01, 0x80, 0x02, 0xff, 0x03, 0x00, 0xcd,
        0xab,
    ]);
    let mut record = WeaponRecord::decode(&bytes).unwrap();
    assert_eq!(record.projection_distance, 0x8123);
    assert_eq!(record.unknown_14, [0x5a; 8]);
    assert_eq!(
        record.render_translation,
        Vec3s {
            x: -1,
            y: i16::MIN,
            z: i16::MAX
        }
    );
    assert_eq!(record.unknown_22, 0x1234);
    assert_eq!(
        record.render_rotation,
        SVector {
            vx: -32767,
            vy: -254,
            vz: 3,
            pad: -21555
        }
    );
    record.render_translation.y = -2;
    record.render_rotation.vy = 254;
    let mut output = [0xcc; WEAPON_RECORD_SIZE + 2];
    assert!(record.encode(&mut output));
    bytes[0x1e..0x20].copy_from_slice(&[0xfe, 0xff]);
    bytes[0x26..0x28].copy_from_slice(&[0xfe, 0x00]);
    assert_eq!(&output[..WEAPON_RECORD_SIZE], &bytes);
    assert_eq!(&output[WEAPON_RECORD_SIZE..], &[0xcc; 2]);
}

#[test]
fn actor_fields_follow_shared_layout_and_preserve_unknown_and_unset_values() {
    let mut bytes = core::array::from_fn::<_, ACTOR_DEFINITION_SIZE, _>(|index| index as u8);
    bytes[5] = 0xff;
    bytes[0x0e] = 0xff;
    bytes[0x28..0x2e].copy_from_slice(&[0xff, 0xff, 0, 0x80, 0xff, 0x7f]);
    bytes[0x34..0x38].copy_from_slice(&[0xfe, 0xff, 0, 0x80]);
    let mut actor = ActorDefinition::decode(&bytes).unwrap();
    assert_eq!(actor.pursuit_distance_scale, 0);
    assert_eq!(actor.model_and_texture, 1);
    assert_eq!(actor.melee_attack_chance, 2);
    assert_eq!(actor.status_effect, 3);
    assert_eq!(actor.status_effect_chance, 4);
    assert_eq!(
        actor.action_parameters,
        ActorActionParameters {
            effect_codes: [0xff, 6, 7],
            effect_chances: [8, 9, 10],
            drop_object: 11,
            drop_chance: 12,
        }
    );
    assert_eq!(actor.move_speed, 13);
    assert_eq!(actor.action_animations[0], 0xff);
    assert_eq!(actor.action_animations[15], 0x1d);
    assert_eq!(actor.turn_rate, 0x1e);
    assert_eq!(
        actor.sounds,
        [
            SoundRef {
                program: 0x1f,
                tone: 0x20,
                note: 0x21
            },
            SoundRef {
                program: 0x22,
                tone: 0x23,
                note: 0x24
            },
            SoundRef {
                program: 0x25,
                tone: 0x26,
                note: 0x27
            },
        ]
    );
    assert_eq!(
        actor.attachment_offsets,
        [
            Vec3s {
                x: -1,
                y: i16::MIN,
                z: i16::MAX
            },
            Vec3s {
                x: 0x2f2e,
                y: 0x3130,
                z: 0x3332
            },
        ]
    );
    assert_eq!(actor.special_attack_chance, -2);
    assert_eq!(actor.special_attack_range, i16::MIN);
    assert_eq!(actor.unknown_38, [0x38, 0x39]);
    assert_eq!(actor.action_animation_steps[0], 0x3b3a);
    assert_eq!(actor.action_animation_steps[15], 0x5958);
    assert_eq!(actor.action_animation_phases[0], 0x5b5a);
    assert_eq!(actor.action_animation_phases[15], 0x7978);
    assert_eq!(actor.collision_radius, 0x7b7a);
    assert_eq!(actor.collision_height, 0x7d7c);
    assert_eq!(actor.awareness_distance, 0x7f7e);
    assert_eq!(actor.initial_health, 0x8180);
    assert_eq!(actor.effect_owner_id, 0x8382);
    assert_eq!(actor.experience_reward, 0x8584);
    assert_eq!(actor.attack_components, [0x8786, 0x8988, 0x8b8a]);
    assert_eq!(actor.defenses, [0x8d8c, 0x8f8e, 0x9190, 0x9392, 0x9594]);
    assert_eq!(actor.gold_drop_limit, 0x9796);

    actor.sounds[1].note = 0xff;
    actor.attachment_offsets[1].z = -3;
    actor.special_attack_chance = i16::MAX;
    bytes[0x24] = 0xff;
    bytes[0x32..0x34].copy_from_slice(&[0xfd, 0xff]);
    bytes[0x34..0x36].copy_from_slice(&[0xff, 0x7f]);
    let mut output = [0xcc; ACTOR_DEFINITION_SIZE + 3];
    assert!(actor.encode(&mut output));
    assert_eq!(&output[..ACTOR_DEFINITION_SIZE], &bytes);
    assert_eq!(&output[ACTOR_DEFINITION_SIZE..], &[0xcc; 3]);
    let mut short = [0x5a; ACTOR_DEFINITION_SIZE - 1];
    assert!(!actor.encode(&mut short));
    assert_eq!(short, [0x5a; ACTOR_DEFINITION_SIZE - 1]);
    assert!(ActorDefinition::decode(&short).is_none());
}

#[test]
fn magic_sounds_and_armor_resistances_keep_their_distinct_fields() {
    let magic = MagicRecord::decode(&core::array::from_fn::<_, MAGIC_RECORD_SIZE, _>(|i| {
        i as u8
    }))
    .unwrap();
    assert_eq!(
        magic.sounds,
        [
            SoundRef {
                program: 2,
                tone: 3,
                note: 4
            },
            SoundRef {
                program: 5,
                tone: 6,
                note: 7
            },
        ]
    );
    let armor = ArmorRecord::decode(&core::array::from_fn::<_, ARMOR_RECORD_SIZE, _>(|i| {
        i as u8
    }))
    .unwrap();
    assert_eq!(armor.cutting_defense, 0x0302);
    assert_eq!(armor.striking_defense, 0x0504);
    assert_eq!(armor.piercing_defense, 0x0706);
    assert_eq!(armor.poison_resistance, 0x0908);
    assert_eq!(armor.magic_defense, 0x0b0a);
    assert_eq!(armor.fire_defense, 0x0d0c);
    assert_eq!(armor.hp_regen_interval, 0x0f0e);
    assert_eq!(armor.hp_drain_interval, 0x1110);
}
