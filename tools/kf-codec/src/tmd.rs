//! In-place TMD primitive-index preparation.
//!
//! This is an independent transcription of `tmd_prepare_primitive_indices`
//! @`0x8001c2b0` in retail `GAME.EXE`. It walks the low 16-bit object and
//! primitive counts, advances packets by the `ilen` header byte, ignores the
//! ABE mode bit through mask `0xfd`, and converts every selected normal/vertex
//! index to an eight-byte table offset with a wrapping 16-bit left shift.

use core::fmt;

pub const HEADER_SIZE: usize = 12;
pub const OBJECT_SIZE: usize = 28;
pub const PACKET_HEADER_SIZE: usize = 4;
pub const MODE_MASK: u8 = 0xfd;

pub const F3: u8 = 0x20;
pub const FT3: u8 = 0x24;
pub const F4: u8 = 0x28;
pub const FT4: u8 = 0x2c;
pub const G3: u8 = 0x30;
pub const GT3: u8 = 0x34;
pub const G4: u8 = 0x38;
pub const GT4: u8 = 0x3c;

const F3_INDICES: &[usize] = &[4, 6, 8, 10];
const G3_INDICES: &[usize] = &[4, 6, 8, 10, 12, 14];
const FT3_INDICES: &[usize] = &[12, 14, 16, 18];
const GT3_INDICES: &[usize] = &[12, 14, 16, 18, 20, 22];
const F4_INDICES: &[usize] = &[4, 6, 8, 10, 12];
const G4_INDICES: &[usize] = &[4, 6, 8, 10, 12, 14, 16, 18];
const FT4_INDICES: &[usize] = &[16, 18, 20, 22, 24];
const GT4_INDICES: &[usize] = &[16, 18, 20, 22, 24, 26, 28, 30];

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TmdError {
    TruncatedHeader {
        available: usize,
    },
    ObjectTableOverflow {
        count: u16,
    },
    TruncatedObjectTable {
        count: u16,
        need: usize,
        available: usize,
    },
    PrimitiveOffsetOverflow {
        object: u16,
        relative: u32,
    },
    TruncatedPacketHeader {
        object: u16,
        primitive: u16,
        at: usize,
        available: usize,
    },
    PacketLengthOverflow {
        object: u16,
        primitive: u16,
        at: usize,
        ilen: u8,
    },
    TruncatedPacket {
        object: u16,
        primitive: u16,
        at: usize,
        body_len: usize,
        available: usize,
    },
    PacketBodyTooSmall {
        object: u16,
        primitive: u16,
        at: usize,
        mode: u8,
        need: usize,
        body_len: usize,
    },
}

impl fmt::Display for TmdError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match *self {
            Self::TruncatedHeader { available } => {
                write!(f, "TMD header has {available} bytes")
            }
            Self::ObjectTableOverflow { count } => {
                write!(f, "TMD object table for {count} objects overflows")
            }
            Self::TruncatedObjectTable {
                count,
                need,
                available,
            } => write!(
                f,
                "TMD declares {count} objects requiring {need} bytes, {available} available"
            ),
            Self::PrimitiveOffsetOverflow { object, relative } => write!(
                f,
                "TMD object {object} primitive offset {relative} overflows"
            ),
            Self::TruncatedPacketHeader {
                object,
                primitive,
                at,
                available,
            } => write!(
                f,
                "object {object} primitive {primitive} header at byte {at} has {available} bytes"
            ),
            Self::PacketLengthOverflow {
                object,
                primitive,
                at,
                ilen,
            } => write!(
                f,
                "object {object} primitive {primitive} at byte {at} ilen {ilen} overflows"
            ),
            Self::TruncatedPacket {
                object,
                primitive,
                at,
                body_len,
                available,
            } => write!(
                f,
                "object {object} primitive {primitive} at byte {at} needs {body_len} body bytes, {available} available"
            ),
            Self::PacketBodyTooSmall {
                object,
                primitive,
                at,
                mode,
                need,
                body_len,
            } => write!(
                f,
                "object {object} primitive {primitive} mode {mode:#04x} at byte {at} has {body_len} body bytes, need {need}"
            ),
        }
    }
}

impl core::error::Error for TmdError {}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct PrepareReport {
    pub object_count: u16,
    pub primitive_count: usize,
    pub recognized_primitive_count: usize,
    pub shifted_index_count: usize,
}

/// Convert supported primitive indices to eight-byte offsets in place.
///
/// Unknown packet modes are skipped exactly as retail skips them. Malformed
/// input returns an error rather than performing an out-of-bounds access. An
/// error may be reported after indices in earlier packets were already shifted.
pub fn prepare_primitive_indices(tmd: &mut [u8]) -> Result<PrepareReport, TmdError> {
    let Some(header) = tmd.get(..HEADER_SIZE) else {
        return Err(TmdError::TruncatedHeader {
            available: tmd.len(),
        });
    };
    let object_count = read_u16(header, 8);
    let object_bytes = usize::from(object_count).checked_mul(OBJECT_SIZE).ok_or(
        TmdError::ObjectTableOverflow {
            count: object_count,
        },
    )?;
    let object_table_end =
        HEADER_SIZE
            .checked_add(object_bytes)
            .ok_or(TmdError::ObjectTableOverflow {
                count: object_count,
            })?;
    if object_table_end > tmd.len() {
        return Err(TmdError::TruncatedObjectTable {
            count: object_count,
            need: object_table_end,
            available: tmd.len(),
        });
    }

    let mut primitive_total = 0usize;
    let mut recognized_total = 0usize;
    let mut shifted_total = 0usize;
    for object in 0..object_count {
        let object_at = HEADER_SIZE + usize::from(object) * OBJECT_SIZE;
        let primitive_relative = read_u32(tmd, object_at + 16);
        let primitive_count = read_u16(tmd, object_at + 20);
        let relative =
            usize::try_from(primitive_relative).map_err(|_| TmdError::PrimitiveOffsetOverflow {
                object,
                relative: primitive_relative,
            })?;
        let mut packet_at =
            HEADER_SIZE
                .checked_add(relative)
                .ok_or(TmdError::PrimitiveOffsetOverflow {
                    object,
                    relative: primitive_relative,
                })?;

        for primitive in 0..primitive_count {
            let available = tmd.len().saturating_sub(packet_at);
            let Some(packet) = tmd.get(packet_at..packet_at.saturating_add(PACKET_HEADER_SIZE))
            else {
                return Err(TmdError::TruncatedPacketHeader {
                    object,
                    primitive,
                    at: packet_at,
                    available,
                });
            };
            let body_len = usize::from(packet[1]) * 4;
            let body_at = packet_at.checked_add(PACKET_HEADER_SIZE).ok_or(
                TmdError::PacketLengthOverflow {
                    object,
                    primitive,
                    at: packet_at,
                    ilen: packet[1],
                },
            )?;
            let next_packet =
                body_at
                    .checked_add(body_len)
                    .ok_or(TmdError::PacketLengthOverflow {
                        object,
                        primitive,
                        at: packet_at,
                        ilen: packet[1],
                    })?;
            if next_packet > tmd.len() {
                return Err(TmdError::TruncatedPacket {
                    object,
                    primitive,
                    at: packet_at,
                    body_len,
                    available: tmd.len().saturating_sub(body_at),
                });
            }

            let mode = packet[3] & MODE_MASK;
            let indices = index_offsets(mode);
            if let Some(&last) = indices.last() {
                let need = last + 2;
                if need > body_len {
                    return Err(TmdError::PacketBodyTooSmall {
                        object,
                        primitive,
                        at: packet_at,
                        mode,
                        need,
                        body_len,
                    });
                }
                for &relative_index in indices {
                    let index_at = body_at + relative_index;
                    let value = read_u16(tmd, index_at).wrapping_shl(3);
                    write_u16(tmd, index_at, value);
                }
                recognized_total += 1;
                shifted_total += indices.len();
            }
            primitive_total += 1;
            packet_at = next_packet;
        }
    }

    Ok(PrepareReport {
        object_count,
        primitive_count: primitive_total,
        recognized_primitive_count: recognized_total,
        shifted_index_count: shifted_total,
    })
}

fn index_offsets(mode: u8) -> &'static [usize] {
    match mode {
        F3 => F3_INDICES,
        G3 => G3_INDICES,
        FT3 => FT3_INDICES,
        GT3 => GT3_INDICES,
        F4 => F4_INDICES,
        G4 => G4_INDICES,
        FT4 => FT4_INDICES,
        GT4 => GT4_INDICES,
        _ => &[],
    }
}

fn read_u16(bytes: &[u8], at: usize) -> u16 {
    u16::from_le_bytes([bytes[at], bytes[at + 1]])
}

fn read_u32(bytes: &[u8], at: usize) -> u32 {
    u32::from_le_bytes([bytes[at], bytes[at + 1], bytes[at + 2], bytes[at + 3]])
}

fn write_u16(bytes: &mut [u8], at: usize, value: u16) {
    bytes[at..at + 2].copy_from_slice(&value.to_le_bytes());
}
