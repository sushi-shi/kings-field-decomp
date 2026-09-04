use kf_codec::tmd::{
    self, prepare_primitive_indices, TmdError, F3, F4, FT3, FT4, G3, G4, GT3, GT4,
};

fn one_packet(mode: u8, body_len: usize) -> Vec<u8> {
    assert_eq!(body_len % 4, 0);
    let packet_at = tmd::HEADER_SIZE + tmd::OBJECT_SIZE;
    let mut tmd = vec![0u8; packet_at + tmd::PACKET_HEADER_SIZE + body_len];

    tmd[0..4].copy_from_slice(&0x41u32.to_le_bytes());
    tmd[8..10].copy_from_slice(&1u16.to_le_bytes());
    let object = tmd::HEADER_SIZE;
    tmd[object + 16..object + 20].copy_from_slice(&(tmd::OBJECT_SIZE as u32).to_le_bytes());
    tmd[object + 20..object + 22].copy_from_slice(&1u16.to_le_bytes());
    tmd[packet_at] = u8::try_from(body_len / 4).unwrap();
    tmd[packet_at + 1] = u8::try_from(body_len / 4).unwrap();
    tmd[packet_at + 3] = mode;
    for at in (0..body_len).step_by(2) {
        let value = 0x2000u16.wrapping_add(u16::try_from(at / 2).unwrap());
        let body_at = packet_at + tmd::PACKET_HEADER_SIZE;
        tmd[body_at + at..body_at + at + 2].copy_from_slice(&value.to_le_bytes());
    }
    tmd
}

fn assert_mode(mode: u8, body_len: usize, indices: &[usize]) {
    for encoded_mode in [mode, mode | 0x02] {
        let mut bytes = one_packet(encoded_mode, body_len);
        let before = bytes.clone();
        let body_at = tmd::HEADER_SIZE + tmd::OBJECT_SIZE + tmd::PACKET_HEADER_SIZE;
        let report = prepare_primitive_indices(&mut bytes).unwrap();

        assert_eq!(report.object_count, 1);
        assert_eq!(report.primitive_count, 1);
        assert_eq!(report.recognized_primitive_count, 1);
        assert_eq!(report.shifted_index_count, indices.len());
        for at in (0..body_len).step_by(2) {
            let old = u16::from_le_bytes([before[body_at + at], before[body_at + at + 1]]);
            let new = u16::from_le_bytes([bytes[body_at + at], bytes[body_at + at + 1]]);
            let expected = if indices.contains(&at) {
                old.wrapping_shl(3)
            } else {
                old
            };
            assert_eq!(new, expected, "mode {encoded_mode:#04x}, body offset {at}");
        }
    }
}

#[test]
fn prepares_f3() {
    assert_mode(F3, 12, &[4, 6, 8, 10]);
}

#[test]
fn prepares_g3() {
    assert_mode(G3, 16, &[4, 6, 8, 10, 12, 14]);
}

#[test]
fn prepares_ft3() {
    assert_mode(FT3, 20, &[12, 14, 16, 18]);
}

#[test]
fn prepares_gt3() {
    assert_mode(GT3, 24, &[12, 14, 16, 18, 20, 22]);
}

#[test]
fn prepares_f4() {
    assert_mode(F4, 16, &[4, 6, 8, 10, 12]);
}

#[test]
fn prepares_g4() {
    assert_mode(G4, 20, &[4, 6, 8, 10, 12, 14, 16, 18]);
}

#[test]
fn prepares_ft4() {
    assert_mode(FT4, 28, &[16, 18, 20, 22, 24]);
}

#[test]
fn prepares_gt4() {
    assert_mode(GT4, 32, &[16, 18, 20, 22, 24, 26, 28, 30]);
}

#[test]
fn shift_wraps_at_sixteen_bits() {
    let mut bytes = one_packet(F3, 12);
    let body_at = tmd::HEADER_SIZE + tmd::OBJECT_SIZE + tmd::PACKET_HEADER_SIZE;
    bytes[body_at + 4..body_at + 6].copy_from_slice(&0xe001u16.to_le_bytes());
    prepare_primitive_indices(&mut bytes).unwrap();
    assert_eq!(
        u16::from_le_bytes([bytes[body_at + 4], bytes[body_at + 5]]),
        8
    );
}

#[test]
fn unknown_mode_advances_without_mutating() {
    let mut bytes = one_packet(0x60, 4);
    let before = bytes.clone();
    let report = prepare_primitive_indices(&mut bytes).unwrap();
    assert_eq!(bytes, before);
    assert_eq!(report.primitive_count, 1);
    assert_eq!(report.recognized_primitive_count, 0);
}

#[test]
fn rejects_truncated_header_and_object_table() {
    assert_eq!(
        prepare_primitive_indices(&mut [0; 11]),
        Err(TmdError::TruncatedHeader { available: 11 })
    );

    let mut bytes = [0u8; tmd::HEADER_SIZE];
    bytes[8..10].copy_from_slice(&1u16.to_le_bytes());
    assert_eq!(
        prepare_primitive_indices(&mut bytes),
        Err(TmdError::TruncatedObjectTable {
            count: 1,
            need: 40,
            available: 12,
        })
    );
}

#[test]
fn rejects_truncated_packet_header() {
    let mut bytes = one_packet(F3, 12);
    bytes.truncate(tmd::HEADER_SIZE + tmd::OBJECT_SIZE + 3);
    assert_eq!(
        prepare_primitive_indices(&mut bytes),
        Err(TmdError::TruncatedPacketHeader {
            object: 0,
            primitive: 0,
            at: 40,
            available: 3,
        })
    );
}

#[test]
fn rejects_packet_shorter_than_its_ilen() {
    let mut bytes = one_packet(F3, 12);
    bytes[tmd::HEADER_SIZE + tmd::OBJECT_SIZE + 1] = 4;
    assert_eq!(
        prepare_primitive_indices(&mut bytes),
        Err(TmdError::TruncatedPacket {
            object: 0,
            primitive: 0,
            at: 40,
            body_len: 16,
            available: 12,
        })
    );
}

#[test]
fn rejects_known_packet_with_too_small_a_body() {
    let mut bytes = one_packet(GT4, 4);
    assert_eq!(
        prepare_primitive_indices(&mut bytes),
        Err(TmdError::PacketBodyTooSmall {
            object: 0,
            primitive: 0,
            at: 40,
            mode: GT4,
            need: 32,
            body_len: 4,
        })
    );
}
