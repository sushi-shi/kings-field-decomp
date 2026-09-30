//! Bounded byte access shared by the resource codecs.
#![deny(clippy::as_conversions)]

macro_rules! endian {
    ($name:ident, $value:ty, $size:literal, $decode:ident) => {
        #[repr(transparent)]
        #[derive(Clone, Copy, Debug, bytemuck::Pod, bytemuck::Zeroable)]
        pub(crate) struct $name([u8; $size]);

        impl $name {
            pub(crate) fn get(self) -> $value {
                <$value>::$decode(self.0)
            }
        }
    };
}

endian!(LeU16, u16, 2, from_le_bytes);
endian!(LeI16, i16, 2, from_le_bytes);
endian!(LeU32, u32, 4, from_le_bytes);
endian!(BeU16, u16, 2, from_be_bytes);
endian!(BeU32, u32, 4, from_be_bytes);

#[repr(transparent)]
#[derive(Clone, Copy, Debug, bytemuck::Pod, bytemuck::Zeroable)]
pub(crate) struct BeU24([u8; 3]);

impl BeU24 {
    pub(crate) fn get(self) -> u32 {
        let [a, b, c] = self.0;
        u32::from_be_bytes([0, a, b, c])
    }
}

#[track_caller]
pub(crate) fn span(bytes: &[u8], at: usize, size: usize) -> crate::Result<&[u8]> {
    at.checked_add(size)
        .and_then(|end| bytes.get(at..end))
        .ok_or(crate::Error::truncated(
            at,
            size,
            bytes.len().saturating_sub(at),
        ))
}

#[track_caller]
pub(crate) fn records<T: bytemuck::AnyBitPattern>(
    bytes: &[u8],
    at: usize,
    count: usize,
) -> crate::Result<impl ExactSizeIterator<Item = T> + '_> {
    const { assert!(size_of::<T>() != 0) };
    let size = count
        .checked_mul(size_of::<T>())
        .ok_or(crate::Error::truncated(
            at,
            usize::MAX,
            bytes.len().saturating_sub(at),
        ))?;
    Ok(span(bytes, at, size)?
        .chunks_exact(size_of::<T>())
        .map(bytemuck::pod_read_unaligned))
}

#[track_caller]
pub(crate) fn read<T: bytemuck::AnyBitPattern>(bytes: &[u8], at: usize) -> crate::Result<T> {
    // The bounded span has exactly T's size; unaligned reads copy the value.
    Ok(bytemuck::pod_read_unaligned(span(
        bytes,
        at,
        size_of::<T>(),
    )?))
}

#[track_caller]
pub(crate) fn read_u16_le(bytes: &[u8], at: usize) -> crate::Result<u16> {
    read(bytes, at).map(LeU16::get)
}

#[track_caller]
pub(crate) fn read_u32_le(bytes: &[u8], at: usize) -> crate::Result<u32> {
    read(bytes, at).map(LeU32::get)
}

#[track_caller]
pub(crate) fn read_u24_be(bytes: &[u8], at: usize) -> crate::Result<u32> {
    read(bytes, at).map(BeU24::get)
}
