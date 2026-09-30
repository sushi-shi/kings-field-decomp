use crate::bytes::{self, read_u32_le, span, ReadError};
use crate::cast::AsUsize;
use core::fmt;

pub const TIM_MAGIC: u32 = 0x10;
pub const TIM_HEADER_BYTES: usize = 8;
pub const TIM_MODE_OFFSET: usize = 4;
pub const TIM_FORMAT_MASK: u32 = 7;
pub const TIM_CLUT_FLAG: u32 = 8;
pub const TIM_FLAGS_MASK: u32 = 15;
pub const TIM_INDEXED4: u32 = 0;
pub const TIM_INDEXED8: u32 = 1;
pub const TIM_DIRECT16: u32 = 2;
pub const TIM_DIRECT24: u32 = 3;
const BLOCK_SIZE_LOW_BITS: u32 = 3;
const BLOCK_HEADER_BYTES: usize = 12;
const BLOCK_RECTANGLE_OFFSET: usize = 4;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TimError {
    Truncated {
        location: &'static core::panic::Location<'static>,
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

impl From<ReadError> for TimError {
    fn from(error: ReadError) -> Self {
        Self::Truncated {
            location: error.location,
            at: error.at,
            need: error.need,
            available: error.available,
        }
    }
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

#[derive(Debug, Clone, Copy)]
pub struct ImageBlock<'a> {
    pub rectangle: Rect,

    pub pixels: &'a [u8],
}

#[derive(Debug, Clone, Copy)]
pub struct Image<'a> {
    pub mode: u32,
    pub clut: Option<ImageBlock<'a>>,
    pub image: ImageBlock<'a>,
    pub encoded_len: usize,
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

    fn parse_next(&mut self) -> Result<Option<Image<'a>>, TimError> {
        if self.bytes.len() - self.at < 4 || read_u32_le(self.bytes, self.at)? != TIM_MAGIC {
            self.stopped = true;
            return Ok(None);
        }
        let start = self.at;
        let mode = read_u32_le(self.bytes, start + TIM_MODE_OFFSET)?;
        let mut cursor = start + TIM_HEADER_BYTES;
        let clut = if mode & TIM_CLUT_FLAG != 0 {
            Some(block(self.bytes, &mut cursor)?)
        } else {
            None
        };
        let image = block(self.bytes, &mut cursor)?;
        self.at = cursor;
        Ok(Some(Image {
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

fn block<'a>(bytes: &'a [u8], cursor: &mut usize) -> Result<ImageBlock<'a>, TimError> {
    let at = *cursor;
    let declared = read_u32_le(bytes, at)?;
    let size = (declared & !BLOCK_SIZE_LOW_BITS).as_usize();
    if size < BLOCK_HEADER_BYTES {
        return Err(TimError::InvalidBlockSize { at, declared });
    }
    let encoded = span(bytes, at, size)?;
    let record = bytes::Record::<8>::new(&encoded[BLOCK_RECTANGLE_OFFSET..])?;
    let rectangle = Rect {
        x: record.i16_le::<0>(),
        y: record.i16_le::<2>(),
        width: record.i16_le::<4>(),
        height: record.i16_le::<6>(),
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
    let pixels = span(encoded, BLOCK_HEADER_BYTES, count)?;
    *cursor = at + size;
    Ok(ImageBlock { rectangle, pixels })
}
