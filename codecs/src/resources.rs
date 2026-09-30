#![deny(clippy::as_conversions)]

use crate::bytes::{read, records, LeU16, LeU32};
use crate::cast::AsUsize;
use crate::ffi::bindings::*;
use crate::formats;
fn asset_header(bytes: &[u8]) -> crate::Result<formats::AssetHeader> {
    let header: formats::AssetHeader = read(bytes, 0)?;
    let encoded_bytes = header.encoded_bytes.get();
    let clip_count = header.clip_count.get();
    let tmd_offset = header.tmd_offset.get();
    if encoded_bytes.as_usize() < size_of::<formats::AssetHeader>()
        || encoded_bytes.as_usize() > bytes.len()
        || tmd_offset.as_usize() < size_of::<formats::AssetHeader>()
        || tmd_offset > encoded_bytes
        || clip_count > 256
    {
        return Err(crate::Error::invalid("invalid asset header"));
    }
    Ok(header)
}

pub fn asset_info(bytes: &[u8]) -> crate::Result<KfAssetInfo> {
    let header = asset_header(bytes)?;
    Ok(KfAssetInfo {
        encoded_bytes: header.encoded_bytes.get(),
        tmd_offset: header.tmd_offset.get(),
        clip_count: header.clip_count.get(),
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
) -> crate::Result<KfAnimationSizes> {
    let header = asset_header(bytes)?;
    let bytes = &bytes[..header.encoded_bytes.get().as_usize()];
    let mut sizes = KfAnimationSizes {
        clips: header.clip_count.get().as_usize(),
        keyframes: 0,
        morphs: 0,
        indices: 0,
        deltas: 0,
    };
    if header.clip_count.get() == 0 {
        return Ok(sizes);
    }
    if vertex_count == 0 {
        return Err(crate::Error::invalid("animated asset has no vertices"));
    }
    let clip_table = records::<LeU32>(
        bytes,
        header.clip_table_offset.get().as_usize(),
        sizes.clips,
    )?;
    let morph_table = header.morph_table_offset.get().as_usize();
    // Morph IDs are 16-bit on disc. Validate/decode shared morphs just once.
    let mut used = [0u64; 1024];
    for (clip_index, offset) in clip_table.enumerate() {
        let at = offset.get().as_usize();
        let clip: formats::AnimationClip = read(bytes, at)?;
        let count = clip.keyframe_count.get().as_usize();
        if count == 0 {
            return Err(crate::Error::invalid("animation clip has no keyframes"));
        }
        let offsets = records::<LeU32>(
            bytes,
            at.checked_add(size_of::<formats::AnimationClip>())
                .ok_or(crate::Error::invalid("clip table offset overflow"))?,
            count,
        )?;
        if let Some(out) = output.as_mut() {
            *out.clips
                .get_mut(clip_index)
                .ok_or(crate::Error::output_full())? = KfAnimationClipData {
                first_keyframe: sizes.keyframes,
                keyframe_count: count,
            };
        }
        for offset in offsets {
            let at = offset.get().as_usize();
            let header: formats::AnimationKeyframe = read(bytes, at)?;
            let rest = header.rest_morph.get();
            let count = header.morph_count.get().as_usize();
            let indices = records::<LeU16>(
                bytes,
                at.checked_add(size_of::<formats::AnimationKeyframe>())
                    .ok_or(crate::Error::invalid("keyframe table offset overflow"))?,
                count,
            )?;
            used[rest.as_usize() / 64] |= 1u64 << (rest % 64);
            if let Some(out) = output.as_mut() {
                *out.keyframes
                    .get_mut(sizes.keyframes)
                    .ok_or(crate::Error::output_full())? = KfAnimationKeyframe {
                    reverse: header.reverse.get(),
                    duration: header.duration.get(),
                    rest_morph: rest,
                    first_morph: sizes.indices,
                    morph_count: count,
                };
            }
            for index in indices {
                let id = index.get();
                used[id.as_usize() / 64] |= 1u64 << (id % 64);
                if let Some(out) = output.as_mut() {
                    *out.indices
                        .get_mut(sizes.indices)
                        .ok_or(crate::Error::output_full())? = id;
                }
                sizes.indices = sizes
                    .indices
                    .checked_add(1)
                    .ok_or(crate::Error::invalid("animation index count overflow"))?;
            }
            sizes.keyframes = sizes
                .keyframes
                .checked_add(1)
                .ok_or(crate::Error::invalid("animation keyframe count overflow"))?;
            // Shared offsets must not amplify a small input into unbounded work/storage.
            if sizes.keyframes > bytes.len() || sizes.indices > bytes.len() {
                return Err(crate::Error::invalid(
                    "animation keyframes exceed input budget",
                ));
            }
        }
    }
    for (block, bits) in used.into_iter().enumerate() {
        let mut bits = bits;
        while bits != 0 {
            let id = block * 64 + bits.trailing_zeros().as_usize();
            bits &= bits - 1;
            let entry = morph_table
                .checked_add(id * size_of::<LeU32>())
                .ok_or(crate::Error::invalid("morph table offset overflow"))?;
            let at = read::<LeU32>(bytes, entry)?.get().as_usize();
            let header: formats::AnimationMorph = read(bytes, at)?;
            let base_vertex = header.base_vertex.get();
            let count = header.delta_count.get();
            if base_vertex > vertex_count || count > vertex_count - base_vertex {
                return Err(crate::Error::invalid("morph vertices exceed model bounds"));
            }
            let deltas = records::<formats::AnimationDelta>(
                bytes,
                at.checked_add(size_of::<formats::AnimationMorph>())
                    .ok_or(crate::Error::invalid("morph delta offset overflow"))?,
                count.as_usize(),
            )?;
            if let Some(out) = output.as_mut() {
                *out.morphs.get_mut(id).ok_or(crate::Error::output_full())? = KfAnimationMorph {
                    base_vertex,
                    first_delta: sizes.deltas,
                    delta_count: count.as_usize(),
                };
            }
            for delta in deltas {
                if let Some(out) = output.as_mut() {
                    *out.deltas
                        .get_mut(sizes.deltas)
                        .ok_or(crate::Error::output_full())? = KfAnimationDelta {
                        x: delta.x.get(),
                        y: delta.y.get(),
                        z: delta.z.get(),
                    };
                }
                sizes.deltas = sizes
                    .deltas
                    .checked_add(1)
                    .ok_or(crate::Error::invalid("animation delta count overflow"))?;
            }
            if sizes.deltas > bytes.len() {
                return Err(crate::Error::invalid(
                    "animation deltas exceed input budget",
                ));
            }
            sizes.morphs = id + 1;
        }
    }
    Ok(sizes)
}

fn placements<Wire: bytemuck::AnyBitPattern, T>(
    bytes: &[u8],
    output: &mut [T],
    decode: impl Fn(Wire) -> crate::Result<T>,
) -> crate::Result<usize> {
    for (index, slot) in output.iter_mut().enumerate() {
        let at = index
            .checked_mul(size_of::<Wire>())
            .ok_or(crate::Error::invalid("placement offset overflow"))?;
        if *bytes.get(at).ok_or(crate::Error::invalid(
            "missing placement record or sentinel",
        ))? == 255
        {
            return Ok(index);
        }
        *slot = decode(read(bytes, at)?)?;
    }
    Ok(output.len())
}

fn cell(z: u8, x: u8, limits: KfPlacementLimits) -> crate::Result<()> {
    if u32::from(z) >= limits.map_side || u32::from(x) >= limits.map_side {
        return Err(crate::Error::invalid("placement cell exceeds map bounds"));
    }
    Ok(())
}

pub fn actors(
    bytes: &[u8],
    limits: KfPlacementLimits,
    output: &mut [KfActorPlacementData],
) -> crate::Result<usize> {
    placements(bytes, output, |b: formats::ActorPlacement| {
        cell(b.tile_z, b.tile_x, limits)?;
        for (tile, local) in [(b.tile_z, b.local_z.get()), (b.tile_x, b.local_x.get())] {
            let position = i64::from(tile) * i64::from(limits.tile_size) + i64::from(local);
            let extent = i64::from(limits.map_side)
                .checked_mul(i64::from(limits.tile_size))
                .ok_or(crate::Error::invalid("map extent overflow"))?;
            if position < 0 || position >= extent {
                return Err(crate::Error::invalid("actor position exceeds map bounds"));
            }
        }
        if b.slot_state > 3
            || u32::from(b.definition_flags & 31) >= limits.definitions
            || b.heading_quadrant > 3
        {
            return Err(crate::Error::invalid("invalid actor placement"));
        }
        Ok(KfActorPlacementData {
            slot_state: b.slot_state,
            definition_id: b.definition_flags & 31,
            near_square_culling: u8::from(b.definition_flags & 32 != 0),
            heading_quadrant: b.heading_quadrant,
            tile_z: b.tile_z,
            tile_x: b.tile_x,
            spawn_chance: b.spawn_chance,
            death_drop_object_id: b.death_drop_object_id,
            local_z: b.local_z.get(),
            local_x: b.local_x.get(),
        })
    })
}

pub fn objects(
    bytes: &[u8],
    limits: KfPlacementLimits,
    output: &mut [KfObjectPlacementData],
) -> crate::Result<usize> {
    placements(bytes, output, |b: formats::ObjectPlacement| {
        cell(b.tile_z, b.tile_x, limits)?;
        if u32::from(b.object_id) >= limits.definitions {
            return Err(crate::Error::invalid(
                "object definition exceeds available definitions",
            ));
        }
        Ok(KfObjectPlacementData {
            object_id: b.object_id,
            tile_z: b.tile_z,
            tile_x: b.tile_x,
            yaw: b.yaw.get(),
            local_z: b.local_z.get(),
            local_x: b.local_x.get(),
            local_y: b.local_y.get(),
            link: [b.link[0].get(), b.link[1].get()],
        })
    })
}

pub fn events(
    bytes: &[u8],
    limits: KfPlacementLimits,
    output: &mut [KfEventPlacementData],
) -> crate::Result<usize> {
    placements(bytes, output, |b: formats::EventPlacement| {
        cell(b.cell_z, b.cell_x, limits)?;
        if !matches!(b.state, 0 | 1 | 3)
            || u32::from(b.model_index) >= limits.definitions
            || b.dialogue_stage_limit > 5
            || b.behavior > 2
        {
            return Err(crate::Error::invalid("invalid event placement"));
        }
        Ok(KfEventPlacementData {
            state: b.state,
            character_id: b.character_id,
            model_index: b.model_index,
            cell_z: b.cell_z,
            cell_x: b.cell_x,
            dialogue_pages: b.dialogue_pages,
            dialogue_stage_limit: b.dialogue_stage_limit,
            unknown_0b: b.unknown_0b,
            unknown_0c: b.unknown_0c,
            behavior: b.behavior,
            position_z_offset: b.position_z_offset.get(),
            position_x_offset: b.position_x_offset.get(),
            initial_rotation: b.initial_rotation.get(),
            radius: b.radius.get(),
        })
    })
}
