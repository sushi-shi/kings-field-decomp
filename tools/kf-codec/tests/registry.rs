use kf_codec::registry::{register_archive, register_asset, register_tmd};

fn asset() -> Vec<u8> {
    let mut bytes = vec![0; 32];
    bytes[..4].copy_from_slice(&32u32.to_le_bytes());
    bytes[8..12].copy_from_slice(&20u32.to_le_bytes());
    bytes[20..24].copy_from_slice(&0x41u32.to_le_bytes());
    bytes
}

#[test]
fn archive_registers_consecutive_psx_addresses_and_retains_other_slots() {
    let mut bytes = vec![2, 0, 0xa5, 0x5a];
    bytes.extend(asset());
    bytes.extend(asset());
    bytes.extend([0x13, 0x37]);
    let mut table = [0x7b; 32];
    let mut selected = 0;
    register_archive(&mut bytes, 3, 0x800b0000, &mut table, &mut selected).unwrap();
    assert_eq!(&table[..12], &[0x7b; 12]);
    assert_eq!(&table[12..16], &0x800b0004u32.to_le_bytes());
    assert_eq!(&table[16..20], &0x800b0024u32.to_le_bytes());
    assert_eq!(&table[20..], &[0x7b; 12]);
    assert_eq!(selected, 0x800b0038);
    assert_eq!(&bytes[68..], &[0x13, 0x37]);
}

#[test]
fn empty_archive_does_not_change_selected_pointer_or_table() {
    let mut bytes = [0, 0, 0x91, 0x37];
    let mut table = [];
    let mut selected = 0x12345678;
    register_archive(&mut bytes, u16::MAX, 0x800b0000, &mut table, &mut selected).unwrap();
    assert_eq!(selected, 0x12345678);
}

#[test]
fn single_asset_and_raw_tmd_have_distinct_pointer_origins() {
    let mut bytes = asset();
    let mut table = [0; 8];
    let mut selected = 0;
    register_asset(&mut bytes, 1, 0x80100000, &mut table, &mut selected).unwrap();
    assert_eq!(selected, 0x80100014);
    register_tmd(&mut bytes[20..], 0, 0x80100014, &mut table, &mut selected).unwrap();
    assert_eq!(&table[..4], &0x80100014u32.to_le_bytes());
}
