use kf_codec::world_state::*;

#[derive(Default)]
struct Context {
    calls: Vec<String>,
    random: i32,
    found_actor: i32,
}

impl RestoreContext for Context {
    fn rand(&mut self) -> i32 {
        self.calls.push("rand".into());
        self.random
    }

    fn floor_height(&mut self, cell_z: u16, cell_x: u16) -> u8 {
        self.calls.push(format!("height:{cell_z}:{cell_x}"));
        (cell_z + cell_x) as u8
    }

    fn apply_copy_region(&mut self, region: u8) {
        self.calls.push(format!("copy:{region}"));
    }

    fn find_actor_at_tile(&mut self, tile_x: u8, tile_z: u8) -> i32 {
        self.calls.push(format!("find:{tile_x}:{tile_z}"));
        self.found_actor
    }

    fn clear_object_link(&mut self, link_id: u8) {
        self.calls.push(format!("clear:{link_id}"));
    }

    fn trigger_object_link(&mut self, link_id: u8) {
        self.calls.push(format!("trigger:{link_id}"));
    }

    fn begin_actor_death_by_definition(&mut self, definition_id: u16) {
        self.calls.push(format!("death:{definition_id}"));
    }
}

fn destinations(fill: u8) -> (Vec<u8>, Vec<u8>, Vec<u8>, Vec<u8>) {
    (
        vec![fill; MAP_EVENT_COUNT * MAP_EVENT_SIZE],
        vec![fill; ACTOR_STATE_HEAD_SIZE],
        vec![fill; ACTOR_COUNT * ACTOR_SIZE],
        vec![fill; MAP_OBJECT_COUNT * MAP_OBJECT_SIZE],
    )
}

fn encoded_record() -> Vec<u8> {
    let mut source = Vec::new();
    source.push(1);
    for event in 0..MAP_EVENT_COUNT {
        source.extend_from_slice(&[event as u8 + 1, 9, 1, 2, 0x30 + event as u8, 3, 4]);
    }
    source.push(2);
    source.extend_from_slice(&[3, 7, 127, 8]);
    for object in 0..MAP_OBJECT_COUNT {
        source.push((object % 150) as u8);
    }
    let object_ids = 1 + 56 + 1 + 4;
    source[object_ids + 170] = 42;
    source[object_ids + 171] = 45;
    source[object_ids + 172] = 50;
    source.push(1);
    source.push(3);
    source.extend_from_slice(&[10, 11, 12, 13, 14, 15, 16, 17]);
    for index in 0..10u8 {
        source.extend_from_slice(&[index + 1, index + 2, 0x80 + index, 0x90 + index]);
    }
    for index in 0..20u8 {
        source.extend_from_slice(&[index + 21, index + 31, index]);
    }
    source
}

#[test]
fn encoded_prefix_expands_exact_fields_and_preserves_unknowns() {
    let source = encoded_record();
    assert_eq!(source.len(), 362);
    let (mut events, mut actor_head, mut actors, mut objects) = destinations(0xa6);
    let mut context = Context::default();
    let report = restore_floor_state(
        &source,
        WorldStateDestinations {
            event_pool: &mut events,
            actor_state_head: &mut actor_head,
            actor_pool: &mut actors,
            object_pool: &mut objects,
        },
        RestoreConditions::default(),
        &mut context,
    )
    .unwrap();
    assert_eq!(
        report,
        RestoreReport {
            marker_was_set: true,
            actor_overrides: 2,
            link_overrides: 1,
            consumed_bytes: 362,
        }
    );
    assert_eq!(events[0], 1);
    assert_eq!(events[8], 9);
    assert_eq!(events[9], 1);
    assert_eq!(events[10], 2);
    assert_eq!(events[3], 0x30);
    assert_eq!(events[11], 3);
    assert_eq!(events[13], 4);
    assert_eq!(events[1], 0xa6);
    assert_eq!(actors[3 * ACTOR_SIZE + 6], 7);
    assert_eq!(actors[127 * ACTOR_SIZE + 6], 8);
    assert_eq!(
        &objects[3 * MAP_OBJECT_SIZE + 32..3 * MAP_OBJECT_SIZE + 40],
        &[10, 11, 12, 13, 14, 15, 16, 17]
    );

    let effect = &objects[160 * MAP_OBJECT_SIZE..161 * MAP_OBJECT_SIZE];
    assert_eq!(u16::from_le_bytes(effect[2..4].try_into().unwrap()), 1);
    assert_eq!(u16::from_le_bytes(effect[4..6].try_into().unwrap()), 2);
    assert_eq!(i32::from_le_bytes(effect[8..12].try_into().unwrap()), 2000);
    assert_eq!(i32::from_le_bytes(effect[12..16].try_into().unwrap()), -300);
    assert_eq!(i32::from_le_bytes(effect[16..20].try_into().unwrap()), 4000);
    assert_eq!(&effect[24..30], &[0; 6]);
    assert_eq!(&effect[32..38], &[0x80, 0x90, 0, 0, 0, 0]);
    assert_eq!(&effect[38..40], &[0xa6; 2]);

    let effect_42 = &objects[170 * MAP_OBJECT_SIZE..171 * MAP_OBJECT_SIZE];
    assert_eq!(
        i16::from_le_bytes(effect_42[24..26].try_into().unwrap()),
        0x400
    );
    let effect_45 = &objects[171 * MAP_OBJECT_SIZE..172 * MAP_OBJECT_SIZE];
    assert_eq!(i16::from_le_bytes(effect_45[24..26].try_into().unwrap()), 0);
    let effect_50 = &objects[172 * MAP_OBJECT_SIZE..173 * MAP_OBJECT_SIZE];
    assert_eq!(&effect_50[24..26], &[0xa6; 2]);
    assert_eq!(context.calls.len(), 90);
    assert_eq!(&context.calls[..3], &["rand", "rand", "height:2:1"]);
}

#[test]
fn malformed_dynamic_index_is_preflighted_before_writes() {
    let mut source = encoded_record();
    source[58] = 0xff;
    let (mut events, mut actor_head, mut actors, mut objects) = destinations(0x77);
    let mut context = Context::default();
    assert_eq!(
        restore_floor_state(
            &source,
            WorldStateDestinations {
                event_pool: &mut events,
                actor_state_head: &mut actor_head,
                actor_pool: &mut actors,
                object_pool: &mut objects,
            },
            RestoreConditions::default(),
            &mut context,
        ),
        Err(RestoreError::ActorIndexOutOfRange {
            entry: 0,
            index: 0xff
        })
    );
    assert!(events.iter().all(|&byte| byte == 0x77));
    assert!(actors.iter().all(|&byte| byte == 0x77));
    assert!(objects.iter().all(|&byte| byte == 0x77));
    assert!(context.calls.is_empty());
}

#[test]
fn floor_one_conditions_keep_retail_helper_order() {
    let (mut events, mut actor_head, mut actors, mut objects) = destinations(0x44);
    let mut context = Context {
        found_actor: 5,
        ..Context::default()
    };
    restore_floor_state(
        &[0],
        WorldStateDestinations {
            event_pool: &mut events,
            actor_state_head: &mut actor_head,
            actor_pool: &mut actors,
            object_pool: &mut objects,
        },
        RestoreConditions {
            current_floor: 1,
            world_byte_2: 1,
            world_byte_1: 0,
            flag_8009f845: 1,
            ..RestoreConditions::default()
        },
        &mut context,
    )
    .unwrap();
    assert_eq!(context.calls, ["copy:1", "find:7:40", "clear:51"]);
    assert_eq!(actors[5 * ACTOR_SIZE + 6], 3);
}

#[test]
fn floor_two_three_and_five_conditions_are_complete() {
    for (floor, conditions, expected) in [
        (
            2,
            RestoreConditions {
                current_floor: 2,
                highest_floor: 3,
                flag_8009f845: 1,
                ..RestoreConditions::default()
            },
            vec!["clear:51"],
        ),
        (
            3,
            RestoreConditions {
                current_floor: 3,
                flag_8009f845: 1,
                flag_8009eafc: 4,
                ..RestoreConditions::default()
            },
            vec!["clear:51", "copy:2", "copy:3"],
        ),
        (
            5,
            RestoreConditions {
                current_floor: 5,
                flag_8009f844: 1,
                flag_8009f846: 0,
                inventory_0a: 1,
                boss_defeat_complete: 1,
                ..RestoreConditions::default()
            },
            vec![
                "clear:52",
                "trigger:13",
                "death:0",
                "death:2",
                "death:3",
                "death:4",
            ],
        ),
    ] {
        let (mut events, mut actor_head, mut actors, mut objects) = destinations(0x22);
        let mut context = Context::default();
        restore_floor_state(
            &[0],
            WorldStateDestinations {
                event_pool: &mut events,
                actor_state_head: &mut actor_head,
                actor_pool: &mut actors,
                object_pool: &mut objects,
            },
            conditions,
            &mut context,
        )
        .unwrap();
        assert_eq!(context.calls, expected);
        if floor == 2 {
            assert_eq!(events[0], 3);
        }
        if floor == 5 {
            assert_eq!(events[MAP_EVENT_SIZE], 1);
            for offset in [0x438, 0x43e, 0x43f, 0x440, 0x441] {
                assert_eq!(actor_head[offset], 0xff);
            }
        }
    }

    let (mut events, mut actor_head, mut actors, mut objects) = destinations(0x22);
    let mut context = Context::default();
    restore_floor_state(
        &[0],
        WorldStateDestinations {
            event_pool: &mut events,
            actor_state_head: &mut actor_head,
            actor_pool: &mut actors,
            object_pool: &mut objects,
        },
        RestoreConditions {
            current_floor: 5,
            flag_8009f846: 1,
            ..RestoreConditions::default()
        },
        &mut context,
    )
    .unwrap();
    assert_eq!(context.calls, ["copy:4"]);
}
