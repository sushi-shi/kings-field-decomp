use kf_codec::world_persist::*;

fn seeded(size: usize, seed: u8) -> Vec<u8> {
    (0..size)
        .map(|index| (index as u8).wrapping_mul(73).wrapping_add(seed))
        .collect()
}

fn sources() -> (Vec<u8>, Vec<u8>, Vec<u8>, Vec<u8>) {
    let mut events = seeded(MAP_EVENT_COUNT * MAP_EVENT_SIZE, 11);
    for event in 0..MAP_EVENT_COUNT {
        events[event * MAP_EVENT_SIZE + 9] = (event % 6) as u8;
    }
    let mut actors = seeded(ACTOR_COUNT * ACTOR_SIZE, 12);
    for actor in 0..ACTOR_COUNT {
        actors[actor * ACTOR_SIZE] = 0xff;
    }
    actors[0] = 1;
    actors[6] = 3;
    actors[ACTOR_SIZE] = 3;
    actors[ACTOR_SIZE + 6] = 2;
    actors[ACTOR_SIZE * 2] = 2;

    let mut definitions = seeded(MAP_OBJECT_DEFINITION_COUNT * MAP_OBJECT_DEFINITION_SIZE, 13);
    for (id, behavior) in [
        (1, 0xff),
        (2, 0x0d),
        (3, 0x40),
        (4, 0x0e),
        (5, 0x41),
        (6, 2),
        (7, 3),
    ] {
        definitions[id * MAP_OBJECT_DEFINITION_SIZE] = behavior;
    }
    let mut objects = seeded(MAP_OBJECT_COUNT * MAP_OBJECT_SIZE, 14);
    for object in 0..MAP_OBJECT_COUNT {
        objects[object * MAP_OBJECT_SIZE] = 0xff;
    }
    for object in 0..5 {
        let at = object * MAP_OBJECT_SIZE;
        objects[at] = object as u8 + 1;
        objects[at + 40] = 0xff;
    }
    objects[5 * MAP_OBJECT_SIZE] = 6;
    objects[5 * MAP_OBJECT_SIZE + 40] = 0xff;
    objects[6 * MAP_OBJECT_SIZE] = 7;
    objects[6 * MAP_OBJECT_SIZE + 40] = 9;
    (events, actors, definitions, objects)
}

#[test]
fn serializes_every_runtime_family_and_retains_record_tail() {
    let (events, actors, definitions, objects) = sources();
    let mut output = vec![0xa5; FLOOR_RECORD_SIZE];
    let report = persist_floor_state(
        &mut output,
        PersistSources {
            event_pool: &events,
            actor_pool: &actors,
            object_definitions: &definitions,
            object_pool: &objects,
        },
    )
    .unwrap();
    assert_eq!(
        report,
        PersistReport {
            actor_overrides: 2,
            link_records: 2,
            written_bytes: 371,
        }
    );
    assert_eq!(output[0], 1);
    assert_eq!(output[1], events[0]);
    assert_eq!(output[2], events[8]);
    assert_eq!(output[3], 0);
    assert_eq!(output[5], events[2]);
    assert_eq!(output[57], 2);
    assert_eq!(&output[58..62], &[0, 3, 1, 0]);
    assert_eq!(&output[62..69], &[1, 2, 3, 4, 5, 6, 7]);
    let link_count = 62 + MAP_OBJECT_COUNT;
    assert_eq!(output[link_count], 2);
    assert_eq!(output[link_count + 1], 5);
    assert_eq!(
        &output[link_count + 2..link_count + 10],
        &objects[5 * MAP_OBJECT_SIZE + 32..5 * MAP_OBJECT_SIZE + 40]
    );
    assert!(output[report.written_bytes..]
        .iter()
        .all(|&byte| byte == 0xa5));
}

#[test]
fn floor_five_capacity_is_checked_before_any_write() {
    let (events, mut actors, mut definitions, mut objects) = sources();
    for actor in 0..ACTOR_COUNT {
        actors[actor * ACTOR_SIZE] = 1;
    }
    definitions.fill(0);
    for object in 0..MAP_OBJECT_DEFINITION_COUNT {
        objects[object * MAP_OBJECT_SIZE] = 0;
    }
    let mut output = vec![0x77; FLOOR_FIVE_RECORD_SIZE];
    assert_eq!(
        persist_floor_state(
            &mut output,
            PersistSources {
                event_pool: &events,
                actor_pool: &actors,
                object_definitions: &definitions,
                object_pool: &objects,
            },
        ),
        Err(PersistError::OutputTooSmall {
            needed: 2_045,
            available: FLOOR_FIVE_RECORD_SIZE,
        })
    );
    assert!(output.iter().all(|&byte| byte == 0x77));
}

#[test]
fn invalid_definition_index_and_event_tag_are_preflighted() {
    let (mut events, actors, definitions, mut objects) = sources();
    let mut output = vec![0x55; FLOOR_RECORD_SIZE];
    objects[0] = 200;
    assert_eq!(
        persist_floor_state(
            &mut output,
            PersistSources {
                event_pool: &events,
                actor_pool: &actors,
                object_definitions: &definitions,
                object_pool: &objects,
            },
        ),
        Err(PersistError::ObjectDefinitionOutOfRange {
            object: 0,
            object_id: 200,
        })
    );
    assert!(output.iter().all(|&byte| byte == 0x55));

    objects[0] = 1;
    events[(MAP_EVENT_COUNT - 1) * MAP_EVENT_SIZE + 9] = 0xff;
    assert_eq!(
        persist_floor_state(
            &mut output,
            PersistSources {
                event_pool: &events,
                actor_pool: &actors,
                object_definitions: &definitions,
                object_pool: &objects,
            },
        ),
        Err(PersistError::EventTagReadOutOfRange {
            event: MAP_EVENT_COUNT - 1,
            image_index: 0xff,
        })
    );
    assert!(output.iter().all(|&byte| byte == 0x55));
}
