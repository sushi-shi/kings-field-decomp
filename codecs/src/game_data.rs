use core::fmt;

use crate::asset_archive::{Archive, AssetArchiveError};
use crate::chunked::{Chunk, ChunkError, Cursor};

pub const MAP_GRID_SIDE: usize = 100;
pub const MAP_GRID_SIZE: usize = MAP_GRID_SIDE * MAP_GRID_SIDE;
pub const MAP_GRID_COUNT: usize = 5;
pub const MAP_GRIDS_SIZE: usize = MAP_GRID_SIZE * MAP_GRID_COUNT;
pub const ARMOR_RECORDS_SOURCE_SIZE: usize = 294 * 4;
pub const MAP_OBJECT_DEFINITIONS_SOURCE_SIZE: usize = 160 * 8;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum RuntimeSource {
    ArmorRecords,
    MapObjectDefinitions,
}

impl fmt::Display for RuntimeSource {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::ArmorRecords => f.write_str("armor records"),
            Self::MapObjectDefinitions => f.write_str("map-object definitions"),
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum GameDataError {
    Chunk {
        index: usize,
        source: ChunkError,
    },
    MissingChunk {
        index: usize,
    },
    MapGridsTooSmall {
        need: usize,
        have: usize,
    },
    AssetArchive {
        chunk_index: usize,
        source: AssetArchiveError,
    },
    RuntimeSourceTooSmall {
        source: RuntimeSource,
        need: usize,
        have: usize,
    },
}

impl fmt::Display for GameDataError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match *self {
            Self::Chunk { index, source } => {
                write!(f, "resource chunk {index}: {source}")
            }
            Self::MissingChunk { index } => {
                write!(f, "resource chunk {index} is missing")
            }
            Self::MapGridsTooSmall { need, have } => {
                write!(f, "map-grid chunk has {have} bytes, need {need}")
            }
            Self::AssetArchive {
                chunk_index,
                source,
            } => write!(f, "asset archive in chunk {chunk_index}: {source}"),
            Self::RuntimeSourceTooSmall { source, need, have } => {
                write!(f, "{source} source has {have} bytes, need {need}")
            }
        }
    }
}

impl core::error::Error for GameDataError {}

#[derive(Debug, Clone, Copy)]
pub struct CommonData<'a> {
    pub common_asset: &'a [u8],
    pub render_cell_windows: &'a [u8],
    pub weapon_records: &'a [u8],
    pub armor_records: &'a [u8],

    pub armor_records_source: &'a [u8],
    pub magic_records: &'a [u8],
    pub map_object_definitions: &'a [u8],

    pub map_object_definitions_source: &'a [u8],
    pub player_level_growth: &'a [u8],
    pub consumed: usize,
    pub remainder: &'a [u8],
}

impl<'a> CommonData<'a> {
    pub fn parse(bytes: &'a [u8]) -> Result<Self, GameDataError> {
        let mut cursor = Cursor::new(bytes);
        let common_asset = next_payload(&mut cursor, 0)?;
        let render_cell_windows = next_payload(&mut cursor, 1)?;
        let weapon_records = next_payload(&mut cursor, 2)?;
        let armor_chunk = next_chunk(&mut cursor, 3)?;
        let armor_records = armor_chunk.payload;
        let magic_records = next_payload(&mut cursor, 4)?;
        let map_object_chunk = next_chunk(&mut cursor, 5)?;
        let map_object_definitions = map_object_chunk.payload;
        let player_level_growth = next_payload(&mut cursor, 6)?;
        let armor_records_source = runtime_source(
            bytes,
            armor_chunk,
            RuntimeSource::ArmorRecords,
            ARMOR_RECORDS_SOURCE_SIZE,
        )?;
        let map_object_definitions_source = runtime_source(
            bytes,
            map_object_chunk,
            RuntimeSource::MapObjectDefinitions,
            MAP_OBJECT_DEFINITIONS_SOURCE_SIZE,
        )?;
        Ok(Self {
            common_asset,
            render_cell_windows,
            weapon_records,
            armor_records,
            armor_records_source,
            magic_records,
            map_object_definitions,
            map_object_definitions_source,
            player_level_growth,
            consumed: cursor.position(),
            remainder: cursor.remainder(),
        })
    }
}

#[derive(Debug, Clone, Copy)]
pub struct MapGrids<'a> {
    pub cell_attributes: &'a [u8],
    pub floor_heights: &'a [u8],
    pub cell_orientations: &'a [u8],
    pub collision_flags: &'a [u8],
    pub collision: &'a [u8],

    pub remainder: &'a [u8],
}

impl<'a> MapGrids<'a> {
    fn parse(bytes: &'a [u8]) -> Result<Self, GameDataError> {
        if bytes.len() < MAP_GRIDS_SIZE {
            return Err(GameDataError::MapGridsTooSmall {
                need: MAP_GRIDS_SIZE,
                have: bytes.len(),
            });
        }
        Ok(Self {
            cell_attributes: &bytes[0..MAP_GRID_SIZE],
            floor_heights: &bytes[MAP_GRID_SIZE..MAP_GRID_SIZE * 2],
            cell_orientations: &bytes[MAP_GRID_SIZE * 2..MAP_GRID_SIZE * 3],
            collision_flags: &bytes[MAP_GRID_SIZE * 3..MAP_GRID_SIZE * 4],
            collision: &bytes[MAP_GRID_SIZE * 4..MAP_GRID_SIZE * 5],
            remainder: &bytes[MAP_GRIDS_SIZE..],
        })
    }
}

#[derive(Debug, Clone, Copy)]
pub struct MixA<'a> {
    pub vab_header: &'a [u8],
    pub vab_body: &'a [u8],
    pub map_grids: MapGrids<'a>,
    pub floor_item_placements: &'a [u8],
    pub map_object_placements: &'a [u8],
    pub actor_placements: &'a [u8],
    pub actor_definitions: &'a [u8],
    pub map_event_definitions: &'a [u8],
    pub consumed: usize,
    pub remainder: &'a [u8],
}

impl<'a> MixA<'a> {
    pub fn parse(bytes: &'a [u8]) -> Result<Self, GameDataError> {
        let mut cursor = Cursor::new(bytes);
        let vab_header = next_payload(&mut cursor, 0)?;
        let vab_body = next_payload(&mut cursor, 1)?;
        let map_grids = MapGrids::parse(next_payload(&mut cursor, 2)?)?;
        let floor_item_placements = next_payload(&mut cursor, 3)?;
        let map_object_placements = next_payload(&mut cursor, 4)?;
        let actor_placements = next_payload(&mut cursor, 5)?;
        let actor_definitions = next_payload(&mut cursor, 6)?;
        let map_event_definitions = next_payload(&mut cursor, 7)?;
        Ok(Self {
            vab_header,
            vab_body,
            map_grids,
            floor_item_placements,
            map_object_placements,
            actor_placements,
            actor_definitions,
            map_event_definitions,
            consumed: cursor.position(),
            remainder: cursor.remainder(),
        })
    }
}

#[derive(Debug, Clone, Copy)]
pub struct MixB<'a> {
    pub primary_tmd: &'a [u8],
    pub secondary_tmd: &'a [u8],
    pub assets_from_id_10: Archive<'a>,
    pub assets_from_id_30: Archive<'a>,
    pub embedded_variant_assets: Option<Archive<'a>>,
    pub consumed: usize,
    pub remainder: &'a [u8],
}

impl<'a> MixB<'a> {
    pub fn parse(bytes: &'a [u8]) -> Result<Self, GameDataError> {
        let mut cursor = Cursor::new(bytes);
        let primary_tmd = next_payload(&mut cursor, 0)?;
        let secondary_tmd = next_payload(&mut cursor, 1)?;
        let assets_from_id_10 = parse_archive(next_payload(&mut cursor, 2)?, 2)?;
        let assets_from_id_30 = parse_archive(next_payload(&mut cursor, 3)?, 3)?;
        let embedded_variant_assets = match cursor.next_chunk() {
            Ok(Some(chunk)) => Some(parse_archive(chunk.payload, 4)?),
            Ok(None) => None,
            Err(source) => return Err(GameDataError::Chunk { index: 4, source }),
        };
        Ok(Self {
            primary_tmd,
            secondary_tmd,
            assets_from_id_10,
            assets_from_id_30,
            embedded_variant_assets,
            consumed: cursor.position(),
            remainder: cursor.remainder(),
        })
    }
}

fn next_payload<'a>(cursor: &mut Cursor<'a>, index: usize) -> Result<&'a [u8], GameDataError> {
    Ok(next_chunk(cursor, index)?.payload)
}

fn next_chunk<'a>(cursor: &mut Cursor<'a>, index: usize) -> Result<Chunk<'a>, GameDataError> {
    match cursor.next_chunk() {
        Ok(Some(chunk)) => Ok(chunk),
        Ok(None) => Err(GameDataError::MissingChunk { index }),
        Err(source) => Err(GameDataError::Chunk { index, source }),
    }
}

fn runtime_source<'a>(
    bytes: &'a [u8],
    chunk: Chunk<'_>,
    source: RuntimeSource,
    need: usize,
) -> Result<&'a [u8], GameDataError> {
    let start = chunk.header_offset + crate::chunked::HEADER_SIZE;
    let have = bytes.len() - start;
    bytes
        .get(start..start + need)
        .ok_or(GameDataError::RuntimeSourceTooSmall { source, need, have })
}

fn parse_archive(bytes: &[u8], chunk_index: usize) -> Result<Archive<'_>, GameDataError> {
    let archive = Archive::parse(bytes).map_err(|source| GameDataError::AssetArchive {
        chunk_index,
        source,
    })?;
    archive
        .validate()
        .map_err(|source| GameDataError::AssetArchive {
            chunk_index,
            source,
        })?;
    Ok(archive)
}
