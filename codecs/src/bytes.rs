//! Bounded byte access shared by the resource codecs.
#![deny(clippy::as_conversions)]

use core::panic::Location;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub(crate) struct ReadError {
    pub location: &'static Location<'static>,
    pub at: usize,
    pub need: usize,
    pub available: usize,
}

type Result<T> = core::result::Result<T, ReadError>;

#[track_caller]
pub(crate) fn span(bytes: &[u8], at: usize, size: usize) -> Result<&[u8]> {
    at.checked_add(size)
        .and_then(|end| bytes.get(at..end))
        .ok_or(ReadError {
            location: Location::caller(),
            at,
            need: size,
            available: bytes.len().saturating_sub(at),
        })
}

#[track_caller]
pub(crate) fn records(bytes: &[u8], at: usize, count: usize, width: usize) -> Result<&[u8]> {
    let size = count.checked_mul(width).ok_or(ReadError {
        location: Location::caller(),
        at,
        need: usize::MAX,
        available: bytes.len().saturating_sub(at),
    })?;
    span(bytes, at, size)
}

#[track_caller]
pub(crate) fn read<T: bytemuck::AnyBitPattern>(bytes: &[u8], at: usize) -> Result<T> {
    // The bounded span has exactly T's size; unaligned reads copy the value.
    Ok(bytemuck::pod_read_unaligned(span(
        bytes,
        at,
        size_of::<T>(),
    )?))
}

#[track_caller]
pub(crate) fn read_u16_le(bytes: &[u8], at: usize) -> Result<u16> {
    read(bytes, at).map(u16::from_le)
}

#[track_caller]
pub(crate) fn read_i16_le(bytes: &[u8], at: usize) -> Result<i16> {
    read(bytes, at).map(i16::from_le)
}

#[track_caller]
pub(crate) fn read_i32_le(bytes: &[u8], at: usize) -> Result<i32> {
    read(bytes, at).map(i32::from_le)
}

#[track_caller]
pub(crate) fn read_u32_le(bytes: &[u8], at: usize) -> Result<u32> {
    read(bytes, at).map(u32::from_le)
}

#[track_caller]
pub(crate) fn read_u16_be(bytes: &[u8], at: usize) -> Result<u16> {
    read(bytes, at).map(u16::from_be)
}

#[track_caller]
pub(crate) fn read_u32_be(bytes: &[u8], at: usize) -> Result<u32> {
    read(bytes, at).map(u32::from_be)
}

#[track_caller]
pub(crate) fn read_u24_be(bytes: &[u8], at: usize) -> Result<u32> {
    let bytes = span(bytes, at, 3)?;
    Ok(u32::from_be(bytemuck::cast([
        0, bytes[0], bytes[1], bytes[2],
    ])))
}
