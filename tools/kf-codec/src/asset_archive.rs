//! Model-asset archives consumed by `asset_registry_load_tmd_archive`
//! @`0x800204c0` in `GAME.EXE`.
//!
//! The archive starts with a little-endian `u16` entry count and two bytes not
//! interpreted by retail. At offset four are consecutive assets. Each starts
//! with a 20-byte [`AssetHeader`], whose `byte_size` advances to the next asset.

use core::fmt;

use crate::Sink;

pub const ARCHIVE_HEADER_SIZE: usize = 4;
pub const ASSET_HEADER_SIZE: usize = 20;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AssetArchiveError {
    TruncatedArchiveHeader {
        available: usize,
    },
    TruncatedAssetHeader {
        index: u16,
        at: usize,
        available: usize,
    },
    InvalidAssetSize {
        index: u16,
        at: usize,
        declared: u32,
    },
    AssetSizeOverflow {
        index: u16,
        at: usize,
        declared: u32,
    },
    TruncatedAsset {
        index: u16,
        at: usize,
        declared: u32,
        available: usize,
    },
    TooManyAssets {
        count: usize,
    },
    EncodedSizeMismatch {
        index: usize,
        declared: u32,
        actual: usize,
    },
    OutputFull {
        need: usize,
        have: usize,
    },
}

impl fmt::Display for AssetArchiveError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match *self {
            Self::TruncatedArchiveHeader { available } => {
                write!(f, "asset archive header has {available} bytes")
            }
            Self::TruncatedAssetHeader {
                index,
                at,
                available,
            } => write!(f, "asset {index} header at byte {at} has {available} bytes"),
            Self::InvalidAssetSize {
                index,
                at,
                declared,
            } => write!(
                f,
                "asset {index} at byte {at} declares invalid size {declared}"
            ),
            Self::AssetSizeOverflow {
                index,
                at,
                declared,
            } => write!(
                f,
                "asset {index} at byte {at} has unrepresentable size {declared}"
            ),
            Self::TruncatedAsset {
                index,
                at,
                declared,
                available,
            } => write!(
                f,
                "asset {index} at byte {at} declares {declared} bytes, {available} available"
            ),
            Self::TooManyAssets { count } => {
                write!(f, "asset count {count} is not representable")
            }
            Self::EncodedSizeMismatch {
                index,
                declared,
                actual,
            } => write!(
                f,
                "asset {index} declares {declared} bytes but contains {actual}"
            ),
            Self::OutputFull { need, have } => {
                write!(f, "output buffer holds {have} bytes, need {need}")
            }
        }
    }
}

impl core::error::Error for AssetArchiveError {}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct AssetHeader {
    pub byte_size: u32,
    pub animation_clip_count: i32,
    pub tmd_data_offset: u32,
    pub object_table_offset: u32,
    pub clip_table_offset: u32,
}

impl AssetHeader {
    pub fn parse(bytes: &[u8]) -> Option<Self> {
        let bytes = bytes.get(..ASSET_HEADER_SIZE)?;
        Some(Self {
            byte_size: read_u32(bytes, 0),
            animation_clip_count: read_u32(bytes, 4) as i32,
            tmd_data_offset: read_u32(bytes, 8),
            object_table_offset: read_u32(bytes, 12),
            clip_table_offset: read_u32(bytes, 16),
        })
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Asset<'a> {
    pub index: u16,
    pub offset: usize,
    pub header: AssetHeader,
    pub bytes: &'a [u8],
}

impl Asset<'_> {
    pub fn tmd_data(&self) -> Option<&[u8]> {
        self.bytes
            .get(usize::try_from(self.header.tmd_data_offset).ok()?..)
    }
}

#[derive(Debug, Clone, Copy)]
pub struct Archive<'a> {
    bytes: &'a [u8],
    count: u16,
    reserved: [u8; 2],
}

impl<'a> Archive<'a> {
    pub fn parse(bytes: &'a [u8]) -> Result<Self, AssetArchiveError> {
        let Some(header) = bytes.get(..ARCHIVE_HEADER_SIZE) else {
            return Err(AssetArchiveError::TruncatedArchiveHeader {
                available: bytes.len(),
            });
        };
        Ok(Self {
            bytes,
            count: u16::from_le_bytes([header[0], header[1]]),
            reserved: [header[2], header[3]],
        })
    }

    pub const fn count(&self) -> u16 {
        self.count
    }

    pub const fn reserved(&self) -> [u8; 2] {
        self.reserved
    }

    pub const fn assets(&self) -> Assets<'a> {
        Assets {
            bytes: self.bytes,
            count: self.count,
            index: 0,
            at: ARCHIVE_HEADER_SIZE,
            done: false,
        }
    }

    /// Walk every declared entry and return the first byte after the archive.
    pub fn validate(&self) -> Result<usize, AssetArchiveError> {
        let mut assets = self.assets();
        for asset in &mut assets {
            asset?;
        }
        Ok(assets.position())
    }

    /// Bytes after all declared assets, if every entry is structurally valid.
    pub fn remainder(&self) -> Result<&'a [u8], AssetArchiveError> {
        Ok(&self.bytes[self.validate()?..])
    }
}

pub struct Assets<'a> {
    bytes: &'a [u8],
    count: u16,
    index: u16,
    at: usize,
    done: bool,
}

impl<'a> Iterator for Assets<'a> {
    type Item = Result<Asset<'a>, AssetArchiveError>;

    fn next(&mut self) -> Option<Self::Item> {
        if self.done || self.index == self.count {
            return None;
        }
        let available = self.bytes.len() - self.at;
        let Some(header_bytes) = self.bytes.get(self.at..self.at + ASSET_HEADER_SIZE) else {
            self.done = true;
            return Some(Err(AssetArchiveError::TruncatedAssetHeader {
                index: self.index,
                at: self.at,
                available,
            }));
        };
        let header = AssetHeader::parse(header_bytes).expect("sliced to the header size");
        if header.byte_size < ASSET_HEADER_SIZE as u32 {
            self.done = true;
            return Some(Err(AssetArchiveError::InvalidAssetSize {
                index: self.index,
                at: self.at,
                declared: header.byte_size,
            }));
        }
        let Ok(byte_size) = usize::try_from(header.byte_size) else {
            self.done = true;
            return Some(Err(AssetArchiveError::AssetSizeOverflow {
                index: self.index,
                at: self.at,
                declared: header.byte_size,
            }));
        };
        let Some(end) = self.at.checked_add(byte_size) else {
            self.done = true;
            return Some(Err(AssetArchiveError::AssetSizeOverflow {
                index: self.index,
                at: self.at,
                declared: header.byte_size,
            }));
        };
        let Some(bytes) = self.bytes.get(self.at..end) else {
            self.done = true;
            return Some(Err(AssetArchiveError::TruncatedAsset {
                index: self.index,
                at: self.at,
                declared: header.byte_size,
                available,
            }));
        };
        let asset = Asset {
            index: self.index,
            offset: self.at,
            header,
            bytes,
        };
        self.index += 1;
        self.at = end;
        Some(Ok(asset))
    }
}

impl<'a> Assets<'a> {
    pub const fn position(&self) -> usize {
        self.at
    }

    pub fn remainder(&self) -> &'a [u8] {
        &self.bytes[self.at..]
    }
}

fn encode(
    assets: &[&[u8]],
    reserved: [u8; 2],
    sink: &mut Sink<'_>,
) -> Result<(), AssetArchiveError> {
    let count = u16::try_from(assets.len()).map_err(|_| AssetArchiveError::TooManyAssets {
        count: assets.len(),
    })?;
    if !sink.extend(&count.to_le_bytes()) || !sink.extend(&reserved) {
        return Err(AssetArchiveError::OutputFull {
            need: 0,
            have: sink.len(),
        });
    }
    for (index, asset) in assets.iter().enumerate() {
        let Some(header) = AssetHeader::parse(asset) else {
            return Err(AssetArchiveError::TruncatedAssetHeader {
                index: u16::try_from(index).unwrap_or(u16::MAX),
                at: sink.len(),
                available: asset.len(),
            });
        };
        if usize::try_from(header.byte_size).ok() != Some(asset.len()) {
            return Err(AssetArchiveError::EncodedSizeMismatch {
                index,
                declared: header.byte_size,
                actual: asset.len(),
            });
        }
        if !sink.extend(asset) {
            return Err(AssetArchiveError::OutputFull {
                need: 0,
                have: sink.len(),
            });
        }
    }
    Ok(())
}

pub fn encoded_len(assets: &[&[u8]]) -> Result<usize, AssetArchiveError> {
    let mut sink = Sink::Count(0);
    encode(assets, [0, 0], &mut sink)?;
    Ok(sink.len())
}

pub fn encode_into(
    assets: &[&[u8]],
    reserved: [u8; 2],
    output: &mut [u8],
) -> Result<usize, AssetArchiveError> {
    let have = output.len();
    let mut sink = Sink::Write {
        bytes: output,
        at: 0,
    };
    match encode(assets, reserved, &mut sink) {
        Ok(()) => Ok(sink.len()),
        Err(AssetArchiveError::OutputFull { .. }) => Err(AssetArchiveError::OutputFull {
            need: encoded_len(assets)?,
            have,
        }),
        Err(error) => Err(error),
    }
}

fn read_u32(bytes: &[u8], at: usize) -> u32 {
    u32::from_le_bytes([bytes[at], bytes[at + 1], bytes[at + 2], bytes[at + 3]])
}
