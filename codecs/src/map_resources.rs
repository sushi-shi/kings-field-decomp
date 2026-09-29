use core::fmt;

use crate::chunked::{ChunkError, Cursor};
use crate::game_data::{GameDataError, MixA, MixB, MAP_GRID_SIZE};

pub const VARIANT_ALLOCATION_SIZE: u32 = 0x5a000;
pub const MIXA_RETAINED_FROM_VAB_BODY_HEADER: usize = 16;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ResourceFile {
    MapTim,
    MixA,
    MixB,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Payload<'a> {
    pub chunk_index: u8,
    pub offset: usize,
    pub bytes: &'a [u8],
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MapResourceEvent<'a> {
    StopSequenceFade,
    ResetEffects,
    ResetAllocator,
    SetFloor(i32),
    LoadFile(ResourceFile),
    UploadMapTim,
    ReleaseLastAllocation,
    LoadVab {
        header: Payload<'a>,
        body: Payload<'a>,
    },
    PlayCurrentMapSequence,
    LoadFloorItems(Payload<'a>),
    LoadMapObjects(Payload<'a>),
    LoadActors(Payload<'a>),
    LoadActorDefinitions(Payload<'a>),
    LoadMapEvents(Payload<'a>),
    RetainMixAAllocation {
        offset: usize,
    },
    RegisterTmd {
        asset_id: u16,
        payload: Payload<'a>,
    },
    RegisterAssetArchive {
        first_asset_id: u16,
        payload: Payload<'a>,
    },
    AllocateVariantAssets {
        size: u32,
    },
    LoadVariantAssets,
    SyncPlayerPosition,
    SetAllocationMode(i32),
}

pub trait MapResourceCallbacks {
    fn event(&mut self, event: MapResourceEvent<'_>);
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MapFile {
    MixA,
    MixB,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MapResourcesError {
    GameData {
        file: MapFile,
        source: GameDataError,
    },
    Chunk {
        file: MapFile,
        index: usize,
        source: ChunkError,
    },
    MissingChunk {
        file: MapFile,
        index: usize,
    },
    GridOutputTooSmall {
        grid: usize,
        need: usize,
        have: usize,
    },
    MissingEmbeddedVariant,
}

impl fmt::Display for MapResourcesError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "map resources {self:?}")
    }
}

impl core::error::Error for MapResourcesError {}

pub struct GridDestinations<'a> {
    pub cell_attributes: &'a mut [u8],
    pub floor_heights: &'a mut [u8],
    pub cell_orientations: &'a mut [u8],
    pub collision_flags: &'a mut [u8],
    pub collision: &'a mut [u8],
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct MapResourcesReport {
    pub mixa_consumed: usize,
    pub mixb_consumed: usize,
    pub used_external_variant: bool,
    pub copied_grid_bytes: usize,
}

#[derive(Debug, Clone, Copy)]
pub struct MapResources<'a> {
    mixa: MixA<'a>,
    mixb: MixB<'a>,
    mixa_chunks: [Payload<'a>; 8],
    mixb_chunks: [Payload<'a>; 4],
    embedded_variant: Option<Payload<'a>>,
}

impl<'a> MapResources<'a> {
    pub fn parse(mixa: &'a [u8], mixb: &'a [u8]) -> Result<Self, MapResourcesError> {
        let mixa_view = MixA::parse(mixa).map_err(|source| MapResourcesError::GameData {
            file: MapFile::MixA,
            source,
        })?;
        let mixb_view = MixB::parse(mixb).map_err(|source| MapResourcesError::GameData {
            file: MapFile::MixB,
            source,
        })?;
        let mut mixa_cursor = Cursor::new(mixa);
        let mixa_chunks = [
            next_payload(&mut mixa_cursor, MapFile::MixA, 0)?,
            next_payload(&mut mixa_cursor, MapFile::MixA, 1)?,
            next_payload(&mut mixa_cursor, MapFile::MixA, 2)?,
            next_payload(&mut mixa_cursor, MapFile::MixA, 3)?,
            next_payload(&mut mixa_cursor, MapFile::MixA, 4)?,
            next_payload(&mut mixa_cursor, MapFile::MixA, 5)?,
            next_payload(&mut mixa_cursor, MapFile::MixA, 6)?,
            next_payload(&mut mixa_cursor, MapFile::MixA, 7)?,
        ];
        let mut mixb_cursor = Cursor::new(mixb);
        let mixb_chunks = [
            next_payload(&mut mixb_cursor, MapFile::MixB, 0)?,
            next_payload(&mut mixb_cursor, MapFile::MixB, 1)?,
            next_payload(&mut mixb_cursor, MapFile::MixB, 2)?,
            next_payload(&mut mixb_cursor, MapFile::MixB, 3)?,
        ];
        let embedded_variant = match mixb_cursor.next_chunk() {
            Ok(Some(chunk)) => Some(Payload {
                chunk_index: 4,
                offset: chunk.header_offset + 4,
                bytes: chunk.payload,
            }),
            Ok(None) => None,
            Err(source) => {
                return Err(MapResourcesError::Chunk {
                    file: MapFile::MixB,
                    index: 4,
                    source,
                })
            }
        };
        Ok(Self {
            mixa: mixa_view,
            mixb: mixb_view,
            mixa_chunks,
            mixb_chunks,
            embedded_variant,
        })
    }

    pub const fn mixa(&self) -> MixA<'a> {
        self.mixa
    }

    pub const fn mixb(&self) -> MixB<'a> {
        self.mixb
    }

    pub fn execute(
        &self,
        floor: i32,
        use_variant: bool,
        grids: GridDestinations<'_>,
        callbacks: &mut impl MapResourceCallbacks,
    ) -> Result<MapResourcesReport, MapResourcesError> {
        if !use_variant && self.embedded_variant.is_none() {
            return Err(MapResourcesError::MissingEmbeddedVariant);
        }
        let GridDestinations {
            cell_attributes,
            floor_heights,
            cell_orientations,
            collision_flags,
            collision,
        } = grids;
        let destinations = [
            cell_attributes,
            floor_heights,
            cell_orientations,
            collision_flags,
            collision,
        ];
        for (grid, destination) in destinations.iter().enumerate() {
            if destination.len() < MAP_GRID_SIZE {
                return Err(MapResourcesError::GridOutputTooSmall {
                    grid,
                    need: MAP_GRID_SIZE,
                    have: destination.len(),
                });
            }
        }

        callbacks.event(MapResourceEvent::StopSequenceFade);
        callbacks.event(MapResourceEvent::ResetEffects);
        callbacks.event(MapResourceEvent::ResetAllocator);
        callbacks.event(MapResourceEvent::SetFloor(floor));
        callbacks.event(MapResourceEvent::LoadFile(ResourceFile::MapTim));
        callbacks.event(MapResourceEvent::UploadMapTim);
        callbacks.event(MapResourceEvent::ReleaseLastAllocation);
        callbacks.event(MapResourceEvent::LoadFile(ResourceFile::MixA));
        callbacks.event(MapResourceEvent::LoadVab {
            header: self.mixa_chunks[0],
            body: self.mixa_chunks[1],
        });
        callbacks.event(MapResourceEvent::PlayCurrentMapSequence);

        let sources = [
            self.mixa.map_grids.cell_attributes,
            self.mixa.map_grids.floor_heights,
            self.mixa.map_grids.cell_orientations,
            self.mixa.map_grids.collision_flags,
            self.mixa.map_grids.collision,
        ];
        for (source, destination) in sources.into_iter().zip(destinations) {
            destination[..MAP_GRID_SIZE].copy_from_slice(source);
        }

        callbacks.event(MapResourceEvent::LoadFloorItems(self.mixa_chunks[3]));
        callbacks.event(MapResourceEvent::LoadMapObjects(self.mixa_chunks[4]));
        callbacks.event(MapResourceEvent::LoadActors(self.mixa_chunks[5]));
        callbacks.event(MapResourceEvent::LoadActorDefinitions(self.mixa_chunks[6]));
        callbacks.event(MapResourceEvent::LoadMapEvents(self.mixa_chunks[7]));
        callbacks.event(MapResourceEvent::ReleaseLastAllocation);
        callbacks.event(MapResourceEvent::RetainMixAAllocation {

            offset: self.mixa_chunks[1].offset - 4 + MIXA_RETAINED_FROM_VAB_BODY_HEADER,
        });
        callbacks.event(MapResourceEvent::LoadFile(ResourceFile::MixB));
        callbacks.event(MapResourceEvent::RegisterTmd {
            asset_id: 1,
            payload: self.mixb_chunks[0],
        });
        callbacks.event(MapResourceEvent::RegisterTmd {
            asset_id: 0,
            payload: self.mixb_chunks[1],
        });
        callbacks.event(MapResourceEvent::RegisterAssetArchive {
            first_asset_id: 10,
            payload: self.mixb_chunks[2],
        });
        callbacks.event(MapResourceEvent::RegisterAssetArchive {
            first_asset_id: 30,
            payload: self.mixb_chunks[3],
        });
        if use_variant {
            callbacks.event(MapResourceEvent::AllocateVariantAssets {
                size: VARIANT_ALLOCATION_SIZE,
            });
            callbacks.event(MapResourceEvent::LoadVariantAssets);
        } else {
            callbacks.event(MapResourceEvent::RegisterAssetArchive {
                first_asset_id: 0,
                payload: self.embedded_variant.unwrap(),
            });
        }
        callbacks.event(MapResourceEvent::SyncPlayerPosition);
        callbacks.event(MapResourceEvent::SetAllocationMode(2));
        Ok(MapResourcesReport {
            mixa_consumed: self.mixa.consumed,
            mixb_consumed: self.mixb.consumed,
            used_external_variant: use_variant,
            copied_grid_bytes: MAP_GRID_SIZE * 5,
        })
    }
}

fn next_payload<'a>(
    cursor: &mut Cursor<'a>,
    file: MapFile,
    index: usize,
) -> Result<Payload<'a>, MapResourcesError> {
    match cursor.next_chunk() {
        Ok(Some(chunk)) => Ok(Payload {
            chunk_index: index as u8,
            offset: chunk.header_offset + 4,
            bytes: chunk.payload,
        }),
        Ok(None) => Err(MapResourcesError::MissingChunk { file, index }),
        Err(source) => Err(MapResourcesError::Chunk {
            file,
            index,
            source,
        }),
    }
}
