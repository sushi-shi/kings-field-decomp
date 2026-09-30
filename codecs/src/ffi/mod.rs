use crate::cast::AsUsize;
pub(crate) mod bindings;
use bindings::*;
mod audio;
mod resources;
mod texture;
use crate::bytes::{read_u16_le, span};
use crate::tim::{Image, Images};
use crate::tim::{
    TIM_DIRECT16, TIM_DIRECT24, TIM_FLAGS_MASK, TIM_FORMAT_MASK, TIM_INDEXED4, TIM_INDEXED8,
};
use core::{panic::PanicInfo, slice};
fn report_error(location: &'static core::panic::Location<'static>, message: &core::ffi::CStr) {
    unsafe extern "C" {
        fn printf(format: *const core::ffi::c_char, ...) -> core::ffi::c_int;
    }
    // printf is supplied by the native/browser C runtime; lengths bound Rust's non-NUL file name.
    unsafe {
        printf(
            c"kf-codec: %.*s:%u:%u: %s\n".as_ptr(),
            i32::try_from(location.file().len()).unwrap_or(i32::MAX),
            location.file().as_ptr(),
            location.line(),
            location.column(),
            message.as_ptr(),
        );
    }
}

// C callers still guarantee valid allocations and non-overlapping borrows.
fn input_valid(data: *const u8, length: usize) -> bool {
    !data.is_null() && length <= isize::MAX as usize
}

fn output_valid<T>(data: *mut T, count: usize) -> bool {
    !data.is_null()
        && data.is_aligned()
        && count
            .checked_mul(size_of::<T>())
            .is_some_and(|size| size <= isize::MAX as usize)
}

const INDEXED4_PALETTE_COLORS: usize = 16;
const INDEXED4_BITS: usize = 4;
const INDEXED4_MASK: u8 = 15;
const INDEXED8_PALETTE_COLORS: usize = 256;
const RGB5_BITS: u32 = 5;
const RGB5_MASK: u16 = 31;
const RGB5_RGB8_SHIFT: u32 = 3;
const RGB5_REPLICATION_SHIFT: u32 = 2;
const ALPHA_OPAQUE: u8 = 255;

extern "C" {
    fn abort() -> !;
}
// Native prebuilt core retains an unwind metadata reference. This library uses
// panic=abort; entering an unwinder is a fatal ABI violation, never a no-op.
#[unsafe(no_mangle)]
pub extern "C" fn rust_eh_personality() -> ! {
    unsafe { abort() }
}
#[panic_handler]
fn panic(_: &PanicInfo<'_>) -> ! {
    // Programming errors must not unwind across the C ABI.
    unsafe { abort() }
}
const OK: KfCodecResult = KF_CODEC_OK;
const END: KfCodecResult = KF_CODEC_END;
const INVALID: KfCodecResult = KF_CODEC_INVALID;
const OUTPUT_FULL: KfCodecResult = KF_CODEC_OUTPUT_FULL;
impl From<crate::bytes::ReadError> for KfCodecResult {
    fn from(error: crate::bytes::ReadError) -> Self {
        report_error(error.location, c"truncated codec input");
        INVALID
    }
}

impl From<crate::tim::TimError> for KfCodecResult {
    fn from(error: crate::tim::TimError) -> Self {
        if let crate::tim::TimError::Truncated { location, .. } = error {
            report_error(location, c"truncated TIM input");
        }
        INVALID
    }
}

fn parse(bytes: &[u8], offset: usize) -> Result<Image<'_>, KfCodecResult> {
    let tail = bytes.get(offset..).ok_or(INVALID)?;
    Images::new(tail).next().ok_or(END)?.map_err(Into::into)
}

fn dimensions(image: &Image<'_>) -> Result<(usize, usize), KfCodecResult> {
    let words = image.image.rectangle.width.get() as usize;
    let height = image.image.rectangle.height.get() as usize;
    let width = match image.mode & TIM_FORMAT_MASK {
        TIM_INDEXED4 => words * 4,
        TIM_INDEXED8 => words * 2,
        TIM_DIRECT16 => words,
        TIM_DIRECT24 if words * 2 % 3 == 0 => words * 2 / 3,
        _ => return Err(INVALID),
    };
    if width == 0 || height == 0 || image.mode & !TIM_FLAGS_MASK != 0 {
        return Err(INVALID);
    }
    Ok((width, height))
}
#[unsafe(no_mangle)]
pub unsafe extern "C" fn kf_tim_info(
    bytes: *const u8,
    length: usize,
    offset: usize,
    info: *mut KfTimInfo,
) -> KfCodecResult {
    if !input_valid(bytes, length) || !output_valid(info, 1) {
        return INVALID;
    }
    let image = match parse(slice::from_raw_parts(bytes, length), offset) {
        Ok(i) => i,
        Err(e) => return e,
    };
    let (width, height) = match dimensions(&image) {
        Ok(d) => d,
        Err(e) => return e,
    };
    if image.encoded_len > u32::MAX.as_usize() {
        return INVALID;
    }
    info.write(KfTimInfo {
        mode: image.mode,
        width: width as u32,
        height: height as u32,
        encoded_bytes: image.encoded_len as u32,
        image_x: image.image.rectangle.x.get() as i32,
        image_y: image.image.rectangle.y.get() as i32,
        palette_x: image.clut.map_or(0, |c| c.rectangle.x.get() as i32),
        palette_y: image.clut.map_or(0, |c| c.rectangle.y.get() as i32),
    });
    OK
}
#[unsafe(no_mangle)]
pub unsafe extern "C" fn kf_tim_rgba(
    bytes: *const u8,
    length: usize,
    offset: usize,
    palette_row: u32,
    rgba: *mut u8,
    capacity: usize,
) -> KfCodecResult {
    if !input_valid(bytes, length) || !output_valid(rgba, capacity) {
        return INVALID;
    }
    let image = match parse(slice::from_raw_parts(bytes, length), offset) {
        Ok(i) => i,
        Err(e) => return e,
    };
    let (width, height) = match dimensions(&image) {
        Ok(d) => d,
        Err(e) => return e,
    };
    let needed = match width.checked_mul(height).and_then(|n| n.checked_mul(4)) {
        Some(n) => n,
        None => return INVALID,
    };
    if capacity < needed {
        return OUTPUT_FULL;
    }
    let mode = image.mode & TIM_FORMAT_MASK;
    let palette = if mode < TIM_DIRECT16 {
        let clut = match image.clut {
            Some(c) => c,
            None => return INVALID,
        };
        let count = if mode == TIM_INDEXED4 {
            INDEXED4_PALETTE_COLORS
        } else {
            INDEXED8_PALETTE_COLORS
        };
        let at = match palette_row.as_usize().checked_mul(count * 2) {
            Some(at) => at,
            None => return INVALID,
        };
        match span(clut.pixels, at, count * 2) {
            Ok(p) => p,
            Err(_) => return INVALID,
        }
    } else {
        &[]
    };
    let output = slice::from_raw_parts_mut(rgba, needed);
    for (index, destination) in output.chunks_exact_mut(4).enumerate() {
        if mode == TIM_DIRECT24 {
            let at = index * 3;
            destination[..3].copy_from_slice(&image.image.pixels[at..at + 3]);
            destination[3] = ALPHA_OPAQUE;
            continue;
        }
        let word = if mode == TIM_DIRECT16 {
            read_u16_le(image.image.pixels, index * 2)
        } else {
            let entry = if mode == TIM_INDEXED4 {
                ((image.image.pixels[index / 2] >> ((index % 2) * INDEXED4_BITS)) & INDEXED4_MASK)
                    as usize
            } else {
                image.image.pixels[index].as_usize()
            };
            read_u16_le(palette, entry * 2)
        };
        let word = match word {
            Ok(word) => word,
            Err(error) => return error.into(),
        };
        for (channel, shift) in destination[..3]
            .iter_mut()
            .zip([0, RGB5_BITS, 2 * RGB5_BITS])
        {
            let value = ((word >> shift) & RGB5_MASK) as u8;
            *channel = (value << RGB5_RGB8_SHIFT) | (value >> RGB5_REPLICATION_SHIFT);
        }
        destination[3] = if word == 0 { 0 } else { ALPHA_OPAQUE };
    }
    OK
}
