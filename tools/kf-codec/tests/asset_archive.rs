use kf_codec::asset_archive::{self, Archive, AssetArchiveError, AssetHeader};

fn asset(animation_clip_count: i32, tmd_offset: u32, tail: &[u8]) -> Vec<u8> {
    let size = 20 + tail.len();
    let mut bytes = Vec::with_capacity(size);
    bytes.extend_from_slice(&(size as u32).to_le_bytes());
    bytes.extend_from_slice(&animation_clip_count.to_le_bytes());
    bytes.extend_from_slice(&tmd_offset.to_le_bytes());
    bytes.extend_from_slice(&12u32.to_le_bytes());
    bytes.extend_from_slice(&16u32.to_le_bytes());
    bytes.extend_from_slice(tail);
    bytes
}

#[test]
fn reads_count_reserved_bytes_and_self_sized_assets() {
    let first = asset(-1, 20, b"TMD0");
    let second = asset(7, 22, b"__TMD1");
    let assets = [first.as_slice(), second.as_slice()];
    let mut bytes = vec![0; asset_archive::encoded_len(&assets).unwrap()];
    asset_archive::encode_into(&assets, [0x34, 0x12], &mut bytes).unwrap();

    let archive = Archive::parse(&bytes).unwrap();
    assert_eq!(archive.count(), 2);
    assert_eq!(archive.reserved(), [0x34, 0x12]);

    let decoded: Vec<_> = archive.assets().map(|entry| entry.unwrap()).collect();
    assert_eq!(decoded[0].index, 0);
    assert_eq!(decoded[0].offset, 4);
    assert_eq!(decoded[0].header.animation_clip_count, -1);
    assert_eq!(decoded[0].tmd_data(), Some(&b"TMD0"[..]));
    assert_eq!(decoded[1].header.tmd_data_offset, 22);
    assert_eq!(decoded[1].tmd_data(), Some(&b"TMD1"[..]));
    assert_eq!(archive.validate().unwrap(), bytes.len());
    assert!(archive.remainder().unwrap().is_empty());
}

#[test]
fn header_parser_is_explicitly_little_endian() {
    let bytes = [
        0x18, 0, 0, 0, 0xfe, 0xff, 0xff, 0xff, 0x14, 0, 0, 0, 0x78, 0x56, 0x34, 0x12, 0xef, 0xcd,
        0xab, 0x90,
    ];
    assert_eq!(
        AssetHeader::parse(&bytes),
        Some(AssetHeader {
            byte_size: 24,
            animation_clip_count: -2,
            tmd_data_offset: 20,
            object_table_offset: 0x1234_5678,
            clip_table_offset: 0x90ab_cdef,
        })
    );
}

#[test]
fn rejects_a_zero_sized_asset_instead_of_looping() {
    let mut bytes = vec![1, 0, 0, 0];
    bytes.extend_from_slice(&[0; 20]);
    let archive = Archive::parse(&bytes).unwrap();

    assert_eq!(
        archive.assets().next().unwrap(),
        Err(AssetArchiveError::InvalidAssetSize {
            index: 0,
            at: 4,
            declared: 0,
        })
    );
}

#[test]
fn rejects_a_truncated_asset() {
    let mut bytes = vec![1, 0, 0, 0];
    bytes.extend_from_slice(&24u32.to_le_bytes());
    bytes.extend_from_slice(&[0; 16]);
    let archive = Archive::parse(&bytes).unwrap();

    assert_eq!(
        archive.assets().next().unwrap(),
        Err(AssetArchiveError::TruncatedAsset {
            index: 0,
            at: 4,
            declared: 24,
            available: 20,
        })
    );
}

#[test]
fn writer_rejects_an_asset_whose_header_size_disagrees() {
    let mut entry = asset(0, 20, b"tail");
    entry[0] = 20;
    let entries = [entry.as_slice()];

    assert_eq!(
        asset_archive::encoded_len(&entries),
        Err(AssetArchiveError::EncodedSizeMismatch {
            index: 0,
            declared: 20,
            actual: 24,
        })
    );
}
