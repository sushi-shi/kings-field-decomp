//! Host adapter for the allocation-free nested world-state encoder.

use kf_codec::world_persist::{persist_floor_state, PersistSources};

pub fn execute(mut blocks: Vec<Vec<u8>>) -> Result<Vec<Vec<u8>>, String> {
    if blocks.len() != 5 {
        return Err(
            "world-persist expects initial record, events, actors, definitions, and objects".into(),
        );
    }
    let mut output = blocks.remove(0);
    let events = blocks.remove(0);
    let actors = blocks.remove(0);
    let definitions = blocks.remove(0);
    let objects = blocks.remove(0);
    persist_floor_state(
        &mut output,
        PersistSources {
            event_pool: &events,
            actor_pool: &actors,
            object_definitions: &definitions,
            object_pool: &objects,
        },
    )
    .map_err(|error| format!("{error:?}"))?;
    Ok(vec![output])
}
