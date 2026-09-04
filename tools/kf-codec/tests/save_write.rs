use kf_codec::save::*;

#[derive(Default)]
struct Writer {
    opens: Vec<(SaveFile, u32)>,
    writes: Vec<(usize, Vec<u8>)>,
    offset: usize,
    fail_payload: bool,
    fail_header: bool,
}

impl IconSource for Writer {
    fn load_icon(&mut self, _: &[u8], _: &mut [u8; ICON_SOURCE_SIZE]) {
        panic!("existing file must not load icons");
    }
}

impl SaveWriteIo for Writer {
    fn clear_events(&mut self) {}
    fn open(&mut self, file: SaveFile, flags: u32) -> i32 {
        self.opens.push((file, flags));
        3
    }
    fn seek(&mut self, _: i32, offset: usize) {
        self.offset = offset;
    }
    fn write(&mut self, _: i32, bytes: &[u8]) -> i32 {
        self.writes.push((self.offset, bytes.to_vec()));
        if (bytes.len() == SAVE_PAYLOAD_SIZE && self.fail_payload)
            || (bytes.len() == SAVE_HEADER_SIZE && self.fail_header)
        {
            return -1;
        }
        bytes.len() as i32
    }
    fn close(&mut self, _: i32) {}
    fn erase_temporary(&mut self) {
        panic!("existing file must not erase temporary file");
    }
}

fn run(ids: [u8; 4], writer: &mut Writer) -> (Result<i32, SaveError>, Vec<u8>, Vec<u8>) {
    let mut header = vec![0xa5; SAVE_HEADER_SIZE + 7];
    header[SLOT_IDS_OFFSET..SLOT_IDS_OFFSET + 4].copy_from_slice(&ids);
    let mut payload = vec![0x7b; SAVE_PAYLOAD_SIZE + 7];
    let mut player = [0; PLAYER_STATE_SIZE];
    player[..4].copy_from_slice(&0xf1234567u32.to_le_bytes());
    player[0x0a] = 0x85;
    for (at, value) in [
        (0x12, 0xff01u16),
        (0x10, 0x8102),
        (0x16, 0x9303),
        (0x14, 0xa404),
    ] {
        player[at..at + 2].copy_from_slice(&value.to_le_bytes());
    }
    let result = write_slot(
        2,
        &mut header,
        &mut payload,
        SaveRuntimeSources {
            player_state: &player,
            world_state: &[0x31; WORLD_STATE_SIZE],
            auxiliary_state: &[0x42; AUXILIARY_STATE_SIZE],
            magic_records: &[0x53; MAGIC_RECORDS_SIZE],
        },
        writer,
    );
    (result, header, payload)
}

#[test]
fn payload_precedes_rotated_header_and_unsigned_summary_fields() {
    let mut writer = Writer::default();
    let (status, header, payload) = run([1, 2, 3, 4], &mut writer);
    assert_eq!(status, Ok(1));
    assert_eq!(&header[SLOT_IDS_OFFSET..SLOT_IDS_OFFSET + 4], &[1, 4, 3, 2]);
    assert_eq!(
        writer.opens,
        [
            (SaveFile::Main, 1),
            (SaveFile::Main, 2),
            (SaveFile::Main, 2)
        ]
    );
    assert_eq!(
        writer.writes[0],
        (
            SAVE_HEADER_SIZE + 3 * SAVE_PAYLOAD_SIZE,
            payload[..SAVE_PAYLOAD_SIZE].to_vec()
        )
    );
    assert_eq!(writer.writes[1], (0, header[..SAVE_HEADER_SIZE].to_vec()));
    let summary: Vec<_> = header[SLOT_SUMMARIES_OFFSET + 3 * 24..SAVE_HEADER_SIZE]
        .chunks_exact(4)
        .map(|word| u32::from_le_bytes(word.try_into().unwrap()))
        .collect();
    assert_eq!(summary, [0xf1234567, 0x85, 0xff01, 0x8102, 0x9303, 0xa404]);
    assert!(payload[PLAYER_STATE_SIZE..WORLD_STATE_OFFSET]
        .iter()
        .all(|&byte| byte == 0x7b));
    assert!(payload[PAYLOAD_OPAQUE_TAIL_OFFSET..]
        .iter()
        .all(|&byte| byte == 0x7b));
    assert_eq!(&header[SAVE_HEADER_SIZE..], &[0xa5; 7]);
}

#[test]
fn failed_payload_keeps_directory_but_failed_header_keeps_rotated_memory() {
    let mut writer = Writer {
        fail_payload: true,
        ..Writer::default()
    };
    let (status, header, _) = run([1, 2, 3, 4], &mut writer);
    assert_eq!(status, Ok(14));
    assert_eq!(&header[SLOT_IDS_OFFSET..SLOT_IDS_OFFSET + 4], &[1, 2, 3, 4]);
    assert_eq!(writer.writes.len(), 5);
    let mut writer = Writer {
        fail_header: true,
        ..Writer::default()
    };
    let (status, header, _) = run([1, 2, 3, 4], &mut writer);
    assert_eq!(status, Ok(14));
    assert_eq!(&header[SLOT_IDS_OFFSET..SLOT_IDS_OFFSET + 4], &[1, 4, 3, 2]);
    assert_eq!(writer.writes.len(), 6);
}

#[test]
fn missing_previous_entry_retains_the_retail_header_minus_one_write() {
    let (status, header, _) = run([1, 0, 3, 4], &mut Writer::default());
    assert_eq!(status, Ok(1));
    assert_eq!(header[SLOT_IDS_OFFSET - 1], 4);
    assert_eq!(&header[SLOT_IDS_OFFSET..SLOT_IDS_OFFSET + 4], &[1, 0, 3, 2]);
}

#[test]
fn full_invalid_directory_cannot_cause_a_negative_file_offset() {
    let mut writer = Writer::default();
    let (status, _, _) = run([1, 2, 3, 1], &mut writer);
    assert_eq!(status, Err(SaveError::NoWritableDirectoryEntry));
    assert!(writer.writes.is_empty());
}
