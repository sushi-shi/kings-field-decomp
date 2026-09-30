use crate::bytes::{self, read_u32_le, span, ReadError};
use crate::cast::AsUsize;
use crate::formats::{TimBlock, TimHeader, TimRectangle};
use core::fmt;

pub const TIM_MAGIC: u32 = 0x10;
pub const TIM_FORMAT_MASK: u32 = 7;
pub const TIM_CLUT_FLAG: u32 = 8;
pub const TIM_FLAGS_MASK: u32 = 15;
pub const TIM_INDEXED4: u32 = 0;
pub const TIM_INDEXED8: u32 = 1;
pub const TIM_DIRECT16: u32 = 2;
pub const TIM_DIRECT24: u32 = 3;
const BLOCK_SIZE_LOW_BITS: u32 = 3;

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

#[derive(Debug, Clone, Copy)]
pub struct ImageBlock<'a> {
    pub rectangle: TimRectangle,

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
        let header: TimHeader = bytes::read(self.bytes, start)?;
        let mode = header.mode.get();
        let mut cursor = start + size_of::<TimHeader>();
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
    let header: TimBlock = bytes::read(bytes, at)?;
    let declared = header.encoded_bytes.get();
    let size = (declared & !BLOCK_SIZE_LOW_BITS).as_usize();
    if size < size_of::<TimBlock>() {
        return Err(TimError::InvalidBlockSize { at, declared });
    }
    let encoded = span(bytes, at, size)?;
    let rectangle = header.rectangle;
    if rectangle.width.get() < 0 || rectangle.height.get() < 0 {
        return Err(TimError::InvalidRectangle {
            at,
            width: rectangle.width.get(),
            height: rectangle.height.get(),
        });
    }
    let count = (rectangle.width.get() as usize)
        .checked_mul(rectangle.height.get() as usize)
        .and_then(|value| value.checked_mul(2))
        .ok_or(TimError::InvalidRectangle {
            at,
            width: rectangle.width.get(),
            height: rectangle.height.get(),
        })?;
    let pixels = span(encoded, size_of::<TimBlock>(), count)?;
    *cursor = at + size;
    Ok(ImageBlock { rectangle, pixels })
}
