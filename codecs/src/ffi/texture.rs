use super::bindings::*;
use super::{input_valid, output_valid, INVALID, OK, OUTPUT_FULL};
use crate::bytes::read_u16_le;
use crate::tim::{ImageBlock, Images};
use crate::tim::{TIM_DIRECT16, TIM_FLAGS_MASK, TIM_FORMAT_MASK};
use core::slice;
const TEXTURE_WIDTH: usize = 1024;
const TEXTURE_HEIGHT: usize = 512;

fn rectangle_valid(block: &ImageBlock<'_>) -> bool {
    let r = block.rectangle;
    r.x.get() >= 0
        && r.y.get() >= 0
        && r.x.get() < TEXTURE_WIDTH as i16
        && r.y.get() < TEXTURE_HEIGHT as i16
        && r.width.get() <= TEXTURE_WIDTH as i16
        && r.height.get() <= TEXTURE_HEIGHT as i16
}
fn copy_block(block: &ImageBlock<'_>, words: &mut [u16]) -> Result<(), crate::bytes::ReadError> {
    let r = block.rectangle;
    for y in 0..r.height.get() as usize {
        for x in 0..r.width.get() as usize {
            let at = (y * r.width.get() as usize + x) * 2;
            // Common CLUT rectangles cross row 511. Authored transfers wrap at
            // the image edges; retain this only during material conversion.
            words[((r.y.get() as usize + y) & (TEXTURE_HEIGHT - 1)) * TEXTURE_WIDTH
                + ((r.x.get() as usize + x) & (TEXTURE_WIDTH - 1))] =
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
    if !input_valid(bytes, length) || !output_valid(words, capacity) {
        return INVALID;
    }
    if capacity < TEXTURE_WIDTH * TEXTURE_HEIGHT {
        return OUTPUT_FULL;
    }
    let bytes = slice::from_raw_parts(bytes, length);
    let mut count = 0;
    for image in Images::new(bytes) {
        let image = match image {
            Ok(i) => i,
            Err(error) => return error.into(),
        };
        if image.mode & TIM_FORMAT_MASK > TIM_DIRECT16
            || image.mode & !TIM_FLAGS_MASK != 0
            || !rectangle_valid(&image.image)
            || image.clut.is_some_and(|c| !rectangle_valid(&c))
        {
            return INVALID;
        }
        count += 1;
    }
    if count == 0 {
        return INVALID;
    }
    let words = slice::from_raw_parts_mut(words, TEXTURE_WIDTH * TEXTURE_HEIGHT);
    for image in Images::new(bytes) {
        let image = match image {
            Ok(image) => image,
            Err(error) => return error.into(),
        };
        if let Some(clut) = image.clut {
            if let Err(error) = copy_block(&clut, words) {
                return error.into();
            }
        }
        if let Err(error) = copy_block(&image.image, words) {
            return error.into();
        }
    }
    OK
}
