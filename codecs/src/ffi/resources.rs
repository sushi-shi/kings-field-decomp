use super::bindings::*;
use super::{input_valid, output_valid, status};
use crate::resources;
use core::slice;

#[track_caller]
unsafe fn input<'a>(bytes: *const u8, length: usize) -> crate::Result<&'a [u8]> {
    if !input_valid(bytes, length) {
        crate::bail!("invalid resource pointer or size");
    }
    Ok(slice::from_raw_parts(bytes, length))
}

#[track_caller]
unsafe fn output<'a, T>(data: *mut T, count: usize) -> crate::Result<&'a mut [T]> {
    if count == 0 {
        return Ok(&mut []);
    }
    if !output_valid(data, count) {
        crate::bail!("invalid resource pointer or size");
    }
    Ok(slice::from_raw_parts_mut(data, count))
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn kf_asset_info(
    bytes: *const u8,
    length: usize,
    info: *mut KfAssetInfo,
) -> KfCodecResult {
    status(asset_info(bytes, length, info))
}

unsafe fn asset_info(bytes: *const u8, length: usize, info: *mut KfAssetInfo) -> crate::Result<()> {
    if !output_valid(info, 1) {
        crate::bail!("invalid resource output pointer");
    }
    info.write(resources::asset_info(input(bytes, length)?)?);
    Ok(())
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn kf_animation_measure(
    bytes: *const u8,
    length: usize,
    vertex_count: u32,
    sizes: *mut KfAnimationSizes,
) -> KfCodecResult {
    status(animation_measure(bytes, length, vertex_count, sizes))
}

unsafe fn animation_measure(
    bytes: *const u8,
    length: usize,
    vertex_count: u32,
    sizes: *mut KfAnimationSizes,
) -> crate::Result<()> {
    if !output_valid(sizes, 1) {
        crate::bail!("invalid resource output pointer");
    }
    sizes.write(resources::animation(
        input(bytes, length)?,
        vertex_count,
        None,
    )?);
    Ok(())
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn kf_animation_decode(
    bytes: *const u8,
    length: usize,
    vertex_count: u32,
    destination: *mut KfAnimationOutput,
) -> KfCodecResult {
    status(animation_decode(bytes, length, vertex_count, destination))
}

unsafe fn animation_decode(
    bytes: *const u8,
    length: usize,
    vertex_count: u32,
    destination: *mut KfAnimationOutput,
) -> crate::Result<()> {
    if !output_valid(destination, 1) {
        crate::bail!("invalid resource output pointer");
    }
    let d = &mut *destination;
    let bytes = input(bytes, length)?;
    let out = resources::AnimationOutput {
        clips: output(d.clips, d.capacity.clips)?,
        keyframes: output(d.keyframes, d.capacity.keyframes)?,
        morphs: output(d.morphs, d.capacity.morphs)?,
        indices: output(d.indices, d.capacity.indices)?,
        deltas: output(d.deltas, d.capacity.deltas)?,
    };
    resources::animation(bytes, vertex_count, Some(out))?;
    Ok(())
}

macro_rules! placement_decoder {
    ($name:ident, $record:ty, $decode:ident) => {
        #[unsafe(no_mangle)]
        pub unsafe extern "C" fn $name(
            bytes: *const u8,
            length: usize,
            limits: KfPlacementLimits,
            destination: *mut $record,
            capacity: usize,
            count: *mut usize,
        ) -> KfCodecResult {
            status($decode(bytes, length, limits, destination, capacity, count))
        }

        unsafe fn $decode(
            bytes: *const u8,
            length: usize,
            limits: KfPlacementLimits,
            destination: *mut $record,
            capacity: usize,
            count: *mut usize,
        ) -> crate::Result<()> {
            if !output_valid(count, 1) {
                crate::bail!("invalid resource output pointer");
            }
            count.write(resources::$decode(
                input(bytes, length)?,
                limits,
                output(destination, capacity)?,
            )?);
            Ok(())
        }
    };
}
placement_decoder!(kf_actor_placements_decode, KfActorPlacementData, actors);
placement_decoder!(kf_object_placements_decode, KfObjectPlacementData, objects);
placement_decoder!(kf_event_placements_decode, KfEventPlacementData, events);
