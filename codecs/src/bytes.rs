//! Bounded byte access shared by the resource codecs.

pub(crate) fn span(bytes: &[u8], at: usize, size: usize) -> Option<&[u8]> {
    bytes.get(at..at.checked_add(size)?)
}

pub(crate) fn records(bytes: &[u8], at: usize, count: usize, width: usize) -> Option<&[u8]> {
    span(bytes, at, count.checked_mul(width)?)
}

pub(crate) fn read<T: bytemuck::AnyBitPattern>(bytes: &[u8], at: usize) -> Option<T> {
    bytemuck::try_pod_read_unaligned(span(bytes, at, size_of::<T>())?).ok()
}

pub(crate) fn read_u16_le(bytes: &[u8], at: usize) -> Option<u16> {
    read(bytes, at).map(u16::from_le)
}

pub(crate) fn read_u32_le(bytes: &[u8], at: usize) -> Option<u32> {
    read(bytes, at).map(u32::from_le)
}

pub(crate) fn read_u16_be(bytes: &[u8], at: usize) -> Option<u16> {
    read(bytes, at).map(u16::from_be)
}

pub(crate) fn read_u32_be(bytes: &[u8], at: usize) -> Option<u32> {
    read(bytes, at).map(u32::from_be)
}

pub(crate) fn read_u24_be(bytes: &[u8], at: usize) -> Option<u32> {
    let bytes = span(bytes, at, 3)?;
    Some(u32::from_be(bytemuck::cast([
        0, bytes[0], bytes[1], bytes[2],
    ])))
}
