use super::bindings::*;
use super::{END, INVALID, OK, OUTPUT_FULL};
use crate::network::transport::{Config, Transport, PACKET_LIMIT};
use std::{mem::align_of, ptr, slice, str};

fn aligned<T>(value: *const T) -> bool {
    !value.is_null() && (value as usize) % align_of::<T>() == 0
}
fn string(bytes: &[u8]) -> Option<String> {
    let end = bytes.iter().position(|byte| *byte == 0)?;
    Some(str::from_utf8(&bytes[..end]).ok()?.to_owned())
}
fn config(value: &KfNetTransportConfig) -> Option<Config> {
    if value.host > 1 {
        return None;
    }
    Some(Config {
        address: string(&value.address)?,
        room: string(&value.room)?,
        resources: string(&value.resources)?,
        recipe: string(&value.recipe)?,
        resume: string(&value.resume)?,
        credential: string(&value.credential)?,
        roster: value.roster,
        host: value.host != 0,
    })
}

#[no_mangle]
pub unsafe extern "C" fn kf_net_transport_open(
    input: *const KfNetTransportConfig,
) -> *mut KfNetTransport {
    if !aligned(input) {
        return ptr::null_mut();
    }
    config(&*input)
        .and_then(Transport::open)
        .map_or(ptr::null_mut(), |transport| {
            Box::into_raw(Box::new(transport)).cast()
        })
}
#[no_mangle]
pub unsafe extern "C" fn kf_net_transport_close(handle: *mut KfNetTransport) {
    if !handle.is_null() {
        drop(Box::from_raw(handle.cast::<Transport>()));
    }
}
#[no_mangle]
pub unsafe extern "C" fn kf_net_transport_poll(
    handle: *mut KfNetTransport,
    output: *mut KfNetTransportEvent,
) -> KfCodecResult {
    if !aligned(handle.cast::<Transport>()) || !aligned(output) {
        return INVALID;
    }
    let Some(event) = (&mut *handle.cast::<Transport>()).poll() else {
        return END;
    };
    if event.data.len() > PACKET_LIMIT {
        return INVALID;
    }
    // Write fields individually: a 60 KB temporary record would exhaust small WASM stacks.
    ptr::addr_of_mut!((*output).kind).write(event.kind);
    ptr::addr_of_mut!((*output).peer).write(event.peer);
    ptr::addr_of_mut!((*output).lane).write(event.lane);
    ptr::addr_of_mut!((*output).identity).write(event.identity);
    ptr::addr_of_mut!((*output).size).write(event.data.len() as u32);
    ptr::copy_nonoverlapping(
        event.data.as_ptr(),
        ptr::addr_of_mut!((*output).data).cast::<u8>(),
        event.data.len(),
    );
    OK
}
#[no_mangle]
pub unsafe extern "C" fn kf_net_transport_send(
    handle: *mut KfNetTransport,
    peer: u8,
    lane: u8,
    data: *const u8,
    size: usize,
) -> KfCodecResult {
    if !aligned(handle.cast::<Transport>()) || data.is_null() || size == 0 || size > PACKET_LIMIT {
        return INVALID;
    }
    if (&mut *handle.cast::<Transport>()).send(peer, lane, slice::from_raw_parts(data, size)) {
        OK
    } else {
        OUTPUT_FULL
    }
}
#[no_mangle]
pub unsafe extern "C" fn kf_net_transport_resume(
    handle: *mut KfNetTransport,
    data: *mut u8,
    capacity: usize,
    written: *mut usize,
) -> KfCodecResult {
    if !aligned(handle.cast::<Transport>()) || data.is_null() || !aligned(written) {
        return INVALID;
    }
    let resume = (&*handle.cast::<Transport>()).shared.resume.lock().unwrap();
    if resume.len() > capacity {
        return OUTPUT_FULL;
    }
    ptr::copy_nonoverlapping(resume.as_ptr(), data, resume.len());
    written.write(resume.len());
    OK
}

#[cfg(target_os = "emscripten")]
pub mod browser {
    use std::ffi::CString;
    extern "C" {
        fn kf_browser_open(handle: u32, address: *const i8) -> i32;
        fn kf_browser_close(handle: u32);
        fn kf_browser_signal(handle: u32, text: *const i8) -> i32;
        fn kf_browser_peer(
            handle: u32,
            peer: u8,
            generation: u32,
            initiator: i32,
            ice: *const i8,
        ) -> i32;
        fn kf_browser_drop_peer(handle: u32, peer: u8);
        fn kf_browser_description(handle: u32, peer: u8, text: *const i8) -> i32;
        fn kf_browser_send(handle: u32, peer: u8, lane: u8, data: *const u8, size: usize) -> i32;
    }
    pub fn open(handle: u32, address: &str) -> bool {
        CString::new(address)
            .is_ok_and(|address| unsafe { kf_browser_open(handle, address.as_ptr()) != 0 })
    }
    pub fn close(handle: u32) {
        unsafe { kf_browser_close(handle) };
    }
    pub fn signal(handle: u32, text: &str) -> bool {
        CString::new(text)
            .is_ok_and(|text| unsafe { kf_browser_signal(handle, text.as_ptr()) != 0 })
    }
    pub fn peer(handle: u32, peer: u8, generation: u32, initiator: bool, ice: &str) -> bool {
        CString::new(ice).is_ok_and(|ice| unsafe {
            kf_browser_peer(handle, peer, generation, i32::from(initiator), ice.as_ptr()) != 0
        })
    }
    pub fn drop_peer(handle: u32, peer: u8) {
        unsafe { kf_browser_drop_peer(handle, peer) };
    }
    pub fn description(handle: u32, peer: u8, text: &str) -> bool {
        CString::new(text)
            .is_ok_and(|text| unsafe { kf_browser_description(handle, peer, text.as_ptr()) != 0 })
    }
    pub fn send(handle: u32, peer: u8, lane: u8, bytes: &[u8]) -> bool {
        unsafe { kf_browser_send(handle, peer, lane, bytes.as_ptr(), bytes.len()) != 0 }
    }
}

#[cfg(target_os = "emscripten")]
#[no_mangle]
pub unsafe extern "C" fn kf_net_browser_event(
    handle: u32,
    kind: u8,
    peer: u8,
    lane: u8,
    generation: u32,
    data: *const u8,
    size: usize,
) {
    if data.is_null() || size > crate::network::transport::SIGNAL_LIMIT || peer > 3 || lane > 1 {
        return;
    }
    crate::network::transport::browser_event(
        handle,
        kind,
        peer,
        lane,
        generation,
        slice::from_raw_parts(data, size),
    );
}
