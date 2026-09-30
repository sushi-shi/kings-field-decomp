use crate::cast::AsUsize;
pub(crate) mod bindings;
use bindings::*;
mod audio;
mod resources;
mod texture;
use crate::bytes::{read_u16_le, span};
use crate::tim::{Image, Images, PixelFormat};
use core::{panic::PanicInfo, slice};
struct Diagnostic;

impl core::fmt::Write for Diagnostic {
    fn write_str(&mut self, text: &str) -> core::fmt::Result {
        unsafe extern "C" {
            fn printf(format: *const core::ffi::c_char, ...) -> core::ffi::c_int;
        }
        let length = core::ffi::c_int::try_from(text.len()).map_err(|_| core::fmt::Error)?;
        // The precision bounds this non-NUL-terminated Rust string.
        unsafe {
            printf(c"%.*s".as_ptr(), length, text.as_ptr());
        }
        Ok(())
    }
}

// C callers still guarantee valid allocations and non-overlapping borrows.
fn input_valid(data: *const u8, length: usize) -> bool {
    !data.is_null() && isize::try_from(length).is_ok()
}

fn output_valid<T>(data: *mut T, count: usize) -> bool {
    !data.is_null()
        && data.is_aligned()
        && count
            .checked_mul(size_of::<T>())
            .is_some_and(|size| isize::try_from(size).is_ok())
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
    fn fflush(stream: *mut core::ffi::c_void) -> core::ffi::c_int;
}
// Native prebuilt core retains an unwind metadata reference. This library uses
// panic=abort; entering an unwinder is a fatal ABI violation, never a no-op.
#[unsafe(no_mangle)]
pub extern "C" fn rust_eh_personality() -> ! {
    unsafe { abort() }
}
#[panic_handler]
fn panic(info: &PanicInfo<'_>) -> ! {
    use core::fmt::Write;
    let _ = writeln!(Diagnostic, "kf-codec panic: {info}");
    // Programming errors must not unwind across the C ABI.
    unsafe {
        fflush(core::ptr::null_mut());
        abort()
    }
}
fn status(result: crate::Result<()>) -> KfCodecResult {
    use core::fmt::Write;
    let Err(error) = result else {
        return KF_CODEC_OK;
    };
    let status = match error.kind {
        crate::error::Kind::End => return KF_CODEC_END,
        crate::error::Kind::OutputFull => KF_CODEC_OUTPUT_FULL,
        _ => KF_CODEC_INVALID,
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

fn parse(bytes: &[u8], offset: usize) -> crate::Result<Image<'_>> {
    let tail = bytes
        .get(offset..)
        .ok_or(crate::Error::invalid("TIM offset exceeds input"))?;
    Images::new(tail).next().ok_or(crate::Error::end())?
}

fn dimensions(image: &Image<'_>) -> crate::Result<(u32, u32)> {
    let words = u32::from(image.image.width);
    let height = u32::from(image.image.height);
    let width = match image.format {
        PixelFormat::Indexed4 => words * 4,
        PixelFormat::Indexed8 => words * 2,
        PixelFormat::Direct16 => words,
        PixelFormat::Direct24 if words * 2 % 3 == 0 => words * 2 / 3,
        _ => crate::bail!("invalid TIM mode or dimensions"),
    };
    if width == 0 || height == 0 {
        crate::bail!("invalid TIM mode or dimensions");
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
    status(tim_info(bytes, length, offset, info))
}

unsafe fn tim_info(
    bytes: *const u8,
    length: usize,
    offset: usize,
    info: *mut KfTimInfo,
) -> crate::Result<()> {
    if !input_valid(bytes, length) || !output_valid(info, 1) {
        crate::bail!("invalid TIM pointer or size");
    }
    let image = parse(slice::from_raw_parts(bytes, length), offset)?;
    let (width, height) = dimensions(&image)?;
    info.write(KfTimInfo {
        mode: image.encoded_mode(),
        width,
        height,
        encoded_bytes: image.encoded_len,
        image_x: i32::from(image.image.x),
        image_y: i32::from(image.image.y),
        palette_x: image.clut.map_or(0, |c| i32::from(c.x)),
        palette_y: image.clut.map_or(0, |c| i32::from(c.y)),
    });
    Ok(())
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
    status(tim_rgba(bytes, length, offset, palette_row, rgba, capacity))
}

unsafe fn tim_rgba(
    bytes: *const u8,
    length: usize,
    offset: usize,
    palette_row: u32,
    rgba: *mut u8,
    capacity: usize,
) -> crate::Result<()> {
    if !input_valid(bytes, length) || !output_valid(rgba, capacity) {
        crate::bail!("invalid TIM pointer or size");
    }
    let image = parse(slice::from_raw_parts(bytes, length), offset)?;
    let (width, height) = dimensions(&image)?;
    let needed = width
        .as_usize()
        .checked_mul(height.as_usize())
        .and_then(|n| n.checked_mul(4))
        .ok_or(crate::Error::invalid("TIM pixel count overflow"))?;
    if capacity < needed {
        return Err(crate::Error::output_full());
    }
    let format = image.format;
    let palette = if matches!(format, PixelFormat::Indexed4 | PixelFormat::Indexed8) {
        let clut = image
            .clut
            .ok_or(crate::Error::invalid("indexed TIM has no palette"))?;
        let count = if format == PixelFormat::Indexed4 {
            INDEXED4_PALETTE_COLORS
        } else {
            INDEXED8_PALETTE_COLORS
        };
        let at = palette_row
            .as_usize()
            .checked_mul(count * 2)
            .ok_or(crate::Error::invalid("TIM palette offset overflow"))?;
        span(clut.pixels, at, count * 2)?
    } else {
        &[]
    };
    let output = slice::from_raw_parts_mut(rgba, needed);
    for (index, destination) in output.chunks_exact_mut(4).enumerate() {
        if format == PixelFormat::Direct24 {
            let at = index * 3;
            destination[..3].copy_from_slice(&image.image.pixels[at..at + 3]);
            destination[3] = ALPHA_OPAQUE;
            continue;
        }
        let word = if format == PixelFormat::Direct16 {
            read_u16_le(image.image.pixels, index * 2)
        } else {
            let entry = if format == PixelFormat::Indexed4 {
                ((image.image.pixels[index / 2] >> ((index % 2) * INDEXED4_BITS)) & INDEXED4_MASK)
                    .as_usize()
            } else {
                image.image.pixels[index].as_usize()
            };
            read_u16_le(palette, entry * 2)
        };
        let word = word?;
        for (channel, shift) in destination[..3]
            .iter_mut()
            .zip([0, RGB5_BITS, 2 * RGB5_BITS])
        {
            let value = u8::try_from((word >> shift) & RGB5_MASK)?;
            *channel = (value << RGB5_RGB8_SHIFT) | (value >> RGB5_REPLICATION_SHIFT);
        }
        destination[3] = if word == 0 { 0 } else { ALPHA_OPAQUE };
    }
    Ok(())
}
