//! Host-only request adapter for the allocation-free VAB and SEQ codecs.

use kf_codec::audio::{
    apply_sequence_event, apply_successful_game_sequence_open, initialize_sequence_score,
    SeqCallback, SeqEventKind, Sequence, VabBank, GAME_AUDIO_STATE_SIZE, SEQ_SCORE_RECORD_SIZE,
    VAB_TONES_PER_PROGRAM,
};

const SEQUENCE_ADDRESS: u32 = 0x800c_0000;
const VBLANK_MINUS: u32 = 60;

fn put_u16(output: &mut Vec<u8>, value: u16) {
    output.extend_from_slice(&value.to_le_bytes());
}

fn put_u32(output: &mut Vec<u8>, value: usize) -> Result<(), String> {
    output.extend_from_slice(
        &u32::try_from(value)
            .map_err(|_| "audio output offset exceeds u32")?
            .to_le_bytes(),
    );
    Ok(())
}

fn put_word(output: &mut Vec<u8>, value: u32) {
    output.extend_from_slice(&value.to_le_bytes());
}

fn signed_word(value: i16) -> u32 {
    i32::from(value) as u32
}

fn put_callback(output: &mut Vec<u8>, callback: SeqCallback) {
    let (tag, words) = match callback {
        SeqCallback::KeyOn {
            sequence,
            vab_id,
            program,
            note,
            volume,
            pan,
        } => (
            1,
            [
                signed_word(sequence),
                signed_word(vab_id),
                u32::from(program),
                u32::from(note),
                signed_word(volume),
                u32::from(pan),
            ],
        ),
        SeqCallback::KeyOff {
            sequence,
            vab_id,
            program,
            note,
        } => (
            2,
            [
                signed_word(sequence),
                signed_word(vab_id),
                u32::from(program),
                u32::from(note),
                0,
                0,
            ],
        ),
        SeqCallback::PitchBend {
            sequence,
            vab_id,
            program,
            value,
        } => (
            3,
            [
                signed_word(sequence),
                signed_word(vab_id),
                u32::from(program),
                u32::from(value),
                0,
                0,
            ],
        ),
    };
    put_word(output, tag);
    for word in words {
        put_word(output, word);
    }
}

fn execute_vab(blocks: Vec<Vec<u8>>) -> Result<Vec<Vec<u8>>, String> {
    let [vh, vb] = blocks.as_slice() else {
        return Err("audio-vab expects VH and VB blocks".into());
    };
    let bank = VabBank::parse(vh, vb).map_err(|error| error.to_string())?;

    let mut programs = Vec::with_capacity(128 * 16);
    for program in bank.program_slots() {
        programs.extend_from_slice(&program.to_le_bytes());
    }

    // Keep all fixed tone slots, including unused zero/reserved rows.  This
    // makes the typed reconstruction cover the complete caller-owned VH.
    let tone_count = usize::from(bank.header.program_count) * VAB_TONES_PER_PROGRAM;
    let mut tones = Vec::with_capacity(tone_count * 32);
    for index in 0..tone_count {
        let program = index / VAB_TONES_PER_PROGRAM;
        let tone = index % VAB_TONES_PER_PROGRAM;
        tones.extend_from_slice(
            &bank
                .tone(program, tone)
                .ok_or("VAB tone slot unexpectedly absent")?
                .to_le_bytes(),
        );
    }

    // Each sample descriptor is <u16 index,u16 size_units,u32 offset,u32 size>.
    let mut samples = Vec::with_capacity(usize::from(bank.header.sample_count) * 12);
    for sample in bank.samples() {
        put_u16(&mut samples, sample.index);
        put_u16(&mut samples, sample.size_units);
        put_u32(&mut samples, sample.offset)?;
        put_u32(&mut samples, sample.data.len())?;
    }

    let mut header_roundtrip = vec![0; vh.len()];
    let mut body_roundtrip = vec![0; vb.len()];
    bank.encode_header_into(&mut header_roundtrip)
        .map_err(|error| error.to_string())?;
    bank.encode_body_into(&mut body_roundtrip)
        .map_err(|error| error.to_string())?;
    Ok(vec![
        bank.header.to_le_bytes().to_vec(),
        programs,
        tones,
        samples,
        header_roundtrip,
        body_roundtrip,
    ])
}

fn execute_seq(blocks: Vec<Vec<u8>>) -> Result<Vec<Vec<u8>>, String> {
    let mut score = [0; SEQ_SCORE_RECORD_SIZE];
    let mut audio_state = [0; GAME_AUDIO_STATE_SIZE];
    audio_state[8..12].copy_from_slice(&SEQUENCE_ADDRESS.to_le_bytes());
    let mut allocated_sequence_id = 0i16;
    let mut open_flags = 0u32;
    let bytes = match blocks.as_slice() {
        [bytes] => bytes,
        [bytes, score_seed] if score_seed.len() == SEQ_SCORE_RECORD_SIZE => {
            score.copy_from_slice(score_seed);
            bytes
        }
        [bytes, score_seed, audio_seed, provider]
            if score_seed.len() == SEQ_SCORE_RECORD_SIZE
                && audio_seed.len() == GAME_AUDIO_STATE_SIZE
                && provider.len() == 6 =>
        {
            score.copy_from_slice(score_seed);
            audio_state.copy_from_slice(audio_seed);
            allocated_sequence_id = i16::from_le_bytes(provider[..2].try_into().unwrap());
            open_flags = u32::from_le_bytes(provider[2..6].try_into().unwrap());
            bytes
        }
        [_bytes, score_seed] => {
            return Err(format!(
                "audio-seq score seed is {} bytes, expected {SEQ_SCORE_RECORD_SIZE}",
                score_seed.len()
            ))
        }
        [_bytes, score_seed, audio_seed, provider] => {
            return Err(format!(
                "audio-seq context sizes are score {}, audio {}, provider {}; expected \
                 {SEQ_SCORE_RECORD_SIZE}, {GAME_AUDIO_STATE_SIZE}, 6",
                score_seed.len(),
                audio_seed.len(),
                provider.len()
            ))
        }
        _ => {
            return Err(
                "audio-seq expects SEQ; optional score; or SEQ, score, audio state, provider"
                    .into(),
            )
        }
    };
    let sequence = Sequence::parse(bytes).map_err(|error| error.to_string())?;
    if sequence.header.tempo == 0 {
        return Err("SEQ initial tempo is zero".into());
    }

    // Record stream: u32 count, followed by a 20-byte header and exact encoded
    // bytes for every event. Header fields are offset, delta, delta_len, kind,
    // status, flags, data1, data2, payload_len, and encoded_len. `data2 == ff`
    // represents absence and flags bit 0 denotes MIDI running status.
    let mut events = Vec::new();
    events.extend_from_slice(&0u32.to_le_bytes());
    let mut event_count = 0u32;
    let mut decoded_events = Vec::new();
    for event in sequence.events() {
        let event = event.map_err(|error| error.to_string())?;
        let offset = u32::try_from(event.offset).map_err(|_| "SEQ event offset exceeds u32")?;
        let delta_len = u8::try_from(event.delta_len).map_err(|_| "SEQ delta exceeds 255 bytes")?;
        let (kind, status, flags, data1, data2, payload_len) = match event.kind {
            SeqEventKind::Channel(channel) => (
                0,
                channel.status,
                u8::from(channel.used_running_status),
                channel.data1,
                channel.data2.unwrap_or(0xff),
                usize::from(channel.data2.is_some()) + 1,
            ),
            SeqEventKind::Meta { meta_type, data } => (1, 0xff, 0, meta_type, 0xff, data.len()),
            SeqEventKind::SystemExclusive { status, data } => (2, status, 0, 0, 0xff, data.len()),
            SeqEventKind::System { status, data } => (3, status, 0, 0, 0xff, data.len()),
        };
        events.extend_from_slice(&offset.to_le_bytes());
        events.extend_from_slice(&event.delta.to_le_bytes());
        events.extend_from_slice(&[delta_len, kind, status, flags, data1, data2]);
        put_u16(
            &mut events,
            u16::try_from(payload_len).map_err(|_| "SEQ event payload exceeds u16")?,
        );
        put_u32(&mut events, event.encoded.len())?;
        events.extend_from_slice(event.encoded);
        event_count = event_count
            .checked_add(1)
            .ok_or("SEQ event count exceeds u32")?;
        decoded_events.push(event);
    }
    events[..4].copy_from_slice(&event_count.to_le_bytes());

    let active_vab_id = i16::from_le_bytes(audio_state[4..6].try_into().unwrap());
    initialize_sequence_score(
        &sequence,
        SEQUENCE_ADDRESS,
        active_vab_id,
        VBLANK_MINUS,
        &mut score,
    )
    .map_err(|error| error.to_string())?;
    apply_successful_game_sequence_open(&mut audio_state, &mut open_flags, allocated_sequence_id)
        .map_err(|error| error.to_string())?;
    let beats_per_minute = u32::from_le_bytes(score[160..164].try_into().unwrap());
    let mut initialized = Vec::with_capacity(16);
    initialized.extend_from_slice(&u32::from(sequence.header.resolution).to_le_bytes());
    initialized.extend_from_slice(&beats_per_minute.to_le_bytes());
    let initial_ticks = u32::from_le_bytes(score[152..156].try_into().unwrap());
    initialized.extend_from_slice(&initial_ticks.to_le_bytes());
    let initial_cursor =
        u32::from_le_bytes(score[48..52].try_into().unwrap()).wrapping_sub(SEQUENCE_ADDRESS);
    put_word(&mut initialized, initial_cursor);

    let initialized_score = score.to_vec();
    let mut score_states = Vec::with_capacity(4 + decoded_events.len() * SEQ_SCORE_RECORD_SIZE);
    put_word(
        &mut score_states,
        u32::try_from(decoded_events.len()).map_err(|_| "SEQ event count exceeds u32")?,
    );
    let mut callbacks = Vec::new();
    put_word(
        &mut callbacks,
        u32::try_from(decoded_events.len()).map_err(|_| "SEQ event count exceeds u32")?,
    );
    for (index, event) in decoded_events.iter().enumerate() {
        let callback = apply_sequence_event(
            event,
            decoded_events.get(index + 1),
            SEQUENCE_ADDRESS,
            0,
            &mut score,
        )
        .map_err(|error| format!("SEQ event {index}: {error}"))?;
        score_states.extend_from_slice(&score);
        put_word(&mut callbacks, u32::from(callback.is_some()));
        if let Some(callback) = callback {
            put_callback(&mut callbacks, callback);
        }
    }

    let mut roundtrip = vec![0; bytes.len()];
    sequence
        .encode_into(&mut roundtrip)
        .map_err(|error| error.to_string())?;
    Ok(vec![
        sequence
            .header
            .to_be_bytes()
            .map_err(|error| error.to_string())?
            .to_vec(),
        events,
        initialized,
        roundtrip,
        initialized_score,
        score_states,
        callbacks,
        audio_state.to_vec(),
        open_flags.to_le_bytes().to_vec(),
    ])
}

pub fn execute(operation: &str, blocks: Vec<Vec<u8>>) -> Result<Vec<Vec<u8>>, String> {
    match operation {
        "audio-vab" => execute_vab(blocks),
        "audio-seq" => execute_seq(blocks),
        _ => Err(format!("unsupported audio operation {operation}")),
    }
}
