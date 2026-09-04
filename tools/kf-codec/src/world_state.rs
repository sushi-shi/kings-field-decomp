//! Expansion of one serialized per-floor world-state record.
//!
//! This models `GAME.EXE` `map_restore_floor_state` (0x80035e44). The record
//! lives on a 1,700-byte stride, but the routine reads only its encoded prefix;
//! this API therefore validates the actually consumed bytes, including counts.

pub const FLOOR_RECORD_STRIDE: usize = 1_700;
pub const MAP_EVENT_SIZE: usize = 68;
pub const MAP_EVENT_COUNT: usize = 8;
pub const ACTOR_SIZE: usize = 72;
pub const ACTOR_COUNT: usize = 128;
pub const MAP_OBJECT_SIZE: usize = 44;
pub const MAP_OBJECT_COUNT: usize = 190;
pub const ACTOR_STATE_HEAD_SIZE: usize = 0x442;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum RestoreError {
    SourceTooSmall {
        at: usize,
        needed: usize,
        available: usize,
    },
    DestinationTooSmall {
        destination: RestoreDestination,
        needed: usize,
        available: usize,
    },
    ActorIndexOutOfRange {
        entry: usize,
        index: u8,
    },
    ObjectIndexOutOfRange {
        entry: usize,
        index: u8,
    },
    EventTagWriteOutOfRange {
        event: usize,
        image_index: u8,
    },
    CallbackActorIndexOutOfRange {
        index: i32,
    },
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum RestoreDestination {
    EventPool,
    ActorStateHead,
    ActorPool,
    ObjectPool,
}

pub struct WorldStateDestinations<'a> {
    pub event_pool: &'a mut [u8],
    /// Prefix of `KfActorState`, used by four direct floor-five byte writes.
    pub actor_state_head: &'a mut [u8],
    pub actor_pool: &'a mut [u8],
    pub object_pool: &'a mut [u8],
}

#[derive(Clone, Copy, Debug, Default, Eq, PartialEq)]
pub struct RestoreConditions {
    pub current_floor: u8,
    pub highest_floor: u8,
    pub world_byte_1: u8,
    pub world_byte_2: u8,
    pub flag_8009eafc: u8,
    pub flag_8009f844: u8,
    pub flag_8009f845: u8,
    pub flag_8009f846: u8,
    pub inventory_0a: u8,
    pub inventory_0b: u8,
    pub boss_defeat_complete: u8,
}

pub trait RestoreContext {
    fn rand(&mut self) -> i32;
    fn floor_height(&mut self, cell_z: u16, cell_x: u16) -> u8;
    fn apply_copy_region(&mut self, region: u8);
    fn find_actor_at_tile(&mut self, tile_x: u8, tile_z: u8) -> i32;
    fn clear_object_link(&mut self, link_id: u8);
    fn trigger_object_link(&mut self, link_id: u8);
    fn begin_actor_death_by_definition(&mut self, definition_id: u16);
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct RestoreReport {
    pub marker_was_set: bool,
    pub actor_overrides: u8,
    pub link_overrides: u8,
    pub consumed_bytes: usize,
}

#[derive(Clone, Copy)]
struct RestoreLayout {
    actor_count: usize,
    actor_entries: usize,
    object_ids: usize,
    link_count: usize,
    link_entries: usize,
    effect_160: usize,
    effect_170: usize,
    consumed: usize,
}

pub fn restore_floor_state<C: RestoreContext>(
    source: &[u8],
    mut destinations: WorldStateDestinations<'_>,
    conditions: RestoreConditions,
    context: &mut C,
) -> Result<RestoreReport, RestoreError> {
    validate_destination(
        destinations.event_pool.len(),
        MAP_EVENT_SIZE * MAP_EVENT_COUNT,
        RestoreDestination::EventPool,
    )?;
    validate_destination(
        destinations.actor_state_head.len(),
        ACTOR_STATE_HEAD_SIZE,
        RestoreDestination::ActorStateHead,
    )?;
    validate_destination(
        destinations.actor_pool.len(),
        ACTOR_SIZE * ACTOR_COUNT,
        RestoreDestination::ActorPool,
    )?;
    validate_destination(
        destinations.object_pool.len(),
        MAP_OBJECT_SIZE * MAP_OBJECT_COUNT,
        RestoreDestination::ObjectPool,
    )?;
    require_source(source, 0, 1)?;
    let layout = if source[0] == 1 {
        Some(preflight_restore(source)?)
    } else {
        None
    };

    let (actor_overrides, link_overrides, consumed_bytes) = if let Some(layout) = layout {
        restore_encoded_prefix(source, &mut destinations, layout, context);
        (
            layout.actor_count as u8,
            ((layout.effect_160 - layout.link_entries) / 9) as u8,
            layout.consumed,
        )
    } else {
        (0, 0, 1)
    };

    apply_floor_conditions(&mut destinations, conditions, context)?;
    Ok(RestoreReport {
        marker_was_set: layout.is_some(),
        actor_overrides,
        link_overrides,
        consumed_bytes,
    })
}

fn preflight_restore(source: &[u8]) -> Result<RestoreLayout, RestoreError> {
    let events = 1;
    require_source(source, events, MAP_EVENT_COUNT * 7)?;
    for event in 0..MAP_EVENT_COUNT {
        let row = events + event * 7;
        let image_index = source[row + 2];
        let target = event * MAP_EVENT_SIZE + 2 + usize::from(image_index);
        if target >= MAP_EVENT_SIZE * MAP_EVENT_COUNT {
            return Err(RestoreError::EventTagWriteOutOfRange { event, image_index });
        }
    }
    let actor_count = events + MAP_EVENT_COUNT * 7;
    require_source(source, actor_count, 1)?;
    let actor_entry_count = usize::from(source[actor_count]);
    let actor_entries = actor_count + 1;
    require_source(source, actor_entries, actor_entry_count * 2)?;
    for entry in 0..actor_entry_count {
        let index = source[actor_entries + entry * 2];
        if usize::from(index) >= ACTOR_COUNT {
            return Err(RestoreError::ActorIndexOutOfRange { entry, index });
        }
    }
    let object_ids = actor_entries + actor_entry_count * 2;
    require_source(source, object_ids, MAP_OBJECT_COUNT)?;
    let link_count = object_ids + MAP_OBJECT_COUNT;
    require_source(source, link_count, 1)?;
    let link_entry_count = usize::from(source[link_count]);
    let link_entries = link_count + 1;
    require_source(source, link_entries, link_entry_count * 9)?;
    for entry in 0..link_entry_count {
        let index = source[link_entries + entry * 9];
        if usize::from(index) >= MAP_OBJECT_COUNT {
            return Err(RestoreError::ObjectIndexOutOfRange { entry, index });
        }
    }
    let effect_160 = link_entries + link_entry_count * 9;
    require_source(source, effect_160, 10 * 4)?;
    let effect_170 = effect_160 + 10 * 4;
    require_source(source, effect_170, 20 * 3)?;
    Ok(RestoreLayout {
        actor_count: actor_entry_count,
        actor_entries,
        object_ids,
        link_count,
        link_entries,
        effect_160,
        effect_170,
        consumed: effect_170 + 20 * 3,
    })
}

fn restore_encoded_prefix<C: RestoreContext>(
    source: &[u8],
    destinations: &mut WorldStateDestinations<'_>,
    layout: RestoreLayout,
    context: &mut C,
) {
    for event in 0..MAP_EVENT_COUNT {
        let input = 1 + event * 7;
        let output = event * MAP_EVENT_SIZE;
        destinations.event_pool[output] = source[input];
        destinations.event_pool[output + 8] = source[input + 1];
        let image_index = source[input + 2];
        destinations.event_pool[output + 9] = image_index;
        destinations.event_pool[output + 10] = source[input + 3];
        destinations.event_pool[output + 2 + usize::from(image_index)] = source[input + 4];
        destinations.event_pool[output + 11] = source[input + 5];
        destinations.event_pool[output + 13] = source[input + 6];
    }
    for entry in 0..layout.actor_count {
        let input = layout.actor_entries + entry * 2;
        let actor = usize::from(source[input]);
        destinations.actor_pool[actor * ACTOR_SIZE + 6] = source[input + 1];
    }
    for object in 0..MAP_OBJECT_COUNT {
        destinations.object_pool[object * MAP_OBJECT_SIZE] = source[layout.object_ids + object];
    }
    let link_entry_count = usize::from(source[layout.link_count]);
    for entry in 0..link_entry_count {
        let input = layout.link_entries + entry * 9;
        let object = usize::from(source[input]);
        let output = object * MAP_OBJECT_SIZE + 32;
        destinations.object_pool[output..output + 8].copy_from_slice(&source[input + 1..input + 9]);
    }
    for effect in 0..10 {
        restore_effect_160(
            &source[layout.effect_160 + effect * 4..layout.effect_160 + effect * 4 + 4],
            &mut destinations.object_pool
                [(160 + effect) * MAP_OBJECT_SIZE..(161 + effect) * MAP_OBJECT_SIZE],
            context,
        );
    }
    for effect in 0..20 {
        restore_effect_170(
            &source[layout.effect_170 + effect * 3..layout.effect_170 + effect * 3 + 3],
            &mut destinations.object_pool
                [(170 + effect) * MAP_OBJECT_SIZE..(171 + effect) * MAP_OBJECT_SIZE],
            context,
        );
    }
}

fn restore_effect_160<C: RestoreContext>(source: &[u8], object: &mut [u8], context: &mut C) {
    let cell_x = u16::from(source[0]);
    let cell_z = u16::from(source[1]);
    write_u16(object, 2, cell_x);
    write_u16(object, 4, cell_z);
    write_i32(object, 8, randomized_coordinate(cell_x, context.rand()));
    write_i32(object, 16, randomized_coordinate(cell_z, context.rand()));
    write_i32(
        object,
        12,
        -(i32::from(context.floor_height(cell_z, cell_x)) * 100),
    );
    write_i16(object, 24, 0);
    write_i16(object, 26, 0);
    write_i16(object, 28, 0);
    object[32] = source[2];
    object[33] = source[3];
    write_u16(object, 34, 0);
    write_i16(object, 36, 0);
}

fn restore_effect_170<C: RestoreContext>(source: &[u8], object: &mut [u8], context: &mut C) {
    let cell_x = u16::from(source[0]);
    let cell_z = u16::from(source[1]);
    write_u16(object, 2, cell_x);
    write_u16(object, 4, cell_z);
    write_i32(object, 8, randomized_coordinate(cell_x, context.rand()));
    write_i32(object, 16, randomized_coordinate(cell_z, context.rand()));
    write_i32(
        object,
        12,
        -(i32::from(context.floor_height(cell_z, cell_x)) * 100),
    );
    if object[0] < 43 {
        write_i16(object, 24, 0x400);
    } else if object[0] < 48 {
        write_i16(object, 24, 0);
    }
    write_i16(object, 28, 0);
    write_u16(object, 32, 0);
    write_u16(object, 34, 0);
    write_i16(object, 36, 0);
    write_i16(object, 26, i16::from(source[2]) << 4);
}

fn apply_floor_conditions<C: RestoreContext>(
    destinations: &mut WorldStateDestinations<'_>,
    conditions: RestoreConditions,
    context: &mut C,
) -> Result<(), RestoreError> {
    match conditions.current_floor {
        1 => {
            if conditions.world_byte_2 == 1 {
                context.apply_copy_region(1);
            }
            if conditions.world_byte_1 != 2 {
                let index = context.find_actor_at_tile(7, 0x28);
                if index != -1 {
                    let Ok(index) = usize::try_from(index) else {
                        return Err(RestoreError::CallbackActorIndexOutOfRange { index });
                    };
                    if index >= ACTOR_COUNT {
                        return Err(RestoreError::CallbackActorIndexOutOfRange {
                            index: index as i32,
                        });
                    }
                    destinations.actor_pool[index * ACTOR_SIZE + 6] = 3;
                }
            }
            if conditions.flag_8009f845 == 1 {
                context.clear_object_link(0x33);
            }
        }
        2 => {
            if conditions.flag_8009f845 == 1 {
                context.clear_object_link(0x33);
            }
            if conditions.highest_floor >= 3 {
                destinations.event_pool[0] = 3;
            }
        }
        3 => {
            if conditions.flag_8009f845 == 1 {
                context.clear_object_link(0x33);
            }
            if conditions.flag_8009eafc == 4 {
                context.apply_copy_region(2);
                context.apply_copy_region(3);
            }
        }
        5 => {
            if conditions.flag_8009f844 == 1 {
                destinations.event_pool[MAP_EVENT_SIZE] = 1;
            }
            if conditions.flag_8009f846 == 0 {
                for offset in [0x438, 0x43e, 0x43f, 0x440, 0x441] {
                    destinations.actor_state_head[offset] = 0xff;
                }
            } else {
                context.apply_copy_region(4);
            }
            if conditions.inventory_0a != 0
                || conditions.inventory_0b != 0
                || conditions.flag_8009f845 == 1
            {
                context.clear_object_link(0x34);
            }
            if conditions.boss_defeat_complete != 0 {
                context.trigger_object_link(0x0d);
                for definition in [0, 2, 3, 4] {
                    context.begin_actor_death_by_definition(definition);
                }
            }
        }
        _ => {}
    }
    Ok(())
}

fn randomized_coordinate(cell: u16, random: i32) -> i32 {
    i32::from(cell)
        .wrapping_mul(2_000)
        .wrapping_add(random.wrapping_mul(2_000) >> 15)
}

fn validate_destination(
    available: usize,
    needed: usize,
    destination: RestoreDestination,
) -> Result<(), RestoreError> {
    if available < needed {
        Err(RestoreError::DestinationTooSmall {
            destination,
            needed,
            available,
        })
    } else {
        Ok(())
    }
}

fn require_source(source: &[u8], at: usize, needed: usize) -> Result<(), RestoreError> {
    if source.len().saturating_sub(at) < needed {
        Err(RestoreError::SourceTooSmall {
            at,
            needed,
            available: source.len().saturating_sub(at),
        })
    } else {
        Ok(())
    }
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
