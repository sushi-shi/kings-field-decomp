use super::bindings::*;
use super::{input_valid, output_valid};
use crate::resources;
use core::slice;

unsafe fn input<'a>(bytes: *const u8, length: usize) -> Result<&'a [u8], KfCodecResult> {
    if !input_valid(bytes, length) {
        return Err(KF_CODEC_INVALID);
    }
    Ok(slice::from_raw_parts(bytes, length))
}

unsafe fn output<'a, T>(data: *mut T, count: usize) -> Result<&'a mut [T], KfCodecResult> {
    if count == 0 {
        return Ok(&mut []);
    }
    if !output_valid(data, count) {
        return Err(KF_CODEC_INVALID);
    }
    Ok(slice::from_raw_parts_mut(data, count))
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn kf_asset_info(
    bytes: *const u8,
    length: usize,
    info: *mut KfAssetInfo,
) -> KfCodecResult {
    if !output_valid(info, 1) {
        return KF_CODEC_INVALID;
    }
    match input(bytes, length).and_then(resources::asset_info) {
        Ok(value) => {
            info.write(value);
            KF_CODEC_OK
        }
        Err(e) => e,
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn kf_animation_measure(
    bytes: *const u8,
    length: usize,
    vertex_count: u32,
    sizes: *mut KfAnimationSizes,
) -> KfCodecResult {
    if !output_valid(sizes, 1) {
        return KF_CODEC_INVALID;
    }
    match input(bytes, length).and_then(|b| resources::animation(b, vertex_count, None)) {
        Ok(value) => {
            sizes.write(value);
            KF_CODEC_OK
        }
        Err(e) => e,
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn kf_animation_decode(
    bytes: *const u8,
    length: usize,
    vertex_count: u32,
    destination: *mut KfAnimationOutput,
) -> KfCodecResult {
    if !output_valid(destination, 1) {
        return KF_CODEC_INVALID;
    }
    let decode = || {
        let d = &mut *destination;
        let bytes = input(bytes, length)?;
        let out = resources::AnimationOutput {
            clips: output(d.clips, d.capacity.clips)?,
            keyframes: output(d.keyframes, d.capacity.keyframes)?,
            morphs: output(d.morphs, d.capacity.morphs)?,
            indices: output(d.indices, d.capacity.indices)?,
            deltas: output(d.deltas, d.capacity.deltas)?,
        };
        resources::animation(bytes, vertex_count, Some(out))
    };
    match decode() {
        Ok(_) => KF_CODEC_OK,
        Err(e) => e,
    }
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
            if !output_valid(count, 1) {
                return KF_CODEC_INVALID;
            }
            let decode = || {
                resources::$decode(
                    input(bytes, length)?,
                    limits,
                    output(destination, capacity)?,
                )
            };
            match decode() {
                Ok(n) => {
                    count.write(n);
                    KF_CODEC_OK
                }
                Err(e) => e,
            }
        }
    };
}
placement_decoder!(kf_actor_placements_decode, KfActorPlacementData, actors);
placement_decoder!(kf_object_placements_decode, KfObjectPlacementData, objects);
placement_decoder!(kf_event_placements_decode, KfEventPlacementData, events);
