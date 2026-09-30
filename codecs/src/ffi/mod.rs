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
struct Diagnostic;

impl core::fmt::Write for Diagnostic {
    fn write_str(&mut self, text: &str) -> core::fmt::Result {
        unsafe extern "C" {
            fn printf(format: *const core::ffi::c_char, ...) -> core::ffi::c_int;
        }
        // The precision bounds each non-NUL-terminated Rust string.
        for chunk in text.as_bytes().chunks(i32::MAX as usize) {
            unsafe {
                printf(c"%.*s".as_ptr(), chunk.len() as i32, chunk.as_ptr());
            }
        }
        Ok(())
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
impl From<crate::Error> for KfCodecResult {
    fn from(error: crate::Error) -> Self {
        use core::fmt::Write;
        let status = match error.kind {
            crate::error::Kind::End => return END,
            crate::error::Kind::OutputFull => OUTPUT_FULL,
            _ => INVALID,
        };
        let _ = writeln!(
            Diagnostic,
            "kf-codec: {}:{}:{}: {}",
            error.location.file(),
            error.location.line(),
            error.location.column(),
            error
        );
        status
    }
}

fn parse(bytes: &[u8], offset: usize) -> crate::Result<Image<'_>> {
    let tail = bytes
        .get(offset..)
        .ok_or(crate::Error::invalid("TIM offset exceeds input"))?;
    Images::new(tail).next().ok_or(crate::Error::end())?
}

fn dimensions(image: &Image<'_>) -> crate::Result<(usize, usize)> {
    let words = image.image.rectangle.width.get() as usize;
    let height = image.image.rectangle.height.get() as usize;
    let width = match image.mode & TIM_FORMAT_MASK {
        TIM_INDEXED4 => words * 4,
        TIM_INDEXED8 => words * 2,
        TIM_DIRECT16 => words,
        TIM_DIRECT24 if words * 2 % 3 == 0 => words * 2 / 3,
        _ => return Err(crate::Error::invalid("invalid TIM mode or dimensions")),
    };
    if width == 0 || height == 0 || image.mode & !TIM_FLAGS_MASK != 0 {
        return Err(crate::Error::invalid("invalid TIM mode or dimensions"));
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
        return crate::Error::invalid("invalid TIM pointer or size").into();
    }
    let image = match parse(slice::from_raw_parts(bytes, length), offset) {
        Ok(i) => i,
        Err(e) => return e.into(),
    };
    let (width, height) = match dimensions(&image) {
        Ok(d) => d,
        Err(e) => return e.into(),
    };
    if image.encoded_len > u32::MAX.as_usize() {
        return crate::Error::invalid("TIM size exceeds u32").into();
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
        return crate::Error::invalid("invalid TIM pointer or size").into();
    }
    let image = match parse(slice::from_raw_parts(bytes, length), offset) {
        Ok(i) => i,
        Err(e) => return e.into(),
    };
    let (width, height) = match dimensions(&image) {
        Ok(d) => d,
        Err(e) => return e.into(),
    };
    let needed = match width.checked_mul(height).and_then(|n| n.checked_mul(4)) {
        Some(n) => n,
        None => return crate::Error::invalid("TIM pixel count overflow").into(),
    };
    if capacity < needed {
        return crate::Error::output_full().into();
    }
    let mode = image.mode & TIM_FORMAT_MASK;
    let palette = if mode < TIM_DIRECT16 {
        let clut = match image.clut {
            Some(c) => c,
            None => return crate::Error::invalid("indexed TIM has no palette").into(),
        };
        let count = if mode == TIM_INDEXED4 {
            INDEXED4_PALETTE_COLORS
        } else {
            INDEXED8_PALETTE_COLORS
        };
        let at = match palette_row.as_usize().checked_mul(count * 2) {
            Some(at) => at,
            None => return crate::Error::invalid("TIM palette offset overflow").into(),
        };
        match span(clut.pixels, at, count * 2) {
            Ok(p) => p,
            Err(error) => return error.into(),
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
