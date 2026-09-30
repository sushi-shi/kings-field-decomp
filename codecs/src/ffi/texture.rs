use super::bindings::*;
use super::{input_valid, output_valid, status};
use crate::bytes::read_u16_le;
use crate::cast::AsUsize;
use crate::tim::{ImageBlock, Images, PixelFormat};
use core::slice;
const TEXTURE_WIDTH: u16 = 1024;
const TEXTURE_HEIGHT: u16 = 512;

fn rectangle_valid(block: &ImageBlock<'_>) -> bool {
    let r = block;
    r.x >= 0
        && r.y >= 0
        && i32::from(r.x) < i32::from(TEXTURE_WIDTH)
        && i32::from(r.y) < i32::from(TEXTURE_HEIGHT)
        && r.width <= TEXTURE_WIDTH
        && r.height <= TEXTURE_HEIGHT
}
fn copy_block(block: &ImageBlock<'_>, words: &mut [u16]) -> crate::Result<()> {
    let r = block;
    let width = r.width.as_usize();
    let height = r.height.as_usize();
    let origin_x = usize::try_from(r.x)?;
    let origin_y = usize::try_from(r.y)?;
    for y in 0..height {
        for x in 0..width {
            let at = (y * width + x) * 2;
            // Common CLUT rectangles cross row 511. Authored transfers wrap at
            // the image edges; retain this only during material conversion.
            words[((origin_y + y) & (TEXTURE_HEIGHT.as_usize() - 1)) * TEXTURE_WIDTH.as_usize()
                + ((origin_x + x) & (TEXTURE_WIDTH.as_usize() - 1))] =
                read_u16_le(block.pixels, at)?;
        }
    }
    Ok(())
}
#[unsafe(no_mangle)]
pub unsafe extern "C" fn kf_tim_compose(
    bytes: *const u8,
    length: usize,
    words: *mut u16,
    capacity: usize,
) -> KfCodecResult {
    status(tim_compose(bytes, length, words, capacity))
}

unsafe fn tim_compose(
    bytes: *const u8,
    length: usize,
    words: *mut u16,
    capacity: usize,
) -> crate::Result<()> {
    if !input_valid(bytes, length) || !output_valid(words, capacity) {
        crate::bail!("invalid texture pointer or size");
    }
    if capacity < TEXTURE_WIDTH.as_usize() * TEXTURE_HEIGHT.as_usize() {
        return Err(crate::Error::output_full());
    }
    let bytes = slice::from_raw_parts(bytes, length);
    let mut count = 0;
    for image in Images::new(bytes) {
        let image = image?;
        if image.format == PixelFormat::Direct24
            || !rectangle_valid(&image.image)
            || image.clut.is_some_and(|c| !rectangle_valid(&c))
        {
            crate::bail!("invalid texture mode or rectangle");
        }
        count += 1;
    }
    if count == 0 {
        crate::bail!("texture contains no TIM images");
    }
    let words =
        slice::from_raw_parts_mut(words, TEXTURE_WIDTH.as_usize() * TEXTURE_HEIGHT.as_usize());
    for image in Images::new(bytes) {
        let image = image?;
        if let Some(clut) = image.clut {
            copy_block(&clut, words)?;
        }
        copy_block(&image.image, words)?;
    }
    Ok(())
}
