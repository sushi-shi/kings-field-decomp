//! Host-only request adapter for the allocation-free animation codec.

use kf_codec::animation::{
    prepare_instance, Animation, CacheKey, InstanceOutcome, InstanceRequest, LifecycleEvent,
    PoolRecord, Vertex, POOL_RECORD_SIZE, VERTEX_SIZE,
};

const ASSET_VA: u32 = 0x8006_0000;
const RECORD_SIZE: usize = 20;
const PARAMETER_SIZE: usize = 12;
const INSTANCE_PARAMETER_SIZE: usize = 16;
const TMD_STATE_SIZE: usize = 36;
const RECORD_VA: u32 = 0x800b_0000;
const ANCHOR_VA: u32 = 0x800b_0100;
const SCRATCH_VA: u32 = 0x8009_30f0;

fn u16_at(bytes: &[u8], at: usize) -> u16 {
    u16::from_le_bytes([bytes[at], bytes[at + 1]])
}

fn u32_at(bytes: &[u8], at: usize) -> u32 {
    u32::from_le_bytes(bytes[at..at + 4].try_into().unwrap())
}

fn put_u16(bytes: &mut [u8], at: usize, value: u16) {
    bytes[at..at + 2].copy_from_slice(&value.to_le_bytes());
}

fn put_u32(bytes: &mut [u8], at: usize, value: u32) {
    bytes[at..at + 4].copy_from_slice(&value.to_le_bytes());
}

fn decode_vertices(bytes: &[u8]) -> Result<Vec<Vertex>, String> {
    if bytes.len() % VERTEX_SIZE != 0 {
        return Err("animation cache length is not a whole number of vertices".into());
    }
    Ok(bytes
        .chunks_exact(VERTEX_SIZE)
        .map(|bytes| Vertex::from_le_bytes(bytes.try_into().unwrap()))
        .collect())
}

fn encode_vertices(vertices: &[Vertex]) -> Vec<u8> {
    let mut bytes = Vec::with_capacity(vertices.len() * VERTEX_SIZE);
    for vertex in vertices {
        bytes.extend_from_slice(&vertex.to_le_bytes());
    }
    bytes
}

fn encode_events(events: &[LifecycleEvent]) -> Vec<u8> {
    let mut bytes = Vec::with_capacity(events.len() * 16);
    for event in events {
        let words = match *event {
            LifecycleEvent::AllocateRecord { available } => [1, u32::from(available), 0, 0],
            LifecycleEvent::ReleaseRecord {
                allocation,
                owner_slot,
            } => [2, allocation, owner_slot, 0],
            LifecycleEvent::AllocateVertices { byte_count, result } => [3, byte_count, result, 0],
            LifecycleEvent::ReleaseAll => [4, 0, 0, 0],
        };
        for word in words {
            bytes.extend_from_slice(&word.to_le_bytes());
        }
    }
    bytes
}

pub fn execute(mut blocks: Vec<Vec<u8>>) -> Result<Vec<Vec<u8>>, String> {
    if blocks.len() != 4 || blocks[1].len() != PARAMETER_SIZE || blocks[2].len() != RECORD_SIZE {
        return Err(
            "animation-bind expects asset, 12-byte parameters, 20-byte record, and cache".into(),
        );
    }
    let asset = blocks.remove(0);
    let parameters = blocks.remove(0);
    let mut record = blocks.remove(0);
    let mut cache = decode_vertices(&blocks.remove(0))?;

    let asset_index = u16_at(&parameters, 0);
    let clip_index = u16_at(&parameters, 2);
    let phase = u16_at(&parameters, 4);
    let initial_kf_index = u16_at(&parameters, 6);
    let previous_present = u16_at(&parameters, 8);
    let previous_rest_index = u16_at(&parameters, 10);
    if previous_present > 1 {
        return Err("animation previous-present parameter must be zero or one".into());
    }
    if u16_at(&record, 2) != asset_index {
        return Err("animation driver only models an existing same-asset pool record".into());
    }

    let animation = Animation::parse(&asset).map_err(|error| error.to_string())?;
    let vertex_count = usize::from(
        animation
            .base_vertex_count()
            .map_err(|error| error.to_string())?,
    );
    if cache.len() != vertex_count {
        return Err(format!(
            "animation cache has {} vertices, asset requires {vertex_count}",
            cache.len()
        ));
    }
    let previous = (previous_present != 0).then(|| CacheKey {
        clip_index: u16_at(&record, 4),
        keyframe_index: u16_at(&record, 6),
        rest_index: previous_rest_index,
    });
    let mut output = vec![Vertex::default(); vertex_count];
    let report = animation
        .bind_frame(
            clip_index,
            phase,
            initial_kf_index,
            previous,
            &mut cache,
            &mut output,
        )
        .map_err(|error| error.to_string())?;

    put_u16(&mut record, 0, 2);
    put_u16(&mut record, 4, report.cache_key.clip_index);
    put_u16(&mut record, 6, report.cache_key.keyframe_index);
    let rest = animation
        .morph_object(report.cache_key.rest_index)
        .map_err(|error| error.to_string())?;
    let rest_pointer = ASSET_VA
        .checked_add(u32::try_from(rest.offset).map_err(|_| "rest-object offset overflow")?)
        .ok_or("rest-object pointer overflow")?;
    put_u32(&mut record, 8, rest_pointer);

    Ok(vec![
        record,
        encode_vertices(&cache),
        encode_vertices(&output),
    ])
}

pub fn execute_static(blocks: Vec<Vec<u8>>) -> Result<Vec<Vec<u8>>, String> {
    let [asset] = blocks.as_slice() else {
        return Err("animation-static expects one asset".into());
    };
    let animation = Animation::parse(asset).map_err(|error| error.to_string())?;
    if animation.is_animated() {
        return Err("animation-static requires animation_clip_count == 0".into());
    }
    let offset = u32::try_from(
        animation
            .base_vertex_offset()
            .map_err(|error| error.to_string())?,
    )
    .map_err(|_| "base-vertex offset overflow")?;
    let pointer = ASSET_VA
        .checked_add(offset)
        .ok_or("base-vertex pointer overflow")?;
    Ok(vec![pointer.to_le_bytes().to_vec()])
}

pub fn execute_instance(mut blocks: Vec<Vec<u8>>) -> Result<Vec<Vec<u8>>, String> {
    if blocks.len() != 9
        || blocks[1].len() != INSTANCE_PARAMETER_SIZE
        || blocks[2].len() != 4
        || blocks[3].len() != POOL_RECORD_SIZE
        || blocks[6].len() != TMD_STATE_SIZE
        || blocks[7].len() != 4
        || blocks[8].len() % 4 != 0
    {
        return Err("animation-instance expects asset, 16-byte parameters, anchor, 20-byte record, cache, scratch, 36-byte TMD state, current pointer, and allocation results".into());
    }
    let asset = blocks.remove(0);
    let parameters = blocks.remove(0);
    let mut anchor = u32_at(&blocks.remove(0), 0);
    let record_bytes: [u8; POOL_RECORD_SIZE] = blocks
        .remove(0)
        .try_into()
        .map_err(|_| "invalid animation pool record")?;
    let mut record = PoolRecord::from_le_bytes(record_bytes);
    let mut cache = decode_vertices(&blocks.remove(0))?;
    let mut scratch = blocks.remove(0);
    let mut tmd_state = blocks.remove(0);
    let mut current = blocks.remove(0);
    let allocation_bytes = blocks.remove(0);
    let allocation_results: Vec<u32> = allocation_bytes
        .chunks_exact(4)
        .map(|bytes| u32::from_le_bytes(bytes.try_into().unwrap()))
        .collect();

    let asset_index = u16_at(&parameters, 0);
    let clip_index = u16_at(&parameters, 2);
    let phase = u16_at(&parameters, 4);
    let initial_kf_index = u16_at(&parameters, 6);
    let caller_vertex_count = u16_at(&parameters, 8);
    let record_present = u16_at(&parameters, 10);
    let pool_record_available = u16_at(&parameters, 12);
    let previous_rest_index = u16_at(&parameters, 14);
    if record_present > 1 || pool_record_available > 1 {
        return Err("animation instance flags must be zero or one".into());
    }
    let vertex_count = usize::from(caller_vertex_count);
    if cache.len() != vertex_count || scratch.len() != (vertex_count + 1) * VERTEX_SIZE {
        return Err("animation instance cache/scratch extent differs from caller count".into());
    }

    let animation = Animation::parse(&asset).map_err(|error| error.to_string())?;
    let mut events = vec![LifecycleEvent::ReleaseAll; allocation_results.len() * 2 + 1];
    let report = prepare_instance(
        &animation,
        &mut anchor,
        &mut record,
        InstanceRequest {
            asset_index,
            caller_vertex_count,
            record_present: record_present != 0,
            pool_record_available: pool_record_available != 0,
            record_address: RECORD_VA,
            owner_slot_address: ANCHOR_VA,
        },
        &allocation_results,
        &mut events,
    )
    .map_err(|error| error.to_string())?;
    events.truncate(report.event_count);

    let status = match report.outcome {
        InstanceOutcome::Static => {
            put_u32(
                &mut tmd_state,
                32,
                ASSET_VA
                    .checked_add(animation.header().tmd_data_offset)
                    .ok_or("animation TMD pointer overflow")?,
            );
            let vertex_pointer = ASSET_VA
                .checked_add(
                    u32::try_from(
                        animation
                            .base_vertex_offset()
                            .map_err(|error| error.to_string())?,
                    )
                    .map_err(|_| "animation vertex pointer overflow")?,
                )
                .ok_or("animation vertex pointer overflow")?;
            put_u32(&mut current, 0, vertex_pointer);
            1
        }
        InstanceOutcome::PoolUnavailable => 0,
        InstanceOutcome::Ready { reinitialized } => {
            let previous = (!reinitialized).then(|| CacheKey {
                clip_index: record.clip_index,
                keyframe_index: record.keyframe_index,
                rest_index: previous_rest_index,
            });
            let mut output = decode_vertices(&scratch[VERTEX_SIZE..])?;
            let bind = animation
                .bind_frame(
                    clip_index,
                    phase,
                    initial_kf_index,
                    previous,
                    &mut cache,
                    &mut output,
                )
                .map_err(|error| error.to_string())?;
            scratch[VERTEX_SIZE..].copy_from_slice(&encode_vertices(&output));
            record.state = 2;
            record.clip_index = bind.cache_key.clip_index;
            record.keyframe_index = bind.cache_key.keyframe_index;
            let rest = animation
                .morph_object(bind.cache_key.rest_index)
                .map_err(|error| error.to_string())?;
            record.rest_morph = ASSET_VA
                .checked_add(u32::try_from(rest.offset).map_err(|_| "rest pointer overflow")?)
                .ok_or("rest pointer overflow")?;
            if bind.cache_rebuilt {
                put_u32(
                    &mut tmd_state,
                    32,
                    ASSET_VA
                        .checked_add(animation.header().tmd_data_offset)
                        .ok_or("animation TMD pointer overflow")?,
                );
            }
            put_u32(&mut current, 0, SCRATCH_VA + VERTEX_SIZE as u32);
            RECORD_VA
        }
    };

    Ok(vec![
        status.to_le_bytes().to_vec(),
        anchor.to_le_bytes().to_vec(),
        record.to_le_bytes().to_vec(),
        encode_vertices(&cache),
        scratch,
        tmd_state,
        current,
        encode_events(&events),
    ])
}
