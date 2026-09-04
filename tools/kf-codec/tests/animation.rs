use kf_codec::animation::{
    prepare_instance, Animation, AnimationError, FrameSelection, InstanceOutcome, InstanceRequest,
    LifecycleEvent, PoolRecord, ValidationReport, Vertex, FULL_WEIGHT,
};
use kf_codec::asset_archive::Archive;
use kf_codec::game_data::{CommonData, MixB};

const ASSET_LEN: usize = 176;

fn put_u16(bytes: &mut [u8], at: usize, value: u16) {
    bytes[at..at + 2].copy_from_slice(&value.to_le_bytes());
}

fn put_u32(bytes: &mut [u8], at: usize, value: u32) {
    bytes[at..at + 4].copy_from_slice(&value.to_le_bytes());
}

fn put_vertex(bytes: &mut [u8], at: usize, vertex: Vertex) {
    bytes[at..at + 8].copy_from_slice(&vertex.to_le_bytes());
}

fn animated_asset() -> Vec<u8> {
    let mut bytes = vec![0; ASSET_LEN];

    // KfAssetHeader and its one-entry clip-offset table.
    put_u32(&mut bytes, 0, ASSET_LEN as u32);
    put_u32(&mut bytes, 4, 1);
    put_u32(&mut bytes, 8, 120);
    put_u32(&mut bytes, 12, 56);
    put_u32(&mut bytes, 16, 20);
    put_u32(&mut bytes, 20, 24);

    // Clip zero has two keyframes. The first applies morph object one at full
    // weight and object zero as the fractional rest delta.
    put_u16(&mut bytes, 24, 2);
    put_u16(&mut bytes, 26, 20);
    put_u32(&mut bytes, 28, 36);
    put_u32(&mut bytes, 32, 46);
    put_u16(&mut bytes, 36, 0);
    put_u16(&mut bytes, 38, 2048);
    put_u16(&mut bytes, 40, 0);
    put_u16(&mut bytes, 42, 1);
    put_u16(&mut bytes, 44, 1);
    put_u16(&mut bytes, 46, 0);
    put_u16(&mut bytes, 48, 2048);
    put_u16(&mut bytes, 50, 0);
    put_u16(&mut bytes, 52, 0);

    // Animation object-offset table, rest object zero, and morph object one.
    put_u32(&mut bytes, 56, 64);
    put_u32(&mut bytes, 60, 92);
    put_u32(&mut bytes, 64, 0x1111_1111);
    put_u32(&mut bytes, 68, 0);
    put_u32(&mut bytes, 72, 2);
    put_vertex(
        &mut bytes,
        76,
        Vertex {
            x: 2,
            y: 4,
            z: 6,
            pad: 99,
        },
    );
    put_vertex(
        &mut bytes,
        84,
        Vertex {
            x: 8,
            y: 10,
            z: 12,
            pad: 99,
        },
    );
    put_u32(&mut bytes, 92, 0x2222_2222);
    put_u32(&mut bytes, 96, 0);
    put_u32(&mut bytes, 100, 2);
    put_vertex(
        &mut bytes,
        104,
        Vertex {
            x: 10,
            y: 20,
            z: 30,
            pad: 99,
        },
    );
    put_vertex(
        &mut bytes,
        112,
        Vertex {
            x: 20,
            y: 40,
            z: 60,
            pad: 99,
        },
    );

    // TMD header, object zero, and two base vertices. TMD offsets are relative
    // to the object-table base at TMD + 12.
    put_u32(&mut bytes, 120, 0x41);
    put_u32(&mut bytes, 128, 1);
    put_u32(&mut bytes, 132, 28);
    put_u32(&mut bytes, 136, 2);
    put_vertex(
        &mut bytes,
        160,
        Vertex {
            x: 100,
            y: 200,
            z: 300,
            pad: 7,
        },
    );
    put_vertex(
        &mut bytes,
        168,
        Vertex {
            x: -100,
            y: -200,
            z: -300,
            pad: 8,
        },
    );
    bytes
}

fn record() -> PoolRecord {
    PoolRecord {
        state: 2,
        asset_id: 7,
        tag: 3,
        keyframe_index: 0x1234,
        rest_object: 0x8006_0040,
        allocation: 0x800a_0000,
        backlink: 0x800b_0100,
    }
}

fn request(record_present: bool) -> InstanceRequest {
    InstanceRequest {
        asset_id: 7,
        caller_vertex_count: 2,
        record_present,
        pool_record_available: true,
        record_address: 0x800b_0000,
        anchor_address: 0x800b_0100,
    }
}

const EMPTY_EVENT: LifecycleEvent = LifecycleEvent::ReleaseAll;

#[test]
fn parses_typed_tables_and_validates_all_references() {
    let bytes = animated_asset();
    let animation = Animation::parse(&bytes).unwrap();

    assert!(animation.is_animated());
    assert_eq!(animation.clip_count(), 1);
    let clip = animation.clip(0).unwrap();
    assert_eq!(clip.keyframe_count, 2);
    assert_eq!(clip.unknown_02, 20);
    let first = clip.keyframe(0).unwrap();
    assert_eq!(first.duration, 2048);
    assert_eq!(first.rest_index, 0);
    assert_eq!(first.morph_indices().collect::<Vec<_>>(), vec![1]);
    assert_eq!(first.rest_object().unwrap().unknown_00, 0x1111_1111);
    assert_eq!(animation.morph_object(1).unwrap().delta(1).unwrap().z, 60);
    assert_eq!(
        animation.base_vertices().unwrap().collect::<Vec<_>>(),
        vec![
            Vertex {
                x: 100,
                y: 200,
                z: 300,
                pad: 7,
            },
            Vertex {
                x: -100,
                y: -200,
                z: -300,
                pad: 8,
            },
        ]
    );
    assert_eq!(
        animation.validate(),
        Ok(ValidationReport {
            clip_count: 1,
            keyframe_count: 2,
            morph_reference_count: 1,
            vertex_count: 2,
        })
    );
}

#[test]
fn selection_exposes_the_uninitialized_retail_cache_index() {
    let bytes = animated_asset();
    let animation = Animation::parse(&bytes).unwrap();

    assert_eq!(
        animation.select_frame(0, 1024, 0x1234),
        Ok(FrameSelection {
            tag: 0,
            keyframe_ordinal: 0,
            keyframe_index: 0x1234,
            fraction: 2048,
            rest_index: 0,
        })
    );
    assert_eq!(
        animation.select_frame(0, 3000, 0x1234),
        Ok(FrameSelection {
            tag: 0,
            keyframe_ordinal: 1,
            keyframe_index: 0x1235,
            fraction: 1904,
            rest_index: 0,
        })
    );
    assert_eq!(
        animation.select_frame(0, 4096, 0x1234),
        Ok(FrameSelection {
            tag: 0,
            keyframe_ordinal: 1,
            keyframe_index: 0x1235,
            fraction: FULL_WEIGHT,
            rest_index: 0,
        })
    );

    let mut reverse = bytes;
    put_u16(&mut reverse, 36, 1);
    assert_eq!(
        Animation::parse(&reverse)
            .unwrap()
            .select_frame(0, 512, 0x1234)
            .unwrap()
            .fraction,
        3072
    );
}

#[test]
fn reproduces_full_morph_then_fractional_rest_blend() {
    let bytes = animated_asset();
    let animation = Animation::parse(&bytes).unwrap();
    let mut cache = [Vertex::default(); 2];
    let mut output = [Vertex::default(); 2];

    let report = animation
        .bind_frame(0, 1024, 0x1234, None, &mut cache, &mut output)
        .unwrap();
    assert!(report.cache_rebuilt);
    assert_eq!(report.vertex_count, 2);
    assert_eq!(
        cache,
        [
            Vertex {
                x: 110,
                y: 220,
                z: 330,
                pad: 7,
            },
            Vertex {
                x: -80,
                y: -160,
                z: -240,
                pad: 8,
            },
        ]
    );
    assert_eq!(
        output,
        [
            Vertex {
                x: 111,
                y: 222,
                z: 333,
                pad: 7,
            },
            Vertex {
                x: -76,
                y: -155,
                z: -234,
                pad: 8,
            },
        ]
    );

    let hit = animation
        .bind_frame(
            0,
            1536,
            0x1234,
            Some(report.cache_key),
            &mut cache,
            &mut output,
        )
        .unwrap();
    assert!(!hit.cache_rebuilt);
    assert_eq!(
        output,
        [
            Vertex {
                x: 111,
                y: 223,
                z: 334,
                pad: 7,
            },
            Vertex {
                x: -74,
                y: -153,
                z: -231,
                pad: 8,
            },
        ]
    );
}

#[test]
fn prepares_null_record_and_retries_vertex_allocation_in_order() {
    let bytes = animated_asset();
    let animation = Animation::parse(&bytes).unwrap();
    let mut anchor = 0;
    let mut pool_record = PoolRecord::from_le_bytes([0x5a; 20]);
    pool_record.state = 0;
    let mut events = [EMPTY_EVENT; 4];

    let report = prepare_instance(
        &animation,
        &mut anchor,
        &mut pool_record,
        request(false),
        &[0, 0x800a_0000],
        &mut events,
    )
    .unwrap();

    assert_eq!(
        report.outcome,
        InstanceOutcome::Ready {
            reinitialized: true
        }
    );
    assert_eq!(report.allocation_attempts, 2);
    assert_eq!(report.event_count, 4);
    assert_eq!(
        events,
        [
            LifecycleEvent::AllocateRecord { available: true },
            LifecycleEvent::AllocateVertices {
                byte_count: 16,
                result: 0,
            },
            LifecycleEvent::ReleaseAll,
            LifecycleEvent::AllocateVertices {
                byte_count: 16,
                result: 0x800a_0000,
            },
        ]
    );
    assert_eq!(anchor, 0x800b_0000);
    assert_eq!(pool_record.asset_id, 7);
    assert_eq!(pool_record.tag, 0xff);
    assert_eq!(pool_record.allocation, 0x800a_0000);
    assert_eq!(pool_record.backlink, 0x800b_0100);
}

#[test]
fn releases_different_or_static_records_and_reports_pool_exhaustion() {
    let bytes = animated_asset();
    let animation = Animation::parse(&bytes).unwrap();
    let mut different = record();
    different.asset_id = 6;
    let mut anchor = 0x800b_0000;
    let mut events = [EMPTY_EVENT; 2];
    let report = prepare_instance(
        &animation,
        &mut anchor,
        &mut different,
        request(true),
        &[0x800a_0000],
        &mut events,
    )
    .unwrap();
    assert_eq!(
        report.outcome,
        InstanceOutcome::Ready {
            reinitialized: true
        }
    );
    assert_eq!(
        events,
        [
            LifecycleEvent::ReleaseRecord {
                allocation: 0x800a_0000,
                backlink: 0x800b_0100,
            },
            LifecycleEvent::AllocateVertices {
                byte_count: 16,
                result: 0x800a_0000,
            },
        ]
    );
    assert_eq!(different.state, 0);
    assert_eq!(different.asset_id, 7);
    assert_eq!(different.tag, 0xff);

    let mut static_bytes = bytes.clone();
    put_u32(&mut static_bytes, 4, 0);
    let static_animation = Animation::parse(&static_bytes).unwrap();
    let mut existing = record();
    let mut anchor = 0x800b_0000;
    let mut static_events = [EMPTY_EVENT; 1];
    let report = prepare_instance(
        &static_animation,
        &mut anchor,
        &mut existing,
        request(true),
        &[],
        &mut static_events,
    )
    .unwrap();
    assert_eq!(report.outcome, InstanceOutcome::Static);
    assert_eq!(anchor, 0);
    assert_eq!(existing.state, 0);
    assert_eq!(existing.allocation, 0);
    assert_eq!(
        static_events[0],
        LifecycleEvent::ReleaseRecord {
            allocation: 0x800a_0000,
            backlink: 0x800b_0100,
        }
    );

    let mut unavailable_request = request(false);
    unavailable_request.pool_record_available = false;
    let mut unused = record();
    let before = unused;
    let mut anchor = 0;
    let mut unavailable_events = [EMPTY_EVENT; 1];
    let report = prepare_instance(
        &animation,
        &mut anchor,
        &mut unused,
        unavailable_request,
        &[],
        &mut unavailable_events,
    )
    .unwrap();
    assert_eq!(report.outcome, InstanceOutcome::PoolUnavailable);
    assert_eq!(unused, before);
    assert_eq!(
        unavailable_events[0],
        LifecycleEvent::AllocateRecord { available: false }
    );
}

#[test]
fn lifecycle_validation_is_transactional() {
    let bytes = animated_asset();
    let animation = Animation::parse(&bytes).unwrap();
    let initial_record = record();
    let mut pool_record = initial_record;
    let mut anchor = 0x800b_0000;
    let mut mismatched = request(true);
    mismatched.caller_vertex_count = 1;
    assert_eq!(
        prepare_instance(
            &animation,
            &mut anchor,
            &mut pool_record,
            mismatched,
            &[],
            &mut [],
        ),
        Err(AnimationError::CallerVertexCountMismatch {
            caller: 1,
            asset: 2,
        })
    );
    assert_eq!(pool_record, initial_record);
    assert_eq!(anchor, 0x800b_0000);

    pool_record.asset_id = 6;
    let different = pool_record;
    assert_eq!(
        prepare_instance(
            &animation,
            &mut anchor,
            &mut pool_record,
            request(true),
            &[0],
            &mut [EMPTY_EVENT; 3],
        ),
        Err(AnimationError::MissingSuccessfulAllocation)
    );
    assert_eq!(pool_record, different);
    assert_eq!(anchor, 0x800b_0000);

    assert_eq!(
        PoolRecord::from_le_bytes(initial_record.to_le_bytes()),
        initial_record
    );
}

#[test]
fn static_assets_do_not_require_animation_table_offsets() {
    let mut bytes = vec![0; 68];
    put_u32(&mut bytes, 0, 68);
    put_u32(&mut bytes, 4, 0);
    put_u32(&mut bytes, 8, 20);
    put_u32(&mut bytes, 12, u32::MAX);
    put_u32(&mut bytes, 16, u32::MAX);
    put_u32(&mut bytes, 20, 0x41);
    put_u32(&mut bytes, 28, 1);
    put_u32(&mut bytes, 32, 28);
    put_u32(&mut bytes, 36, 1);
    put_vertex(
        &mut bytes,
        60,
        Vertex {
            x: 1,
            y: 2,
            z: 3,
            pad: 4,
        },
    );

    let animation = Animation::parse(&bytes).unwrap();
    assert!(!animation.is_animated());
    assert_eq!(animation.clip_count(), 0);
    assert_eq!(animation.base_vertex_count(), Ok(1));
}

#[test]
fn rejects_malformed_offsets_counts_and_output_ranges() {
    assert_eq!(
        Animation::parse(&[0; 19]).unwrap_err(),
        AnimationError::TruncatedAssetHeader { available: 19 }
    );

    let mut negative = vec![0; 20];
    put_u32(&mut negative, 0, 20);
    put_u32(&mut negative, 4, u32::MAX);
    assert_eq!(
        Animation::parse(&negative).unwrap_err(),
        AnimationError::NegativeClipCount { count: -1 }
    );

    let mut bad_clip = animated_asset();
    put_u32(&mut bad_clip, 20, ASSET_LEN as u32);
    assert_eq!(
        Animation::parse(&bad_clip).unwrap().clip(0).unwrap_err(),
        AnimationError::OutOfBounds {
            field: "clip header",
            at: ASSET_LEN,
            need: 4,
            available: 0,
        }
    );

    let mut zero_duration = animated_asset();
    put_u16(&mut zero_duration, 38, 0);
    assert_eq!(
        Animation::parse(&zero_duration)
            .unwrap()
            .select_frame(0, 0, 7),
        Err(AnimationError::ZeroDuration {
            clip: 0,
            keyframe: 0,
        })
    );

    let mut bad_morph_range = animated_asset();
    put_u32(&mut bad_morph_range, 96, 2);
    assert_eq!(
        Animation::parse(&bad_morph_range).unwrap().validate(),
        Err(AnimationError::VertexRangeOutOfBounds {
            object: 1,
            base: 2,
            count: 2,
            vertices: 2,
        })
    );

    let bytes = animated_asset();
    let animation = Animation::parse(&bytes).unwrap();
    let mut cache = [Vertex::default(); 2];
    let mut output = [Vertex::default(); 1];
    assert_eq!(
        animation.bind_frame(0, 0, 0, None, &mut cache, &mut output),
        Err(AnimationError::OutputTooSmall { need: 2, have: 1 })
    );
}

#[derive(Default)]
struct RetailCensus {
    assets: usize,
    animated_assets: usize,
    clips: usize,
    keyframes: usize,
    morph_references: usize,
    max_vertices: u16,
}

impl RetailCensus {
    fn add_asset(&mut self, bytes: &[u8], source: &std::path::Path) {
        let animation =
            Animation::parse(bytes).unwrap_or_else(|error| panic!("{}: {error}", source.display()));
        let report = animation
            .validate()
            .unwrap_or_else(|error| panic!("{}: {error}", source.display()));
        self.assets += 1;
        self.animated_assets += usize::from(animation.is_animated());
        self.clips += report.clip_count;
        self.keyframes += report.keyframe_count;
        self.morph_references += report.morph_reference_count;
        self.max_vertices = self.max_vertices.max(report.vertex_count);

        for clip in animation.clips() {
            let clip = clip.unwrap();
            assert_eq!(clip.unknown_02, 20, "{}", source.display());
            let mut duration = 0u16;
            for keyframe in clip.keyframes() {
                let keyframe = keyframe.unwrap();
                assert_eq!(keyframe.reverse, 0, "{}", source.display());
                duration = duration.wrapping_add(keyframe.duration);
            }
            assert_eq!(duration, 4096, "{}", source.display());
        }
    }

    fn add_archive(&mut self, archive: Archive<'_>, source: &std::path::Path) {
        for asset in archive.assets() {
            self.add_asset(asset.unwrap().bytes, source);
        }
    }
}

#[test]
#[ignore = "requires proprietary SLPS-00017 files via KF_RETAIL_DIR"]
fn every_shipped_asset_animation_table_is_bounded_and_well_formed() {
    let configured = std::path::PathBuf::from(
        std::env::var_os("KF_RETAIL_DIR")
            .expect("set KF_RETAIL_DIR to the extracted retail disc directory"),
    );
    let root = if configured.join("KF").is_dir() {
        configured.join("KF")
    } else {
        configured
    };
    let mut census = RetailCensus::default();

    let com_path = root.join("COM/COM.DAT");
    let com_bytes = std::fs::read(&com_path).unwrap();
    let common = CommonData::parse(&com_bytes).unwrap();
    census.add_asset(common.common_asset, &com_path);

    for floor in 1..=5 {
        let mixb_path = root.join(format!("B{floor}/MIXB.DAT"));
        let mixb_bytes = std::fs::read(&mixb_path).unwrap();
        let mixb = MixB::parse(&mixb_bytes).unwrap();
        census.add_archive(mixb.assets_from_id_10, &mixb_path);
        census.add_archive(mixb.assets_from_id_30, &mixb_path);
        if let Some(archive) = mixb.embedded_variant_assets {
            census.add_archive(archive, &mixb_path);
        }
    }
    for variant in 1..=3 {
        let path = root.join(format!("B5/CHR{variant}.MIM"));
        let bytes = std::fs::read(&path).unwrap();
        census.add_archive(Archive::parse(&bytes).unwrap(), &path);
    }
    for weapon in 0..16 {
        let path = root.join(format!("WEPON/WEP{weapon:02}.MIM"));
        let bytes = std::fs::read(&path).unwrap();
        census.add_asset(&bytes, &path);
    }

    assert_eq!(census.assets, 169);
    assert_eq!(census.animated_assets, 70);
    assert_eq!(census.clips, 214);
    assert_eq!(census.keyframes, 813);
    assert_eq!(census.morph_references, 1_851);
    assert_eq!(census.max_vertices, 651);
}
