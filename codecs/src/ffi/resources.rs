use super::bindings::*;
use crate::resources;
use core::{mem::size_of, slice};

unsafe fn input<'a>(bytes: *const u8, length: usize) -> Result<&'a [u8], KfCodecResult> {
    if bytes.is_null() || length > isize::MAX as usize {
        return Err(KF_CODEC_INVALID);
    }
    Ok(slice::from_raw_parts(bytes, length))
}

unsafe fn output<'a, T>(data: *mut T, count: usize) -> Result<&'a mut [T], KfCodecResult> {
    if count == 0 {
        return Ok(&mut []);
    }
    if data.is_null() || count > isize::MAX as usize / size_of::<T>() {
        return Err(KF_CODEC_INVALID);
    }
    Ok(slice::from_raw_parts_mut(data, count))
}

#[no_mangle]
pub unsafe extern "C" fn kf_asset_info(
    bytes: *const u8,
    length: usize,
    info: *mut KfAssetInfo,
) -> KfCodecResult {
    if info.is_null() {
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

#[no_mangle]
pub unsafe extern "C" fn kf_animation_measure(
    bytes: *const u8,
    length: usize,
    vertex_count: u32,
    sizes: *mut KfAnimationSizes,
) -> KfCodecResult {
    if sizes.is_null() {
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

#[no_mangle]
pub unsafe extern "C" fn kf_animation_decode(
    bytes: *const u8,
    length: usize,
    vertex_count: u32,
    destination: *mut KfAnimationOutput,
) -> KfCodecResult {
    let decode = || {
        let d = destination.as_mut().ok_or(KF_CODEC_INVALID)?;
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
        #[no_mangle]
        pub unsafe extern "C" fn $name(
            bytes: *const u8,
            length: usize,
            limits: KfPlacementLimits,
            destination: *mut $record,
            capacity: usize,
            count: *mut usize,
        ) -> KfCodecResult {
            if count.is_null() {
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
