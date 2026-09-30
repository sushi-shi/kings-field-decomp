use crate::bytes::{self, read_u32_le, span};
use crate::cast::AsUsize;
use crate::formats::{TimBlock, TimHeader};

const TIM_MAGIC: u32 = 0x10;
const FORMAT_MASK: u32 = 0b111;
const CLUT_FLAG: u32 = 1 << 3;
const FLAGS_MASK: u32 = FORMAT_MASK | CLUT_FLAG;
const BLOCK_SIZE_LOW_BITS: u32 = 3;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub(crate) enum PixelFormat {
    Indexed4,
    Indexed8,
    Direct16,
    Direct24,
}

impl PixelFormat {
    fn parse(mode: u32) -> crate::Result<Self> {
        if mode & !FLAGS_MASK != 0 {
            crate::bail!("unsupported TIM flags");
        }
        match mode & FORMAT_MASK {
            0 => Ok(Self::Indexed4),
            1 => Ok(Self::Indexed8),
            2 => Ok(Self::Direct16),
            3 => Ok(Self::Direct24),
            _ => crate::bail!("unsupported TIM pixel format"),
        }
    }
}

#[derive(Debug, Clone, Copy)]
pub struct ImageBlock<'a> {
    pub x: i16,
    pub y: i16,
    pub width: u16,
    pub height: u16,

    pub pixels: &'a [u8],
}

#[derive(Debug, Clone, Copy)]
pub struct Image<'a> {
    pub format: PixelFormat,
    pub clut: Option<ImageBlock<'a>>,
    pub image: ImageBlock<'a>,
    pub encoded_len: u32,
}

impl Image<'_> {
    pub(crate) fn encoded_mode(&self) -> u32 {
        let format = match self.format {
            PixelFormat::Indexed4 => 0,
            PixelFormat::Indexed8 => 1,
            PixelFormat::Direct16 => 2,
            PixelFormat::Direct24 => 3,
        };
        format | if self.clut.is_some() { CLUT_FLAG } else { 0 }
    }
}

pub(crate) struct Images<'a> {
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

    fn parse_next(&mut self) -> crate::Result<Option<Image<'a>>> {
        if self.bytes.len() - self.at < 4 || read_u32_le(self.bytes, self.at)? != TIM_MAGIC {
            self.stopped = true;
            return Ok(None);
        }
        let start = self.at;
        let header: TimHeader = bytes::read(self.bytes, start)?;
        let mode = header.mode.get();
        let format = PixelFormat::parse(mode)?;
        let mut cursor = start + size_of::<TimHeader>();
        let clut = if mode & CLUT_FLAG != 0 {
            Some(block(self.bytes, &mut cursor)?)
        } else {
            None
        };
        let image = block(self.bytes, &mut cursor)?;
        self.at = cursor;
        Ok(Some(Image {
            format,
            clut,
            image,
            encoded_len: u32::try_from(cursor - start)?,
        }))
    }
}

impl<'a> Iterator for Images<'a> {
    type Item = crate::Result<Image<'a>>;

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

fn block<'a>(bytes: &'a [u8], cursor: &mut usize) -> crate::Result<ImageBlock<'a>> {
    let at = *cursor;
    let header: TimBlock = bytes::read(bytes, at)?;
    let declared = header.encoded_bytes.get();
    let size = (declared & !BLOCK_SIZE_LOW_BITS).as_usize();
    if size < size_of::<TimBlock>() {
        return Err(crate::Error::invalid_record_size(at, declared));
    }
    let encoded = span(bytes, at, size)?;
    let rectangle = header.rectangle;
    let invalid =
        crate::Error::invalid_rectangle(at, rectangle.width.get(), rectangle.height.get());
    let width = u16::try_from(rectangle.width.get()).map_err(|_| invalid)?;
    let height = u16::try_from(rectangle.height.get()).map_err(|_| invalid)?;
    let count = width
        .as_usize()
        .checked_mul(height.as_usize())
        .and_then(|value| value.checked_mul(2))
        .ok_or(invalid)?;
    let pixels = span(encoded, size_of::<TimBlock>(), count)?;
    *cursor = at + size;
    Ok(ImageBlock {
        x: rectangle.x.get(),
        y: rectangle.y.get(),
        width,
        height,
        pixels,
    })
}
