//! Expansion of MIXA placement records into caller-initialized runtime pools.
//!
//! The retail routines initialize only selected fields. These codecs write the
//! same fields by explicit little-endian offsets and retain all other bytes.

pub const FLOOR_ITEM_PLACEMENT_SIZE: usize = 12;
pub const FLOOR_ITEM_SIZE: usize = 24;
pub const FLOOR_ITEM_COUNT: usize = 64;
pub const ACTOR_PLACEMENT_SIZE: usize = 16;
pub const ACTOR_SIZE: usize = 72;
pub const ACTOR_COUNT: usize = 128;
pub const MAP_OBJECT_PLACEMENT_SIZE: usize = 20;
pub const MAP_OBJECT_SIZE: usize = 44;
pub const MAP_OBJECT_COUNT: usize = 190;
pub const MAP_OBJECT_DEFINITION_SIZE: usize = 8;
pub const MAP_OBJECT_DEFINITION_COUNT: usize = 160;
pub const MAP_EVENT_DEFINITION_SIZE: usize = 24;
pub const MAP_EVENT_SIZE: usize = 68;
pub const MAP_EVENT_COUNT: usize = 8;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum PlacementError {
    SourceTooSmall {
        at: usize,
        needed: usize,
        available: usize,
    },
    DestinationTooSmall {
        needed: usize,
        available: usize,
    },
    DefinitionsTooSmall {
        needed: usize,
        available: usize,
    },
    MissingItemTerminator {
        maximum_records: usize,
    },
    InvalidObjectId {
        record: usize,
        object_id: u8,
    },
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct PlacementReport {
    pub active_records: u16,
    pub consumed_bytes: usize,
}

#[derive(Clone, Copy, Debug, Default, Eq, PartialEq)]
pub struct Vec3i {
    pub x: i32,
    pub y: i32,
    pub z: i32,
}

#[derive(Clone, Copy, Debug, Default, Eq, PartialEq)]
pub struct EulerAngles {
    pub x: i16,
    pub y: i16,
    pub z: i16,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct FloorItemPlacement {
    pub item_id: u16,
    pub facing_and_frame_count: u8,
    pub unknown_03: u8,
    pub tile_z: u8,
    pub tile_x: u8,
    pub local_z: i16,
    pub local_x: i16,
    pub local_y: i16,
}

impl FloorItemPlacement {
    pub fn decode(bytes: &[u8]) -> Option<Self> {
        bytes.get(..FLOOR_ITEM_PLACEMENT_SIZE)?;
        Some(Self {
            item_id: read_u16(bytes, 0),
            facing_and_frame_count: bytes[2],
            unknown_03: bytes[3],
            tile_z: bytes[4],
            tile_x: bytes[5],
            local_z: read_i16(bytes, 6),
            local_x: read_i16(bytes, 8),
            local_y: read_i16(bytes, 10),
        })
    }

    pub fn encode(&self, bytes: &mut [u8]) -> bool {
        let Some(bytes) = bytes.get_mut(..FLOOR_ITEM_PLACEMENT_SIZE) else {
            return false;
        };
        write_u16(bytes, 0, self.item_id);
        bytes[2] = self.facing_and_frame_count;
        bytes[3] = self.unknown_03;
        bytes[4] = self.tile_z;
        bytes[5] = self.tile_x;
        write_i16(bytes, 6, self.local_z);
        write_i16(bytes, 8, self.local_x);
        write_i16(bytes, 10, self.local_y);
        true
    }
}

pub trait FloorItemContext {
    fn floor_height(&mut self, tile_z: u8, tile_x: u8) -> u8;
    fn rand(&mut self) -> i32;
}

pub fn load_floor_item_placements<C: FloorItemContext>(
    source: &[u8],
    destination: &mut [u8],
    context: &mut C,
) -> Result<PlacementReport, PlacementError> {
    require_destination(destination, FLOOR_ITEM_SIZE * FLOOR_ITEM_COUNT)?;
    let active = match find_sentinel(source, FLOOR_ITEM_PLACEMENT_SIZE, 2, FLOOR_ITEM_COUNT)? {
        Some(active) => active,
        None => {
            let at = FLOOR_ITEM_COUNT * FLOOR_ITEM_PLACEMENT_SIZE;
            if source.len() < at + 2 {
                return Err(PlacementError::SourceTooSmall {
                    at,
                    needed: 2,
                    available: source.len().saturating_sub(at),
                });
            }
            if read_u16(source, at) != 0xffff {
                return Err(PlacementError::MissingItemTerminator {
                    maximum_records: FLOOR_ITEM_COUNT,
                });
            }
            FLOOR_ITEM_COUNT
        }
    };

    for index in 0..active {
        let source_at = index * FLOOR_ITEM_PLACEMENT_SIZE;
        let placement =
            FloorItemPlacement::decode(&source[source_at..source_at + FLOOR_ITEM_PLACEMENT_SIZE])
                .expect("preflighted placement");
        let output_at = index * FLOOR_ITEM_SIZE;
        let output = &mut destination[output_at..output_at + FLOOR_ITEM_SIZE];
        write_u16(output, 0, placement.item_id);
        output[2] = placement.facing_and_frame_count;
        output[3] = placement.unknown_03;
        let x = world_coordinate(placement.tile_x, placement.local_x);
        let z = world_coordinate(placement.tile_z, placement.local_z);
        write_i32(output, 4, x);
        write_i32(
            output,
            8,
            i32::from(placement.local_y).wrapping_sub(
                i32::from(context.floor_height(placement.tile_z, placement.tile_x)) * 100,
            ),
        );
        write_i32(output, 12, z);
        output[20] = context
            .rand()
            .wrapping_mul(i32::from(placement.facing_and_frame_count))
            .wrapping_shr(15) as u8;
    }
    Ok(PlacementReport {
        active_records: active as u16,
        consumed_bytes: active * FLOOR_ITEM_PLACEMENT_SIZE + 2,
    })
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct ActorPlacement {
    pub slot_state: u8,
    pub definition_flags: u8,
    pub heading_quadrant: u8,
    pub tile_z: u8,
    pub tile_x: u8,
    pub spawn_chance: u8,
    pub death_drop_object_id: u8,
    pub unknown_07: [u8; 3],
    pub local_z: i16,
    pub local_x: i16,
    pub unknown_0e: [u8; 2],
}

impl ActorPlacement {
    pub fn decode(bytes: &[u8]) -> Option<Self> {
        bytes.get(..ACTOR_PLACEMENT_SIZE)?;
        Some(Self {
            slot_state: bytes[0],
            definition_flags: bytes[1],
            heading_quadrant: bytes[2],
            tile_z: bytes[3],
            tile_x: bytes[4],
            spawn_chance: bytes[5],
            death_drop_object_id: bytes[6],
            unknown_07: copy_array(bytes, 7),
            local_z: read_i16(bytes, 10),
            local_x: read_i16(bytes, 12),
            unknown_0e: copy_array(bytes, 14),
        })
    }

    pub fn encode(&self, bytes: &mut [u8]) -> bool {
        let Some(bytes) = bytes.get_mut(..ACTOR_PLACEMENT_SIZE) else {
            return false;
        };
        bytes[0] = self.slot_state;
        bytes[1] = self.definition_flags;
        bytes[2] = self.heading_quadrant;
        bytes[3] = self.tile_z;
        bytes[4] = self.tile_x;
        bytes[5] = self.spawn_chance;
        bytes[6] = self.death_drop_object_id;
        bytes[7..10].copy_from_slice(&self.unknown_07);
        write_i16(bytes, 10, self.local_z);
        write_i16(bytes, 12, self.local_x);
        bytes[14..16].copy_from_slice(&self.unknown_0e);
        true
    }
}

pub trait ActorContext {
    /// Models `map_floor_height_at_position`; implementations may apply the
    /// collision/orientation ramp correction before returning world Y.
    fn floor_height_at_position(&mut self, position: Vec3i) -> i32;
}

pub fn load_actor_placements<C: ActorContext>(
    source: &[u8],
    destination: &mut [u8],
    context: &mut C,
) -> Result<PlacementReport, PlacementError> {
    require_destination(destination, ACTOR_SIZE * ACTOR_COUNT)?;
    let active =
        find_sentinel(source, ACTOR_PLACEMENT_SIZE, 1, ACTOR_COUNT)?.unwrap_or(ACTOR_COUNT);

    for index in 0..ACTOR_COUNT {
        let output_at = index * ACTOR_SIZE;
        let output = &mut destination[output_at..output_at + ACTOR_SIZE];
        if index >= active {
            output[0] = 0xff;
            output[6] = 0;
            continue;
        }
        let source_at = index * ACTOR_PLACEMENT_SIZE;
        let placement =
            ActorPlacement::decode(&source[source_at..source_at + ACTOR_PLACEMENT_SIZE])
                .expect("preflighted placement");
        output[0] = placement.slot_state;
        output[1] = placement.definition_flags & 0x1f;
        output[2] = u8::from(placement.definition_flags & 0x20 != 0);
        output[3] = placement.heading_quadrant;
        output[4] = placement.tile_z;
        output[5] = placement.tile_x;
        output[7] = placement.spawn_chance;
        output[9] = placement.death_drop_object_id;
        write_i16(output, 14, placement.local_z);
        write_i16(output, 16, placement.local_x);
        output[6] = 0;
        let z = world_coordinate(placement.tile_z, placement.local_z);
        let x = world_coordinate(placement.tile_x, placement.local_x);
        write_i32(output, 36, z);
        write_i32(output, 28, x);
        let position = Vec3i {
            x,
            y: read_i32(output, 32),
            z,
        };
        write_i32(output, 32, context.floor_height_at_position(position));
        write_u16(output, 22, u16::from(placement.tile_x));
        write_u16(output, 24, u16::from(placement.tile_z));
    }
    Ok(PlacementReport {
        active_records: active as u16,
        consumed_bytes: active * ACTOR_PLACEMENT_SIZE + usize::from(active < ACTOR_COUNT),
    })
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct MapEventDefinition {
    pub state: u8,
    pub kind: u8,
    pub variant: u8,
    pub cell_z: u8,
    pub cell_x: u8,
    pub tag: [u8; 5],
    pub image_limit: u8,
    pub unknown_0b: u8,
    pub unknown_0c: u8,
    pub behavior: u8,
    pub position_z_offset: i16,
    pub position_x_offset: i16,
    pub initial_rotation: u16,
    pub radius: u16,
    pub unknown_16: u16,
}

impl MapEventDefinition {
    pub fn decode(bytes: &[u8]) -> Option<Self> {
        bytes.get(..MAP_EVENT_DEFINITION_SIZE)?;
        Some(Self {
            state: bytes[0],
            kind: bytes[1],
            variant: bytes[2],
            cell_z: bytes[3],
            cell_x: bytes[4],
            tag: copy_array(bytes, 5),
            image_limit: bytes[10],
            unknown_0b: bytes[11],
            unknown_0c: bytes[12],
            behavior: bytes[13],
            position_z_offset: read_i16(bytes, 14),
            position_x_offset: read_i16(bytes, 16),
            initial_rotation: read_u16(bytes, 18),
            radius: read_u16(bytes, 20),
            unknown_16: read_u16(bytes, 22),
        })
    }

    pub fn encode(&self, bytes: &mut [u8]) -> bool {
        let Some(bytes) = bytes.get_mut(..MAP_EVENT_DEFINITION_SIZE) else {
            return false;
        };
        bytes[0] = self.state;
        bytes[1] = self.kind;
        bytes[2] = self.variant;
        bytes[3] = self.cell_z;
        bytes[4] = self.cell_x;
        bytes[5..10].copy_from_slice(&self.tag);
        bytes[10] = self.image_limit;
        bytes[11] = self.unknown_0b;
        bytes[12] = self.unknown_0c;
        bytes[13] = self.behavior;
        write_i16(bytes, 14, self.position_z_offset);
        write_i16(bytes, 16, self.position_x_offset);
        write_u16(bytes, 18, self.initial_rotation);
        write_u16(bytes, 20, self.radius);
        write_u16(bytes, 22, self.unknown_16);
        true
    }
}

pub trait MapEventContext {
    fn floor_height(&mut self, tile_z: u8, tile_x: u8) -> u8;
    fn adjust_occupancy(&mut self, cell_x: u16, cell_z: u16, amount: i32);
}

pub fn load_map_event_definitions<C: MapEventContext>(
    source: &[u8],
    destination: &mut [u8],
    context: &mut C,
) -> Result<PlacementReport, PlacementError> {
    require_destination(destination, MAP_EVENT_SIZE * MAP_EVENT_COUNT)?;
    let active = find_sentinel(source, MAP_EVENT_DEFINITION_SIZE, 1, MAP_EVENT_COUNT)?
        .unwrap_or(MAP_EVENT_COUNT);
    for index in 0..MAP_EVENT_COUNT {
        let output = &mut destination[index * MAP_EVENT_SIZE..(index + 1) * MAP_EVENT_SIZE];
        if index >= active {
            output[0] = 0xff;
            continue;
        }
        let input =
            &source[index * MAP_EVENT_DEFINITION_SIZE..(index + 1) * MAP_EVENT_DEFINITION_SIZE];
        let definition = MapEventDefinition::decode(input).expect("preflighted definition");
        output[0] = definition.state;
        output[1] = definition.kind;
        output[2] = definition.variant;
        output[3..8].copy_from_slice(&definition.tag);
        output[8] = definition.image_limit;
        output[12] = definition.unknown_0b;
        output[13] = definition.unknown_0c;
        output[14] = definition.behavior;
        let x = world_coordinate(definition.cell_x, definition.position_x_offset);
        let z = world_coordinate(definition.cell_z, definition.position_z_offset);
        write_i32(output, 20, x);
        write_i32(output, 36, x);
        write_i32(output, 24, z);
        write_i32(output, 44, z);
        write_u16(output, 28, u16::from(definition.cell_x));
        write_u16(output, 30, u16::from(definition.cell_z));
        write_u16(output, 32, definition.radius);
        write_i32(
            output,
            40,
            -(i32::from(context.floor_height(definition.cell_z, definition.cell_x)) * 100),
        );
        write_i16(output, 54, definition.initial_rotation as i16);
        write_u16(output, 56, 0);
        write_u16(output, 52, 0);
        output[10] = 1;
        output[9] = 1;
        output[11] = 0;
        output[15] = 0;
        write_u16(output, 18, 0);
        write_i16(output, 64, 0);
        output[16] = 0;
        context.adjust_occupancy(
            u16::from(definition.cell_x),
            u16::from(definition.cell_z),
            1,
        );
    }
    Ok(PlacementReport {
        active_records: active as u16,
        consumed_bytes: active * MAP_EVENT_DEFINITION_SIZE + usize::from(active < MAP_EVENT_COUNT),
    })
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct MapObjectLink {
    pub link_id: u8,
    pub action_parameter: u8,
    pub spawn_sequence: u16,
    pub vertical_velocity: i16,
    pub unknown_06: [u8; 2],
}

impl MapObjectLink {
    fn decode(bytes: &[u8]) -> Self {
        Self {
            link_id: bytes[0],
            action_parameter: bytes[1],
            spawn_sequence: read_u16(bytes, 2),
            vertical_velocity: read_i16(bytes, 4),
            unknown_06: copy_array(bytes, 6),
        }
    }

    fn encode(&self, bytes: &mut [u8]) {
        bytes[0] = self.link_id;
        bytes[1] = self.action_parameter;
        write_u16(bytes, 2, self.spawn_sequence);
        write_i16(bytes, 4, self.vertical_velocity);
        bytes[6..8].copy_from_slice(&self.unknown_06);
    }
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct MapObjectPlacement {
    pub object_id: u8,
    pub unknown_01: u8,
    pub tile_z: u8,
    pub tile_x: u8,
    pub yaw: u16,
    pub local_z: i16,
    pub local_x: i16,
    pub local_y: i16,
    pub link: MapObjectLink,
}

impl MapObjectPlacement {
    pub fn decode(bytes: &[u8]) -> Option<Self> {
        bytes.get(..MAP_OBJECT_PLACEMENT_SIZE)?;
        Some(Self {
            object_id: bytes[0],
            unknown_01: bytes[1],
            tile_z: bytes[2],
            tile_x: bytes[3],
            yaw: read_u16(bytes, 4),
            local_z: read_i16(bytes, 6),
            local_x: read_i16(bytes, 8),
            local_y: read_i16(bytes, 10),
            link: MapObjectLink::decode(&bytes[12..20]),
        })
    }

    pub fn encode(&self, bytes: &mut [u8]) -> bool {
        let Some(bytes) = bytes.get_mut(..MAP_OBJECT_PLACEMENT_SIZE) else {
            return false;
        };
        bytes[0] = self.object_id;
        bytes[1] = self.unknown_01;
        bytes[2] = self.tile_z;
        bytes[3] = self.tile_x;
        write_u16(bytes, 4, self.yaw);
        write_i16(bytes, 6, self.local_z);
        write_i16(bytes, 8, self.local_x);
        write_i16(bytes, 10, self.local_y);
        self.link.encode(&mut bytes[12..20]);
        true
    }
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct EffectRequest {
    pub argument: u8,
    pub kind: u8,
    pub effect_id: u8,
    pub position: Vec3i,
    /// Retail passes this argument only for object ids 135, 138, and 139.
    pub rotation: Option<EulerAngles>,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct ObjectSnapshot {
    pub pool_index: u16,
    pub object_id: u8,
    pub cell_x: u16,
    pub cell_z: u16,
    pub position: Vec3i,
    pub rotation: EulerAngles,
    pub link: MapObjectLink,
    pub action: u8,
    pub action_timer: u16,
    pub behavior_type: u8,
}

pub trait MapObjectContext {
    fn floor_height(&mut self, tile_z: u8, tile_x: u8) -> u8;
    fn adjust_occupancy(&mut self, cell_x: u16, cell_z: u16, amount: i32);
    /// Returns the effect-pool index; retail stores its low byte in the link.
    fn construct_effect(&mut self, request: EffectRequest) -> i32;
    fn mark_collision_edge(&mut self, object: ObjectSnapshot, value: u8, yaw: u16);
}

pub fn load_map_object_placements<C: MapObjectContext>(
    source: &[u8],
    definitions: &[u8],
    destination: &mut [u8],
    context: &mut C,
) -> Result<PlacementReport, PlacementError> {
    require_destination(destination, MAP_OBJECT_SIZE * MAP_OBJECT_COUNT)?;
    if definitions.len() < MAP_OBJECT_DEFINITION_SIZE * MAP_OBJECT_DEFINITION_COUNT {
        return Err(PlacementError::DefinitionsTooSmall {
            needed: MAP_OBJECT_DEFINITION_SIZE * MAP_OBJECT_DEFINITION_COUNT,
            available: definitions.len(),
        });
    }
    let active = find_sentinel(source, MAP_OBJECT_PLACEMENT_SIZE, 1, MAP_OBJECT_COUNT)?
        .unwrap_or(MAP_OBJECT_COUNT);
    for index in 0..active {
        let object_id = source[index * MAP_OBJECT_PLACEMENT_SIZE];
        if usize::from(object_id) >= MAP_OBJECT_DEFINITION_COUNT {
            return Err(PlacementError::InvalidObjectId {
                record: index,
                object_id,
            });
        }
    }

    for index in 0..MAP_OBJECT_COUNT {
        let output = &mut destination[index * MAP_OBJECT_SIZE..(index + 1) * MAP_OBJECT_SIZE];
        if index >= active {
            output[0] = 0xff;
            continue;
        }
        let input =
            &source[index * MAP_OBJECT_PLACEMENT_SIZE..(index + 1) * MAP_OBJECT_PLACEMENT_SIZE];
        let placement = MapObjectPlacement::decode(input).expect("preflighted placement");
        let definition_at = usize::from(placement.object_id) * MAP_OBJECT_DEFINITION_SIZE;
        let behavior_type = definitions[definition_at];
        let collision_radius = read_u16(definitions, definition_at + 2);
        output[0] = placement.object_id;
        write_u16(output, 2, u16::from(placement.tile_x));
        write_u16(output, 4, u16::from(placement.tile_z));
        write_i16(output, 28, 0);
        write_i16(output, 24, 0);
        let yaw = placement.yaw & 0x0fff;
        write_i16(output, 26, yaw as i16);
        let x = world_coordinate(placement.tile_x, placement.local_x);
        let z = world_coordinate(placement.tile_z, placement.local_z);
        let y = i32::from(placement.local_y).wrapping_sub(
            i32::from(context.floor_height(placement.tile_z, placement.tile_x)) * 100,
        );
        write_i32(output, 8, x);
        write_i32(output, 16, z);
        write_i32(output, 12, y);
        output[40] = 0xff;
        output[32..40].copy_from_slice(&input[12..20]);

        if collision_radius != 0 {
            context.adjust_occupancy(u16::from(placement.tile_x), u16::from(placement.tile_z), 1);
        }

        match placement.object_id {
            136 => {
                construct_and_store(
                    output,
                    context,
                    EffectRequest {
                        argument: placement.link.spawn_sequence.to_le_bytes()[0],
                        kind: 0x23,
                        effect_id: 0x11,
                        position: Vec3i { x, y, z },
                        rotation: None,
                    },
                );
                start_action_if_idle(output, 0x51);
            }
            115 | 124 | 125 | 137 => start_action_if_idle(output, 0x50),
            138 | 139 => {
                let effect_id = if placement.object_id == 138 {
                    0x0f
                } else {
                    0x10
                };
                construct_and_store(
                    output,
                    context,
                    EffectRequest {
                        argument: placement.link.spawn_sequence.to_le_bytes()[0],
                        kind: 0x23,
                        effect_id,
                        position: Vec3i { x, y, z },
                        rotation: Some(EulerAngles {
                            x: 0,
                            y: yaw as i16,
                            z: 0,
                        }),
                    },
                );
                start_action_if_idle(
                    output,
                    if placement.object_id == 138 {
                        0x51
                    } else {
                        0x52
                    },
                );
            }
            135 => {
                construct_and_store(
                    output,
                    context,
                    EffectRequest {
                        argument: 0,
                        kind: 3,
                        effect_id: 0x30,
                        position: Vec3i { x, y, z },
                        rotation: Some(EulerAngles {
                            x: 0,
                            y: yaw as i16,
                            z: 0,
                        }),
                    },
                );
                start_action_if_idle(output, 0x53);
            }
            56 | 63 | 64 | 68 | 69 => {
                start_action_if_idle(output, 0x0c);
                let raised = read_i32(output, 12).wrapping_add(10_000);
                write_i32(output, 12, raised);
            }
            111 | 123 => start_action_if_idle(output, 0x0b),
            _ => {}
        }
        if behavior_type == 10 {
            start_action_if_idle(output, 10);
        }
        context.mark_collision_edge(
            snapshot(index, output, behavior_type),
            0,
            read_u16(output, 26),
        );
    }
    Ok(PlacementReport {
        active_records: active as u16,
        consumed_bytes: active * MAP_OBJECT_PLACEMENT_SIZE + usize::from(active < MAP_OBJECT_COUNT),
    })
}

fn construct_and_store<C: MapObjectContext>(
    output: &mut [u8],
    context: &mut C,
    request: EffectRequest,
) {
    output[33] = context.construct_effect(request).to_le_bytes()[0];
}

fn start_action_if_idle(output: &mut [u8], action: u8) {
    if output[40] == 0xff {
        output[40] = action;
        write_u16(output, 42, 0);
    }
}

fn snapshot(index: usize, output: &[u8], behavior_type: u8) -> ObjectSnapshot {
    ObjectSnapshot {
        pool_index: index as u16,
        object_id: output[0],
        cell_x: read_u16(output, 2),
        cell_z: read_u16(output, 4),
        position: Vec3i {
            x: read_i32(output, 8),
            y: read_i32(output, 12),
            z: read_i32(output, 16),
        },
        rotation: EulerAngles {
            x: read_i16(output, 24),
            y: read_i16(output, 26),
            z: read_i16(output, 28),
        },
        link: MapObjectLink::decode(&output[32..40]),
        action: output[40],
        action_timer: read_u16(output, 42),
        behavior_type,
    }
}

fn find_sentinel(
    source: &[u8],
    stride: usize,
    marker_width: usize,
    maximum_records: usize,
) -> Result<Option<usize>, PlacementError> {
    for index in 0..maximum_records {
        let at = index * stride;
        let marker_end = at + marker_width;
        if source.len() < marker_end {
            return Err(PlacementError::SourceTooSmall {
                at,
                needed: marker_width,
                available: source.len().saturating_sub(at),
            });
        }
        let sentinel = if marker_width == 2 {
            read_u16(source, at) == 0xffff
        } else {
            source[at] == 0xff
        };
        if sentinel {
            return Ok(Some(index));
        }
        if source.len() < at + stride {
            return Err(PlacementError::SourceTooSmall {
                at,
                needed: stride,
                available: source.len().saturating_sub(at),
            });
        }
    }
    Ok(None)
}

fn require_destination(destination: &[u8], needed: usize) -> Result<(), PlacementError> {
    if destination.len() < needed {
        Err(PlacementError::DestinationTooSmall {
            needed,
            available: destination.len(),
        })
    } else {
        Ok(())
    }
}

fn world_coordinate(tile: u8, local: i16) -> i32 {
    i32::from(tile) * 2_000 + i32::from(local)
}

fn read_u16(bytes: &[u8], at: usize) -> u16 {
    u16::from_le_bytes([bytes[at], bytes[at + 1]])
}

fn read_i16(bytes: &[u8], at: usize) -> i16 {
    i16::from_le_bytes([bytes[at], bytes[at + 1]])
}

fn read_i32(bytes: &[u8], at: usize) -> i32 {
    i32::from_le_bytes([bytes[at], bytes[at + 1], bytes[at + 2], bytes[at + 3]])
}

fn write_u16(bytes: &mut [u8], at: usize, value: u16) {
    bytes[at..at + 2].copy_from_slice(&value.to_le_bytes());
}

fn write_i16(bytes: &mut [u8], at: usize, value: i16) {
    bytes[at..at + 2].copy_from_slice(&value.to_le_bytes());
}

fn write_i32(bytes: &mut [u8], at: usize, value: i32) {
    bytes[at..at + 4].copy_from_slice(&value.to_le_bytes());
}

fn copy_array<const N: usize>(bytes: &[u8], at: usize) -> [u8; N] {
    let mut output = [0; N];
    output.copy_from_slice(&bytes[at..at + N]);
    output
}
