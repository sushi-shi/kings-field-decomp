use core::fmt;

pub const MAP_EVENT_SIZE: usize = 68;
pub const MAP_EVENT_COUNT: usize = 8;
pub const ACTOR_SIZE: usize = 72;
pub const ACTOR_COUNT: usize = 128;
pub const MAP_OBJECT_DEFINITION_SIZE: usize = 8;
pub const MAP_OBJECT_DEFINITION_COUNT: usize = 160;
pub const MAP_OBJECT_SIZE: usize = 44;
pub const MAP_OBJECT_COUNT: usize = 190;
pub const FLOOR_RECORD_SIZE: usize = 1_700;
pub const FLOOR_FIVE_RECORD_SIZE: usize = 1_690;

const FIXED_ENCODED_SIZE: usize =
    1 + MAP_EVENT_COUNT * 7 + 1 + MAP_OBJECT_COUNT + 1 + 10 * 4 + 20 * 3;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum PersistInput {
    EventPool,
    ActorPool,
    ObjectDefinitions,
    ObjectPool,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum PersistError {
    InputTooSmall {
        input: PersistInput,
        needed: usize,
        available: usize,
    },
    EventTagReadOutOfRange {
        event: usize,
        image_index: u8,
    },
    ObjectDefinitionOutOfRange {
        object: usize,
        object_id: u8,
    },
    OutputTooSmall {
        needed: usize,
        available: usize,
    },
}

impl fmt::Display for PersistError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "world persist {self:?}")
    }
}

impl core::error::Error for PersistError {}

pub struct PersistSources<'a> {
    pub event_pool: &'a [u8],
    pub actor_pool: &'a [u8],
    pub object_definitions: &'a [u8],
    pub object_pool: &'a [u8],
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct PersistReport {
    pub actor_overrides: u8,
    pub link_records: u8,
    pub written_bytes: usize,
}

pub fn persist_floor_state(
    output: &mut [u8],
    sources: PersistSources<'_>,
) -> Result<PersistReport, PersistError> {
    require_input(
        sources.event_pool.len(),
        MAP_EVENT_SIZE * MAP_EVENT_COUNT,
        PersistInput::EventPool,
    )?;
    require_input(
        sources.actor_pool.len(),
        ACTOR_SIZE * ACTOR_COUNT,
        PersistInput::ActorPool,
    )?;
    require_input(
        sources.object_definitions.len(),
        MAP_OBJECT_DEFINITION_SIZE * MAP_OBJECT_DEFINITION_COUNT,
        PersistInput::ObjectDefinitions,
    )?;
    require_input(
        sources.object_pool.len(),
        MAP_OBJECT_SIZE * MAP_OBJECT_COUNT,
        PersistInput::ObjectPool,
    )?;

    for event in 0..MAP_EVENT_COUNT {
        let at = event * MAP_EVENT_SIZE;
        let image_index = sources.event_pool[at + 9];
        if at + 2 + usize::from(image_index) >= MAP_EVENT_SIZE * MAP_EVENT_COUNT {
            return Err(PersistError::EventTagReadOutOfRange { event, image_index });
        }
    }

    let mut actor_overrides = 0usize;
    for actor in 0..ACTOR_COUNT {
        let at = actor * ACTOR_SIZE;
        if matches!(sources.actor_pool[at], 1 | 3) {
            actor_overrides += 1;
        }
    }

    let mut link_records = 0usize;
    for object in 0..MAP_OBJECT_DEFINITION_COUNT {
        let at = object * MAP_OBJECT_SIZE;
        let object_id = sources.object_pool[at];
        if object_id == 0xff {
            continue;
        }
        if usize::from(object_id) >= MAP_OBJECT_DEFINITION_COUNT {
            return Err(PersistError::ObjectDefinitionOutOfRange { object, object_id });
        }
        let behavior =
            sources.object_definitions[usize::from(object_id) * MAP_OBJECT_DEFINITION_SIZE];
        if !skips_inactive_link(behavior, sources.object_pool[at + 40]) {
            link_records += 1;
        }
    }

    let needed = FIXED_ENCODED_SIZE + actor_overrides * 2 + link_records * 9;
    if output.len() < needed {
        return Err(PersistError::OutputTooSmall {
            needed,
            available: output.len(),
        });
    }

    let mut at = 0usize;
    output[at] = 1;
    at += 1;
    for event in 0..MAP_EVENT_COUNT {
        let input = event * MAP_EVENT_SIZE;
        let image_index = sources.event_pool[input + 9];
        for value in [
            sources.event_pool[input],
            sources.event_pool[input + 8],
            image_index,
            sources.event_pool[input + 10],
            sources.event_pool[input + 2 + usize::from(image_index)],
            sources.event_pool[input + 11],
            sources.event_pool[input + 13],
        ] {
            output[at] = value;
            at += 1;
        }
    }

    output[at] = actor_overrides as u8;
    at += 1;
    for actor in 0..ACTOR_COUNT {
        let input = actor * ACTOR_SIZE;
        if matches!(sources.actor_pool[input], 1 | 3) {
            output[at] = actor as u8;
            output[at + 1] = if sources.actor_pool[input + 6] == 3 {
                3
            } else {
                0
            };
            at += 2;
        }
    }

    for object in 0..MAP_OBJECT_COUNT {
        output[at] = sources.object_pool[object * MAP_OBJECT_SIZE];
        at += 1;
    }

    output[at] = link_records as u8;
    at += 1;
    for object in 0..MAP_OBJECT_DEFINITION_COUNT {
        let input = object * MAP_OBJECT_SIZE;
        let object_id = sources.object_pool[input];
        if object_id == 0xff {
            continue;
        }
        let behavior =
            sources.object_definitions[usize::from(object_id) * MAP_OBJECT_DEFINITION_SIZE];
        if skips_inactive_link(behavior, sources.object_pool[input + 40]) {
            continue;
        }
        output[at] = object as u8;
        output[at + 1..at + 9].copy_from_slice(&sources.object_pool[input + 32..input + 40]);
        at += 9;
    }

    for object in 160..170 {
        let input = object * MAP_OBJECT_SIZE;
        output[at] = sources.object_pool[input + 2];
        output[at + 1] = sources.object_pool[input + 4];
        output[at + 2] = sources.object_pool[input + 32];
        output[at + 3] = sources.object_pool[input + 33];
        at += 4;
    }
    for object in 170..190 {
        let input = object * MAP_OBJECT_SIZE;
        output[at] = sources.object_pool[input + 2];
        output[at + 1] = sources.object_pool[input + 4];
        output[at + 2] = (read_u16(sources.object_pool, input + 26) >> 4) as u8;
        at += 3;
    }
    debug_assert_eq!(at, needed);

    Ok(PersistReport {
        actor_overrides: actor_overrides as u8,
        link_records: link_records as u8,
        written_bytes: at,
    })
}

fn skips_inactive_link(behavior: u8, action: u8) -> bool {
    matches!(behavior, 0xff | 0x0d | 0x40 | 0x0e | 0x41) && action == 0xff
}

fn require_input(available: usize, needed: usize, input: PersistInput) -> Result<(), PersistError> {
    if available < needed {
        Err(PersistError::InputTooSmall {
            input,
            needed,
            available,
        })
    } else {
        Ok(())
    }
}

fn read_u16(bytes: &[u8], at: usize) -> u16 {
    u16::from_le_bytes([bytes[at], bytes[at + 1]])
}
