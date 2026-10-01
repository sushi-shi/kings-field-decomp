use super::bindings::*;
use super::{END, INVALID, OK, OUTPUT_FULL};
use crate::network::{self, Error};
use core::{mem::align_of, slice};

pub(super) fn aligned<T>(value: *const T) -> bool {
    !value.is_null() && (value as usize) % align_of::<T>() == 0
}

pub(super) fn status(error: Error) -> KfCodecResult {
    match error {
        Error::Invalid => INVALID,
        Error::Full => OUTPUT_FULL,
        Error::Empty => END,
    }
}

// The C caller guarantees live, disjoint storage. Only this module constructs
// slices/references from pointers; protocol code receives bounded safe values.
unsafe fn decode<T>(
    data: *const u8,
    size: usize,
    output: *mut T,
    parse: fn(&[u8]) -> Result<T, Error>,
) -> KfCodecResult {
    if data.is_null() || size > KF_NET_PACKET_LIMIT as usize || !aligned(output) {
        return INVALID;
    }
    match parse(slice::from_raw_parts(data, size)) {
        Ok(value) => {
            output.write(value);
            OK
        }
        Err(error) => status(error),
    }
}

unsafe fn encode<T>(
    input: *const T,
    data: *mut u8,
    capacity: usize,
    written: *mut usize,
    write: fn(&T, &mut [u8]) -> Result<usize, Error>,
) -> KfCodecResult {
    if !aligned(input) || data.is_null() || capacity > isize::MAX as usize || !aligned(written) {
        return INVALID;
    }
    match write(
        &*input,
        slice::from_raw_parts_mut(data, capacity.min(KF_NET_PACKET_LIMIT as usize)),
    ) {
        Ok(size) => {
            written.write(size);
            OK
        }
        Err(error) => status(error),
    }
}

#[no_mangle]
pub unsafe extern "C" fn kf_net_header_decode(
    data: *const u8,
    size: usize,
    output: *mut KfNetHeader,
) -> KfCodecResult {
    decode(data, size, output, network::decode_header)
}

#[no_mangle]
pub unsafe extern "C" fn kf_net_header_encode(
    input: *const KfNetHeader,
    data: *mut u8,
    capacity: usize,
    written: *mut usize,
) -> KfCodecResult {
    encode(input, data, capacity, written, network::encode_header)
}

#[no_mangle]
pub unsafe extern "C" fn kf_net_input_decode(
    data: *const u8,
    size: usize,
    output: *mut KfNetInputBundle,
) -> KfCodecResult {
    decode(data, size, output, network::decode_input)
}

#[no_mangle]
pub unsafe extern "C" fn kf_net_input_encode(
    input: *const KfNetInputBundle,
    data: *mut u8,
    capacity: usize,
    written: *mut usize,
) -> KfCodecResult {
    encode(input, data, capacity, written, network::encode_input)
}

#[no_mangle]
pub unsafe extern "C" fn kf_net_command_decode(
    data: *const u8,
    size: usize,
    output: *mut KfNetCommand,
) -> KfCodecResult {
    decode(data, size, output, network::decode_command)
}

#[no_mangle]
pub unsafe extern "C" fn kf_net_command_encode(
    input: *const KfNetCommand,
    data: *mut u8,
    capacity: usize,
    written: *mut usize,
) -> KfCodecResult {
    encode(input, data, capacity, written, network::encode_command)
}

#[no_mangle]
pub unsafe extern "C" fn kf_net_interaction_decode(
    data: *const u8,
    size: usize,
    output: *mut KfNetInteraction,
) -> KfCodecResult {
    decode(data, size, output, network::decode_interaction)
}

#[no_mangle]
pub unsafe extern "C" fn kf_net_interaction_encode(
    input: *const KfNetInteraction,
    data: *mut u8,
    capacity: usize,
    written: *mut usize,
) -> KfCodecResult {
    encode(input, data, capacity, written, network::encode_interaction)
}

#[no_mangle]
pub unsafe extern "C" fn kf_net_fragment_decode(
    data: *const u8,
    size: usize,
    output: *mut KfNetFragment,
) -> KfCodecResult {
    decode(data, size, output, network::decode_fragment)
}

#[no_mangle]
pub unsafe extern "C" fn kf_net_fragment_encode(
    header: *const KfNetHeader,
    payload: *const u8,
    size: usize,
    offset: u32,
    data: *mut u8,
    capacity: usize,
    written: *mut usize,
) -> KfCodecResult {
    if !aligned(header)
        || payload.is_null()
        || size > KF_NET_TRANSFER_LIMIT as usize
        || data.is_null()
        || capacity > isize::MAX as usize
        || !aligned(written)
    {
        return INVALID;
    }
    match network::encode_fragment(
        &*header,
        slice::from_raw_parts(payload, size),
        offset,
        slice::from_raw_parts_mut(data, capacity.min(KF_NET_PACKET_LIMIT as usize)),
    ) {
        Ok(size) => {
            written.write(size);
            OK
        }
        Err(error) => status(error),
    }
}

#[no_mangle]
pub unsafe extern "C" fn kf_net_input_push(
    inbox: *mut KfNetInputInbox,
    input: *const KfNetInputBundle,
) -> KfCodecResult {
    if !aligned(inbox) || !aligned(input) {
        return INVALID;
    }
    match network::input_push(&mut *inbox, &*input) {
        Ok(()) => OK,
        Err(error) => status(error),
    }
}

#[no_mangle]
pub unsafe extern "C" fn kf_net_input_pop(
    inbox: *mut KfNetInputInbox,
    output: *mut KfNetInputFrame,
) -> KfCodecResult {
    if !aligned(inbox) || !aligned(output) {
        return INVALID;
    }
    match network::input_pop(&mut *inbox) {
        Ok(value) => {
            output.write(value);
            OK
        }
        Err(error) => status(error),
    }
}

#[no_mangle]
pub unsafe extern "C" fn kf_net_fragment_accept(
    transfer: *mut KfNetTransfer,
    data: *const u8,
    size: usize,
    output: *mut u8,
    capacity: usize,
) -> KfCodecResult {
    if !aligned(transfer) {
        return INVALID;
    }
    if data.is_null()
        || size > KF_NET_PACKET_LIMIT as usize
        || output.is_null()
        || capacity > isize::MAX as usize
    {
        network::reset_transfer(&mut *transfer);
        return INVALID;
    }
    match network::accept_fragment(
        &mut *transfer,
        slice::from_raw_parts(data, size),
        slice::from_raw_parts_mut(output, capacity.min(KF_NET_TRANSFER_LIMIT as usize)),
    ) {
        Ok(()) => OK,
        Err(error) => status(error),
    }
}
