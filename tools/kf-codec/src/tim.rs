//! TIM image streams consumed by GAME's `tim_upload_images` (0x8001b100).
//!
//! The layout follows the supplied Release 2.5 LIBGPU `TIM_IMAGE` and the
//! linked `get_tim_addr` at 0x80052cb0. The SDK advances block lengths in
//! whole words and stops at the first word other than 0x10.

use core::fmt;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TimError {
    Truncated {
        at: usize,
        need: usize,
        available: usize,
    },
    InvalidBlockSize {
        at: usize,
        declared: u32,
    },
    InvalidRectangle {
        at: usize,
        width: i16,
        height: i16,
    },
}

impl fmt::Display for TimError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "TIM {self:?}")
    }
}

impl core::error::Error for TimError {}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Rect {
    pub x: i16,
    pub y: i16,
    pub width: i16,
    pub height: i16,
}

impl Rect {
    pub fn to_le_bytes(self) -> [u8; 8] {
        let mut bytes = [0; 8];
        for (destination, value) in
            bytes
                .chunks_exact_mut(2)
                .zip([self.x, self.y, self.width, self.height])
        {
            destination.copy_from_slice(&value.to_le_bytes());
        }
        bytes
    }
}

#[derive(Debug, Clone, Copy)]
pub struct ImageBlock<'a> {
    pub rectangle: Rect,
    pub rectangle_offset: usize,
    pub data_offset: usize,
    /// The halfwords consumed by LoadImage for this rectangle.
    pub pixels: &'a [u8],
    /// Bytes in the length-prefixed block after its rectangle, including padding.
    pub payload: &'a [u8],
}

#[derive(Debug, Clone, Copy)]
pub struct Image<'a> {
    pub offset: usize,
    pub mode: u32,
    pub clut: Option<ImageBlock<'a>>,
    pub image: ImageBlock<'a>,
    pub encoded_len: usize,
}

/// The five words of Psy-Q's `TIM_IMAGE`, using PSX addresses rather than
/// host pointers. The descriptor is separate from the borrowed image view.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct TimImageDescriptor {
    pub mode: u32,
    pub clut_rectangle: u32,
    pub clut_data: u32,
    pub image_rectangle: u32,
    pub image_data: u32,
}

impl TimImageDescriptor {
    pub fn to_le_bytes(self) -> [u8; 20] {
        let mut bytes = [0; 20];
        for (destination, word) in bytes.chunks_exact_mut(4).zip([
            self.mode,
            self.clut_rectangle,
            self.clut_data,
            self.image_rectangle,
            self.image_data,
        ]) {
            destination.copy_from_slice(&word.to_le_bytes());
        }
        bytes
    }
}

impl Image<'_> {
    /// `stream_address` is the address of the complete input to `Images`, not
    /// this record's start. Address arithmetic intentionally has PSX u32 width.
    pub fn psx_descriptor(&self, stream_address: u32) -> TimImageDescriptor {
        let address = |offset: usize| stream_address.wrapping_add(offset as u32);
        TimImageDescriptor {
            mode: self.mode,
            clut_rectangle: self.clut.map_or(0, |block| address(block.rectangle_offset)),
            clut_data: self.clut.map_or(0, |block| address(block.data_offset)),
            image_rectangle: address(self.image.rectangle_offset),
            image_data: address(self.image.data_offset),
        }
    }
}

pub struct Images<'a> {
    bytes: &'a [u8],
    at: usize,
    stopped: bool,
}

impl<'a> Images<'a> {
    pub const fn new(bytes: &'a [u8]) -> Self {
        Self {
            bytes,
            at: 0,
            stopped: false,
        }
    }

    pub const fn position(&self) -> usize {
        self.at
    }

    pub fn remainder(&self) -> &'a [u8] {
        &self.bytes[self.at..]
    }

    fn parse_next(&mut self) -> Result<Option<Image<'a>>, TimError> {
        if self.bytes.len() - self.at < 4 || u32_at(self.bytes, self.at)? != 0x10 {
            self.stopped = true;
            return Ok(None);
        }
        let start = self.at;
        let mode = u32_at(self.bytes, start + 4)?;
        let mut cursor = start + 8;
        let clut = if mode & 8 != 0 {
            Some(block(self.bytes, &mut cursor)?)
        } else {
            None
        };
        let image = block(self.bytes, &mut cursor)?;
        self.at = cursor;
        Ok(Some(Image {
            offset: start,
            mode,
            clut,
            image,
            encoded_len: cursor - start,
        }))
    }
}

impl<'a> Iterator for Images<'a> {
    type Item = Result<Image<'a>, TimError>;

    fn next(&mut self) -> Option<Self::Item> {
        if self.stopped {
            return None;
        }
        match self.parse_next() {
            Ok(Some(image)) => Some(Ok(image)),
            Ok(None) => None,
            Err(error) => {
                self.stopped = true;
                Some(Err(error))
            }
        }
    }
}

fn span(bytes: &[u8], at: usize, size: usize) -> Result<&[u8], TimError> {
    at.checked_add(size)
        .and_then(|end| bytes.get(at..end))
        .ok_or(TimError::Truncated {
            at,
            need: size,
            available: bytes.len().saturating_sub(at),
        })
}

fn u32_at(bytes: &[u8], at: usize) -> Result<u32, TimError> {
    Ok(u32::from_le_bytes(span(bytes, at, 4)?.try_into().unwrap()))
}

fn block<'a>(bytes: &'a [u8], cursor: &mut usize) -> Result<ImageBlock<'a>, TimError> {
    let at = *cursor;
    let declared = u32_at(bytes, at)?;
    let size =
        usize::try_from(declared & !3).map_err(|_| TimError::InvalidBlockSize { at, declared })?;
    if size < 12 {
        return Err(TimError::InvalidBlockSize { at, declared });
    }
    let encoded = span(bytes, at, size)?;
    let half = |offset| i16::from_le_bytes([encoded[offset], encoded[offset + 1]]);
    let rectangle = Rect {
        x: half(4),
        y: half(6),
        width: half(8),
        height: half(10),
    };
    if rectangle.width < 0 || rectangle.height < 0 {
        return Err(TimError::InvalidRectangle {
            at,
            width: rectangle.width,
            height: rectangle.height,
        });
    }
    let count = (rectangle.width as usize)
        .checked_mul(rectangle.height as usize)
        .and_then(|value| value.checked_mul(2))
        .ok_or(TimError::InvalidRectangle {
            at,
            width: rectangle.width,
            height: rectangle.height,
        })?;
    let pixels = span(encoded, 12, count)?;
    *cursor = at + size;
    Ok(ImageBlock {
        rectangle,
        rectangle_offset: at + 4,
        data_offset: at + 12,
        pixels,
        payload: &encoded[12..],
    })
}
