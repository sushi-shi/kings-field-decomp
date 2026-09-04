//! Length-prefixed streams consumed by `common_resources_load` @`0x8001b180`
//! and `map_resources_load` @`0x8001b558` in `GAME.EXE`.
//!
//! COM.DAT, MIXA.DAT and MIXB.DAT are walked as consecutive chunks. Each chunk
//! is one little-endian `u32` payload length followed immediately by that many
//! payload bytes. The game advances from the current header by `4 + length`.

use core::fmt;

use crate::Sink;

pub const HEADER_SIZE: usize = 4;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ChunkError {
    TruncatedHeader {
        at: usize,
        available: usize,
    },
    LengthOverflow {
        at: usize,
        declared: u32,
    },
    TruncatedPayload {
        at: usize,
        declared: u32,
        available: usize,
    },
    TooManyChunks {
        count: usize,
    },
    OutputFull {
        need: usize,
        have: usize,
    },
}

impl fmt::Display for ChunkError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match *self {
            Self::TruncatedHeader { at, available } => {
                write!(f, "chunk header at byte {at} has {available} bytes")
            }
            Self::LengthOverflow { at, declared } => {
                write!(
                    f,
                    "chunk at byte {at} has unrepresentable length {declared}"
                )
            }
            Self::TruncatedPayload {
                at,
                declared,
                available,
            } => write!(
                f,
                "chunk at byte {at} declares {declared} payload bytes, {available} available"
            ),
            Self::TooManyChunks { count } => {
                write!(f, "chunk count {count} is not representable")
            }
            Self::OutputFull { need, have } => {
                write!(f, "output buffer holds {have} bytes, need {need}")
            }
        }
    }
}

impl core::error::Error for ChunkError {}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Chunk<'a> {
    pub header_offset: usize,
    pub payload: &'a [u8],
}

impl Chunk<'_> {
    pub fn encoded_len(&self) -> usize {
        HEADER_SIZE + self.payload.len()
    }
}

/// A cursor over a borrowed chunk stream.
///
/// Call [`Cursor::next_chunk`] only as many times as the enclosing file schema
/// requires. [`Cursor::remainder`] then exposes sector padding or a following
/// format without guessing that it is another chunk.
#[derive(Debug, Clone, Copy)]
pub struct Cursor<'a> {
    bytes: &'a [u8],
    at: usize,
}

impl<'a> Cursor<'a> {
    pub const fn new(bytes: &'a [u8]) -> Self {
        Self { bytes, at: 0 }
    }

    pub const fn position(&self) -> usize {
        self.at
    }

    pub fn remainder(&self) -> &'a [u8] {
        &self.bytes[self.at..]
    }

    pub fn next_chunk(&mut self) -> Result<Option<Chunk<'a>>, ChunkError> {
        if self.at == self.bytes.len() {
            return Ok(None);
        }

        let available = self.bytes.len() - self.at;
        let Some(header) = self.bytes.get(self.at..self.at + HEADER_SIZE) else {
            return Err(ChunkError::TruncatedHeader {
                at: self.at,
                available,
            });
        };
        let declared = u32::from_le_bytes([header[0], header[1], header[2], header[3]]);
        let payload_len = usize::try_from(declared).map_err(|_| ChunkError::LengthOverflow {
            at: self.at,
            declared,
        })?;
        let payload_at = self.at + HEADER_SIZE;
        let payload_end =
            payload_at
                .checked_add(payload_len)
                .ok_or(ChunkError::LengthOverflow {
                    at: self.at,
                    declared,
                })?;
        let Some(payload) = self.bytes.get(payload_at..payload_end) else {
            return Err(ChunkError::TruncatedPayload {
                at: self.at,
                declared,
                available: self.bytes.len() - payload_at,
            });
        };
        let chunk = Chunk {
            header_offset: self.at,
            payload,
        };
        self.at = payload_end;
        Ok(Some(chunk))
    }
}

pub struct Chunks<'a> {
    cursor: Cursor<'a>,
    done: bool,
}

pub const fn chunks(bytes: &[u8]) -> Chunks<'_> {
    Chunks {
        cursor: Cursor::new(bytes),
        done: false,
    }
}

impl<'a> Iterator for Chunks<'a> {
    type Item = Result<Chunk<'a>, ChunkError>;

    fn next(&mut self) -> Option<Self::Item> {
        if self.done {
            return None;
        }
        match self.cursor.next_chunk() {
            Ok(Some(chunk)) => Some(Ok(chunk)),
            Ok(None) => {
                self.done = true;
                None
            }
            Err(error) => {
                self.done = true;
                Some(Err(error))
            }
        }
    }
}

impl Chunks<'_> {
    pub const fn position(&self) -> usize {
        self.cursor.position()
    }
}

fn encode(chunks: &[&[u8]], sink: &mut Sink<'_>) -> Result<(), ChunkError> {
    for payload in chunks {
        let declared = u32::try_from(payload.len()).map_err(|_| ChunkError::TooManyChunks {
            count: payload.len(),
        })?;
        if !sink.extend(&declared.to_le_bytes()) || !sink.extend(payload) {
            return Err(ChunkError::OutputFull {
                need: 0,
                have: sink.len(),
            });
        }
    }
    Ok(())
}

pub fn encoded_len(chunks: &[&[u8]]) -> Result<usize, ChunkError> {
    let mut sink = Sink::Count(0);
    encode(chunks, &mut sink)?;
    Ok(sink.len())
}

pub fn encode_into(chunks: &[&[u8]], output: &mut [u8]) -> Result<usize, ChunkError> {
    let have = output.len();
    let mut sink = Sink::Write {
        bytes: output,
        at: 0,
    };
    match encode(chunks, &mut sink) {
        Ok(()) => Ok(sink.len()),
        Err(ChunkError::OutputFull { .. }) => Err(ChunkError::OutputFull {
            need: encoded_len(chunks)?,
            have,
        }),
        Err(error) => Err(error),
    }
}
