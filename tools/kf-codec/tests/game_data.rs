use kf_codec::asset_archive;
use kf_codec::chunked;
use kf_codec::game_data::{
    CommonData, GameDataError, MixA, MixB, ARMOR_RECORDS_SOURCE_SIZE, MAP_GRID_SIZE,
    MAP_OBJECT_DEFINITIONS_SOURCE_SIZE,
};

fn asset_archive_payload() -> Vec<u8> {
    let mut asset = vec![0u8; asset_archive::ASSET_HEADER_SIZE];
    let asset_len = asset.len() as u32;
    asset[0..4].copy_from_slice(&asset_len.to_le_bytes());
    let entries = [asset.as_slice()];
    let mut archive = vec![0; asset_archive::encoded_len(&entries).unwrap()];
    asset_archive::encode_into(&entries, [0, 0], &mut archive).unwrap();
    archive
}

fn chunk_stream(payloads: &[&[u8]]) -> Vec<u8> {
    let mut bytes = vec![0; chunked::encoded_len(payloads).unwrap()];
    chunked::encode_into(payloads, &mut bytes).unwrap();
    bytes
}

#[test]
fn common_data_names_the_seven_proven_chunks() {
    let payloads = [
        vec![1; 420],
        vec![2; 3264],
        vec![3; 704],
        vec![4; 756],
        vec![5; 480],
        vec![6; 1128],
        vec![7; 600],
    ];
    let borrowed = payloads.each_ref().map(Vec::as_slice);
    let bytes = chunk_stream(&borrowed);
    let common = CommonData::parse(&bytes).unwrap();

    assert_eq!(common.common_asset, payloads[0]);
    assert_eq!(common.render_cell_windows, payloads[1]);
    assert_eq!(common.weapon_records, payloads[2]);
    assert_eq!(common.armor_records, payloads[3]);
    assert_eq!(common.magic_records, payloads[4]);
    assert_eq!(common.map_object_definitions, payloads[5]);
    assert_eq!(common.player_level_growth, payloads[6]);
    assert_eq!(common.armor_records_source.len(), ARMOR_RECORDS_SOURCE_SIZE);
    assert_eq!(&common.armor_records_source[..756], payloads[3]);
    assert_eq!(
        &common.armor_records_source[756..760],
        &480u32.to_le_bytes()
    );
    assert_eq!(&common.armor_records_source[760..], &payloads[4][..416]);
    assert_eq!(
        common.map_object_definitions_source.len(),
        MAP_OBJECT_DEFINITIONS_SOURCE_SIZE
    );
    assert_eq!(&common.map_object_definitions_source[..1128], payloads[5]);
    assert_eq!(
        &common.map_object_definitions_source[1128..1132],
        &600u32.to_le_bytes()
    );
    assert_eq!(
        &common.map_object_definitions_source[1132..],
        &payloads[6][..148]
    );
    assert_eq!(common.consumed, bytes.len());
    assert!(common.remainder.is_empty());
}

#[test]
fn mixa_splits_the_five_grids_without_decoding_records() {
    let grids: Vec<u8> = (0..5)
        .flat_map(|grid| core::iter::repeat_n(grid, MAP_GRID_SIZE))
        .collect();
    let payloads: [&[u8]; 8] = [
        b"vh",
        b"vb",
        &grids,
        b"items",
        b"objects",
        b"actors",
        b"definitions",
        b"events",
    ];
    let bytes = chunk_stream(&payloads);
    let mixa = MixA::parse(&bytes).unwrap();

    assert_eq!(mixa.map_grids.cell_attributes, vec![0; MAP_GRID_SIZE]);
    assert_eq!(mixa.map_grids.floor_heights, vec![1; MAP_GRID_SIZE]);
    assert_eq!(mixa.map_grids.cell_orientations, vec![2; MAP_GRID_SIZE]);
    assert_eq!(mixa.map_grids.collision_flags, vec![3; MAP_GRID_SIZE]);
    assert_eq!(mixa.map_grids.collision, vec![4; MAP_GRID_SIZE]);
    assert!(mixa.map_grids.remainder.is_empty());
    assert_eq!(mixa.actor_definitions, b"definitions");
}

#[test]
fn mixa_rejects_a_short_grid_chunk() {
    let short = vec![0; MAP_GRID_SIZE * 5 - 1];
    let payloads: [&[u8]; 8] = [b"vh", b"vb", &short, b"i", b"o", b"p", b"d", b"e"];
    let bytes = chunk_stream(&payloads);
    assert!(matches!(
        MixA::parse(&bytes),
        Err(GameDataError::MapGridsTooSmall { .. })
    ));
}

#[test]
fn mixb_accepts_present_or_absent_variant_archive() {
    let archive = asset_archive_payload();
    let without: [&[u8]; 4] = [b"tmd1", b"tmd0", &archive, &archive];
    let bytes = chunk_stream(&without);
    let mixb = MixB::parse(&bytes).unwrap();
    assert!(mixb.embedded_variant_assets.is_none());
    assert_eq!(mixb.assets_from_id_10.count(), 1);

    let with: [&[u8]; 5] = [b"tmd1", b"tmd0", &archive, &archive, &archive];
    let bytes = chunk_stream(&with);
    let mixb = MixB::parse(&bytes).unwrap();
    assert_eq!(mixb.embedded_variant_assets.unwrap().count(), 1);
}
