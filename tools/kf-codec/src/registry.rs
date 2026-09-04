//! PSX-visible asset registration and in-place packet preparation.
//!
//! Addresses are explicit 32-bit values, never host pointers. Caller-owned
//! table bytes retain all slots not touched by the retail registration loop.

use crate::asset_archive::{Archive, AssetArchiveError, AssetHeader};
use crate::tmd::{self, TmdError};

#[derive(Debug)]
pub enum RegistryError {
    Archive(AssetArchiveError),
    Tmd(TmdError),
    InvalidAssetHeader,
    TmdOffsetOutOfBounds,
    TableTooSmall { slot: u16 },
}

fn table_slot(table: &mut [u8], slot: u16) -> Result<&mut [u8], RegistryError> {
    let at = usize::from(slot) * 4;
    table
        .get_mut(at..at + 4)
        .ok_or(RegistryError::TableTooSmall { slot })
}

pub fn register_asset(
    asset: &mut [u8],
    slot: u16,
    address: u32,
    table: &mut [u8],
    selected_tmd: &mut u32,
) -> Result<(), RegistryError> {
    let header = AssetHeader::parse(asset).ok_or(RegistryError::InvalidAssetHeader)?;
    let tmd = asset
        .get_mut(header.tmd_data_offset as usize..)
        .ok_or(RegistryError::TmdOffsetOutOfBounds)?;
    table_slot(table, slot)?.copy_from_slice(&address.to_le_bytes());
    *selected_tmd = address.wrapping_add(header.tmd_data_offset);
    tmd::prepare_primitive_indices(tmd).map_err(RegistryError::Tmd)?;
    Ok(())
}

pub fn register_archive(
    archive: &mut [u8],
    first_slot: u16,
    address: u32,
    table: &mut [u8],
    selected_tmd: &mut u32,
) -> Result<(), RegistryError> {
    let parsed = Archive::parse(archive).map_err(RegistryError::Archive)?;
    parsed.validate().map_err(RegistryError::Archive)?;
    let count = parsed.count();
    for index in 0..count {
        table_slot(table, first_slot.wrapping_add(index))?;
    }
    let mut at = 4usize;
    for index in 0..count {
        let size = AssetHeader::parse(&archive[at..]).unwrap().byte_size as usize;
        register_asset(
            &mut archive[at..at + size],
            first_slot.wrapping_add(index),
            address.wrapping_add(at as u32),
            table,
            selected_tmd,
        )?;
        at += size;
    }
    Ok(())
}

pub fn register_tmd(
    tmd: &mut [u8],
    slot: u16,
    address: u32,
    slots: &mut [u8],
    selected_tmd: &mut u32,
) -> Result<(), RegistryError> {
    table_slot(slots, slot)?.copy_from_slice(&address.to_le_bytes());
    *selected_tmd = address;
    tmd::prepare_primitive_indices(tmd).map_err(RegistryError::Tmd)?;
    Ok(())
}
