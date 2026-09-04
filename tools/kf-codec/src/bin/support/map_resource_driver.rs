//! Host-only transport adapter for the allocation-free map-resource walk.

use kf_codec::map_resources::{
    GridDestinations, MapResourceCallbacks, MapResourceEvent, MapResources, Payload, ResourceFile,
};

const GRID_SIZE: usize = 10_000;
const ALL_GRIDS_SIZE: usize = GRID_SIZE * 5;

struct Context {
    trace: Vec<u8>,
}

impl Context {
    fn payload(&mut self, payload: Payload<'_>) {
        self.trace.push(payload.chunk_index);
        self.trace
            .extend_from_slice(&u32::try_from(payload.offset).unwrap().to_le_bytes());
        self.trace
            .extend_from_slice(&u32::try_from(payload.bytes.len()).unwrap().to_le_bytes());
    }
}

impl MapResourceCallbacks for Context {
    fn event(&mut self, event: MapResourceEvent<'_>) {
        use MapResourceEvent::*;
        match event {
            StopSequenceFade => self.trace.push(1),
            ResetEffects => self.trace.push(2),
            ResetAllocator => self.trace.push(3),
            SetFloor(floor) => {
                self.trace.push(4);
                self.trace.extend_from_slice(&floor.to_le_bytes());
            }
            LoadFile(file) => {
                self.trace.push(5);
                self.trace.push(match file {
                    ResourceFile::MapTim => 0,
                    ResourceFile::MixA => 1,
                    ResourceFile::MixB => 2,
                });
            }
            UploadMapTim => self.trace.push(6),
            ReleaseLastAllocation => self.trace.push(7),
            LoadVab { header, body } => {
                self.trace.push(8);
                self.payload(header);
                self.payload(body);
            }
            PlayCurrentMapSequence => self.trace.push(9),
            LoadFloorItems(payload) => {
                self.trace.push(10);
                self.payload(payload);
            }
            LoadMapObjects(payload) => {
                self.trace.push(11);
                self.payload(payload);
            }
            LoadActors(payload) => {
                self.trace.push(12);
                self.payload(payload);
            }
            LoadActorDefinitions(payload) => {
                self.trace.push(13);
                self.payload(payload);
            }
            LoadMapEvents(payload) => {
                self.trace.push(14);
                self.payload(payload);
            }
            RetainMixAAllocation { offset } => {
                self.trace.push(15);
                self.trace
                    .extend_from_slice(&u32::try_from(offset).unwrap().to_le_bytes());
            }
            RegisterTmd { asset_id, payload } => {
                self.trace.push(16);
                self.trace.extend_from_slice(&asset_id.to_le_bytes());
                self.payload(payload);
            }
            RegisterAssetArchive {
                first_asset_id,
                payload,
            } => {
                self.trace.push(17);
                self.trace.extend_from_slice(&first_asset_id.to_le_bytes());
                self.payload(payload);
            }
            AllocateVariantAssets { size } => {
                self.trace.push(18);
                self.trace.extend_from_slice(&size.to_le_bytes());
            }
            LoadVariantAssets => self.trace.push(19),
            SyncPlayerPosition => self.trace.push(20),
            SetAllocationMode(mode) => {
                self.trace.push(21);
                self.trace.extend_from_slice(&mode.to_le_bytes());
            }
        }
    }
}

pub fn execute(mut blocks: Vec<Vec<u8>>) -> Result<Vec<Vec<u8>>, String> {
    if blocks.len() != 4 || blocks[2].len() != ALL_GRIDS_SIZE || blocks[3].len() != 8 {
        return Err(
            "map-resources expects MIXA, MIXB, five 10000-byte grids, and <i32 floor,u32 variant>"
                .into(),
        );
    }
    let params = blocks.pop().unwrap();
    let mut grids = blocks.pop().unwrap();
    let mixb = blocks.pop().unwrap();
    let mixa = blocks.pop().unwrap();
    let floor = i32::from_le_bytes(params[..4].try_into().unwrap());
    let variant = u32::from_le_bytes(params[4..].try_into().unwrap());
    if variant > 1 {
        return Err("map-resources variant must be zero or one".into());
    }

    let resources = MapResources::parse(&mixa, &mixb).map_err(|error| format!("{error:?}"))?;
    let (cell_attributes, tail) = grids.split_at_mut(GRID_SIZE);
    let (floor_heights, tail) = tail.split_at_mut(GRID_SIZE);
    let (cell_orientations, tail) = tail.split_at_mut(GRID_SIZE);
    let (collision_flags, collision) = tail.split_at_mut(GRID_SIZE);
    let mut context = Context { trace: Vec::new() };
    let report = resources
        .execute(
            floor,
            variant != 0,
            GridDestinations {
                cell_attributes,
                floor_heights,
                cell_orientations,
                collision_flags,
                collision,
            },
            &mut context,
        )
        .map_err(|error| format!("{error:?}"))?;
    let mut report_bytes = Vec::with_capacity(16);
    report_bytes.extend_from_slice(
        &u32::try_from(report.mixa_consumed)
            .map_err(|_| "MIXA extent exceeds u32")?
            .to_le_bytes(),
    );
    report_bytes.extend_from_slice(
        &u32::try_from(report.mixb_consumed)
            .map_err(|_| "MIXB extent exceeds u32")?
            .to_le_bytes(),
    );
    report_bytes.extend_from_slice(&u32::from(report.used_external_variant).to_le_bytes());
    report_bytes.extend_from_slice(
        &u32::try_from(report.copied_grid_bytes)
            .map_err(|_| "grid extent exceeds u32")?
            .to_le_bytes(),
    );
    Ok(vec![grids, context.trace, report_bytes])
}
