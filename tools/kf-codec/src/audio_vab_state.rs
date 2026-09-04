//! Observable state transformation performed while GAME opens a Sony VAB.
//!
//! GAME's `audio_load_vab` wrapper delegates the VH parser and SPU transfer
//! setup to the Release 2.5 sound library.  This module is an independent,
//! allocation-free model of the successful path.  Callers supply the Sony
//! work areas explicitly, so tests can compare every mutated byte without
//! pretending those private SDK tables belong to the on-disc VAB format.

use core::fmt;

pub const VAB_HEADER_SIZE: usize = 32;
pub const VAB_PROGRAM_SIZE: usize = 16;
pub const VAB_NEW_PROGRAM_SLOTS: usize = 128;
pub const VAB_OLD_PROGRAM_SLOTS: usize = 64;
pub const VAB_TONES_PER_PROGRAM: usize = 16;
pub const VAB_TONE_SIZE: usize = 32;
pub const VAB_LENGTH_ENTRIES: usize = 256;
pub const VAB_LENGTH_TABLE_SIZE: usize = VAB_LENGTH_ENTRIES * 2;
pub const VAB_BANK_SLOTS: usize = 16;
pub const VAB_POINTER_TABLE_SIZE: usize = VAB_BANK_SLOTS * 4;
pub const GAME_AUDIO_STATE_SIZE: usize = 0x90;
pub const GAME_AUDIO_VH_POINTER_OFFSET: usize = 0;
pub const GAME_AUDIO_VAB_ID_OFFSET: usize = 4;
pub const GAME_AUDIO_SEQUENCE_ACTIVE_OFFSET: usize = 0x10;

const VAB_MAGIC_NEW: u32 = u32::from_le_bytes(*b"pBAV");
const VAB_MAGIC_OLD: u32 = u32::from_le_bytes(*b" BAV");
const SPU_RAM_LIMIT: u32 = 0x80000;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum VabStateError {
    RegionSize {
        region: &'static str,
        expected: usize,
        actual: usize,
    },
    TruncatedHeader {
        need: usize,
        have: usize,
    },
    InvalidMagic(u32),
    InvalidProgramCount {
        count: u16,
        maximum: u16,
    },
    HeaderExtent {
        need: usize,
        have: usize,
    },
    NoFreeBank,
    TransferBusy,
    AllocationFailed,
    SpuAddressOverflow,
    BodyLength {
        expected: usize,
        actual: usize,
    },
    ActiveSequence,
}

impl fmt::Display for VabStateError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "VAB runtime state {self:?}")
    }
}

impl core::error::Error for VabStateError {}

/// Mutable views of the private Sony state touched by `SsVabOpenHead`.
///
/// Each slice corresponds to one exact GAME memory object.  In particular,
/// `vh_end_pointers` receives the first address after the 256-entry length
/// table; it is not a pointer to the beginning of that table.
pub struct SonyVabRegions<'a> {
    pub maximum_programs: &'a mut [u8],
    pub open_bank_count: &'a mut [u8],
    pub bank_status: &'a mut [u8],
    pub vh_end_pointers: &'a mut [u8],
    pub header_pointers: &'a mut [u8],
    pub program_pointers: &'a mut [u8],
    pub tone_pointers: &'a mut [u8],
    pub spu_start_addresses: &'a mut [u8],
    pub body_sizes: &'a mut [u8],
    pub current_header_pointer: &'a mut [u8],
    pub current_program_pointer: &'a mut [u8],
    pub current_tone_pointer: &'a mut [u8],
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct VabLoadInputs {
    pub vh_address: u32,
    pub vb_address: u32,
    pub spu_allocation: u32,
    pub in_transfer: i32,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct VabLoadReport {
    pub bank_id: i16,
    pub maximum_programs: u16,
    pub dense_program_count: u16,
    pub tone_table_address: u32,
    pub length_table_address: u32,
    pub vh_end_address: u32,
    pub body_size: u32,
}

/// One deterministic external-service request made by the successful path.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum VabServiceCall {
    GetInTransfer,
    SetInTransfer(i32),
    Malloc(u32),
    SetTransferMode(i32),
    SetTransferStartAddress(u32),
    Read { source: u32, size: u32 },
    IsTransferCompleted(i32),
}

pub const SUCCESS_CALL_COUNT: usize = 7;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct VabLoadOutput {
    pub report: VabLoadReport,
    pub calls: [VabServiceCall; SUCCESS_CALL_COUNT],
}

/// Apply the complete successful `audio_load_vab`/Sony state transform.
///
/// The supplied VB bytes are not read by the header parser.  Their extent is
/// checked against the sum of all 256 VH length entries because the following
/// transfer consumes exactly that many bytes.
pub fn load_vab_success(
    vh: &mut [u8],
    vb: &[u8],
    game_audio_state: &mut [u8],
    regions: SonyVabRegions<'_>,
    inputs: VabLoadInputs,
) -> Result<VabLoadOutput, VabStateError> {
    validate_region("GAME audio state", game_audio_state, GAME_AUDIO_STATE_SIZE)?;
    validate_region("maximum programs", regions.maximum_programs, 2)?;
    validate_region("open bank count", regions.open_bank_count, 2)?;
    validate_region("bank status", regions.bank_status, VAB_BANK_SLOTS)?;
    validate_region(
        "VH-end pointers",
        regions.vh_end_pointers,
        VAB_POINTER_TABLE_SIZE,
    )?;
    validate_region(
        "header pointers",
        regions.header_pointers,
        VAB_POINTER_TABLE_SIZE,
    )?;
    validate_region(
        "program pointers",
        regions.program_pointers,
        VAB_POINTER_TABLE_SIZE,
    )?;
    validate_region(
        "tone pointers",
        regions.tone_pointers,
        VAB_POINTER_TABLE_SIZE,
    )?;
    validate_region(
        "SPU starts",
        regions.spu_start_addresses,
        VAB_POINTER_TABLE_SIZE,
    )?;
    validate_region("body sizes", regions.body_sizes, VAB_POINTER_TABLE_SIZE)?;
    validate_region("current header pointer", regions.current_header_pointer, 4)?;
    validate_region(
        "current program pointer",
        regions.current_program_pointer,
        4,
    )?;
    validate_region("current tone pointer", regions.current_tone_pointer, 4)?;
    if vh.len() < VAB_HEADER_SIZE {
        return Err(VabStateError::TruncatedHeader {
            need: VAB_HEADER_SIZE,
            have: vh.len(),
        });
    }
    if read_u32(game_audio_state, GAME_AUDIO_SEQUENCE_ACTIVE_OFFSET) == 1 {
        return Err(VabStateError::ActiveSequence);
    }
    if inputs.in_transfer == 1 {
        return Err(VabStateError::TransferBusy);
    }
    let bank_id = regions
        .bank_status
        .iter()
        .position(|&status| status == 0)
        .ok_or(VabStateError::NoFreeBank)?;

    let magic = read_u32(vh, 0);
    let version = read_u32(vh, 4);
    let maximum_programs = match magic {
        VAB_MAGIC_OLD => VAB_OLD_PROGRAM_SLOTS,
        VAB_MAGIC_NEW if (version as i32) < 5 => VAB_OLD_PROGRAM_SLOTS,
        VAB_MAGIC_NEW => VAB_NEW_PROGRAM_SLOTS,
        other => return Err(VabStateError::InvalidMagic(other)),
    };
    let program_count = read_u16(vh, 18);
    if usize::from(program_count) > maximum_programs {
        return Err(VabStateError::InvalidProgramCount {
            count: program_count,
            maximum: maximum_programs as u16,
        });
    }
    // Release 2.5 reads the low byte even though the public VabHdr spells a
    // 16-bit field.  Keep that exact parser-visible width here.
    let sample_count = usize::from(vh[22]);

    let program_at = VAB_HEADER_SIZE;
    let tone_at = checked_add(program_at, maximum_programs * VAB_PROGRAM_SIZE)?;
    let tone_bytes = usize::from(program_count)
        .checked_mul(VAB_TONES_PER_PROGRAM * VAB_TONE_SIZE)
        .ok_or(VabStateError::SpuAddressOverflow)?;
    let lengths_at = checked_add(tone_at, tone_bytes)?;
    let vh_end = checked_add(lengths_at, VAB_LENGTH_TABLE_SIZE)?;
    if vh.len() < vh_end {
        return Err(VabStateError::HeaderExtent {
            need: vh_end,
            have: vh.len(),
        });
    }

    let multiplier = if (version as i32) < 5 { 4u32 } else { 8u32 };
    let mut lengths = [0u32; VAB_LENGTH_ENTRIES];
    let mut body_size = 0u32;
    for (index, length) in lengths.iter_mut().enumerate() {
        let units = u32::from(read_u16(vh, lengths_at + index * 2));
        *length = units
            .checked_mul(multiplier)
            .ok_or(VabStateError::SpuAddressOverflow)?;
        body_size = body_size
            .checked_add(*length)
            .ok_or(VabStateError::SpuAddressOverflow)?;
    }
    if vb.len() != body_size as usize {
        return Err(VabStateError::BodyLength {
            expected: body_size as usize,
            actual: vb.len(),
        });
    }
    if inputs.spu_allocation == u32::MAX {
        return Err(VabStateError::AllocationFailed);
    }
    let spu_end = inputs
        .spu_allocation
        .checked_add(body_size)
        .ok_or(VabStateError::SpuAddressOverflow)?;
    if spu_end > SPU_RAM_LIMIT {
        return Err(VabStateError::SpuAddressOverflow);
    }

    let program_address = checked_address(inputs.vh_address, program_at)?;
    let tone_address = checked_address(inputs.vh_address, tone_at)?;
    let length_address = checked_address(inputs.vh_address, lengths_at)?;
    let vh_end_address = checked_address(inputs.vh_address, vh_end)?;

    // Sony first marks the selected slot as reserved, then as header-open.
    // Only the final byte is observable after the complete GAME wrapper.
    regions.bank_status[bank_id] = 1;
    let open_count = read_u16(regions.open_bank_count, 0).wrapping_add(1);
    write_u16(regions.open_bank_count, 0, open_count);
    write_u16(regions.maximum_programs, 0, maximum_programs as u16);

    write_table(regions.header_pointers, bank_id, inputs.vh_address);
    write_u32(regions.current_header_pointer, 0, inputs.vh_address);
    write_table(regions.program_pointers, bank_id, program_address);
    write_u32(regions.current_program_pointer, 0, program_address);

    let mut dense_program_count = 0u16;
    for slot in 0..maximum_programs {
        let at = program_at + slot * VAB_PROGRAM_SIZE;
        write_u32(vh, at + 8, u32::from(dense_program_count));
        if vh[at] != 0 {
            dense_program_count = dense_program_count.wrapping_add(1);
        }
    }

    write_table(regions.tone_pointers, bank_id, tone_address);
    write_u32(regions.current_tone_pointer, 0, tone_address);
    write_table(regions.vh_end_pointers, bank_id, vh_end_address);
    write_table(regions.spu_start_addresses, bank_id, inputs.spu_allocation);

    let mut cumulative = 0u32;
    for index in 0..=sample_count {
        cumulative = cumulative
            .checked_add(lengths[index])
            .ok_or(VabStateError::SpuAddressOverflow)?;
        let address = inputs
            .spu_allocation
            .checked_add(cumulative)
            .ok_or(VabStateError::SpuAddressOverflow)?;
        let slot = index / 2;
        let half = index & 1;
        write_u16(
            vh,
            program_at + slot * VAB_PROGRAM_SIZE + 12 + half * 2,
            (address >> 3) as u16,
        );
    }

    write_table(regions.body_sizes, bank_id, body_size);
    // OpenHead leaves status 2; TransBody changes it to 1 after SpuRead.
    regions.bank_status[bank_id] = 1;
    write_u32(
        game_audio_state,
        GAME_AUDIO_VH_POINTER_OFFSET,
        inputs.vh_address,
    );
    write_u16(game_audio_state, GAME_AUDIO_VAB_ID_OFFSET, bank_id as u16);

    let report = VabLoadReport {
        bank_id: bank_id as i16,
        maximum_programs: maximum_programs as u16,
        dense_program_count,
        tone_table_address: tone_address,
        length_table_address: length_address,
        vh_end_address,
        body_size,
    };
    Ok(VabLoadOutput {
        report,
        calls: [
            VabServiceCall::GetInTransfer,
            VabServiceCall::SetInTransfer(1),
            VabServiceCall::Malloc(body_size),
            VabServiceCall::SetTransferMode(0),
            VabServiceCall::SetTransferStartAddress(inputs.spu_allocation),
            VabServiceCall::Read {
                source: inputs.vb_address,
                size: body_size,
            },
            VabServiceCall::IsTransferCompleted(1),
        ],
    })
}

fn validate_region(
    region: &'static str,
    bytes: &[u8],
    expected: usize,
) -> Result<(), VabStateError> {
    if bytes.len() != expected {
        return Err(VabStateError::RegionSize {
            region,
            expected,
            actual: bytes.len(),
        });
    }
    Ok(())
}

fn checked_add(left: usize, right: usize) -> Result<usize, VabStateError> {
    left.checked_add(right)
        .ok_or(VabStateError::SpuAddressOverflow)
}

fn checked_address(base: u32, offset: usize) -> Result<u32, VabStateError> {
    let offset = u32::try_from(offset).map_err(|_| VabStateError::SpuAddressOverflow)?;
    base.checked_add(offset)
        .ok_or(VabStateError::SpuAddressOverflow)
}

fn read_u16(bytes: &[u8], at: usize) -> u16 {
    u16::from_le_bytes(bytes[at..at + 2].try_into().unwrap())
}

fn read_u32(bytes: &[u8], at: usize) -> u32 {
    u32::from_le_bytes(bytes[at..at + 4].try_into().unwrap())
}

fn write_u16(bytes: &mut [u8], at: usize, value: u16) {
    bytes[at..at + 2].copy_from_slice(&value.to_le_bytes());
}

fn write_u32(bytes: &mut [u8], at: usize, value: u32) {
    bytes[at..at + 4].copy_from_slice(&value.to_le_bytes());
}

fn write_table(bytes: &mut [u8], index: usize, value: u32) {
    write_u32(bytes, index * 4, value);
}
