use kf_codec::chunked;
use kf_codec::game_data;
use kf_codec::map_resources::{
    GridDestinations, MapResourceCallbacks, MapResourceEvent, MapResources, MapResourcesError,
    Payload, ResourceFile, MIXA_RETAINED_FROM_VAB_BODY_HEADER, VARIANT_ALLOCATION_SIZE,
};

#[derive(Debug, PartialEq, Eq)]
enum Event {
    Simple(&'static str),
    Floor(i32),
    File(ResourceFile),
    Payload(&'static str, u8, usize, usize),
    Register(&'static str, u16, u8, usize, usize),
    Retain(usize),
    Allocate(u32),
    Mode(i32),
}

#[derive(Default)]
struct Recorder(Vec<Event>);

impl MapResourceCallbacks for Recorder {
    fn event(&mut self, event: MapResourceEvent<'_>) {
        use MapResourceEvent::*;
        let event = match event {
            StopSequenceFade => Event::Simple("stop-sequence-fade"),
            ResetEffects => Event::Simple("reset-effects"),
            ResetAllocator => Event::Simple("reset-allocator"),
            SetFloor(floor) => Event::Floor(floor),
            LoadFile(file) => Event::File(file),
            UploadMapTim => Event::Simple("upload-map-tim"),
            ReleaseLastAllocation => Event::Simple("release-last"),
            LoadVab { header, body } => {
                self.0.push(Event::Payload(
                    "vab-header",
                    header.chunk_index,
                    header.offset,
                    header.bytes.len(),
                ));
                Event::Payload("vab-body", body.chunk_index, body.offset, body.bytes.len())
            }
            PlayCurrentMapSequence => Event::Simple("play-map-sequence"),
            LoadFloorItems(payload) => payload_event("items", payload),
            LoadMapObjects(payload) => payload_event("objects", payload),
            LoadActors(payload) => payload_event("actors", payload),
            LoadActorDefinitions(payload) => payload_event("actor-definitions", payload),
            LoadMapEvents(payload) => payload_event("events", payload),
            RetainMixAAllocation { offset } => Event::Retain(offset),
            RegisterTmd { asset_id, payload } => register_event("tmd", asset_id, payload),
            RegisterAssetArchive {
                first_asset_id,
                payload,
            } => register_event("archive", first_asset_id, payload),
            AllocateVariantAssets { size } => Event::Allocate(size),
            LoadVariantAssets => Event::Simple("load-variant-assets"),
            SyncPlayerPosition => Event::Simple("sync-player-position"),
            SetAllocationMode(mode) => Event::Mode(mode),
        };
        self.0.push(event);
    }
}

fn payload_event(name: &'static str, payload: Payload<'_>) -> Event {
    Event::Payload(
        name,
        payload.chunk_index,
        payload.offset,
        payload.bytes.len(),
    )
}

fn register_event(name: &'static str, id: u16, payload: Payload<'_>) -> Event {
    Event::Register(
        name,
        id,
        payload.chunk_index,
        payload.offset,
        payload.bytes.len(),
    )
}

fn chunk_stream(payloads: &[&[u8]]) -> Vec<u8> {
    let mut bytes = vec![0; chunked::encoded_len(payloads).unwrap()];
    chunked::encode_into(payloads, &mut bytes).unwrap();
    bytes
}

fn fixtures(with_variant: bool) -> (Vec<u8>, Vec<u8>, Vec<Vec<u8>>) {
    let grids: Vec<u8> = (0..5)
        .flat_map(|grid| (0..game_data::MAP_GRID_SIZE).map(move |at| (grid * 31 + at) as u8))
        .collect();
    let mixa_payloads = vec![
        b"vh".to_vec(),
        b"sample-body".to_vec(),
        grids,
        b"items".to_vec(),
        b"objects".to_vec(),
        b"actors".to_vec(),
        b"actor-definitions".to_vec(),
        b"events".to_vec(),
    ];
    let mixa_refs: Vec<&[u8]> = mixa_payloads.iter().map(Vec::as_slice).collect();
    let empty_archive = vec![0, 0, 0xa5, 0x5a];
    let mut mixb_payloads = vec![
        b"primary-tmd".to_vec(),
        b"secondary-tmd".to_vec(),
        empty_archive.clone(),
        empty_archive.clone(),
    ];
    if with_variant {
        mixb_payloads.push(empty_archive);
    }
    let mixb_refs: Vec<&[u8]> = mixb_payloads.iter().map(Vec::as_slice).collect();
    (
        chunk_stream(&mixa_refs),
        chunk_stream(&mixb_refs),
        mixa_payloads,
    )
}

fn execute(with_variant: bool, external_variant: bool) -> (Recorder, Vec<Vec<u8>>) {
    let (mixa, mixb, payloads) = fixtures(with_variant);
    let resources = MapResources::parse(&mixa, &mixb).unwrap();
    let mut grids = vec![vec![0x5a; game_data::MAP_GRID_SIZE + 3]; 5];
    let [a, b, c, d, e] = &mut grids[..] else {
        unreachable!()
    };
    let mut events = Recorder::default();
    let report = resources
        .execute(
            3,
            external_variant,
            GridDestinations {
                cell_attributes: a,
                floor_heights: b,
                cell_orientations: c,
                collision_flags: d,
                collision: e,
            },
            &mut events,
        )
        .unwrap();
    assert_eq!(report.mixa_consumed, mixa.len());
    assert_eq!(report.mixb_consumed, mixb.len());
    assert_eq!(report.used_external_variant, external_variant);
    assert_eq!(report.copied_grid_bytes, 50_000);
    for (index, grid) in grids.iter().enumerate() {
        assert_eq!(
            grid[..game_data::MAP_GRID_SIZE],
            payloads[2][index * 10_000..][..10_000]
        );
        assert_eq!(&grid[game_data::MAP_GRID_SIZE..], &[0x5a; 3]);
    }
    (events, payloads)
}

#[test]
fn embedded_variant_walk_has_exact_outer_order_and_pointers() {
    let (events, _payloads) = execute(true, false);
    let names: Vec<&str> = events
        .0
        .iter()
        .map(|event| match event {
            Event::Simple(name) | Event::Payload(name, ..) | Event::Register(name, ..) => *name,
            Event::Floor(_) => "floor",
            Event::File(ResourceFile::MapTim) => "file-map-tim",
            Event::File(ResourceFile::MixA) => "file-mixa",
            Event::File(ResourceFile::MixB) => "file-mixb",
            Event::Retain(_) => "retain-mixa",
            Event::Allocate(_) => "allocate",
            Event::Mode(_) => "mode",
        })
        .collect();
    assert_eq!(
        names,
        [
            "stop-sequence-fade",
            "reset-effects",
            "reset-allocator",
            "floor",
            "file-map-tim",
            "upload-map-tim",
            "release-last",
            "file-mixa",
            "vab-header",
            "vab-body",
            "play-map-sequence",
            "items",
            "objects",
            "actors",
            "actor-definitions",
            "events",
            "release-last",
            "retain-mixa",
            "file-mixb",
            "tmd",
            "tmd",
            "archive",
            "archive",
            "archive",
            "sync-player-position",
            "mode",
        ]
    );
    assert_eq!(events.0[3], Event::Floor(3));
    let Event::Payload("vab-body", _, vab_body_offset, _) = events.0[9] else {
        unreachable!()
    };
    assert_eq!(
        events.0[17],
        Event::Retain(vab_body_offset - 4 + MIXA_RETAINED_FROM_VAB_BODY_HEADER)
    );
    assert!(matches!(events.0[19], Event::Register("tmd", 1, 0, ..)));
    assert!(matches!(events.0[20], Event::Register("tmd", 0, 1, ..)));
    assert!(matches!(
        events.0[21],
        Event::Register("archive", 10, 2, ..)
    ));
    assert!(matches!(
        events.0[22],
        Event::Register("archive", 30, 3, ..)
    ));
    assert!(matches!(events.0[23], Event::Register("archive", 0, 4, ..)));
    assert_eq!(events.0[25], Event::Mode(2));
}

#[test]
fn external_variant_walk_allocates_without_touching_optional_chunk() {
    let (events, _) = execute(false, true);
    assert!(events.0.contains(&Event::Allocate(VARIANT_ALLOCATION_SIZE)));
    assert!(events.0.contains(&Event::Simple("load-variant-assets")));
    assert!(!events
        .0
        .iter()
        .any(|event| matches!(event, Event::Register("archive", 0, 4, ..))));
}

#[test]
fn failures_are_reported_before_outputs_or_callbacks_change() {
    let (mixa, mixb, _) = fixtures(false);
    let resources = MapResources::parse(&mixa, &mixb).unwrap();
    let mut short = [0x77; game_data::MAP_GRID_SIZE - 1];
    let mut full = [[0x77; game_data::MAP_GRID_SIZE]; 4];
    let [b, c, d, e] = &mut full;
    let mut callbacks = Recorder::default();
    assert_eq!(
        resources.execute(
            1,
            true,
            GridDestinations {
                cell_attributes: &mut short,
                floor_heights: b,
                cell_orientations: c,
                collision_flags: d,
                collision: e,
            },
            &mut callbacks,
        ),
        Err(MapResourcesError::GridOutputTooSmall {
            grid: 0,
            need: game_data::MAP_GRID_SIZE,
            have: game_data::MAP_GRID_SIZE - 1,
        })
    );
    assert!(callbacks.0.is_empty());
    assert!(short.iter().all(|&byte| byte == 0x77));

    let mut grids = [[0x77; game_data::MAP_GRID_SIZE]; 5];
    let [a, b, c, d, e] = &mut grids;
    assert_eq!(
        resources.execute(
            1,
            false,
            GridDestinations {
                cell_attributes: a,
                floor_heights: b,
                cell_orientations: c,
                collision_flags: d,
                collision: e,
            },
            &mut callbacks,
        ),
        Err(MapResourcesError::MissingEmbeddedVariant)
    );
    assert!(callbacks.0.is_empty());
}
