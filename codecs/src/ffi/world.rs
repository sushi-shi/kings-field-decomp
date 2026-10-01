use super::bindings::*;
use super::network::{aligned, status};
use super::{INVALID, OK, OUTPUT_FULL};
use crate::network::world;
use core::{ptr, slice};

pub(crate) fn new_world() -> Box<KfNetWorld> {
    // Pointer-free integer records admit an all-zero representation.
    unsafe { Box::<KfNetWorld>::new_zeroed().assume_init() }
}

#[no_mangle]
pub unsafe extern "C" fn kf_net_world_info(
    data: *const u8,
    size: usize,
    output: *mut KfNetWorldHeader,
) -> KfCodecResult {
    if data.is_null() || size > KF_NET_TRANSFER_LIMIT as usize || !aligned(output) {
        return INVALID;
    }
    match world::info(slice::from_raw_parts(data, size)) {
        Ok(value) => {
            output.write(value);
            OK
        }
        Err(error) => status(error),
    }
}
#[no_mangle]
pub unsafe extern "C" fn kf_net_world_summary(
    data: *const u8,
    size: usize,
    output: *mut KfNetWorldSummary,
) -> KfCodecResult {
    if data.is_null() || size > KF_NET_TRANSFER_LIMIT as usize || !aligned(output) {
        return INVALID;
    }
    match world::summary(slice::from_raw_parts(data, size), &mut new_world()) {
        Ok(value) => {
            output.write(value);
            OK
        }
        Err(error) => status(error),
    }
}
#[no_mangle]
pub unsafe extern "C" fn kf_net_world_decode(
    data: *const u8,
    size: usize,
    limits: *const KfNetWorldLimits,
    output: *mut KfNetWorld,
) -> KfCodecResult {
    if data.is_null()
        || size > KF_NET_TRANSFER_LIMIT as usize
        || !aligned(limits)
        || !aligned(output)
    {
        return INVALID;
    }
    // All fields are integers/arrays: zero is valid storage. Allocate directly
    // on the heap to stay within the browser's small stack. Publish only success.
    let mut scratch = new_world();
    match world::decode(slice::from_raw_parts(data, size), &*limits, &mut scratch) {
        Ok(()) => {
            ptr::copy_nonoverlapping(&*scratch, output, 1);
            OK
        }
        Err(error) => status(error),
    }
}
#[no_mangle]
pub unsafe extern "C" fn kf_net_world_encode(
    input: *const KfNetWorld,
    limits: *const KfNetWorldLimits,
    data: *mut u8,
    capacity: usize,
    written: *mut usize,
) -> KfCodecResult {
    if !aligned(input)
        || !aligned(limits)
        || data.is_null()
        || capacity > isize::MAX as usize
        || !aligned(written)
    {
        return INVALID;
    }
    match world::encode(&*input, &*limits) {
        Ok(bytes) if bytes.len() <= capacity => {
            ptr::copy_nonoverlapping(bytes.as_ptr(), data, bytes.len());
            written.write(bytes.len());
            OK
        }
        Ok(_) => OUTPUT_FULL,
        Err(error) => status(error),
    }
}
