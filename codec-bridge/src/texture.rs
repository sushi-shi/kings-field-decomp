use crate::{INVALID, OK, OUTPUT_FULL};
use core::{mem::align_of, slice};
use kf_codec::tim::{ImageBlock, Images};
fn rectangle_valid(block: &ImageBlock<'_>) -> bool {
    let r = block.rectangle;
    r.x >= 0 && r.y >= 0 && r.x < 1024 && r.y < 512 && r.width <= 1024 && r.height <= 512
}
fn copy_block(block: &ImageBlock<'_>, words: &mut [u16]) {
    let r = block.rectangle;
    for y in 0..r.height as usize {
        for x in 0..r.width as usize {
            let at = (y * r.width as usize + x) * 2;
            // Common CLUT rectangles cross row 511. Authored transfers wrap at
            // the image edges; retain this only during material conversion.
            words[((r.y as usize + y) & 511) * 1024 + ((r.x as usize + x) & 1023)] =
                u16::from_le_bytes([block.pixels[at], block.pixels[at + 1]]);
        }
    }
}
#[no_mangle]
pub unsafe extern "C" fn kf_tim_compose(
    bytes: *const u8,
    length: usize,
    words: *mut u16,
    capacity: usize,
) -> i32 {
    if bytes.is_null()
        || words.is_null()
        || length > isize::MAX as usize
        || words as usize % align_of::<u16>() != 0
        || capacity > isize::MAX as usize / 2
    {
        return INVALID;
    }
    if capacity < 1024 * 512 {
        return OUTPUT_FULL;
    }
    let bytes = slice::from_raw_parts(bytes, length);
    let mut count = 0;
    for image in Images::new(bytes) {
        let image = match image {
            Ok(i) => i,
            Err(_) => return INVALID,
        };
        if image.mode & 7 > 2
            || image.mode & !15 != 0
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
    let words = slice::from_raw_parts_mut(words, 1024 * 512);
    for image in Images::new(bytes) {
        let image = image.expect("preflighted TIM stream");
        if let Some(clut) = image.clut {
            copy_block(&clut, words);
        }
        copy_block(&image.image, words);
    }
    OK
}
