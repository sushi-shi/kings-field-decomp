#![no_std]
mod audio;
mod texture;
use core::{panic::PanicInfo, slice};
use kf_codec::tim::{Image, Images};
use kf_codec::tim::{
    TIM_DIRECT16, TIM_DIRECT24, TIM_FLAGS_MASK, TIM_FORMAT_MASK, TIM_INDEXED4, TIM_INDEXED8,
};
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
#[no_mangle]
pub extern "C" fn rust_eh_personality() -> ! {
    unsafe { abort() }
}
#[panic_handler]
fn panic(_: &PanicInfo<'_>) -> ! {
    // Programming errors must not unwind across the C ABI.
    unsafe { abort() }
}
const OK: i32 = 0;
const END: i32 = 1;
const INVALID: i32 = 2;
const OUTPUT_FULL: i32 = 3;
#[repr(C)]
pub struct TimInfo {
    mode: u32,
    width: u32,
    height: u32,
    encoded_bytes: u32,
    image_x: i32,
    image_y: i32,
    palette_x: i32,
    palette_y: i32,
}
fn parse(bytes: &[u8], offset: usize) -> Result<Image<'_>, i32> {
    let tail = bytes.get(offset..).ok_or(INVALID)?;
    Images::new(tail).next().ok_or(END)?.map_err(|_| INVALID)
}
fn dimensions(image: &Image<'_>) -> Result<(usize, usize), i32> {
    let words = image.image.rectangle.width as usize;
    let height = image.image.rectangle.height as usize;
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
#[no_mangle]
pub unsafe extern "C" fn kf_tim_info(
    bytes: *const u8,
    length: usize,
    offset: usize,
    info: *mut TimInfo,
) -> i32 {
    if bytes.is_null() || info.is_null() || length > isize::MAX as usize {
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
    if image.encoded_len > u32::MAX as usize {
        return INVALID;
    }
    info.write(TimInfo {
        mode: image.mode,
        width: width as u32,
        height: height as u32,
        encoded_bytes: image.encoded_len as u32,
        image_x: image.image.rectangle.x as i32,
        image_y: image.image.rectangle.y as i32,
        palette_x: image.clut.map_or(0, |c| c.rectangle.x as i32),
        palette_y: image.clut.map_or(0, |c| c.rectangle.y as i32),
    });
    OK
}
#[no_mangle]
pub unsafe extern "C" fn kf_tim_rgba(
    bytes: *const u8,
    length: usize,
    offset: usize,
    palette_row: u32,
    rgba: *mut u8,
    capacity: usize,
) -> i32 {
    if bytes.is_null()
        || rgba.is_null()
        || length > isize::MAX as usize
        || capacity > isize::MAX as usize
    {
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
        let at = match (palette_row as usize).checked_mul(count * 2) {
            Some(at) => at,
            None => return INVALID,
        };
        match clut.pixels.get(at..at.saturating_add(count * 2)) {
            Some(p) => p,
            None => return INVALID,
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
            u16::from_le_bytes([
                image.image.pixels[index * 2],
                image.image.pixels[index * 2 + 1],
            ])
        } else {
            let entry = if mode == TIM_INDEXED4 {
                ((image.image.pixels[index / 2] >> ((index % 2) * INDEXED4_BITS)) & INDEXED4_MASK)
                    as usize
            } else {
                image.image.pixels[index] as usize
            };
            u16::from_le_bytes([palette[entry * 2], palette[entry * 2 + 1]])
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
