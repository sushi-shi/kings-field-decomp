//! Lossless conversions for the native and WebAssembly targets.

const _: () = assert!(usize::BITS == 32 || usize::BITS == 64);
const _: () = assert!(u32::BITS <= usize::BITS && usize::BITS <= u64::BITS);

pub(crate) trait AsUsize: Copy {
    fn as_usize(self) -> usize;
}

pub(crate) trait AsU64: Copy {
    fn as_u64(self) -> u64;
}

#[expect(
    clippy::as_conversions,
    reason = "the platform width assertions prove this is lossless"
)]
pub(crate) const fn u32_to_usize(value: u32) -> usize {
    value as usize
}

impl AsUsize for u32 {
    fn as_usize(self) -> usize {
        u32_to_usize(self)
    }
}

impl AsU64 for usize {
    #[expect(
        clippy::as_conversions,
        reason = "the platform width assertions prove this is lossless"
    )]
    fn as_u64(self) -> u64 {
        self as u64
    }
}

macro_rules! widening {
    ($trait:ident, $method:ident, $target:ty; $($source:ty),+) => { $(
        const _: () = assert!(<$source>::BITS <= <$target>::BITS);
        impl $trait for $source {
            fn $method(self) -> $target { <$target>::from(self) }
        }
    )+ };
}

widening!(AsUsize, as_usize, usize; u8, u16);
widening!(AsU64, as_u64, u64; u8, u16, u32);
