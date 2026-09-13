use kf_codec::placements::*;

fn put<T>(records: &mut Vec<u8>, size: usize, value: T, encode: impl FnOnce(T, &mut [u8])) {
    let at = records.len();
    records.resize(at + size, 0);
    encode(value, &mut records[at..at + size]);
}

#[derive(Default)]
struct ItemCtx {
    calls: Vec<&'static str>,
}

impl FloorItemContext for ItemCtx {
    fn floor_height(&mut self, tile_z: u8, tile_x: u8) -> u8 {
        self.calls.push("height");
        tile_z.wrapping_add(tile_x)
    }

    fn rand(&mut self) -> i32 {
        self.calls.push("rand");
        0x4000
    }
}

#[test]
fn floor_items_write_only_retail_fields_and_order_callbacks() {
    let mut source = Vec::new();
    put(
        &mut source,
        FLOOR_ITEM_PLACEMENT_SIZE,
        FloorItemPlacement {
            item_id: 0x1234,
            facing_and_frame_count: 8,
            unknown_03: 9,
            tile_z: 2,
            tile_x: 3,
            local_z: -7,
            local_x: 11,
            local_y: 900,
        },
        |value, bytes| assert!(value.encode(bytes)),
    );
    source.extend_from_slice(&0xffffu16.to_le_bytes());
    let mut destination = vec![0xa5; FLOOR_ITEM_COUNT * FLOOR_ITEM_SIZE];
    let mut context = ItemCtx::default();
    let report = load_floor_item_placements(&source, &mut destination, &mut context).unwrap();
    assert_eq!(report.active_records, 1);
    assert_eq!(context.calls, ["height", "rand"]);
    assert_eq!(
        u16::from_le_bytes(destination[0..2].try_into().unwrap()),
        0x1234
    );
    assert_eq!(
        i32::from_le_bytes(destination[4..8].try_into().unwrap()),
        6011
    );
    assert_eq!(
        i32::from_le_bytes(destination[8..12].try_into().unwrap()),
        400
    );
    assert_eq!(
        i32::from_le_bytes(destination[12..16].try_into().unwrap()),
        3993
    );
    assert_eq!(destination[20], 4);
    assert_eq!(&destination[16..20], &[0xa5; 4]);
    assert_eq!(&destination[21..24], &[0xa5; 3]);
    assert!(destination[FLOOR_ITEM_SIZE..]
        .iter()
        .all(|&byte| byte == 0xa5));
}

struct ActorCtx(Vec<Vec3i>);

impl ActorContext for ActorCtx {
    fn floor_height_at_position(&mut self, position: Vec3i) -> i32 {
        self.0.push(position);
        -777
    }
}

#[test]
fn actors_mark_the_terminator_and_tail_free_without_clobbering_other_fields() {
    let mut source = Vec::new();
    put(
        &mut source,
        ACTOR_PLACEMENT_SIZE,
        ActorPlacement {
            slot_state: 2,
            definition_flags: 0x35,
            heading_quadrant: 3,
            tile_z: 4,
            tile_x: 5,
            spawn_chance: 6,
            death_drop_object_id: 7,
            unknown_07: [8, 9, 10],
            local_z: -12,
            local_x: 13,
            unknown_0e: [14, 15],
        },
        |value, bytes| assert!(value.encode(bytes)),
    );
    source.push(0xff);
    let mut destination = vec![0x6a; ACTOR_COUNT * ACTOR_SIZE];
    let mut context = ActorCtx(Vec::new());
    let report = load_actor_placements(&source, &mut destination, &mut context).unwrap();
    assert_eq!(report.active_records, 1);
    assert_eq!(destination[0], 2);
    assert_eq!(destination[1], 0x15);
    assert_eq!(destination[2], 1);
    assert_eq!(destination[6], 0);
    assert_eq!(destination[7], 6);
    assert_eq!(destination[8], 0x6a);
    assert_eq!(destination[9], 7);
    assert_eq!(
        context.0,
        [Vec3i {
            x: 10013,
            y: 0x6a6a6a6a,
            z: 7988
        }]
    );
    assert_eq!(
        i32::from_le_bytes(destination[32..36].try_into().unwrap()),
        -777
    );
    for record in 1..ACTOR_COUNT {
        let at = record * ACTOR_SIZE;
        assert_eq!(destination[at], 0xff);
        assert_eq!(destination[at + 6], 0);
        assert!(destination[at + 1..at + 6].iter().all(|&byte| byte == 0x6a));
    }
}

#[derive(Default)]
struct EventCtx {
    heights: Vec<(u8, u8)>,
    occupancy: Vec<(u16, u16, i32)>,
}

impl MapEventContext for EventCtx {
    fn floor_height(&mut self, tile_z: u8, tile_x: u8) -> u8 {
        self.heights.push((tile_z, tile_x));
        3
    }

    fn adjust_occupancy(&mut self, cell_x: u16, cell_z: u16, amount: i32) {
        self.occupancy.push((cell_x, cell_z, amount));
    }
}

#[test]
fn events_preserve_opaque_fields_and_write_only_state_after_sentinel() {
    let mut source = Vec::new();
    put(
        &mut source,
        MAP_EVENT_DEFINITION_SIZE,
        MapEventDefinition {
            state: 1,
            kind: 2,
            variant: 3,
            cell_z: 4,
            cell_x: 5,
            tag: [6, 7, 8, 9, 10],
            image_limit: 11,
            unknown_0b: 12,
            unknown_0c: 13,
            behavior: 14,
            position_z_offset: -15,
            position_x_offset: 16,
            initial_rotation: 0xf234,
            radius: 18,
            unknown_16: 19,
        },
        |value, bytes| assert!(value.encode(bytes)),
    );
    source.push(0xff);
    let mut destination = vec![0xbc; MAP_EVENT_COUNT * MAP_EVENT_SIZE];
    let mut context = EventCtx::default();
    load_map_event_definitions(&source, &mut destination, &mut context).unwrap();
    assert_eq!(context.heights, [(4, 5)]);
    assert_eq!(context.occupancy, [(5, 4, 1)]);
    assert_eq!(&destination[3..8], &[6, 7, 8, 9, 10]);
    assert_eq!(
        i32::from_le_bytes(destination[20..24].try_into().unwrap()),
        10016
    );
    assert_eq!(
        i32::from_le_bytes(destination[24..28].try_into().unwrap()),
        7985
    );
    assert_eq!(
        i32::from_le_bytes(destination[40..44].try_into().unwrap()),
        -300
    );
    assert_eq!(destination[17], 0xbc);
    assert_eq!(
        u16::from_le_bytes(destination[34..36].try_into().unwrap()),
        0xbcbc
    );
    assert_eq!(&destination[48..52], &[0xbc; 4]);
    for record in 1..MAP_EVENT_COUNT {
        let at = record * MAP_EVENT_SIZE;
        assert_eq!(destination[at], 0xff);
        assert!(destination[at + 1..at + MAP_EVENT_SIZE]
            .iter()
            .all(|&byte| byte == 0xbc));
    }
}

#[derive(Default)]
struct ObjectCtx {
    calls: Vec<&'static str>,
    effects: Vec<EffectRequest>,
    snapshots: Vec<ObjectSnapshot>,
}

impl MapObjectContext for ObjectCtx {
    fn floor_height(&mut self, _tile_z: u8, _tile_x: u8) -> u8 {
        self.calls.push("height");
        2
    }

    fn adjust_occupancy(&mut self, _cell_x: u16, _cell_z: u16, _amount: i32) {
        self.calls.push("occupancy");
    }

    fn construct_effect(&mut self, request: EffectRequest) -> i32 {
        self.calls.push("effect");
        self.effects.push(request);
        0x123
    }

    fn mark_collision_edge(&mut self, object: ObjectSnapshot, value: u8, yaw: u16) {
        assert_eq!(value, 0);
        assert_eq!(yaw, object.rotation.y as u16);
        self.calls.push("edge");
        self.snapshots.push(object);
    }
}

#[test]
fn map_objects_apply_effect_action_and_behavior_rules_in_retail_order() {
    let ids = [136, 138, 135, 56, 7];
    let mut source = Vec::new();
    for (index, object_id) in ids.into_iter().enumerate() {
        put(
            &mut source,
            MAP_OBJECT_PLACEMENT_SIZE,
            MapObjectPlacement {
                object_id,
                unknown_01: 0xee,
                tile_z: 2,
                tile_x: 3,
                yaw: 0x3456,
                local_z: -5,
                local_x: 6,
                local_y: 700,
                link: MapObjectLink {
                    link_id: index as u8,
                    action_parameter: 0xaa,
                    spawn_sequence: 0xabcd,
                    vertical_velocity: -9,
                    unknown_06: [8, 7],
                },
            },
            |value, bytes| assert!(value.encode(bytes)),
        );
    }
    source.push(0xff);
    let mut definitions = vec![0; MAP_OBJECT_DEFINITION_COUNT * MAP_OBJECT_DEFINITION_SIZE];
    for object_id in ids {
        let at = usize::from(object_id) * MAP_OBJECT_DEFINITION_SIZE;
        definitions[at + 2..at + 4].copy_from_slice(&1u16.to_le_bytes());
    }
    definitions[7 * MAP_OBJECT_DEFINITION_SIZE] = 10;
    let mut destination = vec![0x5d; MAP_OBJECT_COUNT * MAP_OBJECT_SIZE];
    let mut context = ObjectCtx::default();
    load_map_object_placements(&source, &definitions, &mut destination, &mut context).unwrap();

    assert_eq!(context.effects.len(), 3);
    assert_eq!(context.effects[0].argument, 0xcd);
    assert_eq!(context.effects[0].effect_id, 0x11);
    assert_eq!(context.effects[0].rotation, None);
    assert_eq!(context.effects[1].effect_id, 0x0f);
    assert_eq!(context.effects[1].rotation.unwrap().y, 0x456);
    assert_eq!(context.effects[2].argument, 0);
    assert_eq!(context.effects[2].kind, 3);
    assert_eq!(context.snapshots.len(), 5);
    assert_eq!(
        context
            .snapshots
            .iter()
            .map(|item| item.action)
            .collect::<Vec<_>>(),
        [0x51, 0x51, 0x53, 0x0c, 10]
    );
    assert_eq!(context.snapshots[0].link.action_parameter, 0x23);
    assert_eq!(context.snapshots[3].position.y, 10_500);
    assert_eq!(
        &context.calls[..4],
        &["height", "occupancy", "effect", "edge"]
    );
    assert_eq!(destination[1], 0x5d);
    assert_eq!(&destination[6..8], &[0x5d; 2]);
    assert_eq!(&destination[20..24], &[0x5d; 4]);
    for record in ids.len()..MAP_OBJECT_COUNT {
        let at = record * MAP_OBJECT_SIZE;
        assert_eq!(destination[at], 0xff);
        assert!(destination[at + 1..at + MAP_OBJECT_SIZE]
            .iter()
            .all(|&byte| byte == 0x5d));
    }
}

#[test]
fn malformed_source_is_rejected_before_callbacks_or_writes() {
    let source = [1u8; MAP_OBJECT_PLACEMENT_SIZE - 1];
    let definitions = [0; MAP_OBJECT_DEFINITION_COUNT * MAP_OBJECT_DEFINITION_SIZE];
    let mut destination = vec![0x77; MAP_OBJECT_COUNT * MAP_OBJECT_SIZE];
    let mut context = ObjectCtx::default();
    assert!(matches!(
        load_map_object_placements(&source, &definitions, &mut destination, &mut context),
        Err(PlacementError::SourceTooSmall { .. })
    ));
    assert!(destination.iter().all(|&byte| byte == 0x77));
    assert!(context.calls.is_empty());
}

#[test]
fn full_capacity_actor_object_and_event_inputs_need_no_sentinel() {
    let mut actors = Vec::new();
    for index in 0..ACTOR_COUNT {
        put(
            &mut actors,
            ACTOR_PLACEMENT_SIZE,
            ActorPlacement {
                slot_state: 1 + index as u8 % 4,
                definition_flags: index as u8 & 0x3f,
                heading_quadrant: index as u8 & 3,
                tile_z: 10 + index as u8 % 7,
                tile_x: 20 + index as u8 % 7,
                spawn_chance: index as u8,
                death_drop_object_id: (index as u8).wrapping_mul(3),
                unknown_07: [0; 3],
                local_z: index as i16 - 64,
                local_x: 63 - index as i16,
                unknown_0e: [0; 2],
            },
            |value, bytes| assert!(value.encode(bytes)),
        );
    }
    let mut actor_output = vec![0xa5; ACTOR_COUNT * ACTOR_SIZE];
    let mut actor_context = ActorCtx(Vec::new());
    let actor_report =
        load_actor_placements(&actors, &mut actor_output, &mut actor_context).unwrap();
    assert_eq!(actor_report.active_records, ACTOR_COUNT as u16);
    assert_eq!(actor_report.consumed_bytes, actors.len());
    assert_eq!(actor_context.0.len(), ACTOR_COUNT);
    assert!((0..ACTOR_COUNT).all(|index| actor_output[index * ACTOR_SIZE] != 0xff));

    let mut objects = Vec::new();
    for index in 0..MAP_OBJECT_COUNT {
        put(
            &mut objects,
            MAP_OBJECT_PLACEMENT_SIZE,
            MapObjectPlacement {
                object_id: 1,
                unknown_01: index as u8,
                tile_z: 10 + index as u8 % 7,
                tile_x: 20 + index as u8 % 7,
                yaw: ((index & 3) << 10) as u16,
                local_z: index as i16 - 95,
                local_x: 94 - index as i16,
                local_y: index as i16 * 13 - 1000,
                link: MapObjectLink {
                    link_id: index as u8,
                    action_parameter: 0x5a,
                    spawn_sequence: 0x1200 + index as u16,
                    vertical_velocity: index as i16 - 40,
                    unknown_06: [index as u8, !(index as u8)],
                },
            },
            |value, bytes| assert!(value.encode(bytes)),
        );
    }
    let mut definitions = vec![0; MAP_OBJECT_DEFINITION_COUNT * MAP_OBJECT_DEFINITION_SIZE];
    definitions[MAP_OBJECT_DEFINITION_SIZE] = 1;
    let mut object_output = vec![0xa5; MAP_OBJECT_COUNT * MAP_OBJECT_SIZE];
    let mut object_context = ObjectCtx::default();
    let object_report = load_map_object_placements(
        &objects,
        &definitions,
        &mut object_output,
        &mut object_context,
    )
    .unwrap();
    assert_eq!(object_report.active_records, MAP_OBJECT_COUNT as u16);
    assert_eq!(object_report.consumed_bytes, objects.len());
    assert_eq!(object_context.snapshots.len(), MAP_OBJECT_COUNT);
    assert!((0..MAP_OBJECT_COUNT).all(|index| object_output[index * MAP_OBJECT_SIZE] != 0xff));

    let mut events = Vec::new();
    for index in 0..MAP_EVENT_COUNT {
        put(
            &mut events,
            MAP_EVENT_DEFINITION_SIZE,
            MapEventDefinition {
                state: 1 + index as u8,
                kind: 2 + index as u8,
                variant: 3 + index as u8,
                cell_z: 10 + index as u8,
                cell_x: 20 + index as u8,
                tag: [index as u8; 5],
                image_limit: 30 + index as u8,
                unknown_0b: 40 + index as u8,
                unknown_0c: 50 + index as u8,
                behavior: 60 + index as u8,
                position_z_offset: index as i16 - 4,
                position_x_offset: 4 - index as i16,
                initial_rotation: index as u16 * 0x400,
                radius: 100 + index as u16,
                unknown_16: 200 + index as u16,
            },
            |value, bytes| assert!(value.encode(bytes)),
        );
    }
    let mut event_output = vec![0xa5; MAP_EVENT_COUNT * MAP_EVENT_SIZE];
    let mut event_context = EventCtx::default();
    let event_report =
        load_map_event_definitions(&events, &mut event_output, &mut event_context).unwrap();
    assert_eq!(event_report.active_records, MAP_EVENT_COUNT as u16);
    assert_eq!(event_report.consumed_bytes, events.len());
    assert_eq!(event_context.occupancy.len(), MAP_EVENT_COUNT);
    assert!((0..MAP_EVENT_COUNT).all(|index| event_output[index * MAP_EVENT_SIZE] != 0xff));
}

#[test]
fn map_object_id_123_starts_action_0b() {
    let mut source = Vec::new();
    put(
        &mut source,
        MAP_OBJECT_PLACEMENT_SIZE,
        MapObjectPlacement {
            object_id: 123,
            unknown_01: 0,
            tile_z: 10,
            tile_x: 20,
            yaw: 0x400,
            local_z: -3,
            local_x: 4,
            local_y: 5,
            link: MapObjectLink {
                link_id: 1,
                action_parameter: 2,
                spawn_sequence: 3,
                vertical_velocity: 4,
                unknown_06: [5, 6],
            },
        },
        |value, bytes| assert!(value.encode(bytes)),
    );
    source.push(0xff);
    let mut definitions = vec![0; MAP_OBJECT_DEFINITION_COUNT * MAP_OBJECT_DEFINITION_SIZE];
    definitions[123 * MAP_OBJECT_DEFINITION_SIZE] = 1;
    let mut destination = vec![0xa5; MAP_OBJECT_COUNT * MAP_OBJECT_SIZE];
    let mut context = ObjectCtx::default();
    let report =
        load_map_object_placements(&source, &definitions, &mut destination, &mut context).unwrap();
    assert_eq!(report.active_records, 1);
    assert_eq!(destination[40], 0x0b);
    assert_eq!(
        u16::from_le_bytes(destination[42..44].try_into().unwrap()),
        0
    );
}

struct RetailContext<'a> {
    floor_heights: &'a [u8],
}

impl RetailContext<'_> {
    fn height(&self, tile_z: u8, tile_x: u8) -> u8 {
        self.floor_heights[usize::from(tile_z) * 100 + usize::from(tile_x)]
    }
}

impl FloorItemContext for RetailContext<'_> {
    fn floor_height(&mut self, tile_z: u8, tile_x: u8) -> u8 {
        self.height(tile_z, tile_x)
    }

    fn rand(&mut self) -> i32 {
        0x4321
    }
}

impl ActorContext for RetailContext<'_> {
    fn floor_height_at_position(&mut self, _position: Vec3i) -> i32 {
        // The differential host supplies the exact collision/orientation
        // helper. Corpus validation only exercises the placement grammar.
        0
    }
}

impl MapEventContext for RetailContext<'_> {
    fn floor_height(&mut self, tile_z: u8, tile_x: u8) -> u8 {
        self.height(tile_z, tile_x)
    }

    fn adjust_occupancy(&mut self, _cell_x: u16, _cell_z: u16, _amount: i32) {}
}

impl MapObjectContext for RetailContext<'_> {
    fn floor_height(&mut self, tile_z: u8, tile_x: u8) -> u8 {
        self.height(tile_z, tile_x)
    }

    fn adjust_occupancy(&mut self, _cell_x: u16, _cell_z: u16, _amount: i32) {}

    fn construct_effect(&mut self, _request: EffectRequest) -> i32 {
        0
    }

    fn mark_collision_edge(&mut self, _object: ObjectSnapshot, _value: u8, _yaw: u16) {}
}

#[test]
#[ignore = "requires proprietary SLPS-00017 files via KF_RETAIL_DIR"]
fn all_five_retail_mixa_placement_sets_expand() {
    use kf_codec::game_data::{CommonData, MixA};

    let configured =
        std::path::PathBuf::from(std::env::var_os("KF_RETAIL_DIR").expect("set KF_RETAIL_DIR"));
    let root = if configured.join("KF").is_dir() {
        configured.join("KF")
    } else {
        configured
    };
    let common_bytes = std::fs::read(root.join("COM/COM.DAT")).unwrap();
    let common = CommonData::parse(&common_bytes).unwrap();
    for floor in 1..=5 {
        let bytes = std::fs::read(root.join(format!("B{floor}/MIXA.DAT"))).unwrap();
        let mixa = MixA::parse(&bytes).unwrap();
        let mut context = RetailContext {
            floor_heights: mixa.map_grids.floor_heights,
        };
        let mut items = vec![0x4a; FLOOR_ITEM_COUNT * FLOOR_ITEM_SIZE];
        let mut actors = vec![0x4a; ACTOR_COUNT * ACTOR_SIZE];
        let mut objects = vec![0x4a; MAP_OBJECT_COUNT * MAP_OBJECT_SIZE];
        let mut events = vec![0x4a; MAP_EVENT_COUNT * MAP_EVENT_SIZE];
        load_floor_item_placements(mixa.floor_item_placements, &mut items, &mut context).unwrap();
        load_actor_placements(mixa.actor_placements, &mut actors, &mut context).unwrap();
        load_map_object_placements(
            mixa.map_object_placements,
            common.map_object_definitions_source,
            &mut objects,
            &mut context,
        )
        .unwrap();
        load_map_event_definitions(mixa.map_event_definitions, &mut events, &mut context).unwrap();
        let mut loaded_actor_definitions =
            vec![0x4a; kf_codec::records::ACTOR_DEFINITIONS_SIZE + 7];
        kf_codec::records::load_actor_definitions(
            mixa.actor_definitions,
            &mut loaded_actor_definitions,
        )
        .unwrap();
        assert!(
            loaded_actor_definitions[kf_codec::records::ACTOR_DEFINITIONS_SIZE..]
                .iter()
                .all(|&byte| byte == 0x4a)
        );
    }
}
