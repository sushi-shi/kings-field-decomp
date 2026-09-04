#[path = "../src/audio.rs"]
mod audio;

use audio::{
    apply_sequence_event, apply_successful_game_sequence_open, encode_variable_length,
    initialize_sequence_score, read_variable_length, ChannelMessage, SeqCallback, SeqError,
    SeqEventKind, SeqHeader, Sequence, VabBank, VabError, VabHeader, VabProgram, VabTone,
    GAME_AUDIO_STATE_SIZE, SEQ_MAGIC, SEQ_SCORE_RECORD_SIZE, VAB_HEADER_SIZE, VAB_MAGIC,
    VAB_OFFSET_TABLE_SIZE, VAB_PROGRAM_SIZE, VAB_PROGRAM_SLOTS, VAB_TONES_PER_PROGRAM,
    VAB_TONE_SIZE,
};

fn synthetic_vab() -> (Vec<u8>, Vec<u8>, VabProgram, VabTone) {
    let program = VabProgram {
        tone_count: 1,
        master_volume: 100,
        priority: 7,
        mode: 2,
        pan: 64,
        reserved0: -1,
        attribute: -1234,
        reserved1: 0x1122_3344,
        reserved2: 0x5566_7788,
    };
    let tone = VabTone {
        priority: 3,
        mode: 4,
        volume: 99,
        pan: 65,
        center_note: 60,
        center_shift: 1,
        minimum_note: 12,
        maximum_note: 100,
        vibrato_width: 5,
        vibrato_time: 6,
        portamento_width: 7,
        portamento_time: 8,
        pitch_bend_minimum: 2,
        pitch_bend_maximum: 3,
        reserved1: 0xee,
        reserved2: 0xdd,
        adsr1: 0x8f12,
        adsr2: 0x7ac3,
        program: -2,
        sample: 1,
        reserved: [-1, 2, -3, 4],
    };
    let header_len = VAB_HEADER_SIZE
        + VAB_PROGRAM_SLOTS * VAB_PROGRAM_SIZE
        + VAB_TONES_PER_PROGRAM * VAB_TONE_SIZE
        + VAB_OFFSET_TABLE_SIZE;
    let body: Vec<u8> = (0..48).map(|value| value ^ 0x5a).collect();
    let header = VabHeader {
        form: VAB_MAGIC,
        version: 6,
        id: 0,
        file_size: (header_len + body.len()) as u32,
        reserved0: 0xeeee,
        program_count: 1,
        tone_count: 1,
        sample_count: 2,
        master_volume: 127,
        pan: 63,
        attribute1: 0,
        attribute2: 0,
        reserved1: u32::MAX,
    };
    let mut vh = vec![0; header_len];
    vh[..VAB_HEADER_SIZE].copy_from_slice(&header.to_le_bytes());
    vh[VAB_HEADER_SIZE..VAB_HEADER_SIZE + VAB_PROGRAM_SIZE].copy_from_slice(&program.to_le_bytes());
    let tones_at = VAB_HEADER_SIZE + VAB_PROGRAM_SLOTS * VAB_PROGRAM_SIZE;
    vh[tones_at..tones_at + VAB_TONE_SIZE].copy_from_slice(&tone.to_le_bytes());
    let offsets_at = header_len - VAB_OFFSET_TABLE_SIZE;
    vh[offsets_at + 2..offsets_at + 4].copy_from_slice(&2u16.to_le_bytes());
    vh[offsets_at + 4..offsets_at + 6].copy_from_slice(&4u16.to_le_bytes());
    (vh, body, program, tone)
}

#[test]
fn vab_typed_tables_samples_and_roundtrip() {
    let (mut vh, vb, expected_program, expected_tone) = synthetic_vab();
    let mut sparse_program = expected_program;
    sparse_program.master_volume = 37;
    let sparse_at = VAB_HEADER_SIZE + 126 * VAB_PROGRAM_SIZE;
    vh[sparse_at..sparse_at + VAB_PROGRAM_SIZE].copy_from_slice(&sparse_program.to_le_bytes());
    let bank = VabBank::parse(&vh, &vb).unwrap();
    assert_eq!(bank.header.version, 6);
    assert_eq!(bank.encoded_header(), vh);
    assert_eq!(bank.encoded_body(), vb);
    assert_eq!(bank.program(0), Some(expected_program));
    assert_eq!(bank.programs().collect::<Vec<_>>(), [expected_program]);
    assert_eq!(bank.program_slot(126), Some(sparse_program));
    assert_eq!(bank.program_slots().count(), VAB_PROGRAM_SLOTS);
    assert_eq!(bank.program_slot(VAB_PROGRAM_SLOTS), None);
    assert_eq!(bank.tone(0, 0), Some(expected_tone));
    assert_eq!(bank.tones(0).unwrap().collect::<Vec<_>>(), [expected_tone]);

    let samples = bank.samples().collect::<Vec<_>>();
    assert_eq!(samples.len(), 2);
    assert_eq!((samples[0].index, samples[1].index), (0, 1));
    assert_eq!((samples[0].offset, samples[0].size_units), (0, 2));
    assert_eq!(samples[0].data, &vb[..16]);
    assert_eq!((samples[1].offset, samples[1].size_units), (16, 4));
    assert_eq!(samples[1].data, &vb[16..]);

    let mut encoded_vh = vec![0xcc; vh.len()];
    let mut encoded_vb = vec![0xcc; vb.len()];
    assert_eq!(bank.encode_header_into(&mut encoded_vh).unwrap(), vh.len());
    assert_eq!(bank.encode_body_into(&mut encoded_vb).unwrap(), vb.len());
    assert_eq!(encoded_vh, vh);
    assert_eq!(encoded_vb, vb);
    assert_eq!(bank.header.to_le_bytes(), vh[..VAB_HEADER_SIZE]);
    assert_eq!(
        expected_program.to_le_bytes(),
        vh[VAB_HEADER_SIZE..VAB_HEADER_SIZE + 16]
    );
    let tones_at = VAB_HEADER_SIZE + VAB_PROGRAM_SLOTS * VAB_PROGRAM_SIZE;
    assert_eq!(expected_tone.to_le_bytes(), vh[tones_at..tones_at + 32]);
}

#[test]
fn vab_rejects_inconsistent_lengths_and_offset_table() {
    let (vh, vb, _, _) = synthetic_vab();
    assert!(matches!(
        VabBank::parse(&vh[..vh.len() - 1], &vb),
        Err(VabError::HeaderLength { .. })
    ));
    assert!(matches!(
        VabBank::parse(&vh, &vb[..vb.len() - 1]),
        Err(VabError::FileSize { .. })
    ));
    let mut bad_offset = vh;
    let offsets_at = bad_offset.len() - VAB_OFFSET_TABLE_SIZE;
    bad_offset[offsets_at] = 1;
    assert_eq!(
        VabBank::parse(&bad_offset, &vb).unwrap_err(),
        VabError::NonzeroFirstSampleOffset(1)
    );
}

fn synthetic_sequence() -> Vec<u8> {
    let header = SeqHeader {
        form: SEQ_MAGIC,
        version: 1,
        resolution: 480,
        tempo: 500_000,
        time_signature_numerator: 4,
        time_signature_denominator_shift: 2,
    };
    let mut bytes = header.to_be_bytes().unwrap().to_vec();
    bytes.extend_from_slice(&[
        0x00, 0xc1, 0x1d, // program change
        0x00, 0x20, // running-status program change
        0x01, 0x91, 0x29, 0x65, // note on
        0x81, 0x00, 0x29, 0x00, // delta 128, running-status note off by velocity zero
        0x00, 0xff, 0x51, 0x03, 0x07, 0xa1, 0x20, // tempo meta-event
        0x00, 0xff, 0x2f, 0x00, // end of track
    ]);
    bytes
}

#[test]
fn seq_reads_deltas_running_status_meta_and_roundtrips() {
    let bytes = synthetic_sequence();
    let sequence = Sequence::parse(&bytes).unwrap();
    assert_eq!(sequence.encoded(), bytes);
    assert_eq!(sequence.header.version, 1);
    assert_eq!(sequence.header.resolution, 480);
    assert_eq!(sequence.header.tempo, 500_000);
    let mut reader = sequence.events();
    let mut events = Vec::new();
    while let Some(event) = reader.next() {
        events.push(event.unwrap());
    }
    assert_eq!(reader.position(), bytes.len());
    assert!(reader.remainder().is_empty());
    assert_eq!(events.len(), 6);
    assert_eq!(events[3].delta, 128);
    assert_eq!(events[3].delta_len, 2);
    match events[1].kind {
        SeqEventKind::Channel(event) => {
            assert_eq!(event.message, ChannelMessage::ProgramChange);
            assert_eq!(event.data1, 0x20);
            assert!(event.used_running_status);
        }
        _ => panic!("expected channel event"),
    }
    match events[4].kind {
        SeqEventKind::Meta { meta_type, data } => {
            assert_eq!(meta_type, 0x51);
            assert_eq!(data, [0x07, 0xa1, 0x20]);
        }
        _ => panic!("expected meta event"),
    }
    assert_eq!(
        events
            .iter()
            .map(|event| event.encoded.len())
            .sum::<usize>()
            + 15,
        bytes.len()
    );
    let mut output = vec![0; bytes.len()];
    assert_eq!(sequence.encode_into(&mut output).unwrap(), bytes.len());
    assert_eq!(output, bytes);
    assert_eq!(sequence.header.to_be_bytes().unwrap(), bytes[..15]);
}

#[test]
fn variable_length_boundaries_roundtrip() {
    for value in [
        0,
        0x7f,
        0x80,
        0x3fff,
        0x4000,
        0x1f_ffff,
        0x20_0000,
        0x0fff_ffff,
    ] {
        let mut encoded = [0; 4];
        let size = encode_variable_length(value, &mut encoded).unwrap();
        assert_eq!(
            read_variable_length(&encoded[..size]).unwrap(),
            (value, size)
        );
    }
    assert!(matches!(
        read_variable_length(&[0x81, 0x80, 0x80, 0x80, 0]),
        Err(SeqError::VariableLengthTooLong { .. })
    ));
}

#[test]
fn seq_rejects_missing_running_status_and_truncated_meta_payload() {
    let header = SeqHeader {
        form: SEQ_MAGIC,
        version: 1,
        resolution: 60,
        tempo: 500_000,
        time_signature_numerator: 4,
        time_signature_denominator_shift: 2,
    }
    .to_be_bytes()
    .unwrap();
    let mut missing = header.to_vec();
    missing.extend_from_slice(&[0, 0x40]);
    assert!(matches!(
        Sequence::parse(&missing).unwrap().events().next().unwrap(),
        Err(SeqError::MissingRunningStatus { .. })
    ));
    let mut truncated = header.to_vec();
    truncated.extend_from_slice(&[0, 0xff, 0x51, 3, 0x07]);
    assert!(matches!(
        Sequence::parse(&truncated)
            .unwrap()
            .events()
            .next()
            .unwrap(),
        Err(SeqError::Truncated { .. })
    ));
}

#[test]
fn seq_runtime_preserves_private_bytes_and_emits_typed_spu_effects() {
    let header = SeqHeader {
        form: SEQ_MAGIC,
        version: 1,
        resolution: 480,
        tempo: 500_000,
        time_signature_numerator: 4,
        time_signature_denominator_shift: 2,
    };
    let mut bytes = header.to_be_bytes().unwrap().to_vec();
    bytes.extend_from_slice(&[
        0, 0xc1, 29, // explicit program change, channel 1
        0, 32, // running program change
        1, 0x91, 41, 101, // note on
        0, 41, 0, // running note on with zero velocity: key off
        0, 0xb5, 7, 118, // channel-volume controller
        0, 0xe4, 0, 64, // pitch bend
        0, 0xff, 0x2f, 0, // end of track
    ]);
    let sequence = Sequence::parse(&bytes).unwrap();
    let events = sequence.events().collect::<Result<Vec<_>, _>>().unwrap();
    let mut score = [0xa5; SEQ_SCORE_RECORD_SIZE];
    score[104..106].fill(0);
    initialize_sequence_score(&sequence, 0x800c_0000, 3, 60, &mut score).unwrap();

    assert_eq!(score[0], 0xa5, "initializer must preserve private bytes");
    assert_eq!(score[61], 0xa5, "initializer does not write running status");
    assert_eq!(
        u32::from_le_bytes(score[48..52].try_into().unwrap()),
        0x800c_0010
    );
    assert_eq!(u32::from_le_bytes(score[160..164].try_into().unwrap()), 120);
    assert_eq!(
        u16::from_le_bytes(score[144..146].try_into().unwrap()),
        u16::MAX
    );
    assert_eq!(u16::from_le_bytes(score[146..148].try_into().unwrap()), 160);

    let mut callbacks = Vec::new();
    for (index, event) in events.iter().enumerate() {
        callbacks.push(
            apply_sequence_event(event, events.get(index + 1), 0x800c_0000, 2, &mut score).unwrap(),
        );
    }
    assert_eq!(score[89], 32);
    assert_eq!(u16::from_le_bytes(score[122..124].try_into().unwrap()), 118);
    assert_eq!(u16::from_le_bytes(score[106..108].try_into().unwrap()), 1);
    assert_eq!(
        u32::from_le_bytes(score[48..52].try_into().unwrap()),
        0x800c_0010
    );
    assert_eq!(u32::from_le_bytes(score[156..160].try_into().unwrap()), 0);
    assert_eq!(
        callbacks[2],
        Some(SeqCallback::KeyOn {
            sequence: 2,
            vab_id: 3,
            program: 32,
            note: 41,
            volume: 101,
            pan: 64,
        })
    );
    assert_eq!(
        callbacks[3],
        Some(SeqCallback::KeyOff {
            sequence: 2,
            vab_id: 3,
            program: 32,
            note: 41,
        })
    );
    assert_eq!(
        callbacks[5],
        Some(SeqCallback::PitchBend {
            sequence: 2,
            vab_id: 3,
            program: 4,
            value: 64,
        })
    );
}

#[test]
fn game_sequence_open_preserves_audio_state_and_uses_provider_slot() {
    let mut audio_state = [0xa5; GAME_AUDIO_STATE_SIZE];
    audio_state[8..12].copy_from_slice(&0x8018_0000u32.to_le_bytes());
    let mut open_flags = 1 << 2;

    apply_successful_game_sequence_open(&mut audio_state, &mut open_flags, 7).unwrap();

    assert_eq!(audio_state[0], 0xa5);
    assert_eq!(audio_state[8..12], 0x8018_0000u32.to_le_bytes());
    assert_eq!(
        i16::from_le_bytes(audio_state[12..14].try_into().unwrap()),
        7
    );
    assert_eq!(
        u32::from_le_bytes(audio_state[16..20].try_into().unwrap()),
        1
    );
    assert_eq!(audio_state[20], 0xa5);
    assert_eq!(open_flags, (1 << 2) | (1 << 7));
}

fn chunks(bytes: &[u8]) -> Vec<&[u8]> {
    let mut result = Vec::new();
    let mut at = 0;
    while at < bytes.len() {
        let size = u32::from_le_bytes(bytes[at..at + 4].try_into().unwrap()) as usize;
        result.push(&bytes[at + 4..at + 4 + size]);
        at += 4 + size;
    }
    assert_eq!(at, bytes.len());
    result
}

#[test]
#[ignore = "requires proprietary SLPS-00017 files via KF_RETAIL_DIR"]
fn shipped_vab_and_seq_corpus_parses_and_roundtrips() {
    use std::fs;
    use std::path::PathBuf;

    let configured = PathBuf::from(
        std::env::var_os("KF_RETAIL_DIR")
            .expect("set KF_RETAIL_DIR to the extracted retail disc directory"),
    );
    let root = if configured.join("KF").is_dir() {
        configured.join("KF")
    } else {
        configured
    };
    let expected = [
        (21_536, 489_792, 37, 77, 49),
        (28_192, 481_920, 50, 90, 51),
        (26_144, 487_408, 46, 74, 53),
        (26_144, 487_664, 46, 78, 54),
        (25_632, 488_896, 45, 88, 55),
    ];
    for (floor, &(vh_len, vb_len, programs, tones, samples)) in (1..=5).zip(expected.iter()) {
        let mixa = fs::read(root.join(format!("B{floor}/MIXA.DAT"))).unwrap();
        let chunks = chunks(&mixa);
        assert_eq!((chunks[0].len(), chunks[1].len()), (vh_len, vb_len));
        let bank = VabBank::parse(chunks[0], chunks[1]).unwrap();
        assert_eq!(
            (
                bank.header.program_count,
                bank.header.tone_count,
                bank.header.sample_count,
            ),
            (programs, tones, samples)
        );
        assert_eq!(
            bank.samples()
                .map(|sample| sample.data.len())
                .sum::<usize>(),
            vb_len
        );
        let mut vh_output = vec![0; vh_len];
        let mut vb_output = vec![0; vb_len];
        bank.encode_header_into(&mut vh_output).unwrap();
        bank.encode_body_into(&mut vb_output).unwrap();
        assert_eq!(vh_output, chunks[0]);
        assert_eq!(vb_output, chunks[1]);
    }

    let files = [
        "B1/SND0.SEQ",
        "B1/SND1.SEQ",
        "B1/SND2.SEQ",
        "B2/SND0.SEQ",
        "B2/SND1.SEQ",
        "B3/SND0.SEQ",
        "B4/SND0.SEQ",
        "B5/SND0.SEQ",
        "B5/SND1.SEQ",
    ];
    let mut event_count = 0;
    let mut running_count = 0;
    for relative in files {
        let bytes = fs::read(root.join(relative)).unwrap();
        let sequence = Sequence::parse(&bytes).unwrap();
        let mut saw_end = false;
        for event in sequence.events() {
            let event = event.unwrap_or_else(|error| panic!("{relative}: {error}"));
            event_count += 1;
            if let SeqEventKind::Channel(channel) = event.kind {
                running_count += usize::from(channel.used_running_status);
            }
            if matches!(
                event.kind,
                SeqEventKind::Meta {
                    meta_type: 0x2f,
                    data: []
                }
            ) {
                saw_end = true;
            }
        }
        assert!(saw_end, "{relative}: no end-of-track event");
        let mut output = vec![0; bytes.len()];
        sequence.encode_into(&mut output).unwrap();
        assert_eq!(output, bytes);
    }
    assert!(event_count > 10_000);
    assert!(running_count > 1_000);
}
