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
const BLOCK_RECTANGLE_X_OFFSET: usize = 4;
const BLOCK_RECTANGLE_Y_OFFSET: usize = 6;
const BLOCK_RECTANGLE_WIDTH_OFFSET: usize = 8;
const BLOCK_RECTANGLE_HEIGHT_OFFSET: usize = 10;


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

    pub pixels: &'a [u8],

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
        if self.bytes.len() - self.at < 4 || u32_at(self.bytes, self.at)? != TIM_MAGIC {
            self.stopped = true;
            return Ok(None);
        }
        let start = self.at;
        let mode = u32_at(self.bytes, start + TIM_MODE_OFFSET)?;
        let mut cursor = start + TIM_HEADER_BYTES;
        let clut = if mode & TIM_CLUT_FLAG != 0 {
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
        usize::try_from(declared & !BLOCK_SIZE_LOW_BITS).map_err(|_| TimError::InvalidBlockSize { at, declared })?;
    if size < BLOCK_HEADER_BYTES {
        return Err(TimError::InvalidBlockSize { at, declared });
    }
    let encoded = span(bytes, at, size)?;
    let half = |offset| i16::from_le_bytes([encoded[offset], encoded[offset + 1]]);
    let rectangle = Rect {
        x: half(BLOCK_RECTANGLE_X_OFFSET),
        y: half(BLOCK_RECTANGLE_Y_OFFSET),
        width: half(BLOCK_RECTANGLE_WIDTH_OFFSET),
        height: half(BLOCK_RECTANGLE_HEIGHT_OFFSET),
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
    Ok(ImageBlock {
        rectangle,
        rectangle_offset: at + BLOCK_RECTANGLE_X_OFFSET,
        data_offset: at + BLOCK_HEADER_BYTES,
        pixels,
        payload: &encoded[BLOCK_HEADER_BYTES..],
    })
}
