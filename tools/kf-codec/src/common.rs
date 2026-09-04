//! Complete COM.DAT transformation performed by `common_resources_load`.

use crate::game_data::{CommonData, GameDataError};
use crate::records::{self, TransformError};
use crate::registry::{self, RegistryError};

pub struct Destinations<'a> {
    pub render_windows: &'a mut [u8],
    pub weapons: &'a mut [u8],
    pub armor: &'a mut [u8],
    pub magic: &'a mut [u8],
    pub object_definitions: &'a mut [u8],
    pub growth: &'a mut [u8],
    pub registry: &'a mut [u8],
}

#[derive(Debug)]
pub enum CommonError {
    Data(GameDataError),
    Record(TransformError),
    Registry(RegistryError),
}

pub struct LoadResult {
    pub selected_tmd: u32,
    pub arena_cursor: u32,
}

pub fn load(
    data: &mut [u8],
    address: u32,
    destinations: Destinations<'_>,
) -> Result<LoadResult, CommonError> {
    let parsed = CommonData::parse(data).map_err(CommonError::Data)?;
    let asset_size = parsed.common_asset.len();
    records::load_render_cell_windows(parsed.render_cell_windows, destinations.render_windows)
        .map_err(CommonError::Record)?;
    records::load_weapon_records(parsed.weapon_records, destinations.weapons)
        .map_err(CommonError::Record)?;
    records::load_armor_records(parsed.armor_records_source, destinations.armor)
        .map_err(CommonError::Record)?;
    records::load_magic_records(parsed.magic_records, destinations.magic)
        .map_err(CommonError::Record)?;
    records::load_map_object_definitions(
        parsed.map_object_definitions_source,
        destinations.object_definitions,
    )
    .map_err(CommonError::Record)?;
    records::load_player_level_growth(parsed.player_level_growth, destinations.growth)
        .map_err(CommonError::Record)?;
    let mut selected = 0;
    registry::register_asset(
        &mut data[4..4 + asset_size],
        21,
        address.wrapping_add(4),
        destinations.registry,
        &mut selected,
    )
    .map_err(CommonError::Registry)?;
    Ok(LoadResult {
        selected_tmd: selected,
        arena_cursor: address.wrapping_add(asset_size as u32).wrapping_add(20),
    })
}
