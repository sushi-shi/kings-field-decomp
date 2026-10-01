use super::bindings::*;
use crate::avatar::disc::Import;
use crate::avatar::{decode, Pack};

#[no_mangle]
pub unsafe extern "C" fn kf_avatar_open(bytes: *const u8, length: usize) -> *mut KfAvatarPack {
    if bytes.is_null() || length > KF_AVATAR_MAX_BYTES as usize {
        return core::ptr::null_mut();
    }
    decode(core::slice::from_raw_parts(bytes, length))
        .map_or(core::ptr::null_mut(), |p| Box::into_raw(Box::new(p)).cast())
}
#[no_mangle]
pub unsafe extern "C" fn kf_avatar_close(pack: *mut KfAvatarPack) {
    if !pack.is_null() {
        drop(Box::from_raw(pack.cast::<Pack>()));
    }
}
#[no_mangle]
pub unsafe extern "C" fn kf_avatar_count(pack: *const KfAvatarPack) -> u32 {
    pack.cast::<Pack>().as_ref().map_or(0, |p| p.0.len() as u32)
}
#[no_mangle]
pub unsafe extern "C" fn kf_avatar_mesh(
    pack: *const KfAvatarPack,
    index: u32,
    output: *mut KfAvatarMesh,
) -> i32 {
    if !super::network::aligned(output) {
        return 0;
    }
    let Some(pack) = pack.cast::<Pack>().as_ref() else {
        return 0;
    };
    let Some(mesh) = pack.0.get(index as usize) else {
        return 0;
    };
    let Some(output) = output.as_mut() else {
        return 0;
    };
    *output = KfAvatarMesh {
        triangles: (mesh.vertices.len() / 3) as u32,
        slot: mesh.slot,
        height: mesh.height,
    };
    1
}
#[no_mangle]
pub unsafe extern "C" fn kf_avatar_vertices(
    pack: *const KfAvatarPack,
    index: u32,
) -> *const KfAvatarVertex {
    pack.cast::<Pack>()
        .as_ref()
        .and_then(|p| p.0.get(index as usize))
        .map_or(core::ptr::null(), |m| m.vertices.as_ptr())
}
#[no_mangle]
pub unsafe extern "C" fn kf_avatar_rgba(pack: *const KfAvatarPack, index: u32) -> *const u8 {
    pack.cast::<Pack>()
        .as_ref()
        .and_then(|p| p.0.get(index as usize))
        .map_or(core::ptr::null(), |m| m.rgba.as_ptr())
}

#[no_mangle]
pub extern "C" fn kf_avatar_import_open(size: u32) -> *mut KfAvatarImport {
    Import::new(size).map_or(core::ptr::null_mut(), |i| Box::into_raw(Box::new(i)).cast())
}
#[no_mangle]
pub unsafe extern "C" fn kf_avatar_import_close(importer: *mut KfAvatarImport) {
    if !importer.is_null() {
        drop(Box::from_raw(importer.cast::<Import>()));
    }
}
#[no_mangle]
pub unsafe extern "C" fn kf_avatar_import_request(
    importer: *const KfAvatarImport,
    output: *mut KfAvatarRead,
) -> i32 {
    if !super::network::aligned(output) {
        return -1;
    }
    let Some(importer) = importer.cast::<Import>().as_ref() else {
        return -1;
    };
    match importer.request() {
        Ok(Some((offset, length))) => {
            *output = KfAvatarRead { offset, length };
            1
        }
        Ok(None) => 0,
        Err(_) => -1,
    }
}
#[no_mangle]
pub unsafe extern "C" fn kf_avatar_import_supply(
    importer: *mut KfAvatarImport,
    bytes: *const u8,
    length: usize,
) -> i32 {
    if bytes.is_null() || length > 40 * 1024 * 1024 {
        return 0;
    }
    let Some(importer) = importer.cast::<Import>().as_mut() else {
        return 0;
    };
    i32::from(
        importer
            .supply(core::slice::from_raw_parts(bytes, length))
            .is_ok(),
    )
}
#[no_mangle]
pub unsafe extern "C" fn kf_avatar_import_result(
    importer: *const KfAvatarImport,
    length: *mut u32,
) -> *const u8 {
    if !super::network::aligned(length) {
        return core::ptr::null();
    }
    let Some(importer) = importer.cast::<Import>().as_ref() else {
        return core::ptr::null();
    };
    let Some(result) = importer.result() else {
        return core::ptr::null();
    };
    *length = result.len() as u32;
    result.as_ptr()
}
#[no_mangle]
pub unsafe extern "C" fn kf_avatar_import_error(
    importer: *const KfAvatarImport,
    message: *mut u8,
    capacity: usize,
) {
    if message.is_null() || capacity == 0 || capacity > 512 {
        return;
    }
    let error = importer
        .cast::<Import>()
        .as_ref()
        .map_or("Cannot start character import", |i| i.error);
    let length = error.len().min(capacity - 1);
    core::ptr::copy_nonoverlapping(error.as_ptr(), message, length);
    *message.add(length) = 0;
}
