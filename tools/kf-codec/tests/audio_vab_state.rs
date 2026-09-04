use kf_codec::audio_vab_state::{
    load_vab_success, SonyVabRegions, VabLoadInputs, VabServiceCall, VabStateError,
    GAME_AUDIO_SEQUENCE_ACTIVE_OFFSET, GAME_AUDIO_STATE_SIZE, VAB_HEADER_SIZE,
    VAB_LENGTH_TABLE_SIZE, VAB_NEW_PROGRAM_SLOTS, VAB_POINTER_TABLE_SIZE, VAB_PROGRAM_SIZE,
    VAB_TONES_PER_PROGRAM, VAB_TONE_SIZE,
};

const VH_ADDRESS: u32 = 0x800b_0000;
const VB_ADDRESS: u32 = 0x8018_0000;
const SPU_START: u32 = 0x1010;

struct Fixture {
    vh: Vec<u8>,
    vb: Vec<u8>,
    audio: [u8; GAME_AUDIO_STATE_SIZE],
    maximum: [u8; 2],
    count: [u8; 2],
    status: [u8; 16],
    vh_ends: [u8; VAB_POINTER_TABLE_SIZE],
    headers: [u8; VAB_POINTER_TABLE_SIZE],
    programs: [u8; VAB_POINTER_TABLE_SIZE],
    tones: [u8; VAB_POINTER_TABLE_SIZE],
    starts: [u8; VAB_POINTER_TABLE_SIZE],
    sizes: [u8; VAB_POINTER_TABLE_SIZE],
    current_header: [u8; 4],
    current_program: [u8; 4],
    current_tone: [u8; 4],
}

impl Fixture {
    fn new() -> Self {
        let programs = 2usize;
        let tones_at = VAB_HEADER_SIZE + VAB_NEW_PROGRAM_SLOTS * VAB_PROGRAM_SIZE;
        let lengths_at = tones_at + programs * VAB_TONES_PER_PROGRAM * VAB_TONE_SIZE;
        let mut vh = vec![0xa5; lengths_at + VAB_LENGTH_TABLE_SIZE];
        vh[0..4].copy_from_slice(b"pBAV");
        vh[4..8].copy_from_slice(&6u32.to_le_bytes());
        vh[12..16].copy_from_slice(&80u32.to_le_bytes());
        vh[18..20].copy_from_slice(&(programs as u16).to_le_bytes());
        vh[20..22].copy_from_slice(&2u16.to_le_bytes());
        vh[22..24].copy_from_slice(&2u16.to_le_bytes());
        // Live program rows are deliberately sparse: program_count is the
        // dense count, not a prefix extent in the 128-row table.
        for slot in 0..VAB_NEW_PROGRAM_SLOTS {
            vh[VAB_HEADER_SIZE + slot * VAB_PROGRAM_SIZE] = 0;
        }
        vh[VAB_HEADER_SIZE] = 1;
        vh[VAB_HEADER_SIZE + 3 * VAB_PROGRAM_SIZE] = 1;
        for byte in &mut vh[lengths_at..] {
            *byte = 0;
        }
        vh[lengths_at + 2..lengths_at + 4].copy_from_slice(&2u16.to_le_bytes());
        vh[lengths_at + 4..lengths_at + 6].copy_from_slice(&3u16.to_le_bytes());

        let mut audio = [0x3c; GAME_AUDIO_STATE_SIZE];
        audio[GAME_AUDIO_SEQUENCE_ACTIVE_OFFSET..GAME_AUDIO_SEQUENCE_ACTIVE_OFFSET + 4]
            .copy_from_slice(&0u32.to_le_bytes());
        let mut status = [1u8; 16];
        status[2] = 0;
        let pattern = |seed: u8| {
            let mut bytes = [0u8; VAB_POINTER_TABLE_SIZE];
            for (index, byte) in bytes.iter_mut().enumerate() {
                *byte = seed.wrapping_add(index as u8);
            }
            bytes
        };
        Self {
            vh,
            vb: vec![0x5a; 40],
            audio,
            maximum: 0x5a5au16.to_le_bytes(),
            count: 2u16.to_le_bytes(),
            status,
            vh_ends: pattern(0x10),
            headers: pattern(0x20),
            programs: pattern(0x30),
            tones: pattern(0x40),
            starts: pattern(0x50),
            sizes: pattern(0x60),
            current_header: 0x1111_1111u32.to_le_bytes(),
            current_program: 0x2222_2222u32.to_le_bytes(),
            current_tone: 0x3333_3333u32.to_le_bytes(),
        }
    }

    fn load(
        &mut self,
        in_transfer: i32,
    ) -> Result<kf_codec::audio_vab_state::VabLoadOutput, VabStateError> {
        load_vab_success(
            &mut self.vh,
            &self.vb,
            &mut self.audio,
            SonyVabRegions {
                maximum_programs: &mut self.maximum,
                open_bank_count: &mut self.count,
                bank_status: &mut self.status,
                vh_end_pointers: &mut self.vh_ends,
                header_pointers: &mut self.headers,
                program_pointers: &mut self.programs,
                tone_pointers: &mut self.tones,
                spu_start_addresses: &mut self.starts,
                body_sizes: &mut self.sizes,
                current_header_pointer: &mut self.current_header,
                current_program_pointer: &mut self.current_program,
                current_tone_pointer: &mut self.current_tone,
            },
            VabLoadInputs {
                vh_address: VH_ADDRESS,
                vb_address: VB_ADDRESS,
                spu_allocation: SPU_START,
                in_transfer,
            },
        )
    }
}

fn table_word(bytes: &[u8], index: usize) -> u32 {
    u32::from_le_bytes(bytes[index * 4..index * 4 + 4].try_into().unwrap())
}

fn vh_half(vh: &[u8], slot: usize, half: usize) -> u16 {
    let at = VAB_HEADER_SIZE + slot * VAB_PROGRAM_SIZE + 12 + half * 2;
    u16::from_le_bytes(vh[at..at + 2].try_into().unwrap())
}

#[test]
fn successful_load_matches_sparse_program_and_sample_state() {
    let mut fixture = Fixture::new();
    let untouched_header_slot = table_word(&fixture.headers, 1);
    let output = fixture.load(0).unwrap();

    assert_eq!(output.report.bank_id, 2);
    assert_eq!(output.report.maximum_programs, 128);
    assert_eq!(output.report.dense_program_count, 2);
    assert_eq!(output.report.body_size, 40);
    assert_eq!(
        u32::from_le_bytes(fixture.audio[..4].try_into().unwrap()),
        VH_ADDRESS
    );
    assert_eq!(
        u16::from_le_bytes(fixture.audio[4..6].try_into().unwrap()),
        2
    );
    assert_eq!(fixture.maximum, 128u16.to_le_bytes());
    assert_eq!(fixture.count, 3u16.to_le_bytes());
    assert_eq!(fixture.status[2], 1);
    assert_eq!(table_word(&fixture.headers, 2), VH_ADDRESS);
    assert_eq!(table_word(&fixture.programs, 2), VH_ADDRESS + 32);
    assert_eq!(
        table_word(&fixture.tones, 2),
        output.report.tone_table_address
    );
    assert_eq!(
        table_word(&fixture.vh_ends, 2),
        output.report.vh_end_address
    );
    assert_eq!(table_word(&fixture.starts, 2), SPU_START);
    assert_eq!(table_word(&fixture.sizes, 2), 40);
    assert_eq!(table_word(&fixture.headers, 1), untouched_header_slot);
    assert_eq!(u32::from_le_bytes(fixture.current_header), VH_ADDRESS);
    assert_eq!(u32::from_le_bytes(fixture.current_program), VH_ADDRESS + 32);
    assert_eq!(
        u32::from_le_bytes(fixture.current_tone),
        output.report.tone_table_address
    );

    let dense = |slot: usize| {
        let at = VAB_HEADER_SIZE + slot * VAB_PROGRAM_SIZE + 8;
        u32::from_le_bytes(fixture.vh[at..at + 4].try_into().unwrap())
    };
    assert_eq!(
        (dense(0), dense(1), dense(2), dense(3), dense(4)),
        (0, 1, 1, 1, 2)
    );
    // Sony stores sample_count+1 cumulative SPU addresses across reserved2.
    assert_eq!(vh_half(&fixture.vh, 0, 0), 0x202);
    assert_eq!(vh_half(&fixture.vh, 0, 1), 0x204);
    assert_eq!(vh_half(&fixture.vh, 1, 0), 0x207);
    assert_eq!(
        output.calls,
        [
            VabServiceCall::GetInTransfer,
            VabServiceCall::SetInTransfer(1),
            VabServiceCall::Malloc(40),
            VabServiceCall::SetTransferMode(0),
            VabServiceCall::SetTransferStartAddress(SPU_START),
            VabServiceCall::Read {
                source: VB_ADDRESS,
                size: 40,
            },
            VabServiceCall::IsTransferCompleted(1),
        ]
    );
}

#[test]
fn busy_transfer_is_rejected_without_mutating_caller_state() {
    let mut fixture = Fixture::new();
    let original_vh = fixture.vh.clone();
    let original_audio = fixture.audio;
    let original_status = fixture.status;
    assert_eq!(fixture.load(1), Err(VabStateError::TransferBusy));
    assert_eq!(fixture.vh, original_vh);
    assert_eq!(fixture.audio, original_audio);
    assert_eq!(fixture.status, original_status);
}

#[test]
fn transfer_extent_must_match_all_256_length_entries() {
    let mut fixture = Fixture::new();
    fixture.vb.pop();
    assert_eq!(
        fixture.load(0),
        Err(VabStateError::BodyLength {
            expected: 40,
            actual: 39,
        })
    );
}

#[test]
fn sony_parser_uses_the_low_byte_of_the_sample_count_field() {
    let mut fixture = Fixture::new();
    fixture.vh[23] = 0x7f;

    let output = fixture.load(0).unwrap();

    assert_eq!(output.report.body_size, 40);
    assert_eq!(vh_half(&fixture.vh, 1, 0), 0x207);
}
