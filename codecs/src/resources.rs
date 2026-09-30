use crate::bytes::{read, read_u16_le, read_u32_le, records, span};
use crate::ffi::bindings::*;

type Result<T> = core::result::Result<T, KfCodecResult>;
const INVALID: KfCodecResult = KF_CODEC_INVALID;

pub fn asset_info(bytes: &[u8]) -> Result<KfAssetInfo> {
    span(bytes, 0, 20).ok_or(INVALID)?;
    let encoded_bytes = read_u32_le(bytes, 0).ok_or(INVALID)?;
    let clip_count = read_u32_le(bytes, 4).ok_or(INVALID)?;
    let tmd_offset = read_u32_le(bytes, 8).ok_or(INVALID)?;
    if encoded_bytes < 20
        || encoded_bytes as usize > bytes.len()
        || tmd_offset < 20
        || tmd_offset > encoded_bytes
        || clip_count > 256
    {
        return Err(INVALID);
    }
    Ok(KfAssetInfo {
        encoded_bytes,
        tmd_offset,
        clip_count,
    })
}

pub struct AnimationOutput<'a> {
    pub clips: &'a mut [KfAnimationClipData],
    pub keyframes: &'a mut [KfAnimationKeyframe],
    pub morphs: &'a mut [KfAnimationMorph],
    pub indices: &'a mut [u16],
    pub deltas: &'a mut [KfAnimationDelta],
}

// Both passes follow the same checked decoder. Only the second writes outputs.
pub fn animation(
    bytes: &[u8],
    vertex_count: u32,
    mut output: Option<AnimationOutput<'_>>,
) -> Result<KfAnimationSizes> {
    let info = asset_info(bytes)?;
    let bytes = &bytes[..info.encoded_bytes as usize];
    let mut sizes = KfAnimationSizes {
        clips: info.clip_count as usize,
        keyframes: 0,
        morphs: 0,
        indices: 0,
        deltas: 0,
    };
    if info.clip_count == 0 {
        return Ok(sizes);
    }
    if vertex_count == 0 {
        return Err(INVALID);
    }
    let clip_table = records(
        bytes,
        read_u32_le(bytes, 16).ok_or(INVALID)? as usize,
        sizes.clips,
        4,
    )
    .ok_or(INVALID)?;
    let object_table = read_u32_le(bytes, 12).ok_or(INVALID)? as usize;
    // Morph IDs are 16-bit on disc. Validate/decode shared morphs just once.
    let mut used = [0u64; 1024];
    for (clip_index, offset) in clip_table.chunks_exact(4).enumerate() {
        let at = read_u32_le(offset, 0).ok_or(INVALID)? as usize;
        let count = read_u16_le(bytes, at).ok_or(INVALID)? as usize;
        if count == 0 {
            return Err(INVALID);
        }
        let offsets = records(bytes, at.checked_add(4).ok_or(INVALID)?, count, 4).ok_or(INVALID)?;
        if let Some(out) = output.as_mut() {
            *out.clips.get_mut(clip_index).ok_or(KF_CODEC_OUTPUT_FULL)? = KfAnimationClipData {
                first_keyframe: sizes.keyframes,
                keyframe_count: count,
            };
        }
        for offset in offsets.chunks_exact(4) {
            let at = read_u32_le(offset, 0).ok_or(INVALID)? as usize;
            let header = span(bytes, at, 8).ok_or(INVALID)?;
            let rest = read_u16_le(header, 4).ok_or(INVALID)?;
            let count = read_u16_le(header, 6).ok_or(INVALID)? as usize;
            let indices =
                records(bytes, at.checked_add(8).ok_or(INVALID)?, count, 2).ok_or(INVALID)?;
            used[rest as usize / 64] |= 1u64 << (rest % 64);
            if let Some(out) = output.as_mut() {
                *out.keyframes
                    .get_mut(sizes.keyframes)
                    .ok_or(KF_CODEC_OUTPUT_FULL)? = KfAnimationKeyframe {
                    reverse: read_u16_le(header, 0).ok_or(INVALID)?,
                    duration: read_u16_le(header, 2).ok_or(INVALID)?,
                    rest_morph: rest,
                    first_morph: sizes.indices,
                    morph_count: count,
                };
            }
            for index in indices.chunks_exact(2) {
                let id = read_u16_le(index, 0).ok_or(INVALID)?;
                used[id as usize / 64] |= 1u64 << (id % 64);
                if let Some(out) = output.as_mut() {
                    *out.indices
                        .get_mut(sizes.indices)
                        .ok_or(KF_CODEC_OUTPUT_FULL)? = id;
                }
                sizes.indices = sizes.indices.checked_add(1).ok_or(INVALID)?;
            }
            sizes.keyframes = sizes.keyframes.checked_add(1).ok_or(INVALID)?;
            // Shared offsets must not amplify a small input into unbounded work/storage.
            if sizes.keyframes > bytes.len() || sizes.indices > bytes.len() {
                return Err(INVALID);
            }
        }
    }
    for (block, bits) in used.into_iter().enumerate() {
        let mut bits = bits;
        while bits != 0 {
            let id = block * 64 + bits.trailing_zeros() as usize;
            bits &= bits - 1;
            let entry = object_table.checked_add(id * 4).ok_or(INVALID)?;
            let at = read_u32_le(bytes, entry).ok_or(INVALID)? as usize;
            let header = span(bytes, at, 12).ok_or(INVALID)?;
            let base_vertex = read_u32_le(header, 4).ok_or(INVALID)?;
            let count = read_u32_le(header, 8).ok_or(INVALID)?;
            if base_vertex > vertex_count || count > vertex_count - base_vertex {
                return Err(INVALID);
            }
            let deltas = records(bytes, at.checked_add(12).ok_or(INVALID)?, count as usize, 8)
                .ok_or(INVALID)?;
            if let Some(out) = output.as_mut() {
                *out.morphs.get_mut(id).ok_or(KF_CODEC_OUTPUT_FULL)? = KfAnimationMorph {
                    base_vertex,
                    first_delta: sizes.deltas,
                    delta_count: count as usize,
                };
            }
            for delta in deltas.chunks_exact(8) {
                if let Some(out) = output.as_mut() {
                    *out.deltas
                        .get_mut(sizes.deltas)
                        .ok_or(KF_CODEC_OUTPUT_FULL)? = KfAnimationDelta {
                        x: read_u16_le(delta, 0).ok_or(INVALID)? as i16,
                        y: read_u16_le(delta, 2).ok_or(INVALID)? as i16,
                        z: read_u16_le(delta, 4).ok_or(INVALID)? as i16,
                    };
                }
                sizes.deltas = sizes.deltas.checked_add(1).ok_or(INVALID)?;
            }
            if sizes.deltas > bytes.len() {
                return Err(INVALID);
            }
            sizes.morphs = id + 1;
        }
    }
    Ok(sizes)
}

fn placements<T>(
    bytes: &[u8],
    width: usize,
    output: &mut [T],
    decode: impl Fn(&[u8]) -> Result<T>,
) -> Result<usize> {
    for (index, slot) in output.iter_mut().enumerate() {
        let at = index.checked_mul(width).ok_or(INVALID)?;
        if *bytes.get(at).ok_or(INVALID)? == 255 {
            return Ok(index);
        }
        *slot = decode(span(bytes, at, width).ok_or(INVALID)?)?;
    }
    Ok(output.len())
}

fn cell(z: u8, x: u8, limits: KfPlacementLimits) -> Result<()> {
    if u32::from(z) >= limits.map_side || u32::from(x) >= limits.map_side {
        return Err(INVALID);
    }
    Ok(())
}

pub fn actors(
    bytes: &[u8],
    limits: KfPlacementLimits,
    output: &mut [KfActorPlacementData],
) -> Result<usize> {
    placements(bytes, 16, output, |b| {
        cell(b[3], b[4], limits)?;
        for (tile, local) in [
            (b[3], read_u16_le(b, 10).ok_or(INVALID)? as i16),
            (b[4], read_u16_le(b, 12).ok_or(INVALID)? as i16),
        ] {
            let position = i64::from(tile) * i64::from(limits.tile_size) + i64::from(local);
            let extent = i64::from(limits.map_side)
                .checked_mul(i64::from(limits.tile_size))
                .ok_or(INVALID)?;
            if position < 0 || position >= extent {
                return Err(INVALID);
            }
        }
        if b[0] > 3 || u32::from(b[1] & 31) >= limits.definitions || b[2] > 3 {
            return Err(INVALID);
        }
        Ok(KfActorPlacementData {
            slot_state: b[0],
            definition_id: b[1] & 31,
            near_square_culling: u8::from(b[1] & 32 != 0),
            heading_quadrant: b[2],
            tile_z: b[3],
            tile_x: b[4],
            spawn_chance: b[5],
            death_drop_object_id: b[6],
            local_z: read_u16_le(b, 10).ok_or(INVALID)? as i16,
            local_x: read_u16_le(b, 12).ok_or(INVALID)? as i16,
        })
    })
}

pub fn objects(
    bytes: &[u8],
    limits: KfPlacementLimits,
    output: &mut [KfObjectPlacementData],
) -> Result<usize> {
    placements(bytes, 20, output, |b| {
        cell(b[2], b[3], limits)?;
        if u32::from(b[0]) >= limits.definitions {
            return Err(INVALID);
        }
        Ok(KfObjectPlacementData {
            object_id: b[0],
            tile_z: b[2],
            tile_x: b[3],
            yaw: read_u16_le(b, 4).ok_or(INVALID)?,
            local_z: read_u16_le(b, 6).ok_or(INVALID)? as i16,
            local_x: read_u16_le(b, 8).ok_or(INVALID)? as i16,
            local_y: read_u16_le(b, 10).ok_or(INVALID)? as i16,
            link: [
                read_u32_le(b, 12).ok_or(INVALID)?,
                read_u32_le(b, 16).ok_or(INVALID)?,
            ],
        })
    })
}

pub fn events(
    bytes: &[u8],
    limits: KfPlacementLimits,
    output: &mut [KfEventPlacementData],
) -> Result<usize> {
    placements(bytes, 24, output, |b| {
        cell(b[3], b[4], limits)?;
        if !matches!(b[0], 0 | 1 | 3)
            || u32::from(b[2]) >= limits.definitions
            || b[10] > 5
            || b[13] > 2
        {
            return Err(INVALID);
        }
        Ok(KfEventPlacementData {
            state: b[0],
            character_id: b[1],
            model_index: b[2],
            cell_z: b[3],
            cell_x: b[4],
            dialogue_pages: read(b, 5).ok_or(INVALID)?,
            dialogue_stage_limit: b[10],
            unknown_0b: b[11],
            unknown_0c: b[12],
            behavior: b[13],
            position_z_offset: read_u16_le(b, 14).ok_or(INVALID)? as i16,
            position_x_offset: read_u16_le(b, 16).ok_or(INVALID)? as i16,
            initial_rotation: read_u16_le(b, 18).ok_or(INVALID)?,
            radius: read_u16_le(b, 20).ok_or(INVALID)?,
        })
    })
}
