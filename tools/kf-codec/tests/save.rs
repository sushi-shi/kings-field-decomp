use kf_codec::save::*;

fn runtime(fill: u8) -> (Vec<u8>, Vec<u8>, Vec<u8>, Vec<u8>) {
    (
        vec![fill; PLAYER_STATE_SIZE + 5],
        vec![fill; WORLD_STATE_SIZE + 5],
        vec![fill; AUXILIARY_STATE_SIZE + 5],
        vec![fill; MAGIC_RECORDS_SIZE + 5],
    )
}

#[test]
fn payload_views_restore_and_inverse_serialization_preserve_opaque_bytes() {
    let mut payload = vec![0x6c; SAVE_PAYLOAD_SIZE + 7];
    for index in 0..PLAYER_STATE_SIZE {
        payload[index] = index as u8;
    }
    payload[WORLD_STATE_OFFSET..WORLD_STATE_OFFSET + WORLD_STATE_SIZE].fill(0x22);
    payload[AUXILIARY_STATE_OFFSET..AUXILIARY_STATE_OFFSET + AUXILIARY_STATE_SIZE].fill(0x33);
    for index in 0..MAGIC_FLAG_COUNT {
        payload[MAGIC_FLAGS_OFFSET + index] = 0x40 + index as u8;
    }
    let view = SavePayload::parse(&payload).unwrap();
    assert_eq!(view.player_state().len(), PLAYER_STATE_SIZE);
    assert_eq!(
        view.unresolved_0e0().len(),
        WORLD_STATE_OFFSET - PLAYER_STATE_SIZE
    );
    assert_eq!(view.world_state().len(), WORLD_STATE_SIZE);
    assert_eq!(view.auxiliary_state().len(), AUXILIARY_STATE_SIZE);
    assert_eq!(view.magic_flags().len(), MAGIC_FLAG_COUNT);
    assert_eq!(view.opaque_tail().len(), PAYLOAD_OPAQUE_TAIL_SIZE);

    let (mut player, mut world, mut auxiliary, mut magic) = runtime(0xa5);
    player[0x6c..0x70].copy_from_slice(&[1, 2, 3, 4]);
    player[0x74..0x78].copy_from_slice(&[5, 6, 7, 8]);
    restore_payload(
        &payload,
        SaveRuntimeDestinations {
            player_state: &mut player,
            world_state: &mut world,
            auxiliary_state: &mut auxiliary,
            magic_records: &mut magic,
        },
    )
    .unwrap();
    assert_eq!(&player[0x6c..0x70], &[1, 2, 3, 4]);
    assert_eq!(&player[0x74..0x78], &[5, 6, 7, 8]);
    assert!(world[..WORLD_STATE_SIZE].iter().all(|&byte| byte == 0x22));
    assert!(auxiliary[..AUXILIARY_STATE_SIZE]
        .iter()
        .all(|&byte| byte == 0x33));
    for index in 0..MAGIC_FLAG_COUNT {
        assert_eq!(magic[index * MAGIC_RECORD_SIZE], 0x40 + index as u8);
        assert!(
            magic[index * MAGIC_RECORD_SIZE + 1..(index + 1) * MAGIC_RECORD_SIZE]
                .iter()
                .all(|&byte| byte == 0xa5)
        );
    }
    assert!(player[PLAYER_STATE_SIZE..].iter().all(|&byte| byte == 0xa5));

    let mut encoded = vec![0xee; SAVE_PAYLOAD_SIZE + 3];
    serialize_payload(
        SaveRuntimeSources {
            player_state: &player,
            world_state: &world,
            auxiliary_state: &auxiliary,
            magic_records: &magic,
        },
        &mut encoded,
    )
    .unwrap();
    assert!(encoded[PLAYER_STATE_SIZE..WORLD_STATE_OFFSET]
        .iter()
        .all(|&byte| byte == 0xee));
    assert!(encoded[PAYLOAD_OPAQUE_TAIL_OFFSET..]
        .iter()
        .all(|&byte| byte == 0xee));
    assert_eq!(
        &encoded[WORLD_STATE_OFFSET..WORLD_STATE_OFFSET + WORLD_STATE_SIZE],
        &world[..WORLD_STATE_SIZE]
    );
}

#[test]
fn catalog_clears_only_0x24_and_distributes_slot_summaries() {
    let mut header = vec![0; SAVE_HEADER_SIZE];
    header[SLOT_IDS_OFFSET..SLOT_IDS_OFFSET + 4].copy_from_slice(&[3, 0, 1, 4]);
    for directory_index in 0..SLOT_COUNT {
        let at = SLOT_SUMMARIES_OFFSET + directory_index * SLOT_SUMMARY_SIZE;
        header[at..at + SLOT_SUMMARY_SIZE].fill(0x10 + directory_index as u8);
    }
    let view = SaveHeader::parse(&header).unwrap();
    assert_eq!(view.playstation_header().len(), PLAYSTATION_HEADER_SIZE);
    assert_eq!(view.directory().len(), SAVE_DIRECTORY_SIZE);
    let mut summaries = vec![0xaa; SLOT_SUMMARY_SIZE * 3 + 5];
    assert_eq!(apply_catalog(1, &header, &mut summaries).unwrap(), 1);
    assert!(summaries[..SLOT_SUMMARY_SIZE]
        .iter()
        .all(|&byte| byte == 0x12));
    // Retail's 0x24 clear covers slot one and the first half of slot two.
    assert!(summaries[SLOT_SUMMARY_SIZE..0x24]
        .iter()
        .all(|&byte| byte == 0));
    assert!(summaries[0x24..SLOT_SUMMARY_SIZE * 2]
        .iter()
        .all(|&byte| byte == 0xaa));
    assert!(summaries[SLOT_SUMMARY_SIZE * 2..SLOT_SUMMARY_SIZE * 3]
        .iter()
        .all(|&byte| byte == 0x10));
    assert!(summaries[SLOT_SUMMARY_SIZE * 3..]
        .iter()
        .all(|&byte| byte == 0xaa));
}

struct Io {
    file: Vec<u8>,
    offset: usize,
    failures: usize,
    open: bool,
    reads: Vec<(usize, usize)>,
    closes: usize,
    clears: usize,
}

impl SaveReadIo for Io {
    fn clear_events(&mut self) {
        self.clears += 1;
    }

    fn open_read(&mut self) -> bool {
        self.open
    }

    fn seek(&mut self, offset: usize) {
        self.offset = offset;
    }

    fn read(&mut self, destination: &mut [u8]) -> i32 {
        self.reads.push((self.offset, destination.len()));
        if self.failures != 0 {
            self.failures -= 1;
            destination[0] = 0x99;
            return 1;
        }
        let Some(source) = self.file.get(self.offset..self.offset + destination.len()) else {
            return -1;
        };
        destination.copy_from_slice(source);
        destination.len() as i32
    }

    fn close(&mut self) {
        self.closes += 1;
    }
}

#[test]
fn header_read_retries_five_and_uses_retail_statuses() {
    let expected = vec![0x31; SAVE_HEADER_SIZE];
    let mut io = Io {
        file: expected.clone(),
        offset: 0,
        failures: 2,
        open: true,
        reads: Vec::new(),
        closes: 0,
        clears: 0,
    };
    let mut output = vec![0xa1; SAVE_HEADER_SIZE + 4];
    assert_eq!(read_header(&mut output, &mut io).unwrap(), 1);
    assert_eq!(&output[..SAVE_HEADER_SIZE], &expected);
    assert_eq!(&output[SAVE_HEADER_SIZE..], &[0xa1; 4]);
    assert_eq!(io.reads.len(), 3);
    assert_eq!(io.closes, 1);

    io.failures = READ_ATTEMPTS;
    io.reads.clear();
    assert_eq!(read_header(&mut output, &mut io).unwrap(), 13);
    assert_eq!(io.reads.len(), READ_ATTEMPTS);
    io.open = false;
    output.fill(0xcc);
    assert_eq!(read_header(&mut output, &mut io).unwrap(), 7);
    assert!(output[..SAVE_HEADER_SIZE].iter().all(|&byte| byte == 0));
}

#[test]
fn slot_read_validates_summaries_with_status_12_and_restores_payload() {
    let mut cached = vec![0; SAVE_HEADER_SIZE];
    cached[SLOT_IDS_OFFSET + 1] = 2;
    cached[SLOT_SUMMARIES_OFFSET..SAVE_HEADER_SIZE].fill(0x55);
    let mut saved_payload = vec![0; SAVE_PAYLOAD_SIZE];
    saved_payload[..PLAYER_STATE_SIZE].fill(0x11);
    saved_payload[WORLD_STATE_OFFSET..WORLD_STATE_OFFSET + WORLD_STATE_SIZE].fill(0x22);
    saved_payload[AUXILIARY_STATE_OFFSET..AUXILIARY_STATE_OFFSET + AUXILIARY_STATE_SIZE].fill(0x33);
    saved_payload[MAGIC_FLAGS_OFFSET..MAGIC_FLAGS_OFFSET + MAGIC_FLAG_COUNT].fill(0x44);
    let mut file = vec![0; SAVE_HEADER_SIZE + SAVE_PAYLOAD_SIZE * 2];
    file[..SAVE_HEADER_SIZE].copy_from_slice(&cached);
    file[SAVE_HEADER_SIZE + SAVE_PAYLOAD_SIZE..].copy_from_slice(&saved_payload);
    let mut io = Io {
        file,
        offset: 0,
        failures: 0,
        open: true,
        reads: Vec::new(),
        closes: 0,
        clears: 0,
    };
    let mut payload_buffer = vec![0x99; SAVE_PAYLOAD_SIZE];
    let (mut player, mut world, mut auxiliary, mut magic) = runtime(0xa7);
    assert_eq!(
        read_slot(
            2,
            &cached,
            &mut payload_buffer,
            SaveRuntimeDestinations {
                player_state: &mut player,
                world_state: &mut world,
                auxiliary_state: &mut auxiliary,
                magic_records: &mut magic,
            },
            &mut io,
        )
        .unwrap(),
        1
    );
    assert_eq!(
        io.reads,
        [
            (0, SAVE_HEADER_SIZE),
            (SAVE_HEADER_SIZE + SAVE_PAYLOAD_SIZE, SAVE_PAYLOAD_SIZE)
        ]
    );
    assert!(world[..WORLD_STATE_SIZE].iter().all(|&byte| byte == 0x22));

    io.file[SLOT_SUMMARIES_OFFSET] ^= 1;
    io.reads.clear();
    assert_eq!(
        read_slot(
            2,
            &cached,
            &mut payload_buffer,
            SaveRuntimeDestinations {
                player_state: &mut player,
                world_state: &mut world,
                auxiliary_state: &mut auxiliary,
                magic_records: &mut magic,
            },
            &mut io,
        )
        .unwrap(),
        12
    );
    assert_eq!(io.reads, [(0, SAVE_HEADER_SIZE)]);
}

#[derive(Default)]
struct Icons {
    paths: Vec<Vec<u8>>,
}

impl IconSource for Icons {
    fn load_icon(&mut self, path: &[u8], image: &mut [u8; ICON_SOURCE_SIZE]) {
        self.paths.push(path.to_vec());
        image.fill(self.paths.len() as u8);
        for (index, byte) in image.iter_mut().enumerate() {
            *byte = byte.wrapping_add(index as u8);
        }
    }
}

#[test]
fn initialized_header_extracts_clut_and_three_icon_frames() {
    let mut header = vec![0xcc; SAVE_HEADER_SIZE + 3];
    let mut payload = vec![0xcc; SAVE_PAYLOAD_SIZE + 3];
    let mut icons = Icons::default();
    initialize_buffers(&mut header, &mut payload, &mut icons).unwrap();
    assert_eq!(&header[..4], &[b'S', b'C', 0x13, 5]);
    assert_eq!(
        icons.paths,
        [b"TIM\\ICO1.TIM", b"TIM\\ICO2.TIM", b"TIM\\ICO3.TIM"]
    );
    assert_eq!(header[0x60], 1u8.wrapping_add(0x14));
    assert_eq!(header[0x80], 1u8.wrapping_add(0x40));
    assert_eq!(header[0x100], 2u8.wrapping_add(0x40));
    assert_eq!(header[0x180], 3u8.wrapping_add(0x40));
    assert!(header[PLAYSTATION_HEADER_SIZE..SAVE_HEADER_SIZE]
        .iter()
        .all(|&byte| byte == 0));
    assert!(payload[..SAVE_PAYLOAD_SIZE].iter().all(|&byte| byte == 0));
    assert_eq!(&header[SAVE_HEADER_SIZE..], &[0xcc; 3]);
    assert_eq!(&payload[SAVE_PAYLOAD_SIZE..], &[0xcc; 3]);
}
